#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <string>

#include "csc/UserInterface.hpp"

using namespace std::literals::chrono_literals;
using namespace csc::date::literals;  // NOLINT
using Key = sf::Keyboard::Key;

constexpr inline auto Title{"CSC1029 Assessment 2"};
constexpr inline auto WindowWidth{800U};
constexpr inline auto WindowHeight{600U};

class MediaImages : public csc::UserInterface {
 private:
  enum class State {
    Main,
    Image,
  } state_;

 public:
  MediaImages(const sf::VideoMode& window_size, const char* const window_title)
      : window_{window_size, window_title} {}

  void run() {
    while (window_.isOpen()) {
      sf::Event event;
      while (window_.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
          window_.close();
        } else if (event.type == sf::Event::KeyPressed) {
          key_pressed_ = event.key.code;
        }
      }
      auto x = event.key.code;
      window_.clear(sf::Color::Black);

      draw();

      window_.display();
    }
  }

  void put(std::string_view message) override { render_text_.append(message); }
  void putln(std::string_view message) override {
    render_text_.append(message);
    render_text_.append("\n");
  }
  void show_image(const csc::ImageRecord& image) override {
    load_image(image);
    window_.draw(bundle_.sprite_);
  }
  void clear_screen() override { window_.clear(); }
  std::string& read_input(std::string& buf) override {
    buf.append(" ");
    return buf;
  }
  void wait_for_enter() noexcept override {}

 private:
  auto load_image(const csc::ImageRecord& image) -> void {
    bundle_.image_ = sf::Image{};
    bundle_.image_.loadFromFile(image.get_thumbnail_path().string());
    bundle_.load();
  }

  auto input_complete() const noexcept -> bool {
    return key_pressed_ == Key::Return;
  }

  void draw() const {}

  mutable sf::RenderWindow window_;
  struct Image {
    sf::Image image_;
    sf::Texture texture_;
    sf::Sprite sprite_;
    void load() {
      texture_.loadFromImage(image_);
      sprite_.setTexture(texture_, true);
    }
  } bundle_;
  std::string render_text_;
  std::string user_input_;  // user input buffer
  Key key_pressed_ = Key::Unknown;
};

auto main() -> int {
  MediaImages console{{WindowWidth, WindowHeight}, Title};
  console.run();
}