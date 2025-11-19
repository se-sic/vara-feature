#include "BDDFactory.h"
#include "BDDFeats.h"
#include "Constraints.h"
#include "Probabilities.h"
#include "oxidd/bdd.hpp"
#include "oxidd/util.hpp"
#include <algorithm>
#include <cstddef>
#include <optional>
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

       std::cout << "Passed relationship processing\n";

       // Process each feature: add to VarMap and add children-parent relationships to FinalBdd (Z3)
       for(auto *F: Model.features()) { 
        auto R = featureToBdd(Manager,std::ranges::find(V, F->getName().str()) != V.end(),*F,VarMap,FinalBdd);
        if(!R) {
            continue;
        }
       }

       std::cout << "Passed feature processing\n";

       // Process boolean constraints from the feature model (Z3)
       BDDConstraintVisitor Visitor(Manager, VarMap, FinalBdd, false, false);    
       for (const auto &C : Model.booleanConstraints()) {
            if (!processConstraints(Visitor, C, FinalBdd)) { break; }
        }

       std::cout << "passed processing constraints" << '\n';

       /*For Testing*/
       std::string_view DiagramName = "HIPPACC";
       std::vector<oxidd::bdd_function> Funcs = {FinalBdd};
       auto Result = Manager.visualize(DiagramName, Funcs);

       // Find the root feature to start probability calculation
       BDDFeat* Root = findFeatureinBDD(&FinalBdd);
       std::optional<oxidd::var_no_t> OptId = Manager.name_to_var(Root->Name);
       oxidd::var_no_t RootId = OptId.value_or(-1);

       // Calculate max level including terminal level
       BDDFeat FMax = VarMap[VarMap.size()-1];
       auto MaxLevelOpt = Manager.name_to_var(FMax.Name);
       if(MaxLevelOpt.has_value()) {
           setMaxLevel(MaxLevelOpt.value() + 2);
        }

        auto R = getPr(
            Manager,
            FinalBdd,
            RootId,
            *Root,
            *this
        );

       if(!R){
            std::cerr << "Error calculating probabilities." << '\n';
        }

       return FinalBdd;
    }

    // Add all features to manager including their names
    void BDDFactory::fillManager(
        const vara::feature::FeatureModel &Model
    ) { 
        std::vector<const vara::feature::Feature*> Features;

        // Collect all features from the model
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
        
        /* For Testing */
        if(Res.has_value()) {
            for(auto & Name: Names) {
                auto Id = Manager.name_to_var(Name);
                if(Id.has_value()){
                    llvm::outs() << "Variable ID for " << Name << " is " << Id.value() << "\n";
                } else{
                    llvm::errs() << "Variable ID for " << Name << " not found\n";
                }
            }
        } else {
            llvm::errs() << "Error adding named variables to BDD manager\n";
        }
    }

    // Given a BDD node, find the corresponding BDDFeat in the varMap
    BDDFactory::BDDFeat*  BDDFactory::findFeatureinBDD(
        oxidd::bdd_function *Node
    ) {

        oxidd::bdd_manager Manager = this->Manager;
        auto Level = Node->node_level();
        for(auto& [id, f]: this->VarMap) {
            if(Level.has_value()){
                auto Lev = Level.value();
                if(id == Lev) {
                    f.BddNode = *Node;
                    return &f;
                }
            } if ((f.Name == "TRUE_TERMINAL" && Node->satisfiable())) {
                return &f;
            } if ((f.Name == "FALSE_TERMINAL" && !Node->satisfiable())) {
                return &f;
            }
        } 



        if(Node->satisfiable()) {
            if( VarMap[VarMap.size()-1].Name == "TRUE_TERMINAL") {
                return &VarMap[VarMap.size()-1];
            }

            auto *Terminal = new BDDFactory::BDDFeat{
                .BddNode = *Node,
                .IsRoot = false,
                .Name = "TRUE_TERMINAL",
                .Marked = false,
                .SatCount = static_cast<size_t>(Node->sat_count_double(0)),
                .Probability = 0.0,
                .CC = Node->sat_count_double(0),
            };

            VarMap[VarMap.size()] = *Terminal;

            return Terminal;
        } 
        
        if (!Node->satisfiable()) {
            if( VarMap[VarMap.size()-1].Name == "FALSE_TERMINAL") {
                return &VarMap[VarMap.size()-1];
            }

            auto *Terminal = new BDDFactory::BDDFeat{
                .BddNode = *Node,
                .IsRoot = false,
                .Name = "FALSE_TERMINAL",
                .Marked = false,
                .SatCount = 0,
                .Probability = 0.0,
                .CC = Node->sat_count_double(0),
            };

            VarMap[VarMap.size()] = *Terminal;

            return Terminal;
        }

        return nullptr;
    }
} // namespace bdd::sample