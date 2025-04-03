grammar UVLcpp;
import UVLBase;

options { tokenVocab=UVLcppLexer; }  // Use the lexer tokens
@members {
    int opened = 0;
}

startRule: INCLUDE IDENTIFIER ';';

OPEN_PAREN : '(' {this->opened += 1;};
CLOSE_PAREN : ')' {this->opened -= 1;};
OPEN_BRACK : '[' {this->opened += 1;};
CLOSE_BRACK : ']' {this->opened -= 1;};
OPEN_BRACE : '{' {this->opened += 1;};
CLOSE_BRACE : '}' {this->opened -= 1;};
OPEN_COMMENT: '/*' {this->opened += 1;};
CLOSE_COMMENT: '*/' {this->opened -= 1;};

