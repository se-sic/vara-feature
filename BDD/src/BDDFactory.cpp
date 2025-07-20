#include <vector>
#include <string>
#include <unordered_map>
#include "BDD/include/BDDFactory.h"

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

    std::unique_ptr<oxidd_bdd_manager_t> BDDFactory::modelToBdd( 
        const vara::feature::FeatureModel &model) 
    {
        oxidd_bdd_manager_t manager = oxidd_bdd_manager_new(0, 0, 0);
        oxidd_bdd_t finalBDD = oxidd_bdd_true(manager);
        std::unordered_map<std::string, BDDFeat> varMap;
        std::unordered_map<std::string, oxidd_bdd_t> binaryVarMap;
        std::unordered_map<std::string, std::vector<std::pair<string, oxidd_bdd_t>>> numericVarMap;
        std::vector<string> V;

        for (const auto &rltsps : model.relationships()) {
            for (const auto &Child : rltsps->children()) {
                const auto *ChildFeature = (const vara::feature::Feature *)Child;
                V.insert(V.begin(), ChildFeature->getName().str());
            }
        }

        // for (const auto &feats : model.features()) {
            
        // }

        
    }


}