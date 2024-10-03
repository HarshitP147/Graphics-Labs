#include <string> 

static std::string cubeVertexShader = R"(
#version 330 core

// Input
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;

// Matrix for vertex transformation
uniform mat4 MVP;

// Output data, to be interpolated for each fragment
out vec3 color;

void main() {
    // Transform vertex
    gl_Position =  MVP * vec4(vertexPosition, 1);
    
    // Pass vertex color to the fragment shader
    color = vertexColor;
}
)";

static std::string cubeFragmentShader = R"(
#version 330 core

in vec3 color;

out vec3 finalColor;

void main()
{
	finalColor = color;
}
)";
