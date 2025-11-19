#ifndef OXIDD_BDD_FACTORY
#define OXIDD_BDD_FACTORY

#include "oxidd/bdd.hpp"
#include "oxidd/util.hpp"
#include "vara/Feature/FeatureModel.h"
#include <algorithm>
#include <optional>
#include <string>
#include <unordered_map>

namespace bdd::sample
{
    class BDDFactory
    {
        public:

            oxidd::var_no_t MAXLEVEL = 0;
            struct BDDFeat {
                oxidd::bdd_function BddNode;                   // BDD node representing the feature
                bool IsRoot = false;                           // Is it the root feature?
                std::string Name;                              // Feature name
                bool Marked = false;                           // Marking for probability calculation
                size_t SatCount = 0;                           // Number of satisfying assignments
                std::optional<double> Probability;             // Probability of the node
                std::optional<double> CC;
            };

            BDDFeat* findFeatureinBDD(
                oxidd::bdd_function* Node
            );

            oxidd::bdd_function modelToBdd(const vara::feature::FeatureModel &Model);
            void fillManager(const vara::feature::FeatureModel &Model);

            std::unordered_map<oxidd::level_no_t, BDDFeat> VarMap;
            oxidd::bdd_manager Manager = oxidd::bdd_manager(1024L * 1024, 1024L * 1024, 8);
            oxidd::bdd_function FinalBdd = Manager.t();

            void setMaxLevel(oxidd::var_no_t Level) {
                MAXLEVEL = std::max(Level, MAXLEVEL);
            }

            [[nodiscard]] oxidd::var_no_t getMaxLevel() const {
                return MAXLEVEL;
            };
    };
    
} // namespace bdd::sample

#endif // OXIDD_BDD_FACTORY