#ifndef CSC_OPTIONPACK_HPP
#define CSC_OPTIONPACK_HPP

#include <algorithm>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <string_view>

#include "csc/core.h"

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

template <ComptimePair... MyOptions>
  requires(core::folding_type<decltype(MyOptions.value())...>::value)
struct OptionPack {
  using ValueType = core::folding_type<decltype(MyOptions.value())...>::Type;

  static constexpr std::string_view Options[]{MyOptions.str()...};
  static constexpr const char* OptionsCStr[]{MyOptions.c_str()...};
  static constexpr ValueType Values[]{MyOptions.value()...};
  static constexpr std::size_t Size{sizeof...(MyOptions)};

  static constexpr auto options() noexcept
      -> std::span<const std::string_view, Size> {
    return std::span<const std::string_view, Size>(Options);
  }

  static constexpr inline auto value_at(std::size_t index) -> ValueType {
    if (index >= Size) {
      throw std::out_of_range("index out of range");
    }
    return Values[index];
  }
};

}  // namespace csc

#endif  // CSC_OPTIONPACK_HPP
