#include "BDDSampler.h"
#include "../../BDD/include/BDDFactory.h"
#include "Plotter.h"
#include "vara/Feature/FeatureModelParser.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
extern "C" {
#include <oxidd/bdd.hpp>
}

using std::string;
using std::vector;


int main(int argc, char* argv[]) noexcept(false){ 

    if (argc < 2) {
        std::cerr << "Usage: ./my_program <feature_model.xml>\n";
        return 1;
    }

    std::vector<std::string> Args(argv + 1, argv + argc);
    std::string FilePath = Args[1]; // Use std::string constructor directly

    // Lambda function to load and parse the feature model
    std::unique_ptr<vara::feature::FeatureModel> Fd = [&]() {
        // Read the file content
        std::ifstream FileIn(FilePath);
        if (!FileIn) {
            throw std::runtime_error("Could not open file: " + FilePath);
        }
        std::ostringstream Oss;
        Oss << FileIn.rdbuf();
        std::string XMLContent = Oss.str();

        // Parse the content
        vara::feature::FeatureModelXmlParser Parser(XMLContent);

        // Verify the feature model is valid
        auto Verify = Parser.verifyFeatureModel();
        if(!Verify) {
            throw std::runtime_error("Error parsing XML: verification failed");
        }   

        // Build the feature model object
        auto Fm = Parser.buildFeatureModel();
        if(!Fm) {
            throw std::runtime_error("Error building Feature Model: ");
        }

        //(void)vara::feature::FeatureModelXmlParser::detectXMLAlternatives(*fm);

        return Fm;
    }();

    std::cout << "Feature Model loaded successfully from: " << FilePath <<'\n';

    // Create BDD factory and convert feature model to BDD
    bdd::sample::BDDFactory Factory;
    oxidd::bdd_function FinalBDD = Factory.modelToBdd(*Fd);
    oxidd::bdd_manager Manager = FinalBDD.containing_manager();
    std::cout << "BDD constructed successfully." << '\n';

    // Generate a single sample configuration
    std::unordered_map<oxidd::capi::oxidd_var_no_t, bool> Sample = generateConfiguration(
        Manager, 
        FinalBDD, 
        Factory
    );

    // Initialize frequency counts for features
    std::unordered_map<oxidd::capi::oxidd_var_no_t, bdd::sample::Freq> Counts;
    size_t N = 10; // Number of samples to generate

    // Generate multiple samples and update frequency counts
    for(size_t I=0; I<N; ++I) {
        auto S = generateConfiguration(
            Manager, 
            FinalBDD, 
            Factory
        );
        bdd::sample::updateCounts(S, Counts);
    }

    // Convert counts to readable rows and print them
    auto Rows = bdd::sample::toRows(Counts, &Factory.VarMap);
    
    for(const auto& R : Rows) {
        std::cout << R.Label << " (id=" << R.V << "): "
                  << R.P << "  [" << R.T << "/" << R.N << "]\n";
    }

    // Write the frequency data to CSV file
    bdd::sample::writeCsv(Rows, "freq.csv");

    return 0;
 }



//     oxidd::capi::BDDFactory factory;
//     oxidd::bdd_function finalBDD = factory.modelToBdd(*fd); 
//     auto sample =  oxidd::capi::generateConfiguration(
//                 finalBDD,
//                 factory
//             );

//     std::cout << "Sampled Configuration:\n";
//     for (const auto& [level, enabled] : sample) {
//         std::cout << "  level " << level
//                 << " -> " << (enabled ? "true" : "false") << '\n';
//     }


//     return 0;

    // auto children = fd->getRoot()->getChildren<Feature>();
    // for(Feature *c : children) {
    //     std::cout << "Child Feature: " << c->getName().str() << std::endl;
    //     if(c->isOptional()) {
    //         std::cout << "  - Optional Feature" << std::endl;
    //     } else {
    //         std::cout << "  - Mandatory Feature" << std::endl;
    //     }
    //     checkType(*fd, c);
    //     auto child = c->getChildren<Feature>();
    //     if (child.empty()) {
    //         std::cout << "  - Leaf Feature" << std::endl;
    //     } else {
    //         std::cout << "  - Non-Leaf Feature with children: ";
    //         for(Feature *cici : child) {
    //             std::cout << cici->getName().str() << " ";
    //             checkType(*fd, cici);
    //         }
    //     }
        // switch(checkType(*fd, c)) {
        //     case NodeType::AND:
        //         std::cout << "  - AND Node" << std::endl;
        //         break;
        //     case NodeType::OR:
        //         std::cout << "  - OR Node" << std::endl;
        //         break;
        //     case NodeType::XOR:
        //         std::cout << "  - XOR Node" << std::endl;
        //         break;
        //     case NodeType::LEAF:
        //         std::cout << "  - Leaf Node" << std::endl;
        //         if(c->getChildren<Feature>().empty()) {
        //             std::cout << "    - No children, this is a leaf node." << std::endl;
        //         } else {
        //             std::cout << "    - Has children, not a true leaf." << std::endl;
        //         }
        //         break;
        //     default:
        //         std::cout << "  - Unknown Node Type" << std::endl;
        // }
    
    // std::cout << "Parsed Feature Model: " << fd->getRoot()->getName().str() << std::endl;
    // std::unordered_map<Feature*, int> cc = count_valid_configs_from_featureModel(xmlPath, fd.get());

    // oxidd::bdd_manager mgr;
    // unordered_map<std::string, oxidd::bdd_function> featureVars;
    // vector<pair<oxidd::bdd_function, std::string>> varList;
    
    // std::random_device rd;
    // unordered_map<Feature * , bool> sample = sampleRandomly(*fd, cc, rd);


    // for (const auto &entry : sample) {
    //     std::cout << entry.first->getName().str() << ": " << (entry.second ? "true" : "false") << std::endl;
    // }

    // return 0;




    // oxidd::bdd_manager mgr(32, 320, 1);

    // oxidd::bdd_function a = mgr.new_var();
    // oxidd::bdd_function b = mgr.new_var();
    // oxidd::bdd_function c =  mgr.new_var();
    // oxidd::bdd_function f = a & b | ~a & c | b & ~c;

    // const oxidd::bdd_manager* c_mgr = reinterpret_cast<const oxidd::bdd_manager*>(&mgr);

    // const oxidd::bdd_function* c_a = reinterpret_cast<const oxidd::bdd_function*>(&a);
    // const oxidd::bdd_function* c_b = reinterpret_cast<const oxidd::bdd_function*>(&b);
    // const oxidd::bdd_function* c_c = reinterpret_cast<const oxidd::bdd_function*>(&c);
    // const oxidd::bdd_function* c_f = reinterpret_cast<const oxidd::bdd_function*>(&f);

    // const oxidd::bdd_function functions[] = { *c_f };
    // const char* function_names[] = { "Function" };

    // const oxidd::bdd_function vars[] = { *c_a, *c_b, *c_c };
    // const char* var_names[] = { "a", "b", "c"};

    // bool check = oxidd_bdd_manager_dump_all_dot_file(
    //     *c_mgr,
    //     "../results/bdd.dot",
    //     functions,
    //     function_names,
    //     1,
    //     vars,
    //     var_names,
    //     3
    // );