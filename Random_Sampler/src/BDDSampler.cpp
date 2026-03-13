#include "BDDFactory.h"
#include "oxidd/bdd.hpp"
#include "oxidd/util.hpp"
#include <random>
#include <utility>
namespace bdd::sample {

  std::vector<bool> Sample; // NOLINT

  // Helper function: Generates a random number ∈ [0,1)
  static std::mt19937 Rng(std::random_device{}()); //NOLINT
  static std::uniform_real_distribution<double> Dist(0.0, 1.0); //NOLINT
  double random() {  
    return Dist(Rng); 
  }

  // Generate a random configuration by traversing the BDD
  std::vector<bool> generateConfiguration(
    const oxidd::bdd_manager &Manager, 
    const oxidd::bdd_function& Root, 
    BDDFactory &Factory, 
    std::map<oxidd::bdd_function, std::pair<double, double>> *SatMap) {
    
    oxidd::bdd_function Trav = Root;
    oxidd::level_no_t TotalLevels = Manager.num_vars();
    
    Sample.clear();
    Sample.reserve(TotalLevels);
    
    // Initialize with random
    for (oxidd::var_no_t I = 0; I < TotalLevels; ++I) {
        Sample.push_back(random() < 0.5);
    }
        
    // ORIGINAL condition
    while (!Trav.valid()) {
        
        auto VarIdOpt = Trav.node_var();
        if (!VarIdOpt.has_value()) {
            std::cout << "No variable - breaking\n";
            break;
        }
        
        oxidd::var_no_t CurrentVar = VarIdOpt.value();
        
        double Probability = SatMap->at(Trav).second;
        double CheckRan = random();
        
        if (CheckRan < Probability) {
            Trav = Trav.cofactor_true();
            Sample[CurrentVar] = true;
        } else {
            Trav = Trav.cofactor_false();
            Sample[CurrentVar] = false;
        }

        
    }
    
    
    return Sample;
}
} // namespace bdd::sample