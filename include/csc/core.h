#ifndef CSC_CORE_H
#define CSC_CORE_H

#include <type_traits>

namespace csc::core {

#if defined(_MSC_VER) or defined(__clang__)
#define MAYBE_CONSTEXPR constexpr
#else
#define MAYBE_CONSTEXPR constexpr
#endif

/// Fold a type, ensure they are the same type. And capture the type.
template <typename... Types>
struct folding_type;
template <typename MyType, typename... Types>
struct folding_type<MyType, Types...> : std::true_type {
 private:
  static_assert((... and std::is_same_v<MyType, Types>));

 public:
  using Type = MyType;
};

}  // namespace csc::core

#endif  // CSC_CORE_H
