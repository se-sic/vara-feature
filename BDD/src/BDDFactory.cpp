#include <vector>
#include <string>
#include <unordered_map>
#include "vara/Feature/Feature.h"
#include "BDDFactory.h"
#include "BDDFeats.h"
#include "Relationships.h"
#include "Probabilities.h"
#include "Constraints.h"

namespace oxidd::capi     
{

    oxidd_bdd_t BDDFactory::modelToBdd( 
        const vara::feature::FeatureModel &model) 
    {
        std::cout << model.<< std::endl;
        oxidd_bdd_manager_t manager = oxidd_bdd_manager_new(0, 0, 0);
        std::cout<< manager._p << std::endl;
        oxidd_bdd_t finalBDD = oxidd_bdd_true(manager);
        std::cout<< "finalBDD"<< std::endl;
        std::unordered_map<std::string, oxidd_bdd_t> binaryVarMap;
        std::cout<< "binary"<< std::endl;
        std::unordered_map<string, std::vector<std::pair<string, oxidd_bdd_t>>> numericVarMap;
        std::cout<< "numeric"<< std::endl;
        std::vector<string> V;
        BDDConstraintVisitor visitor = 
            BDDConstraintVisitor(manager, &varMap, &binaryVarMap, &numericVarMap);
        
        std::cout<< "visitor"<< std::endl;


        for (const auto &rltsps : model.relationships()) {
            std::cout<< "ChildFeature->getName().str()" << std::endl;
            for (const auto &Child : rltsps->children()) {
                const auto *ChildFeature = (const vara::feature::Feature *)Child;
                V.insert(V.begin(), ChildFeature->getName().str());
                std::cout<< ChildFeature->getName().str() << std::endl;
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
            oxidd_bdd_t constraintBDD = visitor.addConstraint(C->constraint());
            finalBDD = oxidd_bdd_and(finalBDD, constraintBDD);
        }

        for (auto *C : model.nonBooleanConstraints()) {
            oxidd_bdd_t constraintBDD = visitor.addConstraint(C->constraint());
            finalBDD = oxidd_bdd_and(finalBDD, constraintBDD);
        }

        for (auto *C : model.mixedConstraints()) {
            BDDConstraintVisitor mixedVisitor = 
                BDDConstraintVisitor(manager, &varMap, &binaryVarMap, &numericVarMap, true, true);
            oxidd_bdd_t constraintBDD = mixedVisitor.addConstraint(C->constraint());
            finalBDD = oxidd_bdd_and(finalBDD, constraintBDD);
        }

        for (const auto &R : model.relationships()) {
            if (auto Error = RelationshipToBdd(manager, R.get(), &varMap, &finalBDD); !Error) {
                return oxidd_bdd_t{nullptr,0};
            }
        }

        auto fb = oxidd_bdd_false(manager);
        if (fb._i == finalBDD._i) {
            oxidd_bdd_unref(finalBDD);
            return oxidd_bdd_t{nullptr,0}; // Return an invalid BDD if the final BDD is false
        }

        // Count SAT solutions
        oxidd_bdd_t oneTerminal = oxidd_bdd_true(manager);
        oxidd_bdd_t zeroTerminal = oxidd_bdd_false(manager);
        size_t nodeCount = oxidd_bdd_node_count(finalBDD);
        BDDFeat* root = findFeatureinBDD(&finalBDD);

        auto R = getPr(
            &manager,
            &finalBDD,
            root,
            nodeCount,
            &oneTerminal,
            &zeroTerminal,
            *this);

        if (!R) { 
            oxidd_bdd_unref(finalBDD);
            return oxidd_bdd_t{nullptr,0}; // Return an invalid BDD if getPr fails
        };

        
        return finalBDD;
    }

    BDDFactory::BDDFeat*
    BDDFactory::findFeatureinBDD(
         oxidd_bdd_t* node
    )  {
        for (auto& [name, f] : this->varMap) {
            if(f.type == featType::BINARY) {
                oxidd_bdd_t* node_ptr = std::get<oxidd_bdd_t*>(f.data);
                if(node_ptr && node_ptr->_p == node->_p && node_ptr->_i == node->_i) {
                    return &f;
                }
            } else if (f.type == featType::NUMERIC) {
                auto numericFeats = std::get<std::vector<std::pair<string, oxidd_bdd_t>>*>(f.data);
                if(numericFeats && !numericFeats->empty()) {
                    auto& [name, feat] = (*numericFeats)[0];
                    if(feat._p == node->_p && feat._i == node->_i) {
                        for(auto& [name2, feat2]: this->varMap){
                            if(name == name2){
                                return &feat2;
                            }
                        }
                    } else {
                        for (auto& [name3, feat3] : *numericFeats) {
                            if (feat3._p == node->_p && feat3._i == node->_i) {
                                for(auto& [name4, feat4]: this->varMap){
                                    if(name3 == name4){
                                        return &feat4;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return nullptr;
    }
}