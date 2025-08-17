#include "BDDFactory.h"
#include "vara/Feature/FeatureModel.h"
#include <algorithm>
#include <unordered_map>
#include "Constraints.h"

// Alias-Definitionen für bessere Lesbarkeit
using GlobalVarMap = std::unordered_map<std::string, oxidd::capi::BDDFactory::BDDFeat>;
using BinaryVarMap = std::unordered_map<std::string, oxidd::capi::oxidd_bdd_t>;
using NumericVarMap = std::unordered_map<std::string, std::vector<std::pair<std::string, oxidd::capi::oxidd_bdd_t>>>;

namespace oxidd::capi {


 /**
 * Fügt einen Constraint zur BDD hinzu und berücksichtigt dabei, ob der Constraint negiert werden soll.
 * @param C Der Constraint, der verarbeitet werden soll.
 * @param negate Falls true, wird der Constraint negiert.
 * @param requireAll Falls true, werden alle Variablen des Constraints als erforderlich behandelt.
 * @return Die erzeugte BDD für den Constraint.
 */
  oxidd_bdd_t BDDConstraintVisitor::addConstraint(vara::feature::Constraint* C, bool negate, bool requireAll) {
    this->RequireAll = requireAll;
    C->accept(*this); // Besucht den Constraint und erzeugt die BDD in CurrentBDD
    if (negate) {
      CurrentBDD = oxidd_bdd_or(oxidd_bdd_false(Manager), CurrentBDD); // Negiert den Constraint falls erforderlich
    }
    if (IsMixedConstraint && RequireAll) {
      CurrentBDD = oxidd_bdd_or(VariableConstraint, CurrentBDD); // Kombiniert mit Variablen-Constraint
    } 
    return CurrentBDD;
  }

  /**
  * Verarbeitet binäre Constraints (AND, OR, IMPLIES, etc.).
  * @param C Der binäre Constraint.
  * @return true, falls die Verarbeitung erfolgreich war.
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
        CurrentBDD = oxidd_bdd_imp(left, oxidd_bdd_not(right)); // ¬(A ∧ B)
        break;

      // Numerische Vergleiche (<, >, <=, >=, =, !=)  
      case CK::CK_LESS:
        CurrentBDD = handleNumericExpr(C, "<");
        break;
      case CK::CK_GREATER:
        CurrentBDD = handleNumericExpr(C, ">");
        break;
      case CK::CK_LESS_EQUAL:
        CurrentBDD = handleNumericExpr(C, "<=");
        break;
      case CK::CK_GREATER_EQUAL:
        CurrentBDD = handleNumericExpr(C, ">=");
        break;
      case CK::CK_EQUAL:
        if (isNumericComparison(C)) {
          CurrentBDD = handleNumericExpr(C, "=");
        } else {
          CurrentBDD = oxidd_bdd_equiv(left, right);
        }
        break;
      case CK::CK_NOT_EQUAL:
        if (isNumericComparison(C)) {
          CurrentBDD = handleNumericExpr(C, "!=");
        } else {
          CurrentBDD = oxidd_bdd_not(oxidd_bdd_equiv(left, right));
        }
        break;

      // Arithmetische Operationen (+, -, *, /)
      case CK::CK_ADDITION:
      case CK::CK_SUBTRACTION:
      case CK::CK_MULTIPLICATION:
      case CK::CK_DIVISION:
        CurrentBDD = createTempVarForOperation(C);  // Erstellt temporäre Variable für komplexe Ausdrücke
        break;

      default:
        CurrentBDD = oxidd_bdd_false(Manager); // Ungültiger Constraint
        return false;
    }
    return true;
  }

  /**
  * Verarbeitet unäre Constraints (NOT, NEG).
  * @param C Der unäre Constraint.
  * @return true, falls die Verarbeitung erfolgreich war.
  */
  bool BDDConstraintVisitor::visit(vara::feature::UnaryConstraint* C) {
    using CK = vara::feature::Constraint::ConstraintKind;
    
    C->getOperand()->accept(*this);
    switch(C->getKind()) {
      case CK::CK_NOT:
        CurrentBDD = oxidd_bdd_not(CurrentBDD); // Logische Negation
        return true;
      case CK::CK_NEG:
        CurrentBDD = createTempVar("neg"); // Numerische Negation (als temporäre Variable)
        return true;
      default:
        CurrentBDD = oxidd_bdd_false(Manager);
        return false;
    }
  }

