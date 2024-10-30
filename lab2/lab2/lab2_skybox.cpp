#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <render/shader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <iostream>
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>

// In this lab we store our GLSL shaders as C++ string in a header file and load them directly instead of reading them from files

static GLFWwindow *window;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float yaw = 90.0f;          // Horizontal rotation (starts facing along -Z axis)
float pitch = 0.0f;         // Vertical rotation
float rotationSpeed = 1.0f; // Adjust for comfortable rotation speed

glm::vec3 calculateCameraFront() {
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    return glm::normalize(front);
}

float cameraSpeed = 0.05f; // Adjust this for speed

glm::float32 FoV = 90;
glm::float32 zNear = 0.1f;
glm::float32 zFar = 1000.0f;

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        std::cout << "Reset." << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        yaw += rotationSpeed; // Turn right
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        yaw -= rotationSpeed; // Turn left
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        pitch += rotationSpeed; // Look up
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        pitch -= rotationSpeed; // Look down

    // Clamp pitch to prevent flipping
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

static GLuint LoadTextureTileBox(const char *texture_file_path) {
    int w, h, channels;
    uint8_t *img = stbi_load(texture_file_path, &w, &h, &channels, 3);
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    // To tile textures on a box, we set wrapping to repeat
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);

    if (img) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture " << texture_file_path << std::endl;
    }
    stbi_image_free(img);

    return texture;
}

struct SkyBox {

    GLfloat vertex_buffer_data[72] = {
        // Vertex definition for a canonical box
        // CC from top right

        // Back face
        1.0f, 1.0f, 1.0f,   // right top front
        -1.0f, 1.0f, 1.0f,  // left top front
        -1.0f, -1.0f, 1.0f, // left bottom front
        1.0f, -1.0f, 1.0f,  // right bottom front

        // Front face
        1.0f, -1.0f, -1.0f,  // right bottom back
        -1.0f, -1.0f, -1.0f, // left bottom back
        -1.0f, 1.0f, -1.0f,  // left top back
        1.0f, 1.0f, -1.0f,   // right top back

        // Left face
        -1.0f, -1.0f, -1.0f, // left bottom back
        -1.0f, -1.0f, 1.0f,  // right bottom front
        -1.0f, 1.0f, 1.0f,   // right top front
        -1.0f, 1.0f, -1.0f,  // left top back

        // Right face
        1.0f, 1.0f, 1.0f,   // right top front
        1.0f, -1.0f, 1.0f,  // right bottom front
        1.0f, -1.0f, -1.0f, // right bottom back
        1.0f, 1.0f, -1.0f,  // right top back

        // Top face
        1.0f, 1.0f, 1.0f,   // right top front
        1.0f, 1.0f, -1.0f,  // right top back
        -1.0f, 1.0f, -1.0f, // left top back
        -1.0f, 1.0f, 1.0f,  // left top front

        // Bottom face
        1.0f, -1.0f, -1.0f,  // right bottom back
        1.0f, -1.0f, 1.0f,   // right bottom front
        -1.0f, -1.0f, 1.0f,  // left bottom front
        -1.0f, -1.0f, -1.0f, // left bottom back
    };

    GLfloat color_buffer_data[72] = {
        // Front, red
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,

        // Back, yellow
        1.0f,
        1.0f,
        0.0f,
        1.0f,
        1.0f,
        0.0f,
        1.0f,
        1.0f,
        0.0f,
        1.0f,
        1.0f,
        0.0f,

        // Left, green
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,

        // Right, cyan
        0.0f,
        1.0f,
        1.0f,
        0.0f,
        1.0f,
        1.0f,
        0.0f,
        1.0f,
        1.0f,
        0.0f,
        1.0f,
        1.0f,

        // Top, blue
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,

        // Bottom, magenta
        1.0f,
        0.0f,
        1.0f,
        1.0f,
        0.0f,
        1.0f,
        1.0f,
        0.0f,
        1.0f,
        1.0f,
        0.0f,
        1.0f,
    };

    GLuint index_buffer_data[36] = {
        // 12 triangle faces of a box
        0,
        1,
        2,
        0,
        2,
        3,

        4,
        5,
        6,
        4,
        6,
        7,

        8,
        9,
        10,
        8,
        10,
        11,

        12,
        13,
        14,
        12,
        14,
        15,

        16,
        17,
        18,
        16,
        18,
        19,

        20,
        21,
        22,
        20,
        22,
        23,
    };

