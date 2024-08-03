#ifndef CSC_IMAGEMANAGER_HPP
#define CSC_IMAGEMANAGER_HPP

#include <cstddef>
#include <optional>
#include <string_view>
#include <vector>

#include "csc/ImageAlbum.hpp"
#include "csc/ImageRecord.hpp"
#include "csc/core.h"

namespace csc {

#define NO_DISCARD [[nodiscard]]
class ImageManager {
 public:
  inline ImageManager() noexcept = default;
  template <typename... Args>
    requires(std::is_same_v<Args, ImageRecord> and ...)
  explicit inline ImageManager(Args&&... images) noexcept
      : album_(std::forward<ImageRecord>(images)...) {}

  inline auto add_image(ImageRecord&& image) noexcept -> void {
    album_.emplace(std::move(image));
  }
  template <typename... Args>
  inline auto add_image(Args&&... args) noexcept -> void {
    album_.emplace(std::forward<Args>(args)...);
  }

  NO_DISCARD inline auto search_id(const std::size_t id) const noexcept
      -> std::optional<const ImageRecord*> {
    for (const auto& image : album_) {
      if (image.get_id() == id) {
        return &image;
      }
    }
    return std::nullopt;
  }

  NO_DISCARD inline auto search_title(
      const std::string_view title) const noexcept -> ImageAlbum {
    std::vector<const ImageRecord*> out;
    ImageAlbum album;
    for (const auto& image : album_) {
      if (image.get_title().find(title) != std::string_view::npos) {
        album.emplace(image);
      }
    }
    return std::move(album);
  }
  NO_DISCARD inline auto search_description(
      const std::string_view title) const noexcept -> ImageAlbum {
    std::vector<const ImageRecord*> out;
    ImageAlbum album;
    for (const auto& image : album_) {
      if (image.get_description().find(title) != std::string_view::npos) {
        album.emplace(image);
      }
    }
    return std::move(album);
  }

  NO_DISCARD inline auto search_genre(
      const ImageRecord::Genre& genre) const noexcept -> ImageAlbum {
    ImageAlbum album;
    for (const auto& image : album_) {
      if (image.get_genre() == genre) {
        album.emplace(image);
      }
    }
    return std::move(album);
  }

  NO_DISCARD inline auto search_between_dates(
      const date::DateTime& start,
      const date::DateTime& end) const noexcept -> ImageAlbum {
    ImageAlbum album;
    for (const auto& image : album_) {
      const auto date = image.get_date_taken();
      if (date >= start && date <= end) {
        album.emplace(image);
      }
    }
    return std::move(album);
  }

  NO_DISCARD MAYBE_CONSTEXPR inline auto get_all_images() const noexcept
      -> const ImageAlbum& {
    return album_;
  }
  NO_DISCARD MAYBE_CONSTEXPR inline auto get_all_images() noexcept
      -> ImageAlbum& {
    return album_;
  }

  NO_DISCARD MAYBE_CONSTEXPR inline auto is_empty() const noexcept -> bool {
    return album_.is_empty();
  }

  NO_DISCARD MAYBE_CONSTEXPR inline auto size() const noexcept -> std::size_t {
    return album_.size();
  }

 private:
  friend inline auto operator<<(std::ostream& os,
                                const ImageManager& manager) -> std::ostream& {
    return os << manager.album_;
  }

  ImageAlbum album_;
};
#undef NO_DISCARD

inline auto operator<<(std::ostream& os,
                       const std::vector<const ImageRecord*>& album)
    -> std::ostream& {
  for (const auto& image : album) {
    os << *image << "\n";
  }
  return os;
}

}  // namespace csc

#endif  // CSC_IMAGEMANAGER_HPP
