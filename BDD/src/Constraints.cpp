#include "Constraints.h"
#include "BDDFactory.h"
#include <oxidd/bdd.hpp>
#include <oxidd/util.hpp>
#include <vector>

namespace bdd::sample {

    /**
     * Add a constraint to the BDD, optionally negating it
     */
    oxidd::bdd_function BDDConstraintVisitor::addConstraint(vara::feature::Constraint* C, bool Negate, bool RequireAll) {
        this->RequireAll = RequireAll;

        ExprBDD = Manager->t(); // Reset ExprBDD to true
        VariableConstraint = Manager->f(); // Reset VariableConstraint to false

        if (C->accept(*this)) { // Visit the constraint
            oxidd::bdd_function FinalExpr = ExprBDD;

            if (Negate) {
                FinalExpr= ~FinalExpr; // Negate if requested
            }
            if (IsMixedConstraint && RequireAll) {
                FinalExpr = VariableConstraint | FinalExpr;
            } 
            (*CurrentBDD) = (*CurrentBDD) & FinalExpr;

            return FinalExpr;
        }

        return Manager->f(); // Return false BDD on failure
    }

    /**
     * Visit binary constraints (AND, OR, IMPLIES, etc.)
     */
    bool BDDConstraintVisitor::visit(vara::feature::BinaryConstraint* C) {
        using CK = vara::feature::Constraint::ConstraintKind;

        //auto Temp = *CurrentBDD; // Save current BDD state

        // Process Left and Right operands
        C->getLeftOperand()->accept(*this);
        oxidd::bdd_function Left  = ExprBDD;
        
        C->getRightOperand()->accept(*this);
        oxidd::bdd_function Right = ExprBDD;

        // Apply appropriate BDD operation based on constraint type
        switch(C->getKind()) {
            case CK::CK_AND:
                ExprBDD = Left & Right;
                break;
            case CK::CK_OR: {
                ExprBDD = Left | Right;
                break;
            }
            case CK::CK_IMPLIES: {
                ExprBDD = Left.imp(Right);
                break;
            }
            case CK::CK_EQUIVALENCE:
                ExprBDD = Left.equiv(Right);
                break;
            case CK::CK_XOR:
                ExprBDD = Left ^ Right;
                break;
            case CK::CK_EXCLUDES: {
                ExprBDD = Left.imp(~(Right));       
                break;
            }
            case CK::CK_LESS:
            case CK::CK_GREATER:
            case CK::CK_LESS_EQUAL:
            case CK::CK_GREATER_EQUAL:
            case CK::CK_EQUAL: // Boolean equality handled below
            case CK::CK_NOT_EQUAL: // Boolean inequality handled below
                // For boolean comparisons, use equivalence/not equivalence
                if (C->getKind() == CK::CK_EQUAL) {
                    ExprBDD = Left.equiv(Right);
                } else if (C->getKind() == CK::CK_NOT_EQUAL) {
                    ExprBDD = ~(Left.equiv(Right));
                } else {
                    std::cerr << "Error: Numeric comparisons (<, >, <=, >=) are not supported. Only binary constraints are supported.\n";
                    ExprBDD = Manager->f();
                    return false;
                }
                break;

            case CK::CK_ADDITION:
            case CK::CK_SUBTRACTION:
            case CK::CK_MULTIPLICATION:
            case CK::CK_DIVISION:
                std::cerr << "Error: Arithmetic operations (+, -, *, /) are not supported. Only binary constraints are supported.\n";
                ExprBDD = Manager->f(); 
                return false;

            default:
                std::cerr << "Error: Unknown constraint type encountered. Only binary constraints are supported.\n";
                ExprBDD = Manager->f();
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
                ExprBDD = ~ExprBDD;
                return true;
            case CK::CK_NEG:
                std::cerr << "Error: Numeric negation (~) is not supported. Only binary constraints are supported.\n";
                ExprBDD = Manager->f();
                return false;
            default:
                std::cerr << "Error: Unknown unary constraint type encountered. Only binary constraints are supported.\n";
                ExprBDD = Manager->f();
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
        oxidd::var_no_t Id = IdCheck.value(); //NOLINT

        // Check for unsupported numeric features
        if (C->getFeature()->getKind() == vara::feature::Feature::FeatureKind::FK_NUMERIC) {
            std::cerr << "Error: Numeric features are not supported. Feature '" << FeatureName 
                    << "' is numeric. Only binary features are supported.\n";
            (*CurrentBDD) = Manager->f();
            return false;
        }

        oxidd::bdd_function Var = Manager->var(Id);
        ExprBDD = Var;

        if(IsMixedConstraint) {
            VariableConstraint = VariableConstraint | ~Var;
        }

        return true;
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
            auto Res = Manager->var(Id);
            (*CurrentBDD) = Manager->var(Id) & (*CurrentBDD);
            // std::string_view DiagramNameEnd = "HandleBDD"; 
            // std::vector<oxidd::bdd_function> FuncEnd = {*CurrentBDD};
            // auto ResultEnd = Manager->visualize(DiagramNameEnd, FuncEnd);
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
    //  bool processConstraints(  //NOLINT
    //     // TODO oxidd::bdd_manager &Manager,
    //     // TODO oxidd::bdd_function &Bdd,
    //     // TODO GlobalVarMap &VarMap,
    //     // TODO const vara::feature::FeatureModel &Model,
    //     // TODO BDDFactory &Factory
    //     BDDConstraintVisitor &Visitor,
    //     const auto &Constraint,
    //     oxidd::bdd_function &Bdd
    //     ) {
    //         oxidd::bdd_function ConstraintBDD = Visitor.addConstraint(Constraint->constraint());
    //          if (ConstraintBDD.is_invalid()) {
    //             std::cerr << "Warning: Failed to process constraint. Skipping.\n";
    //             return false;
    //             }
    //          Bdd = Bdd & ConstraintBDD;
    //          return true;
    //     }

        // Initialize constraint visitor with all required parameters
        // TODO BDDConstraintVisitor Visitor(Manager, VarMap, Bdd, false, false);
        // TODO int Cnt = 0;
       // Lambda function to process a single constraint
        //TODO const auto Process = [&](const auto& Constraint) {
            //TODO oxidd::bdd_function ConstraintBDD = Visitor.addConstraint(Constraint->constraint());
            //TODO if (ConstraintBDD.is_invalid()) {
                //TODOstd::cerr << "Warning: Failed to process constraint. Skipping.\n";
                //TODOreturn false;
            //TODO}
            //TODOBdd = Bdd & ConstraintBDD; // Add constraint to main BDD
            //TODOif(Cnt > 725) {
                //TODOstd::string_view DiagramNameMid = "VisitorMid"; 
                //TODOstd::vector<oxidd::bdd_function> FuncMid = {Bdd};
                //TODOauto ResultMid = Manager.visualize(DiagramNameMid, FuncMid);
            //TODO}
           //TODO return true;
        //TODO};

        //TODOstd::string_view DiagramName = "HIPPACC"; 
        //TODOstd::vector<oxidd::bdd_function> Func = {Bdd};
        // Process only Boolean constraints (others are not supported)
        //TODOfor (const auto& C : Model.booleanConstraints()) {
            //TODOCnt++;
            //TODOif(Cnt > 735) {
                //TODOauto Result = Manager.visualize(DiagramName, Func);
            //TODO}
            // auto Result = Manager.visualize(DiagramName, Func);
            //TODOif (!Process(C)) { break; }
        //TODO}
    //TODO}

    // template bool bdd::sample::processConstraints<vara::feature::FeatureModel::BooleanConstraint*>(
    // bdd::sample::BDDConstraintVisitor&,
    // vara::feature::FeatureModel::BooleanConstraint* const&,
    // oxidd::bdd_function&);

} // namespace bdd::sample