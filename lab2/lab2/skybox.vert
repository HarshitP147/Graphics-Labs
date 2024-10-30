#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec2 vertexUV;


out vec3 color;
out vec2 uv;

uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(vertexPosition, 1);
    color = vertexColor;

    uv = vertexUV;
}
