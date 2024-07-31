#ifndef CSC_OPTIONPACK_HPP
#define CSC_OPTIONPACK_HPP

#include <algorithm>
#include <cstddef>
#include <span>
#include <string_view>

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

template <ComptimeString... MyOptions>
struct OptionPack {
  static constexpr std::string_view Options[]{MyOptions.str()...};
  static constexpr std::size_t Size{sizeof...(MyOptions)};

  static constexpr auto options() noexcept
      -> std::span<const std::string_view, Size> {
    return std::span<const std::string_view, Size>(Options);
  }
};

}  // namespace csc

#endif  // CSC_OPTIONPACK_HPP