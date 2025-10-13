#include "BDDFactory.h"
#include "BDDFeats.h"
#include "Constraints.h"
#include "Probabilities.h"
#include "oxidd/bdd.hpp"
#include "oxidd/util.hpp"
#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

namespace bdd::sample    
{ 
    // Convert a feature model to a BDD representation
    oxidd::bdd_function BDDFactory::modelToBdd(
        const vara::feature::FeatureModel &Model
    ) {
        // Check if model is empty
       if(Model.size()== 0) {
        std::cerr << "Feature model is empty." << '\n';
       }

       // Vector to store names of features in XOR relationships
       std::vector<std::string> V; 

       // Add all features to manager including their names
       fillManager(Model);

       // If relationshios of FM are filles, add them to the XOR vetcor V for the function "FeatureToBdd"
       // Process XOR relationships from the feature model
       if(!Model.relationships().empty()) {
        for(const auto &S: Model.relationships()){
            for(const auto &Child: S->children()) {
                const auto *ChildFeature = (const vara::feature::Feature *)Child;
                V.insert(V.begin(), ChildFeature->getName().str());
            }
        }
       } else {
        std::cerr << "Feature model has no XOR relationships." << '\n';
       }

       std::cout << "passed relationships\n";

       // Process each feature: add to varMap and add constraints to finalBdd
       for(auto *F: Model.features()) { 
        auto R = featureToBdd(&Manager,std::ranges::find(V, F->getName().str()) != V.end(),*F,&VarMap,&FinalBdd);
        if(!R) {
            continue; // Skip to the next feature if there is an error
        }
       }

       std::cout << "passed features" << '\n';

       // Process explicit constraints from the feature model
       bdd::sample::BDDConstraintVisitor Visitor(&Manager, &VarMap, FinalBdd, false, false);

       processConstraints(
        Manager,
        FinalBdd,
        VarMap,
        Model
       );

        std::cout << "passed processing constraints" << '\n';

        // Find the root feature to start probability calculation
        BDDFeat* Root = findFeatureinBDD(&VarMap.at(0).BddNode);
        std::optional<oxidd::var_no_t> OptId = Manager.name_to_var(Root->Name);
        oxidd::var_no_t RootId = OptId.value_or(-1);
        size_t NodeCount = Manager.num_inner_nodes();
        std::cout << "Total number of inner nodes in BDD: " << NodeCount << '\n';

        // Calculate probabilities for all features
        auto R = getPr(
            Manager,
            Root->BddNode,
            RootId,
            *Root,
            *this
        );

        if(!R){
            std::cerr << "Error calculating probabilities." << '\n';
        }

        return FinalBdd;
    }

    // Add all features from the model to the BDD manager
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
        
