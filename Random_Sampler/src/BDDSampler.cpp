#include "BDDSampler.h"
#include "BDDFactory.h"
#include "oxidd/bdd.hpp"
#include "oxidd/util.hpp"

#include <cstdint>
#include <iostream>
#include <map>
#include <random>
#include <utility>
#include <vector>

namespace bdd::sample {
  
  //NOLINTBEGIN Variable 'Rng'/'Dist' declared 'static', move to anonymous namespace instead
  static std::mt19937 Rng;
  static std::uniform_real_distribution<double> Dist(0.0, 1.0);
  //NOLINTEND

  double random() {  
    return Dist(Rng); 
  }

  void SeedRng(std::uint32_t Seed) { 
    Rng.seed(Seed); 
  }

  std::vector<bool> generateConfiguration(const oxidd::bdd_manager &Manager, 
    const oxidd::bdd_function& Root, 
    std::map<oxidd::bdd_function, std::pair<double, double>> *SatMap) {

      auto RootOpt = Root.node_level();
      if (!RootOpt.has_value()) {
        std::cerr << "Error: Sampling called on a terminal node." << "\n";
        return {};
      }

      oxidd::level_no_t TotalLevels = Manager.num_vars();

      // Initialize each variable with an indipendent 50% draw.
      // This affects variables, that have been reduces away by the BDD such that they
      // carry a value, whereas variables appearing on the traversal path get overwritten
      // below.
      std::vector<bool> Sample;
      Sample.reserve(TotalLevels);
      for (oxidd::var_no_t I = 0; I < TotalLevels; ++I) {
          Sample.push_back(random() < 0.5);
      }

      // BDD traversal: at each internal node, branch on the probability given by the SatMap
      oxidd::bdd_function Trav = Root;
      while (Trav.node_var().has_value()) {
        auto VarIdOpt = Trav.node_var();
        oxidd::var_no_t CurrentVar = VarIdOpt.value();

        auto ProbCheck = SatMap->find(Trav);
        if (ProbCheck == SatMap->end()) {
          std::cerr << "Error: BDD node not found in SatMap." << "\n";
          return {};
        }
        const double Probability = SatMap->at(Trav).second;

        auto CheckRan = random();
        if (CheckRan < Probability) {
          Trav = Trav.cofactor_true();
          Sample[CurrentVar] = true;    // Include feature
        } else {
          Trav = Trav.cofactor_false();
          Sample[CurrentVar] = false;   // Exclude feature
        }
      }

    return Sample;
  }

} // namespace bdd::sample