cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ! ! ! ! ! ! !
# set this to true to ship the game!
# basically this will change the RESOURCES_PATH to be the local path
# ! ! ! ! ! ! !
# DELETE THE OUT FOLDER AFTER CHANGING THIS BECAUSE VISUAL STUDIO DOESN'T SEEM TO RECOGNIZE THIS CHANGE AND REBUILD!
option(PRODUCTION_BUILD "Make this a production build" OFF)

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
# DON'T ADD THE SOURCES BY HAND, they are already added with this macro
set(MY_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/src/Command.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/Console.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/RequiredImages.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/UserInterface.cpp")

add_library("${CMAKE_PROJECT_NAME}" STATIC)

set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY CXX_STANDARD 23)

if(PRODUCTION_BUILD)
	# setup the ASSETS_PATH macro to be in the root folder of your exe
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="./resources/")

	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=1)

else()
	# This is useful to get an ASSETS_PATH in your IDE during development
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=0)
endif()

target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${MY_SOURCES})

if(MSVC) # If using the VS compiler...
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC _CRT_SECURE_NO_WARNINGS)

	# YOU CAN REMOVE THE CONSOLE WITH THIS LINE! YOU CAN EVEN DO AN IF PRODUCTION_BUILD TO REMOVE IT ONLY IN PRODUCTION BUILDS
	# set_target_properties("${CMAKE_PROJECT_NAME}" PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup") #no console
endif()

target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")

add_executable(QUBImages src/QUBImages.cpp)
target_link_libraries(QUBImages PRIVATE "${CMAKE_PROJECT_NAME}")

set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui")

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(QUBMediaImages
	src/QUBMediaImages.cpp
	${IMGUI_DIR}/imgui.cpp
	${IMGUI_DIR}/imgui_demo.cpp
	${IMGUI_DIR}/imgui_draw.cpp
	${IMGUI_DIR}/imgui_tables.cpp
	${IMGUI_DIR}/imgui_widgets.cpp
	
	${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
	${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp)
target_include_directories(QUBMediaImages PRIVATE imgui)
target_link_libraries(QUBMediaImages PRIVATE glfw)
target_link_libraries(QUBMediaImages PRIVATE OpenGL::GL)

target_link_libraries(QUBMediaImages PRIVATE "${CMAKE_PROJECT_NAME}")
