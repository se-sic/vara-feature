#include "Constraints.h"
#include <oxidd/bdd.hpp>
#include <oxidd/util.hpp>
#include <vector>

namespace bdd::sample {

    /**
     * Add a constraint to the BDD, optionally negating it
     */
    oxidd::bdd_function BDDConstraintVisitor::addConstraint(vara::feature::Constraint* C, bool Negate, bool RequireAll) {
        this->RequireAll = RequireAll;
        C->accept(*this); // Visit the constraint
        if (Negate) {
            (*CurrentBDD) = ~(*CurrentBDD); // Negate if requested
        }
        if (IsMixedConstraint && RequireAll) {
            (*CurrentBDD) = VariableConstraint & (*CurrentBDD);
        } 
        return (*CurrentBDD);
    }

    /**
     * Visit binary constraints (AND, OR, IMPLIES, etc.)
     */
    bool BDDConstraintVisitor::visit(vara::feature::BinaryConstraint* C) {
        using CK = vara::feature::Constraint::ConstraintKind;
        
        // Process Left and Right operands
        C->getLeftOperand()->accept(*this);
        oxidd::bdd_function Left  = (*CurrentBDD);
        
        C->getRightOperand()->accept(*this);
        oxidd::bdd_function Right = (*CurrentBDD);

        // Apply appropriate BDD operation based on constraint type
        switch(C->getKind()) {
            case CK::CK_AND:
                (*CurrentBDD) = (*CurrentBDD) & (Left & Right);
                break;
            case CK::CK_OR:
                (*CurrentBDD) = ((*CurrentBDD) & (Left | Right));
                break;
            case CK::CK_IMPLIES:
                (*CurrentBDD) = (*CurrentBDD) &(Left.imp(Right));
                break;
            case CK::CK_EQUIVALENCE:
                (*CurrentBDD) = (*CurrentBDD) & (Left.equiv(Right));
                break;
            case CK::CK_XOR:
                (*CurrentBDD) = (*CurrentBDD) & (Left ^ Right);
                break;
            case CK::CK_EXCLUDES:
                (*CurrentBDD) = (*CurrentBDD) & (Left.imp(~Right));
                break;

            case CK::CK_LESS:
            case CK::CK_GREATER:
            case CK::CK_LESS_EQUAL:
            case CK::CK_GREATER_EQUAL:
            case CK::CK_EQUAL: // Boolean equality handled below
            case CK::CK_NOT_EQUAL: // Boolean inequality handled below
                // For boolean comparisons, use equivalence/not equivalence
                if (C->getKind() == CK::CK_EQUAL) {
                    (*CurrentBDD) = (*CurrentBDD) & (Left.equiv(Right));
                } else if (C->getKind() == CK::CK_NOT_EQUAL) {
                    (*CurrentBDD) = (*CurrentBDD) & ~(Left.equiv(Right));
                } else {
                    std::cerr << "Error: Numeric comparisons (<, >, <=, >=) are not supported. Only binary constraints are supported.\n";
                    (*CurrentBDD) = Manager->f();
                    return false;
                }
                break;

            case CK::CK_ADDITION:
            case CK::CK_SUBTRACTION:
            case CK::CK_MULTIPLICATION:
            case CK::CK_DIVISION:
                std::cerr << "Error: Arithmetic operations (+, -, *, /) are not supported. Only binary constraints are supported.\n";
                (*CurrentBDD) = Manager->f(); 
                return false;

            default:
                std::cerr << "Error: Unknown constraint type encountered. Only binary constraints are supported.\n";
                (*CurrentBDD) = Manager->f();
                return false;
        }
        return true;
    }

