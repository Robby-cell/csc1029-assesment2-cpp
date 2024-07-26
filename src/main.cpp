#include <SFML/Graphics.hpp>
#include <chrono>
#include <iostream>

#include "csc/ImageManager.hpp"
#include "csc/ImageRecord.hpp"
#include "csc/date.hpp"

using namespace std::literals::chrono_literals;
using namespace csc::date::literals;  // NOLINT

constexpr inline auto Title{"CSC1029 Assessment 2"};
constexpr inline auto WindowWidth{800U};
constexpr inline auto WindowHeight{600U};

auto main() -> int {
  sf::RenderWindow window(sf::VideoMode(WindowWidth, WindowHeight), Title);

  csc::ImageManager manager{
      csc::ImageRecord{"Title1",
                       "Description",
                       csc::ImageRecord::Genre::Food(),
                       {2024y / std::chrono::February / 3d, 3_h},
                       "path/to/image"},
      csc::ImageRecord{"Title2",
                       "Description",
                       csc::ImageRecord::Genre::Astronomy(),
                       {2024y / std::chrono::April / 4d, 3_h / 20_m / 5_s},
                       "path/to/image"},
  };
  manager.add_image(csc::ImageRecord(
      "Title3", "Description", csc::ImageRecord::Genre::Food(),
      {2024y / std::chrono::March / 3d, 2_h / 24_m}, "path/to/image"));
  manager.add_image(csc::ImageRecord(
      "Title1", "Description", csc::ImageRecord::Genre::Astronomy(),
      {2024y / std::chrono::December / 3d, 4_m}, "path/to/image"));

  auto images = manager.search_title("Title1");
  std::cout << images;

  std::cout << "\nAll images:\n" << manager;

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        window.close();
      }
    }

    window.clear(sf::Color::Black);
    window.display();
  }
}
