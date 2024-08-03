#include "csc/Console.hpp"

using namespace std::literals::chrono_literals;
using namespace csc::date::literals;  // NOLINT

auto main() -> int {
  csc::console::Console console{};
  console.run();
}
