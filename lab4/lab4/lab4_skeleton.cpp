#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

// GLTF model loader
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

#include <render/shader.h>

#include <vector>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#include <iomanip>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

static GLFWwindow *window;
static int windowWidth = 1024;
static int windowHeight = 768;

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

// Camera
static glm::vec3 eye_center(0.0f, 100.0f, 300.0f);
static glm::vec3 lookat(0.0f, 0.0f, 0.0f);
static glm::vec3 up(0.0f, 1.0f, 0.0f);
static float FoV = 45.0f;
static float zNear = 100.0f;
static float zFar = 1500.0f;

// Lighting
static glm::vec3 lightIntensity(5e6f, 5e6f, 5e6f);
static glm::vec3 lightPosition(-275.0f, 500.0f, 800.0f);

// Animation
static bool playAnimation = true;
static float playbackSpeed = 2.0f;

// Helper class to render skeleton based on given animation data
struct Skeleton {

	const std::string vertexShader = R"(
	#version 330 core

	layout(location = 0) in vec3 vertexPosition;

	uniform mat4 MVP;

	void main() {
		gl_Position =  MVP * vec4(vertexPosition, 1);
	}
	)";

	const std::string fragmentShader = R"(
	#version 330 core

	out vec3 finalColor;

	void main()
	{
		finalColor = vec3(0.2, 0.8, 0);
	}
	)";

	GLuint programID;
    GLuint mvpMatrixID;
	GLuint sphereVAO, sphereVBO, sphereEBO;
	int sphereIndexCount = 0;

	void initialize() {
		createSphereMesh(1.0f, 8, 8);

        programID = LoadShadersFromString(vertexShader, fragmentShader);
		if (programID == 0)
		{
			std::cerr << "Failed to load shaders." << std::endl;
		}

		mvpMatrixID = glGetUniformLocation(programID, "MVP");
	}

	void createSphereMesh(float radius, int sectorCount, int stackCount) {
		std::vector<GLfloat> vertices;
		std::vector<GLuint> indices;

		float x, y, z, xy;                            // vertex position
		float sectorStep = 2 * M_PI / sectorCount;
		float stackStep = M_PI / stackCount;
		float sectorAngle, stackAngle;

		for (int i = 0; i <= stackCount; ++i) {
			stackAngle = M_PI / 2 - i * stackStep;    // from pi/2 to -pi/2
			xy = radius * cosf(stackAngle);           // r * cos(u)
			z = radius * sinf(stackAngle);            // r * sin(u)

			for (int j = 0; j <= sectorCount; ++j) {
				sectorAngle = j * sectorStep;         // from 0 to 2pi
				x = xy * cosf(sectorAngle);           // x = r * cos(u) * cos(v)
				y = xy * sinf(sectorAngle);           // y = r * cos(u) * sin(v)

				vertices.push_back(x);
				vertices.push_back(y);
				vertices.push_back(z);
			}
		}

		for (int i = 0; i < stackCount; ++i) {
			int k1 = i * (sectorCount + 1);
			int k2 = k1 + sectorCount + 1;

			for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
				if (i != 0) {
					indices.push_back(k1);
					indices.push_back(k2);
					indices.push_back(k1 + 1);
				}

				if (i != (stackCount - 1)) {
					indices.push_back(k1 + 1);
					indices.push_back(k2);
					indices.push_back(k2 + 1);
				}
			}
		}

		sphereIndexCount = indices.size();

		glGenVertexArrays(1, &sphereVAO);
		glGenBuffers(1, &sphereVBO);
		glGenBuffers(1, &sphereEBO);

		glBindVertexArray(sphereVAO);

		glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glBindVertexArray(0);
	}

	void renderSphere(const glm::vec3& position, float radius, const glm::mat4& viewProjMatrix) {
		glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, position);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(radius));

        glm::mat4 mvp = viewProjMatrix * modelMatrix;

		glUseProgram(programID);
		glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

		glBindVertexArray(sphereVAO);
		glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	void renderLine(const glm::vec3& start, const glm::vec3& end, const glm::mat4& viewProjMatrix) {
		glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &viewProjMatrix[0][0]);

		GLfloat vertices[] = { start.x, start.y, start.z, end.x, end.y, end.z };
		GLuint VBO, VAO;
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glUseProgram(programID);
		glBindVertexArray(VAO);
		glDrawArrays(GL_LINES, 0, 2);

		glBindVertexArray(0);
		glDeleteBuffers(1, &VBO);
		glDeleteVertexArrays(1, &VAO);
	}

	void renderSkeleton(const tinygltf::Model& model,
        const tinygltf::Skin &skin,
        const std::vector<glm::mat4> &globalTransforms,
        const glm::mat4& viewProjMatrix)
    {
        if (skin.joints.size() <= 0) return;

		for (size_t j = 0; j < skin.joints.size(); ++j) {
            int nodeIndex = skin.joints[j];
            const tinygltf::Node &node = model.nodes[nodeIndex];
			if (node.translation.size() >= 0) {
                glm::vec3 jointPosition =
                    glm::vec3(globalTransforms[nodeIndex][3]);
				renderSphere(jointPosition, 2.0f, viewProjMatrix);

				for (const int childIndex : node.children) {
					const auto& childNode = model.nodes[childIndex];
					glm::vec3 childPosition =
                        glm::vec3(globalTransforms[childIndex][3]);
					renderLine(jointPosition, childPosition, viewProjMatrix);
				}
			}
		}
	}
};

