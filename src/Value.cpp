#include "Value.h"
#include "Chunk.h"
#include <sstream>

namespace cvm {

std::string valueToString(const Value& v) {
    switch (v.kind) {
        case ValueKind::Int:  return std::to_string(v.intVal);
        case ValueKind::Bool: return v.boolVal ? "true" : "false";
        case ValueKind::Null: return "null";
        case ValueKind::Obj: {
            if (!v.obj) return "null";
            if (v.obj->kind == ObjKind::String) {
                return static_cast<const ObjString*>(v.obj.get())->str;
            }
            if (v.obj->kind == ObjKind::Array) {
                const auto* arr = static_cast<const ObjArray*>(v.obj.get());
                std::ostringstream oss;
                oss << "[";
                for (std::size_t i = 0; i < arr->elements.size(); ++i) {
                    if (i) oss << ", ";
                    oss << valueToString(arr->elements[i]);
                }
                oss << "]";
                return oss.str();
            }
            if (v.obj->kind == ObjKind::Map) {
                const auto* m = static_cast<const ObjMap*>(v.obj.get());
                std::ostringstream oss;
                oss << "{";
                bool first = true;
                for (const auto& pair : m->pairs) {
                    if (!first) oss << ", ";
                    oss << valueToString(pair.first) << ": " << valueToString(pair.second);
                    first = false;
                }
                oss << "}";
                return oss.str();
            }
            if (v.obj->kind == ObjKind::Function) {
                return "<fn " + static_cast<const ObjFunction*>(v.obj.get())->name + ">";
            }
            return "<obj>";
        }
    }
    return "???";
}

} // namespace cvm
