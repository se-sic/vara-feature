#include "BDDFeats.h"

namespace oxidd::capi {
    
    // Convert a feature to BDD representation and add constraints
    Result<SolverErrorCode>FeatureToBdd(
        const oxidd_bdd_manager_t* mgr,
        const bool isInXOR,
        const Feature& feature,
        std::unordered_map<oxidd_var_no_t, BDDFactory::BDDFeat>* varMap,
        oxidd_bdd_t* finalBdd
    ){
        // Extract feature properties
        bool isOpt = feature.isOptional();
        Feature* parent = feature.getParentFeature();
        const std::string featureName = feature.getName();
        const std::string parentName = parent ? parent->getName().str() : "";
        oxidd_var_no_t id = oxidd_bdd_manager_name_to_var(*mgr, featureName.c_str());
        oxidd_var_no_t parentId = parent ? oxidd_bdd_manager_name_to_var(*mgr, parentName.c_str()) : -1;

        ;
        // If ID is already in the varMap, return back to the next feature
        if(varMap->find(id) != varMap->end()) {
            return SolverErrorCode::ALREADY_PRESENT;
        }
        // Handle different feature types: Only consider Binary and root features, else return NOT_SUPPORTED
        switch(feature.getKind()) {
            case Feature::FeatureKind::FK_NUMERIC: {
                std::cerr << "Numeric features are not supported. Please choose a different feature diagram." << std::endl;
                return SolverErrorCode::NOT_SUPPORTED;
            }
            case Feature::FeatureKind::FK_BINARY: {
                // Verify it's a binary feature
                std::cout << "now here" << std::endl;
                if(!llvm::isa<vara::feature::BinaryFeature>(&feature)) {
                    std::cerr << "Feature is not a binary feature." << std::endl;
                    return SolverErrorCode::NOT_SUPPORTED;
                }
                // Add binary feature to the varMap
                addFeatureToBdd(
                    featureName,
                    varMap,
                    id,
                    *mgr
                );
                // Add binary constraints according to Z3 ruless
                auto R = addBinaryConstraints(
                    parentId,
                    id,
                    isInXOR,
                    isOpt,
                    varMap,
                    finalBdd);
                if(!R) {
                    return R;
                }
                break;
            }
            // If root feature, add it to varMap and then add it as AND to the finalBdd
            case Feature::FeatureKind::FK_ROOT: {
                // Handle root feature specially

                
                (*varMap)[id] = BDDFactory::BDDFeat{
                    .bddNode = oxidd_bdd_var(*mgr, id),
                    .isRoot = true,
                    .name = featureName,
                    .marked = false,
                    .satCount = 0,
                    .probability = {},
                };

                *finalBdd = oxidd_bdd_and(*finalBdd, varMap->at(id).bddNode);
                
                break;
            }
            default: {
                std::cerr << "Unknown feature kind encountered." << std::endl;
                return SolverErrorCode::NOT_SUPPORTED;
            }
        }
    }
    // Add binary feature to the varMap
    Result<SolverErrorCode> addFeatureToBdd(
        const string featureName,
        std::unordered_map<oxidd_var_no_t, BDDFactory::BDDFeat>* varMap,
        oxidd_var_no_t id,
        oxidd_bdd_manager_t manager
    ){
        // Create BDDFeat entry for this feature
        (*varMap)[id] = BDDFactory::BDDFeat{
            .bddNode = oxidd_bdd_var(manager, id),
            .isRoot = false,
            .name = featureName,
            .marked = false,
            .satCount = 0,
            .probability = {},
        };

        return vara::Ok<void>();
    }

