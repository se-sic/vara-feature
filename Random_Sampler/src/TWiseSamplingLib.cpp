#include "BDDFactory.h"
#include "TWiseSamplingLib.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace twise {

namespace {

std::string getConfigCachePath(const std::string& SystemName) {
    std::filesystem::create_directories("cached_configs");
    return "cached_configs/" + SystemName + "_all_configs.csv";
}

std::string getCoveredIdsCachePath(const std::string& SystemName, size_t T) {
    std::filesystem::create_directories("cached_covered_ids");
    return "cached_covered_ids/" + SystemName + "_t" + std::to_string(T) + "_covered_ids.bin";
}

void saveConfigurationsToCsv(
    const std::string& Path,
    const std::vector<Configuration>& Configs,
    const FeatureMaps& FeatureMap
) {
    std::ofstream Out(Path);
    if (!Out) {
        throw std::runtime_error("Could not open configuration cache for writing: " + Path);
    }

    for (size_t I = 0; I < FeatureMap.IndexToName.size(); ++I) {
        Out << FeatureMap.IndexToName.at(I);
        if (I + 1 < FeatureMap.IndexToName.size()) {
            Out << ",";
        }
    }
    Out << "\n";

    for (const auto& Config : Configs) {
        for (size_t I = 0; I < Config.size(); ++I) {
            Out << (Config[I] ? 1 : 0);
            if (I + 1 < Config.size()) {
                Out << ",";
            }
        }
        Out << "\n";
    }
}

std::vector<Configuration> loadConfigurationsFromCsv(
    const std::string& Path,
    const FeatureMaps& FeatureMap
) {
    std::ifstream In(Path);
    if (!In) {
        throw std::runtime_error("Could not open configuration cache for reading: " + Path);
    }

    std::vector<Configuration> Configs;
    std::string Line;

    if (!std::getline(In, Line)) {
        throw std::runtime_error("Configuration cache is empty: " + Path);
    }

    {
        std::stringstream HeaderStream(Line);
        std::string Cell;
        std::vector<std::string> HeaderNames;

        while (std::getline(HeaderStream, Cell, ',')) {
            HeaderNames.push_back(Cell);
        }

        if (HeaderNames.size() != FeatureMap.IndexToName.size()) {
            throw std::runtime_error("Configuration cache header size mismatch in: " + Path);
        }

        for (size_t I = 0; I < HeaderNames.size(); ++I) {
            if (HeaderNames[I] != FeatureMap.IndexToName.at(I)) {
                throw std::runtime_error(
                    "Configuration cache header does not match current feature order in: " + Path
                );
            }
        }
    }

    while (std::getline(In, Line)) {
        if (Line.empty()) {
            continue;
        }

        std::stringstream RowStream(Line);
        std::string Cell;
        Configuration Config;

        while (std::getline(RowStream, Cell, ',')) {
            if (Cell == "1") {
                Config.push_back(true);
            } else if (Cell == "0") {
                Config.push_back(false);
            } else {
                throw std::runtime_error("Invalid cell in configuration cache: " + Cell);
            }
        }

        if (Config.size() != FeatureMap.IndexToName.size()) {
            throw std::runtime_error("Configuration width mismatch in cache: " + Path);
        }

        Configs.push_back(std::move(Config));
    }

    return Configs;
}

void expandRemainingVariables(
    size_t CurrentVar,
    size_t NumVars,
    Configuration& Partial,
    std::vector<Configuration>& Out
) {
    if (CurrentVar == NumVars) {
        Out.push_back(Partial);
        return;
    }

    Partial[CurrentVar] = false;
    expandRemainingVariables(CurrentVar + 1, NumVars, Partial, Out);

    Partial[CurrentVar] = true;
    expandRemainingVariables(CurrentVar + 1, NumVars, Partial, Out);
}

void enumerateRec(
    const oxidd::bdd_manager& Manager,
    const oxidd::bdd_function& Node,
    size_t CurrentVar,
    Configuration& Partial,
    std::vector<Configuration>& Out
) {
    const size_t NumVars = Manager.num_vars();

    if (Node.is_invalid()) {
        return;
    }

    if (!Node.satisfiable()) {
        return;
    }

    if (Node.valid()) {
        expandRemainingVariables(CurrentVar, NumVars, Partial, Out);
        return;
    }

    auto VarOpt = Node.node_var();
    if (!VarOpt.has_value()) {
        expandRemainingVariables(CurrentVar, NumVars, Partial, Out);
        return;
    }

    const size_t NodeVar = VarOpt.value();

    if (CurrentVar < NodeVar) {
        Partial[CurrentVar] = false;
        enumerateRec(Manager, Node, CurrentVar + 1, Partial, Out);

        Partial[CurrentVar] = true;
        enumerateRec(Manager, Node, CurrentVar + 1, Partial, Out);
        return;
    }

    Partial[NodeVar] = false;
    auto Low = Node.cofactor_false();
    enumerateRec(Manager, Low, NodeVar + 1, Partial, Out);

    Partial[NodeVar] = true;
    auto High = Node.cofactor_true();
    enumerateRec(Manager, High, NodeVar + 1, Partial, Out);
}

size_t binomialCoefficient(size_t N, size_t K) {
    if (K > N) {
        return 0;
    }
    if (K == 0 || K == N) {
        return 1;
    }

    K = std::min(K, N - K);

    size_t Result = 1;
    for (size_t I = 0; I < K; ++I) {
        Result = Result * (N - I) / (I + 1);
    }
    return Result;
}

bool isInteractionSatisfiable(
    const coverage::Interaction& Interaction,
    const oxidd::bdd_function& BDD,
    const std::map<std::string, oxidd::bdd_function>& FeatureToBddVar
) {
    oxidd::bdd_function Constrained = BDD;

    if (Constrained.is_invalid()) {
        return false;
    }

    for (const auto& [FeatureName, Value] : Interaction.Literals) {
        auto It = FeatureToBddVar.find(FeatureName);
        if (It == FeatureToBddVar.end()) {
            return false;
        }

        oxidd::bdd_function Var = It->second;
        if (Var.is_invalid()) {
            return false;
        }

        oxidd::bdd_function Literal = Value ? Var : ~Var;
        if (Literal.is_invalid()) {
            return false;
        }

        Constrained = Constrained & Literal;
        if (Constrained.is_invalid()) {
            return false;
        }

        if (!Constrained.satisfiable()) {
            return false;
        }
    }

    return Constrained.satisfiable();
}

void checkAllValueAssignments(
    const std::vector<size_t>& FeatureIndices,
    const std::vector<std::string>& FeatureNames,
    std::set<coverage::Interaction>& ValidInteractions,
    const oxidd::bdd_function& BDD,
    const std::map<std::string, oxidd::bdd_function>& FeatureToBddVar
) {
    const size_t T = FeatureIndices.size();

    for (size_t Mask = 0; Mask < (size_t{1} << T); ++Mask) {
        coverage::Interaction Interaction;

        for (size_t I = 0; I < T; ++I) {
            const size_t FeatureIndex = FeatureIndices[I];
            const bool Value = ((Mask >> I) & size_t{1}) != 0;
            Interaction.Literals[FeatureNames[FeatureIndex]] = Value;
        }

        if (isInteractionSatisfiable(Interaction, BDD, FeatureToBddVar)) {
            ValidInteractions.insert(Interaction);
        }
    }
}

void saveCoveredIdsToBinary(
    const std::string& Path,
    const CoveredIdsList& CoveredIdsPerConfig,
    size_t NumCandidates,
    size_t NumConfigs
) {
    std::ofstream Out(Path, std::ios::binary);
    if (!Out) {
        throw std::runtime_error("Could not open covered-ids cache for writing: " + Path);
    }

    Out.write(reinterpret_cast<const char*>(&NumCandidates), sizeof(NumCandidates));
    Out.write(reinterpret_cast<const char*>(&NumConfigs), sizeof(NumConfigs));

    for (const auto& CoveredIds : CoveredIdsPerConfig) {
        const size_t RowSize = CoveredIds.size();
        Out.write(reinterpret_cast<const char*>(&RowSize), sizeof(RowSize));

        if (RowSize > 0) {
            Out.write(
                reinterpret_cast<const char*>(CoveredIds.data()),
                static_cast<std::streamsize>(RowSize * sizeof(CandidateId))
            );
        }
    }

    if (!Out) {
        throw std::runtime_error("Failed while writing covered-ids cache: " + Path);
    }
}

CoveredIdsList loadCoveredIdsFromBinary(
    const std::string& Path,
    size_t ExpectedNumCandidates,
    size_t ExpectedNumConfigs
) {
    std::ifstream In(Path, std::ios::binary);
    if (!In) {
        throw std::runtime_error("Could not open covered-ids cache for reading: " + Path);
    }

    size_t NumCandidates = 0;
    size_t NumConfigs = 0;

    In.read(reinterpret_cast<char*>(&NumCandidates), sizeof(NumCandidates));
    In.read(reinterpret_cast<char*>(&NumConfigs), sizeof(NumConfigs));

    if (!In) {
        throw std::runtime_error("Failed to read covered-ids cache header: " + Path);
    }

    if (NumCandidates != ExpectedNumCandidates) {
        throw std::runtime_error("Covered-ids cache candidate count mismatch in: " + Path);
    }
    if (NumConfigs != ExpectedNumConfigs) {
        throw std::runtime_error("Covered-ids cache config count mismatch in: " + Path);
    }

    CoveredIdsList Result;
    Result.resize(NumConfigs);

    for (size_t I = 0; I < NumConfigs; ++I) {
        size_t RowSize = 0;
        In.read(reinterpret_cast<char*>(&RowSize), sizeof(RowSize));
        if (!In) {
            throw std::runtime_error("Failed to read covered-ids row size in: " + Path);
        }

        Result[I].resize(RowSize);
        if (RowSize > 0) {
            In.read(
                reinterpret_cast<char*>(Result[I].data()),
                static_cast<std::streamsize>(RowSize * sizeof(CandidateId))
            );
            if (!In) {
                throw std::runtime_error("Failed to read covered-ids row data in: " + Path);
            }
        }
    }

    return Result;
}

} // namespace

