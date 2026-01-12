#include "BDDFactory.h"
#include "BDDFeats.h"
#include "Constraints.h"
#include "Probabilities.h"
#include "oxidd/bdd.hpp"
#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

namespace bdd::sample    
{ 
    // Conversion of the given feature model to a BDD representation
    oxidd::bdd_function BDDFactory::modelToBdd(
        const vara::feature::FeatureModel &Model
    ) {
       if(Model.size()== 0) {
        std::cerr << "Feature model is empty\n";
       }
       // Store names of features in XOR relationships (Z3)
       std::vector<std::string> V; 

       fillManager(Model);

       // Process XOR relationships from the feature model (Z3)
       if(!Model.relationships().empty()) {
        for(const auto &S: Model.relationships()){
            for(const auto &Child: S->children()) {
                const auto *ChildFeature = (const vara::feature::Feature *)Child;
                V.insert(V.begin(), ChildFeature->getName().str());
            }
        }
       } else {
        std::cerr << "Feature model has no XOR relationships\n";
       }

       // Process each feature: add to VarMap and add children-parent relationships to FinalBdd (Z3)
       for(auto *F: Model.features()) { 
        auto R = featureToBdd(
            Manager,
            std::ranges::find(V, F->getName().str()) != V.end(),
            *F,
            //VarMap,
            FinalBdd
            );
        if(!R) {
            continue;
        }
       }

       std::cout << "Passed feature processing\n";

       // Process boolean constraints from the feature model (Z3)
       BDDConstraintVisitor Visitor(Manager, /*VarMap*/ FinalBdd, false, false);    
       for (const auto &C : Model.booleanConstraints()) {
            if (!processConstraints(Visitor, C, FinalBdd)) { break; }
        }

       std::cout << "Passed processing constraints" << '\n';

       auto R = getPr(
        Manager,
        FinalBdd,
        &SatMap,
        *this
       );

       if(!R){
            std::cerr << "Error calculating probabilities." << '\n';
        }

       /*For Testing*/
       std::string_view DiagramName = "7z";
       std::vector<oxidd::bdd_function> Funcs = {FinalBdd};
       auto Result = Manager.visualize(DiagramName, Funcs);
       Manager.export_dddmp("hippacc.dddmp", Funcs).value();

       return FinalBdd;
    }

    // Add all features to manager including their names
    void BDDFactory::fillManager(
        const vara::feature::FeatureModel &Model
    ) { 
        // Collect all features from the model
        std::vector<const vara::feature::Feature*> Features;
        Features.reserve(Model.size());
        for(auto* F: Model.features()) {
            Features.push_back(F);
        }

        // Extract feature names
        std::vector<std::string> Names;
        Names.reserve(Features.size());
        for (const auto* F: Features) {
            Names.push_back(F->getName().str());
        }

        // Convert to C-style strings for oxidd API
        std::vector<const char*> NamesCstr;
        NamesCstr.reserve(Names.size());
        for (auto & Name: Names) { 
            NamesCstr.push_back(Name.c_str());
        }

        std::span<const char*> NamesCstrSpan(NamesCstr);

        // Add all named variables to the BDD manager
        auto  Res = Manager.add_named_vars(NamesCstrSpan);
    }
} // namespace bdd::sample