#pragma once

// =============================================================================
//  Value.h
// =============================================================================

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace cvm {

// Forward-declare Chunk so ObjFunction can hold one.
struct Chunk;

enum class ObjKind { String, Array, Map, Function };

struct Obj {
    virtual ~Obj() = default;
    ObjKind kind;
protected:
    explicit Obj(ObjKind k) : kind(k) {}
};

struct ObjString : Obj {
    std::string str;
    explicit ObjString(std::string s) : Obj(ObjKind::String), str(std::move(s)) {}
};

// A function object contains its own bytecode chunk and metadata.
struct ObjFunction : Obj {
    std::string              name;
    int                      arity;
    std::shared_ptr<cvm::Chunk> chunk; // We'll update Chunk.h to be shareable

    ObjFunction(std::string n, int a, std::shared_ptr<cvm::Chunk> c)
        : Obj(ObjKind::Function), name(std::move(n)), arity(a), chunk(std::move(c)) {}
};

enum class ValueKind { Int, Bool, Null, Obj };

struct Value {
    ValueKind kind = ValueKind::Null;

    std::int64_t                intVal  = 0;
    bool                        boolVal = false;
    std::shared_ptr<cvm::Obj>   obj;

    Value() : kind(ValueKind::Null) {}
    explicit Value(std::int64_t v)           : kind(ValueKind::Int),  intVal(v) {}
    explicit Value(bool b)                   : kind(ValueKind::Bool), boolVal(b) {}
    explicit Value(std::shared_ptr<cvm::Obj> o) : kind(ValueKind::Obj),  obj(std::move(o)) {}

    bool operator==(const Value& o) const {
        if (kind != o.kind) return false;
        switch (kind) {
            case ValueKind::Int:  return intVal  == o.intVal;
            case ValueKind::Bool: return boolVal == o.boolVal;
            case ValueKind::Null: return true;
            case ValueKind::Obj: {
                if (obj.get() == o.obj.get()) return true;
                if (obj->kind != o.obj->kind) return false;
                if (obj->kind == ObjKind::String) {
                    return static_cast<ObjString*>(obj.get())->str == 
                           static_cast<ObjString*>(o.obj.get())->str;
                }
                return false;
            }
        }
        return false;
    }
    bool operator!=(const Value& o) const { return !(*this == o); }

    bool isTruthy() const {
        switch (kind) {
            case ValueKind::Int:  return intVal != 0;
            case ValueKind::Bool: return boolVal;
            case ValueKind::Null: return false;
            case ValueKind::Obj:  return true; 
        }
        return false;
    }

    bool operator<(const Value& o) const {
        if (kind == ValueKind::Int && o.kind == ValueKind::Int)
            return intVal < o.intVal;
        return false; 
    }
};

struct ValueHash {
    std::size_t operator()(const Value& v) const {
        switch (v.kind) {
            case ValueKind::Int:  return std::hash<std::int64_t>{}(v.intVal);
            case ValueKind::Bool: return std::hash<bool>{}(v.boolVal);
            case ValueKind::Null: return 0xDEADBEEF;
            case ValueKind::Obj: {
                if (v.obj->kind == ObjKind::String) {
                    return std::hash<std::string>{}(static_cast<ObjString*>(v.obj.get())->str);
                }
                return std::hash<void*>{}(v.obj.get());
            }
        }
        return 0;
    }
};

struct ValueEqual {
    bool operator()(const Value& a, const Value& b) const { return a == b; }
};

struct ObjArray : Obj {
    std::vector<Value> elements;
    ObjArray() : Obj(ObjKind::Array) {}
};

struct ObjMap : Obj {
    std::unordered_map<Value, Value, ValueHash, ValueEqual> pairs;
    ObjMap() : Obj(ObjKind::Map) {}
};

inline Value makeInt (std::int64_t v)            { return Value(v); }
inline Value makeBool(bool b)                    { return Value(b); }
inline Value makeNull()                          { return Value(); }
inline Value makeObj (std::shared_ptr<Obj> o)    { return Value(std::move(o)); }
inline Value makeString(std::string s) {
    return makeObj(std::make_shared<ObjString>(std::move(s)));
}
inline Value makeFunction(std::string name, int arity, std::shared_ptr<cvm::Chunk> chunk) {
    return makeObj(std::make_shared<ObjFunction>(std::move(name), arity, std::move(chunk)));
}

std::string valueToString(const Value& v);

} // namespace cvm
