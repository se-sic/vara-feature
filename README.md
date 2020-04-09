# VaRA feature library

Installation
------------
Installing the feature library simple.
First, install the required dependencies:
```bash
sudo apt install llvm-dev
```
Note: We advice to use newer version of LLVM to stay up-to-date with our improvements. If `apt` on your system does not support these out of the box visit [apt.llvm.org](https://apt.llvm.org)

Second, setup the repository and build the `vara-feature` lib.
```bash
git clone https://github.com/se-passau/vara-feature
cd vara-feature
mkdir build && cd build
CC=clang CXX=clang++ cmake ..
make
```


Development
-----------
All VaRA development is coordinated from the main [VaRA]() repository.
So, for issues, please visit the [VaRA bugtracker](https://github.com/se-passau/VaRA/labels/FeatureLibrary) and search for the `FeatureLib` tag.
If you want to reference a issue, please use the full reference `se-passau/VaRA#42`.
