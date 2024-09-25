# CSC1029 Assessment 2

## Build Instructions

### Pre-requisites

- [CMake](https://cmake.org/download/)
- [vcpkg](https://github.com/microsoft/vcpkg) and install the following packages:
    - opengl
    - glfw3
    - stb
- Set VCPKG_ROOT environment variable to the vcpkg folder
- On Windows:
    - [Visual Studio](https://visualstudio.microsoft.com/downloads/) for MSVC (or use clang)

1. Open the `csc1029-assessment2` folder in a terminal.
2. Run:
- Debug
```bash
# Debug
DEBUG_FLAGS = -DCMAKE_BUILD_TYPE=Debug
mkdir out && \
cd out && \
	cmake .. -G Ninja -DCMAKE_TOOLCHAIN_FILE="${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" ${DEBUG_FLAGS} && \
	cmake --build .
```
- Release
```bash
# Debug
RELEASE_FLAGS = -DCMAKE_BUILD_TYPE=Release
mkdir out && \
cd out && \
	cmake .. -G Ninja -DCMAKE_TOOLCHAIN_FILE="${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" ${RELEASE_FLAGS} && \
	cmake --build .
```

## Running the Project

Use above commands, or, with make installed:

Open the `csc1029-assessment2` folder in a terminal and run the following command:

- GUI
```bash
make gui
```
- TUI
```bash
make tui
```

This will start the application and display the main menu.
