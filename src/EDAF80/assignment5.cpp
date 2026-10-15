#include "assignment5.hpp"
#include "parametric_shapes.hpp"   

#include "config.hpp"              
#include "core/Bonobo.h"
#include "core/FPSCamera.h"
#include "core/helpers.hpp"        
#include "core/node.hpp"           // Node: 도형 + 셰이더 + 변환을 묶은 렌더 단위
#include "core/ShaderProgramManager.hpp"

#include <glm/gtc/type_ptr.hpp>    // glm::value_ptr (uniform 업로드용)
#include <imgui.h>
#include <tinyfiledialogs.h>

#include <clocale>
#include <stdexcept>
#include <array>
#include <cstdlib>                 // rand, srand

edaf80::Assignment5::Assignment5(WindowManager& windowManager) :
	mCamera(0.5f * glm::half_pi<float>(),   
		static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y), 
		0.01f, 1000.0f),                    
	inputHandler(), mWindowManager(windowManager), window(nullptr)
{
	WindowManager::WindowDatum window_datum{ inputHandler, mCamera, config::resolution_x, config::resolution_y, 0, 0, 0, 0 };

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

void
edaf80::Assignment5::run()
{
	// Shaders
	ShaderProgramManager program_manager;

	// fallback: 단색 흰색 셰이더 (레인에 사용)
	GLuint fallback_shader = 0u;
	program_manager.CreateAndRegisterProgram("Fallback",
		{ { ShaderType::vertex, "common/fallback.vert" },
		  { ShaderType::fragment, "common/fallback.frag" } },
		fallback_shader);
	if (fallback_shader == 0u) {
		LogError("Failed to load fallback shader");
		return;
	}

	// coin: 단색 + 카메라 기준 조명 (별, 플레이어에 사용. 색은 uniform으로 받음)
	GLuint coin_shader = 0u;
	program_manager.CreateAndRegisterProgram("Coin",
		{ { ShaderType::vertex, "EDAF80/coin.vert" },
		  { ShaderType::fragment, "EDAF80/coin.frag" } },
		coin_shader);
	if (coin_shader == 0u)
		LogError("Failed to load coin shader");

	// skybox: 배경 큐브맵용 
	GLuint skybox_shader = 0u;
	program_manager.CreateAndRegisterProgram("Skybox",
		{ { ShaderType::vertex, "EDAF80/skybox.vert" },
		  { ShaderType::fragment, "EDAF80/skybox.frag" } },
		skybox_shader);
	if (skybox_shader == 0u)
		LogError("Failed to load skybox shader");

	GLuint water_shader = 0u;
	program_manager.CreateAndRegisterProgram("Water",
		{ { ShaderType::vertex, "EDAF80/water.vert" },
		  { ShaderType::fragment, "EDAF80/water.frag" } },
		water_shader);
	if (water_shader == 0u)
		LogError("Failed to load water shader");

	// 셰이더에 넘길 uniforms
	// camera_position은 참조(&)로 잡아서, 값이 바뀌면 항상 최신 위치가 업로드됨.
	glm::vec3 camera_position = mCamera.mWorld.GetTranslation();
	float elapsed_time_s = 0.0f;
	auto const make_uniforms = [&camera_position](glm::vec3 color) {
		return [&camera_position, color](GLuint program) {
			glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
			glUniform3fv(glGetUniformLocation(program, "coin_color"), 1, glm::value_ptr(color));
			};
		};

	auto const set_uniforms = [&elapsed_time_s, &camera_position](GLuint program) {
		glUniform1f(glGetUniformLocation(program, "elapsed_time_s"), elapsed_time_s);
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
		};

	std::srand(static_cast<unsigned>(std::time(nullptr)));   // 랜덤 시드: 실행마다 다른 별 배치

	constexpr int N = 30;                        // 별 개수 (고정 배열, 재사용)
	float const lane_x[3] = { -3.0f, 0.0f, 3.0f };   // 3개 레인의 x 좌표
	float const row_spacing = 10.0f;              // 별 사이 간격(z 방향)
	float const player_y = 0.0f;                 // 플레이어/별 높이
	float const player_radius = 0.5f;            // 충돌용 반지름 (플레이어)
	float const star_radius = 0.5f;              // 충돌용 반지름 (별)        
	float const pickup_radius = player_radius + star_radius;   // 이 거리 안이면 별을 먹음

	float const lane_width = 2.4f;               // 레인 폭
	float const lane_length = 600.0f;            // 레인 길이

	// geometry
	// skybox cubemap 이미지와 바다를 GPU 텍스처로 로드
	GLuint const cubemap_texture = bonobo::loadTextureCubeMap(
		config::resources_path("cubemaps/NissiBeach2/posx.jpg"),
		config::resources_path("cubemaps/NissiBeach2/negx.jpg"),
		config::resources_path("cubemaps/NissiBeach2/posy.jpg"),
		config::resources_path("cubemaps/NissiBeach2/negy.jpg"),
		config::resources_path("cubemaps/NissiBeach2/posz.jpg"),
		config::resources_path("cubemaps/NissiBeach2/negz.jpg"),
		true);
	GLuint const waves_texture = bonobo::loadTexture2D(config::resources_path("textures/waves.png"));

	// 큐브맵 구
	auto const skybox_shape = parametric_shapes::createSphere(300.0f, 100u, 100u);
	if (skybox_shape.vao == 0u) {
		LogError("Failed to retrieve the mesh for the skybox");
		return;
	}

	// Node = 도형 + 셰이더 + 텍스처 + 변환(위치/회전)을 묶은 객체
	Node skybox;
	skybox.set_geometry(skybox_shape);
	skybox.add_texture("SkyboxTexture", cubemap_texture, GL_TEXTURE_CUBE_MAP);
	skybox.set_program(&skybox_shader, make_uniforms(glm::vec3(0.0f)));   // 색 인자는 skybox에선 무의미

	// 큰 평면 바다
	auto const sea_shape = parametric_shapes::createQuad(3000u, 3000u, 500u, 500u);
	if (sea_shape.vao == 0u) {
		LogError("Failed to retrieve the mesh for the sea");
		return;
	}
	Node sea;
	sea.set_geometry(sea_shape);
	sea.set_program(&water_shader, set_uniforms);
	sea.add_texture("WavesTexture", waves_texture, GL_TEXTURE_2D);
	// createQuad의 원점이 모서리라 절반만큼 보정해서 중앙 정렬, y=-3: 레인(-1)보다 아래
	sea.get_transform().SetTranslate(glm::vec3(-0.5f * 3000u, -200.0f, -0.5f * 3000u));


	// 별 모델(.glb) 로드
	auto const star_meshes = bonobo::loadObjects(config::resources_path("scenes/star.glb"));
	if (star_meshes.empty()) {
		LogError("Failed to load star.glb");
		return;
	}
	auto const& star_shape = star_meshes[0];
	auto const player_shape = parametric_shapes::createSphere(player_radius, 30u, 30u);   // 구형 플레이어
	auto const lane_shape = parametric_shapes::createQuad(lane_width, lane_length, 1u, 1u); // 평면형 레인

	if (star_shape.vao == 0u || player_shape.vao == 0u || lane_shape.vao == 0u || sea_shape.vao == 0u) {
		LogError("Failed to retrieve one of the meshes");
		return;
	}

	// 플레이어: 파란색 구
	Node player;
	player.set_geometry(player_shape);
	player.set_program(&coin_shader, make_uniforms(glm::vec3(0.2f, 0.8f, 1.0f)));

	// 별: 고정 크기 배열(N=30)을 만들어 재사용(먹히거나 지나가면 다시 앞쪽에 배치)
	std::array<Node, N> stars;
	std::array<glm::vec3, N> star_pos;   // 각 별의 월드 위치
	std::array<bool, N> active;          // 아직 안 먹혔는지 (false면 안 그림)

	for (int i = 0; i < N; ++i) {
		stars[i].set_geometry(star_shape);
		stars[i].set_program(&coin_shader, make_uniforms(glm::vec3(1.0f, 0.85f, 0.2f)));   // 노란색
		stars[i].get_transform().RotateX(-glm::half_pi<float>());   // 모델이 누워 있어서 세움
		stars[i].get_transform().RotateY(0.5f * i);                 // 별마다 시작 각도를 다르게
	}

	// 레인 3개
	std::array<Node, 3> lanes;
	for (int l = 0; l < 3; ++l) {
		lanes[l].set_geometry(lane_shape);
		lanes[l].set_program(&fallback_shader, [](GLuint) {});   // 따로 넘길 uniform 없음
	}

	// GameState 
	int score = 0;
	int target_lane = 1;               // 가려는 레인 번호 (0,1,2). 처음엔 가운데
	float player_x = lane_x[1];        
	float player_z = 0.0f;             // 플레이어의 z (-방향으로 전진)

	// 별 생성 규칙: 한 레인에 3~5개씩 줄지어 나오게 함
	float furthest_z = player_z - 15.0f;   // 가장 멀리 배치된 별의 z
	int current_lane = 1;                  // 지금 별을 놓고 있는 레인
	int run_left = 0;                      // 이 레인에 더 놓을 별 개수

	// 별 i를 가장 앞쪽(더 먼 -z)에 새로 배치하는 함수
	auto spawn_next = [&](int i) {
		if (run_left <= 0) {                        // 한 줄을 다 놨으면 새 레인/길이 결정
			current_lane = std::rand() % 3;         // 레인 랜덤
			run_left = 3 + std::rand() % 3;         // 3~5개
		}
		--run_left;
		furthest_z -= row_spacing;                  // 이전 별보다 한 칸 더 앞(-z)
		star_pos[i] = glm::vec3(lane_x[current_lane], player_y, furthest_z); // 별 i를 그 자리에 배치
		active[i] = true;                           // 다시 먹을 수 있게 active
		};
	for (int i = 0; i < N; ++i)
		spawn_next(i);                              // 첫 시작: 별 30개를 앞쪽에 쭉 배치


	glClearDepthf(1.0f);
	glClearColor(1.0f, 0.5f, 0.0f, 1.0f);
	// glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glEnable(GL_DEPTH_TEST);                       

	auto lastTime = std::chrono::high_resolution_clock::now();
	auto cull_mode = bonobo::cull_mode_t::disabled;
	bool show_logs = true;
	bool show_gui = true;
	bool shader_reload_failed = false;
	bool show_basis = false;
	float basis_thickness_scale = 1.0f;
	float basis_length_scale = 1.0f;

	float speed = 12.0f;               // 현재 전진 속도
	float const accel = 0.4f;          // 초당 가속량
	float const max_speed = 25.0f;     // 속도 상한

	changeCullMode(cull_mode);

	while (!glfwWindowShouldClose(window)) {
		auto const nowTime = std::chrono::high_resolution_clock::now();
		auto const deltaTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(nowTime - lastTime);
		lastTime = nowTime;
		float const dt = std::chrono::duration<float>(deltaTimeUs).count();
		elapsed_time_s += dt;

		auto& io = ImGui::GetIO();
		inputHandler.SetUICapture(io.WantCaptureMouse, io.WantCaptureKeyboard);

		glfwPollEvents();
		inputHandler.Advance();

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


		// Retrieve the actual framebuffer size: for HiDPI monitors,
		// you might end up with a framebuffer larger than what you
		// actually asked for. For example, if you ask for a 1920x1080
		// framebuffer, you might get a 3840x2160 one instead.
		// Also it might change as the user drags the window between
		// monitors with different DPIs, or if the fullscreen status is
		// being toggled.
		int framebuffer_width, framebuffer_height;
		glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
		glViewport(0, 0, framebuffer_width, framebuffer_height);

		// Handle Inputs

		// 레인 변경
		if (inputHandler.GetKeycodeState(GLFW_KEY_A) & JUST_PRESSED)
			target_lane = glm::max(0, target_lane - 1);   
		if (inputHandler.GetKeycodeState(GLFW_KEY_D) & JUST_PRESSED)
			target_lane = glm::min(2, target_lane + 1);   

		speed = glm::min(max_speed, speed + accel * dt);   // 시간에 따라 가속, 상한에서 멈춤
		player_z -= speed * dt;                             // -z 방향이 '앞'
		
		// 플레이어 x축 슥 미끄러지는 이동
		player_x += (lane_x[target_lane] - player_x) * glm::min(1.0f, 10.0f * dt);
		glm::vec3 const player_pos(player_x, player_y, player_z);
		player.get_transform().SetTranslate(player_pos);

		for (int i = 0; i < N; ++i) {
			// 충돌 검사: 거리가 두 반지름 합보다 작으면 먹음
			if (active[i] && glm::length(star_pos[i] - player_pos) < pickup_radius) {
				active[i] = false;   // 숨김
				++score;
			}
			// 플레이어 뒤로 12m 이상 지나간 별은 앞쪽으로 재배치
			if (star_pos[i].z > player_z + 12.0f)
				spawn_next(i);

			stars[i].get_transform().SetTranslate(star_pos[i]);   // 위치 겡신
			stars[i].get_transform().RotateY(3.0f * dt);          // 초당 2 rad씩 계속 자전
		}

		// createQuad의 원점이 모서리라서 폭의 절반만큼 왼쪽으로 보정(중앙 정렬)
		// z는 플레이어 앞 280 ~ 뒤 20을 덮도록 계속 따라다니게 배치
		for (int l = 0; l < 3; ++l)
			lanes[l].get_transform().SetTranslate(glm::vec3(lane_x[l] - 0.5f * lane_width, -1.0f, player_z - lane_length + 20.0f));

		// 3인칭 카메라: 플레이어 뒤(+z) 8m, 위 3.5m에서 앞쪽(-z)을 바라봄
		// x는 0.6배만 따라가서 레인 이동 시 카메라가 덜 흔들림
		camera_position = glm::vec3(player_x * 0.6f, 3.5f, player_z + 8.0f);
		mCamera.mWorld.SetTranslate(camera_position);
		mCamera.mWorld.LookAt(glm::vec3(player_x * 0.6f, 0.0f, player_z - 6.0f));
		//skybox.get_transform().SetTranslate(camera_position);   // 하늘이 항상 카메라를 따라다님
		
		mWindowManager.NewImGuiFrame();

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);   

		if (!shader_reload_failed) {
			// Todo: Render all your geometry here.
			auto const vp = mCamera.GetWorldToClipMatrix();   // view × projection (월드 → 클립 좌표 행렬)
			sea.render(vp);
			skybox.render(vp);               
			for (auto& l : lanes)
				l.render(vp);                
			player.render(vp);               
			for (int i = 0; i < N; ++i) {
				if (active[i])               // 안 먹힌 별만 그림
					stars[i].render(vp);
			}
		}

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// Todo: If you want a custom ImGUI window, you can set it up
		//       here
		// 점수/속도 표시 UI
		bool const opened = ImGui::Begin("Game", nullptr, ImGuiWindowFlags_None);
		if (opened) {
			ImGui::Text("Score: %d", score);
			ImGui::Text("Speed: %.1f", speed);
			ImGui::Text("A / D : change lane");
			ImGui::Checkbox("Show basis", &show_basis);
			ImGui::SliderFloat("Basis thickness scale", &basis_thickness_scale, 0.0f, 100.0f);
			ImGui::SliderFloat("Basis length scale", &basis_length_scale, 0.0f, 100.0f);
		}
		ImGui::End();

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
	}
	catch (std::runtime_error const& e) {
		LogError(e.what());
	}
}
