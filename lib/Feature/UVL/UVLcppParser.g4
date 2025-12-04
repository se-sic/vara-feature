parser grammar UVLcppParser;

options {
    tokenVocab = UVLcppLexer;
}

import UVLParser;

@lexer::header {
#include <antlr4-runtime.h>
}
