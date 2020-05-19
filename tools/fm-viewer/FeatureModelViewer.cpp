#include "vara/Feature/FeatureModelParser.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Errc.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/Program.h"

#include <fstream>
#include <iostream>

static llvm::cl::OptionCategory
    FMViewerCategory("Feature model viewer options");

static llvm::cl::opt<bool> Xml("xml",
                               llvm::cl::desc("Use XML format (default)."),
                               llvm::cl::init(true),
                               llvm::cl::cat(FMViewerCategory));

static llvm::cl::opt<string>
    Viewer("viewer", llvm::cl::desc("Path to viewer executable."),
           llvm::cl::value_desc("path"), llvm::cl::init(""),
           llvm::cl::cat(FMViewerCategory));

static llvm::cl::opt<bool>
    Dump("dump", llvm::cl::desc("Dump feature model to stdout and exit."),
         llvm::cl::init(false), llvm::cl::cat(FMViewerCategory));

static llvm::cl::list<std::string> FileNames(llvm::cl::Positional,
                                             llvm::cl::desc("<file>"),
                                             llvm::cl::cat(FMViewerCategory));

int main(int Argc, char **Argv) {
  llvm::InitLLVM X(Argc, Argv);

  llvm::cl::HideUnrelatedOptions(FMViewerCategory);

  llvm::cl::ParseCommandLineOptions(Argc, Argv,
                                    "Shows feature model as graph.");

  if (FileNames.size() == 1) {
    std::ifstream DocFile(FileNames[0]);
    if (DocFile.is_open()) {
      std::unique_ptr<vara::feature::FeatureModel> FM;

      if (Xml) {
        std::string DocRaw((std::istreambuf_iterator<char>(DocFile)),
                           std::istreambuf_iterator<char>());
        FM = vara::feature::FeatureModelXmlParser(DocRaw).buildFeatureModel();
      }

      DocFile.close();

      if (FM) {
        if (Dump) {
          FM->dump();
        } else {
          std::string Filename =
              llvm::WriteGraph(FM.get(), llvm::Twine(FM->getName()));
          if (!Filename.empty()) {
            if (llvm::ErrorOr<std::string> P =
                    Viewer.empty() ? llvm::errc::invalid_argument
                                   : llvm::sys::findProgramByName(Viewer)) {
              llvm::errs() << "Trying '" << *P << "' program... \n";
              llvm::sys::ExecuteAndWait(*P, {*P, Filename});
            } else {
              llvm::DisplayGraph(Filename);
            }
            llvm::errs() << "Remember to erase graph file: " << Filename
                         << "\n";
          }
        }
        return 0;
      }
    } else {
      llvm::errs() << "error: Could not open file.\n";
    }
  } else {
    llvm::errs() << "error: Expected single file.\n";
  }
  return 1;
}
