#include <SFML/Graphics.hpp>
#include <chrono>
#include <iostream>

#include "csc/ImageManager.hpp"
#include "csc/ImageRecord.hpp"

using namespace std::literals::chrono_literals;

constexpr inline auto Title{"CSC1029 Assessment 2"};
constexpr inline auto WindowWidth{800U};
constexpr inline auto WindowHeight{600U};

auto main() -> int {
  sf::RenderWindow window(sf::VideoMode(WindowWidth, WindowHeight), Title);

  std::chrono::year_month_day time{2024y / std::chrono::April / 4d};

  csc::ImageManager manager{
      csc::ImageRecord{"Title1", "Description", csc::ImageRecord::Genre::Food(),
                       2024y / std::chrono::February / 3d, "path/to/image"},
      csc::ImageRecord{"Title2", "Description",
                       csc::ImageRecord::Genre::Astronomy(),
                       2024y / std::chrono::April / 4d, "path/to/image"},
  };
  manager.add_image(
      csc::ImageRecord("Title3", "Description", csc::ImageRecord::Genre::Food(),
                       2024y / std::chrono::March / 3d, "path/to/image"));
  manager.add_image(csc::ImageRecord(
      "Title1", "Description", csc::ImageRecord::Genre::Astronomy(),
      2024y / std::chrono::December / 3d, "path/to/image"));

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