FeatureMaps buildFeatureMaps(const vara::feature::FeatureModel& FeatureModel) {
    FeatureMaps Maps;

    size_t Idx = 0;
    for (auto* Feature : FeatureModel.features()) {
        const std::string Name = Feature->getName().str();
        Maps.NameToIndex[Name] = Idx;
        Maps.IndexToName[Idx] = Name;
        ++Idx;
    }

    return Maps;
}

std::string baseNameWithoutExtension(const std::string& Path) {
    const size_t SlashPos = Path.find_last_of("/\\");
    const std::string FileName =
        (SlashPos == std::string::npos) ? Path : Path.substr(SlashPos + 1);

    const size_t DotPos = FileName.find_last_of('.');
    if (DotPos == std::string::npos) {
        return FileName;
    }
    return FileName.substr(0, DotPos);
}

std::map<std::string, oxidd::bdd_function> buildFeatureVarMap(
    const vara::feature::FeatureModel& FeatureModel,
    oxidd::bdd_manager& Manager
) {
    std::map<std::string, oxidd::bdd_function> FeatureToBddVar;

    for (auto* Feature : FeatureModel.features()) {
        const std::string Name = Feature->getName().str();

        auto MaybeVar = Manager.name_to_var(Feature->getName());
        if (!MaybeVar) {
            continue;
        }

        auto Var = Manager.var(*MaybeVar);
        if (Var.is_invalid()) {
            continue;
        }

        FeatureToBddVar[Name] = Var;
    }

    return FeatureToBddVar;
}

