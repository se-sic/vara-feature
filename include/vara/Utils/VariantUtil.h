#ifndef VARA_UTILS_VARIANTUTIL_H
#define VARA_UTILS_VARIANTUTIL_H

//===----------------------------------------------------------------------===//
//                               Overloaded Class
//===----------------------------------------------------------------------===//

/// \brief Overloaded to combine different classes call operators into one
/// overloaded call operator. This is helpfull when working with std::variant
/// and std::visit.
template <class... Ts> struct Overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> Overloaded(Ts...)->Overloaded<Ts...>;

#endif // VARA_UTILS_VARIANTUTIL_H