    /**
     * / Visit unary constraints (NOT, NEG)
     */
    bool BDDConstraintVisitor::visit(vara::feature::UnaryConstraint* C) {
        using CK = vara::feature::Constraint::ConstraintKind;
        
        C->getOperand()->accept(*this);
        switch(C->getKind()) {
            case CK::CK_NOT:
                (*CurrentBDD) = ~(*CurrentBDD);
                return true;
            case CK::CK_NEG:
                std::cerr << "Error: Numeric negation (~) is not supported. Only binary constraints are supported.\n";
                (*CurrentBDD) = Manager->f();
                return false;
            default:
                std::cerr << "Error: Unknown unary constraint type encountered. Only binary constraints are supported.\n";
                (*CurrentBDD) = Manager->f();
                return false;
        }
    }

    /**
     * Visit feature constraints (e.g., "FeatureA")
     */
    bool BDDConstraintVisitor::visit(vara::feature::PrimaryFeatureConstraint* C) {
        std::string FeatureName = C->getFeature()->getName().str();
        auto IdCheck = Manager->name_to_var(FeatureName);
        if (!IdCheck.has_value()) {
            std::cerr << "Error: Could not find variable ID for feature '" << FeatureName << "'.\n";
            return false;
        }
        oxidd::var_no_t Id = IdCheck.value();

        // Check for unsupported numeric features
        if (C->getFeature()->getKind() == vara::feature::Feature::FeatureKind::FK_NUMERIC) {
            std::cerr << "Error: Numeric features are not supported. Feature '" << FeatureName 
                    << "' is numeric. Only binary features are supported.\n";
            (*CurrentBDD) = Manager->f();
            return false;
        }

        return handleFeatureConstraint(Id);
    }

    /**
     * Verarbeitet Integer-Constraints (werden nicht unterstützt).
     */
    // bool BDDConstraintVisitor::visit(vara::feature::PrimaryIntegerConstraint* C) {
    //     std::cerr << "Error: Integer constraints are not supported. Value '" << C->getValue() 
    //             << "' encountered. Only binary constraints are supported.\n";
    //     CurrentBDD = oxidd_bdd_false(Manager);
    //     return false;
    // }

    /**
     * Handle a feature constraint by looking up its BDD variable
     */
    bool BDDConstraintVisitor::handleFeatureConstraint(const oxidd::capi::oxidd_var_no_t Id) {
        auto It = VarMap->find(Id);
        if (It != VarMap->end()) {
            auto Feat = It->second;
            (*CurrentBDD) = Manager->var(Id) & Feat.BddNode;
        }    
        // Falls das Feature nicht gefunden wurde, wird eine temporäre Variable erstellt
        // static oxidd_var_no_t tempVarCounter = 10000;
        // oxidd_bdd_t var = oxidd_bdd_var(Manager, tempVarCounter++);
        // CurrentBDD = var;
        return true;
    }

    /**
     * Process all constraints of a feature model and add them to the BDD
     */
     void processConstraints(//NOLINT 
        oxidd::bdd_manager &Manager,
        oxidd::bdd_function &Bdd,
        GlobalVarMap &VarMap,
        const vara::feature::FeatureModel &Model) {
            
        // Initialize constraint visitor with all required parameters
        BDDConstraintVisitor Visitor(Manager, VarMap, Bdd, false, false);

       // Lambda function to process a single constraint
        const auto Process = [&](const auto& Constraint) {
            oxidd::bdd_function ConstraintBDD = Visitor.addConstraint(Constraint->constraint());
            if (ConstraintBDD.is_invalid()) {
                std::cerr << "Warning: Failed to process constraint. Skipping.\n";
                return false;
            }
            Bdd = Bdd & ConstraintBDD; // Add constraint to main BDD
            return true;
        };

        std::string_view DiagramName = "HIPPACC"; 
        std::vector<oxidd::bdd_function> Func = {Bdd};
        int Cnt = 0;
        // Process only Boolean constraints (others are not supported)
        for (const auto& C : Model.booleanConstraints()) {
            std::cout << "Processing Boolean Constraint " << ++Cnt << '\n';
            if(Cnt > 735) {
                auto Result = Manager.visualize(DiagramName, Func);
            }
            // auto Result = Manager.visualize(DiagramName, Func);
            if (!Process(C)) { break; }
        }
    }

} // namespace bdd::sample