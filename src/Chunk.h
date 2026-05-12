#pragma once

// =============================================================================
//  Chunk.h
//
//  A Chunk is the bytecode container: a flat byte array plus the parallel
//  name table for global-variable opcodes. See docs/ISA.md.
// =============================================================================

#include <cstdint>
#include <string>
#include <vector>

namespace cvm {

// Opcodes — keep in sync with docs/ISA.md and the VM dispatch switch.
enum OpCode : std::uint8_t {
    OP_CONST_INT,        // push int32 (next 4 bytes, little-endian)
    OP_TRUE,
    OP_FALSE,
    OP_POP,

    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_NEG,

    OP_EQ,
    OP_LT,

    OP_DEFINE_GLOBAL,    // 1-byte name index
    OP_GET_GLOBAL,       // 1-byte name index
    OP_SET_GLOBAL,       // 1-byte name index

    OP_JUMP,             // 2-byte forward offset
    OP_JUMP_IF_FALSE,    // 2-byte forward offset
    OP_LOOP,             // 2-byte backward offset

    OP_PRINT,
    OP_INPUT,
    OP_HALT,
};

struct Chunk {
    std::vector<std::uint8_t> code;    // bytecode
    std::vector<std::string>  names;   // global-variable names (indexed by 1-byte operand)
    std::vector<int>          lines;   // source line per byte, for error messages

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
        writeByte(static_cast<std::uint8_t>((u >> 8)  & 0xFF), line);
        writeByte(static_cast<std::uint8_t>((u >> 16) & 0xFF), line);
        writeByte(static_cast<std::uint8_t>((u >> 24) & 0xFF), line);
    }

    // Add a global name to the name table, deduping. Returns the index.
    // Throws if the table would overflow the 1-byte operand.
    std::uint8_t addName(const std::string& n) {
        for (std::size_t i = 0; i < names.size(); ++i) {
            if (names[i] == n) return static_cast<std::uint8_t>(i);
        }
        names.push_back(n);
        return static_cast<std::uint8_t>(names.size() - 1);
    }

    std::size_t size() const { return code.size(); }
};

} // namespace cvm
