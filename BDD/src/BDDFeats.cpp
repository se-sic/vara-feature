#include "BDDFeats.h"
#include "oxidd/util.hpp"
#include <iostream>
#include <optional>
#include <oxidd/bdd.hpp>

using vara::Result;
using vara::solver::SolverErrorCode;
using vara::feature::Feature;

namespace bdd::sample {
    
    namespace {
        /// \brief Encodes a binary feature's parent-child-relationship into \p FinalBdd.
        ///
        /// Mandatory features are forced as a binary constraint if there is no parent feature registered to the manager.
        /// Otherwise, the child alwas implies the parent, while the parent implies the child only if the feature is mandatory
        /// and not in an XOR (alternative) group.
        Result<SolverErrorCode> addBinaryConstraints(
            std::optional<oxidd::var_no_t> ParentId,
            oxidd::var_no_t Id,
            const bool IsInXor,
            const bool IsOpt,
            oxidd::bdd_function &FinalBdd,
            const oxidd::bdd_manager &Manager
        ){
            oxidd::bdd_function Child = Manager.var(Id);
            
            // If no parent (top-level feature), we force the feature if mandatory, else we are done
            if(!ParentId.has_value()) {
                if (!IsOpt) {
                    std::cerr << "Note: Forcing mandatory top-level feature: " 
                              << Manager.var_name(Id) << "\n";
                    FinalBdd &= Child;
                }
                return vara::Ok<void>();
            }

            oxidd::bdd_function Parent = Manager.var(ParentId.value());
            oxidd::bdd_function ChildToParent = Child.imp(Parent);
            FinalBdd &= ChildToParent;

            // parent -> child only when the child is mandatory and not in an XOR (alternative) group
            if (!IsInXor && !IsOpt) {
                oxidd::bdd_function ParentToChild = Parent.imp(Child);
                FinalBdd &= ParentToChild;
            }

            return vara::Ok<void>();
        }
    } // namespace

    Result<SolverErrorCode>featureToBdd(
        oxidd::bdd_manager &Mgr,
        const bool IsInXor,
        const Feature &Feat,
        oxidd::bdd_function &FinalBdd
    ){
        bool IsOpt = Feat.isOptional();
        const Feature *Parent = Feat.getParentFeature();
        const std::string FeatureName = Feat.getName().str();

        auto IdCheck = Mgr.name_to_var(FeatureName);
        if(!IdCheck.has_value()) {
            std::cerr << "Error: Could not find variable ID for feature '" 
                      << FeatureName << "'.\n";
            return SolverErrorCode::ILLEGAL_STATE;
        }
        oxidd::var_no_t Id = IdCheck.value();

        std::optional<oxidd::var_no_t> ParentId;
        if (Parent) {
            const std::string ParentName = Parent->getName().str();
            ParentId = Mgr.name_to_var(ParentName);
            if(!ParentId.has_value()) {
                std::cerr << "Error: Could not find variable ID for parent feature '" 
                          << ParentName << "'.\n";
                return SolverErrorCode::PARENT_NOT_PRESENT;
            }
        }

        switch(Feat.getKind()) {
            case Feature::FeatureKind::FK_NUMERIC: {
                std::cerr << "Error: Numeric features are not supported. "
                             "Please choose a different feature diagram.\n";
                return SolverErrorCode::NOT_SUPPORTED;
            }
            case Feature::FeatureKind::FK_BINARY: {
                if(!llvm::isa<vara::feature::BinaryFeature>(&Feat)) {
                    std::cerr << "Error: Feature is not a binary feature.\n";
                    return SolverErrorCode::NOT_SUPPORTED;
                }
                return addBinaryConstraints(ParentId, Id, IsInXor, IsOpt, FinalBdd, Mgr);
            }
            case Feature::FeatureKind::FK_ROOT: {
                FinalBdd &= Mgr.var(Id);        
                return vara::Ok<void>();
            }

            default: {
                std::cerr << "Error: Unknown feature kind encountered.\n";
                return SolverErrorCode::NOT_SUPPORTED;
            }
        }
    }
} // namespace bdd::sample