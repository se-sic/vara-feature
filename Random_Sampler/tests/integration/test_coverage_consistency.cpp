#include "BDDFactory.h"
#include "BDDSampler.h"
#include "CoverageIntegration.h"
#include "test_helpers.h"
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>

extern "C" {
#include <oxidd/bdd.hpp>
}

int main(int argc, char* argv[]) {
    std::cout << "\033[34m" << "COVERAGE CONSISTENCY TEST" << "\033[0m\n";

    
    try {

        if (argc != 4) {
            std::cerr << "\033[31m" << "Usage: ./my_program <file1.xml> <file2.xml> <integer_value> \033[0m\n";
            return 1;
        }

        std::vector<std::string> Args(argv + 1, argv + argc);

        const std::string& ConstrainedXmlFile  = Args[0];
        const std::string& UnconstrainedXmlFile = Args[1];

        size_t SampleSize = 0;

        try {
            SampleSize = std::stoi(Args[2]);
        } catch (const std::invalid_argument& E) {
            std::cerr << "\033[31m" << "Error: Third argument must be an integer.\033[0m\n";
            return 1;
        } catch (const std::out_of_range& E) {
            std::cerr << "\033[31m" << "Error: Integer value is out of range.\033[0m\n";
            return 1;
        }
        
        std::ifstream TestFile(UnconstrainedXmlFile);
        if (!TestFile) {
            std::cout << "\033[31m" << "Unconstrained file not found!\033[0m\n";
            return 1;
        }
        TestFile.close();
        std::cout << "\033[32m" << "Unconstrained file xml" << "\033[0m\n";
        
        // Load constrained model and generate sample
        std::cout << "\033[34m" << "LOADING CONSTRAINED MODEL" << "\033[0m\n";
        
        auto Fm1 = test_utils::loadFeatureModel(ConstrainedXmlFile);
        std::cout << "Loaded constrained.xml " << Fm1->size() << " features" << "\033[0m\n";
        
        bdd::sample::BDDFactory Factory1;
        oxidd::bdd_function Bdd1 = Factory1.modelToBdd(*Fm1);
        oxidd::bdd_manager Manager1 = Bdd1.containing_manager();
        std::cout << "BDD constructed" << "\033[0m\n";
        
        // Generate small test sample
        std::cout << "\n Generating test sample ("<< SampleSize << " configs)" << "\033[0m\n";
        std::vector<std::vector<bool>> Sample;
        for (size_t I = 0; I < SampleSize; ++I) {
            auto Config = bdd::sample::generateConfiguration(
                Manager1, Bdd1, Factory1, &Factory1.SatMap);
            Sample.push_back(Config);
        }
        std::cout << "Generated " << Sample.size() << " configs" << "\033[0m\n";
        
        // Save sample for inspection
        test_utils::saveConfigsToCSV(Sample, *Fm1, "test_data/test_sample.csv");
        std::cout << "Saved to test_data/test_sample.csv" << "\033[0m\n";
        
        // Evaluate on constrained model
        std::cout << "\033[34m" << "\nEVALUATING WITH CONSTRAINTS" << "\033[0m\n";
        
        coverage::CoverageEvaluator Evaluator1(*Fm1, Bdd1, Manager1, Factory1);
        
        auto Results1Fort2 = Evaluator1.evaluateSample(Sample, 2);
        
        std::cout << "\n Results WITH constraints (t=2):" << "\033[0m\n";
        for (const auto& [metric, cov] : Results1Fort2) {
            std::cout << "  " << std::setw(25) << metric << ": " 
                      << std::fixed << std::setprecision(2) << (cov * 100) << "%" << "\033[0m\n";
        }
        
        // Get covered interactions count
        std::set<coverage::Interaction> Covered1;
        for (const auto& Config : Sample) {
            auto Interactions = Evaluator1.extractInteractionsFromConfig(Config, 2);
            Covered1.insert(Interactions.begin(), Interactions.end());
        } 
        
        // Load unconstrained model
        std::cout << "\033[34m" << "\nLOADING UNCONSTRAINED MODEL" << "\033[0m\n";
        
        auto Fm2 = test_utils::loadFeatureModel(UnconstrainedXmlFile);
        std::cout  << "\033[32m" << "Loaded Unconstrained.xml: " << Fm2->size() << " features" << "\033[0m\n";
        
        bdd::sample::BDDFactory Factory2;
        oxidd::bdd_function Bdd2 = Factory2.modelToBdd(*Fm2);
        oxidd::bdd_manager Manager2 = Bdd2.containing_manager();
        std::cout  << "\033[32m" << "BDD constructed" << "\033[0m\n";
        
        // Evaluate same sample on unconstrained model
        std::cout << "\033[34m" << "\nEVALUATING WITHOUT CONSTRAINTS" << "\033[0m\n";
        
        coverage::CoverageEvaluator Evaluator2(*Fm2, Bdd2, Manager2, Factory2);
        
        auto Results2Andt2 = Evaluator2.evaluateSample(Sample, 2);
        
        std::cout << "\n Results WITHOUT constraints (t=2):" << '\n';
        for (const auto& [metric, cov] : Results2Andt2) {
            std::cout << "  " << std::setw(25) << metric << ": " 
                      << std::fixed << std::setprecision(2) << (cov * 100) << "%" << "\033[0m\n";
        }
        
        // Get covered interactions count
        std::set<coverage::Interaction> Covered2;
        for (const auto& Config : Sample) {
            auto Interactions = Evaluator2.extractInteractionsFromConfig(Config, 2);
            Covered2.insert(Interactions.begin(), Interactions.end());
        }
        
        // Compare covered sets
   
        std::cout << "\033[34m" << "COMPARISON" << "\033[0m\n";
        
        std::cout << "Covered interactions:" << "\033[0m\n";
        std::cout << "  WITH constraints:    " << Covered1.size() << "\033[0m\n";
        std::cout << "  WITHOUT constraints: " << Covered2.size() << "\033[0m\n";
        
        bool CoveredMatch = (Covered1 == Covered2);
        
        if (CoveredMatch) {
            std::cout  << "\033[32m" << "  Status: IDENTICAL" << "\033[0m\n";
        } else {
            std::cout << "\033[31m" << "  Status: DIFFERENT" << "\033[0m\n";
            
            // Show difference
            std::set<coverage::Interaction> OnlyIn1;
            std::set_difference(Covered1.begin(), Covered1.end(),
                              Covered2.begin(), Covered2.end(),
                              std::inserter(OnlyIn1, OnlyIn1.begin()));
            std::cout << "  Only in constrained: " << OnlyIn1.size() << "\033[0m\n";
        }
        
        // Verdict
        std::cout << "\033[34m" << "VERDICT" << "\033[0m\n";
        
        if (CoveredMatch) {
            std::cout  << "\033[32m" << "TEST PASSED!" << "\033[0m\n";
            std::cout << "Covered interaction sets are IDENTICAL." << "\033[0m\n";
            return 0;
        }              
            std::cout << "\033[31m" << "TEST FAILED!" << "\033[0m\n";
            std::cout << "Covered interaction sets are DIFFERENT." << "\033[0m\n";
            return 1;
       
        
    } catch (const std::exception& E) {
        std::cerr << "\033[31m" << "\n ERROR: " << E.what() << "\033[0m\n";
        return 1;
    }
}