#ifndef CSC_IMAGEALBUM_HPP
#define CSC_IMAGEALBUM_HPP

#include <algorithm>
#include <concepts>
#include <cstring>
#include <ostream>
#include <vector>

#include "csc/ImageRecord.hpp"
#include "csc/core.h"

namespace csc {

class ImageAlbum {
 public:
  using ImageCollection = std::vector<ImageRecord>;

  explicit inline ImageAlbum(ImageCollection images = {}) noexcept
      : images_(std::move(images)) {}

  template <typename... Args>
    requires(std::is_same_v<Args, ImageRecord> and ...)
  explicit inline ImageAlbum(Args&&... images) noexcept
      : images_{std::forward<Args>(images)...} {}

  MAYBE_CONSTEXPR inline auto get_images() const noexcept
      -> const ImageCollection& {
    return images_;
  }
  MAYBE_CONSTEXPR inline auto get_images() noexcept -> ImageCollection& {
    return images_;
  }

  inline auto get_first_image() -> const ImageRecord& {
    current_image_ = 0ULL;
    return images_.at(current_image_);
  }

  inline auto get_next_image() -> const ImageRecord& {
    if (current_image_ >= images_.size() - 1) {
      throw std::out_of_range{"No next image."};
    }
    return images_[++current_image_];
  }

  inline auto get_previous_image() -> const ImageRecord& {
    if (current_image_ == 0) {
      throw std::out_of_range{"No previous image."};
    }
    return images_[--current_image_];
  }

  inline auto emplace(ImageRecord image) -> void {
    auto it{
        std::lower_bound(images_.begin(), images_.end(), image, std::less{})};
    images_.insert(it, std::move(image));
  }

  template <typename... MyArgs>
    requires(std::constructible_from<ImageAlbum, MyArgs...>)
  inline auto emplace(MyArgs&&... args) -> void {
    emplace(ImageRecord{std::forward<MyArgs>(args)...});
  }

  inline auto is_empty() const noexcept -> bool { return images_.empty(); }

  explicit inline operator std::string() const noexcept {
    std::string result;

    for (const auto& image : images_) {
      result += image.to_string() + "\n";
    }
    result.erase(result.end() - 1);
    return result;
  }

  MAYBE_CONSTEXPR inline auto begin() const noexcept { return images_.begin(); }
  MAYBE_CONSTEXPR inline auto end() const noexcept { return images_.end(); }

 private:
  friend auto operator<<(std::ostream& os,
                         const ImageAlbum& album) -> std::ostream&;

  ImageCollection images_;

  /// \brief Iterator for the images in the album.
  std::size_t current_image_;
};

inline auto operator<<(std::ostream& os,
                       const ImageAlbum::ImageCollection& images)
    -> std::ostream& {
  for (const auto& image : images) {
    os << image << "\n";
  }
  return os;
}
inline auto operator<<(std::ostream& os,
                       const ImageAlbum& album) -> std::ostream& {
  os << album.images_;
  return os;
}

}  // namespace csc

#endif  // CSC_IMAGEALBUM_HPP
