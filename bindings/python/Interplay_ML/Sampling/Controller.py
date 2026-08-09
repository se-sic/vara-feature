import argparse, subprocess, csv
from .PythonSampler import variant_sampler
from .Config import Workspace, Bin, Sizes, SatCounts, Samples, Systems, TSize, Runs, Proportions

#---------------- Functions to call C++ main.cpp ----------------#‚
def cpp_tsizes(system_path, tsize):
    r = subprocess.run([str(Bin), system_path, "tsizes", str(tsize)], capture_output=True, text=True)
    if r.returncode != 0:
        raise Exception(f"Error getting t size {tsize} for sampling system {system_path}: {r.stderr}\n")
    return int(r.stdout.strip().split("\n")[-1])

def cpp_sample(system_path, strategy, rq, t, sample_size=None, seed=None, prop=None):
    args = [str(Bin), system_path, "sample", strategy]
    if strategy == "twise":
        args += [str(t if t is not None else 0)] 
    else:
        prop_full = f"{int(prop*100)}" if prop is not None else "NA"
        args += [str(t), str(sample_size), str(seed), str(rq), prop_full]
    r = subprocess.run(args, capture_output=True, text=True)
    if r.returncode != 0:
        raise Exception(f"Error sampling system {system_path} with strategy {strategy}: {r.stderr}\n")
    
def cpp_sat_count(system_path):
    r = subprocess.run([str(Bin), system_path, "sat_count"], capture_output=True, text=True)
    if r.returncode != 0:
        raise Exception(f"Error getting sat count for system {system_path}: {r.stderr}\n")
    return int(r.stdout.strip().split("\n")[-1])

def tuning_samples(system):
    if not Sizes.exists():
        raise Exception("first generate twise reference sizes")
    system_path =  Workspace / f"Random_Sampler/examples/{system}.xml"
    for t in TSize:
        size = getSampleSize()[(system, t)]
        cpp_sample(system_path, "random", 1, t, size, 0)
    for p in Proportions:
        size = round(p * getValidConifgs()[system])
        cpp_sample(system_path, "random", 2, t= None, sample_size=size, seed=0, prop=p)

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
            for ts in TSize:
                writeTSizes(system=Sys, t=ts)
        mergeTSizes()
    if not SatCounts.exists():
        writeValidConfifgs()
    system_path = Workspace / f"Random_Sampler/examples/{system}.xml"
    if rq == 1:
        if t is None:
            parser.error("--t is missing")
        sample_size = getSampleSize()[(system, t)]
    else:
        if prop is None:
            parser.error("--prop is missing")
        sample_size = round(prop * getValidConifgs()[system])   
    
    if strategy == "twise":
        cpp_sample(system_path, "twise", rq, t=t)
    elif strategy == "sbs":
        variant_sampler(system_path=system_path, strategy=strategy, sample_size=sample_size, t=t)
    else:
        for r in range(1, Runs + 1):
            if strategy == "random":
                cpp_sample(system_path, "random", rq, t=t ,sample_size=sample_size, seed=r, prop=prop),  # type: ignore
            else:
                variant_sampler(system_path=system_path, strategy=strategy, sample_size=sample_size, seed=r, t=t)

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

    tune_parser = parser_2.add_parser("tune")
    tune_parser.add_argument("system")

    args = parser.parse_args()

    if args.command == "writetsizes":
        writeTSizes(args.system, args.t)
    elif args.command == "mergetsizes":
        mergeTSizes()
    elif args.command == "validconfigs":
        writeValidConfifgs()
    elif args.command == "tune":
        tuning_samples(args.system)
    elif args.command == "run":
        run(args.rq, args.system, args.strategy, t=args.t, prop=args.prop)

        