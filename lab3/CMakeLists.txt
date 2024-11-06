cmake_minimum_required(VERSION 3.0)
project(lab3)

find_package(OpenGL REQUIRED)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

add_subdirectory(external)

include_directories(
	external/glfw-3.1.2/include/
	external/glm-0.9.7.1/
	external/glad-opengl-3.3/include/
	external/
	lab3/
)

add_executable(lab3_cornellbox
	lab3/lab3_cornellbox.cpp
	lab3/render/shader.cpp
)
target_link_libraries(lab3_cornellbox
	${OPENGL_LIBRARY}
	glfw
	glad
)
