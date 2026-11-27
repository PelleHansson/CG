// Do not use intrinsic functions, which allows using `constexpr` on GLM
// functions.
#define GLM_FORCE_PURE 1

#include "fluidSim.hpp"

#include "config.hpp"
#include "core/Bonobo.h"
#include "core/FPSCamera.h"
#include "core/helpers.hpp"
#include "core/node.hpp"
#include "core/opengl.hpp"
#include "core/ShaderProgramManager.hpp"

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <tinyfiledialogs.h>

#include <array>
#include <clocale>
#include <cstdlib>
#include <stdexcept>
#include <random>


project::FluidSim::FluidSim(WindowManager& windowManager) :
mCamera(0.5f * glm::half_pi<float>(),
	static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
	0.01f, 1000.0f),
	inputHandler(), mWindowManager(windowManager), window(nullptr)
{
	WindowManager::WindowDatum window_datum{ inputHandler, mCamera, config::resolution_x, config::resolution_y, 0, 0, 0, 0 };

	window = mWindowManager.CreateGLFWWindow("EDAF80: Assignment 2", window_datum, config::msaa_rate);
	if (window == nullptr) {
		throw std::runtime_error("Failed to get a window: aborting!");
	}

	bonobo::init();
}

project::FluidSim::~FluidSim()
{

	bonobo::deinit();
}



