#include "parametric_shapes.hpp"
#include "core/Log.h"

#include <glm/glm.hpp>

#include <array>
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

bonobo::mesh_data
parametric_shapes::createQuad(float const width, float const height,
	unsigned int const horizontal_split_count,
	unsigned int const vertical_split_count)
{
	// vertex, 점 개수만큼, split + 2 
	auto vertices = std::vector<glm::vec3>();
	for (unsigned int i = 0; i < vertical_split_count + 2; i++) {
		for (unsigned int j = 0; j < horizontal_split_count + 2; j++) {
			vertices.push_back(glm::vec3(j * width / (horizontal_split_count + 1), 0.0f, i * height / (vertical_split_count + 1)));
		}
		}

		// 삼각형 indexing, 칸 수만큼, split+ 1, 사각형 = 2개의 삼각형
		auto index_sets = std::vector<glm::uvec3>();
		unsigned int vertices_per_row = horizontal_split_count + 2; // 한 줄에 있는 vertex 개수
		for (unsigned int i = 0; i < vertical_split_count + 1; i++) {
			for (unsigned int j = 0; j < horizontal_split_count + 1; j++) {
				unsigned int top_left = i * vertices_per_row + j;
				unsigned int top_right = top_left + 1;
				unsigned int bottom_left = (i + 1) * vertices_per_row + j;
				unsigned int bottom_right = bottom_left + 1;
				index_sets.push_back(glm::uvec3(top_left, bottom_left, top_right));
				index_sets.push_back(glm::uvec3(top_right, bottom_left, bottom_right));
			}
		}

		// texture coordinates, vertex 개수만큼, split + 2
		auto texcoords = std::vector<glm::vec2>();
		for (unsigned int i = 0; i < vertical_split_count + 2; i++) {
			for (unsigned int j = 0; j < horizontal_split_count + 2; j++) {
				texcoords.push_back(glm::vec2(static_cast<float>(j) / (horizontal_split_count + 1), static_cast<float>(i) / (vertical_split_count + 1)));
			}
		}

		bonobo::mesh_data data;

		//if (horizontal_split_count > 0u || vertical_split_count > 0u)
		//{
		//	LogError("parametric_shapes::createQuad() does not support tesselation.");
		//	return data;
		//}

		// Create a Vertex Array Object: it will remember where we stored the
		// data on the GPU, and  which part corresponds to the vertices, which
		// one for the normals, etc.
		
		// VAO(Vertex Array Object) 만들고 bind: 어떤 buffer의 어느 부분이 vertex/texcoord인지 기억하는 상자
		glGenVertexArrays(1, &data.vao);
		glBindVertexArray(data.vao);

		auto const vertices_offset = 0u;
		auto const vertices_size = static_cast<GLsizeiptr>(vertices.size() * sizeof(glm::vec3));
		auto const texcoords_offset = vertices_offset + vertices_size;
		auto const texcoords_size = static_cast<GLsizeiptr>(texcoords.size() * sizeof(glm::vec2));
		auto const bo_size = static_cast<GLsizeiptr>(vertices_size + texcoords_size);

		// vertex용 buffer(`data.bo`) 만들고 전체 크기만큼 GPU에 메모리 공간 만들기(데이터는 아직 X)
		glGenBuffers(1, &data.bo);
		glBindBuffer(GL_ARRAY_BUFFER, data.bo);
		glBufferData(GL_ARRAY_BUFFER, bo_size, nullptr, GL_STATIC_DRAW);

		// 앞부분에 vertices 넣고, 읽는 법 알려주기 (float 3개씩, 0바이트부터)
		glBufferSubData(GL_ARRAY_BUFFER, vertices_offset, vertices_size, static_cast<GLvoid const*>(vertices.data()));
		glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::vertices));
		glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::vertices), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(0x0));

		// 뒷부분에 texcoords 넣고, 읽는 법 알려주기 (float 2개씩, texcoords_offset부터)
		glBufferSubData(GL_ARRAY_BUFFER, texcoords_offset, texcoords_size, static_cast<GLvoid const*>(texcoords.data()));
		glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::texcoords));
		glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::texcoords), 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(texcoords_offset));

		// index용 buffer(data.ibo) 만들고 bind (삼각형 번호 목록)
		glGenBuffers(1, &data.ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ibo);

		// buffer에 index 데이터 복사 (크기 = 삼각형 개수 × uvec3 크기)
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, /*! \todo how many bytes should the buffer contain? */
			static_cast<GLsizeiptr>(index_sets.size() * sizeof(glm::uvec3)),
			/* where is the data stored on the CPU? */index_sets.data(),
			/* inform OpenGL that the data is modified once, but used often */GL_STATIC_DRAW);

		// 총 index 개수 = 삼각형 개수 × 3
		data.indices_nb = /*! \todo how many indices do we have? */
			static_cast<GLsizei>(index_sets.size() * 3u);

		// 설정 끝, bind 풀기
		glBindVertexArray(0u);
		glBindBuffer(GL_ARRAY_BUFFER, 0u);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);

		return data;
	}