    // Add Binary constraint acccording to Z3 rules: 
    //   Add child -> parent
    //   If not in XOR and not optional, add parent -> child as well
    Result<SolverErrorCode> addBinaryConstraints(
        oxidd_var_no_t parentId,
        oxidd_var_no_t id,
        const bool isInXOR,
        const bool isOpt,
        std::unordered_map<oxidd_var_no_t, BDDFactory::BDDFeat>* varMap,
        oxidd_bdd_t* finalBdd
    ){
        // Check if parent exists
        if(parentId < 0) {
            return SolverErrorCode::PARENT_NOT_PRESENT;
        }

        // Get BDD nodes for child and parent
        oxidd_bdd_t child = varMap->at(id).bddNode;
        oxidd_bdd_t parent = varMap->at(parentId).bddNode;

        // Add constraint: child → parent
        oxidd_bdd_t childToParent = oxidd_bdd_imp(child, parent);

        *finalBdd = oxidd_bdd_and(*finalBdd, childToParent);

        // If mandatory (not optional and not in XOR), add parent → child
        if(!isInXOR && !isOpt) {
            oxidd_bdd_t parentToChild = oxidd_bdd_imp(parent, child);
            *finalBdd = oxidd_bdd_and(*finalBdd, parentToChild);
        }

        return vara::Ok<void>();
    }
}

// ------------------------------------------------------ OLD CODE ------------------------------------------------------
//     Result<SolverErrorCode>FeatureToBdd(
//         const oxidd_bdd_manager_t manager,
//         const bool isInXOR,
//         const Feature& feature,
//         unordered_map<string, BDDFactory::BDDFeat> *varMap,
//         unordered_map<string, oxidd_bdd_t> *binaryVarMap,
//         unordered_map<string, vector<pair<string,oxidd_bdd_t>>> *numericVarMap,
//         oxidd_bdd_t *finalBDD) {

//         Feature *Parent = feature.getParentFeature();
//         bool isOptional = feature.isOptional();
//         string parentName = Parent ? Parent->getName().str() : "";
//         string featureName = feature.getName().str();
    
//         if(Parent != nullptr && varMap->find(Parent->getName().str()) == varMap->end()) {
//             return SolverErrorCode::PARENT_NOT_PRESENT;
//         }

//         if (varMap->find(featureName) != varMap->end()) {
//             return SolverErrorCode::ALREADY_PRESENT;
//         }

//         switch(feature.getKind()) {
//             case Feature::FeatureKind::FK_NUMERIC: {
//                 const auto *F = llvm::dyn_cast<vara::feature::NumericFeature>(&feature);
//                 if (!F) {
//                 return SolverErrorCode::NOT_SUPPORTED;
//                 }

//                 const auto vals = F->getValues();
//                 if (std::holds_alternative<vara::feature::NumericFeature::ValueListType>(vals)) {
//                     auto& list = std::get<vara::feature::NumericFeature::ValueListType>(vals);
//                     if(auto R = addFeatureToBdd(
//                             F->getName().str(),
//                             &list,
//                             varMap,
//                             numericVarMap,
//                             manager,
//                             finalBDD);
//                         !R) {
//                         return R;
//                     }
//                 } else {
//                     auto Range = std::get<vara::feature::NumericFeature::ValueRangeType>(vals);
//                     auto *StepFunction = F->getStepFunction();
//                     auto Step = Range.first;
//                     vara::feature::NumericFeature::ValueListType Values;
//                     while (Step <= Range.second) {
//                         Values.insert(Values.begin(), Step);
//                         Step = StepFunction->next(Step);
//                     }
//                     if(auto R = addFeatureToBdd(
//                             F->getName().str(),
//                             &Values,
//                             varMap,
//                             numericVarMap,
//                             manager,
//                             finalBDD);
//                         !R) {
//                         return R;
//                     }
//                 }
//                 break;
//             }
//             case Feature::FeatureKind::FK_BINARY: {
//                 if (!llvm::isa<vara::feature::BinaryFeature>(&feature)) {
//                     return SolverErrorCode::NOT_SUPPORTED;
//                 }
//                 addFeatureToBdd(
//                     featureName,
//                     varMap,
//                     binaryVarMap,
//                     manager);
//                 if(auto R = addBinaryConstraints(
//                         parentName,
//                         featureName,
//                         isInXOR,
//                         isOptional,
//                         varMap,
//                         finalBDD);
//                     !R) {
//                     return R;
//                 }
//                 break;
//             }
//             case Feature::FeatureKind::FK_ROOT: {
//                 addFeatureToBdd(featureName, varMap, binaryVarMap, manager);
//                 assert(varMap->find(featureName) != varMap->end() &&
//                     "Root feature should be present in the varMap");
//                 auto* rootPointer = std::get<oxidd::capi::oxidd_bdd_t*>((varMap)->at(featureName).data);
//                 oxidd_bdd_t root = *rootPointer;
//                 *finalBDD = oxidd_bdd_and(*finalBDD, root);
//                 break;
//             }
//             case Feature::FeatureKind::FK_UNKNOWN:
//                 return SolverErrorCode::NOT_SUPPORTED;
//         }
//         return SolverErrorCode::UNSAT;
//     };

