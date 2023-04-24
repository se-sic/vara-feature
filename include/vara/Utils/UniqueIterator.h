#ifndef VARA_UTILS_UNIQUEITERATOR_H
#define VARA_UTILS_UNIQUEITERATOR_H

#include <type_traits>

namespace vara {

//===----------------------------------------------------------------------===//
//                            UniqueIterator Class
//===----------------------------------------------------------------------===//

template <typename ContainerTy,
          typename ContainedTy = typename ContainerTy::value_type::element_type,
          typename IteratorTy =
              typename std::conditional<std::is_const<ContainerTy>::value,
                                        typename ContainerTy::const_iterator,
                                        typename ContainerTy::iterator>::type>
class UniqueIterator {
public:
  UniqueIterator(IteratorTy Iterator) : Iterator{Iterator} {}
  UniqueIterator(const UniqueIterator &) = default;
  UniqueIterator &operator=(const UniqueIterator &) = delete;
  UniqueIterator(UniqueIterator &&) noexcept = default;
  UniqueIterator &operator=(UniqueIterator &&) = delete;
  ~UniqueIterator() = default;

  ContainedTy *operator*() const { return Iterator->get(); }

  ContainedTy *operator->() const { return operator*(); }

  UniqueIterator operator++() {
    ++Iterator;
    return *this;
  }

  UniqueIterator operator++(int) {
    auto Iter(*this);
    ++*this;
    return Iter;
  }

  bool operator==(const UniqueIterator &Other) const {
    return Iterator == Other.Iterator;
  }

  bool operator!=(const UniqueIterator &Other) const {
    return !(*this == Other);
  }

private:
  IteratorTy Iterator;
};

} // namespace vara

#endif // VARA_UTILS_UNIQUEITERATOR_H
