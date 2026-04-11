from pathlib import Path
import subprocess
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]

PYTHON_BINDINGS = PROJECT_ROOT / "bindings" / "python"
BUILD_BINDINGS = PROJECT_ROOT / "build" / "bindings" / "python" / "vara-feature"

sys.path.insert(0, str(PYTHON_BINDINGS))
sys.path.insert(0, str(BUILD_BINDINGS))

from ml.sampling.variant_generator import generate_variants, export_configurations_to_csv
from ml.script.data_helper import load_feature_model_and_extract_names

def generate_and_evaluate_sample(
    system_path: Path,
    system_name: str,
    strategy: str,
    source_t: int,
    sample_size: int,
    run: int,
    output_csv: Path,
) -> None:

    feature_model, features, _ = load_feature_model_and_extract_names(str(system_path))

    sampled_configurations = generate_variants(
        feature_model=feature_model,
        features_to_consider=features,
        strategy=strategy,
        sample_size=sample_size,
        seed=run,
        distances=None,
    )

    tmp_dir = (PROJECT_ROOT / "experiment" / "tmp_samples")
    tmp_dir.mkdir(exist_ok=True)

    sample_csv = tmp_dir / f"{system_name}_{strategy}_t{source_t}_run{run}.csv"

    export_configurations_to_csv(
        configurations=sampled_configurations,
        features=features,
        file_path=str(sample_csv),
    )

    evaluator_path = (PROJECT_ROOT / "build" / "bin" / "evaluate_sample_from_csv").resolve()

    subprocess.run(
        [
            str(evaluator_path),
            str(system_path),   # feature model xml
            str(sample_csv),    # sampled configurations csv
            str(strategy),      # random / solver / distance
            str(source_t),      # sample_size_source_t
            str(sample_size),   # sample_size
            str(run),           # repetition number
            str(output_csv),    # shared results.csv
        ],
        check=True,
    )