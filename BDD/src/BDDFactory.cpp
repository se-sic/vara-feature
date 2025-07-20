#include <vector>
#include <string>
#include <unordered_map>
#include "vara/Feature/Feature.h"
#include "BDD/include/BDDFactory.h"
#include "BDD/include/BDDFeats.h"
#include "BDD/include/Relationships.h"

namespace oxidd::capi
{

    enum class featType {
        NUMERIC,
        BINARY
    };

    struct BDDFeat {
        featType type;
        std::variant<oxidd_bdd_t*, std::vector<std::pair<string,oxidd_bdd_t>>*> data;
    };

    oxidd_bdd_t BDDFactory::modelToBdd( 
        const vara::feature::FeatureModel &model) 
    {
        oxidd_bdd_manager_t manager = oxidd_bdd_manager_new(0, 0, 0);
        oxidd_bdd_t finalBDD = oxidd_bdd_true(manager);
        std::unordered_map<std::string, BDDFeat> varMap;
        std::unordered_map<std::string, oxidd_bdd_t> binaryVarMap;
        std::unordered_map<string, std::vector<std::pair<string, oxidd_bdd_t>>> numericVarMap;
        std::vector<string> V;

        for (const auto &rltsps : model.relationships()) {
            for (const auto &Child : rltsps->children()) {
                const auto *ChildFeature = (const vara::feature::Feature *)Child;
                V.insert(V.begin(), ChildFeature->getName().str());
            }
        }

        for (auto* F : model.features()) {
            if (auto R = FeatureToBdd(
                manager, 
                std::find(V.begin(), V.end(), F->getName().str()) != V.end(),
                *F, 
                &varMap, 
                &binaryVarMap, 
                &numericVarMap, 
                &finalBDD
            ); !R) {
                return oxidd_bdd_t{nullptr,0};
            }
        }

        for (auto *C : model.booleanConstraints()) {
            oxidd_bdd_t constraintBDD = addConstraint(C->constraint(), manager, varMap);
            finalBDD = oxidd_bdd_and(finalBDD, constraintBDD);
        }

        for (auto *C : model.nonBooleanConstraints()) {
            oxidd_bdd_t constraintBDD = addConstraint(C->constraint(), manager, varMap);
            finalBDD = oxidd_bdd_and(finalBDD, constraintBDD);
        }

        for (auto *C : model.mixedConstraints()) {
            oxidd_bdd_t constraintBDD = addMixedConstraint(C->constraint(), C->exprKind(), C->req(), manager, varMap);
            finalBDD = oxidd_bdd_and(finalBDD, constraintBDD);
        }

        for (const auto &R : model.relationships()) {
            if (auto Error = RelationshipToBdd(manager, R.get(), &varMap, &finalBDD); !Error) {
                return oxidd_bdd_t{nullptr,0};
            }
        }

        return finalBDD;
    }


}