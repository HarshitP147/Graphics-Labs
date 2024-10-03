cmake_minimum_required(VERSION 3.1)
project(lab1)

find_package(OpenGL REQUIRED)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

add_subdirectory(external)

include_directories(
	external/glfw-3.1.2/include/
	external/glm-0.9.7.1/
	external/glad-opengl-3.3/include/
	lab1/
)

add_executable(lab1_window
	lab1/lab1_window.cpp
)
target_link_libraries(lab1_window
	${OPENGL_LIBRARY}
	glfw
)

add_executable(lab1_triangle
	lab1/lab1_triangle.cpp
)
target_link_libraries(lab1_triangle
	${OPENGL_LIBRARY}
	glfw
	glad
)

add_executable(lab1_cube
	lab1/lab1_cube.cpp
	lab1/render/shader.cpp
)
target_link_libraries(lab1_cube
	${OPENGL_LIBRARY}
	glfw
	glad
)
