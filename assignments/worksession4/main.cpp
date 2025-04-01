#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>
#include <ew/shader.h>
#include <ew/model.h>
#include <ew/camera.h>
#include <ew/cameraController.h>
#include <ew/transform.h>
#include <ew/texture.h>

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
glm::vec3 bgColor = glm::vec3(0.6f, 0.8f, 0.92f);

//Camera
ew::Camera camera;
ew::CameraController cameraController;

struct Material {
	float Ka = 1.0;
	float Kd = 0.5;
	float Ks = 0.5;
	float Shininess = 128;
}material;

float roughness = 0.5f;
float metallic = 0.5f;

void render(ew::Shader shader, ew::Model model, GLuint texture, ew::Transform transform) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glEnable(GL_DEPTH_TEST);

		glClearColor(bgColor.r, bgColor.g, bgColor.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		shader.use();

		shader.setFloat("_Material.Ka", material.Ka);
		shader.setFloat("_Material.Kd", material.Kd);
		shader.setFloat("_Material.Ks", material.Ks);
		shader.setFloat("_Material.Shininess", material.Shininess);

		shader.setVec3("_EyePos", camera.position);
		shader.setInt("_MainTex", 0);
		shader.setMat4("model", transform.modelMatrix());
		shader.setMat4("viewProjection", camera.projectionMatrix() * camera.viewMatrix());

		shader.setFloat("roughness", roughness);
		shader.setFloat("metallic", metallic);
		model.draw();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
}

int main() {
	GLFWwindow* window = initWindow("Assignment 0", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	ew::Shader litShader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Shader PBRshader = ew::Shader("assets/pbr.vert", "assets/pbr.frag");
	ew::Model suzanne = ew::Model("assets/suzanne.obj");
	ew::Transform monkeyTransform;

	//Culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	//Camera
	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;

	//Textures
	GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, brickTexture);


	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;
		cameraController.move(window, &camera, deltaTime);

		monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));

		//RENDER
		render(PBRshader, suzanne, brickTexture, monkeyTransform);

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

	if (ImGui::CollapsingHeader("Material")) {
		ImGui::SliderFloat("AmbientK", &material.Ka, 0.0f, 1.0f);
		ImGui::SliderFloat("DiffuseK", &material.Kd, 0.0f, 1.0f);
		ImGui::SliderFloat("SpecularK", &material.Ks, 0.0f, 1.0f);
		ImGui::SliderFloat("Shininess", &material.Shininess, 2.0f, 1024.0f);
	}

	if (ImGui::CollapsingHeader("PBR")) {
		ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f);
		ImGui::SliderFloat("metallic", &metallic, 0.0f, 1.0f);
	}

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

