#ifndef OXIDD_BDD_FACTORY
#define OXIDD_BDD_FACTORY

#include "oxidd/bdd.hpp"
#include "oxidd/capi.h"
#include "vara/Feature/FeatureModel.h"
#include "vara/Feature/Feature.h"

namespace oxidd::capi
{
    class BDDFactory
    {
    public:
        static std::unique_ptr<oxidd_bdd_t> 
        modelToBdd(const vara::feature::FeatureModel &model);

        enum class featType {
        NUMERIC,
        BINARY
        };

        struct BDDFeat {
        featType type;
        std::variant<oxidd_bdd_t*, std::vector<std::pair<string, oxidd_bdd_t>>*> data;
        };
    };
    
} // namespace oxidd::capi

#endif // OXIDD_BDD_FACTORY