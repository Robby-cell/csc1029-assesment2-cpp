#ifndef CSC_OPTIONPACK_HPP
#define CSC_OPTIONPACK_HPP

#include <algorithm>
#include <cstddef>
#include <span>
#include <string_view>
#include <type_traits>

namespace csc {

template <std::size_t MySize>
struct ComptimeString {
  consteval ComptimeString(const char (&str)[MySize]) noexcept {  // NOLINT
    std::copy_n(str, MySize, data_);
  }
  constexpr inline auto str() const noexcept -> std::string_view {
    return std::string_view{data_, MySize};
  }
  constexpr inline auto c_str() const noexcept -> const char* { return data_; }
  char data_[MySize];
};

template <typename MyType, std::size_t MySize>
struct ComptimePair {
  using ValueType = MyType;

  consteval ComptimePair(const char (&s)[MySize], MyType&& value) noexcept
      : String{s}, Value{std::forward<MyType>(value)} {}
  constexpr inline auto str() const noexcept -> std::string_view {
    return String.str();
  }
  constexpr inline auto c_str() const noexcept -> const char* {
    return String.c_str();
  }
  constexpr inline auto value() const noexcept -> MyType { return Value; }

  const ComptimeString<MySize> String;
  const MyType Value;
};

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

template <ComptimePair... MyOptions>
  requires(folding_type<decltype(MyOptions.value())...>::value)
struct OptionPack {
  using ValueType = folding_type<decltype(MyOptions.value())...>::Type;

  static constexpr std::string_view Options[]{MyOptions.str()...};
  static constexpr const char* OptionsCStr[]{MyOptions.c_str()...};
  static constexpr ValueType Values[]{MyOptions.value()...};
  static constexpr std::size_t Size{sizeof...(MyOptions)};

  static constexpr auto options() noexcept
      -> std::span<const std::string_view, Size> {
    return std::span<const std::string_view, Size>(Options);
  }

  static constexpr inline auto value_at(std::size_t index) noexcept
      -> ValueType {
    return Values[index];
  }
};

}  // namespace csc

#endif  // CSC_OPTIONPACK_HPP