        if(Res.has_value()) {
            for(auto & Name: Names) {
                auto Id = Manager.name_to_var(Name);
                if(Id.has_value()){
                    llvm::outs() << "Variable ID for" << Name << " is " << Id.value() << "\n";
                } else{
                    llvm::errs() << "Variable ID for" << Name << " not found\n";
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
        if(Level.has_value()){
            auto Lev = Level.value();

            for(auto& [id, f]: this->VarMap) {
                if(id == Lev) {
                    return &f;
                }
            }
        }
        return nullptr;
    }
} // namespace bdd::sample


//-------------------------------------------------------- OLD CODE --------------------------------------------------------

//     oxidd::bdd_function BDDFactory::modelToBdd( 
//         const vara::feature::FeatureModel &model) 
//     {
//         oxidd::bdd_function finalBDD = oxidd::bdd_functionrue(manager);
//         std::unordered_map<std::string, oxidd::bdd_function> binaryVarMap;
//         std::unordered_map<string, std::vector<std::pair<string, oxidd::bdd_function>>> numericVarMap;
//         std::vector<string> V;
//         BDDConstraintVisitor visitor = 
//             BDDConstraintVisitor(manager, &varMap, &binaryVarMap, &numericVarMap);

//         // for (const auto &rltsps : model.relationships()) {
//         //     for (const auto &Child : rltsps->children()) {
//         //         const auto *ChildFeature = (const vara::feature::Feature *)Child;
//         //         V.insert(V.begin(), ChildFeature->getName().str());
//         //     }
//         // } //prinzipiell richtig aber besser mit features()

//         for(auto* feats: model.features()) {
//             V.insert(V.begin(), feats->getName().str());
//         }

//         for (auto* F : model.features()) {
//             if (auto R = FeatureToBdd(
//                 manager, 
//                 std::find(V.begin(), V.end(), F->getName().str()) != V.end(),
//                 *F, 
//                 &varMap, 
//                 &binaryVarMap, 
//                 &numericVarMap, 
//                 &finalBDD
//             ); !R) {
//                 continue; // Skip to the next feature if there is an error
//             }
//         }



//         for (auto *C : model.booleanConstraints()) {
//             oxidd::bdd_function constraintBDD = visitor.addConstraint(C->constraint());
//             finalBDD = oxidd_bdd_and(finalBDD, constraintBDD);
//         }

//         for (auto *C : model.nonBooleanConstraints()) {
//             oxidd::bdd_function constraintBDD = visitor.addConstraint(C->constraint());
//             finalBDD = oxidd_bdd_and(finalBDD, constraintBDD);
//         }

//         for (auto *C : model.mixedConstraints()) {
//             BDDConstraintVisitor mixedVisitor = 
//                 BDDConstraintVisitor(manager, &varMap, &binaryVarMap, &numericVarMap, true, true);
//             oxidd::bdd_function constraintBDD = mixedVisitor.addConstraint(C->constraint());
//             finalBDD = oxidd_bdd_and(finalBDD, constraintBDD);
//         }

//         // for (const auto &R : model.relationships()) {
//         //     if (auto Error = RelationshipToBdd(manager, R.get(), &varMap, &finalBDD); !Error) {
//         //         return oxidd::bdd_function{nullptr,0};
//         //     }
//         // }

//         auto fb = oxidd_bdd_false(manager);
//         if (fb._i == finalBDD._i) {
//             oxidd_bdd_unref(finalBDD);
//             return oxidd::bdd_function{nullptr,0}; // Return an invalid BDD if the final BDD is false
//         }

//         // Count SAT solutions
//         oxidd::bdd_function oneTerminal = oxidd::bdd_functionrue(manager);
//         oxidd::bdd_function zeroTerminal = oxidd_bdd_false(manager);
//         size_t nodeCount = oxidd_bdd_node_count(finalBDD);
//         BDDFeat* root = findFeatureinBDD(&finalBDD);

//         auto R = getPr(
//             &manager,
//             &finalBDD,
//             root,
//             nodeCount,
//             &oneTerminal,
//             &zeroTerminal,
//             *this);

//         if (!R) { 
//             oxidd_bdd_unref(finalBDD);
//             return oxidd::bdd_function{nullptr,0}; // Return an invalid BDD if getPr fails
//         };

        
//         return finalBDD;
//     }

//     BDDFactory::BDDFeat*
//     BDDFactory::findFeatureinBDD(
//          oxidd::bdd_function* node
//     )  {
//         for (auto& [name, f] : this->varMap) {
//             if(f.type == featType::BINARY) {
//                 oxidd::bdd_function* node_ptr = std::get<oxidd::bdd_function*>(f.data);
//                 if(node_ptr && node_ptr->_p == node->_p && node_ptr->_i == node->_i) {
//                     return &f;
//                 }
//             } else if (f.type == featType::NUMERIC) {
//                 auto numericFeats = std::get<std::vector<std::pair<string, oxidd::bdd_function>>*>(f.data);
//                 if(numericFeats && !numericFeats->empty()) {
//                     auto& [name, feat] = (*numericFeats)[0];
//                     if(feat._p == node->_p && feat._i == node->_i) {
//                         for(auto& [name2, feat2]: this->varMap){
//                             if(name == name2){
//                                 return &feat2;
//                             }
//                         }
//                     } else {
//                         for (auto& [name3, feat3] : *numericFeats) {
//                             if (feat3._p == node->_p && feat3._i == node->_i) {
//                                 for(auto& [name4, feat4]: this->varMap){
//                                     if(name3 == name4){
//                                         return &feat4;
//                                     }
//                                 }
//                             }
//                         }
//                     }
//                 }
//             }
//         }
//         return nullptr;
//     }
// }