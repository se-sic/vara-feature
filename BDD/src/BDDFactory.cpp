#include <vector>
#include <string>
#include <unordered_map>
#include "BDD/include/BDDFactory.h"

namespace oxidd::capi
{
    std::unique_ptr<oxidd_bdd_manager_t> BDDFactory::modelToBdd( 
        const vara::feature::FeatureModel &model) 
    {
        oxidd_bdd_manager_t manager = oxidd_bdd_manager_new(0, 0, 0);
        std::unordered_map<std::string, oxidd_bdd_t> varMap;

        for (const auto &rltsps : model.relationships()) {
            for (const auto &Child : rltsps->children()) {
                const auto *ChildFeature = (const vara::feature::Feature *)Child;
               varMap.insert({ChildFeature->getName().str(), oxidd_bdd_new_var(manager)});
            }
        }

        // for (const auto &feats : model.features()) {
            
        // }
    }


}