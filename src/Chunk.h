#pragma once

// =============================================================================
//  Chunk.h
// =============================================================================

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

namespace cvm {

struct Obj;

enum OpCode : std::uint8_t {
    OP_CONST_INT,
    OP_CONST_STR,
    OP_TRUE,
    OP_FALSE,
    OP_NULL,
    OP_POP,

    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_NEG,
    OP_NOT,       // Logical NOT

    OP_EQ,
    OP_NE,
    OP_LT,

    OP_DEFINE_GLOBAL,
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,

    OP_GET_LOCAL,
    OP_SET_LOCAL,

    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,

    OP_PRINT,
    OP_INPUT,

    OP_BUILD_ARRAY,
    OP_BUILD_MAP,
    OP_INDEX_GET,
    OP_INDEX_SET,
    OP_LEN,
    OP_HAS_KEY,

    OP_CLOSURE,
    OP_CALL,
    OP_RETURN,

    OP_HALT,
};

struct Chunk {
    std::vector<std::uint8_t> code;
    std::vector<std::string>  names;
    std::vector<int>          lines;
    std::vector<std::shared_ptr<cvm::Obj>> constants;

    void writeByte(std::uint8_t b, int line) {
        code.push_back(b);
        lines.push_back(line);
    }
    void writeShort(std::uint16_t v, int line) {
        writeByte(static_cast<std::uint8_t>(v & 0xFF), line);
        writeByte(static_cast<std::uint8_t>((v >> 8) & 0xFF), line);
    }
    void writeInt32(std::int32_t v, int line) {
        std::uint32_t u = static_cast<std::uint32_t>(v);
        writeByte(static_cast<std::uint8_t>(u & 0xFF), line);
        writeByte(static_cast<std::uint8_t>((u >>  8) & 0xFF), line);
        writeByte(static_cast<std::uint8_t>((u >> 16) & 0xFF), line);
        writeByte(static_cast<std::uint8_t>((u >> 24) & 0xFF), line);
    }
    std::uint8_t addName(const std::string& n) {
        for (std::size_t i = 0; i < names.size(); ++i) {
            if (names[i] == n) return static_cast<std::uint8_t>(i);
        }
        names.push_back(n);
        return static_cast<std::uint8_t>(names.size() - 1);
    }
    std::uint8_t addConstant(std::shared_ptr<cvm::Obj> o) {
        constants.push_back(std::move(o));
        return static_cast<std::uint8_t>(constants.size() - 1);
    }
    std::size_t size() const { return code.size(); }
};

} // namespace cvm
