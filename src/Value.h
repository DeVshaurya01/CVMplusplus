#pragma once

// =============================================================================
//  Value.h
//
//  Runtime value type. For v1, every value is an int64_t:
//    - integer literals stored directly
//    - booleans encoded as 1 (true) / 0 (false)
//    - comparisons return 1 / 0
//
//  If we ever add strings or arrays, upgrade this to a tagged union or
//  std::variant<int64_t, bool, std::string, ...>.
// =============================================================================

#include <cstdint>

namespace cvm {

using Value = std::int64_t;

} // namespace cvm
