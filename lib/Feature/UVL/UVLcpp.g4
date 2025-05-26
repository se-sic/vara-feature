grammar UVLcpp;
import UVLBase;

@members {
    int opened = 0;
}
NEWLINE : [\r\n]+ -> skip ;
OPEN_PAREN : '(' {this->opened += 1;};
CLOSE_PAREN : ')' {this->opened -= 1;};
OPEN_BRACK : '[' {this->opened += 1;};
CLOSE_BRACK : ']' {this->opened -= 1;};
OPEN_BRACE : '{' {this->opened += 1;};
CLOSE_BRACE : '}' {this->opened -= 1;};
OPEN_COMMENT: '/*' {this->opened += 1;};
CLOSE_COMMENT: '*/' {this->opened -= 1;};

// Parser rules

file        : (name NEWLINE (TAB elements+=action NEWLINE?)* ) EOF;

name        : NAME ':';

action      : DRAW size shape IN color AT position
            | WRITE size STRING IN color AT position
            ;

size        : SMALL | MEDIUM | BIG ;

shape       : CIRCLE | SQUARE;

color       : BLACK | BLUE | BROWN | GREEN | RED | ORANGE | PURPLE | YELLOW | WHITE ;

position    : x=(LEFT | CENTER | RIGHT) ',' y=(TOP | CENTER | BOTTOM) ;

// Lexer rules

DRAW        : 'draw' ;
WRITE       : 'write' ;
IN          : 'in' ;
AT          : 'at';

CIRCLE      : 'circle' ;
SQUARE      : 'square' ;

SMALL       : 'small' ;
MEDIUM      : 'medium' ;
BIG         : 'big' ;

LEFT        : 'left' ;
RIGHT       : 'right' ;
CENTER      : 'center' ;
TOP         : 'top' ;
BOTTOM      : 'bottom' ;

STRING      : '"' .*? '"' ;

BLACK       : 'black' ;
BLUE        : 'blue' ;
BROWN       : 'brown' ;
GREEN       : 'green' ;
RED         : 'red' ;
ORANGE      : 'orange' ;
PURPLE      : 'purple' ;
YELLOW      : 'yellow' ;
WHITE       : 'white' ;

NAME        :  [a-zA-Z0-9\-_]+;

TAB         : ('\t' | '        ' | '    ' ) ;
WHITESPACE  : ' ' -> skip ;
