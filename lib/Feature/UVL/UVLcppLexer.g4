lexer grammar UVLcppLexer;

import UVLLexer;

@lexer::header {
#include <regex>
}
@lexer::members {
    int opened = 0;
    std::stack<int> indents;

    std::deque<std::unique_ptr<antlr4::Token>> tokens;  //HAVE A QUEUE OF NORMAL POINTERS.
    //std::deque<antlr4:: Token*> tokens;
    std::unique_ptr<antlr4::Token> lastToken;
    //antlr4:: Token  // fine for now as it is.

    int getIndentationCount(const std::string &spaces) {
        int count = 0;
        for (char ch : spaces) {
            if (ch == '\t') count += 8 - (count % 8);
            else count++;
        }
        return count;
    }

    // Override nextToken to manage INDENT/DEDENT logic
    std::unique_ptr<antlr4::Token> nextToken() override {

        if (_input->LA(1) == antlr4::Token::EOF && !indents.empty()) {
            for (int i = tokens.size() - 1; i >= 0 ; i--)
            {
                if (tokens.at(i)->getType() == antlr4::Token::EOF)
                {
                    auto it = std::find(tokens.begin(), tokens.end(), tokens.at(i));
                    tokens.erase(it);
                }
            }
            emit(commonToken(NEWLINE, "\n"));

            while (!indents.empty()) {
                emit(createDedent());
                indents.pop();
            }

           emit(commonToken(antlr4::Token::EOF, "<EOF>"));
        }

        // Get next token from super
        std::unique_ptr<antlr4::Token> next = antlr4::Lexer::nextToken();
        //auto next
        if (next->getChannel() == antlr4::Token::DEFAULT_CHANNEL) {
            lastToken = std::unique_ptr<antlr4::Token>(new antlr4::CommonToken(next.get()));
        }

        if (!tokens.empty()) {
            std::unique_ptr<antlr4::Token> token = std::unique_ptr<antlr4::Token>(new antlr4::CommonToken(tokens.front().get()));
            tokens.pop_front();
            return std::move(token);
        }
        return std::move(next);
    }

    void emit (std::unique_ptr<antlr4::Token> token) override
    {
        tokens.push_back(std::unique_ptr<antlr4::Token>(new antlr4::CommonToken(token.get()))); //CHANGES
        antlr4::Lexer::setToken(std::move(token));
    }

    std::unique_ptr<antlr4::CommonToken> commonToken(std::size_t type, const std::string &text) {
        size_t stop = this->getCharIndex() - 1;
        size_t start = text.empty() ? stop : stop - text.length() + 1;
        /*return this->getTokenFactory()->create(
            {this, _input}, type, text,
            antlr4::Token::DEFAULT_CHANNEL, start,
            stop,
            getLine(), getCharPositionInLine());*/
        auto token = std::make_unique<antlr4::CommonToken>(type,text);
        token->setStartIndex(static_cast<int>(start));
        token->setStopIndex(static_cast<int>(stop));
        token->setChannel(antlr4::Token::DEFAULT_CHANNEL);


        return token;
    }

    std::unique_ptr<antlr4::Token> createDedent() {
        auto ret = commonToken(DEDENT, "");

        ret->setLine(lastToken->getLine());
        return std::move(ret);
    }

    bool atStartOfInput() {
        return Lexer::getCharPositionInLine() == 0 && Lexer::getLine() == 1;//this->_input->index() == 0;
    }
}

OPEN_PAREN : '(' {opened++;};
CLOSE_PAREN : ')' {opened--;};
OPEN_BRACK : '[' {opened++;};
CLOSE_BRACK : ']' {opened--;};
OPEN_BRACE : '{' {opened++;};
CLOSE_BRACE : '}' {opened--;};

OPEN_COMMENT : '/*' {opened++;};
CLOSE_COMMENT : '*/' {opened--;};

SPACES: [ \t]+ -> channel(HIDDEN);
WS: [ \t]+ -> channel(HIDDEN);

// Match newline and defer indentation logic to action



NEWLINE
 : ( {atStartOfInput()}? SPACES
   | ('\r'? '\n' | '\r') SPACES?
   )
   {
   {
     std::string newLineText = getText();
     std::string newline = std::regex_replace(newLineText, std::regex("[^\r\n]+"), "");
     std::string newlines;
     /*for (char c : newLineText) {
       if (c == '\r' || c == '\n') newlines += c;
     }*/

     std::string spaceText = getText();
     std::string spaces = std::regex_replace(spaceText, std::regex("[\r\n]+"), "");
     /*for (char c : newLineText) {
       if (c == ' ' || c == '\t') spaces += c;
     }*/

     std::size_t nextChar = _input->LA(1);
     std::size_t nextNextChar = _input->LA(2);


     if (opened > 0 || nextChar == '\r' || nextChar == '\n' || (nextChar == '/' && nextNextChar == '/')) {
         skip();
     }
     // Emit NEWLINE token first
     else {
     //auto newlineToken = commonToken(NEWLINE, newline);
     //this->setToken(std::move(newlineToken));
     emit(commonToken(NEWLINE, newline));
     int indent = getIndentationCount(spaces);
     int prevIndent = indents.empty() ? 0 : indents.top();


     if (indent == prevIndent) {
         skip();
     }
     else if (indent > prevIndent) {
         indents.push(indent);
         //auto indentToken = commonToken(INDENT, spaces);
         //this->setToken(std::move(indentToken));
         emit(commonToken(INDENT, spaces));
     } else {
         while (!indents.empty() && indents.top() > indent) {
             //auto dedentToken = createDedent();
             //this->setToken(std::move(dedentToken));
             emit(createDedent());
             indents.pop();
         }
     }
     }
   }
   }
 ;
