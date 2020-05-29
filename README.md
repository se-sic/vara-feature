# VaRA feature library
[![Build Status](https://travis-ci.org/se-passau/vara-feature.svg?branch=vara-dev)](https://travis-ci.org/se-passau/vara-feature) [![Documentation Status](https://readthedocs.org/projects/vara/badge/?version=vara-dev)](https://vara.readthedocs.io/en/vara-dev/?badge=vara-dev)

Installation
------------
Installing the feature library simple.
First, install the required dependencies:
```bash
sudo apt install llvm-dev libxml2-dev graphviz
```
Note: We advice to use newer version of LLVM to stay up-to-date with our improvements. If `apt` on your system does not support these out of the box visit [apt.llvm.org](https://apt.llvm.org).

Please also assure that you have `cmake` and `make` installed on your system. Otherwise, the build process will fail.

Second, setup the repository and build the `vara-feature` lib.
```bash
git clone https://github.com/se-passau/vara-feature
git submodule update --init
cd vara-feature
mkdir build && cd build
CC=clang CXX=clang++ cmake ..
cmake --build .
```
If you happen to have an older version of LLVM and `cmake` aborts, you can still use the `LLVM_REQUESTED_VERSION` accordingly.
To set the version to version 9, for instance, you can use the following command:
```bash
CC=clang CXX=clang++ cmake -DLLVM_REQUESTED_VERSION=9 ..
```

Python bindings
---------------
The feature library offers python bindings for easier scripting.
Currently, these bindings need to be installed locally like this.

```bash
  git clone https://github.com/se-passau/vara-feature.git
  cd vara-feature
  git submodule init && git submodule update --recursive
  pip3 install --user .
```


Development
-----------
All VaRA development is coordinated from the main [VaRA]() repository.
So, for issues, please visit the [VaRA bugtracker](https://github.com/se-passau/VaRA/labels/FeatureLibrary) and search for the `FeatureLib` tag.
If you want to reference a issue, please use the full reference `se-passau/VaRA#42`.
