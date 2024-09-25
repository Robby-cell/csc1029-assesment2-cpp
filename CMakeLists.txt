cmake_minimum_required(VERSION 3.27)
project(csc)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# DELETE THE OUT FOLDER AFTER CHANGING THIS BECAUSE VISUAL STUDIO DOESN'T SEEM TO RECOGNIZE THIS CHANGE AND REBUILD!
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE) # link time optimization

# Ensure SFML is built as static libraries
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build SFML as static libraries" FORCE)

if(MSVC)
	add_compile_options(/arch:AVX2) # make sure SIMD optimizations take place
endif()

project(assessment2)

# MY_SOURCES is defined to be a list of all the source files for my game
set(MY_SOURCES
	src/Command.cpp
	src/Console.cpp
	src/RequiredImages.cpp
	src/UserInterface.cpp
	src/ImageRecord.cpp)

# The shared components between the gui and the tui parts of the app
add_library("${CMAKE_PROJECT_NAME}" STATIC)

set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY CXX_STANDARD 23)

target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${MY_SOURCES})

if(MSVC) # If using the VS compiler...
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC _CRT_SECURE_NO_WARNINGS)
	# set_target_properties("${CMAKE_PROJECT_NAME}" PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup") #no console
endif()

target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")

add_executable(QUBImages src/QUBImages.cpp)
target_link_libraries(QUBImages PRIVATE "${CMAKE_PROJECT_NAME}")

# ImGui directory
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui")

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Stb REQUIRED)

add_library(ImGui STATIC
	# need to add ImGui files
	${IMGUI_DIR}/imgui.cpp
	${IMGUI_DIR}/imgui_demo.cpp
	${IMGUI_DIR}/imgui_draw.cpp
	${IMGUI_DIR}/imgui_tables.cpp
	${IMGUI_DIR}/imgui_widgets.cpp
	
	${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
	${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp)
target_include_directories(QUBMediaImages PRIVATE imgui)
target_include_directories(QUBMediaImages PRIVATE ${STB_INCLUDE_DIR})

target_link_libraries(QUBMediaImages PRIVATE glfw)
target_link_libraries(QUBMediaImages PRIVATE OpenGL::GL)

target_link_libraries(QUBMediaImages PRIVATE "${CMAKE_PROJECT_NAME}")
target_link_libraries(QUBMediaImages PRIVATE ImGui)