void project::FluidSim::run() {
	// Set up the camera
	mCamera.mWorld.SetTranslate(glm::vec3(0.0f, 0.0f, 25.0f));
	mCamera.mWorld.LookAt(glm::vec3(0.0f));
	mCamera.mMouseSensitivity = glm::vec2(0.003f);
	mCamera.mMovementSpeed = glm::vec3(3.0f); // 3 m/s => 10.8 km/h
	auto camera_position = mCamera.mWorld.GetTranslation();

	// Create the shader programs
	ShaderProgramManager program_manager;
	GLuint fallback_shader = 0u;
	program_manager.CreateAndRegisterProgram("Fallback",
		{ { ShaderType::vertex, "common/fallback.vert" },
		  { ShaderType::fragment, "common/fallback.frag" } },
		fallback_shader);
	if (fallback_shader == 0u) {
		LogError("Failed to load fallback shader");
		return;
	}
	GLuint raycast_shader = 0u;

	program_manager.CreateAndRegisterProgram(
		"raycast",
		{
			{ ShaderType::vertex, "project/fullscreen.vert" },
			{ ShaderType::fragment, "project/raycast.frag" }
		},
		raycast_shader
	);

	if (raycast_shader == 0u) {
		LogError("Failed to load fullscreen shaders");
		return;
	}
	//
	// Todo: Insert the creation of other shader programs.
	//       (Check how it was done in assignment 3.)
	//
	glm::vec3 planePosition =
		camera_position + glm::vec3(0.0f, 0.0f, -10.0f);

	glm::vec3 planeNormal =
		glm::normalize(camera_position - planePosition);

	glm::vec3 worldUp(0.0f, 1.0f, 0.0f);

	glm::vec3 planeRight =
		glm::normalize(glm::cross(worldUp, planeNormal));

	glm::vec3 planeUp =
		glm::normalize(glm::cross(planeNormal, planeRight));

	float sheetWidth = 10.0f;
	float sheetHeight = 10.0f;

	auto load2dtex = [](const std::string& texname) {
		return bonobo::loadTexture2D(
			config::resources_path("textures/" + texname),
			true
		);
		};

	const int textureWidth = 20;
	const int textureHeight = 20;
	float elapsed_time_s = 0.0f;
	std::vector<unsigned char> pixels(textureWidth * textureHeight * 4);

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> colorDist(0, 255);

	for (int i = 0; i < textureWidth * textureHeight; ++i) {
		pixels[i * 4 + 0] = colorDist(gen); 
		pixels[i * 4 + 1] = colorDist(gen); 
		pixels[i * 4 + 2] = colorDist(gen); 
		pixels[i * 4 + 3] = 255;            
	}
	GLuint texture;
	glGenTextures(1, &texture);

	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA8,
		textureWidth,
		textureHeight,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		pixels.data()
	);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);


	// Todo: Load your geometry
	//

	glClearDepthf(1.0f);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glEnable(GL_DEPTH_TEST);


	auto lastTime = std::chrono::high_resolution_clock::now();

	bool pause_animation = true;
	bool use_orbit_camera = false;
	auto cull_mode = bonobo::cull_mode_t::disabled;
	auto polygon_mode = bonobo::polygon_mode_t::fill;
	bool show_logs = true;
	bool show_gui = true;
	bool shader_reload_failed = false;
	bool show_basis = false;
	float basis_thickness_scale = 1.0f;
	float basis_length_scale = 1.0f;

	changeCullMode(cull_mode);

	while (!glfwWindowShouldClose(window)) {
		auto const nowTime = std::chrono::high_resolution_clock::now();
		auto const deltaTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(nowTime - lastTime);
		lastTime = nowTime;
		if (!pause_animation) {
		//	elapsed_time_s += std::chrono::duration<float>(deltaTimeUs).count();
		}

		auto& io = ImGui::GetIO();
		inputHandler.SetUICapture(io.WantCaptureMouse, io.WantCaptureKeyboard);

		glfwPollEvents();
		inputHandler.Advance();
		mCamera.Update(deltaTimeUs, inputHandler);
		if (use_orbit_camera) {
			mCamera.mWorld.LookAt(glm::vec3(0.0f));
		}
		camera_position = mCamera.mWorld.GetTranslation();

		if (inputHandler.GetKeycodeState(GLFW_KEY_R) & JUST_PRESSED) {
			shader_reload_failed = !program_manager.ReloadAllPrograms();
			if (shader_reload_failed)
				tinyfd_notifyPopup("Shader Program Reload Error",
					"An error occurred while reloading shader programs; see the logs for details.\n"
					"Rendering is suspended until the issue is solved. Once fixed, just reload the shaders again.",
					"error");
		}
		if (inputHandler.GetKeycodeState(GLFW_KEY_F3) & JUST_RELEASED)
			show_logs = !show_logs;
		if (inputHandler.GetKeycodeState(GLFW_KEY_F2) & JUST_RELEASED)
			show_gui = !show_gui;
		if (inputHandler.GetKeycodeState(GLFW_KEY_F11) & JUST_RELEASED)
			mWindowManager.ToggleFullscreenStatusForWindow(window);


	
		int framebuffer_width, framebuffer_height;
		glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
		glViewport(0, 0, framebuffer_width, framebuffer_height);

	


		mWindowManager.NewImGuiFrame();

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		bonobo::changePolygonMode(polygon_mode);


		if (!shader_reload_failed)
		{
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			glUseProgram(raycast_shader);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			glUniform1i(glGetUniformLocation(raycast_shader, "inputArray"),0);

			glm::mat4 viewProjection = mCamera.GetWorldToClipMatrix();
			glm::mat4 inverseViewProjection = glm::inverse(viewProjection);
			glm::vec3 cameraPosition =	mCamera.mWorld.GetTranslation();

			GLint invVP_location = glGetUniformLocation(raycast_shader,"inverseViewProjection");

			glUniformMatrix4fv(invVP_location,1,GL_FALSE,glm::value_ptr(inverseViewProjection));


			GLint camera_location = glGetUniformLocation(raycast_shader,"cameraPosition");
			glUniform3fv(camera_location,1,glm::value_ptr(cameraPosition));
	

			bonobo::drawFullscreen();

			glUseProgram(0u);

			glEnable(GL_DEPTH_TEST);

			glEnable(GL_CULL_FACE);

		}


		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);



		bool opened = ImGui::Begin("Scene Control", nullptr, ImGuiWindowFlags_None);
		if (opened) {
			ImGui::Checkbox("Pause animation", &pause_animation);
			ImGui::Checkbox("Use orbit camera", &use_orbit_camera);
			ImGui::Separator();
			auto const cull_mode_changed = bonobo::uiSelectCullMode("Cull mode", cull_mode);
			if (cull_mode_changed) {
				changeCullMode(cull_mode);
			}
			bonobo::uiSelectPolygonMode("Polygon mode", polygon_mode);
			ImGui::Separator();
			ImGui::Checkbox("Show basis", &show_basis);
			ImGui::SliderFloat("Basis thickness scale", &basis_thickness_scale, 0.0f, 100.0f);
			ImGui::SliderFloat("Basis length scale", &basis_length_scale, 0.0f, 100.0f);
		}
		ImGui::End();

		if (show_basis)
			bonobo::renderBasis(basis_thickness_scale, basis_length_scale, mCamera.GetWorldToClipMatrix());
		if (show_logs)
			Log::View::Render();
		mWindowManager.RenderImGuiFrame(show_gui);

		glfwSwapBuffers(window);
	}
	glDeleteTextures(1, &texture);
}
int main()
{
	std::setlocale(LC_ALL, "");

	Bonobo framework;

	try {
		project::FluidSim fluidSim(framework.GetWindowManager());
		fluidSim.run();
	} catch (std::runtime_error const& e) {
		LogError(e.what());
	}
}