std::vector<Configuration> enumerateAllConfigurations(
    const oxidd::bdd_manager& Manager,
    const oxidd::bdd_function& Root
) {
    std::vector<Configuration> Out;
    Configuration Partial(Manager.num_vars(), false);
    enumerateRec(Manager, Root, 0, Partial, Out);
    return Out;
}

std::vector<Configuration> loadOrEnumerateConfigurations(
    const std::string& SystemName,
    const FeatureMaps& FeatureMap,
    const oxidd::bdd_manager& Manager,
    const oxidd::bdd_function& FinalBDD
) {
    const std::string CachePath = getConfigCachePath(SystemName);

    if (std::filesystem::exists(CachePath)) {
        std::cout << "Loading cached configurations from " << CachePath << "...\n";
        auto Configs = loadConfigurationsFromCsv(CachePath, FeatureMap);
        std::cout << "Loaded " << Configs.size() << " cached configurations.\n";
        return Configs;
    }

    std::cout << "No configuration cache found. Enumerating all valid configurations...\n";
    auto Configs = enumerateAllConfigurations(Manager, FinalBDD);
    std::cout << "Enumerated " << Configs.size() << " configurations.\n";

    saveConfigurationsToCsv(CachePath, Configs, FeatureMap);
    std::cout << "Saved configuration cache to " << CachePath << ".\n";

    return Configs;
}

