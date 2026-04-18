from pathlib import Path
from enum import Enum

systems = [
    (Path("../Random_Sampler/examples/FeatureModel/7z.xml"), "7z"),
    (Path("../Random_Sampler/examples/FeatureModel/BerkeleyDBC.xml"), "BerkeleyDBC"),
    (Path("../Random_Sampler/examples/FeatureModel/Dune.xml"), "Dune"),
    (Path("../Random_Sampler/examples/FeatureModel/Hipacc.xml"), "Hipacc"),
    (Path("../Random_Sampler/examples/FeatureModel/JavaGC.xml"), "JavaGC"),
    (Path("../Random_Sampler/examples/FeatureModel/LLVM.xml"), "LLVM"),
    (Path("../Random_Sampler/examples/FeatureModel/lrzip.xml"), "lrzip"),
    (Path("../Random_Sampler/examples/FeatureModel/Polly.xml"), "Polly"),
    (Path("../Random_Sampler/examples/FeatureModel/VP9.xml"), "VP9"),
    (Path("../Random_Sampler/examples/FeatureModel/x264.xml"), "x264"),

    (Path("../Random_Sampler/examples/FeatureModel/AJStats.xml"), "AJStats"),
    (Path("../Random_Sampler/examples/FeatureModel/Curl.xml"), "Curl"),
    (Path("../Random_Sampler/examples/FeatureModel/HSMGP.xml"), "HSMGP"),
    (Path("../Random_Sampler/examples/FeatureModel/HSQLDB.xml"), "HSQLDB"),
    (Path("../Random_Sampler/examples/FeatureModel/HyTeG.xml"), "HyTeG"),
    (Path("../Random_Sampler/examples/FeatureModel/PKJab.xml"), "PKJab"),
    (Path("../Random_Sampler/examples/FeatureModel/SQLite.xml"), "SQLite"),
    (Path("../Random_Sampler/examples/FeatureModel/TriMesh.xml"), "TriMesh"),
    (Path("../Random_Sampler/examples/FeatureModel/WGet.xml"), "WGet"),
    (Path("../Random_Sampler/examples/FeatureModel/clasp.xml"), "clasp"),
    (Path("../Random_Sampler/examples/FeatureModel/z3.xml"), "z3"),
]

class SamplingStrategy(Enum):
    RANDOM = "random"
    DISTANCE = "distance"
    SOLVER = "solver"

class Metrics(Enum):
    MDAP = "MDAP_MF-DF-ALS-PCI"
    MDA = "MDA_MF-DF-ALS"
    MDP = "MDP_MF-DF-PCI"
    MD = "MD_MF-DF"
    AP = "AP_ALS-PCI"
    PCI = "PCI"
    ALS = "ALS"
    DEFAULT = "Default"

class Strength(Enum):
    VERYWEAK = " Very Weak"
    WEAK = "Weak"
    MODERATE = "Moderate"
    STRONG = "Strong"
    VERYSTRONG = "Very Strong"

class EffectStrength(Enum):
    NEGLIGIBLE = "Negligible"
    SMALL = "Small"
    MEDIUM = "Medium"
    LARGER = "Large"

class Relations(Enum):
   DECREASING = "decreasing_with_constraint"
   INCREASING = "increasing_with_constraint"
   NONMONOTONIC = "non_monotonic"

SamplingStrategies = [
    SamplingStrategy.RANDOM,
    SamplingStrategy.DISTANCE,
    SamplingStrategy.SOLVER,
]

def effect_strength(eps_sq: float) -> EffectStrength:
    if (eps_sq >= 0.14):
        return EffectStrength.LARGER
    if (eps_sq >= 0.06):
        return EffectStrength.MEDIUM
    if (eps_sq >= 0.01):
        return EffectStrength.SMALL
    return EffectStrength.NEGLIGIBLE

def kruskal_epsilon_squared(h_stat: float, n: int, k: int) -> float:
    if (n <= k):
        return 0.0
    return max(0.0, (h_stat - k + 1) / (n - k))


def relation_label(p: float | None, left_name: str, left_med: float, right_name: str, right_med: float) -> str:
    if p is None or p >= 0.05:
        return f"{left_name} ~ {right_name}"
    if left_med > right_med:
        return f"{left_name} > {right_name}"
    if right_med > left_med:
        return f"{right_name} > {left_name}"
    return f"{left_name} ~ {right_name}"