  /**
  * Verarbeitet Feature-Constraints (z. B. "FeatureA" oder "FeatureB = 5").
  * @param C Der Feature-Constraint.
  * @return true, falls die Verarbeitung erfolgreich war.
  */
  bool BDDConstraintVisitor::visit(vara::feature::PrimaryFeatureConstraint* C) {
    std::string featureName = C->getFeature()->getName().str();

    // Falls es sich um ein numerisches Feature handelt und Mixed-Constraints aktiv sind,
    // wird eine temporäre Variable erstellt.
    if (C->getFeature()->getKind()== vara::feature::Feature::FeatureKind::FK_NUMERIC) {
      if (IsMixedConstraint) {
        oxidd_bdd_t var = oxidd_bdd_new_var(Manager);
        (*binaryVarMap)[featureName] = var;
        VariableConstraint = oxidd_bdd_or(VariableConstraint, oxidd_bdd_not(var));
        CurrentBDD = var;
        return true;
      } else {
        return handleFeatureConstraint(featureName); // Standardbehandlung für Features
      }
    } else {
      return handleFeatureConstraint(featureName); // Standardbehandlung für Features
    }
    
  }

  /**
  * Verarbeitet Integer-Constraints (z. B. "42").
  * @param C Der Integer-Constraint.
  * @return true, falls die Verarbeitung erfolgreich war.
  */
  bool BDDConstraintVisitor::visit(vara::feature::PrimaryIntegerConstraint* C) {
    CurrentBDD = oxidd_bdd_false(Manager);
    return true;
  }

  /**
  * Überprüft, ob ein Constraint einen numerischen Vergleich darstellt.
  * @param C Der zu überprüfende Constraint.
  * @return true, falls es sich um einen numerischen Vergleich handelt.
  */
  bool BDDConstraintVisitor::isNumericComparison(vara::feature::BinaryConstraint* C) {
    using namespace vara::feature;
    return (dynamic_cast<NumericConstraint*>(C->getLeftOperand()) ||
            dynamic_cast<PrimaryIntegerConstraint*>(C->getLeftOperand())) &&
           (dynamic_cast<NumericConstraint*>(C->getRightOperand()) ||
            dynamic_cast<PrimaryIntegerConstraint*>(C->getRightOperand()));
  }

  /**
  * Verarbeitet numerische Ausdrücke (z. B. "A < B").
  * @param C Der Constraint.
  * @param op Der Operator (z. B. "<", ">=", etc.).
  * @return Die erzeugte BDD für den Ausdruck.
  */
  oxidd_bdd_t BDDConstraintVisitor::handleNumericExpr(vara::feature::BinaryConstraint* C, 
                              const std::string& op) {
    std::string left = getOperandName(C->getLeftOperand());
    std::string right = getOperandName(C->getRightOperand());
    std::string constraintName = left + op + right;
    
    // Falls der Constraint bereits existiert, wird die zugehörige BDD-Variable zurückgegeben
    auto it = VarMap->find(constraintName);
    oxidd_bdd_t child;
    if (it != VarMap->end()) {
      auto feat = it->second;  
      if(feat.type == BDDFactory::featType::BINARY) {
        oxidd_bdd_t* node_ptr = std::get<oxidd_bdd_t*>(feat.data);
        CurrentBDD =  *node_ptr;
      } else if(feat.type == BDDFactory::featType::NUMERIC) {
        auto numericFeats = std::get<std::vector<std::pair<string, oxidd_bdd_t>>*>(feat.data);
         child = std::find_if(numericFeats->begin(), numericFeats->end(), [&](const std::pair<std::string, oxidd_bdd_t>& pair){
            return pair.first == constraintName;}
          )->second;
        if (child._p != nullptr) {
          CurrentBDD = child;
        }
      }
      return oxidd_bdd_t{nullptr, 0};
    }


    // Andernfalls wird eine neue BDD-Variable erstellt
    oxidd_bdd_t var = oxidd_bdd_new_var(Manager);
    (*binaryVarMap)[constraintName] = var; 
    return var;
  }

  /**
  * Erstellt eine temporäre Variable für eine arithmetische Operation (z. B. "A + B").
  * @param C Der Constraint.
  * @return Die erzeugte BDD-Variable.
  */
  oxidd_bdd_t BDDConstraintVisitor::createTempVarForOperation(vara::feature::BinaryConstraint* C) {
    using CK = vara::feature::Constraint::ConstraintKind;
    static const std::unordered_map<CK, std::string> opMap = {
      {CK::CK_ADDITION, "add"},
      {CK::CK_SUBTRACTION, "sub"},
      {CK::CK_MULTIPLICATION, "mul"},
      {CK::CK_DIVISION, "div"}
    };
    
    std::string tempName = opMap.at(C->getKind()) + "_" + std::to_string(tempCounter++);
    return createTempVar(tempName);
  }

  /**
  * Erstellt eine temporäre BDD-Variable.
  * @param name Der Name der Variable.
  * @return Die erzeugte BDD-Variable.
  */
  oxidd_bdd_t BDDConstraintVisitor::createTempVar(const std::string& name) {
    oxidd_bdd_t var = oxidd_bdd_new_var(Manager);
    (*binaryVarMap)[name] = var;
    return var;
  }

