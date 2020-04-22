#ifndef VARA_FEATURE_RELATIONSHIP_H
#define VARA_FEATURE_RELATIONSHIP_H

#include "llvm/Support/raw_ostream.h"

#include <sstream>
#include <variant>

namespace vara {

//===----------------------------------------------------------------------===//
//                               Relationship Class
//===----------------------------------------------------------------------===//

template <typename T> class Relationship {
protected:
  std::variant<T *, std::vector<T *>> F;

  explicit Relationship(std::variant<T *, std::vector<T *>> F)
      : F(std::move(F)) {}

public:
  [[nodiscard]] virtual std::string toString() const = 0;

  LLVM_DUMP_METHOD
  void dump() const { llvm::outs() << toString() << "\n"; }

  virtual ~Relationship() = default;
};

template <typename T> class Mandatory : public Relationship<T> {
public:
  explicit Mandatory(T *F) : Relationship<T>(F) {}

  [[nodiscard]] std::string toString() const override {
    return "mandatory " + std::get<T *>(this->F)->getName();
  }
};

template <typename T> class Optional : public Relationship<T> {
public:
  explicit Optional(T *F) : Relationship<T>(F) {}

  [[nodiscard]] std::string toString() const override {
    return "optional " + std::get<T *>(this->F)->getName();
  }
};

template <typename T> class Or : public Relationship<T> {
public:
  explicit Or(std::vector<T *> F) : Relationship<T>(F) {}

  [[nodiscard]] std::string toString() const override {
    std::stringstream StrS;
    StrS << "alternatives ";
    for (auto *C : std::get<std::vector<T *>>(this->F)) {
      StrS << C->getName();
      if (C != std::get<std::vector<T *>>(this->F).back()) {
        StrS << ", ";
      }
    }
    return StrS.str();
  }
};

template <typename T> class Xor : public Relationship<T> {
public:
  explicit Xor(std::vector<T *> F) : Relationship<T>(F) {}

  [[nodiscard]] std::string toString() const override {
    std::stringstream StrS;
    StrS << "xor ";
    for (auto *C : std::get<std::vector<T *>>(this->F)) {
      StrS << C->getName();
      if (C != std::get<std::vector<T *>>(this->F).back()) {
        StrS << ", ";
      }
    }
    return StrS.str();
  }
};

} // namespace vara

#endif // VARA_FEATURE_RELATIONSHIP_H
