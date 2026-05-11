#pragma once

// =============================================================================
//  VM.h
//
//  Stack-based bytecode interpreter. Holds globals across run() calls so the
//  REPL can build up state interactively.
// =============================================================================

#include "Chunk.h"
#include "Value.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace cvm {

class VM {
public:
    // Execute a chunk to completion (until OP_HALT or end of code).
    void run(const Chunk& chunk);

    // Reset only the value stack (globals are preserved across REPL turns).
    void resetStack();

private:
    // TODO(vm):
    //   void push(Value v);
    //   Value pop();
    //   Value peek(std::size_t depth = 0) const;
    //
    //   // Operand readers (advance instruction pointer)
    //   std::uint8_t  readByte(const Chunk& c, std::size_t& ip);
    //   std::uint16_t readShort(const Chunk& c, std::size_t& ip);
    //   std::int32_t  readInt32(const Chunk& c, std::size_t& ip);

    std::vector<Value>                       stack_;
    std::unordered_map<std::string, Value>   globals_;
};

} // namespace cvm
