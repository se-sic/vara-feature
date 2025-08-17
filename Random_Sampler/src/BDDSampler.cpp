#include "../../BDD/include/BDDFactory.h"
#include "BDDSampler.h"
#include <unordered_map>
#include <cstdlib>
#include "oxidd/capi.h"

namespace oxidd::capi {

    // Globale Variablen zur Speicherung der aktuellen Stichprobe und Knotenwahrscheinlichkeiten
    std::unordered_map<oxidd_level_no_t , bool> sample; // Speichert die Belegung der Variablen (true/false)
    std::unordered_map<oxidd_bdd_t, double, BDDHash> pr; // Wird von BDDFactory mit Wahrscheinlichkeiten aus getPr() gefüllt

    // Hilfsfunktion: Generiert eine Zufallszahl ∈ [0,1)
    double random() {
        return static_cast<double>(std::rand()) / RAND_MAX;
    }

    // Prüft, ob ein BDD-Knoten der Terminalknoten "true" ist
    bool is_bdd_true(const oxidd_bdd_t& f, oxidd_bdd_manager_t manager) {
        oxidd_bdd_t true_node = oxidd_bdd_true(manager);
        return f._p == true_node._p && f._i == true_node._i;
    }

    /**
    * Generiert eine uniform zufällige Konfiguration aus dem BDD.
    * 
    * Nutzt die von BDDFactory::getPr() berechneten Wahrscheinlichkeiten (feat->info.probability),
    * um sicherzustellen, dass jede gültige Konfiguration mit gleicher Wahrscheinlichkeit erscheint.
    * 
    * @param ROOT      Wurzelknoten des BDD (von BDDFactory::modelToBdd erstellt)
    * @param factory   BDDFactory-Instanz (für Zugriff auf Feature-Informationen)
    * @return          Die generierte Konfiguration (Variable → Wert)
    */
    std::unordered_map<oxidd_level_no_t , bool> generateConfiguration(
        oxidd_bdd_t ROOT,
        BDDFactory& factory) {

        oxidd_bdd_t trav = ROOT; // Startet bei der Wurzel
        oxidd_bdd_manager_t manager = oxidd_bdd_containing_manager(ROOT);
        oxidd_bdd_t TRUE = oxidd_bdd_true(manager);

        // 1. Initialisierung: Zufällige Belegung für Variablen vor der Wurzelebene
        // (Diese Variablen sind redundant und haben keinen Einfluss auf die Gültigkeit)
        for (uint32_t i = 0; i < oxidd_bdd_level(ROOT); ++i) {
            sample[i] = (random() < 0.5);
        }

        // 2. Traversierung des BDDs bis zum "true"-Terminalknoten
        while (trav._p != nullptr && !is_bdd_true(trav, manager)) {
            uint32_t ind = oxidd_bdd_level(trav); // Aktuelle Ebene
            oxidd_bdd_pair_t cofactors = oxidd_bdd_cofactors(trav); // High- und Low-Kinder
            auto trav_feat = factory.findFeatureinBDD(&trav); // Feature-Info (enthält probability)

            // Entscheidung: High-Kind (true) oder Low-Kind (false) basierend auf der
            // von getPr() berechneten Wahrscheinlichkeit
            if (random() < trav_feat->info.probability) { 
                trav = cofactors.first; // High-Kind (Variable = true)
                sample[ind] = true;
            } else {
                trav = cofactors.second; // Low-Kind (Variable = false)
                sample[ind] = false;
            }

            // 3. Zufällige Belegung für Variablen zwischen aktueller und nächster Ebene
            // (Reduzierte Knoten – haben keinen Einfluss auf die Gültigkeit)
            for (uint32_t i = ind + 1; i < oxidd_bdd_level(ROOT); ++i) {
                sample[i] = (random() < 0.5);
            }
        }

        return sample; // Gültige Konfiguration gemäß BDD
    }
}