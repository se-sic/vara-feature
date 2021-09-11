#!/usr/bin/env python3
"""
Helper script to prepare coverage data for upload.
"""

import subprocess
import os
import glob
from shutil import which


def get_llvm_cov_binary_name() -> str:
    """
    Retrieves the llvm-cov binary name for the current system.
    """
    possible_binary_names = ["llvm-cov"]
    possible_binary_names.extend(
        ["llvm-cov-" + str(x) for x in range(14, 7, -1)])

    for binary_name in possible_binary_names:
        if which(binary_name) is not None:
            return binary_name

    raise LookupError("Could not find llvm-cov binary.")


def get_llvm_profdata_binary_name() -> str:
    """
    Retrieves the llvm-profdata binary name for the current system.
    """
    possible_binary_names = ["llvm-profdata"]
    possible_binary_names.extend(
        ["llvm-profdata-" + str(x) for x in range(14, 7, -1)])

    for binary_name in possible_binary_names:
        if which(binary_name) is not None:
            return binary_name

    raise LookupError("Could not find llvm-cov binary.")


def merge_llvm_data():
    """
    Merges together the llvm profile data

    llvm-profdata merge --output=merged.profdata profiles/*
    """
    subprocess_cmd = [
        get_llvm_profdata_binary_name(), "merge", "--output=merged.profdata"
    ]
    subprocess_cmd.extend(glob.glob('profiles/*'))

    subprocess.check_call(subprocess_cmd)


def prepare_clang_coverage_data():
    """
    Prepare clang/llvm profile data.
    """
    binary_paths = [
        "unittests/Feature/VaRAFeatureTests",
        "unittests/Utils/VaRAVariantUtilsTests",
        "bin/fm-viewer",
    ]

    merge_llvm_data()

    subprocess_cmd = [
        get_llvm_cov_binary_name(), "show", "-instr-profile=merged.profdata"
    ]

    subprocess_cmd.extend(
        ['--object=' + str(binary) for binary in binary_paths])

    with open('coverage.txt', 'w', encoding='utf-8') as out_file:
        subprocess.check_call(subprocess_cmd, stdout=out_file)


def prepare_gcc_coverage_data():
    """
    Prepare gcc profile data.
    """
    # everything is handled automatically by codecov upload


def main():
    """
    Run the necessary coverage preparation steps.
    """
    if os.environ['CXX'].startswith("clang"):
        prepare_clang_coverage_data()
    else:
        prepare_gcc_coverage_data()


if __name__ == "__main__":
    main()
