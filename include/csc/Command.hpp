#ifndef CSC_COMMAND_HPP
#define CSC_COMMAND_HPP

namespace csc {

class UserInterface;

namespace command {

class Command {
 public:
  enum class Tag : unsigned char {
    Exit,
  };

  static auto get(const UserInterface& ui) -> Command;
  auto execute(const UserInterface& ui) -> void;

  inline auto is_exit() const noexcept -> bool { return false; }

 private:
  inline Command() noexcept = default;
  explicit inline Command(Tag tag) noexcept : tag_{tag} {}

  friend class ::csc::UserInterface;

  Tag tag_;
};

}  // namespace command
}  // namespace csc

#endif  // CSC_COMMAND_HPP