std::set<coverage::Interaction> generateValidInteractions(
    size_t T,
    const vara::feature::FeatureModel& FeatureModel,
    const oxidd::bdd_function& BDD,
    const std::map<std::string, oxidd::bdd_function>& FeatureToBddVar
) {
    std::cout << "Generating all valid " << T << "-wise interactions using BDD...\n";

    std::set<coverage::Interaction> ValidInteractions;

    std::vector<std::string> FeatureNames;
    for (auto* Feature : FeatureModel.features()) {
        FeatureNames.push_back(Feature->getName().str());
    }
    const size_t NumberOfFeatures = FeatureNames.size();

    const size_t TotalCombination = binomialCoefficient(NumberOfFeatures, T);
    size_t Processed = 0;

    std::function<void(size_t, std::vector<size_t>&)> GenerateCombos;
    GenerateCombos = [&](size_t Start, std::vector<size_t>& Current) {
        if (Current.size() == T) {
            checkAllValueAssignments(
                Current,
                FeatureNames,
                ValidInteractions,
                BDD,
                FeatureToBddVar
            );

            ++Processed;
            if (Processed % 100 == 0) {
                std::cout << "  Progress: " << Processed << "/"
                          << TotalCombination << " ("
                          << ValidInteractions.size() << " valid)\r"
                          << std::flush;
            }
            return;
        }

        for (size_t I = Start; I < NumberOfFeatures; ++I) {
            Current.push_back(I);
            GenerateCombos(I + 1, Current);
            Current.pop_back();
        }
    };

    std::vector<size_t> Current;
    GenerateCombos(0, Current);

    std::cout << "\nFound " << ValidInteractions.size() << " valid interactions.\n";
    return ValidInteractions;
}

bool configurationCoversInteraction(
    const Configuration& Config,
    const coverage::Interaction& Interaction,
    const FeatureMaps& FeatureMap
) {
    for (const auto& [Name, Selected] : Interaction.Literals) {
        const size_t Position = FeatureMap.NameToIndex.at(Name);
        if (Config.at(Position) != Selected) {
            return false;
        }
    }
    return true;
}

std::vector<size_t> greedyTWiseSamplingWithIdsNaive(
    const std::vector<Configuration>& AllConfigs,
    const CoveredIdsList& CoveredIdsPerConfig,
    size_t NumCandidates
) {
    std::vector<bool> Uncovered(NumCandidates, true);
    std::vector<bool> AlreadyChosen(AllConfigs.size(), false);
    std::vector<size_t> SelectedIdxs;

    size_t RemainingCount = NumCandidates;

    while (RemainingCount > 0) {
        auto BestIdx = static_cast<size_t>(-1);
        size_t BestScore = 0;

        for (size_t I = 0; I < AllConfigs.size(); ++I) {
            if (AlreadyChosen[I]) {
                continue;
            }

            size_t Score = 0;
            for (size_t Id : CoveredIdsPerConfig[I]) {
                if (Uncovered[Id]) {
                    ++Score;
                }
            }

            if (Score > BestScore) {
                BestScore = Score;
                BestIdx = I;
            }
        }

        if (BestIdx == static_cast<size_t>(-1) || BestScore == 0) {
            break;
        }

        AlreadyChosen[BestIdx] = true;
        SelectedIdxs.push_back(BestIdx);

        for (size_t Id : CoveredIdsPerConfig[BestIdx]) {
            if (Uncovered[Id]) {
                Uncovered[Id] = false;
                --RemainingCount;
            }
        }
    }

    return SelectedIdxs;
}


