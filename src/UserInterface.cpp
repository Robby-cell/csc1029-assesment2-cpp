#include "csc/UserInterface.hpp"

#include <stdexcept>

#include "csc/ImageAlbum.hpp"
#include "csc/ImageRecord.hpp"
#include "csc/RequiredImages.hpp"

using namespace csc;  // NOLINT

UserInterface::UserInterface() noexcept
    : manager_{required::manager_with_required_images()} {}

auto UserInterface::show_images(ImageAlbum& images) const noexcept -> void {
  enum class GetImage { Next, Previous, Exit };
  using MyExtractor =
      Extractor<GetImage, OptionPack<{"Next image", GetImage::Next},
                                     {"Previous image", GetImage::Previous},
                                     {"Exit", GetImage::Exit}>>;

  const ImageRecord* image = nullptr;
  try {
    image = &images.get_first_image();
  } catch (std::out_of_range& e) {
    println("No images.");
    wait_for_enter();
    return;
  }

  enum class Info {
    None,
    OutOfBoundsLeft,
    OutOfBoundsRight,
  } previous_info{Info::None};

  while (true) {
    clear_screen();
    show_image(*image);

    switch (previous_info) {
      case Info::None: {
        break;
      }
      case Info::OutOfBoundsLeft: {
        println("There is no previous image.");
        break;
      }
      case Info::OutOfBoundsRight: {
        println("There is no next image.");
        break;
      }
    }

    auto option{MyExtractor::get(*this)};
    previous_info = Info::None;

    switch (option) {
      case GetImage::Next: {
        try {
          image = &images.get_next_image();
        } catch (std::out_of_range& e) {
          previous_info = Info::OutOfBoundsRight;
        }
        break;
      }
      case GetImage::Previous: {
        try {
          image = &images.get_previous_image();
        } catch (std::out_of_range& e) {
          previous_info = Info::OutOfBoundsLeft;
        }
        break;
      }
      case GetImage::Exit: {
        return;
      }
    }
  }
}

auto UserInterface::run() -> void {
  command::Command cmd{};

  while (not cmd.is_exit()) {
    clear_screen();
    cmd = command::Command::get(*this);
    cmd.execute(*this);
  }
}

auto UserInterface::display_image_with_id(const std::size_t id) const noexcept
    -> void {
  const auto image = manager_.search_id(id);
  if (image) {
    show_image(**image);
  }
}

auto UserInterface::display_images_with_title(
    const std::string_view title) const noexcept -> void {
  auto images = manager_.search_title(title);
  show_images(images);
}

auto UserInterface::add_image() -> void {}

auto UserInterface::search_image() -> void {}

auto UserInterface::display_all_images() -> void {
  auto images = manager_.get_all_images();
  show_images(images);
}
