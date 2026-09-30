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

	unsigned int vertices_nb = (horizontal_split_count+1) * (vertical_split_count+1);
	std::vector<glm::vec3> vertices = std::vector<glm::vec3>(vertices_nb);

	std::vector<glm::vec2> texcoords = std::vector<glm::vec2>(vertices_nb);
	std::vector<glm::uvec3> index_sets = std::vector<glm::uvec3>(2u *horizontal_split_count * vertical_split_count);

	size_t index = 0u;
	for(unsigned int i = 0u; i < horizontal_split_count +1 ; ++i) {
		float const x =  width * static_cast<float>(i) / static_cast<float>(horizontal_split_count) ;
		for (unsigned int j = 0u; j < vertical_split_count +1 ; ++j) {

			float const z =  height * static_cast<float>(j) /static_cast<float>(vertical_split_count) ;
			vertices[index] = glm::vec3(x, 0.0f, z);
			texcoords[index] = glm::vec2(static_cast<float>(i) / static_cast<float>(horizontal_split_count),
			                             static_cast<float>(j) / static_cast<float>(vertical_split_count));
			++index;
		}
	}

	index = 0u;
	int const v_s_count = vertical_split_count + 1;
	
	for (unsigned int i = 0u; i < horizontal_split_count; ++i) {
		for (unsigned int j = 0u; j < vertical_split_count; ++j) {


			index_sets[index++] = glm::uvec3(v_s_count * i + j,	//current node
				v_s_count * (i + 1) + (j), //next right
				v_s_count * (i + 1) + (j + 1)); //diagonal node	
	
			index_sets[index++] = glm::uvec3(v_s_count * i + j,	//current node
				v_s_count * (i + 1) + (j + 1), //diagonal
				v_s_count * (i)+(j + 1));  // next one above
		}
	}
	
	bonobo::mesh_data data;

		glGenVertexArrays(1, &data.vao);
		glBindVertexArray(data.vao);
		glGenBuffers(1, &data.bo);

	
		glBindBuffer(GL_ARRAY_BUFFER, data.bo);

		auto const vertices_offset = 0u;
		auto const vertices_size = static_cast<GLsizeiptr>(vertices.size() * sizeof(glm::vec3));
		
		auto const texcoords_offset = vertices_size;
		auto const texcoords_size =	static_cast<GLsizeiptr>(texcoords.size() * sizeof(glm::vec2));

		glBufferData(GL_ARRAY_BUFFER, vertices_size+ texcoords_size,
					 /* where is the data stored on the CPU? */nullptr,
					 /* inform OpenGL that the data is modified once, but used often */GL_STATIC_DRAW);

		// enables the input for vertices:

		
		glBufferSubData(GL_ARRAY_BUFFER, vertices_offset, vertices_size, static_cast<GLvoid const*>(vertices.data()));
		glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::vertices));
		glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::vertices), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(0x0));

		glBufferSubData(GL_ARRAY_BUFFER, texcoords_offset, texcoords_size, static_cast<GLvoid const*>(texcoords.data()));
		glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::texcoords));
		glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::texcoords), 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(texcoords_offset));

		// Now, let's allocate a second one for the indices.
		//
		// Have the buffer's name stored into `data.ibo`.
		glGenBuffers(1, &data.ibo);

		// We still want a 1D-array, but this time it should be a 1D-array of
		// elements, aka. indices!
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ibo);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_sets.size() * sizeof(index_sets[0]),
					 /* where is the data stored on the CPU? */index_sets.data(),
					 /* inform OpenGL that the data is modified once, but used often */GL_STATIC_DRAW);

		data.indices_nb = static_cast<GLsizei>(index_sets.size() * 3u);


	// All the data has been recorded, we can unbind them.
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
	unsigned int const longitude_slice_vertices_count = longitude_split_count + 1u;
	unsigned int const latitude_slice_vertices_count  = latitude_split_count + 1u;

	unsigned int const vertices_nb = longitude_slice_vertices_count * latitude_slice_vertices_count;

	std::vector<glm::vec3> vertices  = std::vector<glm::vec3>(vertices_nb);
	std::vector<glm::vec3> normals   = std::vector<glm::vec3>(vertices_nb);
	std::vector<glm::vec3> texcoords = std::vector<glm::vec3>(vertices_nb);
	std::vector<glm::vec3> tangents  = std::vector<glm::vec3>(vertices_nb);
	std::vector<glm::vec3> binormals = std::vector<glm::vec3>(vertices_nb);

	float const d_theta = glm::two_pi<float>() / (static_cast<float>(longitude_split_count));
	float const d_phi = glm::pi<float>() / (static_cast<float>(latitude_split_count)); //halfcircle




	float theta = 0.0f;
	
	size_t index = 0u;
	for (unsigned int i = 0u; i < longitude_slice_vertices_count; ++i) {
		float const cos_theta = std::cos(theta);
		float const sin_theta = std::sin(theta);

		float phi = 0.0f;
		for (unsigned int j = 0u; j < latitude_slice_vertices_count; ++j) {
			float const cos_phi = std::cos(phi);
			float const sin_phi = std::sin(phi);

			
			//pos
			vertices[index] = glm::vec3(radius * sin_theta * sin_phi,
										-radius *cos_phi,
									radius * cos_theta * sin_phi);
			

			texcoords[index] = glm::vec3(static_cast<float>(i) / static_cast<float>(longitude_split_count),
										static_cast<float>(j) / static_cast<float>(latitude_split_count),
										0.0f);
			//tangent
			auto const t =glm::normalize(glm::vec3(cos_theta ,
										0.0f,
									-sin_theta ));
			tangents[index] = t;
			auto const b = glm::normalize(glm::vec3(radius * sin_theta * cos_phi,
										radius* sin_phi,
									radius * cos_theta * cos_phi));
			binormals[index] = b;
			auto const n = glm::cross(t, b);
			normals[index] = n;

			phi += d_phi;
			index++;
		}
		theta += d_theta;
		
	}
	std::vector<glm::uvec3> index_sets = std::vector<glm::uvec3>(2u * longitude_split_count * latitude_split_count);

	index = 0u;
	for (unsigned int i = 0u; i < longitude_split_count; ++i) {
		for (unsigned int j = 0u; j < latitude_split_count; ++j) {
			index_sets[index] = glm::uvec3(latitude_slice_vertices_count * i + j,	//current node
				latitude_slice_vertices_count * (i+1) + (j), //next right
				latitude_slice_vertices_count * (i + 1) + (j + 1)); //diagonal node

			//				(3)
			//	 		 /	 I 	
			//	 	 /		 I 
			//  (1) ------- (2)


			index++;
			index_sets[index] = glm::uvec3(latitude_slice_vertices_count* i + j,	//current node
											latitude_slice_vertices_count* (i +1) + (j + 1), //diagonal
											latitude_slice_vertices_count * (i ) + (j+1));  // next one above

		//	(3)--------(2)
		//	 I 		 /	 	
		//	 I 	 /		 
		//  (1) 

			index++;
			
		}
	}
	std::cout << "number of index_sets : " << index << "\n";
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
	//auto const spread_slice_vertices_count = 
	//! \todo Implement this function
	return data;
}

bonobo::mesh_data
parametric_shapes::createTorus(float const major_radius,
                               float const minor_radius,
                               unsigned int const major_split_count,
                               unsigned int const minor_split_count)
{
	//! \todo (Optional) Implement this function
	return bonobo::mesh_data();
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
