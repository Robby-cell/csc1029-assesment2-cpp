#include "csc/Console.hpp"

#include <iostream>
#include <string>

using namespace csc::console;  // NOLINT

void Console::put(std::string_view message) { std::cout << message; }
void Console::putln(std::string_view message) { std::cout << message << '\n'; }

void Console::show_image(const ImageRecord& image) {
  std::cout << image << '\n';
}
void Console::clear_screen() { std::cout << "\033[2J\033[1;1H"; }
void Console::wait_for_enter() noexcept {
  char c;
  while (std::cin.get(c), c != '\n') {
  }
}

auto Console::read_input(std::string& buf) -> std::string& {
  std::getline(std::cin, buf);
  return buf;
}
