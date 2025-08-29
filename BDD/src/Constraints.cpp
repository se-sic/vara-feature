#include "BDDFactory.h"
#include "vara/Feature/FeatureModel.h"
#include <algorithm>
#include <unordered_map>
#include "Constraints.h"

// Alias-Definitionen für bessere Lesbarkeit
using GlobalVarMap = std::unordered_map<std::string, oxidd::capi::BDDFactory::BDDFeat>;
using BinaryVarMap = std::unordered_map<std::string, oxidd::capi::oxidd_bdd_t>;

namespace oxidd::capi {

/**
 * Fügt einen Constraint zur BDD hinzu und berücksichtigt dabei, ob der Constraint negiert werden soll.
 */
oxidd_bdd_t BDDConstraintVisitor::addConstraint(vara::feature::Constraint* C, bool negate, bool requireAll) {
    this->RequireAll = requireAll;
    C->accept(*this);
    if (negate) {
        CurrentBDD = oxidd_bdd_not(CurrentBDD);
    }
    if (IsMixedConstraint && RequireAll) {
        CurrentBDD = oxidd_bdd_and(VariableConstraint, CurrentBDD);
    } 
    return CurrentBDD;
}

/**
 * Verarbeitet binäre Constraints (AND, OR, IMPLIES, etc.).
 */
bool BDDConstraintVisitor::visit(vara::feature::BinaryConstraint* C) {
    using CK = vara::feature::Constraint::ConstraintKind;
    
    // Verarbeite linken und rechten Operanden
    C->getLeftOperand()->accept(*this);
    oxidd_bdd_t left = CurrentBDD;
    
    C->getRightOperand()->accept(*this);
    oxidd_bdd_t right = CurrentBDD;

    // Wende die entsprechende BDD-Operation basierend auf dem Constraint-Typ an
    switch(C->getKind()) {
        case CK::CK_AND:
            CurrentBDD = oxidd_bdd_and(left, right);
            break;
        case CK::CK_OR:
            CurrentBDD = oxidd_bdd_or(left, right);
            break;
        case CK::CK_IMPLIES:
            CurrentBDD = oxidd_bdd_imp(left, right);
            break;
        case CK::CK_EQUIVALENCE:
            CurrentBDD = oxidd_bdd_equiv(left, right);
            break;
        case CK::CK_XOR:
            CurrentBDD = oxidd_bdd_xor(left, right);
            break;
        case CK::CK_EXCLUDES:
            CurrentBDD = oxidd_bdd_imp(left, oxidd_bdd_not(right));
            break;

        case CK::CK_LESS:
        case CK::CK_GREATER:
        case CK::CK_LESS_EQUAL:
        case CK::CK_GREATER_EQUAL:
        case CK::CK_EQUAL: // Boolean equality handled below
        case CK::CK_NOT_EQUAL: // Boolean inequality handled below
            // For boolean comparisons, use equivalence/not equivalence
            if (C->getKind() == CK::CK_EQUAL) {
                CurrentBDD = oxidd_bdd_equiv(left, right);
            } else if (C->getKind() == CK::CK_NOT_EQUAL) {
                CurrentBDD = oxidd_bdd_not(oxidd_bdd_equiv(left, right));
            } else {
                std::cerr << "Error: Numeric comparisons (<, >, <=, >=) are not supported. Only binary constraints are supported.\n";
                CurrentBDD = oxidd_bdd_false(Manager);
                return false;
            }
            break;

        case CK::CK_ADDITION:
        case CK::CK_SUBTRACTION:
        case CK::CK_MULTIPLICATION:
        case CK::CK_DIVISION:
            std::cerr << "Error: Arithmetic operations (+, -, *, /) are not supported. Only binary constraints are supported.\n";
            CurrentBDD = oxidd_bdd_false(Manager); 
            return false;

        default:
            std::cerr << "Error: Unknown constraint type encountered. Only binary constraints are supported.\n";
            CurrentBDD = oxidd_bdd_false(Manager);
            return false;
    }
    return true;
}

/**
 * Verarbeitet unäre Constraints (NOT, NEG).
 */
bool BDDConstraintVisitor::visit(vara::feature::UnaryConstraint* C) {
    using CK = vara::feature::Constraint::ConstraintKind;
    
    C->getOperand()->accept(*this);
    switch(C->getKind()) {
        case CK::CK_NOT:
            CurrentBDD = oxidd_bdd_not(CurrentBDD);
            return true;
        case CK::CK_NEG:
            std::cerr << "Error: Numeric negation (~) is not supported. Only binary constraints are supported.\n";
            CurrentBDD = oxidd_bdd_false(Manager);
            return false;
        default:
            std::cerr << "Error: Unknown unary constraint type encountered. Only binary constraints are supported.\n";
            CurrentBDD = oxidd_bdd_false(Manager);
            return false;
    }
}

/**
 * Verarbeitet Feature-Constraints (z. B. "FeatureA").
 */
bool BDDConstraintVisitor::visit(vara::feature::PrimaryFeatureConstraint* C) {
    std::string featureName = C->getFeature()->getName().str();

    if (C->getFeature()->getKind() == vara::feature::Feature::FeatureKind::FK_NUMERIC) {
        std::cerr << "Error: Numeric features are not supported. Feature '" << featureName 
                  << "' is numeric. Only binary features are supported.\n";
        CurrentBDD = oxidd_bdd_false(Manager);
        return false;
    }

    return handleFeatureConstraint(featureName);
}

/**
 * Verarbeitet Integer-Constraints (werden nicht unterstützt).
 */
bool BDDConstraintVisitor::visit(vara::feature::PrimaryIntegerConstraint* C) {
    std::cerr << "Error: Integer constraints are not supported. Value '" << C->getValue() 
              << "' encountered. Only binary constraints are supported.\n";
    CurrentBDD = oxidd_bdd_false(Manager);
    return false;
}

/**
 * Verarbeitet einen Feature-Constraint.
 */
bool BDDConstraintVisitor::handleFeatureConstraint(const std::string& name) {
    auto it = VarMap->find(name);
    if (it != VarMap->end()) {
        auto feat = it->second;
        if(feat.type == BDDFactory::featType::BINARY) {
            oxidd_bdd_t* node_ptr = std::get<oxidd_bdd_t*>(feat.data);
            CurrentBDD = *node_ptr;
            return true;
        } else {
            std::cerr << "Error: Numeric feature '" << name << "' found in variable map. Only binary features are supported.\n";
            CurrentBDD = oxidd_bdd_false(Manager);
            return false;
        }
    }
    
    // Falls das Feature nicht gefunden wurde, wird eine temporäre Variable erstellt
    static oxidd_var_no_t tempVarCounter = 10000;
    oxidd_bdd_t var = oxidd_bdd_var(Manager, tempVarCounter++);
    (*binaryVarMap)[name] = var;
    CurrentBDD = var;
    return true;
}

/**
 * Verarbeitet alle Constraints eines Feature-Modells und fügt sie der BDD hinzu.
 */
void processConstraints(
    oxidd_bdd_manager_t manager,
    oxidd_bdd_t& bdd,
    GlobalVarMap& varMap,
    BinaryVarMap& binaryVarMap,
    const vara::feature::FeatureModel& model) {
  
    // Create a dummy NumericVarMap since it's required by the constructor but not used
    std::unordered_map<std::string, std::vector<std::pair<std::string, oxidd_bdd_t>>> dummyNumericVarMap;
    
    // Initialisiert den Constraint-Visitor mit all required parameters
    BDDConstraintVisitor visitor(manager, &varMap, &binaryVarMap, &dummyNumericVarMap, false, false);

    // Lambda-Funktion zur Verarbeitung eines einzelnen Constraints
    const auto process = [&](const auto& constraint) {
        oxidd_bdd_t constraintBDD = visitor.addConstraint(constraint->constraint());
         if (constraintBDD._p == nullptr) {
            std::cerr << "Warning: Failed to process constraint. Skipping.\n";
            return false;
        }
        bdd = oxidd_bdd_and(bdd, constraintBDD);
        return true;
    };

    // Verarbeitet nur Boolean Constraints (andere werden nicht unterstützt)
    for (const auto& C : model.booleanConstraints()) {
        if (!process(C)) break;
    }
}

} // namespace oxidd::capi