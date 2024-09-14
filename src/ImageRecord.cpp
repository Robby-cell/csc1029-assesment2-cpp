#include "csc/ImageRecord.hpp"

#include <iostream>

namespace csc {

consteval auto operator""_UZ(unsigned long long n) noexcept -> size_t {
  return static_cast<size_t>(n);
}

constexpr size_t BeginId{1_UZ};
size_t ImageRecord::next_id{BeginId};

ImageRecord::ImageRecord(std::string title, std::string description,
                         Genre genre, DateType time,
                         std::filesystem::path thumbnail_path)
    : title_(std::move(title)),
      description_(std::move(description)),
      genre_(genre),
      date_taken_(time),
      thumbnail_path_(std::move(thumbnail_path)) {
  std::cout << "My id is: " << id_ << std::endl;
}

auto ImageRecord::to_string() const noexcept -> std::string {
  return std::format(
      "Id: {}, Title: {}, Description: {}, Genre: {}, Date taken: {}", id_,
      title_, description_, genre_.to_string(), date_taken_.to_string());
}

auto operator<<(std::ostream& os, const ImageRecord& image) -> std::ostream& {
  // std::print(os, "Title: {}, Description: {}, Genre: {}, Date taken: {}",
  //            image.title_, image.description_, image.genre_.to_string(),
  //            image.date_taken_.to_string());
  std::printf("Title: %s, Description: %s, Genre: %s, Date taken: %s",
              image.title_.c_str(), image.description_.c_str(),
              image.genre_.to_string().data(),
              image.date_taken_.to_string().data());
  return os;
}

}  // namespace csc