bonobo::mesh_data
parametric_shapes::createSphere(float const radius,
                                unsigned int const longitude_split_count,
                                unsigned int const latitude_split_count)
{
	//! \todo Implement this function
	auto const longitude_edges_count = longitude_split_count + 1u; 
	auto const latitude_edges_count = latitude_split_count + 1u;
	auto const longitude_vertices_count = longitude_edges_count + 1u; 
	auto const latitude_vertices_count = latitude_edges_count + 1u;
	auto const vertices_count = longitude_vertices_count * latitude_vertices_count;

	// vertex attributes 
	auto vertices = std::vector<glm::vec3>(vertices_count);
	auto normals = std::vector<glm::vec3>(vertices_count); // 법선, 머리 위 
	auto tangents = std::vector<glm::vec3>(vertices_count); // 접선, 동쪽으로 걸음, 세타 증가
	auto binormals = std::vector<glm::vec3>(vertices_count); // 종법선, 북쪽으로 걸음, 파이 증가
	auto texcoords = std::vector<glm::vec3>(vertices_count);

	float const theta_step = glm::two_pi<float>() / static_cast<float>(longitude_edges_count); // 한 바퀴 / 칸 수
	float const phi_step = glm::pi<float>() / static_cast<float>(latitude_edges_count); // 반 바퀴 / 칸 수 

	size_t index = 0u;
	for (unsigned int i = 0u; i < longitude_vertices_count; ++i) {
		float const theta = i * theta_step;         
		float const cos_theta = std::cos(theta);
		float const sin_theta = std::sin(theta);

		for (unsigned int j = 0u; j < latitude_vertices_count; ++j) {
			float const phi = j * phi_step;
			float const cos_phi = std::cos(phi);
			float const sin_phi = std::sin(phi);

			vertices[index] = glm::vec3(radius * sin_theta * sin_phi, -radius * cos_phi, radius * cos_theta * sin_phi);
			// tangents[index] = glm::normalize(glm::vec3(radius * cos_theta * sin_phi, 0.0f, -radius * sin_theta * sin_phi)); // without simplifying
			tangents[index] = glm::normalize(glm::vec3(cos_theta, 0.0f, -sin_theta));
			binormals[index] = glm::normalize(glm::vec3(sin_theta * cos_phi, sin_phi, cos_theta * cos_phi));
			normals[index] = glm::normalize(glm::cross(tangents[index], binormals[index]));
			texcoords[index] = glm::vec3(static_cast<float>(i) / static_cast<float>(longitude_edges_count),
				static_cast<float>(j) / static_cast<float>(latitude_edges_count),
				0.0f);

			index++;
		}
	}

	// create index array
	auto index_sets = std::vector<glm::uvec3>(2u * longitude_edges_count * latitude_edges_count);
	index = 0u;

	// generate indices iteratively
	for (unsigned int i = 0u; i < longitude_edges_count; ++i)
	{
		for (unsigned int j = 0u; j < latitude_edges_count; ++j)
		{
			// (i, j) = i * 한 줄의 정점 수(latitude_vertices_count) + j
			// counter clock-wise = front
			index_sets[index] = glm::uvec3(latitude_vertices_count * (i + 0u) + (j + 0u),   // v00
				latitude_vertices_count * (i + 1u) + (j + 0u),   // v10
				latitude_vertices_count * (i + 1u) + (j + 1u));  // v11 
			++index;

			index_sets[index] = glm::uvec3(latitude_vertices_count * (i + 0u) + (j + 0u),   // v00 
				latitude_vertices_count * (i + 1u) + (j + 1u),   // v11
				latitude_vertices_count * (i + 0u) + (j + 1u));  // v01 
			++index;
		}
	}

	bonobo::mesh_data data;
	glGenVertexArrays(1, &data.vao);
	assert(data.vao != 0u);
	glBindVertexArray(data.vao);

	auto const vertices_offset = 0u;
	auto const vertices_size = static_cast<GLsizeiptr>(vertices.size() * sizeof(glm::vec3));
	auto const normals_offset = vertices_size;
	auto const normals_size = static_cast<GLsizeiptr>(normals.size() * sizeof(glm::vec3));
	auto const texcoords_offset = normals_offset + normals_size;
	auto const texcoords_size = static_cast<GLsizeiptr>(texcoords.size() * sizeof(glm::vec3));
	auto const tangents_offset = texcoords_offset + texcoords_size;
	auto const tangents_size = static_cast<GLsizeiptr>(tangents.size() * sizeof(glm::vec3));
	auto const binormals_offset = tangents_offset + tangents_size;
	auto const binormals_size = static_cast<GLsizeiptr>(binormals.size() * sizeof(glm::vec3));
	auto const bo_size = static_cast<GLsizeiptr>(vertices_size
		+ normals_size
		+ texcoords_size
		+ tangents_size
		+ binormals_size
		);
	glGenBuffers(1, &data.bo);
	assert(data.bo != 0u);
	glBindBuffer(GL_ARRAY_BUFFER, data.bo);
	glBufferData(GL_ARRAY_BUFFER, bo_size, nullptr, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, vertices_offset, vertices_size, static_cast<GLvoid const*>(vertices.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::vertices));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::vertices), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(0x0));

	glBufferSubData(GL_ARRAY_BUFFER, normals_offset, normals_size, static_cast<GLvoid const*>(normals.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::normals));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::normals), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(normals_offset));

	glBufferSubData(GL_ARRAY_BUFFER, texcoords_offset, texcoords_size, static_cast<GLvoid const*>(texcoords.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::texcoords));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::texcoords), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(texcoords_offset));

	glBufferSubData(GL_ARRAY_BUFFER, tangents_offset, tangents_size, static_cast<GLvoid const*>(tangents.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::tangents));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::tangents), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(tangents_offset));

	glBufferSubData(GL_ARRAY_BUFFER, binormals_offset, binormals_size, static_cast<GLvoid const*>(binormals.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::binormals));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::binormals), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(binormals_offset));

	glBindBuffer(GL_ARRAY_BUFFER, 0u);

	data.indices_nb = static_cast<GLsizei>(index_sets.size() * 3u);
	glGenBuffers(1, &data.ibo);
	assert(data.ibo != 0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(index_sets.size() * sizeof(glm::uvec3)), reinterpret_cast<GLvoid const*>(index_sets.data()), GL_STATIC_DRAW);

	glBindVertexArray(0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);

	return data;
}

