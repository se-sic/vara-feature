#!/usr/bin/env python3

import subprocess
import os
import glob


def merge_llvm_data():
    """
    Merges together the llvm profile data

    llvm-profdata merge --output=merged.profdata profiles/*
    """
    subprocess_cmd = [
        "llvm-profdata-11",
        "merge",
        "--output=merged.profdata"  #, "'profiles/*'"
    ]
    subprocess_cmd.extend(glob.glob('profiles/*'))

    subprocess.check_call(subprocess_cmd)


def upload_clang():
    """
    Upload clang/llvm profile data.
    """
    binary_paths = [
        "unittests/Feature/VaRAFeatureTests",
        "unittests/Utils/VaRAVariantUtilsTests",
    ]

    merge_llvm_data()

    subprocess_cmd = ["llvm-cov-11", "show", "-instr-profile=merged.profdata"]

    subprocess_cmd.extend(
        ['--object=' + str(binary) for binary in binary_paths])

    with open('coverage.txt', 'w', encoding='utf-8') as out_file:
        subprocess.check_call(subprocess_cmd, stdout=out_file)

    # bash <(curl -s https://codecov.io/bash) #-x 'llvm-cov-${{ matrix.llvm-major }} gcov'


def upload_gcc():
    # bash <(curl -s https://codecov.io/bash)
    pass


def main():
    if os.environ['CXX'].startswith("clang"):
        upload_clang()
    else:
        upload_gcc()


if __name__ == "__main__":
    main()
