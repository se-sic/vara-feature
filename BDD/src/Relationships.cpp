#include "oxidd/bdd.hpp"
#include "oxidd/capi.h"
#include "vara/Feature/FeatureModel.h"
#include "vara/Feature/Feature.h"
#include <vector>
#include <string>
#include <unordered_map>
#include "BDD/include/BDDFactory.h"
#include "llvm/Support/ErrorHandling.h"
#include "vara/Utils/Result.h"
#include "vara/Solver/Error.h"
#include "BDD/include/Constraints.h"
#include "vara/Feature/Relationship.h"

using vara::Result;
using vara::solver::SolverErrorCode;
using vara::feature::Relationship;
using vara::feature::Feature;
using oxidd::capi::BDDFactory;
using std::unordered_map;
using std::string;
using std::vector;
using std::pair;

namespace oxidd::capi {
    Result<SolverErrorCode> RelationshipToBdd(
        const oxidd_bdd_manager_t manager,
        Relationship *relationship,
        unordered_map<string, BDDFactory::BDDFeat> *varMap,
        oxidd_bdd_t *finalBDD
    ) {
        const auto* Parent = reinterpret_cast<const Feature*>(relationship->getParent());
        auto ParentEntry = (*varMap)[Parent->getName().str()];
        oxidd_bdd_t ParentVar;
        vector<oxidd_bdd_t> V;

        if(ParentEntry.type != BDDFactory::featType::BINARY) {
            auto* bddPointer = std::get<oxidd::capi::oxidd_bdd_t*>((varMap)->at(Parent->getName().str()).data);
            ParentVar = *bddPointer;
        } else if(ParentEntry.type != BDDFactory::featType::NUMERIC) {
            auto numericFeats = std::get<std::vector<std::pair<string, oxidd_bdd_t>>*>(varMap->at(Parent->getName().str()).data);
            ParentVar = std::find_if(numericFeats->begin(), numericFeats->end(), [&](const std::pair<std::string, oxidd_bdd_t>& pair){
                    return pair.first == Parent->getName().str();}
                    )->second;
        } else {
            return SolverErrorCode::NOT_IMPLEMENTED;
        }

        for (const auto &Child : relationship->children()) {
            const auto *ChildFeature = reinterpret_cast<const Feature *>(Child);
            auto ChildEntry = (*varMap)[ChildFeature->getName().str()];
            oxidd_bdd_t ChildVar;

            if(ChildEntry.type != BDDFactory::featType::BINARY) {
                auto* bddPointer = std::get<oxidd::capi::oxidd_bdd_t*>((varMap)->at(ChildFeature->getName().str()).data);
                ChildVar = *bddPointer;
            } else if(ChildEntry.type != BDDFactory::featType::NUMERIC) {
                auto numericFeats = std::get<std::vector<std::pair<string, oxidd_bdd_t>>*>(varMap->at(ChildFeature->getName().str()).data);
                ChildVar = std::find_if(numericFeats->begin(), numericFeats->end(), [&](const std::pair<std::string, oxidd_bdd_t>& pair){
                        return pair.first == ChildFeature->getName().str();}
                        )->second;
            } else {
                return SolverErrorCode::NOT_IMPLEMENTED;
            }

            V.push_back(ChildVar);
        }

        oxidd_bdd_t orChildren = oxidd_bdd_false(manager);
        for (const auto &ChildVar : V) {
            orChildren = oxidd_bdd_or(orChildren, ChildVar);
        }
        oxidd_bdd_t atMostOne = oxidd_bdd_true(manager);
        for(size_t i = 0; i < V.size(); ++i) {
            for(size_t j = i + 1; j < V.size(); ++j) {
                oxidd_bdd_t notTwo = oxidd_bdd_not(oxidd_bdd_and (V[i], V[j]));
                atMostOne = oxidd_bdd_and(atMostOne, notTwo);
            }
        }

        oxidd_bdd_t combinedConstraint = oxidd_bdd_and(atMostOne, orChildren); 

        switch(relationship->getKind()) {
            case Relationship::RelationshipKind::RK_ALTERNATIVE: {
                oxidd_bdd_t altOption = oxidd_bdd_imp(ParentVar, combinedConstraint);
                *finalBDD = oxidd_bdd_and(*finalBDD, altOption);
                break;
            }
            case Relationship::RelationshipKind::RK_OR:{
                oxidd_bdd_t orOption = oxidd_bdd_imp(ParentVar, orChildren);
                *finalBDD = oxidd_bdd_and(*finalBDD, orOption);
                break;
            }
        }

        return vara::Ok<void>();
    }
}