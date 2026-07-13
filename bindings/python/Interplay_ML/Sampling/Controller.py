import argparse, subprocess, csv
from pathlib import Path
from .PythonSampler import variant_sampler
from .Path import Workspace, Bin, Samples, Sizes, SatCounts

Systems = ["7z", 
           "BerkeleyDBC", 
           "Dune", 
           "Hippacc", 
           "Irzip", 
           "JavaGC", "LLVM", "Polly", "VP9", "x264"
           ]
TSize = [1, 2, 3]
Proportions = [0.05, 0.1, 0.3, 0.5]
Runs = 100

#---------------- Functions to call C++ main.cpp ----------------#‚
def cpp_tsizes(system_path, tsize):
    r = subprocess.run([str(Bin), system_path, "tsizes", str(tsize)], capture_output=True, text=True)
    if r.returncode != 0:
        raise Exception(f"Error getting t size {tsize} for sampling system {system_path}: {r.stderr}\n")
    breakpoint()
    return int(r.stdout.strip().split("\n")[-1])

def cpp_sample(system_path, strategy, t=None, sample_size=None, seed=None):
    args = [str(Bin), system_path, "sample", strategy]
    args += [str(t)] if strategy == "twise" else [str(sample_size), str(seed)]
    r = subprocess.run(args, capture_output=True, text=True)
    if r.returncode != 0:
        raise Exception(f"Error sampling system {system_path} with strategy {strategy}: {r.stderr}\n")
    
def cpp_sat_count(system_path):
    r = subprocess.run([str(Bin), system_path, "sat_count"], capture_output=True, text=True)
    if r.returncode != 0:
        raise Exception(f"Error getting sat count for system {system_path}: {r.stderr}\n")
    return int(r.stdout.strip().split("\n")[-1])

#---------------- Functions to precompute tsizes and to compute valid configs ----------------#
def writeTSizes():
    rows = []
    for Sys in Systems:
        system_path = Workspace / f"Random_Sampler/examples/{Sys}.xml"
        for t in TSize:
            rows.append({"System": Sys, "T": t, "Sample Size": cpp_tsizes(system_path, t)})
    with open(Sizes, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=["System", "T", "Sample Size"])
        writer.writeheader()
        writer.writerows(rows)

def getSampleSize():
    if not Sizes.exists():
        raise Exception(f"Sample sizes file {Sizes} does not exist. Please run writeTSizes() first.")
    with open(Sizes) as f: 
        return {(row["System"], int(row["T"])): int(row["Sample Size"]) for row in csv.DictReader(f)}

def writeValidConfifgs():
    rows = []
    for Sys in Systems:
        system_path = Workspace / f"Random_Sampler/examples/{Sys}.xml"
        sat_count = cpp_sat_count(system_path)
        rows.append({"System": Sys, "Sat Count": sat_count})
    with open(SatCounts, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=["System", "Sat Count"])
        writer.writeheader()
        writer.writerows(rows)

def getValidConifgs():
    if not SatCounts.exists():
        raise Exception(f"Valid configs file {SatCounts} does not exist. Please run writeValidConfifgs() first.")
    with open(SatCounts) as f: 
        return {row["System"]: int(row["Sat Count"]) for row in csv.DictReader(f)}

#---------------- Function for sampling ----------------#
def run(rq, system, strategy, t=None, prop=None):
    if not Sizes.exists():
        writeTSizes()
    if not SatCounts.exists():
        writeValidConfifgs()
    system_path = Workspace / f"Random_Sampler/examples/{system}.xml"
    if rq == 1:
        sample_size = getSampleSize()[(system, t)]
    else:
        sample_size = round(prop * getValidConifgs()[system])
    
    if strategy == "twise":
        cpp_sample(system_path, "twise", t=t)
    elif strategy == "sbs":
        variant_sampler(system_path=system_path, strategy=strategy, sample_size=sample_size)
    else:
        for r in range(1, Runs + 1):
            if strategy == "random":
                cpp_sample(system_path, "random", sample_size=sample_size, seed=r)
            else:
                variant_sampler(system_path=system_path, strategy=strategy, sample_size=sample_size, seed=r)

#---------------- Main ----------------#
if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser_2 = parser.add_subparsers(dest="command", required=True)
    run_parser = parser_2.add_parser("run")
    run_parser.add_argument("rq", type=int, choices=[1, 2])
    run_parser.add_argument("strategy", choices=["twise", "random", "sbs", "rsbs", "dbs", "ddbs"])
    run_parser.add_argument("system")
    run_parser.add_argument("--t", type=int)
    run_parser.add_argument("--prop", type=float)
    args = parser.parse_args()

    run(args.rq, args.system, args.strategy, t=args.t, prop=args.prop)

        