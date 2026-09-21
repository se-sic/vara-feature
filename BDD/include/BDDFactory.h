#ifndef OXIDD_BDD_FACTORY
#define OXIDD_BDD_FACTORY

#include "oxidd/bdd.hpp"
#include "oxidd/util.hpp"
#include "vara/Feature/FeatureModel.h"
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace bdd::sample {

    /// \brief Main BDD builder using a feature model
    ///
    /// Pipeline: All features from the feature model get registered to the manager, checked for XOR (alternative) groups,
    /// then encoded into binary parent-child constraints. Next, all constraints from the feature model encoded into
    /// the BDD and ultimately probabilities are calculated for each node of the resulting BDD.
    ///
    ///  \p FinalBdd gets used throughout each instance, meaning that using modelToBdd multiple times will layer 
    ///  the same BDD on top of itself. To prevent, always create a new instance of a BDDFactory.
    class BDDFactory {
    public:
        /// \brief Converts \p Model into a BDD and returns it
        ///
        /// The result reflects a conjunction of all constraints and relationships in the feature model. Also, for each 
        /// node the probability of being true is calculated and stored in \p SatMap.
        ///
        /// \param Model The feature model to be converted
        ///
        /// \return The converted feature model aka BDD
        oxidd::bdd_function modelToBdd(const vara::feature::FeatureModel &Model);

        /// \brief Returns a mutable reference to the per-node-probability map
        std::map<oxidd::bdd_function, std::pair<double, double>> &getSatMap() {
            return SatMap;
        }

    private:
        /// \brief Registers every feature's name as an variable to \p Manager and stores a mapping between ID and variable in \p Vars 
        void fillManager(const vara::feature::FeatureModel &Model);

        /// \brief Adds constraints for alternative groups in the feature model to \p FinalBdd. Specifically, relation 
        /// parent -> (child1 | child2 | ... | childN) is added.
        ///
        /// Alternative groups are identified by checking if there are >= 2 mandatory children of the same parent feature and
        /// they mutually exclude one another.
        void addAlternativeGroupConstraints(const vara::feature::FeatureModel &Model, oxidd::bdd_function &FinalBdd);

        static constexpr size_t InnerNodeCapacity = 1024L * 1024 * 64;
        static constexpr size_t CacheCapacity = 1024L * 1024 * 16;
        static constexpr uint32_t ThreadCount = 8;
        std::map<oxidd::bdd_function, std::pair<double, double>> SatMap;
        oxidd::bdd_manager Manager = oxidd::bdd_manager(InnerNodeCapacity, CacheCapacity, ThreadCount);
        oxidd::bdd_function FinalBdd = Manager.t();
        std::vector<oxidd::bdd_function> Vars; 
    };
    
} // namespace bdd::sample

#endif // OXIDD_BDD_FACTORY