bonobo::mesh_data
parametric_shapes::createTorus(float const major_radius,
                               float const minor_radius,
                               unsigned int const major_split_count,
                               unsigned int const minor_split_count)
{
	//! \todo (Optional) Implement this function
	auto const major_edges_count = major_split_count + 1u;
	auto const minor_edges_count = minor_split_count + 1u;
	auto const major_vertices_count = major_edges_count + 1u;
	auto const minor_vertices_count = minor_edges_count + 1u;
	auto const vertices_count = major_vertices_count * minor_vertices_count;

	auto vertices = std::vector<glm::vec3>(vertices_count);
	auto normals = std::vector<glm::vec3>(vertices_count);
	auto tangents = std::vector<glm::vec3>(vertices_count);
	auto binormals = std::vector<glm::vec3>(vertices_count);
	auto texcoords = std::vector<glm::vec3>(vertices_count);

	float const phi_step = glm::two_pi<float>() / static_cast<float>(major_edges_count);
	float const theta_step = glm::two_pi<float>() / static_cast<float>(minor_edges_count);

	size_t index = 0u;

	for (unsigned int i = 0u; i < major_vertices_count; ++i) {
		//float const cos_theta = std::cos(theta);
		//float const sin_theta = std::sin(theta);

		for (unsigned int j = 0u; j < minor_vertices_count; ++j) {

			index++; 
		}

	}

	auto index_sets = std::vector<glm::uvec3>(2u * major_edges_count * minor_edges_count);
	index = 0u;

	bonobo::mesh_data data;
	glGenVertexArrays(1, &data.vao);
	assert(data.vao != 0u);
	glBindVertexArray(data.vao);

	auto const vertices_offset = 0u;
	auto const vertices_size = static_cast<GLsizeiptr>(vertices.size() * sizeof(glm::vec3));
	auto const normals_offset = vertices_size;
	auto const normals_size = static_cast<GLsizeiptr>(normals.size() * sizeof(glm::vec3));
	auto const texcoords_offset = normals_offset + normals_size;
	auto const texcoords_size = static_cast<GLsizeiptr>(texcoords.size() * sizeof(glm::vec3));
	auto const tangents_offset = texcoords_offset + texcoords_size;
	auto const tangents_size = static_cast<GLsizeiptr>(tangents.size() * sizeof(glm::vec3));
	auto const binormals_offset = tangents_offset + tangents_size;
	auto const binormals_size = static_cast<GLsizeiptr>(binormals.size() * sizeof(glm::vec3));
	auto const bo_size = static_cast<GLsizeiptr>(vertices_size
		+ normals_size
		+ texcoords_size
		+ tangents_size
		+ binormals_size
		);
	glGenBuffers(1, &data.bo);
	assert(data.bo != 0u);
	glBindBuffer(GL_ARRAY_BUFFER, data.bo);
	glBufferData(GL_ARRAY_BUFFER, bo_size, nullptr, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, vertices_offset, vertices_size, static_cast<GLvoid const*>(vertices.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::vertices));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::vertices), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(0x0));

	glBufferSubData(GL_ARRAY_BUFFER, normals_offset, normals_size, static_cast<GLvoid const*>(normals.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::normals));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::normals), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(normals_offset));

	glBufferSubData(GL_ARRAY_BUFFER, texcoords_offset, texcoords_size, static_cast<GLvoid const*>(texcoords.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::texcoords));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::texcoords), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(texcoords_offset));

	glBufferSubData(GL_ARRAY_BUFFER, tangents_offset, tangents_size, static_cast<GLvoid const*>(tangents.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::tangents));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::tangents), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(tangents_offset));

	glBufferSubData(GL_ARRAY_BUFFER, binormals_offset, binormals_size, static_cast<GLvoid const*>(binormals.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::binormals));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::binormals), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(binormals_offset));

	glBindBuffer(GL_ARRAY_BUFFER, 0u);

	data.indices_nb = static_cast<GLsizei>(index_sets.size() * 3u);
	glGenBuffers(1, &data.ibo);
	assert(data.ibo != 0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(index_sets.size() * sizeof(glm::uvec3)), reinterpret_cast<GLvoid const*>(index_sets.data()), GL_STATIC_DRAW);

	glBindVertexArray(0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);

	return data;
}

