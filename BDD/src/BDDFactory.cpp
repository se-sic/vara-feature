#include "BDDFactory.h"
#include "BDDFeats.h"
#include "Constraints.h"
#include "Probabilities.h"
#include "oxidd/bdd.hpp"
#include <algorithm>
#include <cstddef>
#include <iostream>
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

       // Add all features to manager including their names
       fillManager(Model);
       std::cout << "Added all features to BDD manager\n";

       // Store names of features in XOR relationships
       std::vector<std::string> V;

       // Process XOR relationships from the feature model
       if(!Model.relationships().empty()) {
        for(const auto &S: Model.relationships()){
            for(const auto &Child: S->children()) {
                const auto *ChildFeature = (const vara::feature::Feature *)Child;
                V.insert(V.begin(), ChildFeature->getName().str());
            }
        }
       } else {
        std::cout << "Feature model has no XOR relationships\n";
       }
       std::cout << "Processed XOR relationships\n";

       // Process each feature: add binary and root features to BDD and process their constraints
       for(auto *F: Model.features()) { 
        auto R = featureToBdd(
            Manager,
            std::ranges::find(V, F->getName().str()) != V.end(),
            *F,
            FinalBdd);
        if(!R) {
            continue;
        }
       }
       std::cout << "Passed feature processing\n";

       addAlternativeGroupConstraints(Model, FinalBdd);
       std::cout << "Passed alternative group processing\n";

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

       std::cout << "Final BDD has " << FinalBdd.node_count() << " nodes.\n";
       std::string_view DiagramName = "Sora";
       std::vector<oxidd::bdd_function> Funcs = {FinalBdd};
       auto Result = Manager.visualize(DiagramName, Funcs, 4000);
       if(!Result) {
            std::cerr << "Error visualizing BDD: " << Result.error().message() << '\n';
       } else {
            std::cout << "BDD visualization successful.\n";
       }
       Manager.export_dddmp("hippacc.dddmp", Funcs).value();
       return FinalBdd;
    }

    // ----- Auxiliary functions -----
    void BDDFactory::addAlternativeGroupConstraints(
    const vara::feature::FeatureModel &Model,
    oxidd::bdd_function &FinalBdd
    ) {
        // Track which parents we've already processed
        std::set<std::string> ProcessedParents;
        
        for(auto *F: Model.features()) {
            const class vara::feature::Feature *Parent = F->getParentFeature();
            if (!Parent) { 
                continue;
            }
            
            std::string ParentName = Parent->getName().str();
            
            // Skip if already processed this parent
            if (ProcessedParents.contains(ParentName)) { 
                continue;
            }
            
            // Get all children of this parent
            std::vector<const vara::feature::Feature*> Children;
            for (auto* Sibling : Model.features()) {
                if (Sibling->getParentFeature() == Parent) {
                    Children.push_back(Sibling);
                }
            }
            
            // Check if this is a mandatory alternative group
            if (Children.size() < 2) { 
                continue;
            }
            
            bool AllMandatory = true;
            
            for (const auto* Child : Children) {
                if (Child->isOptional()) {
                    AllMandatory = false;
                    break;
                }
            }
            
            if (!AllMandatory) { 
                continue;
            }
            
            // Check if children mutually exclude each other by checking their excludes constraints
            bool HaveMutualExclusion = false;
            for (const auto* Child : Children) {
                for (auto* ExcludeConstraint : Child->excludes()) {
                    // Check if this child excludes other siblings
                    for (const auto* OtherChild : Children) {
                        if (Child == OtherChild) { 
                            continue;
                        }  
                        // Check if the excluded feature is the other child
                        auto* RightOperand = ExcludeConstraint->getRightOperand();
                        if (auto* Pfc = llvm::dyn_cast<vara::feature::PrimaryFeatureConstraint>(RightOperand)) {
                            if (Pfc->getFeature() == OtherChild) {
                                HaveMutualExclusion = true;
                                break;
                            }
                        }
                    }
                    if (HaveMutualExclusion) { 
                        break;
                    }
                }
                if (HaveMutualExclusion) { 
                    break;
                }
            }      
            if (!HaveMutualExclusion) { 
                continue;
            }
            
            // This is a mandatory alternative group!
            // Add constraint: Parent → (child1 | child2 | ... | childN)
            
            auto ParentIdOpt = Manager.name_to_var(ParentName);
            if (!ParentIdOpt.has_value()) { 
                continue;
            }
            
            oxidd::bdd_function ParentVar = Manager.var(ParentIdOpt.value());
            oxidd::bdd_function ChildrenOr = Manager.f();  // Start with false
            
            for (const auto* Child : Children) {
                auto ChildIdOpt = Manager.name_to_var(Child->getName().str());
                if (ChildIdOpt.has_value()) {
                    oxidd::bdd_function ChildVar = Manager.var(ChildIdOpt.value());
                    ChildrenOr = ChildrenOr | ChildVar;
                }
            }
            
            // Add: parent → (child1 | child2 | ...)
            FinalBdd &= ParentVar.imp(ChildrenOr);
            
            ProcessedParents.insert(ParentName);
            
        }
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
        if (Res) {
            auto VarRange = Res.value();
            for (auto VarNo : VarRange) {
                Vars.push_back(Manager.var(VarNo));  // Store the variables
            }
        }
    }
} // namespace bdd::sample