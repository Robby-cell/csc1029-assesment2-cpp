#include "csc/Command.hpp"

#include <string>
#include <unordered_map>

#include "csc/UserInterface.hpp"

using namespace csc;           // NOLINT
using namespace csc::command;  // NOLINT

static inline std::unordered_map<std::string, Command::Tag> command_map{
    {"exit", Command::Tag::Exit},
};

auto Command::get(const UserInterface& ui) -> Command {
  std::string option{};

  while (true) {
    ui.read_input(option);
    for (auto& ch : option) {
      ch = std::tolower(ch);
    }

    if (command_map.contains(option)) {
      return Command{command_map.at(option)};
    } else {
      ui.print("Invalid command: ");
      ui.println(option);
    }
  }
}

auto Command::execute(const UserInterface& ui) -> void {
  switch (tag_) {
    case Tag::Exit:
      ui.clear_screen();
      break;
  }
}
