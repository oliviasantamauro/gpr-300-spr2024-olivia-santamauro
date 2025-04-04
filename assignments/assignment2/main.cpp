#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <ew/shader.h>
#include <ew/model.h>
#include <ew/camera.h>
#include <ew/cameraController.h>
#include <ew/transform.h>
#include <ew/texture.h>
#include <ew/procGen.h>

#include<fb/framebuffer.h>
#include "glm/gtx/transform.hpp"

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
ew::Camera lightCam;

struct Material {
	float Ka = 1.0;
	float Kd = 0.5;
	float Ks = 0.5;
	float Shininess = 128;
}material;

struct Light
{
	glm::vec3 lightDirection = glm::vec3(-2.0f, 4.0f, -1.0f);
	glm::vec3 lightColor = glm::vec3(1.0);

}light;

//framebuffer
fb::FrameBuffer shadowMap;

//depth map
const int SHADOW_SIZE = 1024;
float near_plane = 1.0f, far_plane = 8.0f;
glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
glm::mat4 lightView = glm::lookAt(glm::vec3(-2.0f, 4.0f, -1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
float bias;


void renderShadow(ew::Shader& shader, glm::mat4 lightMat, ew::Transform transform, ew::Model model) {

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMap.fbo);
	{
		glViewport(0, 0, SHADOW_SIZE, SHADOW_SIZE);

		glEnable(GL_DEPTH_TEST);
		glClear(GL_DEPTH_BUFFER_BIT);
		shader.use();
		shader.setMat4("lightSpaceMatrix", lightMat);
		shader.setMat4("model", transform.modelMatrix());

		model.draw();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0, 0, screenWidth, screenHeight);

}

void renderModel(ew::Shader& shader, glm::mat4 lightMat, ew::Model model, GLuint texture, ew::Transform transform) {

	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, shadowMap.depthBuffer);

	shader.use();

	shader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
	shader.setMat4("_LightSpaceMatrix", lightMat);
	shader.setVec3("_EyePos", camera.position);
	shader.setMat4("_Model", transform.modelMatrix());
	shader.setFloat("_Material.Ka", material.Ka);
	shader.setFloat("_Material.Kd", material.Kd);
	shader.setFloat("_Material.Ks", material.Ks);
	shader.setFloat("_Material.Shininess", material.Shininess);
	shader.setFloat("_Bias", bias);
	shader.setInt("_MainTex", 0);
	shader.setInt("_ShadowMap", 1);

	model.draw();
}

void renderPlane(ew::Shader& shader, ew::Transform transform, ew::Mesh plane, glm::mat4 lightMat, GLuint texture, GLFWwindow* window) {

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, shadowMap.depthBuffer);

	shader.use();
	shader.setMat4("viewProjection", camera.projectionMatrix() * camera.viewMatrix());
	shader.setMat4("model", transform.modelMatrix());
	shader.setMat4("_LightSpaceMatrix", lightMat);
	shader.setFloat("_Material.Ka", material.Ka);
	shader.setFloat("_Material.Kd", material.Kd);
	shader.setFloat("_Material.Ks", material.Ks);
	shader.setFloat("_Material.Shininess", material.Shininess);
	shader.setFloat("_Bias", bias);
	shader.setVec3("_EyePos", camera.position);
	shader.setInt("_MainTex", 0);
	shader.setInt("_ShadowMap", 1);

	plane.draw();

	cameraController.move(window, &camera, deltaTime);
}

int main() {
	GLFWwindow* window = initWindow("Assignment 2", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	//shaders
	ew::Shader depthShader = ew::Shader("assets/depth.vert", "assets/depth.frag");
	ew::Shader shadowShader = ew::Shader("assets/shadow.vert", "assets/shadow.frag");
	ew::Shader planeShader = ew::Shader("assets/plane.vert", "assets/depth.frag");

	//suzanne
	ew::Model suzanne = ew::Model("assets/suzanne.obj");
	ew::Transform monkeyTransform;

	//plane
	ew::Mesh plane = ew::createPlane(10, 10, 10);
	ew::Transform planeTransform;
	planeTransform.position = glm::vec3(0.0, -2.0, 0.0);
	
	//shadow map
	shadowMap = fb::createShadowBuffer(SHADOW_SIZE, SHADOW_SIZE);

	//Camera
	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;

	lightCam.target = glm::vec3();
	lightCam.position = light.lightDirection;
	lightCam.orthographic = true;
	lightCam.orthoHeight = 10.0f;
	lightCam.aspectRatio = 1.0f;
	lightCam.fov = 60.0f;

	//Textures
	GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, brickTexture);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		glm::mat4 lightView = glm::lookAt(light.lightDirection, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		const glm::mat4 lightMat = lightProjection * lightView;

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));

		//RENDER
		glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderShadow(shadowShader, lightMat, monkeyTransform, suzanne);
		renderModel(depthShader, lightMat, suzanne, brickTexture, monkeyTransform);
		renderPlane(planeShader, planeTransform, plane, lightMat, brickTexture, window);

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
	ImGui::ColorEdit3("Background", (float*)&bgColor);
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
	if (ImGui::CollapsingHeader("Shadow Map")) {
		ImGui::DragFloat3("Light Position", &light.lightDirection.x);
		ImGui::SliderFloat("Bias", &bias, 0.001f, 0.1f);
	}
	ImGui::Image((ImTextureID)(intptr_t)shadowMap.depthBuffer, ImVec2(400, 300));

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