#include "BDDFeats.h"
#include "oxidd/util.hpp"
#include <oxidd/bdd.hpp>

namespace bdd::sample {

        // Add binary feature to the varMap
    namespace {
        Result<SolverErrorCode> addFeatureToBdd(
            const string *FeatureName,
            std::unordered_map<oxidd::var_no_t, BDDFactory::BDDFeat> *VarMap,
            oxidd::var_no_t Id,
            const oxidd::bdd_manager *Manager
        ){
            // Create BDDFeat entry for this feature
            (*VarMap)[Id] = BDDFactory::BDDFeat{
                .BddNode = Manager->var(Id),
                .IsRoot = false,
                .Name = *FeatureName,
                .Marked = false,
                .SatCount = 0,
                .Probability = {},
            };

            return vara::Ok<void>();
        }

        // Add Binary constraint acccording to Z3 rules: 
        //   Add child -> parent
        //   If not in XOR and not optional, add parent -> child as well
        Result<SolverErrorCode> addBinaryConstraints(
            oxidd::var_no_t ParentId,
            oxidd::var_no_t Id,
            const bool IsInXOR,
            const bool IsOpt,
            std::unordered_map<oxidd::var_no_t, BDDFactory::BDDFeat> *VarMap,
            oxidd::bdd_function &FinalBdd
        ){
            // Check if parent exists
            if(ParentId < 0) {
                return SolverErrorCode::PARENT_NOT_PRESENT;
            }

            // Get BDD nodes for child and parent
            oxidd::bdd_function Child = VarMap->at(Id).BddNode;
            oxidd::bdd_function Parent = VarMap->at(ParentId).BddNode;

            // Add constraint: child → parent
            oxidd::bdd_function ChildToParent = Child.imp(Parent);

            FinalBdd &= ChildToParent;

            // If mandatory (not optional and not in XOR), add parent → child
            if(!IsInXOR && !IsOpt) {
                oxidd::bdd_function ParentToChild = Parent.imp(Child);
                FinalBdd &= ParentToChild;
            }

            return vara::Ok<void>();
        }
    } // namespace
    
    // Convert a feature to BDD representation and add constraints
    Result<SolverErrorCode>featureToBdd(
        const oxidd::bdd_manager* Mgr,
        const bool IsInXOR,
        const Feature& Feature,
        std::unordered_map<oxidd::var_no_t, BDDFactory::BDDFeat>* VarMap,
        oxidd::bdd_function* FinalBdd
    ){
        // Extract feature properties
        bool IsOpt = Feature.isOptional();
        const class Feature *Parent = Feature.getParentFeature();
        const std::string FeatureName = Feature.getName().str();
        const std::string ParentName = Parent ? Parent->getName().str() : ""; // Changed 'parent' to 'Parent'
        auto IdCheck = Mgr->name_to_var(FeatureName);
        if(!IdCheck.has_value()) {
            std::cerr << "Error: Could not find variable ID for feature '" << FeatureName << "'.\n";
            return SolverErrorCode::ILLEGAL_STATE;
        }
        oxidd::capi::oxidd_var_no_t Id = IdCheck.value();
        auto ParentIdCheck = Parent ? Mgr->name_to_var(ParentName) : -1;
        if(Parent && !ParentIdCheck.has_value()) {
            std::cerr << "Error: Could not find variable ID for parent feature '" << ParentName << "'.\n";
            return SolverErrorCode::PARENT_NOT_PRESENT;
        }
        oxidd::capi::oxidd_var_no_t ParentId = Parent ? ParentIdCheck.value() : -1;

        // If ID is already in the varMap, return back to the next feature
        if(VarMap->contains(Id)) {
            return SolverErrorCode::ALREADY_PRESENT;
        }
        // Handle different feature types: Only consider Binary and root features, else return NOT_SUPPORTED
        switch(Feature.getKind()) {
            case Feature::FeatureKind::FK_NUMERIC: {
                std::cerr << "Numeric features are not supported. Please choose a different feature diagram." << '\n';
                return SolverErrorCode::NOT_SUPPORTED;
            }
            case Feature::FeatureKind::FK_BINARY: {
                // Verify it's a binary feature
                std::cout << "now here" << '\n';
                if(!llvm::isa<vara::feature::BinaryFeature>(&Feature)) {
                    std::cerr << "Feature is not a binary feature." << '\n';
                    return SolverErrorCode::NOT_SUPPORTED;
                }
                // Add binary feature to the varMap
                addFeatureToBdd(
                    &FeatureName,
                    VarMap,
                    Id,
                    Mgr
                );
                // Add binary  constraints according to Z3 ruless
                auto R = addBinaryConstraints(
                    ParentId,
                    Id,
                    IsInXOR,
                    IsOpt,
                    VarMap,
                    *FinalBdd);
                if(!R) {
                    return R;
                }

                return vara::Ok<void>();
            }
            // If root feature, add it to varMap and then add it as AND to the finalBdd
            case Feature::FeatureKind::FK_ROOT: {
                // Handle root feature specially

                
                (*VarMap)[Id] = BDDFactory::BDDFeat{
                    .BddNode = Mgr->var(Id),
                    .IsRoot = true,
                    .Name = FeatureName,
                    .Marked = false,
                    .SatCount = 0,
                    .Probability = {},
                };

                *FinalBdd = (*FinalBdd) & Mgr->var(Id);
                
                return vara::Ok<void>();
            }
            default: {
                std::cerr << "Unknown feature kind encountered." << '\n';
                return SolverErrorCode::NOT_SUPPORTED;
            }
        }
    }
} // namespace bdd::sample

