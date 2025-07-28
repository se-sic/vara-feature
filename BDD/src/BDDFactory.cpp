#include <vector>
#include <string>
#include <unordered_map>
#include "vara/Feature/Feature.h"
#include "BDD/include/BDDFactory.h"
#include "BDD/include/BDDFeats.h"
#include "BDD/include/Relationships.h"
#include "BDD/include/Probabilities.h"

namespace oxidd::capi
{

    enum class featType {
        NUMERIC,
        BINARY
    };

    struct nodeInfo {
        bool marked = false;
        size_t satCount = 0; // Number of satisfying assignments
        double probability = 0.0; // Probability of the node
    }

    struct BDDFeat {
    featType type;
    std::variant<oxidd_bdd_t*, std::vector<std::pair<string, oxidd_bdd_t>>*> data;
    nodeInfo info;
    bool isRoot = false;
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

        if (oxidd_bdd_false(manager) == finalBDD) {
            oxidd_bdd_unref(finalBDD);
            return oxidd_bdd_t{nullptr,0}; // Return an invalid BDD if the final BDD is false
        }

        // Count SAT solutions
        oxidd_bdd_t oneTerminal = oxidd_bdd_true(manager);
        oxidd_bdd_t zeroTerminal = oxidd_bdd_false(manager);
        size_t nodeCount = oxidd_bdd_node_count(finalBDD);
        BDDFeat* root = findFeatureinBDD(&finalBDD, &varMap);

        Result<SolverErrorCode>getPr(
            oxidd_bdd_manager_t &manager,
            oxidd_bdd_t &finalBDD,
            BDDFactory::BDDFeat* root,
            size_t nodeCount,
            oxidd_bdd_t *oneTerminal,
            oxidd_bdd_t *zeroTerminal,
            unordered_map<std::string, BDDFactory::BDDFeat> &varMap
        )

        
        return finalBDD;
    }

    BDDFeat* findFeatureinBDD(
        const oxidd_bdd_t* node,
        std::unordered_map<std::string, BDDFeat>* varMap
    ) {
        for (const auto& [name, feat] : varMap) {
            if(feat.type == featType.Binary) {
                oxidd_bdd_t* node_ptr = std::get<oxidd_bdd_t*>(feat.data);
                if(node_ptr && node_ptr->_p == node->_p && node_ptr->_i == node->_i) {
                    return &feat;
                }
            } else if (feat.type == featType.Numeric) {
                auto numericFeats = std::get<std::vector<std::pair<string, oxidd_bdd_t>>*>(feat.data);
                if(numericFeats && !numericFeats->empty()) {
                    const auto& [name, feat] = (*numericFeats)[0];
                    if(feat._p == node->_p && feat._i == node->_i) {
                        return &feat;
                    } else {
                        for (const auto& [name, feat] : *numericFeats) {
                            if (feat._p == node->_p && feat._i == node->_i) {
                                return &feat;
                            }
                        }
                    }
                }
            }
        }
        return nullptr;
    }
}