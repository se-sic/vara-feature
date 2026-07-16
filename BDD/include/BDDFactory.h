#ifndef OXIDD_BDD_FACTORY
#define OXIDD_BDD_FACTORY

#include "oxidd/bdd.hpp"
#include "oxidd/util.hpp"
#include "vara/Feature/FeatureModel.h"
#include <map>
#include <optional>
#include <string>
namespace bdd::sample
{
    class BDDFactory
    {
        public:
            oxidd::bdd_function modelToBdd(const vara::feature::FeatureModel &Model);
            void fillManager(const vara::feature::FeatureModel &Model);

            void addAlternativeGroupConstraints(const vara::feature::FeatureModel &Model,oxidd::bdd_function &FinalBdd);

            std::map<oxidd::bdd_function, std::pair<double, double>> SatMap;
            oxidd::bdd_manager Manager = oxidd::bdd_manager(1024L * 1024 * 64, 1024L * 1024 * 16, 8);
            oxidd::bdd_function FinalBdd = Manager.t();
            std::vector<oxidd::bdd_function> Vars; 
    };
    
} // namespace bdd::sample

#endif // OXIDD_BDD_FACTORY