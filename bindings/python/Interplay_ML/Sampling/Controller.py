import argparse, subprocess, csv
from pathlib import Path
from .PythonSampler import variant_sampler
from .Path import Workspace, Bin, Sizes, SatCounts, Samples

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
def writeTSizes(system, t):
    #rows = []
    #for Sys in Systems:
    #    system_path = Workspace / f"Random_Sampler/examples/{Sys}.xml"
    #    print(f"{Sys}\n")
    #    for t in TSize:
    #        print(f"Running {Sys} on T {t}\n")
    #        rows.append({"System": Sys, "T": t, "Sample Size": cpp_tsizes(system_path, t)})
    Samples.mkdir(parents=True, exist_ok=True)
    system_path = Workspace / f"Random_Sampler/examples/{system}.xml"
    size = cpp_tsizes(system_path=system_path, tsize=t)
    Out = Samples / f"tsize_{system}_T{t}.csv"
    with open(Out, "w", newline="") as f:
       csv.writer(f).writerow([system, t, size])
    print(f"{system}, {t}, {size}")

def mergeTSizes():
    rows = []
    for fileFrag in sorted(Samples.glob("tsize_*.csv")):
        with open(fileFrag, newline="") as f:
            for row in csv.reader(f):
                if row and row[2].strip() != "0":
                    rows.append(tuple(row))
    rows = sorted(set(rows))
    with open(Sizes, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["System","T","Sample Size"])
        w.writerows(rows)
    print(f"Merged {len(rows)} rows -> {Sizes}")
    if(len(rows) != len(Systems) * len(TSize)):
        print(f"Expected {len(Systems) * len(TSize)} rows, got {len(rows)}")

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
        for Sys in Systems: 
            for t in TSize:
                writeTSizes(system=Sys, t=t)
        mergeTSizes()
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

    tparser = parser_2.add_parser("writetsizes")
    tparser.add_argument("system")
    tparser.add_argument("t", type=int)

    mparser = parser_2.add_parser("mergetsizes")
    vparser = parser_2.add_parser("validconfigs")

    args = parser.parse_args()

    if args.command == "writetsizes":
        writeTSizes(args.system, args.t)
    elif args.command == "mergetsizes":
        mergeTSizes()
    elif args.command == "validconfigs":
        writeValidConfifgs()
    elif args.command == "run":
        run(args.rq, args.system, args.strategy, t=args.t, prop=args.prop)

        