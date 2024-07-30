#include "csc/UserInterface.hpp"

using namespace csc;  // NOLINT

auto UserInterface::show_images(
    const std::vector<const ImageRecord*>& images) const noexcept -> void {
  for (const auto& image : images) {
    show_image(*image);
  }
}

auto UserInterface::run(this auto& self) -> void {
  command::Command cmd{};

  while (not cmd.is_exit()) {
    cmd = command::Command::get(self);
    cmd.execute(self);
  }
}

auto UserInterface::search_id(const std::size_t id) const noexcept -> void {
  const auto image = manager_.search_id(id);
  if (image) {
    show_image(**image);
  }
}

auto UserInterface::search_title(const std::string_view title) const noexcept
    -> void {
  auto images = manager_.search_title(title);
  show_images(images);
}
