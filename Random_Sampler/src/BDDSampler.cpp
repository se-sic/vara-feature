#include "BDDSampler.h"
#include "../../BDD/include/BDDFactory.h"
#include "oxidd/capi.h"
#include <cstdlib> //for RAND_MAX
#include <unordered_map>

// namespace oxidd::capi {

// Globale Variablen zur Speicherung der aktuellen Stichprobe und
// Knotenwahrscheinlichkeiten
std::unordered_map<oxidd_level_no_t, bool>
    sample; // Speichert die Belegung der Variablen (true/false)
std::unordered_map<oxidd_bdd_t, double, BDDHash>
    pr; // Wird von BDDFactory mit Wahrscheinlichkeiten aus getPr() gefüllt

// Hilfsfunktion: Generiert eine Zufallszahl ∈ [0,1)
double random() { return static_cast<double>(std::rand()) / RAND_MAX; }

// Prüft, ob ein BDD-Knoten der Terminalknoten "true" ist
bool is_bdd_true(const oxidd_bdd_t &f, oxidd_bdd_manager_t manager) {
  oxidd_bdd_t true_node = oxidd_bdd_true(manager);
  return f._p == true_node._p && f._i == true_node._i;
}

std::unordered_map<oxidd_var_no_t, bool>
generateConfiguration(oxidd_bdd_manager_t manager, oxidd_bdd_t root,
                      const std::unordered_map<oxidd_bdd_t, double> &pr_map,
                      BDDFactory &factory) {

  std::unordered_map<oxidd_var_no_t, bool> sample;
  oxidd_bdd_t trav = root;

  // Get the LEVEL of the root node, not the variable number
  oxidd_level_no_t root_level = oxidd_bdd_node_level(root);

  // Fill variables that come BEFORE the root in the ordering
  // We need to find all variables with level < root_level
  oxidd_level_no_t total_levels = oxidd_bdd_manager_num_vars(manager);

  for (oxidd_level_no_t level = 0; level < root_level; level++) {
    oxidd_var_no_t var = oxidd_bdd_manager_level_to_var(manager, level);
    sample[var] = random() < 0.5;
  }

  // Main traversal
  oxidd_level_no_t prev_level = root_level - 1;

  while (trav._p != nullptr && !is_bdd_true(trav, manager)) {
    oxidd_level_no_t current_level = oxidd_bdd_node_level(trav);
    oxidd_var_no_t current_var = oxidd_bdd_node_var(trav);

    // Fill gaps between previous level and current level
    for (oxidd_level_no_t level = prev_level + 1; level < current_level;
         level++) {
      oxidd_var_no_t var = oxidd_bdd_manager_level_to_var(manager, level);
      sample[var] = random() < 0.5;
    }

    // Make probabilistic decision
    auto trav_feat = factory.findFeatureinBDD(&trav);
    double probability = trav_feat->info.probability;
    if (random() < probability) {
      trav = oxidd_bdd_cofactor_true(trav);
      sample[current_var] = true;
    } else {
      trav = oxidd_bdd_cofactor_false(trav);
      sample[current_var] = false;
    }

    prev_level = current_level;
  }

  // Fill variables after the last processed level
  for (oxidd_level_no_t level = prev_level + 1; level < total_levels; level++) {
    oxidd_var_no_t var = oxidd_bdd_manager_level_to_var(manager, level);
    sample[var] = random() < 0.5;
  }

  return sample;
}
} // namespace oxidd::capi