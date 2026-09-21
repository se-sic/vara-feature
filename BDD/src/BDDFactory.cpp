#include "BDDFactory.h"
#include "BDDFeats.h"
#include "Constraints.h"
#include "Probabilities.h"
#include "oxidd/bdd.hpp"

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <set>
#include <span>
#include <string>
#include <vector>

namespace bdd::sample { 
    oxidd::bdd_function BDDFactory::modelToBdd(
        const vara::feature::FeatureModel &Model) {
       if(Model.size()== 0) {
        std::cerr << "Error: Feature model is empty\n";
        return Manager.f();
       }

       fillManager(Model);

       // XOR processing 1: collect al feature names that appear in alternative groups, such that 
       // we can check whether we have to add constraint: parent -> child.
       std::vector<std::string> XorList;
       if(!Model.relationships().empty()) {
        for(const auto &Relation: Model.relationships()){
            for(const auto &Child: Relation->children()) {
                const auto *ChildFeature = llvm::dyn_cast<vara::feature::Feature>(Child);
                if (!ChildFeature) {
                    std::cerr << "Note: Child is not a feature\n";
                    continue;
                }
                XorList.insert(XorList.begin(), ChildFeature->getName().str());
            }
        }
       } else {
        std::cerr << "Note: Feature model has no XOR relationships\n";
       }

       // Encode all binary and root features and the parent-relations constraints
       for(auto *F: Model.features()) { 
        const bool IsInXor = std::ranges::find(XorList, F->getName().str()) != XorList.end();
        auto R = featureToBdd( Manager, IsInXor, *F, FinalBdd);
        if(!R) {
            std::cerr << "Error: featureToBdd failed on feature " << F->getName().str() << '\n';
            return Manager.f();
        }
       }

       // XOR processing 2: adds parent constraint parent -> (child1 | child2 | ... | childN)
       addAlternativeGroupConstraints(Model, FinalBdd);

       // Apply all constraints from the feature model
       BDDConstraintVisitor Visitor(Manager, /*VarMap*/ FinalBdd, false, false);    
       for (const auto &C : Model.booleanConstraints()) {
            if (!processConstraints(Visitor, C, FinalBdd)) { 
                std::cerr << "Error: processCosntraints failed." << '\n';
                return Manager.f();
            }
        }

        auto R = getPr(Manager, FinalBdd, &SatMap);
        if(!R){
            std::cerr << "Error: calculating probabilities failed." << '\n';
            return Manager.f();
        }

       //------ Code for visualizing the BDD on the Oxidd Viz page ---------------------
       //std::string_view DiagramName = "Sora";
       //std::vector<oxidd::bdd_function> Funcs = {FinalBdd};
       /*auto Result = Manager.visualize(DiagramName, Funcs, 4000);
       if(!Result) {
            std::cerr << "Error: visualizing BDD: " << Result.error().message() << '\n';
       } else {
            std::cerr << "Note: BDD visualization successful.\n";
       }*/
       //Manager.export_dddmp("hippacc.dddmp", Funcs);
       //-------------------------------------------------------------------------------

       return FinalBdd;
    }

    void BDDFactory::addAlternativeGroupConstraints(
        const vara::feature::FeatureModel &Model, 
        oxidd::bdd_function &FinalBdd) {

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
            oxidd::bdd_function ChildrenOr = Manager.f();
            
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

    void BDDFactory::fillManager(const vara::feature::FeatureModel &Model) { 

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
                Vars.push_back(Manager.var(VarNo));
            }
        } else {
            std::cerr << "Error: Failed to register variables." << '\n';
            return;
        }
    }
} // namespace bdd::sample