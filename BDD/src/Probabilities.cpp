#include "Probabilities.h"
#include "vara/Solver/Error.h"

#include <iostream>
#include <oxidd/bdd.hpp>
#include <oxidd/bridge.hpp>
#include <oxidd/util.hpp>

using vara::Result;
using vara::solver::SolverErrorCode;

namespace bdd::sample {
    Result<vara::solver::SolverErrorCode>getPr(
        const oxidd::bdd_manager &Manager,
        oxidd::bdd_function &Node,
        std::map<oxidd::bdd_function, std::pair<double, double>> *SatMap) {

        // Node has already been memoized, thus its subtree has been as well
        if (SatMap->contains(Node)) {
            return vara::Ok<void>();
        }

        // Terminal node
        if (Node == Manager.t() || Node == Manager.f()) {
            return vara::Ok<void>();
        }

        auto LevOpt = Node.node_level();
        if(!LevOpt.has_value()) {
            std::cerr << "Error: getPr found a node with no level." << '\n';
            return SolverErrorCode::ILLEGAL_STATE;
        }
        auto Lev = LevOpt.value();

        auto FB = Node.cofactor_false();
        auto TB = Node.cofactor_true();

        // Bryant'S algorithm: recursion into child nodes/cofactors of the current node
        auto RF = getPr(Manager, FB, SatMap);
        if (!RF) {
            return RF;
        }
        auto RT = getPr(Manager, TB, SatMap);
        if (!RT) {
            return RT;
        }

        auto Remaining = Manager.num_vars() - Lev;
        auto SatTrue = TB.sat_count_double(Remaining - 1);
        auto SatCount = Node.sat_count_double(Remaining);;

        if (SatCount == 0) {
            std::cerr << "Error: getPr found a node with no satisfying assignments." << '\n';
            return SolverErrorCode::ILLEGAL_STATE;
        }

        auto Prob = SatTrue / SatCount;

        std::pair <double, double> PrPair = std::make_pair(SatCount, Prob);

        (*SatMap)[Node] = PrPair;

        return vara::Ok<void>();
    }
} // namespace bdd::sample