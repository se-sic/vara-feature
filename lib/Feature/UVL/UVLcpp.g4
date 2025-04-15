grammar UVLcpp;
import UVLBase;

options { tokenVocab=UVLcppLexer; }  // Use the lexer tokens
@members {
    int opened = 0;
}

COLON   : ':' ;
DASH    : '-' ;
LCURLY  : '{' ;
RCURLY  : '}' ;
LPAREN  : '(' ;
RPAREN  : ')' ;
//startRule: INCLUDE IDENTIFIER ';';

startRule
    : section+ EOF ;

section
    : 'features' COLON featureBlock
    | 'constraints' COLON constraintBlock
    ;

featureBlock
    : ID COLON LCURLY featureList RCURLY ;

featureList
    : (DASH ID)+
    ;

constraintBlock
    : (DASH constraint)+ ;

constraint
    : ID LPAREN ID COMMA ID RPAREN
    ;

NEWLINE : [\r\n]+ -> skip ;
OPEN_PAREN : '(' {this->opened += 1;};
CLOSE_PAREN : ')' {this->opened -= 1;};
OPEN_BRACK : '[' {this->opened += 1;};
CLOSE_BRACK : ']' {this->opened -= 1;};
OPEN_BRACE : '{' {this->opened += 1;};
CLOSE_BRACE : '}' {this->opened -= 1;};
OPEN_COMMENT: '/*' {this->opened += 1;};
CLOSE_COMMENT: '*/' {this->opened -= 1;};

