#ifndef CSC_COMMAND_HPP
#define CSC_COMMAND_HPP

namespace csc {

class UserInterface;

namespace command {

class Command {
 private:
  enum class Tag : unsigned char {
    Exit,
  };

 public:
  static auto get(const UserInterface& ui) -> Command;
  auto execute(const UserInterface& ui) -> void;

  inline auto is_exit() const noexcept -> bool { return false; }

 private:
  friend class ::csc::UserInterface;

  Tag tag_;
};

}  // namespace command
}  // namespace csc

#endif  // CSC_COMMAND_HPP
