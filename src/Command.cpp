#include "csc/Command.hpp"

#include "csc/UserInterface.hpp"

using namespace csc;           // NOLINT
using namespace csc::command;  // NOLINT

using Tag = Command::Tag;
using CommandOptions = Extractor<
    Command::Tag,
    OptionPack<{"Add image", Tag::AddImage}, {"Search image", Tag::SearchImage},
               {"Display all images", Tag::DisplayAllImages},
               {"Exit", Tag::Exit}>>;

auto Command::get(const UserInterface& ui) -> Command {
  return Command{CommandOptions::get(ui)};
}

auto Command::execute(UserInterface& ui) -> void {
  switch (tag_) {
    case Tag::AddImage: {
      ui.add_image();
      break;
    }
    case Tag::SearchImage: {
      ui.search_image();
      break;
    }
    case Tag::DisplayAllImages: {
      ui.display_all_images();
      break;
    }
    case Tag::Exit: {
      break;
    }
  }
}