CoveredIdsList precomputeCoveredIdsPerConfig(
    const std::vector<Configuration>& AllConfigs,
    const std::vector<coverage::Interaction>& CandidateList,
    const FeatureMaps& FeatureMap
) {
    CoveredIdsList Result;
    Result.reserve(AllConfigs.size());

    for (size_t ConfigIdx = 0; ConfigIdx < AllConfigs.size(); ++ConfigIdx) {
        const Configuration& Config = AllConfigs[ConfigIdx];
        std::vector<CandidateId> CoveredIds;
        CoveredIds.reserve(CandidateList.size() / 8);

        for (size_t Id = 0; Id < CandidateList.size(); ++Id) {
            const auto& Interaction = CandidateList[Id];
            if (configurationCoversInteraction(Config, Interaction, FeatureMap)) {
                CoveredIds.push_back(Id);
            }
        }

        Result.push_back(std::move(CoveredIds));

        if ((ConfigIdx + 1) % 1000 == 0 || ConfigIdx + 1 == AllConfigs.size()) {
            std::cout << "  Covered-ids progress: " << (ConfigIdx + 1)
                      << "/" << AllConfigs.size() << "\r" << std::flush;
        }
    }

    std::cout << "\n";
    return Result;
}

CoveredIdsList loadOrPrecomputeCoveredIdsPerConfig(
    const std::string& SystemName,
    size_t T,
    const std::vector<Configuration>& AllConfigs,
    const std::vector<coverage::Interaction>& CandidateList,
    const FeatureMaps& FeatureMap
) {
    const std::string CachePath = getCoveredIdsCachePath(SystemName, T);

    if (std::filesystem::exists(CachePath)) {
        std::cout << "Loading covered-ids cache from " << CachePath << "...\n";
        auto CoveredIds = loadCoveredIdsFromBinary(
            CachePath,
            CandidateList.size(),
            AllConfigs.size()
        );
        std::cout << "Loaded covered-ids cache.\n";
        return CoveredIds;
    }

    std::cout << "No covered-ids cache found. Precomputing covered candidate IDs...\n";
    auto CoveredIds = precomputeCoveredIdsPerConfig(
        AllConfigs,
        CandidateList,
        FeatureMap
    );

    saveCoveredIdsToBinary(
        CachePath,
        CoveredIds,
        CandidateList.size(),
        AllConfigs.size()
    );
    std::cout << "Saved covered-ids cache to " << CachePath << ".\n";

    return CoveredIds;
}