// ------------------------------------------------------ OLD CODE ------------------------------------------------------
//     Result<SolverErrorCode>FeatureToBdd(
//         const oxidd::bdd_manager manager,
//         const bool isInXOR,
//         const Feature& feature,
//         unordered_map<string, BDDFactory::BDDFeat> *varMap,
//         unordered_map<string, oxidd::bdd_function> *binaryVarMap,
//         unordered_map<string, vector<pair<string,oxidd::bdd_function>>> *numericVarMap,
//         oxidd::bdd_function *finalBDD) {

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
//                 auto* rootPointer = std::get<oxidd::capi::oxidd::bdd_function*>((varMap)->at(featureName).data);
//                 oxidd::bdd_function root = *rootPointer;
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
//         unordered_map<string, vector<pair<string, oxidd::bdd_function>>> *numericVarMap,
//         oxidd::bdd_manager manager,
//         oxidd::bdd_function *finalBDD) {

//         if (varMap->find(featureName) != varMap->end()) {
//             return SolverErrorCode::ALREADY_PRESENT;
//         }

//         //Add numeric feature to the numericVarMap
//         vector<pair<string,oxidd::bdd_function>> valueVars;
//         oxidd::bdd_function orValues = oxidd_bdd_false(manager);

//         valueVars.push_back({featureName, oxidd_bdd_new_var(manager)});

//         auto test = valueVars.back().first;
//         auto test2 = valueVars.back().second;

//         std::cout << "Current Name" << test << "and their level" << test2._i << std::endl;

//         //Create BDD variables for each value in the numeric feature
//         //Also considers an OR over all values of the feature in form: (val1 v val2 v ... v valN)
//         for(auto &val : *vals) {
//             string valName = featureName + "_" + std::to_string(val);
//             oxidd::bdd_function valVar = oxidd_bdd_new_var(manager);
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
//         auto* numericFeats = std::get<std::vector<std::pair<string, oxidd::bdd_function>>*>(varMap->at(featureName).data);
//         //This is an equivalence over all values of the feature in form: feature <=> (val1 v val2 v ... v valN)
//         oxidd::bdd_function minimalConstraint = oxidd_bdd_equiv(
//             std::find_if(numericFeats->begin(), numericFeats->end(), [&](const std::pair<std::string, oxidd::bdd_function>& pair) {
//                 return pair.first == featureName;}
//                 )->second, orValues);

