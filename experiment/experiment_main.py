from pathlib import Path
import subprocess
import pandas as pd


def get_sample_size_from_twise(system_path: Path, source_t: int) -> int:
    t_wise_sampling_csv = Path("t_wise_sampling.csv").resolve()

    strategy_path = Path("/scratch/miec00001/Thesis/vara-feature/build/bin/greedy_twise_sampling").resolve()

    if t_wise_sampling_csv.exists():
        t_wise_sampling_csv.unlink()

    subprocess.run(
        [
            str(strategy_path),
            str(system_path),
            str(source_t),
            str(t_wise_sampling_csv),
        ],
        check=True,
    )

    df = pd.read_csv(t_wise_sampling_csv)
    return int(df["sample_size"].iloc[0])