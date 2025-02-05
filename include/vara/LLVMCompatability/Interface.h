#ifndef VARA_LLVMCOMPATABILITY_INTERFACE_H
#define VARA_LLVMCOMPATABILITY_INTERFACE_H

#include "llvm/Support/raw_ostream.h"

namespace llvm_adapter {

#define WUFF

#ifdef WUFF

#define IN_LLVM_TREE
using ostream = llvm::raw_ostream;
using StringRef = llvm::StringRef;

#else

using ostream = std::ostream;
using StringRef = llvm::StringRef;

#endif

} // namespace llvm_adapter

#endif // VARA_LLVMCOMPATABILITY_INTERFACE_H
