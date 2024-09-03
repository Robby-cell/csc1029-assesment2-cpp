#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "csc/ImageAlbum.hpp"
#include "csc/ImageManager.hpp"
#include "csc/ImageRecord.hpp"
#include "csc/OptionPack.hpp"
#include "csc/UserInterface.hpp"
#include "csc/core.h"
#include "csc/date.hpp"
#include "imgui.h"

#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h>

#include <cstdio>

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && \
    !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace WindowConfig {
constexpr inline int Width{1280};
constexpr inline int Height{720};
constexpr inline const char* Title{"QUBMediaImages"};
}  // namespace WindowConfig

inline auto enter_pressed() noexcept -> bool {
  return ImGui::IsKeyPressed(ImGuiKey_Enter);
}

template <std::size_t OptionNameSize, std::size_t OptionDescSize,
          typename MyValue>
struct Option {
  using ValueType = MyValue;

  constexpr Option(const char (&name)[OptionNameSize],
                   const char (&desc)[OptionDescSize], MyValue value)
      : OptionName{name}, OptionDesc{desc}, OptionValue{value} {}

  constexpr auto GetOptionName() const noexcept -> std::string_view {
    return OptionName.str();
  }

  constexpr auto GetOptionDescription() const noexcept -> std::string_view {
    return OptionDesc.str();
  }

  constexpr auto Display() const noexcept -> std::string {
    return std::format("{} - {}", OptionName.str(), OptionDesc.str());
  }

  constexpr auto GetValue() const noexcept -> ValueType { return OptionValue; }

  const csc::ComptimeString<OptionNameSize> OptionName;
  const csc::ComptimeString<OptionDescSize> OptionDesc;
  const ValueType OptionValue;
};

template <Option... Options>
struct GetFromTheseOptions {
  static constexpr auto Arity{sizeof...(Options)};

  static_assert(Arity > 0, "Can't have zero options");

  using ValueType =
      csc::core::folding_type<typename decltype(Options)::ValueType...>::Type;

  static constexpr std::array<ValueType, Arity> Values{Options.GetValue()...};
  static constexpr std::array<std::string_view, Arity> OptionNames{
      Options.GetOptionName()...};
  static constexpr std::array<std::string_view, Arity> OptionDescriptions{
      Options.GetOptionDescription()...};
  static inline std::array<std::string, Arity> option_display_strings{
      Options.Display()...};

  static constexpr auto map_items() -> decltype(auto) {
    std::array<const char*, Arity> array{nullptr};
    std::transform(option_display_strings.begin(), option_display_strings.end(),
                   array.begin(),
                   [](const std::string& s) { return s.c_str(); });
    return array;
  }

  static constexpr auto GetValue() -> std::optional<ValueType> {
    const auto OptionDisplayStrings_CStr{map_items()};  // NOLINT
    static int index{0};

    ImGui::Combo("##Combo", &index, OptionDisplayStrings_CStr.data(), Arity);

    if (ImGui::Button("Ok") or enter_pressed()) {
      auto value = GetValueAtIndex(index);
      index = 0;
      return value;
    }
    return std::nullopt;
  }

  static constexpr auto GetValueAtIndex(std::size_t index) -> ValueType {
    return Values[index];
  }
};

namespace image {
auto LoadTextureFromMemory(const void* data, size_t data_size,
                           GLuint* out_texture, int* out_width,
                           int* out_height) -> int {
  // Load from file
  int image_width = 0;
  int image_height = 0;
  unsigned char* image_data = stbi_load_from_memory(
      static_cast<const unsigned char*>(data), static_cast<int>(data_size),
      &image_width, &image_height, nullptr, 4);
  if (image_data == nullptr) {
    return 0;
  }

  // Create a OpenGL texture identifier
  GLuint image_texture;
  glGenTextures(1, &image_texture);
  glBindTexture(GL_TEXTURE_2D, image_texture);

  // Setup filtering parameters for display
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Upload pixels into texture
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, image_data);
  stbi_image_free(image_data);

  *out_texture = image_texture;
  *out_width = image_width;
  *out_height = image_height;

  return 1;
}