//     Result<SolverErrorCode> addFeatureToBdd(
//         const string featureName,
//         const vara::feature::NumericFeature::ValueListType *vals,
//         unordered_map<std::string, BDDFactory::BDDFeat> *varMap,
//         unordered_map<string, vector<pair<string, oxidd_bdd_t>>> *numericVarMap,
//         oxidd_bdd_manager_t manager,
//         oxidd_bdd_t *finalBDD) {

//         if (varMap->find(featureName) != varMap->end()) {
//             return SolverErrorCode::ALREADY_PRESENT;
//         }

//         //Add numeric feature to the numericVarMap
//         vector<pair<string,oxidd_bdd_t>> valueVars;
//         oxidd_bdd_t orValues = oxidd_bdd_false(manager);

//         valueVars.push_back({featureName, oxidd_bdd_new_var(manager)});

//         auto test = valueVars.back().first;
//         auto test2 = valueVars.back().second;

//         std::cout << "Current Name" << test << "and their level" << test2._i << std::endl;

//         //Create BDD variables for each value in the numeric feature
//         //Also considers an OR over all values of the feature in form: (val1 v val2 v ... v valN)
//         for(auto &val : *vals) {
//             string valName = featureName + "_" + std::to_string(val);
//             oxidd_bdd_t valVar = oxidd_bdd_new_var(manager);
//             valueVars.push_back({valName, valVar});

//             auto test = valueVars.back().first;
//             auto test2 = valueVars.back().second;

//             std::cout << "Current Name " << test << " and their level " << valVar._i<< std::endl;


//             orValues = oxidd_bdd_or(orValues, valVar);
//         };
//         if (numericVarMap->find(featureName) == numericVarMap->end()) {
//             (*numericVarMap)[featureName]=valueVars;
//         }

//         //Add numeric feature to the varMap
//         (*varMap)[featureName] = BDDFactory::BDDFeat{
//             .type = BDDFactory::featType::NUMERIC,
//             .data = &(*numericVarMap)[featureName]
//         };

//         //Add numeric values as constraints to the finalBDD
//         auto* numericFeats = std::get<std::vector<std::pair<string, oxidd_bdd_t>>*>(varMap->at(featureName).data);
//         //This is an equivalence over all values of the feature in form: feature <=> (val1 v val2 v ... v valN)
//         oxidd_bdd_t minimalConstraint = oxidd_bdd_equiv(
//             std::find_if(numericFeats->begin(), numericFeats->end(), [&](const std::pair<std::string, oxidd_bdd_t>& pair) {
//                 return pair.first == featureName;}
//                 )->second, orValues);

//         //This is an AND over all values of the feature in form: ¬(valI ∧ valJ) for all i != j
//         oxidd_bdd_t maximalConstraint = oxidd_bdd_true(manager);

//         for (size_t i=0; i < (*vals).size(); ++i) {
//             for(size_t j=i+1; j < (*vals).size(); ++j) {
//                 oxidd_bdd_t notTwo = oxidd_bdd_not(
//                     oxidd_bdd_and(
//                         std::find_if(numericFeats->begin(), numericFeats->end(), [&](const std::pair<std::string, oxidd_bdd_t>& pair) {
//                             return pair.first == featureName + "_" + std::to_string((*vals)[i]);}
//                             )->second,
//                         std::find_if(numericFeats->begin(), numericFeats->end(), [&](const std::pair<std::string, oxidd_bdd_t>& pair) {
//                             return pair.first == featureName + "_" + std::to_string((*vals)[j]);}
//                             )->second));
//                 maximalConstraint = oxidd_bdd_and(maximalConstraint, notTwo);
//             }
//         }