std::vector<size_t> greedyTWiseSamplingWithIds(
    const std::vector<Configuration>& AllConfigs,
    const CoveredIdsList& CoveredIdsPerConfig,
    size_t NumCandidates
) {
    struct HeapEntry {
        size_t Score;
        size_t ConfigIdx;

        bool operator<(const HeapEntry& Other) const {
            return Score < Other.Score;
        }
    };

    auto recomputeScore = [&](size_t ConfigIdx, const std::vector<uint8_t>& Uncovered) -> size_t {
        size_t Score = 0;
        for (size_t Id : CoveredIdsPerConfig[ConfigIdx]) {
            if (Uncovered[Id]) {
                ++Score;
            }
        }
        return Score;
    };

    std::vector<uint8_t> Uncovered(NumCandidates, 1);
    std::vector<uint8_t> AlreadyChosen(AllConfigs.size(), 0);
    std::vector<size_t> SelectedIdxs;
    SelectedIdxs.reserve(AllConfigs.size());

    size_t RemainingCount = NumCandidates;

    std::priority_queue<HeapEntry> Heap;
    for (size_t I = 0; I < AllConfigs.size(); ++I) {
        Heap.push(HeapEntry{.Score=CoveredIdsPerConfig[I].size(), .ConfigIdx=I});
    }

    while (RemainingCount > 0 && !Heap.empty()) {
        HeapEntry Top = Heap.top();
        Heap.pop();

        if (AlreadyChosen[Top.ConfigIdx]) {
            continue;
        }

        const size_t TrueScore = recomputeScore(Top.ConfigIdx, Uncovered);

        if (TrueScore == 0) {
            break;
        }

        while (!Heap.empty() && AlreadyChosen[Heap.top().ConfigIdx]) {
            Heap.pop();
        }

        const size_t NextBestUpperBound = Heap.empty() ? 0 : Heap.top().Score;

        if (TrueScore >= NextBestUpperBound) {
            AlreadyChosen[Top.ConfigIdx] = 1;
            SelectedIdxs.push_back(Top.ConfigIdx);

            for (size_t Id : CoveredIdsPerConfig[Top.ConfigIdx]) {
                if (Uncovered[Id]) {
                    Uncovered[Id] = 0;
                    --RemainingCount;
                }
            }
        } else {
            Heap.push(HeapEntry{.Score=TrueScore, .ConfigIdx=Top.ConfigIdx});
        }
    }

    return SelectedIdxs;
}

size_t computeGreedySampleSize(
    const std::string& FilePath,
    size_t SourceT
) {
    auto FeatureModel = vara::feature::loadFeatureModel(FilePath);

    bdd::sample::BDDFactory Factory;
    oxidd::bdd_function FinalBDD = Factory.modelToBdd(*FeatureModel);
    oxidd::bdd_manager Manager = FinalBDD.containing_manager();

    auto FeatureToBddVar = buildFeatureVarMap(*FeatureModel, Manager);
    FeatureMaps FeatureMap = buildFeatureMaps(*FeatureModel);
    const std::string SystemName = baseNameWithoutExtension(FilePath);

    std::vector<Configuration> AllConfigs = loadOrEnumerateConfigurations(
        SystemName,
        FeatureMap,
        Manager,
        FinalBDD
    );

    std::set<coverage::Interaction> Candidates = generateValidInteractions(
        SourceT,
        *FeatureModel,
        FinalBDD,
        FeatureToBddVar
    );

    std::vector<coverage::Interaction> CandidateList(
        Candidates.begin(),
        Candidates.end()
    );

    CoveredIdsList CoveredIdsPerConfig = loadOrPrecomputeCoveredIdsPerConfig(
        SystemName,
        SourceT,
        AllConfigs,
        CandidateList,
        FeatureMap
    );

    std::vector<size_t> SelectedIdxsNaive = greedyTWiseSamplingWithIdsNaive(
        AllConfigs,
        CoveredIdsPerConfig,
        CandidateList.size()
    );

    std::vector<size_t> SelectedIdxsLazy = greedyTWiseSamplingWithIds(
        AllConfigs,
        CoveredIdsPerConfig,
        CandidateList.size()
    );

    std::cout << "Naive size: " << SelectedIdxsNaive.size() << "\n";
    std::cout << "Lazy size: " << SelectedIdxsLazy.size() << "\n";

    if (SelectedIdxsNaive.size() != SelectedIdxsLazy.size()) {
        throw std::runtime_error("Lazy greedy result differs from naive greedy");
    }

    return SelectedIdxsLazy.size();
}

void appendResultsToCsv(
    const std::string& OutputCsv,
    const std::string& SystemName,
    size_t SourceT,
    size_t SampleSize
) {
    bool WriteHeader = false;
    {
        std::ifstream In(OutputCsv);
        WriteHeader = !In.good() || In.peek() == std::ifstream::traits_type::eof();
    }

    std::ofstream Out(OutputCsv, std::ios::app);
    if (!Out) {
        throw std::runtime_error("Could not open output CSV: " + OutputCsv);
    }

    if (WriteHeader) {
        Out << "system,source_t,sample_size\n";
    }

    Out << SystemName << "," << SourceT << "," << SampleSize << "\n";
}

} // namespace twise