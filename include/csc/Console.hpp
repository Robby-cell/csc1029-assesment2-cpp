#ifndef CSC_CONSOLE_HPP
#define CSC_CONSOLE_HPP

#include "csc/UserInterface.hpp"

namespace csc {  // NOLINT
namespace console {

class Console : public UserInterface {
 public:
  Console() = default;
  ~Console() override = default;

  void put(std::string_view message) override;
  void putln(std::string_view message) override;

  void show_image(const ImageRecord& image) override;
  void clear_screen() override;
  std::string& read_input(std::string& buf) override;  // NOLINT

  void wait_for_enter() noexcept override;

 private:
};

}  // namespace console
}  // namespace csc

#endif  // CSC_CONSOLE_HPP