// Our 3D character model
struct MyBot {
	tinygltf::Model model;

	// Animation data
	struct SamplerObject {
		std::vector<float> input;
		std::vector<glm::vec4> output;
		int interpolation;
	};
	struct ChannelObject {
		int sampler;
		std::string targetPath;
		int targetNode;
	};
	struct AnimationObject {
		std::vector<SamplerObject> samplers;
	};
	std::vector<AnimationObject> animationObjects;

	// The skeleton class for rendering the skeleton given the transforms
	// obtained from the animation object
    Skeleton skeleton;

	// The current global transforms for each joint
	// Update this will result in skeleton in different poses
    std::vector<glm::mat4> globalTransforms;

	// Parse the transformation data at a node and turn into a 4x4 matrix
	glm::mat4 getNodeTransform(const tinygltf::Node& node) {
		glm::mat4 transform(1.0f);

		if (node.matrix.size() == 16) {
			transform = glm::make_mat4(node.matrix.data());
		} else {
			if (node.translation.size() == 3) {
				transform = glm::translate(transform, glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
			}
			if (node.rotation.size() == 4) {
				glm::quat q(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
				transform *= glm::mat4_cast(q);
			}
			if (node.scale.size() == 3) {
				transform = glm::scale(transform, glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
			}
		}
		return transform;
	}

	void computeLocalNodeTransform(const tinygltf::Model& model,
		int nodeIndex,
		std::vector<glm::mat4> &localTransforms)
	{
		// TODO: your code here
		// model.nodes
		// localTransforms[nodeIndex]
 		glm::vec3 translation(0.0f);
	    glm::quat rotation(1.0f, 0.0f, 0.0f, 0.0f); // Identity quaternion
	    glm::vec3 scale(1.0f);

		const tinygltf::Node &node = model.nodes[nodeIndex];

		// Extract translation if present
		if (!node.translation.empty()) {
			translation = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);
		}

		// Extract rotation if present
		if (!node.rotation.empty()) {
			rotation = glm::quat(
				node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
		}

		// Extract scale if present
		if (!node.scale.empty()) {
			scale = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);
		}

		// Compute the transformation matrices
		glm::mat4 T = glm::translate(glm::mat4(1.0f), translation);
		glm::mat4 R = glm::mat4_cast(rotation); // Convert quaternion to rotation matrix
		glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);

		localTransforms[nodeIndex] = T * R * S;
	}

	void computeGlobalNodeTransform(const tinygltf::Model& model,
		const std::vector<glm::mat4> &localTransforms,
		int nodeIndex, const glm::mat4& parentTransform,
		std::vector<glm::mat4> &globalTransforms)
	{
		// std::cout << "Processing Node:" << nodeIndex << std::endl;

		// TODO: your code here
		globalTransforms[nodeIndex] = parentTransform * localTransforms[nodeIndex];

		// Now go to the children nodes if the nodes are not empty
		const tinygltf::Node &node = model.nodes[nodeIndex];
		if(!node.children.empty()){
			for(int childNode: node.children){
				computeGlobalNodeTransform(model,localTransforms,childNode,globalTransforms[nodeIndex],globalTransforms);
			}
		}
	}

	int findKeyframeIndex(const std::vector<float>& times, float animationTime)
	{
		int left = 0;
		int right = times.size() - 1;

		while (left <= right) {
			int mid = (left + right) / 2;

			if (mid + 1 < times.size() && times[mid] <= animationTime && animationTime < times[mid + 1]) {
				return mid;
			}
			else if (times[mid] > animationTime) {
				right = mid - 1;
			}
			else { // animationTime >= times[mid + 1]
				left = mid + 1;
			}
		}

		// Target not found
		return times.size() - 2;
	}

	std::vector<AnimationObject> prepareAnimation(const tinygltf::Model &model)
	{
		std::vector<AnimationObject> animationObjects;
		for (const auto &anim : model.animations) {
			AnimationObject animationObject;

			for (const auto &sampler : anim.samplers) {
				SamplerObject samplerObject;

				const tinygltf::Accessor &inputAccessor = model.accessors[sampler.input];
				const tinygltf::BufferView &inputBufferView = model.bufferViews[inputAccessor.bufferView];
				const tinygltf::Buffer &inputBuffer = model.buffers[inputBufferView.buffer];

				assert(inputAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
				assert(inputAccessor.type == TINYGLTF_TYPE_SCALAR);

				// Input (time) values
				samplerObject.input.resize(inputAccessor.count);

				const unsigned char *inputPtr = &inputBuffer.data[inputBufferView.byteOffset + inputAccessor.byteOffset];
				const float *inputBuf = reinterpret_cast<const float*>(inputPtr);

				// Read input (time) values
				int stride = inputAccessor.ByteStride(inputBufferView);
				for (size_t i = 0; i < inputAccessor.count; ++i) {
					samplerObject.input[i] = *reinterpret_cast<const float*>(inputPtr + i * stride);
				}

				const tinygltf::Accessor &outputAccessor = model.accessors[sampler.output];
				const tinygltf::BufferView &outputBufferView = model.bufferViews[outputAccessor.bufferView];
				const tinygltf::Buffer &outputBuffer = model.buffers[outputBufferView.buffer];

				assert(outputAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

				const unsigned char *outputPtr = &outputBuffer.data[outputBufferView.byteOffset + outputAccessor.byteOffset];
				const float *outputBuf = reinterpret_cast<const float*>(outputPtr);

				int outputStride = outputAccessor.ByteStride(outputBufferView);

				// Output values
				samplerObject.output.resize(outputAccessor.count);

				for (size_t i = 0; i < outputAccessor.count; ++i) {

					if (outputAccessor.type == TINYGLTF_TYPE_VEC3) {
						memcpy(&samplerObject.output[i], outputPtr + i * 3 * sizeof(float), 3 * sizeof(float));
					} else if (outputAccessor.type == TINYGLTF_TYPE_VEC4) {
						memcpy(&samplerObject.output[i], outputPtr + i * 4 * sizeof(float), 4 * sizeof(float));
					} else {
						std::cout << "Unsupport accessor type ..." << std::endl;
					}

				}

				animationObject.samplers.push_back(samplerObject);
			}

			animationObjects.push_back(animationObject);
		}
		return animationObjects;
	}

	void updateAnimation(
		const tinygltf::Model &model,
		const tinygltf::Animation &anim,
		const AnimationObject &animationObject,
		float time,
		std::vector<glm::mat4> &nodeTransforms)
	{
		// There are many channels so we have to accumulate the transforms
		for (const auto &channel : anim.channels) {

			int targetNodeIndex = channel.target_node;
			const auto &sampler = anim.samplers[channel.sampler];

			// Access output (value) data for the channel
			const tinygltf::Accessor &outputAccessor = model.accessors[sampler.output];
			const tinygltf::BufferView &outputBufferView = model.bufferViews[outputAccessor.bufferView];
			const tinygltf::Buffer &outputBuffer = model.buffers[outputBufferView.buffer];

			// Calculate current animation time (wrap if necessary)
			const std::vector<float> &times = animationObject.samplers[channel.sampler].input;
			float animationTime = fmod(time, times.back());

			// ----------------------------------------------------------
			// TODO: Find a keyframe for getting animation data
			// ----------------------------------------------------------
			int keyframeIndex = 0;

			const unsigned char *outputPtr = &outputBuffer.data[outputBufferView.byteOffset + outputAccessor.byteOffset];
			const float *outputBuf = reinterpret_cast<const float*>(outputPtr);

			// -----------------------------------------------------------
			// TODO: Add interpolation for smooth interpolation
			// -----------------------------------------------------------
			if (channel.target_path == "translation") {
				glm::vec3 translation0, translation1;
				memcpy(&translation0, outputPtr + keyframeIndex * 3 * sizeof(float), 3 * sizeof(float));

				glm::vec3 translation = translation0;
				nodeTransforms[targetNodeIndex] = glm::translate(nodeTransforms[targetNodeIndex], translation);
			} else if (channel.target_path == "rotation") {
				glm::quat rotation0, rotation1;
				memcpy(&rotation0, outputPtr + keyframeIndex * 4 * sizeof(float), 4 * sizeof(float));

				glm::quat rotation = rotation0;
				nodeTransforms[targetNodeIndex] *= glm::mat4_cast(rotation);
			} else if (channel.target_path == "scale") {
				glm::vec3 scale0, scale1;
				memcpy(&scale0, outputPtr + keyframeIndex * 3 * sizeof(float), 3 * sizeof(float));

				glm::vec3 scale = scale0;
				nodeTransforms[targetNodeIndex] = glm::scale(nodeTransforms[targetNodeIndex], scale);
			}

			// inverse
		}
	}

	void update(float time) {
		// TODO:
		// return;	// Do nothing for T-pose, comment out for animation

		// std::cout << time << std::endl;


		if (model.animations.size() > 0) {
			const tinygltf::Animation &animation = model.animations[0];
			const AnimationObject &animationObject = animationObjects[0];

            const tinygltf::Skin &skin = model.skins[0];
			std::vector<glm::mat4> nodeTransforms(skin.joints.size());
			for (size_t i = 0; i < nodeTransforms.size(); ++i) {
				nodeTransforms[i] = glm::mat4(1.0);
			}

			updateAnimation(model, animation, animationObject, time, nodeTransforms);

			// ----------------------------------------------
            // TODO: Recompute global transforms at each node
			// using the updated node transforms above
            // ----------------------------------------------
		}
	}

	bool loadModel(tinygltf::Model &model, const char *filename) {
		tinygltf::TinyGLTF loader;
		std::string err;
		std::string warn;

		bool res = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
		if (!warn.empty()) {
			std::cout << "WARN: " << warn << std::endl;
		}

		if (!err.empty()) {
			std::cout << "ERR: " << err << std::endl;
		}

		if (!res)
			std::cout << "Failed to load glTF: " << filename << std::endl;
		else
			std::cout << "Loaded glTF: " << filename << std::endl;

		return res;
	}

	void initialize() {
		// Modify your path if needed
		if (!loadModel(model, "../lab4/model/bot/bot.gltf")) {
			return;
		}

		// Prepare animation data
		animationObjects = prepareAnimation(model);

		// Just take the first skin/skeleton model
		const tinygltf::Skin &skin = model.skins[0];

		std::vector<glm::mat4> localNodeTransforms(skin.joints.size());

		glm::mat4 parentTransform(1.0f);
		std::vector<glm::mat4> globalNodeTransforms(skin.joints.size());

        // Compute local transforms at each node
		for(int i=0;i<skin.joints.size();i++){
			int rootNodeIndex = skin.joints[i];
			// std::cout << rootNodeIndex << std::endl;
			computeLocalNodeTransform(model, i, localNodeTransforms);
		}

		// Compute global transforms at each node
		computeGlobalNodeTransform(model, localNodeTransforms, 24, parentTransform, globalNodeTransforms);
		globalTransforms = globalNodeTransforms;


        skeleton.initialize();
	}

	void render(glm::mat4 cameraMatrix) {

        const tinygltf::Skin &skin = model.skins[0];
        skeleton.renderSkeleton(model, skin, globalTransforms, cameraMatrix);

	}

	void cleanup() {

	}
};

int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW." << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MacOS
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(windowWidth, windowHeight, "Lab 4", NULL, NULL);
	if (window == NULL)
	{
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
	if (version == 0)
	{
		std::cerr << "Failed to initialize OpenGL context." << std::endl;
		return -1;
	}

	// Background
	glClearColor(0.2f, 0.2f, 0.25f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// Our 3D character
	MyBot bot;
	bot.initialize();

	// Camera setup
    glm::mat4 viewMatrix, projectionMatrix;
	projectionMatrix = glm::perspective(glm::radians(FoV), (float)windowWidth / windowHeight, zNear, zFar);

	// Time and frame rate tracking
	static double lastTime = glfwGetTime();
	float time = 0.0f;			// Animation time
	float fTime = 0.0f;			// Time for measuring fps
	unsigned long frames = 0;

	// Main loop
	do
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Update states for animation
        double currentTime = glfwGetTime();
        float deltaTime = float(currentTime - lastTime);
		lastTime = currentTime;

		if (playAnimation) {
			time += deltaTime * playbackSpeed;
			bot.update(time);
		}

		// Rendering
		viewMatrix = glm::lookAt(eye_center, lookat, up);
		glm::mat4 vp = projectionMatrix * viewMatrix;
		bot.render(vp);

		// FPS tracking
		// Count number of frames over a few seconds and take average
		frames++;
		fTime += deltaTime;
		if (fTime > 2.0f) {
			float fps = frames / fTime;
			frames = 0;
			fTime = 0;

			std::stringstream stream;
			stream << std::fixed << std::setprecision(2) << "Lab 4 | Frames per second (FPS): " << fps;
			glfwSetWindowTitle(window, stream.str().c_str());
		}

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (!glfwWindowShouldClose(window));

	// Clean up
	bot.cleanup();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_UP && action == GLFW_PRESS)
	{
		playbackSpeed += 1.0f;
		if (playbackSpeed > 10.0f)
			playbackSpeed = 10.0f;
	}

	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
	{
		playbackSpeed -= 1.0f;
		if (playbackSpeed < 1.0f) {
			playbackSpeed = 1.0f;
		}
	}

	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		playAnimation = !playAnimation;
	}

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}
