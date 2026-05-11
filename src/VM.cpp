// =============================================================================
//  VM.cpp  —  implementation goes here.
//
//  Owner: <fill in>
//  Branch: feat/vm
//
//  Day 1 target: OP_CONST_INT, OP_ADD/SUB/MUL/DIV/NEG, OP_PRINT, OP_HALT.
//  Day 2: OP_DEFINE_GLOBAL/GET/SET, OP_EQ/LT, OP_JUMP/JUMP_IF_FALSE/LOOP,
//         OP_TRUE/FALSE/POP, OP_INPUT.
//
//  Use a plain `switch (op)` inside `while (true)`. Don't reach for computed
//  gotos — measurable wins only at millions of ops/sec, demo-irrelevant.
// =============================================================================

#include "VM.h"

namespace cvm {

void VM::run(const Chunk& chunk) {
    // TODO(vm): dispatch loop.
    //   std::size_t ip = 0;
    //   while (true) {
    //       OpCode op = static_cast<OpCode>(chunk.code[ip++]);
    //       switch (op) {
    //           case OP_CONST_INT: { ... }
    //           case OP_ADD:       { auto b = pop(); auto a = pop(); push(a + b); break; }
    //           ...
    //           case OP_HALT:      return;
    //       }
    //   }
    (void)chunk;
}

void VM::resetStack() {
    stack_.clear();
}

} // namespace cvm
