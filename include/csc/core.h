#ifndef CSC_CORE_H
#define CSC_CORE_H

namespace csc::core {

#if defined(_MSC_VER) or defined(__clang__)
#define MAYBE_CONSTEXPR constexpr
#else
#define MAYBE_CONSTEXPR
#endif

}  // namespace csc::core

#endif  // CSC_CORE_H