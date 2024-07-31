#include <SFML/Graphics.hpp>

#include "csc/Console.hpp"

using namespace std::literals::chrono_literals;
using namespace csc::date::literals;  // NOLINT

constexpr inline auto Title{"CSC1029 Assessment 2"};
constexpr inline auto WindowWidth{800U};
constexpr inline auto WindowHeight{600U};

auto main() -> int {
  // sf::RenderWindow window(sf::VideoMode(WindowWidth, WindowHeight), Title);

  csc::console::Console console{};
  console.run();

  // csc::ImageManager manager{csc::required::manager_with_required_images()};
  // manager.add_image(csc::ImageRecord(
  //     "Title3", "Description", csc::ImageRecord::Genre::Food(),
  //     {2024y / std::chrono::March / 3d, 2_h / 24_m}, "path/to/image"));
  // manager.add_image(csc::ImageRecord(
  //     "Title1", "Description", csc::ImageRecord::Genre::Astronomy(),
  //     {2024y / std::chrono::December / 3d, 4_m}, "path/to/image"));

  // auto images = manager.search_title("Title1");
  // std::cout << images;

  // auto found{
  //     manager.search_between_dates({2023y / std::chrono::January / 2d, 0_h},
  //                                  {2023y / std::chrono::January / 5d,
  //                                  0_h})};
  // std::cout << found << '\n';

  // std::cout << "\nAll images:\n" << manager;

  // while (window.isOpen()) {
  //   sf::Event event;
  //   while (window.pollEvent(event)) {
  //     if (event.type == sf::Event::Closed) {
  //       window.close();
  //     }
  //   }

  //   window.clear(sf::Color::Black);
  //   window.display();
  // }
}
