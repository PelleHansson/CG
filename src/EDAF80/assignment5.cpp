#include "assignment5.hpp"
#include "parametric_shapes.hpp"

#include "config.hpp"
#include "core/Bonobo.h"
#include "core/FPSCamera.h"
#include "core/helpers.hpp"
#include "core/node.hpp"
#include "core/ShaderProgramManager.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <tinyfiledialogs.h>

#include <clocale>
#include <stdexcept>
#include <random>

edaf80::Assignment5::Assignment5(WindowManager& windowManager) :
	mCamera(0.5f * glm::half_pi<float>(),
	        static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
	        0.01f, 600.0f),
	inputHandler(), mWindowManager(windowManager), window(nullptr)
{
	WindowManager::WindowDatum window_datum{ inputHandler, mCamera, config::resolution_x, config::resolution_y, 0, 0, 0, 0};

	window = mWindowManager.CreateGLFWWindow("EDAF80: Assignment 5", window_datum, config::msaa_rate);
	if (window == nullptr) {
		throw std::runtime_error("Failed to get a window: aborting!");
	}

	bonobo::init();
}

edaf80::Assignment5::~Assignment5()
{
	bonobo::deinit();
}


struct ObjRef {
	Node* node;
	float speed = 0;
	float angle = 0;
	float hightOffset = 0;
	float stability = 1; //how much is it affected but the waves
	float BSradius;
	//float verticalSpeed;
	ObjRef(Node* n, float r) : node(n), BSradius(r){}
};
static bool intersects(ObjRef a, ObjRef b) {
	glm::vec3 dif = a.node->get_transform().GetTranslation()- b.node->get_transform().GetTranslation();
	return glm::length(dif) < (a.BSradius + b.BSradius);
}

static float wave(glm::vec2 position, glm::vec2 direction, float amplitude, float frequency, float phase, float sharpness, float time, float& dx, float& dz)
{
	float theta =
		dot(position, direction) * frequency + phase * time;

	dx = 0.5f * sharpness * frequency * amplitude * pow(sin(theta) * 0.5 + 0.5, sharpness - 1) * cos(theta) * direction.x;
	dz = 0.5f * sharpness * frequency * amplitude * pow(sin(theta) * 0.5 + 0.5, sharpness - 1) * cos(theta) * direction.y;

	return amplitude * pow(sin(theta) * 0.5 + 0.5, sharpness);
}
static glm::quat waveRotation(ObjRef object, float dx, float dz) {
	glm::vec3 waterNormal = glm::normalize(glm::vec3(-dx * object.stability, 1.0f, -dz * object.stability));
	glm::vec3 objectdir = glm::normalize(-object.node->get_transform().GetMatrix()[2]);

	glm::vec3 dynamicRight = glm::normalize(glm::cross(objectdir, waterNormal));

	glm::vec3 dynamicForward = glm::normalize(glm::cross(waterNormal, dynamicRight));

	glm::mat3 rotationMatrix(dynamicRight, waterNormal, -dynamicForward);

	return glm::quat_cast(rotationMatrix);
	
}

static void waterfloat(ObjRef object, float time,float deltaTime_s) {
	glm::vec3 pos = object.node->get_transform().GetTranslation();

	float height = object.hightOffset;
	float dx, dz, dx2, dz2, dx3, dz3;

	glm::vec2 offsetPos = glm::vec2(pos[0]/* - waterwidth / 2*/, pos[2]/* - waterwidth / 2*/);//since we offset water the wave is also offset
	height += wave(offsetPos, glm::vec2(-1.0, 0.0), 1.5, 0.2, 1.6, 2.0, time, dx, dz);
	height += wave(offsetPos, glm::vec2(-0.7, 0.7), 0.8, 0.4, 1.3, 2.0, time, dx2, dz2);
	height += wave(offsetPos, glm::vec2(0.3, -0.95), 0.5, 0.15, 2.7, 2.0, time, dx3, dz3);


	pos.y = height;

	float dx_tot = dx + dx2 + dx3;
	object.node->get_transform().SetTranslate(pos);
	glm::quat waveRot = waveRotation(object, dx_tot, dz +dz2 +dz3 );



	float angle = glm::angle(waveRot);
	glm::vec3 axis = glm::axis(waveRot);
	object.node->get_transform().SetRotate(angle, glm::normalize(axis));
}


