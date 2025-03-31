#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>
#include <ew/shader.h>
#include <ew/model.h>
#include <ew/camera.h>
#include <ew/cameraController.h>
#include <ew/transform.h>
#include <ew/texture.h>
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <ew/procGen.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>


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
glm::vec4 bgColor = glm::vec4(0.8f, 0.6f, 1.0f, 1.0f);

int count = 8;

struct Material {
	float Ka = 1.0;
	float Kd = 0.5;
	float Ks = 0.5;
	float Shininess = 128;
}material;

const int maxLights = 64;
struct PointLight {
	glm::vec3 position;
	float radius;
	glm::vec4 color;

	void initialize(int x, int z) {
		radius = 0.4f;
		color.r = (rand() % 256) / 255.0f;
		color.g = (rand() % 256) / 255.0f;
		color.b = (rand() % 256) / 255.0f;
		color.a = 1.0f;
		position = glm::vec3(2.0f * x, 3.0f, 2.0f * z);
	}
} pointLights[maxLights];

static float quad_vertices[] = {
	// pos (x, y) texcoord (u, v)
	-1.0f,  1.0f, 0.0f, 1.0f,
	-1.0f, -1.0f, 0.0f, 0.0f,
	 1.0f, -1.0f, 1.0f, 0.0f,

	-1.0f,  1.0f, 0.0f, 1.0f,
	 1.0f, -1.0f, 1.0f, 0.0f,
	 1.0f,  1.0f, 1.0f, 1.0f,
};

struct FullscreenQuad {
	GLuint vao;
	GLuint vbo;
	void intitialize() {
		glGenVertexArrays(1, &fullscreen_quad.vao);
		glBindVertexArray(fullscreen_quad.vao);
			glGenBuffers(1, &fullscreen_quad.vbo);
			glBindBuffer(GL_ARRAY_BUFFER, fullscreen_quad.vbo);

			//buffer data to vbo
			glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), &quad_vertices, GL_STATIC_DRAW);

			glEnableVertexAttribArray(0); //positions
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);
			glEnableVertexAttribArray(1); //tex coords
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));
		glBindVertexArray(0);
	}
} fullscreen_quad;

struct FrameBuffer {
	GLuint fbo;
	GLuint color0; //albedo
	GLuint color1; //position
	GLuint color2; //normal
	GLuint depth;
	void intitialize() {
		// initialize framebuffer
		glGenFramebuffers(1, &framebuffer.fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

			// color attachment
			glGenTextures(1, &framebuffer.color0);
			glBindTexture(GL_TEXTURE_2D, framebuffer.color0);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer.color0, 0);

			// color attachment
			glGenTextures(1, &framebuffer.color1);
			glBindTexture(GL_TEXTURE_2D, framebuffer.color1);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, framebuffer.color1, 0);

			// color attachment
			glGenTextures(1, &framebuffer.color2);
			glBindTexture(GL_TEXTURE_2D, framebuffer.color2);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, framebuffer.color2, 0);

			glGenRenderbuffers(1, &framebuffer.depth);
			glBindRenderbuffer(GL_RENDERBUFFER, framebuffer.depth);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenWidth, screenHeight);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebuffer.depth);

			GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
			glDrawBuffers(3, attachments);

			// check completeness
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
				printf("framebuffer incomplete\n");
			}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
} framebuffer;

void postProcessing(ew::Shader& shader, FrameBuffer& buffer) {
	glDisable(GL_DEPTH_TEST);
	glClearColor(bgColor.r, bgColor.g, bgColor.b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, buffer.color0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, buffer.color1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, buffer.color2);

	shader.use();

	shader.setInt("_Albedo", 0);
	shader.setInt("_Coords", 1);
	shader.setInt("_Normals", 2);
	shader.setVec3("_EyePos", camera.position);
	shader.setFloat("_Material.Ka", material.Ka);
	shader.setFloat("_Material.Kd", material.Kd);
	shader.setFloat("_Material.Ks", material.Ks);
	shader.setFloat("_Material.Shininess", material.Shininess);

	for (int i = 0; i < maxLights; i++) {
		//Creates prefix "_PointLights[0]." etc
		std::string prefix = "_PointLights[" + std::to_string(i) + "].";
		shader.setVec3(prefix + "position", pointLights[i].position);
		shader.setFloat(prefix + "radius", pointLights[i].radius);
		shader.setVec4(prefix + "color", pointLights[i].color);
	}

	glBindVertexArray(fullscreen_quad.vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void render(ew::Shader& shader, ew::Transform& transform, ew::Model& model, GLFWwindow* window, GLuint texture) {
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);

	//gfx pass
	glClearColor(bgColor.r, bgColor.g, bgColor.b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	shader.use();

	shader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
	shader.setVec3("_EyePos", camera.position);
	shader.setInt("_MainTex", 0);

	for (int i = 0; i < count; i++)
	{
		for (int j = 0; j < count; j++)
		{
			model.draw();
			shader.setMat4("_Model", glm::translate(glm::vec3(2.0f * i, 0, 2.0f * j)));
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderLight(ew::Shader& shader, ew::Mesh& light) {
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer.fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	shader.use();

	shader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
	for (int i = 0; i < count * count; i++)
	{
		glm::mat4 m = glm::mat4(1.0f);
		m = glm::translate(m, pointLights[i].position);
		m = glm::scale(m, glm::vec3(pointLights[i].radius));

		shader.setMat4("_Model", m);
		shader.setVec4("_LightColor", pointLights[i].color);
		light.draw();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


int main() {
	GLFWwindow* window = initWindow("Assignment 0", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);


	//Shader and asset creation
	//ew::Shader defaultShader = ew::Shader("assets/default.vert", "assets/default.frag");
	ew::Shader geoShader = ew::Shader("assets/geo.vert", "assets/geo.frag");
	ew::Shader litShader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Shader sphereShader = ew::Shader("assets/sphere.vert", "assets/sphere.frag");
	ew::Model suzanne = ew::Model("assets/suzanne.obj");
	ew::Transform monkeyTransform;
	ew::Mesh sphereMesh = ew::Mesh(ew::createSphere(1.0f, 8));
	GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");

	//Culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	//Camera
	camera.position = glm::vec3(0.0f, 5.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;

	framebuffer.intitialize();
	fullscreen_quad.intitialize();

	int lightIndex = 0;
	for (int i = 0; i < count; i++) {
		for (int j = 0; j < count; j++) {
				pointLights[lightIndex].initialize(i, j);
				lightIndex++;
		}
	}


	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;
		cameraController.move(window, &camera, deltaTime);

		//RENDER
		render(geoShader, monkeyTransform, suzanne, window, brickTexture);
		postProcessing(litShader, framebuffer);
		renderLight(sphereShader, sphereMesh);

		drawUI();

		glfwSwapBuffers(window);
	}
	printf("Shutting down...");
}

void resetCamera(ew::Camera* camera, ew::CameraController* controller) {
	camera->position = glm::vec3(0, 5.0f, 5.0f);
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
	if (ImGui::CollapsingHeader("So Many Suzannes")) {
		ImGui::SliderInt("Suzanne Count", &count, 1, 50);
		ImGui::Image((ImTextureID)(intptr_t)framebuffer.color0, ImVec2(400, 300));
		ImGui::Image((ImTextureID)(intptr_t)framebuffer.color1, ImVec2(400, 300));
		ImGui::Image((ImTextureID)(intptr_t)framebuffer.color2, ImVec2(400, 300));
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