//         oxidd_bdd_t numericConstraint = oxidd_bdd_and(minimalConstraint, maximalConstraint);
//         *finalBDD = oxidd_bdd_and(*finalBDD, numericConstraint);

//         return vara::Ok<void>();
//     };

//     Result<SolverErrorCode> addFeatureToBdd(
//         const string featureName, 
//         unordered_map<std::string, BDDFactory::BDDFeat> *varMap,
//         unordered_map<string, oxidd_bdd_t> *binaryVarMap,
//         oxidd_bdd_manager_t manager) {

//         if (varMap->find(featureName) != varMap->end()) {
//             return SolverErrorCode::ALREADY_PRESENT;
//         }

//         //Add binary feature to the binaryVarMap
//         oxidd_bdd_t binaryVar = oxidd_bdd_new_var(manager);
//         if(binaryVarMap->find(featureName) == binaryVarMap->end()) {
//             (*binaryVarMap)[featureName] = binaryVar;
//         }

//         auto test = (*binaryVarMap)[featureName];

//         std::cout << "Current Name: " << featureName 
//                   << " and their level: " << binaryVar._i << std::endl;

//         //Add binary feature to the varMap
//         (*varMap)[featureName] = BDDFactory::BDDFeat{
//             .type = BDDFactory::featType::BINARY,
//             .data = &(*binaryVarMap)[featureName]
//         };

//         return vara::Ok<void>();
//     };

//     Result<SolverErrorCode> addBinaryConstraints(
//         const string& parentName,
//         const string& featureName, 
//         const bool isInXOR,
//         const bool isOptional,
//         unordered_map<std::string, BDDFactory::BDDFeat> *varMap,
//         oxidd_bdd_t *finalBDD) {

//         oxidd_bdd_t child;
//         oxidd_bdd_t parent;

//         if ((*varMap)[featureName].type == BDDFactory::featType::NUMERIC) { //unnecessary check, but kept for clarity
//             auto numericFeats = std::get<std::vector<std::pair<string, oxidd_bdd_t>>*>(varMap->at(featureName).data);
//             child = std::find_if(numericFeats->begin(), numericFeats->end(), [&](const std::pair<std::string, oxidd_bdd_t>& pair){
//                     return pair.first == featureName;}
//                     )->second;
//         } else if ((*varMap)[featureName].type == BDDFactory::featType::BINARY) {
//             auto* bddPointer = std::get<oxidd::capi::oxidd_bdd_t*>((varMap)->at(featureName).data);
//             child = *bddPointer;
//         } else {
//             return SolverErrorCode::NOT_SUPPORTED;
//         }

//         if ((*varMap)[parentName].type == BDDFactory::featType::NUMERIC) {
//             auto numericFeats = std::get<std::vector<std::pair<string, oxidd_bdd_t>>*>(varMap->at(parentName).data);
//             parent = std::find_if(numericFeats->begin(), numericFeats->end(), [&](const std::pair<std::string, oxidd_bdd_t>& pair){
//                     return pair.first == parentName;}
//                     )->second;
//         } else if ((*varMap)[parentName].type == BDDFactory::featType::BINARY) {
//             auto* bddPointer = std::get<oxidd::capi::oxidd_bdd_t*>((varMap)->at(parentName).data);
//             parent = *bddPointer;
//         } else {
//             return SolverErrorCode::NOT_SUPPORTED;
//         }

//         oxidd_bdd_t childToParent = oxidd_bdd_imp(child, parent);
//         *finalBDD = oxidd_bdd_and(*finalBDD, childToParent);
        
//         if(!isOptional) {
//             oxidd_bdd_t parentToChild = oxidd_bdd_imp(parent, child);
//             *finalBDD = oxidd_bdd_and(*finalBDD, parentToChild);
//         }
//         return vara::Ok<void>();
//     }
// }