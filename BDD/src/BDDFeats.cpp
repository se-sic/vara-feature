#include "BDDFeats.h"
#include "oxidd/util.hpp"
#include <cstddef>
#include <optional>
#include <oxidd/bdd.hpp>

namespace bdd::sample {

        // Add binary feature to the varMap
    namespace {
        // Add Binary constraint acccording to Z3 rules: 
        // Add child -> parent
        // If not in XOR and not optional, add parent -> child as well
        Result<SolverErrorCode> addBinaryConstraints(
            oxidd::var_no_t ParentId,
            oxidd::var_no_t Id,
            const bool IsInXOR,
            const bool IsOpt,
            //std::map<oxidd::var_no_t, BDDFactory::BDDFeat> &VarMap,
            oxidd::bdd_function &FinalBdd,
            const oxidd::bdd_manager &Manager
        ){
            // Check if parent exists
            if(ParentId < 0) {
                return SolverErrorCode::PARENT_NOT_PRESENT;
            }

            // Get BDD nodes for child and parent
            oxidd::bdd_function Child = Manager.var(Id);
            oxidd::bdd_function Parent = Manager.var(ParentId);

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

    // Convert a feature to BDD representation and add child-parent relationships
    Result<SolverErrorCode>featureToBdd(
        oxidd::bdd_manager &Mgr,
        const bool IsInXOR,
        const Feature &Feature,
        oxidd::bdd_function &FinalBdd
    ){
        // Extract feature properties
        bool IsOpt = Feature.isOptional();
        const class Feature *Parent = Feature.getParentFeature();
        const std::string FeatureName = Feature.getName().str();
        const std::string ParentName = Parent ? Parent->getName().str() : "";

        auto IdCheck = Mgr.name_to_var(FeatureName);
        if(!IdCheck.has_value()) {
            std::cerr << "Error: Could not find variable ID for feature '" << FeatureName << "'.\n";
            return SolverErrorCode::ILLEGAL_STATE;
        }
        oxidd::capi::oxidd_var_no_t Id = IdCheck.value();

        auto ParentIdCheck = Parent ? Mgr.name_to_var(ParentName) : -1;
        if(Parent && !ParentIdCheck.has_value()) {
            std::cerr << "Error: Could not find variable ID for parent feature '" << ParentName << "'.\n";
            return SolverErrorCode::PARENT_NOT_PRESENT;
        }
        oxidd::capi::oxidd_var_no_t ParentId = Parent ? ParentIdCheck.value() : -1;

        // Handle different feature types: Only consider Binary and Root features, else return NOT_SUPPORTED
        switch(Feature.getKind()) {
            case Feature::FeatureKind::FK_NUMERIC: {
                std::cerr << "Numeric features are not supported. Please choose a different feature diagram." << '\n';
                return SolverErrorCode::NOT_SUPPORTED;
            }
            case Feature::FeatureKind::FK_BINARY: {
                // Verify it's a binary feature
                if(!llvm::isa<vara::feature::BinaryFeature>(&Feature)) {
                    std::cerr << "Feature is not a binary feature." << '\n';
                    return SolverErrorCode::NOT_SUPPORTED;
                }

                // Add binary  constraints according to Z3 ruless
                auto R = addBinaryConstraints(
                    ParentId,
                    Id,
                    IsInXOR,
                    IsOpt,
                    FinalBdd,
                    Mgr);
                if(!R) {
                    return R;
                }
                return vara::Ok<void>();
            }
            // If root feature, add it to varMap and then add it as AND to the finalBdd
            case Feature::FeatureKind::FK_ROOT: {
                FinalBdd = (FinalBdd) & Mgr.var(Id);
                
                return vara::Ok<void>();
            }

            default: {
                std::cerr << "Unknown feature kind encountered." << '\n';
                return SolverErrorCode::NOT_SUPPORTED;
            }
        }
    }
} // namespace bdd::sample