#version 330 core

// Input
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;

// Output data, to be interpolated for each fragment
out vec3 color;

// TODO: To add UV to this vertex shader

// Matrix for vertex transformation
uniform mat4 MVP;

void main() {
    // Transform vertex
    gl_Position =  MVP * vec4(vertexPosition, 1);

    // Pass vertex color to the fragment shader
    color = vertexColor;

    // TODO: Pass UV to the fragment shader
}