auto LoadTextureFromFile(const char* file_name, GLuint* out_texture,
                         int* out_width, int* out_height) -> bool {
  FILE* f{nullptr};
  fopen_s(&f, file_name, "rb");
  if (f == nullptr) {
    return false;
  }
  fseek(f, 0, SEEK_END);
  auto file_size = static_cast<size_t>(ftell(f));
  if (file_size == -1) {
    return false;
  }
  fseek(f, 0, SEEK_SET);
  void* file_data = IM_ALLOC(file_size);
  fread(file_data, 1, file_size, f);
  bool ret = LoadTextureFromMemory(file_data, file_size, out_texture, out_width,
                                   out_height);
  IM_FREE(file_data);
  return ret;
}
}  // namespace image

namespace render {

class Text {
 public:
  explicit Text(const char* text) : text_(text) {}

  void render() const { ImGui::Text("%s", text_); }

 private:
  const char* text_;
};
class Image {
 public:
  explicit Image(const char* file_name) {
    int width;
    int height;
    if (!image::LoadTextureFromFile(file_name, &texture_, &width, &height)) {
      throw std::runtime_error("Failed to load image");
    }
    size_ = ImVec2(width, height);
  }
  ~Image() = default;

  void render() const {
    ImGui::Image((void*)(intptr_t)texture_, size_);  // NOLINT
  }

 private:
  GLuint texture_;
  ImVec2 size_;
};

}  // namespace render

