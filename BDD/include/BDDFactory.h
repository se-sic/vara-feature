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
        static std::unique_ptr<oxidd_bdd_manager_t> 
        modelToBdd(const vara::feature::FeatureModel &model);
    };
    
} // namespace oxidd::capi

#endif // OXIDD_BDD_FACTORY