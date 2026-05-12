#pragma once

// =============================================================================
//  VM.h
// =============================================================================

#include "Chunk.h"
#include "Value.h"
#include <vector>
#include <unordered_map>
#include <string>

namespace cvm {

// A CallFrame represents a single active function call.
struct CallFrame {
    std::shared_ptr<ObjFunction> function;
    std::size_t                  ip;
    std::size_t                  stackBase; // index into the VM stack where this frame starts
};

class VM {
public:
    void run(const Chunk& mainChunk);
    void resetStack();

private:
    std::vector<Value>     stack_;
    std::unordered_map<std::string, Value> globals_;
    
    // Call stack
    std::vector<CallFrame> frames_;

    void push(Value v) { stack_.push_back(std::move(v)); }
    Value pop() {
        if (stack_.empty()) throw std::runtime_error("stack underflow");
        Value v = std::move(stack_.back());
        stack_.pop_back();
        return v;
    }
};

} // namespace cvm
