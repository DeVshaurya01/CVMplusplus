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
    std::vector<std::uint8_t> code;     // bytecode
    std::vector<std::string>  names;    // global-variable names (indexed by 1-byte operand)
    std::vector<int>          lines;    // optional: source line per byte, for error messages

    // TODO(compiler): helpers
    //   void writeByte(uint8_t b, int line);
    //   void writeInt32(int32_t v, int line);
    //   void writeShort(uint16_t v, int line);
    //   uint8_t addName(const std::string& n);   // returns index, dedupes
    //   std::size_t size() const { return code.size(); }
};

} // namespace cvm