    GLfloat uv_buffer_data[48] = {
        // Back (-Z) - Third square from the left on the bottom row
        3.0f / 4.0f, 1.0f / 3.0f, // Top-left
        1.0f, 1.0f / 3.0f,        // Top-right
        1.0f, 2.0f / 3.0f,        // Bottom-right
        3.0f / 4.0f, 2.0f / 3.0f, // Bottom-left

        // Front (+Z)
        2.0f / 4.0f, 2.0f / 3.0f, // Top-right
        1.0f / 4.0f, 2.0f / 3.0f, // Bottom-right
        1.0f / 4.0f, 1.0f / 3.0f, // Bottom-left
        2.0f / 4.0f, 1.0f / 3.0f, // Top-left

        // Left (+X)
        1.0f / 4.0f, 2.0f / 3.0f, // Top right
        0.0f, 2.0f / 3.0f,        // top left
        0.0f, 1.0f / 3.0f,        // bottom left
        1.0f / 4.0f, 1.0f / 3.0f, // bottom right

        // Right (-X) - Fourth square from the left on the bottom row
        3.0f / 4.0f, 1.0f / 3.0f, // Top-right
        3.0f / 4.0f, 2.0f / 3.0f, // Bottom-right
        2.0f / 4.0f, 2.0f / 3.0f, // Bottom-left
        2.0f / 4.0f, 1.0f / 3.0f, // Top-left

        // Top (+Y)
        2.0f / 4.0f, 0.0f,        // Top-right
        2.0f / 4.0f, 1.0f / 3.0f, // Bottom-right
        1.0f / 4.0f, 1.0f / 3.0f, // Bottom-left
        1.0f / 4.0f, 0.0f,        // Top-left

        // Bottom (-Y)
        2.0f / 4.0f, 2.0f / 3.0f, // Top-right
        2.0f / 4.0f, 1.0f,        // Bottom-right
        1.0f / 4.0f, 1.0f,        // Bottom-left
        1.0f / 4.0f, 2.0f / 3.0f, // Top-left
    };

    // OpenGL buffers
    GLuint vertexArrayID;
    GLuint vertexBufferID;
    GLuint indexBufferID;
    GLuint colorBufferID;
    GLuint uvBufferID;
    GLuint textureID;

    // Shader variable IDs
    GLuint mvpMatrixID;
    GLuint textureSamplerID;
    GLuint programID;

    void initialize(const char *texturePath) {
        // Create a vertex array object
        glGenVertexArrays(1, &vertexArrayID);
        glBindVertexArray(vertexArrayID);

        // Create a vertex buffer object to store the vertex data
        glGenBuffers(1, &vertexBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

        // Create a vertex buffer object to store the color data
        glGenBuffers(1, &colorBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data), color_buffer_data, GL_STATIC_DRAW);

        // create a vertex buffer ohject to store the UV data
        glGenBuffers(1, &uvBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_data), uv_buffer_data, GL_STATIC_DRAW);

        // Create an index buffer object to store the index data that defines triangle faces
        glGenBuffers(1, &indexBufferID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_STATIC_DRAW);

        // Create and compile our GLSL program from the shaders
        programID = LoadShadersFromFile("../lab2/skybox.vert", "../lab2/skybox.frag");
        if (programID == 0) {
            std::cerr << "Failed to load shaders." << std::endl;
        }

        // Get a handle for our "MVP" uniform
        mvpMatrixID = glGetUniformLocation(programID, "MVP");

        // Load the texture
        textureID = LoadTextureTileBox(texturePath);

        // Get a handle to texture sampler
        textureSamplerID = glGetUniformLocation(programID, "textureSampler");
    }

    void render(glm::mat4 cameraMatrix) {

        glUseProgram(programID);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

        // TODO: Model transform
        // ------------------------------------
        glm::mat4 modelMatrix = glm::mat4();

        // TODO: Set model-view-projection matrix
        glm::mat4 mvp = cameraMatrix * modelMatrix;

        // ------------------------------------
        glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

        // Enable the UV buffer and texture sampler
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

        // Draw the box
        glDrawElements(
            GL_TRIANGLES,    // mode
            36,              // number of indices
            GL_UNSIGNED_INT, // type
            (void *)0        // element array buffer offset
        );

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
    }

    void cleanup() {
        glDeleteBuffers(1, &vertexBufferID);
        glDeleteBuffers(1, &colorBufferID);
        glDeleteBuffers(1, &indexBufferID);
        glDeleteVertexArrays(1, &vertexArrayID);
        glDeleteBuffers(1, &uvBufferID);
        glDeleteBuffers(1, &textureID);
        glDeleteProgram(programID);
    }
};

int main(void) {
    // Initialise GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW." << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MacOS
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(1024, 768, "Lab 2", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to open a GLFW window." << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetKeyCallback(window, key_callback);

    // Load OpenGL functions, gladLoadGL returns the loaded version, 0 on error.
    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        std::cerr << "Failed to initialize OpenGL context." << std::endl;
        return -1;
    }

    // Background
    glClearColor(0.2f, 0.2f, 0.2f, 0.f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // A default box
    SkyBox bgBox;
    const char *texturePath = "../lab2/sky.png";
    bgBox.initialize(texturePath);

    // ------------------------------------
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, zNear, zFar);
    // ------------------------------------

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

    do {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Calculate new camera front direction
        glm::vec3 cameraFront = calculateCameraFront();
        glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 0.0f); // Fixed at origin

        // Update the view matrix (camera at origin, looking along cameraFront)
        glm::mat4 viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        // For convenience, we multiply the projection and view matrix together and pass a single matrix for rendering
        glm::mat4 vp = projectionMatrix * viewMatrix;

        // Render the box
        bgBox.render(vp);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while (!glfwWindowShouldClose(window));

    bgBox.cleanup();

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}
