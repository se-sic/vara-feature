#ifndef VARA_UTILS_VARIANTUTIL_H
#define VARA_UTILS_VARIANTUTIL_H

#include <cassert>
#include <limits>

//===----------------------------------------------------------------------===//
//                               Overloaded Class
//===----------------------------------------------------------------------===//

/// \brief Overloaded to combine different classes call operators into one
/// overloaded call operator. This is helpfull when working with std::variant
/// and std::visit.
template <class... Ts>
struct Overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

/// \brief Convert an unsigned value to an signed value.
[[nodiscard]] inline int checkedNarrowingSignConversion(unsigned Val) {
  assert(Val <= static_cast<unsigned>(std::numeric_limits<int>::max()) &&
         "Error: value to be narrowed was to large.");
  if (Val > static_cast<unsigned>(std::numeric_limits<int>::max())) {
    return std::numeric_limits<int>::max();
  }
  return static_cast<int>(Val);
}

#endif // VARA_UTILS_VARIANTUTIL_H
