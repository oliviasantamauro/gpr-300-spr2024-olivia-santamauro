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

//lit shader
struct Material {
	float Ka = 1.0;
	float Kd = 0.5;
	float Ks = 0.5;
	float Shininess = 128;
}material;


//Framebuffer
struct FrameBuffer {
	GLuint fbo;
	GLuint color0;
	GLuint brightness;
	GLuint depth;
} framebuffer;

struct PingPongBuffer {
	GLuint fbo[2];
	GLuint color[2];
} pingpongbuffer;


// Post-Processing variables
float exposure = 0.1f;
float chromatic = 1.0f;
int amount = 10;
float sharpness = 1.0f;
float detection = 0.2f;
glm::vec3 edgeColor = glm::vec3(1.0f, 1.0f, 1.0f);
float vigStrength = 0.2f;
float vigAmount = 1.0f;

struct FullscreenQuad {
	GLuint vao;
	GLuint vbo;
} fullscreen_quad;

static float quad_vertices[] = {
	// pos (x, y) texcoord (u, v)
	-1.0f,  1.0f, 0.0f, 1.0f,
	-1.0f, -1.0f, 0.0f, 0.0f,
	 1.0f, -1.0f, 1.0f, 0.0f,

	-1.0f,  1.0f, 0.0f, 1.0f,
	 1.0f, -1.0f, 1.0f, 0.0f,
	 1.0f,  1.0f, 1.0f, 1.0f,
};

static int effect_index = 0;
static std::vector<std::string> post_processing_effects = {
	"None",
	"Grayscale",
	"Blur",
	"Inverse",
	"Chromatic Aberration",
	"HDR",
	"Bloom", //i hate you
	"Sharpness",
	"Edge Detection",
	"Vignette",
};

