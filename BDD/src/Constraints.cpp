#include "Constraints.h"
#include <oxidd/bdd.hpp>
#include <oxidd/util.hpp>

namespace bdd::sample {

    oxidd::bdd_function BDDConstraintVisitor::addConstraint(vara::feature::Constraint* C, bool Negate, bool RequireAll) {
        
        // Reset accumulators such that all nested visits within one call share a consistent starting state
        this->RequireAll = RequireAll;
        ExprBDD = Manager->t();
        VariableConstraint = Manager->f();
        if (C->accept(*this)) {
            oxidd::bdd_function FinalExpr = ExprBDD;

            if (Negate) {
                FinalExpr= ~FinalExpr;
            }

            // Fallback for mixed constraints outside of binary ones, for more info see header doc.
            if (IsMixedConstraint && RequireAll) {
                FinalExpr = VariableConstraint | FinalExpr;
            } 
            (*CurrentBDD) = (*CurrentBDD) & FinalExpr;

            return FinalExpr;
        }

        return Manager->f();
    }

    bool BDDConstraintVisitor::visit(vara::feature::BinaryConstraint* C) {
        using CK = vara::feature::Constraint::ConstraintKind;

        C->getLeftOperand()->accept(*this);
        oxidd::bdd_function Left  = ExprBDD;
        
        C->getRightOperand()->accept(*this);
        oxidd::bdd_function Right = ExprBDD;

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

            // Both CK_EQUAL and CK_NOT_EQUAL fall into the numeric feature case group.
            // For boolean opearands we use equivalence/not-equivalence, just as above.
            case CK::CK_LESS:
            case CK::CK_GREATER:
            case CK::CK_LESS_EQUAL:
            case CK::CK_GREATER_EQUAL:
            case CK::CK_EQUAL:
            case CK::CK_NOT_EQUAL:
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

    bool BDDConstraintVisitor::visit(vara::feature::PrimaryFeatureConstraint* C) {
        std::string FeatureName = C->getFeature()->getName().str();
        auto IdCheck = Manager->name_to_var(FeatureName);
        if (!IdCheck.has_value()) {
            std::cerr << "Error: Could not find variable ID for feature '" << FeatureName << "'.\n";
            return false;
        }
        oxidd::var_no_t Id = IdCheck.value();

        if (C->getFeature()->getKind() == vara::feature::Feature::FeatureKind::FK_NUMERIC) {
            std::cerr << "Error: Numeric features are not supported. Feature '" << FeatureName 
                    << "' is numeric. Only binary features are supported.\n";

            // In case of a numeric feature, we immediately return a false BDD, as we
            // do not support this kind of feature in our setup.
            (*CurrentBDD) = Manager->f();
            return false;
        }

        oxidd::bdd_function Var = Manager->var(Id);
        ExprBDD = Var;

        // Fallback for mixed constraints outside of binary ones, for more info see header doc.
        if(IsMixedConstraint) {
            VariableConstraint = VariableConstraint | ~Var;
        }

        return true;
    }

} // namespace bdd::sample