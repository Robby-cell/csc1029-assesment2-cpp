#include <array>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "csc/ImageRecord.hpp"
#include "csc/OptionPack.hpp"
#include "csc/UserInterface.hpp"
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
    // For an Emscripten build we are disabling file-system access, so let's not
    // attempt to do a fopen() of the imgui.ini file. You may manually call
    // LoadIniSettingsFromMemory() to load settings from your own storage.
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
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
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
  }
  void clear_screen() const override {}
  std::string& read_input(std::string& buf) const override  // NOLINT
  {
    return buf;
  }

  void wait_for_enter() const noexcept override {}

 private:
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
    static bool bad_input{false};

    Extractor::display_options(*this);
    if (ImGui::InputText("##Input:", input_buffer.data(),
                         input_buffer.size())) {
      //
    }
    if (ImGui::Button("Enter")) {
      try {
        const auto value{std::stoul(input_buffer.data()) - 1};
        if (value < Extractor::Size) {
          state_ = Extractor::value_at(value);
        } else {
          bad_input = true;
        }
      } catch (...) {
        bad_input = true;
      }
    }
    if (bad_input) {
      print("Bad input");
    }
  }
  void add_image_state() {}
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
        ImGui::Text("Id");
        break;
      }
      case SearchCriteria::Title: {
        ImGui::Text("Title");
        break;
      }
      case SearchCriteria::Description: {
        ImGui::Text("Description");
        break;
      }
      case SearchCriteria::Genre: {
        ImGui::Text("Genre");
        break;
      }
      case SearchCriteria::Date: {
        ImGui::Text("Date");
        break;
      }
    }
  }
  void display_all_state() {
    static const csc::ImageRecord* image{next_image()};
    static const char* message{nullptr};

    if (image) {
      show_image(*image);
    } else {
      ImGui::Text("There are no images");
    }

    if (ImGui::Button("Next")) {
      const auto* next{next_image()};
      if (next) {
        image = next;
        message = nullptr;
      } else {
        message = "No more images";
      }
    }
    if (ImGui::Button("Previous")) {
      const auto* previous{previous_image()};
      if (previous) {
        image = previous;
        message = nullptr;
      } else {
        message = "No previous image";
      }
    }
    if (ImGui::Button("Exit")) {
      state_ = State::Base;
    }

    if (message) {
      put(message);
    }
  }
  void exit_state() {}

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
