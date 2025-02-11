#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>
#include <ew/shader.h>
#include <ew/model.h>
#include <ew/camera.h>
#include <ew/cameraController.h>
#include <ew/transform.h>
#include <ew/texture.h>
#include <vector>
#include <ew/procGen.h>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

//Camera
ew::Camera camera;
ew::CameraController cameraController;


//WindWaker
struct
{
	glm::vec3 color = glm::vec3(0.1f, 0.45f, 1.0f);
	float tiling = 5.0f;
	float speed = 0.5f;
	float b1 = 1.0f;
	float b2 = 0.15f;
	float strength = 1.0f;
}debug;


int main() {
	GLFWwindow* window = initWindow("Work Session 1", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	//Culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	//Camera
	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;

	ew::Shader waterShader = ew::Shader("assets/water.vert", "assets/water.frag");
	GLuint texture = ew::loadTexture("assets/Archive/windwaker/water.png");


	ew::Mesh plane = ew::createPlane(50.0f, 50.0f, 100);


	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		const auto viewProj = camera.projectionMatrix() * camera.viewMatrix();
		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;
		cameraController.move(window, &camera, deltaTime);

		//RENDER
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.6f, 0.8f, 0.92f, 1.0f);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		waterShader.use();

		waterShader.setMat4("model", glm::mat4(1.0f));
		waterShader.setMat4("viewProjection", viewProj);
		waterShader.setVec3("camera_pos", camera.position);
		waterShader.setFloat("uniform_strength", debug.strength);

		waterShader.setVec3("water_color", debug.color);
		waterShader.setInt("texture0", 0);
		waterShader.setFloat("tiling", debug.tiling);
		waterShader.setFloat("time", time * debug.speed);
		waterShader.setFloat("b1", debug.b1);
		waterShader.setFloat("b2", debug.b2);
		
		plane.draw();

		drawUI();

		glfwSwapBuffers(window);
	}
	printf("Shutting down...");
}

void resetCamera(ew::Camera* camera, ew::CameraController* controller) {
	camera->position = glm::vec3(0, 0, 5.0f);
	camera->target = glm::vec3(0);
	controller->yaw = controller->pitch = 0;
}


void drawUI() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();


	ImGui::Begin("Settings");
	if (ImGui::CollapsingHeader("Camera")) {
		if (ImGui::Button("Reset Camera")) {
			resetCamera(&camera, &cameraController);
		}
		ImGui::SliderFloat("FOV", &camera.fov, 0.0f, 120.0f);
	}
	ImGui::End();

	ImGui::Begin("Wind Waker");
	ImGui::ColorEdit3("Water Color", (float*)&debug.color);
	ImGui::SliderFloat("Tiling", &debug.tiling, 1.0f, 10.0f);
	ImGui::SliderFloat("Water Speed", &debug.speed, 0.0f, 1.0f);
	ImGui::SliderFloat("Foam Blend", &debug.b1, 0.0f, 1.0f);
	ImGui::SliderFloat("Shadow Blend", &debug.b2, 0.0f, 1.0f);
	ImGui::SliderFloat("Wave", &debug.strength, 0.0f, 5.0f);
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	screenWidth = width;
	screenHeight = height;
}

/// <summary>
/// Initializes GLFW, GLAD, and IMGUI
/// </summary>
/// <param name="title">Window title</param>
/// <param name="width">Window width</param>
/// <param name="height">Window height</param>
/// <returns>Returns window handle on success or null on fail</returns>
GLFWwindow* initWindow(const char* title, int width, int height) {
	printf("Initializing...");
	if (!glfwInit()) {
		printf("GLFW failed to init!");
		return nullptr;
	}

	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (window == NULL) {
		printf("GLFW failed to create window");
		return nullptr;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGL(glfwGetProcAddress)) {
		printf("GLAD Failed to load GL headers");
		return nullptr;
	}

	//Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	return window;
}

GLenum glCheckError_(const char* file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		//printf(error + " | " + file + " (" + line + ")");
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__) 