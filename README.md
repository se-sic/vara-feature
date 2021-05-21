# VaRA feature library
[![Build Status](https://github.com/se-sic/vara-feature/workflows/VaRA%20Feature%20Library%20Build/badge.svg?branch=vara-dev)](https://github.com/se-sic/vara-feature/actions?query=branch%3Avara-dev) [![Documentation Status](https://readthedocs.org/projects/vara/badge/?version=vara-dev)](https://vara.readthedocs.io/en/vara-dev/?badge=vara-dev)

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
git clone https://github.com/se-sic/vara-feature
cd vara-feature
git submodule init && git submodule update --recursive
mkdir build && cd build
CC=clang CXX=clang++ cmake ..
cmake --build .
```

If you happen to have an older version of LLVM and `cmake` aborts, you can still use the `LLVM_REQUESTED_VERSION` accordingly.
It may be required to specify the minor version of your LLVM.
Adhere to the output of
```bash
  llvm-config --version
```
To set the version to version 10.0, for instance, you can use the following command:
```bash
CC=clang CXX=clang++ cmake -DLLVM_REQUESTED_VERSION=10.0 ..
```


Test Execution
---------------
To execute all unit tests you can use the following command in your build-directory:

```console
cmake --build . --target check-vara-feature-unittests
```

Python bindings
---------------
The feature library offers python bindings for easier scripting.
Currently, these bindings need to be installed locally like this.

```bash
  git clone https://github.com/se-sic/vara-feature.git
  cd vara-feature
  git submodule init && git submodule update --recursive
  pip3 install --user .
```

Development
-----------
All VaRA development is coordinated from the main [VaRA]() repository.
So, for issues, please visit the [VaRA bugtracker](https://github.com/se-passau/VaRA/labels/FeatureLibrary) and search for the `FeatureLib` tag.
If you want to reference a issue, please use the full reference `se-passau/VaRA#42`.

Internal API documentation for vara-feature can be found [here](https://se-sic.github.io/vara-feature/index.html)
