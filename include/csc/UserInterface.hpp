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
#include "csc/ImageRecord.hpp"
#include "csc/OptionPack.hpp"
#include "csc/date.hpp"

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

 protected:
  inline auto get_all_images() const noexcept -> const ImageAlbum& {
    return manager_.get_all_images();
  }
  inline auto emplace_image(ImageRecord&& record) {
    return manager_.add_image(std::move(record));
  }
  template <typename... Args>
  inline auto emplace_image(Args&&... args) {
    return manager_.add_image(std::forward<Args>(args)...);
  }

 private:
  auto get_non_empty_string() const noexcept -> std::string;
  auto get_genre() const noexcept -> ImageRecord::Genre;

  auto get_year_month_day() const noexcept -> date::Date;
  auto get_time() const noexcept -> date::Time;

  auto get_date() const noexcept -> ImageRecord::DateType;

  auto get_file_path() const noexcept -> std::filesystem::path;

  ImageManager manager_;
};

template <typename>
struct Extractor;

template <ComptimePair... Options>
struct Extractor<OptionPack<Options...>> {
  using MyOptions = OptionPack<Options...>;
  using ReturnType = MyOptions::ValueType;
  static constexpr auto Size{sizeof...(Options)};

  static constexpr auto options() noexcept
      -> std::span<const std::string_view> {
    return MyOptions::options();
  }

  static inline auto display_options(const UserInterface& ui) noexcept -> void {
    for (auto i : std::ranges::iota_view{0ULL, MyOptions::Size}) {
      ui.println("{}. {}", i + 1, MyOptions::Options[i]);
    }
  }

  static constexpr inline auto value_at(std::size_t index) -> ReturnType {
    return MyOptions::value_at(index);
  }

  static inline auto get(const UserInterface& ui) noexcept -> ReturnType {
    std::string result;

    while (true) {
      display_options(ui);

      ui.println("Enter a number between 1 and {}.", MyOptions::Size);
      try {
        ui.read_input(result);

        auto option = std::stoull(result);
        if (option <= MyOptions::Size) {
          return value_at(option - 1);
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