static void boatMove(ObjRef b,float time , float deltaTime_s) {

	b.node->get_transform().SetRotateY(b.angle);
	
	waterfloat(b, time,deltaTime_s);

	glm::vec3 pos = b.node->get_transform().GetTranslation();
	glm::vec3 dir = glm::normalize(-b.node->get_transform().GetMatrix()[2]);

	glm::quat currentRot = b.node->get_transform().GetRotation();
	//b.speed = glm::max(b.speed - 2 * deltaTimeSeconds, 0.0f);
	//float frictionFactor = std::pow(0.85f, deltaTimeSeconds);
	if (b.speed < 0.01f && b.speed > -0.01f) {
		b.speed = 0.0f;
	}
	//b.speed *= frictionFactor;

	//std::cout << b.speed << "\n";
	pos += b.speed * dir * deltaTime_s ;
	b.node->get_transform().SetTranslate(pos);

	
}


void
edaf80::Assignment5::run()
{

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> randDist(0.0f, 1.0f);
	// Set up the camera
	mCamera.mWorld.SetTranslate(glm::vec3(510.0f, 20.0f, 510.0f));
	mCamera.mMouseSensitivity = glm::vec2(0.003f);
	mCamera.mMovementSpeed = glm::vec3(3.0f); // 3 m/s => 10.8 km/h
	glm::vec2 mousePos = inputHandler.GetMousePosition();
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
	GLuint water_shader = 0u;
	program_manager.CreateAndRegisterProgram("Water",
		{ { ShaderType::vertex, "EDAF80/game_water.vert" },
		  { ShaderType::fragment, "EDAF80/game_water.frag" } },
		water_shader);

	if (water_shader == 0u) {
		LogError("Failed to load water shader");
		return;
	}
	GLuint phongboat_shader = 0u;
	program_manager.CreateAndRegisterProgram("Phongboat",
		{ { ShaderType::vertex, "EDAF80/phongboat.vert" },
		 { ShaderType::fragment, "EDAF80/phongboat.frag" } },
		phongboat_shader);
	if (phongboat_shader == 0u)
		LogError("Failed to load diffuse shader");


	GLuint phong_shader = 0u;

	program_manager.CreateAndRegisterProgram("Phong",
		{ { ShaderType::vertex, "EDAF80/phongbarrel.vert" },
		 { ShaderType::fragment, "EDAF80/phongbarrel.frag" } },
		phong_shader);
	if (phong_shader == 0u)
		LogError("Failed to load diffuse shader");


	GLuint skybox_shader = 0u;
	program_manager.CreateAndRegisterProgram("Skybox",
		{ { ShaderType::vertex, "EDAF80/skybox.vert" },
		{ ShaderType::fragment, "EDAF80/skybox.frag" } },
		skybox_shader);

	if (skybox_shader == 0u)
		LogError("Failed to load skybox shader");




	float elapsed_time_s = 0.0f;
	float water_size = 1000;
	auto quad = parametric_shapes::createQuad(water_size, water_size, 1000u, 1000u);

	if (quad.vao == 0u) {
		LogError("Failed to retrieve the mesh for the quad");
		return;
	}
	glm::vec3 light_position = glm::vec3(1000.0f, 50.0f, 500.0f);



	auto loadCubemap = [](const std::string& cubename) {
		//std::cout << "Loaded cubemap: " << cubename << std::endl;
		return bonobo::loadTextureCubeMap(
			config::resources_path("game/" + cubename + "/posx.png"),
			config::resources_path("game/" + cubename + "/negx.png"),
			config::resources_path("game/" + cubename + "/posy.png"),
			config::resources_path("game/" + cubename + "/negy.png"),
			config::resources_path("game/" + cubename + "/posz.png"),
			config::resources_path("game/" + cubename + "/negz.png"),
			true
		);
		};
	GLuint cubemap = loadCubemap("GameSky");
	auto const set_uniforms = [&light_position](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		};

	auto const water_set_uniforms = [&light_position, &elapsed_time_s, &camera_position, &cubemap](GLuint program) {
		glUniform1f(glGetUniformLocation(program, "time"), elapsed_time_s);
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));

		};
	bool use_normal_mapping = false;
	auto const phong_set_uniforms = [&use_normal_mapping, &light_position, &camera_position](GLuint program) {

		glUniform1i(glGetUniformLocation(program, "use_normal_mapping"), use_normal_mapping ? 1 : 0);
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));


		};

	Node water;
	auto load2dtex = [](const std::string& texname) {
		return bonobo::loadTexture2D(
			config::resources_path("textures/" + texname),
			true
		);
		};
	auto normal_tex = load2dtex("waves.png");
	water.set_geometry(quad);
	water.add_texture("reflection_texture", cubemap, GL_TEXTURE_CUBE_MAP);
	water.add_texture("normal_texture", normal_tex, GL_TEXTURE_2D);
	water.set_program(&water_shader, water_set_uniforms);
	//water.get_transform().SetTranslate(glm::vec3(-water_size/2, 0,-water_size/2));


	auto skybox_shape = parametric_shapes::createSphere(20.0f, 100u, 100u);
	if (skybox_shape.vao == 0u) {
		LogError("Failed to retrieve the mesh for the skybox");
		return;
	}

	Node skybox;
	skybox.set_geometry(skybox_shape);
	skybox.add_texture("cubemap", cubemap, GL_TEXTURE_CUBE_MAP);
	skybox.set_program(&skybox_shader, set_uniforms);






	bonobo::material_data boat_material;
	boat_material.ambient = glm::vec3(0.3f, 0.3f, 0.3f);
	boat_material.diffuse = glm::vec3(0.996, 1.0, 0.702);
	boat_material.specular = glm::vec3(1.0f, 1.0f, 1.0f);
	boat_material.shininess = 10.0f;


	std::vector<bonobo::mesh_data> const boatmeshes = bonobo::loadObjects(config::resources_path("game/boat/boat.obj"));


	LogInfo("Boat meshes loaded: {}", boatmeshes.size());
	if (boatmeshes.empty()) {
		LogError("Failed to load the boat geometry: exiting.");
		bonobo::deinit();
		return;
	}

	Node b;
	ObjRef boat = ObjRef(&b, 2);

	boat.node->get_transform().SetTranslate(glm::vec3(500.0f, 0.0f, 500.0f));
	std::vector<Node> boatChildren;
	boatChildren.resize(boatmeshes.size());
	GLuint textureFallbackID = normal_tex;
	for (size_t i = 0; i < boatmeshes.size(); ++i) {
		Node& childNode = boatChildren[i];
		childNode.set_geometry(boatmeshes[i]);
		bool hasDiffuse = false;
		bool hasNormal = false;
		childNode.set_material_constants(boat_material);
		for (const auto& binding : boatmeshes[i].bindings) {

			if (binding.first == "diffuse_texture") hasDiffuse = true;
			if (binding.first == "normals_texture") {
				hasNormal = true;
				childNode.add_texture("normal_texture", binding.second, GL_TEXTURE_2D);
				continue;
			}
			childNode.add_texture(binding.first, binding.second, GL_TEXTURE_2D);
		}
		childNode.set_program(&phongboat_shader, phong_set_uniforms);

	}
	for (size_t i = 0; i < boatChildren.size(); ++i) {
		boat.node->add_child(&boatChildren[i]);
	}
	boat.node->get_transform().SetScale(1.0f);
	boat.stability = 0.8f;
	boat.hightOffset = 0.2f;



	std::vector<bonobo::mesh_data> const mesh = bonobo::loadObjects(config::resources_path("game/barrel/Barrel.obj"));
	if (mesh.empty()) {
		LogError("Failed to load the sphere geometry: exiting.");

		bonobo::deinit();

		return ;
	}
	bonobo::material_data demo_material;
	demo_material.ambient = glm::vec3(0.3f, 0.3f, 0.3f);
	demo_material.diffuse = glm::vec3(0.7f, 0.2f, 0.4f);
	demo_material.specular = glm::vec3(1.0f, 1.0f, 1.0f);
	demo_material.shininess = 0.2f;
	bonobo::mesh_data const& barrelMesh = mesh.front();

	Node bar;
	ObjRef barrel(&bar,1);
	bool hasDiffuse = false;
	bool hasNormal = false;
	barrel.node->set_geometry(barrelMesh);
	
	barrel.node->add_texture("diffuse_texture", bonobo::loadTexture2D(config::resources_path("game/barrel/barrel_dif.png"),true) , GL_TEXTURE_2D);
	barrel.node->add_texture("specular_texture", bonobo::loadTexture2D(config::resources_path("game/barrel/Image_1.png"), true), GL_TEXTURE_2D);
	barrel.node->add_texture("normal_texture", bonobo::loadTexture2D(config::resources_path("game/barrel/barrel_nor.png"), true), GL_TEXTURE_2D);

	barrel.node->set_material_constants(demo_material);
	barrel.node->set_program(&phong_shader, phong_set_uniforms);
	barrel.node->get_transform().SetTranslate(glm::vec3(520, 5, 520));
	barrel.node->get_transform().SetScale(0.02f);
	barrel.stability = 1.3f;
	barrel.hightOffset = -0.6f;


	glClearDepthf(1.0f);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	auto lastTime = std::chrono::high_resolution_clock::now();

	bool show_logs = true;
	bool show_gui = true;
	bool shader_reload_failed = false;
	bool show_basis = false;
	float basis_thickness_scale = 1.0f;
	float basis_length_scale = 1.0f;
	bool freeroam = false;

	float orbitYaw = 0.0f;
	float orbitPitch = 0.4f;
	float orbitRadius = 30.0f;

	
	while (!glfwWindowShouldClose(window)) {
		auto const nowTime = std::chrono::high_resolution_clock::now();
		auto const deltaTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(nowTime - lastTime);
		lastTime = nowTime;

		elapsed_time_s += std::chrono::duration<float>(deltaTimeUs).count();


		auto& io = ImGui::GetIO();
		inputHandler.SetUICapture(io.WantCaptureMouse, io.WantCaptureKeyboard);

		glfwPollEvents();
		inputHandler.Advance();
		glm::vec3 boatPos = boat.node->get_transform().GetTranslation();
		mCamera.mWorld.LookAt(boatPos);
		mCamera.Update(deltaTimeUs, inputHandler, true, true);
		
		camera_position = mCamera.mWorld.GetTranslation();
		float deltaTime_s = static_cast<float>(deltaTimeUs.count()) / 1000000.0f;



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


		//
		
		if ((inputHandler.GetKeycodeState(GLFW_KEY_W) & PRESSED))  boat.speed = glm::min(boat.speed + (10.0f * deltaTime_s), 30.0f);
		else boat.speed = glm::max(boat.speed - (8.0f * deltaTime_s), 0.0f);

		
	/*	if ((inputHandler.GetKeycodeState(GLFW_KEY_S) & PRESSED)) b.speed = glm::fmax(-500.0f, b.speed - 1);
		*/
		if ((inputHandler.GetKeycodeState(GLFW_KEY_A) & PRESSED)  ) boat.angle = boat.angle + (2.0f * deltaTime_s);
		if ((inputHandler.GetKeycodeState(GLFW_KEY_D) & PRESSED)  ) boat.angle = boat.angle - (2.0f * deltaTime_s)  ;

	
		glm::vec2 newMousePos = inputHandler.GetMousePosition();
		float defaltPitch =0.3 ;
		float defaltYaw = boat.angle;
		if (!inputHandler.IsMouseCapturedByUI() && (inputHandler.GetMouseState(GLFW_MOUSE_BUTTON_LEFT) & PRESSED)) {
			glm::vec2 mouseDelta = mousePos - newMousePos ;
			orbitYaw -= mouseDelta.x * mCamera.mMouseSensitivity.x ;
			orbitPitch -= mouseDelta.y * mCamera.mMouseSensitivity.y ;
			orbitPitch = glm::clamp(orbitPitch, 0.05f, glm::half_pi<float>() - 0.05f);
		}
		else {
			orbitYaw = glm::mix(orbitYaw, defaltYaw, glm::min(4.0f * deltaTime_s, 1.0f));
			orbitPitch = glm::mix(orbitPitch, defaltPitch, glm::min(4.0f * deltaTime_s, 1.0f));
		}
		if (!inputHandler.IsMouseCapturedByUI() &&(inputHandler.GetMouseState(GLFW_MOUSE_BUTTON_RIGHT) & PRESSED)) {
			// Handle scroll
			float mouseDeltaY = mousePos.y - newMousePos.y;
			orbitRadius = glm::max(orbitRadius + mouseDeltaY * 0.05, 3.0);
		}


		glm::vec3 cameraOffset;
		cameraOffset.x = orbitRadius * std::cos(orbitPitch) * std::sin(orbitYaw);
		cameraOffset.y = orbitRadius * std::sin(orbitPitch);
		cameraOffset.z = orbitRadius * std::cos(orbitPitch) * std::cos(orbitYaw);
		mCamera.mWorld.SetTranslate(cameraOffset + boatPos);

		mousePos = newMousePos;

		mWindowManager.NewImGuiFrame();

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


		if (!shader_reload_failed) {
			skybox.get_transform().SetTranslate(camera_position);
			glCullFace(GL_FRONT);
			skybox.render(mCamera.GetWorldToClipMatrix());
			glCullFace(GL_BACK);
			glClear(GL_DEPTH_BUFFER_BIT);
			water.render(mCamera.GetWorldToClipMatrix());

			waterfloat(barrel, elapsed_time_s, deltaTime_s);
			barrel.node->render(mCamera.GetWorldToClipMatrix());
		//	glm::vec3 barrelrot = glm::vec3(barrel.get_transform().GetMatrix()[2]) + glm::vec3(randDist(gen)*deltaTime_s, randDist(gen)*deltaTime_s, deltaTime_s);
			//barrel.get_transform().SetRotate(barrelrot.);
			boatMove(boat, elapsed_time_s, deltaTime_s);
			for (size_t i = 0; i < boatChildren.size(); ++i) {
				// This forces the vertex shader to compound: Camera * Parent * Mesh Offset
				boatChildren[i].render(mCamera.GetWorldToClipMatrix(), boat.node->get_transform().GetMatrix());
			}
		}

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		if (intersects(boat, barrel)) {
			glm::vec3 newPos = glm::vec3(randDist(gen)*200 + 400, 0, randDist(gen)*200 + 400);
			barrel.node->get_transform().SetTranslate(newPos);
		

			std::cout <<newPos << "\n";
		}


		//
		// Todo: If you want a custom ImGUI window, you can set it up
		//       here
		//
		bool const opened = ImGui::Begin("Scene Controls", nullptr, ImGuiWindowFlags_None);
		if (opened) {
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
}

int main()
{
	std::setlocale(LC_ALL, "");

	Bonobo framework;

	try {
		edaf80::Assignment5 assignment5(framework.GetWindowManager());
		assignment5.run();
	} catch (std::runtime_error const& e) {
		LogError(e.what());
	}
}
