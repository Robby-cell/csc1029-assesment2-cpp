#ifndef CSC_IMAGERECORD_HPP
#define CSC_IMAGERECORD_HPP

#include <filesystem>
#include <ostream>
#include <string>
#include <string_view>

#include "csc/core.h"
#include "csc/date.hpp"

namespace csc {

class ImageRecord {
 private:
  static inline size_t next_id{1ULL};

 public:
  using DateType = date::DateTime;

  class Genre {
   private:
    enum class Tag {
      Astronomy,
      Architecture,
      Sport,
      Landscape,
      Portrait,
      Nature,
      Aerial,
      Food,
      Other,
    };
    constexpr explicit inline Genre(Tag tag) noexcept : tag_(tag) {}

   public:
    constexpr inline static auto Astronomy() noexcept -> Genre {
      return Genre(Tag::Astronomy);
    }
    constexpr inline static auto Architecture() noexcept -> Genre {
      return Genre(Tag::Architecture);
    }
    constexpr inline static auto Sport() noexcept -> Genre {
      return Genre(Tag::Sport);
    }
    constexpr inline static auto Landscape() noexcept -> Genre {
      return Genre(Tag::Landscape);
    }
    constexpr inline static auto Portrait() noexcept -> Genre {
      return Genre(Tag::Portrait);
    }
    constexpr inline static auto Nature() noexcept -> Genre {
      return Genre(Tag::Nature);
    }
    constexpr inline static auto Aerial() noexcept -> Genre {
      return Genre(Tag::Aerial);
    }
    constexpr inline static auto Food() noexcept -> Genre {
      return Genre(Tag::Food);
    }
    constexpr inline static auto Other() noexcept -> Genre {
      return Genre(Tag::Other);
    }

    MAYBE_CONSTEXPR inline auto operator==(const Genre& other) const noexcept
        -> bool {
      return tag_ == other.tag_;
    }
    MAYBE_CONSTEXPR inline auto operator!=(const Genre& other) const noexcept
        -> bool {
      return tag_ != other.tag_;
    }

    MAYBE_CONSTEXPR inline Genre(const Genre& other) noexcept = default;
    MAYBE_CONSTEXPR inline Genre(Genre&& other) noexcept = default;
    MAYBE_CONSTEXPR inline auto operator=(const Genre& other) noexcept
        -> Genre& = default;
    MAYBE_CONSTEXPR inline auto operator=(Genre&& other) noexcept
        -> Genre& = default;

    MAYBE_CONSTEXPR inline auto to_string() const noexcept -> std::string_view {
      switch (tag_) {
        case Tag::Astronomy:
          return "Photography or imaging of astronomical objects, celestial "
                 "events, or areas of the night sky.";
        case Tag::Architecture:
          return "Focuses on the capture of images that accurately represent "
                 "the design and feel of buildings.";
        case Tag::Sport:
          return "Covers all types of sports and can be considered a branch of "
                 "photojournalism.";
        case Tag::Landscape:
          return "The study of the textured surface of the Earth and features "
                 "images of natural scenes.";
        case Tag::Portrait:
          return "Images of a person or a group of people where the face and "
                 "facial features are predominant.";
        case Tag::Nature:
          return "Focused on elements of the outdoors including sky, water, "
                 "and land, or the flora and fauna.";
        case Tag::Aerial:
          return "Images taken from an aircraft or other airborne platforms.";
        case Tag::Food:
          return "Captures everything related to food, from fresh ingredients "
                 "and plated dishes to the cooking process.";
        case Tag::Other:
          return "Covers just about any other type of image and photography "
                 "genre.";
      }
    }

    //  private:
    Tag tag_;
  };

  friend MAYBE_CONSTEXPR inline auto operator<(
      const ImageRecord& self, const ImageRecord& other) noexcept -> bool {
    return self.date_taken_ < other.date_taken_;
  }
  // friend MAYBE_CONSTEXPR inline auto operator==(
  //     const ImageRecord& self, const ImageRecord& other) noexcept -> bool {
  //   return self.date_taken_ == other.date_taken_;
  // }
  // friend MAYBE_CONSTEXPR inline auto operator!=(
  //     const ImageRecord& self, const ImageRecord& other) noexcept -> bool {
  //   return self.date_taken_ != other.date_taken_;
  // }

  inline auto to_string() const noexcept -> std::string {
    return std::format("Title: {}, Description: {}, Genre: {}, Date taken: {}",
                       title_, description_, genre_.to_string(),
                       date_taken_.to_string());
  }
  explicit inline operator std::string() const noexcept { return to_string(); }

  constexpr inline auto set_title(std::string title) noexcept -> void {
    title_ = std::move(title);
  }
  constexpr inline auto set_description(std::string description) noexcept
      -> void {
    description_ = std::move(description);
  }
  constexpr inline auto set_genre(Genre genre) noexcept -> void {
    genre_ = genre;
  }
  constexpr inline auto set_thumbnail_path(std::filesystem::path path) noexcept
      -> void {
    thumbnail_path_ = std::move(path);
  }

  constexpr inline auto get_id() const noexcept -> std::size_t { return id_; }
  constexpr inline auto get_title() const noexcept -> std::string_view {
    return title_;
  }
  constexpr inline auto get_description() const noexcept -> std::string_view {
    return description_;
  }
  constexpr inline auto get_genre() const noexcept -> Genre { return genre_; }
  constexpr inline auto get_date_taken() const noexcept -> DateType {
    return date_taken_;
  }
  constexpr inline auto get_thumbnail_path() const noexcept
      -> const std::filesystem::path& {
    return thumbnail_path_;
  }

  constexpr explicit inline ImageRecord(std::string title,
                                        std::string description, Genre genre,
                                        DateType time,
                                        std::filesystem::path thumbnail_path)
      : title_(std::move(title)),
        description_(std::move(description)),
        genre_(genre),
        date_taken_(time),
        thumbnail_path_(std::move(thumbnail_path)) {}
  ImageRecord(const ImageRecord& other) noexcept = default;
  ImageRecord(ImageRecord&& other) noexcept = default;
  auto operator=(const ImageRecord& other) noexcept -> ImageRecord& = default;
  auto operator=(ImageRecord&& other) noexcept -> ImageRecord& = default;

 private:
  friend class ImageAlbum;

  friend inline auto operator<<(std::ostream& os,
                                const ImageRecord& image) -> std::ostream& {
    std::print(os, "Title: {}, Description: {}, Genre: {}, Date taken: {}",
               image.title_, image.description_, image.genre_.to_string(),
               image.date_taken_.to_string());
    return os;
  }

  std::string title_;
  std::string description_;
  std::filesystem::path thumbnail_path_;
  DateType date_taken_;
  std::size_t id_;
  Genre genre_;
};

}  // namespace csc

#endif  // CSC_IMAGERECORD_HPP