bonobo::mesh_data
parametric_shapes::createCircleRing(float const radius,
                                    float const spread_length,
                                    unsigned int const circle_split_count,
                                    unsigned int const spread_split_count)
{
	auto const circle_slice_edges_count = circle_split_count + 1u;
	auto const spread_slice_edges_count = spread_split_count + 1u;
	auto const circle_slice_vertices_count = circle_slice_edges_count + 1u;
	auto const spread_slice_vertices_count = spread_slice_edges_count + 1u;
	auto const vertices_nb = circle_slice_vertices_count * spread_slice_vertices_count;

	auto vertices  = std::vector<glm::vec3>(vertices_nb);
	auto normals   = std::vector<glm::vec3>(vertices_nb);
	auto texcoords = std::vector<glm::vec3>(vertices_nb);
	auto tangents  = std::vector<glm::vec3>(vertices_nb);
	auto binormals = std::vector<glm::vec3>(vertices_nb);

	float const spread_start = radius - 0.5f * spread_length;
	float const d_theta = glm::two_pi<float>() / (static_cast<float>(circle_slice_edges_count));
	float const d_spread = spread_length / (static_cast<float>(spread_slice_edges_count));

	// generate vertices iteratively
	size_t index = 0u;
	float theta = 0.0f;
	for (unsigned int i = 0u; i < circle_slice_vertices_count; ++i) {
		float const cos_theta = std::cos(theta);
		float const sin_theta = std::sin(theta);

		float distance_to_centre = spread_start;
		for (unsigned int j = 0u; j < spread_slice_vertices_count; ++j) {
			// vertex
			vertices[index] = glm::vec3(distance_to_centre * cos_theta,
			                            distance_to_centre * sin_theta,
			                            0.0f);

			// texture coordinates
			texcoords[index] = glm::vec3(static_cast<float>(j) / (static_cast<float>(spread_slice_vertices_count)),
			                             static_cast<float>(i) / (static_cast<float>(circle_slice_vertices_count)),
			                             0.0f);

			// tangent
			auto const t = glm::vec3(cos_theta, sin_theta, 0.0f);
			tangents[index] = t;

			// binormal
			auto const b = glm::vec3(-sin_theta, cos_theta, 0.0f);
			binormals[index] = b;

			// normal
			auto const n = glm::cross(t, b);
			normals[index] = n;

			distance_to_centre += d_spread;
			++index;
		}

		theta += d_theta;
	}

	// create index array
	auto index_sets = std::vector<glm::uvec3>(2u * circle_slice_edges_count * spread_slice_edges_count);

	// generate indices iteratively
	index = 0u;
	for (unsigned int i = 0u; i < circle_slice_edges_count; ++i)
	{
		for (unsigned int j = 0u; j < spread_slice_edges_count; ++j)
		{
			index_sets[index] = glm::uvec3(spread_slice_vertices_count * (i + 0u) + (j + 0u),
			                               spread_slice_vertices_count * (i + 0u) + (j + 1u),
			                               spread_slice_vertices_count * (i + 1u) + (j + 1u));
			++index;

			index_sets[index] = glm::uvec3(spread_slice_vertices_count * (i + 0u) + (j + 0u),
			                               spread_slice_vertices_count * (i + 1u) + (j + 1u),
			                               spread_slice_vertices_count * (i + 1u) + (j + 0u));
			++index;
		}
	}

	bonobo::mesh_data data;
	glGenVertexArrays(1, &data.vao);
	assert(data.vao != 0u);
	glBindVertexArray(data.vao);

	auto const vertices_offset = 0u;
	auto const vertices_size = static_cast<GLsizeiptr>(vertices.size() * sizeof(glm::vec3));
	auto const normals_offset = vertices_size;
	auto const normals_size = static_cast<GLsizeiptr>(normals.size() * sizeof(glm::vec3));
	auto const texcoords_offset = normals_offset + normals_size;
	auto const texcoords_size = static_cast<GLsizeiptr>(texcoords.size() * sizeof(glm::vec3));
	auto const tangents_offset = texcoords_offset + texcoords_size;
	auto const tangents_size = static_cast<GLsizeiptr>(tangents.size() * sizeof(glm::vec3));
	auto const binormals_offset = tangents_offset + tangents_size;
	auto const binormals_size = static_cast<GLsizeiptr>(binormals.size() * sizeof(glm::vec3));
	auto const bo_size = static_cast<GLsizeiptr>(vertices_size
	                                            +normals_size
	                                            +texcoords_size
	                                            +tangents_size
	                                            +binormals_size
	                                            );
	glGenBuffers(1, &data.bo);
	assert(data.bo != 0u);
	glBindBuffer(GL_ARRAY_BUFFER, data.bo);
	glBufferData(GL_ARRAY_BUFFER, bo_size, nullptr, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, vertices_offset, vertices_size, static_cast<GLvoid const*>(vertices.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::vertices));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::vertices), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(0x0));

	glBufferSubData(GL_ARRAY_BUFFER, normals_offset, normals_size, static_cast<GLvoid const*>(normals.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::normals));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::normals), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(normals_offset));

	glBufferSubData(GL_ARRAY_BUFFER, texcoords_offset, texcoords_size, static_cast<GLvoid const*>(texcoords.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::texcoords));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::texcoords), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(texcoords_offset));

	glBufferSubData(GL_ARRAY_BUFFER, tangents_offset, tangents_size, static_cast<GLvoid const*>(tangents.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::tangents));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::tangents), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(tangents_offset));

	glBufferSubData(GL_ARRAY_BUFFER, binormals_offset, binormals_size, static_cast<GLvoid const*>(binormals.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::binormals));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::binormals), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(binormals_offset));

	glBindBuffer(GL_ARRAY_BUFFER, 0u);

	data.indices_nb = static_cast<GLsizei>(index_sets.size() * 3u);
	glGenBuffers(1, &data.ibo);
	assert(data.ibo != 0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(index_sets.size() * sizeof(glm::uvec3)), reinterpret_cast<GLvoid const*>(index_sets.data()), GL_STATIC_DRAW);

	glBindVertexArray(0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);

	return data;
}
