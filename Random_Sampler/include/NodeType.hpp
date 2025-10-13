#ifndef NODE_TYPE_HPP
#define NODE_TYPE_HPP

#include <cstdint> // For std::uint8_t

enum class NodeType : std::uint8_t { // Changed to use std::uint8_t
    AND,
    OR,
    XOR,
    LEAF
};

#endif // NODE_TYPE_HPP