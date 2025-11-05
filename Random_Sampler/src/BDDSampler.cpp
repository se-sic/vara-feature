#include "BDDSampler.h"
#include "BDDFactory.h"
#include "oxidd/util.hpp"
namespace bdd::sample {

  std::unordered_map<oxidd::level_no_t, bool> Sample; // NOLINT

  // Helper function: Generates a random number ∈ [0,1)
  double random() { return static_cast<double>(std::rand()) / RAND_MAX; }

  // Generate a random configuration by traversing the BDD
  std::unordered_map<oxidd::level_no_t, bool> generateConfiguration(const oxidd::bdd_manager &Manager, const oxidd::bdd_function& Root, BDDFactory &Factory) { //NOLINT
      oxidd::bdd_function Trav = Root;
      BDDFactory::BranchType NT = BDDFactory::BranchType::NONE;

      // Get the LEVEL of the root node, not the variable number
      auto RootOpt = Root.node_level();
      if (!RootOpt.has_value()) {
        throw std::logic_error("Root node does not have a valid level.");
      }
      oxidd::level_no_t RootLevel = *RootOpt;

      // Fill variables that come BEFORE the root in the ordering
      // We need to find all variables with level < root_level
      oxidd::level_no_t TotalLevels = Manager.num_vars(); // Fixed case style

      for (oxidd::level_no_t Level = 0; Level < RootLevel; Level++) {
        oxidd::var_no_t Var = Manager.var_to_level(Level);
        Sample[Var] = random() < 0.5; // Random assignment for variables before root
      }

      // Main BDD traversal
      oxidd::level_no_t PrevLevel = RootLevel - 1;

      while (!Trav.valid()) {
        auto LevelOpt = Trav.node_level();
        if (!LevelOpt.has_value()) {
          throw std::logic_error("BDD node does not have a valid level.");
        }
        oxidd::level_no_t CurrentLevel = *LevelOpt;
        auto VarOpt = Trav.node_var();
        if (!VarOpt.has_value()) {
          throw std::logic_error("BDD node does not have a valid variable.");
        }
        oxidd::var_no_t CurrentVar = *VarOpt;

        // Fill gaps between previous level and current level with random values
        for (oxidd::level_no_t Level = PrevLevel + 1; Level < CurrentLevel; Level++) {
          oxidd::var_no_t Var = Manager.level_to_var(Level);
          Sample[Var] = random() < 0.5;
        }

        // Make probabilistic decision based on precomputed probability
        auto *TravFeat = Factory.findFeatureinBDD(&Trav, NT);
        if (!TravFeat->Probability.has_value()) {
            throw std::logic_error("Missing probability for feature: " + TravFeat->Name);
        }

        double Probability = std::clamp(*TravFeat->Probability, 0.0, 1.0);

        // Choose branch based on probability
        if (random() < Probability) {
          Trav = Trav.cofactor_true(); // Take true branch
          Sample[CurrentVar] = true; // Include feature
        } else {
          Trav = Trav.cofactor_false(); // Take false branch
          Sample[CurrentVar] = false; // Exclude feature
        }

        PrevLevel = CurrentLevel;
      }

      // Fill remaining variables after the last processed level
      for (oxidd::level_no_t Level = PrevLevel + 1; Level < TotalLevels; Level++) {
        oxidd::var_no_t Var = Manager.level_to_var(Level);
        Sample[Var] = random() < 0.5;
      }

      return Sample;
  };
} // namespace bdd::sample