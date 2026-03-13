#include "BDDFactory.h"
#include "Probabilities.h"
#include "vara/Solver/Error.h"
#include "vara/Utils/Result.h"
#include <cstddef>
#include <oxidd/bdd.hpp>
#include <oxidd/bridge.hpp>
#include <oxidd/util.hpp>
#include <utility>

using vara::Result;
using bdd::sample::BDDFactory;

namespace bdd::sample {
    // Using Bryant's algorithm to calculate the probabilities used for uniform, ranodm samplin
    Result<vara::solver::SolverErrorCode>getPr(
        const oxidd::bdd_manager &Manager,
        oxidd::bdd_function &Node,
        std::map<oxidd::bdd_function, std::pair<double, double>> *SatMap,
        BDDFactory &Factory
    ) {

        if (SatMap->contains(Node)) {
            return vara::Ok<void>();
        }

        if (Node == Manager.t() || Node == Manager.f()) {
            return vara::Ok<void>();
        }

        auto LevOpt = Node.node_level();
        if(!LevOpt.has_value()) {
            return vara::Ok<void>();
        }
        auto Lev = LevOpt.value();

        auto FB = Node.cofactor_false();
        auto TB = Node.cofactor_true();

        getPr(Manager, FB, SatMap, Factory);
        getPr(Manager, TB, SatMap, Factory);

        auto Remaining = Manager.num_vars() - Lev;
        auto SatTrue = TB.sat_count_double(Remaining - 1);
        auto SatCount = Node.sat_count_double(Remaining);;

        auto Prob = SatTrue / SatCount;

        std::pair <double, double> PrPair = std::make_pair(SatCount, Prob);

        (*SatMap)[Node] = PrPair;

        return vara::Ok<void>();
    }
} // namespace bdd::sample