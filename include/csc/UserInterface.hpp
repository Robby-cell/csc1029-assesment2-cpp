#ifndef CSC_USERINTERFACE_HPP
#define CSC_USERINTERFACE_HPP

#include <string_view>

#include "csc/Command.hpp"
#include "csc/ImageManager.hpp"

namespace csc {

class UserInterface {
 public:
  inline UserInterface() noexcept = default;
  virtual ~UserInterface() noexcept = default;

  virtual void print(std::string_view message) const = 0;
  virtual void println(std::string_view message) const = 0;
  virtual void show_image(const ImageRecord& image) const = 0;
  virtual void clear_screen() const = 0;
  virtual std::string read_input() const = 0;  // NOLINT

  inline auto show_images(
      const std::vector<const ImageRecord*>& images) const noexcept -> void {
    for (const auto& image : images) {
      show_image(*image);
    }
  }

  inline auto run(this auto& self) -> void {
    command::Command cmd{};

    while (not cmd.is_exit()) {
      cmd = command::Command::get(self);
      cmd.execute(self);
    }
  }

  inline auto search_id(const std::size_t id) const noexcept -> void {
    const auto image = manager_.search_id(id);
    if (image) {
      show_image(**image);
    }
  }

  inline auto search_title(const std::string_view title) const noexcept
      -> void {
    auto images = manager_.search_title(title);
    show_images(images);
  }

 private:
  ImageManager manager_;
};

}  // namespace csc

#endif  // CSC_USERINTERFACE_HPP
