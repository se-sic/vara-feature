#include "vara/Feature/FeatureModel.h"
#include "vara/Sampling/SampleSetParser.h"
#include "vara/Sampling/SampleSetWriter.h"
#include "vara/Solver/ConfigurationFactory.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Errc.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/MemoryBuffer.h"

static llvm::cl::OptionCategory
    ConfigCreatorCategory("Configuration generator options");

static llvm::cl::list<std::string>
    FileNames(llvm::cl::Positional, llvm::cl::desc("Path to the feature model"),
              llvm::cl::cat(ConfigCreatorCategory));

enum class ConfigurationGenerationChoice : unsigned {
  ALL,
  SAMPLING_STRATEGY,
  SAMPLE_SET,
};

static llvm::cl::opt<ConfigurationGenerationChoice, false>
    ConfigurationGenerationOption(
        "type",
        llvm::cl::desc("The way how the configurations should be generated."),
        llvm::cl::values(
            clEnumValN(ConfigurationGenerationChoice::ALL, "All",
                       "Generate all configurations from feature model."),
            clEnumValN(ConfigurationGenerationChoice::SAMPLING_STRATEGY,
                       "sampling",
                       "Use a sampling strategy to select configurations."),
            clEnumValN(ConfigurationGenerationChoice::SAMPLE_SET, "sample",
                       "Read the sample set from a file.")),
        llvm::cl::init(ConfigurationGenerationChoice::ALL), llvm::cl::Optional,
        llvm::cl::cat(ConfigCreatorCategory));

static llvm::cl::opt<std::string>
    CsvInputFilePath("csv", llvm::cl::desc("Path to the csv input file."),
                     llvm::cl::value_desc("filename"), llvm::cl::init(""),
                     llvm::cl::cat(ConfigCreatorCategory));

static llvm::cl::opt<std::string>
    OutputFilePath("out", llvm::cl::desc("Path to the yml output file."),
                   llvm::cl::value_desc("filename"), llvm::cl::init(""),
                   llvm::cl::cat(ConfigCreatorCategory));

int main(int Argc, char **Argv) {
  const llvm::InitLLVM X(Argc, Argv);
  llvm::cl::HideUnrelatedOptions(ConfigCreatorCategory);

  const char *FlagsEnvVar = "CONFIG_GENERATOR_FLAGS";
  const char *Overview = R"(Generate configurations.)";

  llvm::cl::ParseCommandLineOptions(Argc, Argv, Overview, nullptr, FlagsEnvVar);
  if (FileNames.size() != 1) {
    llvm::errs() << "error: Expected single file.\n";
    return 1;
  }

  if (!vara::feature::verifyFeatureModel(FileNames[0])) {
    llvm::errs() << "error: Invalid feature model.\n";
    return 1;
  }

  std::unique_ptr<vara::feature::FeatureModel> FM =
      vara::feature::loadFeatureModel(FileNames[0]);

  if (!FM) {
    llvm::errs() << "error: Could not build feature model.\n";
    return 1;
  }

  if (ConfigurationGenerationOption.getValue() ==
          ConfigurationGenerationChoice::SAMPLE_SET &&
      CsvInputFilePath.empty()) {
    llvm::errs()
        << "error: Please provide the sample set using the csv argument.\n";
    return 1;
  }

  std::vector<std::unique_ptr<vara::feature::Configuration>> Configurations;
  switch (ConfigurationGenerationOption.getValue()) {
  case ConfigurationGenerationChoice::ALL:
    if (auto R = vara::solver::ConfigurationFactory::getAllConfigs(*FM); R) {
      Configurations = R.extractValue();
    } else {
      llvm::errs() << "error: Error while computing all configurations.\n";
      return 1;
    }
    break;
  case ConfigurationGenerationChoice::SAMPLE_SET:
    Configurations = vara::sampling::SampleSetParser::readConfigurations(
        *FM, CsvInputFilePath);
    break;
  case ConfigurationGenerationChoice::SAMPLING_STRATEGY:
    llvm::errs() << "error: Sampling is not implemented yet.\n";
    return 1;
  }

  if (!OutputFilePath.empty() && !Configurations.empty()) {
    const std::string Str = vara::sampling::SampleSetWriter::writeConfigurations(
        *FM, Configurations);
    std::error_code EC;
    auto *Out = new llvm::raw_fd_ostream(OutputFilePath.getValue(), EC);
    if (EC) {
      llvm::errs() << "error: Error while writing to file.\n";
      return 1;
    }
    *Out << Str;
    delete Out;
  }

  return 0;
}
