#ifndef CSC_USERINTERFACE_HPP
#define CSC_USERINTERFACE_HPP

#include <format>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <utility>

#include "csc/Command.hpp"
#include "csc/ImageAlbum.hpp"
#include "csc/ImageManager.hpp"
#include "csc/OptionPack.hpp"

namespace csc {

class UserInterface {
 public:
  UserInterface() noexcept;
  virtual ~UserInterface() noexcept = default;

  virtual void put(std::string_view message) const = 0;
  virtual void putln(std::string_view message) const = 0;
  virtual void show_image(const ImageRecord& image) const = 0;
  virtual void clear_screen() const = 0;
  virtual std::string& read_input(std::string& buf) const = 0;  // NOLINT

  virtual void wait_for_enter() const noexcept = 0;

  auto show_images(ImageAlbum& images) const noexcept -> void;

  auto run() -> void;

  auto display_image_with_id(std::size_t id) const noexcept -> void;

  auto display_images_with_title(std::string_view title) const noexcept -> void;

  auto add_image() -> void;
  auto search_image() -> void;
  auto display_all_images() -> void;

  template <typename... Args>
  inline auto print(const std::format_string<Args...> fmt,
                    Args&&... args) const noexcept -> void {
    auto str = std::format(fmt, std::forward<Args>(args)...);
    put(str);
  }
  template <typename... Args>
  inline auto println(const std::format_string<Args...> fmt,
                      Args&&... args) const noexcept -> void {
    auto str = std::format(fmt, std::forward<Args>(args)...);
    putln(str);
  }

 private:
  ImageManager manager_;
};

template <typename, typename>
struct Extractor;

template <typename ReturnType, ComptimeString... Options>
struct Extractor<ReturnType, OptionPack<Options...>> {
  using MyReturnType = ReturnType;
  using MyOptions = OptionPack<Options...>;

  static constexpr auto options() noexcept
      -> std::span<const std::string_view> {
    return MyOptions::options();
  }

  static inline auto get(const UserInterface& ui) noexcept -> ReturnType {
    std::string result;

    while (true) {
      for (auto i : std::ranges::iota_view{0ULL, MyOptions::Size}) {
        ui.println("{}. {}", i + 1, MyOptions::Options[i]);
      }
      ui.println("Enter a number between 1 and {}.", MyOptions::Size);
      try {
        ui.read_input(result);

        auto option = std::stoull(result);
        if (option <= MyOptions::Size) {
          return static_cast<ReturnType>(option - 1);
        }
        ui.println("Number must be between 1 and {}.", MyOptions::Size);
      } catch (...) {
        ui.println("You must enter a number.");
      }
    }
  }
};

}  // namespace csc

#endif  // CSC_USERINTERFACE_HPP
