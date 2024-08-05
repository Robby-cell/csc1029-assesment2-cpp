#include <stdio.h>

#include <stdexcept>
#include <string>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "csc/ImageRecord.hpp"
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

class RenderableBase {
 public:
  virtual void render() const = 0;

 private:
};
class Text : public RenderableBase {
 public:
  explicit Text(const char* text) : text_(text) {}

  void render() const override { ImGui::Text("%s", text_); }

 private:
  const char* text_;
};
class Image : public RenderableBase {
 public:
  explicit Image(const char* file_name) {
    int width;
    int height;
    if (!image::LoadTextureFromFile(file_name, &texture_, &width, &height)) {
      throw std::runtime_error("Failed to load image");
    }
    size_ = ImVec2(width, height);
  }

  void render() const override {
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
 public:
  auto run() -> void {
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

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can
    // also load multiple fonts and use ImGui::PushFont()/PopFont() to select
    // them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you
    // need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr.
    // Please handle those errors in your application (e.g. use an assertion, or
    // display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and
    // stored into a texture when calling
    // ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame
    // below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use
    // Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string
    // literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at
    // runtime from the "fonts/" folder. See Makefile.emscripten for details.
    // io.Fonts->AddFontDefault();
    // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    // ImFont* font =
    // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f,
    // nullptr, io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != nullptr);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
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
      // Poll and handle events (inputs, window resize, etc.)
      // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to
      // tell if dear imgui wants to use your inputs.
      // - When io.WantCaptureMouse is true, do not dispatch mouse input data to
      // your main application, or clear/overwrite your copy of the mouse data.
      // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input
      // data to your main application, or clear/overwrite your copy of the
      // keyboard data. Generally you may always pass all inputs to dear imgui,
      // and hide them from your application based on those two flags.
      glfwPollEvents();
      if (glfwGetWindowAttrib(window_, GLFW_ICONIFIED) != 0) {
        ImGui_ImplGlfw_Sleep(10);
        continue;
      }

      // Start the Dear ImGui frame
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      // 1. Show the big demo window (Most of the sample code is in
      // ImGui::ShowDemoWindow()! You can browse its code to learn more about
      // Dear ImGui!).
      if (show_demo_window) {
        ImGui::ShowDemoWindow(&show_demo_window);
      }

      // 2. Show a simple window that we create ourselves. We use a Begin/End
      // pair to create a named window.
      {
        static float f = 0.0F;
        static int counter = 0;

        ImGui::Begin("Hello, world!");  // Create a window called "Hello,
                                        // world!" and append into it.

        ImGui::Text("This is some useful text.");  // Display some text (you can
                                                   // use a format strings too)
        ImGui::Checkbox("Demo Window",
                        &show_demo_window);  // Edit bools storing our window
                                             // open/close state
        ImGui::Checkbox("Another Window", &show_another_window);

        ImGui::SliderFloat(
            "float", &f, 0.0F,
            1.0F);  // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3(
            "clear color",
            reinterpret_cast<float*>(
                &clear_color));  // Edit 3 floats representing a color

        if (ImGui::Button(
                "Button")) {  // Buttons return true when clicked (most widgets
                              // return true when edited/activated)
          counter++;
        }
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                    1000.0F / io.Framerate, io.Framerate);
        ImGui::End();
      }

      // 3. Show another simple window.
      if (show_another_window) {
        ImGui::Begin(
            "Another Window",
            &show_another_window);  // Pass a pointer to our bool variable (the
                                    // window will have a closing button that
                                    // will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me")) {
          show_another_window = false;
        }
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

  void put(std::string_view message) const override {}
  void putln(std::string_view message) const override {}
  void show_image(const csc::ImageRecord& image) const override {}
  void clear_screen() const override {}
  std::string& read_input(std::string& buf) const override  // NOLINT
  {
    return buf;
  }

  void wait_for_enter() const noexcept override {}

 private:
  GLFWwindow* window_ = nullptr;
};

// Main code
auto main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) -> int {
  MediaImages media_images{WindowConfig::Width, WindowConfig::Height,
                           WindowConfig::Title};
  media_images.run();
}
