#include "BDDFactory.h"
#include "CoverageIntegration.h"
#include "CoverageMetrics.h"

#include <cstddef>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "TWiseSampling.h"


namespace twise {

namespace {

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

    size_t NodeVar = VarOpt.value();

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
            size_t FeatureIndex = FeatureIndices[I];
            bool Value = ((Mask >> I) & size_t{1}) != 0;
            Interaction.Literals[FeatureNames[FeatureIndex]] = Value;
        }

        if (isInteractionSatisfiable(Interaction, BDD, FeatureToBddVar)) {
            ValidInteractions.insert(Interaction);
        }
    }
}

} // namespace

FeatureMaps buildFeatureMaps(const vara::feature::FeatureModel& FeatureModel) {
    FeatureMaps Maps;

    size_t Idx = 0;
    for (auto* Feature : FeatureModel.features()) {
        std::string Name = Feature->getName().str();
        Maps.NameToIndex[Name] = Idx;
        Maps.IndexToName[Idx] = Name;
        ++Idx;
    }

    return Maps;
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

std::set<coverage::Interaction> generateValidInteractions(
    size_t T,
    const vara::feature::FeatureModel& FeatureModel,
    const oxidd::bdd_function& BDD,
    const std::map<std::string, oxidd::bdd_function>& FeatureToBddVar
) {
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
            (void)TotalCombination;
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

CoveredIdsList precomputeCoveredIdsPerConfig(
    const std::vector<Configuration>& AllConfigs,
    const std::vector<coverage::Interaction>& CandidateList,
    const FeatureMaps& FeatureMap
) {
    CoveredIdsList Result;
    Result.reserve(AllConfigs.size());

    for (const Configuration& Config : AllConfigs) {
        std::vector<CandidateId> CoveredIds;

        for (size_t Id = 0; Id < CandidateList.size(); ++Id) {
            if (configurationCoversInteraction(Config, CandidateList[Id], FeatureMap)) {
                CoveredIds.push_back(Id);
            }
        }

        Result.push_back(std::move(CoveredIds));
    }

    return Result;
}

std::vector<size_t> greedyTWiseSamplingWithIds(
    const std::vector<Configuration>& AllConfigs,
    const CoveredIdsList& CoveredIdsPerConfig,
    size_t NumCandidates
) {
    std::vector<bool> Uncovered(NumCandidates, true);
    std::vector<bool> AlreadyChosen(AllConfigs.size(), false);

    size_t RemainingCount = NumCandidates;
    std::vector<size_t> SelectedIndices;

    while (RemainingCount > 0) {
        size_t BestIdx = static_cast<size_t>(-1);
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
        SelectedIndices.push_back(BestIdx);

        for (size_t Id : CoveredIdsPerConfig[BestIdx]) {
            if (Uncovered[Id]) {
                Uncovered[Id] = false;
                --RemainingCount;
            }
        }
    }

    return SelectedIndices;
}

} // namespace twise