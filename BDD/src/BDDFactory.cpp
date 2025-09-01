#include "BDDFactory.h"
namespace oxidd::capi     
{ 
    oxidd_bdd_t BDDFactory::modelToBdd(
        const vara::feature::FeatureModel &model
    ) {
       if(model.size() == 0) {
        std::cerr << "Feature model is empty." << std::endl;
       }
       // Add all features to manager including their names
       fillManager(model);
       // If relationshios of FM are filles, add them to the XOR vecor V for teh function "FeatureToBdd"
       if(!model.relationships().empty()) {
        for(const auto &R: model.relationships()){
            for(const auto &Child: R->children()) {
                const auto *ChildFeature = (const vara::feature::Feature *)Child;
                V.insert(V.begin(), ChildFeature->getName().str());
                std::cout<< ChildFeature->getName().str() << std::endl;
            }
        }
       } else {
        std::cerr << "Feature model has no XOR relationships." << std::endl;
        continue;
       }
       // For each feature, add it to the global varMap alongside constraints to the finalBdd
       for(auto *F: model.features()) { 
        if(auto R = FeatureToBdd(
            &manager,
            std::find(V.begin(), V.end(), F->getName().str()) != V.end(),
            *F,
            &varMap,
            &finalBdd
        ); !R) {
            continue; // Skip to the next feature if there is an error
        }
       }


       //TODO: Input Michaels Visitor ConstraintCode
       oxidd::capi::processConstraints(
            manager,
            finalBdd,
            varMap,
            model
        );

        // Get the root feature from the varMap to start the probability calculation
        BDDFeat* root = findFeatureinBDD(&finalBdd);
        oxidd_var_no_t rootId = oxidd_bdd_manager_name_to_var(manager, root->name.c_str());

        auto R = getPr(
            &manager,
            &finalBdd,
            rootId,
            root,
            *this
        );

        if (!R) { 
            oxidd_bdd_unref(finalBdd);
            return oxidd_bdd_t{nullptr,0}; // Return an invalid BDD if getPr fails
        };

        return finalBdd;
    }

    // Iterate over all features of the FM, get thier names and add tehem respectiveky to the bdd manager 
    void BDDFactory::fillManager(
        const vara::feature::FeatureModel &model
    ) {
        std::vector<const vara::feature::Feature*> features;
        std::vector<str::string> names;
        std::vector<const char*> names_cstr;

        for(auto* F: model.features()) {
            features.push_back(F);
            names.push_back(F->getName().str());
            names_cstr.push_back(names.back().c_str());
        }

        oxidd_bdd_manager_add_named_vars(
            manager,
            names_cstr.data(),
            static_cast<oxidd_var_no_t>(features.size())
        )

        // std::unordered_map<std::string, oxidd_var_no_t> featMap;
        // featMap.reserve(features.size());
        // for(size_t i=0; i < features.size(); ++i) {
        //     oxidd_var_no_t id  = oxidd_bdd_manager_name_to_var(
        //         manager,
        //         names_cstr[i]
        //     );
        //     std::string name = names[i];
        //     featMap[name] = id;
        // }

        // return featMap;
    }

    // Given a BDD node, find the corresponding BDDFeat in the varMap
    BDDFactory::BDDFeat*  BDDFactory::findFeatureinBDD(
        oxidd_bdd_t *node
    ) {
        for(auto& [id, f]: this->varMap) {
            if(f.bddNode._p == node->_p && f.bddNode._i == node->_i) {
                return &f;
            }
        }
        return nullptr;
    }
}


//-------------------------------------------------------- OLD CODE --------------------------------------------------------

//     oxidd_bdd_t BDDFactory::modelToBdd( 
//         const vara::feature::FeatureModel &model) 
//     {
//         oxidd_bdd_t finalBDD = oxidd_bdd_true(manager);
//         std::unordered_map<std::string, oxidd_bdd_t> binaryVarMap;
//         std::unordered_map<string, std::vector<std::pair<string, oxidd_bdd_t>>> numericVarMap;
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
//             oxidd_bdd_t constraintBDD = visitor.addConstraint(C->constraint());
//             finalBDD = oxidd_bdd_and(finalBDD, constraintBDD);
//         }

//         for (auto *C : model.nonBooleanConstraints()) {
//             oxidd_bdd_t constraintBDD = visitor.addConstraint(C->constraint());
//             finalBDD = oxidd_bdd_and(finalBDD, constraintBDD);
//         }

//         for (auto *C : model.mixedConstraints()) {
//             BDDConstraintVisitor mixedVisitor = 
//                 BDDConstraintVisitor(manager, &varMap, &binaryVarMap, &numericVarMap, true, true);
//             oxidd_bdd_t constraintBDD = mixedVisitor.addConstraint(C->constraint());
//             finalBDD = oxidd_bdd_and(finalBDD, constraintBDD);
//         }

//         // for (const auto &R : model.relationships()) {
//         //     if (auto Error = RelationshipToBdd(manager, R.get(), &varMap, &finalBDD); !Error) {
//         //         return oxidd_bdd_t{nullptr,0};
//         //     }
//         // }

//         auto fb = oxidd_bdd_false(manager);
//         if (fb._i == finalBDD._i) {
//             oxidd_bdd_unref(finalBDD);
//             return oxidd_bdd_t{nullptr,0}; // Return an invalid BDD if the final BDD is false
//         }

//         // Count SAT solutions
//         oxidd_bdd_t oneTerminal = oxidd_bdd_true(manager);
//         oxidd_bdd_t zeroTerminal = oxidd_bdd_false(manager);
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
//             return oxidd_bdd_t{nullptr,0}; // Return an invalid BDD if getPr fails
//         };

        
//         return finalBDD;
//     }

//     BDDFactory::BDDFeat*
//     BDDFactory::findFeatureinBDD(
//          oxidd_bdd_t* node
//     )  {
//         for (auto& [name, f] : this->varMap) {
//             if(f.type == featType::BINARY) {
//                 oxidd_bdd_t* node_ptr = std::get<oxidd_bdd_t*>(f.data);
//                 if(node_ptr && node_ptr->_p == node->_p && node_ptr->_i == node->_i) {
//                     return &f;
//                 }
//             } else if (f.type == featType::NUMERIC) {
//                 auto numericFeats = std::get<std::vector<std::pair<string, oxidd_bdd_t>>*>(f.data);
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