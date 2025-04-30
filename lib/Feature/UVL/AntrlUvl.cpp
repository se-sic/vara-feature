
#ifdef ANTLR_AVAILABLE
#include "UVLcppLexer.h"
#include "antlr4-runtime.h"
#include <fstream>
#include <iostream>
#include <string>

#include "UVLcppParser.h"

using namespace antlr4;
using namespace std;

int main(int argc, char *argv[]) {
  string line;
  ifstream antlrFile("/home/taqi457/Aatir/vara-feature/vara-feature/lib/"
                     "Feature/UVL/testuvl.txt");
  std::cout << "Opening file" << endl;
  if (antlrFile.is_open()) {
    ANTLRInputStream input(antlrFile);
    UVLcppLexer lexer(&input);
    CommonTokenStream tokens(&lexer);

    tokens.fill();
    for (auto token : tokens.getTokens()) {
      std::cout << token->toString() << std::endl;
    }

    // UVLcppParser parser(&tokens);
    // tree::ParseTree *tree = parser.;

    // std::cout << tree->toStringTree(&parser) << std::endl;
    // modelicaFile.close();
  }
}
#endif