static void glfw_error_callback(int error, const char* description) {
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

constexpr inline auto GetGlslVersion() -> const char* {
#if defined(IMGUI_IMPL_OPENGL_ES2)
  // GL ES 2.0 + GLSL 100
  return "#version 100";
#elif defined(__APPLE__)
  // GL 3.2 + GLSL 150
  return "#version 150";
#else
  // GL 3.0 + GLSL 130
  return "#version 130";
#endif
}

class MediaImages : public csc::UserInterface {
 private:
  using ImageAtlas = std::unordered_map<std::string, render::Image>;
  static inline ImageAtlas images;

 public:
  auto run() -> void {
    std::array<char, 128> input_buffer{0};

    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(window_, "#canvas");
#endif
    ImGui_ImplOpenGL3_Init(GetGlslVersion());

    // Our state
    ImVec4 clear_color = ImVec4(0.45F, 0.55F, 0.60F, 1.00F);

    // Main loop
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's
    // not attempt to do a fopen() of the imgui.ini file. You may manually
    // call LoadIniSettingsFromMemory() to load settings from your own
    // storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!glfwWindowShouldClose(window_))
#endif
    {
      glfwPollEvents();
      if (glfwGetWindowAttrib(window_, GLFW_ICONIFIED) != 0) {
        ImGui_ImplGlfw_Sleep(10);
        continue;
      }

      // Start the Dear ImGui frame
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      {
        static float f = 0.0F;
        static int counter = 0;

        ImGui::Begin(WindowConfig::Title);

        root();

        ImGui::End();
      }

      // Rendering
      ImGui::Render();
      int display_w;
      int display_h;
      glfwGetFramebufferSize(window_, &display_w, &display_h);
      glViewport(0, 0, display_w, display_h);
      glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
                   clear_color.z * clear_color.w, clear_color.w);
      glClear(GL_COLOR_BUFFER_BIT);
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      glfwSwapBuffers(window_);

      if (state_ == State::Exit) {
        glfwSetWindowShouldClose(window_, GL_TRUE);
      }
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif
  }

  MediaImages(int width, int height, const char* title) {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
      throw std::runtime_error("Failed to initialize GLFW");
    }

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE,
                   GLFW_OPENGL_CORE_PROFILE);             // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);  // Required on Mac
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+
    // only glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // 3.0+ only
#endif

    window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (window_ == nullptr) {
      throw std::runtime_error("Failed to create GLFW window");
    }
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);  // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
  }
  ~MediaImages() override {
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (window_) {
      glfwDestroyWindow(window_);
    }
    glfwTerminate();
  }

  void put(std::string_view message) const override {
    ImGui::Text("%s", message.data());
  }
  void putln(std::string_view message) const override {
    ImGui::Text("%s\n", message.data());
  }
  void show_image(const csc::ImageRecord& image) const override {
    auto path{image.get_thumbnail_path().string()};
    if (not images.contains(path)) {
      load_image(path.c_str());
    }
    const auto& renderable_image{images.at(path)};
    renderable_image.render();

    println("Title: {}, Description: {}, Genre: {}, Date taken: {}",
            image.get_title(), image.get_description(),
            image.get_genre().to_string(), image.get_date_taken().to_string());
  }

  void clear_screen() const override {}
  std::string& read_input(std::string& buf) const override  // NOLINT
  {
    return buf;
  }

  void wait_for_enter() const noexcept override {}

 private:
  static inline std::optional<csc::ImageAlbum> current_images{std::nullopt};

  void show_current_images() {
    static const csc::ImageRecord* image{nullptr};
    static const char* message{nullptr};

    if (not current_images.has_value() or current_images->size() == 0) {
      ImGui::Text("There are no images");
      goto ExitButtonLabel;
    } else {
      if (not image) {
        try {
          image = &current_images->get_first_image();
        } catch (...) {
          std::unreachable();
        }
      }
    }

    if (image) {
      show_image(*image);
    }

    if (ImGui::Button("Next")) {
      try {
        image = &current_images->get_next_image();
        message = nullptr;
      } catch (...) {
        message = "No more images";
      }
    }
    if (ImGui::Button("Previous")) {
      try {
        image = &current_images->get_previous_image();
        message = nullptr;
      } catch (...) {
        message = "No previous image";
      }
    }
  ExitButtonLabel:
    if (ImGui::Button("Exit")) {
      // so next time we enter we will start from the beginning
      image = nullptr;
      transition_to_base();
    }

    if (message) {
      put(message);
    }
  }

  template <std::size_t Size>
  using Buffer = std::array<char, Size>;

  void load_image(const char* file_name) const {
    render::Image my_image{file_name};
    images.insert({std::string(file_name), my_image});
  }

  void root() {
    switch (state_) {
      case State::Base: {
        base_state();
        break;
      }
      case State::AddImage: {
        add_image_state();
        break;
      }
      case State::SearchImage: {
        search_image_state();
        break;
      }
      case State::DisplayAll: {
        display_all_state();
        break;
      }
      case State::Exit: {
        exit_state();
        break;
      }
    }
  }
  void base_state() {
    using Extractor = csc::Extractor<csc::OptionPack<
        {"Add image", State::AddImage}, {"Search image", State::SearchImage},
        {"Display all images", State::DisplayAll}, {"Exit", State::Exit}>>;

    static Buffer<32> input_buffer{0};
    static const char* message{nullptr};

    static constexpr auto Reset = [&] {
      input_buffer[0] = 0;
      message = nullptr;
    };

    Extractor::display_options(*this);
    if (ImGui::InputText("##Input:", input_buffer.data(),
                         input_buffer.size())) {
      //
    }
    if (ImGui::Button("Enter") or enter_pressed()) {
      try {
        const auto value{std::stoul(input_buffer.data()) - 1};
        auto state = Extractor::value_at(value);

        Reset();
        switch (state) {
          case State::Base: {
            transition_to_base();
            break;
          }
          case State::AddImage: {
            transition_to_add_image();
            break;
          }
          case State::SearchImage: {
            transition_to_search_image();
            break;
          }
          case State::DisplayAll: {
            transition_to_display_all();
            break;
          }
          case State::Exit: {
            transition_to_exit();
            break;
          }
        }
      } catch (...) {
        message = "Bad input";
      }
    }
    if (message) {
      put(message);
    }
  }
  void add_image_state() {
    using GenreExtractor = csc::Extractor<csc::OptionPack<
        {"Astronomy", csc::ImageRecord::Genre::Astronomy()},
        {"Architecture", csc::ImageRecord::Genre::Architecture()},
        {"Sport", csc::ImageRecord::Genre::Sport()},
        {"Landscape", csc::ImageRecord::Genre::Landscape()},
        {"Portrait", csc::ImageRecord::Genre::Portrait()},
        {"Nature", csc::ImageRecord::Genre::Nature()},
        {"Aerial", csc::ImageRecord::Genre::Aerial()},
        {"Food", csc::ImageRecord::Genre::Food()},
        {"Other", csc::ImageRecord::Genre::Other()}>>;

    static Buffer<32> title{0};
    static Buffer<64> description{0};
    static int genre{0};

    static Buffer<5> year{0};
    static Buffer<3> month{0};
    static Buffer<3> day{0};

    static Buffer<64> thumbnail_path{0};

    static const char* message{nullptr};

    static constexpr auto Reset = [&] {
      title[0] = 0;
      description[0] = 0;
      genre = 0;

      year[0] = 0;
      month[0] = 0;
      day[0] = 0;

      thumbnail_path[0] = 0;
      message = nullptr;
    };

    putln("Add image");
    put("Title:");
    ImGui::InputText("##Title", title.data(), title.size());
    put("Description:");
    ImGui::InputText("##Description", description.data(), description.size());

    put("Genre:");
    ImGui::Combo("##Genre", &genre, GenreExtractor::MyOptions::OptionsCStr,
                 GenreExtractor::MyOptions::Size);

    put("Year:");
    ImGui::InputText("##Year", year.data(), year.size());
    put("Month:");
    ImGui::InputText("##Month", month.data(), month.size());
    put("Day:");
    ImGui::InputText("##Day", day.data(), day.size());

    put("Thumbnail path:");
    ImGui::InputText("##ThumbnailPath", thumbnail_path.data(),
                     thumbnail_path.size());

    if (ImGui::Button("Add")) {
      std::cerr << "Button 'Add' pressed" << std::endl;
      try {
        auto year_ul{std::stoul(year.data())};
        auto month_ul{std::stoul(month.data())};
        auto day_ul{std::stoul(day.data())};

        std::filesystem::path valid_path{thumbnail_path.data()};
        emplace_image(csc::ImageRecord{
            title.data(),
            description.data(),
            GenreExtractor::value_at(genre),
            {{std::chrono::year(year_ul), std::chrono::month(month_ul),
              std::chrono::day(day_ul)},
             {}},
            std::move(valid_path)});

        Reset();

        transition_to_base();
      } catch (...) {
        message = "Failed to add image";
      }
    }

    if (ImGui::Button("Cancel")) {
      transition_to_base();
    }

    if (message) {
      put(message);
    }
  }
  void search_image_state() {
    enum class SearchCriteria {
      Id = 0,
      Title,
      Description,
      Genre,
      Date,
    };
    static int search_criteria{0};
    using Extractor = csc::Extractor<csc::OptionPack<
        {"Id", SearchCriteria::Id}, {"Title", SearchCriteria::Title},
        {"Description", SearchCriteria::Description},
        {"Genre", SearchCriteria::Genre}, {"Date", SearchCriteria::Date}>>;

    if (ImGui::Combo("##SearchCriteria", &search_criteria,
                     Extractor::MyOptions::OptionsCStr,
                     Extractor::MyOptions::Size)) {
      std::cerr << "SearchCriteria: "
                << Extractor::MyOptions::Options[search_criteria] << std::endl;
    }

    switch (Extractor::MyOptions::Values[search_criteria]) {
      case SearchCriteria::Id: {
        search_id();
        break;
      }
      case SearchCriteria::Title: {
        search_title();
        break;
      }
      case SearchCriteria::Description: {
        search_description();
        break;
      }
      case SearchCriteria::Genre: {
        search_genre();
        break;
      }
      case SearchCriteria::Date: {
        search_date();
        break;
      }
    }
  }
  void search_id() {
    static std::string id(16, '\0');
    static const char* message = nullptr;
    ImGui::Text("Enter Id:");
    ImGui::InputText("##Id", id.data(), id.size());

    if (enter_pressed()) {
      try {
        auto number_id = std::stoull(id);

        id[0] = 0;
        auto image = get_image_manager().search_id(number_id);

        if (image) {
          auto true_image = **image;
          csc::ImageManager manager;
          manager.add_image(true_image);

          transition_to_display_with_images(manager.take_album());
        }
      } catch (...) {
        message = "Enter a number for the id";
      }
    }
    if (message) {
      put(message);
    }
  }
  void search_title() {
    static std::string title(32, '\0');
    ImGui::Text("Title");
    ImGui::InputText("##Title", title.data(), title.size());

    if (enter_pressed()) {
      auto album{get_image_manager().search_title(title)};
      title[0] = 0;
      transition_to_display_with_images(std::move(album));
    }
  }
  void search_description() {
    static Buffer<64> description{0};
    ImGui::Text("Description");
    ImGui::InputText("##Description", description.data(), description.size());

    if (enter_pressed()) {
      auto album{get_image_manager().search_description(description.data())};
      description[0] = 0;
      transition_to_display_with_images(std::move(album));
    }
  }
  void search_genre() {
    using Extraction = GetFromTheseOptions<
        {"Astronomy",
         "Photography or imaging of astronomical objects, celestial "
         "events, or areas of the night sky.",
         csc::ImageRecord::Genre::Astronomy()},
        {"Architecture",
         "Focuses on the capture of images that accurately represent "
         "the design and feel of buildings.",
         csc::ImageRecord::Genre::Architecture()},
        {"Sport",
         "Covers all types of sports and can be considered a branch of "
         "photojournalism.",
         csc::ImageRecord::Genre::Sport()},
        {"Landscape",
         "The study of the textured surface of the Earth and features "
         "images of natural scenes.",
         csc::ImageRecord::Genre::Landscape()},
        {"Portrait",
         "Images of a person or a group of people where the face and "
         "facial features are predominant.",
         csc::ImageRecord::Genre::Portrait()},
        {"Nature",
         "Focused on elements of the outdoors including sky, water, "
         "and land, or the flora and fauna.",
         csc::ImageRecord::Genre::Nature()},
        {"Aerial", "Images taken from an aircraft or other airborne platforms.",
         csc::ImageRecord::Genre::Aerial()},
        {"Food",
         "Captures everything related to food, from fresh ingredients "
         "and plated dishes to the cooking process.",
         csc::ImageRecord::Genre::Food()},
        {"Other",
         "Covers just about any other type of image and photography "
         "genre.",
         csc::ImageRecord::Genre::Other()}>;

    auto genre{Extraction::GetValue()};

    if (genre) {
      transition_to_display_with_images(
          get_image_manager().search_genre(*genre));
    }
  }

  auto get_date(const char* const date_label, const char* const time_label,
                char (&date)[11], char (&time)[9]) -> void {
    ImGui::Text("Date [Enter YYYY/MM/DD]");
    ImGui::SameLine();
    ImGui::InputText(date_label, date, std::size(date));

    ImGui::Text("Time [Enter HH:MM:SS]");
    ImGui::SameLine();
    ImGui::InputText(time_label, time, std::size(time));
  }

  auto input_to_date(char (&date)[11],
                     char (&time)[9]) -> std::optional<csc::date::DateTime> {
    int year;
    int month;
    int day;
    auto status = std::sscanf(date, "%d/%d/%d", &year, &month, &day);  // NOLINT
    if (status != 3) {
      return std::nullopt;
    }

    int hour;
    int minute;
    int second;
    status = std::sscanf(time, "%d:%d:%d", &hour, &minute, &second);  // NOLINT
    if (status != 3) {
      return std::nullopt;
    }

    // NOLINTBEGIN
    using d = std::chrono::day;
    using m = std::chrono::month;
    using y = std::chrono::year;
    return csc::date::DateTime{
        std::chrono::year_month_day{y(year), m(month), d(day)},
        csc::date::Time{std::size_t(hour), std::size_t(minute),
                        std::size_t(second)}};
    // NOLINTEND

    return std::nullopt;
  }

  void search_date() {
    static char date1[11]{0};
    static char time1[9]{"00:00:00"};

    static char date2[11]{0};
    static char time2[9]{"00:00:00"};

    ImGui::Text("Date");
    ImGui::Text("From:");
    get_date("##Date1", "##Time1", date1, time1);

    ImGui::Text("To:");
    get_date("##Date2", "##Time2", date2, time2);

    if (ImGui::Button("Search") or enter_pressed()) {
      auto from = input_to_date(date1, time1);
      auto to = input_to_date(date2, time2);

      if (from and to) {
        auto images = get_image_manager().search_between_dates(*from, *to);
        transition_to_display_with_images(std::move(images));
      }
      date1[0] = 0;
      time1[0] = 0;
      date2[0] = 0;
      time2[0] = 0;
    }
  }

  void display_all_state() { show_current_images(); }
  void exit_state() {}

  constexpr inline void transition_to_base() { state_ = State::Base; }
  constexpr inline void transition_to_add_image() { state_ = State::AddImage; }
  constexpr inline void transition_to_search_image() {
    state_ = State::SearchImage;
  }
  inline void transition_to_display_with_images(
      const csc::ImageManager& images) {
    transition_to_display_with_images(images.get_all_images());
  }
  inline void transition_to_display_with_images(csc::ImageAlbum&& images) {
    current_images.emplace(std::move(images));
    state_ = State::DisplayAll;
  }
  inline void transition_to_display_with_images(const csc::ImageAlbum& images) {
    current_images.emplace(images);
    state_ = State::DisplayAll;
  }
  constexpr inline void transition_to_display_all() {
    transition_to_display_with_images(get_all_images());
  }
  constexpr inline void transition_to_exit() { state_ = State::Exit; }

  GLFWwindow* window_ = nullptr;
  enum class State {
    Base,
    AddImage,
    SearchImage,
    DisplayAll,
    Exit,
  } state_{State::Base};
};

// Main code
auto main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) -> int {
  MediaImages media_images{WindowConfig::Width, WindowConfig::Height,
                           WindowConfig::Title};
  media_images.run();
}