//         //This is an AND over all values of the feature in form: ¬(valI ∧ valJ) for all i != j
//         oxidd::bdd_function maximalConstraint = oxidd::bdd_functionrue(manager);

//         for (size_t i=0; i < (*vals).size(); ++i) {
//             for(size_t j=i+1; j < (*vals).size(); ++j) {
//                 oxidd::bdd_function notTwo = oxidd_bdd_not(
//                     oxidd_bdd_and(
//                         std::find_if(numericFeats->begin(), numericFeats->end(), [&](const std::pair<std::string, oxidd::bdd_function>& pair) {
//                             return pair.first == featureName + "_" + std::to_string((*vals)[i]);}
//                             )->second,
//                         std::find_if(numericFeats->begin(), numericFeats->end(), [&](const std::pair<std::string, oxidd::bdd_function>& pair) {
//                             return pair.first == featureName + "_" + std::to_string((*vals)[j]);}
//                             )->second));
//                 maximalConstraint = oxidd_bdd_and(maximalConstraint, notTwo);
//             }
//         }

//         oxidd::bdd_function numericConstraint = oxidd_bdd_and(minimalConstraint, maximalConstraint);
//         *finalBDD = oxidd_bdd_and(*finalBDD, numericConstraint);

//         return vara::Ok<void>();
//     };

//     Result<SolverErrorCode> addFeatureToBdd(
//         const string featureName, 
//         unordered_map<std::string, BDDFactory::BDDFeat> *varMap,
//         unordered_map<string, oxidd::bdd_function> *binaryVarMap,
//         oxidd::bdd_manager manager) {

//         if (varMap->find(featureName) != varMap->end()) {
//             return SolverErrorCode::ALREADY_PRESENT;
//         }

//         //Add binary feature to the binaryVarMap
//         oxidd::bdd_function binaryVar = oxidd_bdd_new_var(manager);
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
//         oxidd::bdd_function *finalBDD) {

//         oxidd::bdd_function child;
//         oxidd::bdd_function parent;

//         if ((*varMap)[featureName].type == BDDFactory::featType::NUMERIC) { //unnecessary check, but kept for clarity
//             auto numericFeats = std::get<std::vector<std::pair<string, oxidd::bdd_function>>*>(varMap->at(featureName).data);
//             child = std::find_if(numericFeats->begin(), numericFeats->end(), [&](const std::pair<std::string, oxidd::bdd_function>& pair){
//                     return pair.first == featureName;}
//                     )->second;
//         } else if ((*varMap)[featureName].type == BDDFactory::featType::BINARY) {
//             auto* bddPointer = std::get<oxidd::capi::oxidd::bdd_function*>((varMap)->at(featureName).data);
//             child = *bddPointer;
//         } else {
//             return SolverErrorCode::NOT_SUPPORTED;
//         }

//         if ((*varMap)[parentName].type == BDDFactory::featType::NUMERIC) {
//             auto numericFeats = std::get<std::vector<std::pair<string, oxidd::bdd_function>>*>(varMap->at(parentName).data);
//             parent = std::find_if(numericFeats->begin(), numericFeats->end(), [&](const std::pair<std::string, oxidd::bdd_function>& pair){
//                     return pair.first == parentName;}
//                     )->second;
//         } else if ((*varMap)[parentName].type == BDDFactory::featType::BINARY) {
//             auto* bddPointer = std::get<oxidd::capi::oxidd::bdd_function*>((varMap)->at(parentName).data);
//             parent = *bddPointer;
//         } else {
//             return SolverErrorCode::NOT_SUPPORTED;
//         }

//         oxidd::bdd_function childToParent = oxidd_bdd_imp(child, parent);
//         *finalBDD = oxidd_bdd_and(*finalBDD, childToParent);
        
//         if(!isOptional) {
//             oxidd::bdd_function parentToChild = oxidd_bdd_imp(parent, child);
//             *finalBDD = oxidd_bdd_and(*finalBDD, parentToChild);
//         }
//         return vara::Ok<void>();
//     }
// }