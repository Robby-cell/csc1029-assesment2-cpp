#ifndef CSC_USERINTERFACE_HPP
#define CSC_USERINTERFACE_HPP

#include <string>
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
  virtual std::string& read_input(std::string& buf) const = 0;  // NOLINT

  auto show_images(const std::vector<const ImageRecord*>& images) const noexcept
      -> void;

  auto run(this auto& self) -> void;

  auto search_id(std::size_t id) const noexcept -> void;

  auto search_title(std::string_view title) const noexcept -> void;

 private:
  ImageManager manager_;
};

}  // namespace csc

#endif  // CSC_USERINTERFACE_HPP
