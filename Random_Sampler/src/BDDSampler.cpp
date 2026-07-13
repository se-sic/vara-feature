#include "BDDSampler.h"
#include "BDDFactory.h"
#include "oxidd/bdd.hpp"
#include "oxidd/util.hpp"
#include <cstddef>
#include <random>
#include <utility>
#include <vector>
namespace bdd::sample {

  std::vector<bool> Sample; // NOLINT

  // Helper function: Generates a random number ∈ [0,1)
  static std::mt19937 Rng; //NOLINT
  static std::uniform_real_distribution<double> Dist(0.0, 1.0); //NOLINT
  double random() {  
    return Dist(Rng); 
  }

  void SeedRng(std::uint32_t Seed) { Rng.seed(Seed); } //NOLINT

  // Generate a random configuration by traversing the BDD
  std::vector<bool> generateConfiguration(const oxidd::bdd_manager &Manager, 
    const oxidd::bdd_function& Root, 
    BDDFactory &Factory, 
    std::map<oxidd::bdd_function, 
    std::pair<double, double>> *SatMap) {

      oxidd::bdd_function Trav = Root;

      // Get the LEVEL of the root node, not the variable number
      auto RootOpt = Root.node_level();
      if (!RootOpt.has_value()) {
        throw std::logic_error("Root node does not have a valid level.");
      }

      oxidd::level_no_t TotalLevels = Manager.num_vars();

      std::vector<bool> Sample;
      Sample.reserve(TotalLevels);

      // Initialize sample with random values for all variables
      for (oxidd::var_no_t I = 0; I < TotalLevels; ++I) {
        Sample.push_back(random() < 0.5);
      }

      // Main BDD traversal
      while (Trav.node_var().has_value()) {
        auto VarIdOpt = Trav.node_var();
        oxidd::var_no_t CurrentVar = VarIdOpt.value();

        double Probability = SatMap->at(Trav).second;

        // Choose branch based on probability
        auto CheckRan = random();
        if (CheckRan < Probability) {
          Trav = Trav.cofactor_true(); // Take true branch
          Sample[CurrentVar] = true;   // Include feature
        } else {
          Trav = Trav.cofactor_false(); // Take false branch
          Sample[CurrentVar] = false;   // Include feature
        }
      }
    return Sample;
  };
} // namespace bdd::sample