  /**
  * Extrahiert den Namen eines Operanden (Feature oder Integer).
  * @param operand Der Operand.
  * @return Der Name oder Wert als String.
  */
  std::string BDDConstraintVisitor::getOperandName(vara::feature::Constraint* operand) {
    using namespace vara::feature;
    
    if (auto fc = dynamic_cast<PrimaryFeatureConstraint*>(operand)) {
      return fc->getFeature()->getName().str();
    }
    else if (auto ic = dynamic_cast<PrimaryIntegerConstraint*>(operand)) {
      return std::to_string(ic->getValue());
    }
    return "const_0";
  }

  /**
  * Verarbeitet einen Feature-Constraint (z. B. "FeatureA").
  * @param name Der Name des Features.
  * @return true, falls die Verarbeitung erfolgreich war.
  */
  bool BDDConstraintVisitor::handleFeatureConstraint(const std::string& name) {
    auto it = VarMap->find(name);
    oxidd_bdd_t child;
    if (it != VarMap->end()) { //TODO --> DONE?
      auto feat = it->second;
      if(feat.type == BDDFactory::featType::BINARY) {
        oxidd_bdd_t* node_ptr = std::get<oxidd_bdd_t*>(feat.data);
        CurrentBDD = *node_ptr;
      } else if(feat.type == BDDFactory::featType::NUMERIC) {
        auto numericFeats = std::get<std::vector<std::pair<string, oxidd_bdd_t>>*>(feat.data);
          child = std::find_if(numericFeats->begin(), numericFeats->end(), [&](const std::pair<std::string, oxidd_bdd_t>& pair){
            return pair.first == name;}
          )->second;
          if (child._p != nullptr) {
          CurrentBDD = child;
        }
      }
      return true;
    }
    CurrentBDD = createTempVar(name); // Falls das Feature nicht gefunden wurde, wird eine temporäre Variable erstellt
    return true;
  }

  /**
  * Verarbeitet alle Constraints eines Feature-Modells und fügt sie der BDD hinzu.
  * @param manager Der BDD-Manager.
  * @param bdd Die aktuelle BDD, zu der die Constraints hinzugefügt werden.
  * @param varMap Die Map aller Features und ihrer BDD-Repräsentationen.
  * @param binaryVarMap Die Map der binären Features.
  * @param numericVarMap Die Map der numerischen Features.
  * @param model Das Feature-Modell, aus dem die Constraints gelesen werden.
  */
  void processConstraints(
      oxidd_bdd_manager_t manager,
      oxidd_bdd_t& bdd,
      GlobalVarMap& varMap,
      ::BinaryVarMap& binaryVarMap,
      ::NumericVarMap& numericVarMap,
      const vara::feature::FeatureModel& model) {
  
    // Initialisiert den Constraint-Visitor
    BDDConstraintVisitor visitor(manager, &varMap, &binaryVarMap, &numericVarMap);

    // Lambda-Funktion zur Verarbeitung eines einzelnen Constraints
    const auto process = [&](const auto& constraint) {
      oxidd_bdd_t constraintBDD = visitor.addConstraint(constraint->constraint());
      if (constraintBDD._p == nullptr) return false; // Fehlerfall
      bdd = oxidd_bdd_and(bdd, constraintBDD); // Fügt den Constraint zur BDD hinzu
      return true;
    };

    // Lambda-Funktion zur Verarbeitung von Mixed Constraints
    const auto processMixed = [&] (const auto& constraint) {
      BDDConstraintVisitor mixedVisitor(manager, &varMap, &binaryVarMap, &numericVarMap, true);
      oxidd_bdd_t constraintBDD = mixedVisitor.addConstraint(constraint->constraint(), 
        constraint->exprKind() == vara::feature::FeatureModel::MixedConstraint::ExprKind::NEG,
        constraint->req() == vara::feature::FeatureModel::MixedConstraint::Req::ALL);
      if (constraintBDD._p == nullptr) {
        return false;
      } // Fehlerfall
      bdd = oxidd_bdd_and(bdd, constraintBDD); // Fügt den Constraint zur BDD hinzu
      return true;
    };

    // Verarbeitet alle Constraint-Typen:
    // 1. Boolean Constraints (z. B. "A ∧ B")
    for (const auto& C : model.booleanConstraints()) if (!process(C)) break; // Bei Fehler abbrechen

    // 2. Non-Boolean Constraints (z. B. "A < 5")
    for (const auto& C : model.nonBooleanConstraints()) if (!process(C)) break;

    // 3. Mixed Constraints (z. B. "A ∧ (B < 5)")
    for (const auto& C : model.mixedConstraints()) if (!process(C)) break;
  }

} // namespace oxidd::capi