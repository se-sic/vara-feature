#include "vara/Feature/FeatureModel.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Errc.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Program.h"

static llvm::cl::OptionCategory
    FMViewerCategory("Feature model viewer options");

static llvm::cl::list<std::string> FileNames(llvm::cl::Positional,
                                             llvm::cl::desc("file"),
                                             llvm::cl::cat(FMViewerCategory));

static llvm::cl::opt<bool> Xml("xml",
                               llvm::cl::desc("Use XML format (default)."),
                               llvm::cl::init(true),
                               llvm::cl::cat(FMViewerCategory));

static llvm::cl::opt<bool> Verify("verify",
                                  llvm::cl::desc("Verify input and exit."),
                                  llvm::cl::init(false),
                                  llvm::cl::cat(FMViewerCategory));

static llvm::cl::opt<std::string>
    Viewer("viewer", llvm::cl::desc("Use <executable> to view DOT file."),
           llvm::cl::value_desc("executable"), llvm::cl::init(""),
           llvm::cl::cat(FMViewerCategory));

static llvm::cl::opt<std::string>
    Out("o", llvm::cl::desc("Place DOT code into <file>, disables viewer."),
        llvm::cl::value_desc("file"), llvm::cl::init(""),
        llvm::cl::cat(FMViewerCategory));

static llvm::cl::opt<bool>
    Dump("dump", llvm::cl::desc("Dump feature model to stdout and exit."),
         llvm::cl::init(false), llvm::cl::cat(FMViewerCategory));

int main(int Argc, char **Argv) {
  llvm::InitLLVM X(Argc, Argv);
  llvm::cl::HideUnrelatedOptions(FMViewerCategory);

  const char *FlagsEnvVar = "FM_VIEWER_FLAGS";
  const char *Overview = R"(View feature model as graph.)";

  llvm::cl::ParseCommandLineOptions(Argc, Argv, Overview, nullptr, FlagsEnvVar);
  if (FileNames.size() != 1) {
    llvm::errs() << "error: Expected single file.\n";
    return 1;
  }

  if (Verify && !vara::feature::verifyFeatureModel(FileNames[0])) {
    llvm::errs() << "error: Invalid feature model.\n";
    return 1;
  }

  std::unique_ptr<vara::feature::FeatureModel> FM =
      vara::feature::loadFeatureModel(FileNames[0]);

  if (!FM) {
    llvm::errs() << "error: Could not build feature model.\n";
    return 1;
  }

  if (Dump) {
    FM->dump();
  } else if (!Out.empty()) {
    llvm::errs() << "Writing '" << Out << "'...";
    llvm::WriteGraph(FM.get(), llvm::Twine(FM->getName()), false, "",
                     Out.getValue());
  } else {
    auto Filename = llvm::WriteGraph(FM.get(), llvm::Twine(FM->getName()));

    if (Filename.empty()) {
      return 1;
    }

    if (llvm::ErrorOr<std::string> P =
            Viewer.empty() ? llvm::errc::invalid_argument
                           : llvm::sys::findProgramByName(Viewer)) {
      llvm::errs() << "Trying '" << *P << "' program... \n";
      llvm::sys::ExecuteNoWait(*P, {*P, Filename}, std::nullopt);
    } else {
      llvm::DisplayGraph(Filename);
    }

    llvm::errs() << "Remember to erase graph file: \'" << Filename << "\'\n";
  }

  return 0;
}
