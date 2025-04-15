lexer grammar UVLcppLexer;

INCLUDE: 'include';
IMPORT: 'import';

IDENTIFIER: [a-zA-Z_][a-zA-Z0-9_]*;

COLON   : ':' ;
DASH    : '-' ;
LCURLY  : '{' ;
RCURLY  : '}' ;
LPAREN  : '(' ;
RPAREN  : ')' ;

ID      : [a-zA-Z_][a-zA-Z_0-9]* ;
WS      : [ \t\r]+ -> skip ;
