#include "csc/UserInterface.hpp"

#include <chrono>
#include <stdexcept>

#include "csc/ImageAlbum.hpp"
#include "csc/ImageRecord.hpp"
#include "csc/RequiredImages.hpp"

using namespace csc;  // NOLINT

static inline auto read_number_between(
    const UserInterface& ui, const std::size_t min,
    const std::size_t max) noexcept -> std::size_t {
  std::string buf;
  std::size_t result;
  do {
    ui.print("Enter a number between {} and {}: ", min, max);
    ui.read_input(buf);

    try {
      result = std::stoi(buf);
    } catch (...) {
      ui.println("Must enter a number.");
    }
  } while (result < min or result > max);
  return result;
}

UserInterface::UserInterface() noexcept
    : manager_{required::manager_with_required_images()} {}

auto UserInterface::show_images(ImageAlbum& images) const noexcept -> void {
  enum class GetImage { Next, Previous, Exit };
  using MyExtractor =
      Extractor<OptionPack<{"Next image", GetImage::Next},
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

auto UserInterface::add_image() -> void {
  std::string buf;

  println("Enter the title of the image.");
  auto title{read_input(buf)};

  println("Enter the description of the image.");
  auto description{read_input(buf)};

  println("Enter the genre of the image.");
  auto genre{get_genre()};

  println("Enter the date of the image.");
  auto date{get_date()};

  println("Enter the file path of the image.");
  auto file_path{get_file_path()};

  manager_.add_image(ImageRecord{title, description, genre, date, file_path});
}

auto UserInterface::search_image() -> void {
  enum class SearchCriteria {
    Id,
    Title,
    Description,
    Genre,
    Date,
  };
  using ExtractorType = Extractor<OptionPack<
      {"Id", SearchCriteria::Id}, {"Title", SearchCriteria::Title},
      {"Description", SearchCriteria::Description},
      {"Genre", SearchCriteria::Genre}, {"Date", SearchCriteria::Date}>>;

  auto result{ExtractorType::get(*this)};

  switch (result) {
    case SearchCriteria::Id: {
      break;
    }
    case SearchCriteria::Title: {
      break;
    }
    case SearchCriteria::Description: {
      break;
    }
    case SearchCriteria::Genre: {
      break;
    }
    case SearchCriteria::Date: {
      break;
    }
  }
}

auto UserInterface::display_all_images() -> void {
  auto images = manager_.get_all_images();
  show_images(images);
}

auto UserInterface::get_non_empty_string() const noexcept -> std::string {
  std::string buf;
  while (true) {
    read_input(buf);
    if (buf.empty()) {
      continue;
    }
    return std::move(buf);
  }
}

auto UserInterface::get_genre() const noexcept -> ImageRecord::Genre {
  using ExtractorType =
      Extractor<OptionPack<{"Astronomy", ImageRecord::Genre::Astronomy()},
                           {"Architecture", ImageRecord::Genre::Architecture()},
                           {"Sport", ImageRecord::Genre::Sport()},
                           {"Landscape", ImageRecord::Genre::Landscape()},
                           {"Portrait", ImageRecord::Genre::Portrait()},
                           {"Nature", ImageRecord::Genre::Nature()},
                           {"Aerial", ImageRecord::Genre::Aerial()},
                           {"Food", ImageRecord::Genre::Food()},
                           {"Other", ImageRecord::Genre::Other()}>>;

  auto result{ExtractorType::get(*this)};
  return result;
}

auto UserInterface::get_year_month_day() const noexcept -> date::Date {
  println("Enter the year of the image.");
  std::size_t year{read_number_between(*this, 1900UZ, 2024UZ)};
  println("Enter the month of the image.");
  std::size_t month{read_number_between(*this, 1UZ, 12UZ)};

  static constexpr auto DaysPerMonth = [](std::size_t year, std::size_t month) {
    static constexpr auto IsLeapYear = [](std::size_t year) {
      return (year % 4UZ == 0UZ and
              (year % 100UZ != 0UZ or year % 400UZ == 0UZ));
    };
    if (month == 2UZ) {
      if (IsLeapYear(year)) {
        return 29UZ;
      }
      return 28UZ;
    }
    static constexpr std::size_t DaysInMonth[]{
        31UZ, 28UZ, 31UZ, 30UZ, 31UZ, 30UZ, 31UZ, 31UZ, 30UZ, 31UZ, 30UZ, 31UZ};
    return DaysInMonth[month - 1UZ];
  };

  println("Enter the day of the image.");
  std::size_t day{read_number_between(*this, 1UZ, DaysPerMonth(year, month))};
  return date::Date{std::chrono::year(year), std::chrono::month(month),
                    std::chrono::day(day)};
}

auto UserInterface::get_time() const noexcept -> date::Time {
  println("Enter the hour of the image.");
  std::size_t hour{read_number_between(*this, 0UZ, 23UZ)};
  println("Enter the minute of the image.");
  std::size_t minute{read_number_between(*this, 0UZ, 59UZ)};
  println("Enter the second of the image.");
  std::size_t second{read_number_between(*this, 0UZ, 59UZ)};
  println("Enter the millisecond of the image.");
  std::size_t milli{read_number_between(*this, 0UZ, 999UZ)};
  return date::Time{hour, minute, second, milli};
}

auto UserInterface::get_date() const noexcept -> ImageRecord::DateType {
  auto date{get_year_month_day()};
  auto time{get_time()};
  return date::DateTime{date, time};
}

auto UserInterface::get_file_path() const noexcept -> std::filesystem::path {
  std::string buf;
  do {
    print("Enter a file path (relative or absolute): ");
    read_input(buf);
  } while (not std::filesystem::exists(buf));
  return std::filesystem::path{buf};
}