void render(ew::Shader shader, ew::Model model, GLuint texture, ew::Transform transform) {
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);
	{
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
		model.draw();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


int main() {
	GLFWwindow* window = initWindow("Assignment 1", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	ew::Shader litShader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Shader fullscreenShader = ew::Shader("assets/fullscreen.vert", "assets/fullscreen.frag");
	ew::Shader inverseShader = ew::Shader("assets/fullscreen.vert", "assets/inverse.frag");
	ew::Shader grayscaleShader = ew::Shader("assets/fullscreen.vert", "assets/greyscale.frag");
	ew::Shader blurShader = ew::Shader("assets/fullscreen.vert", "assets/blur.frag");
	ew::Shader chromaticShader = ew::Shader("assets/fullscreen.vert", "assets/chromatic.frag");
	ew::Shader hdrShader = ew::Shader("assets/HDR.vert", "assets/HDR.frag");
	ew::Shader bloomShader = ew::Shader("assets/HDR.vert", "assets/bloom.frag");
	ew::Shader sharpnessShader = ew::Shader("assets/fullscreen.vert", "assets/sharpness.frag");
	ew::Shader edgeShader = ew::Shader("assets/fullscreen.vert", "assets/edge.frag");
	ew::Shader vignetteShader = ew::Shader("assets/fullscreen.vert", "assets/vignette.frag");
	ew::Model suzanne = ew::Model("assets/suzanne.obj");
	ew::Transform monkeyTransform;

	//Camera
	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;

	// initialize fullscreen quad
	glGenVertexArrays(1, &fullscreen_quad.vao);
	glBindVertexArray(fullscreen_quad.vao);
	{
		glGenBuffers(1, &fullscreen_quad.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, fullscreen_quad.vbo);

		//buffer data to vbo
		glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), &quad_vertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0); //positions
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);
		glEnableVertexAttribArray(1); //tex coords
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));
	}
	glBindVertexArray(0);


	// initialize framebuffer
	glGenFramebuffers(1, &framebuffer.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);
	{

		// color attachment
		glGenTextures(1, &framebuffer.color0);
		glBindTexture(GL_TEXTURE_2D, framebuffer.color0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer.color0, 0);
		
		// color attachment
		glGenTextures(1, &framebuffer.brightness);
		glBindTexture(GL_TEXTURE_2D, framebuffer.brightness);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, framebuffer.brightness, 0);

		GLuint attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
		glDrawBuffers(2, attachments);

		// check completeness
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			printf("framebuffer incomplete\n");
			return 0;
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// initialize pingpong buffer
	glGenFramebuffers(2, pingpongbuffer.fbo);
	glGenTextures(2, pingpongbuffer.color);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongbuffer.fbo[i]);
		glBindTexture(GL_TEXTURE_2D, pingpongbuffer.color[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongbuffer.color[i], 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			printf("framebuffer incomplete\n");
			return 0;
		}
	}

	//Textures
	GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");


	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;
		cameraController.move(window, &camera, deltaTime);

		monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));

		//RENDER to framebuffer
		render(litShader, suzanne, brickTexture, monkeyTransform);

		// render to default buffer
		glDisable(GL_DEPTH_TEST);
		glClearColor(bgColor.r, bgColor.g, bgColor.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// render fullscreen quad
		switch (effect_index)
		{
		case 1:
			grayscaleShader.use();
			grayscaleShader.setInt("texture0", 0);
			break;
		case 2:
			blurShader.use();
			blurShader.setInt("texture0", 0);
			break;
		case 3:
			inverseShader.use();
			inverseShader.setInt("texture0", 0);
			break;
		case 4:
			chromaticShader.use();
			chromaticShader.setInt("texture0", 0);
			chromaticShader.setFloat("strength", chromatic);
			break;
		case 5:
			hdrShader.use();
			hdrShader.setInt("texture0", 0);
			hdrShader.setFloat("exposure", exposure);
			break;
		case 6: {
			bool horizontal = true, first_iteration = true;
			blurShader.use();
			blurShader.setInt("texture0", 0);
			for (unsigned int i = 0; i < amount; i++)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, pingpongbuffer.fbo[horizontal]);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				// render to fullscreen quad
				glBindVertexArray(fullscreen_quad.vao);
				{
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, first_iteration ? framebuffer.brightness : pingpongbuffer.color[!horizontal]);
					glDrawArrays(GL_TRIANGLES, 0, 6);
				}
				glBindVertexArray(0);

				horizontal = !horizontal;
				if (first_iteration)
					first_iteration = false;
			}
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			// render bloom
			glBindVertexArray(fullscreen_quad.vao);
			bloomShader.use();
			bloomShader.setFloat("exposure", exposure);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, framebuffer.color0);
			bloomShader.setInt("scene", 0);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, pingpongbuffer.color[1]);
			bloomShader.setInt("bloomBlur", 1);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindVertexArray(0);
		} break;
		case 7:
			sharpnessShader.use();
			sharpnessShader.setInt("texture0", 0);
			sharpnessShader.setFloat("strength", sharpness);
			break;
		case 8:
			edgeShader.use();
			edgeShader.setInt("texture0", 0);
			edgeShader.setFloat("strength", detection);
			edgeShader.setVec3("edgeColor", edgeColor);
			break;
		case 9:
			vignetteShader.use();
			vignetteShader.setInt("texture0", 0);
			vignetteShader.setFloat("falloff", vigStrength);
			vignetteShader.setFloat("amount", vigAmount);
			break;
		default:
			fullscreenShader.use();
			fullscreenShader.setInt("texture0", 0);
			break;
		}

		if (effect_index != 6)
		{
			glBindVertexArray(fullscreen_quad.vao);
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, framebuffer.color0);
				glDrawArrays(GL_TRIANGLES, 0, 6);
			}
			glBindVertexArray(0);
		}

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
	ImGui::End();

	ImGui::Begin("Fullscreen");
	ImGui::Image((ImTextureID)(intptr_t)framebuffer.color0, ImVec2(800, 600));
	ImGui::Image((ImTextureID)(intptr_t)framebuffer.brightness, ImVec2(800, 600));
	ImGui::Image((ImTextureID)(intptr_t)pingpongbuffer.color[0], ImVec2(800, 600));
	ImGui::Image((ImTextureID)(intptr_t)pingpongbuffer.color[1], ImVec2(800, 600));
	ImGui::End();

	ImGui::Begin("Post-Processing Toggles");
		ImGui::SliderFloat("Exposure", &exposure, 0.0f, 1.0f);
		ImGui::SliderFloat("Chromatic Strength", &chromatic, 0.0f, 10.0f);
		ImGui::SliderInt("Bloom Intensity", &amount, 1, 20);
		ImGui::SliderFloat("Sharpness", &sharpness, 0.0f, 5.0f);
		ImGui::SliderFloat("Edge Detection Strength", &detection, 0.0f, 1.0f);
		ImGui::ColorEdit3("Edge Detection Color", (float*)&edgeColor);
		ImGui::SliderFloat("Vignette Strength", &vigStrength, 0.0f, 1.0f);
		ImGui::SliderFloat("Vignette Distance", &vigAmount, 0.0f, 10.0f);
	ImGui::End();

	if (ImGui::BeginCombo("Effect", post_processing_effects[effect_index].c_str()))
	{
		for (auto n = 0; n < post_processing_effects.size(); ++n)
		{
			auto is_selected = (post_processing_effects[effect_index] == post_processing_effects[n]);
			if (ImGui::Selectable(post_processing_effects[n].c_str(), is_selected))
			{
				effect_index = n;
			}
			if (is_selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

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

