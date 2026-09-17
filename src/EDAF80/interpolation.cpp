#include "interpolation.hpp"

glm::vec3
interpolation::evalLERP(glm::vec3 const& p0, glm::vec3 const& p1, float const x)
{
	//! \todo Implement this function
	return glm::mix(p0, p1, x);
}

glm::vec3
interpolation::evalCatmullRom(glm::vec3 const& p0, glm::vec3 const& p1,
	glm::vec3 const& p2, glm::vec3 const& p3,
	float const t, float const x)
{
	//! \todo Implement this function
	// column-major ordering
	glm::mat4 const M = glm::mat4(
		glm::vec4(0.0f, -t, 2.0f * t, -t),
		glm::vec4(1.0f, 0.0f, t - 3.0f, 2.0f - t),
		glm::vec4(0.0f, t, 3.0f - 2.0f * t, t - 2.0f),
		glm::vec4(0.0f, 0.0f, -t, t)
	);
	glm::vec4 const w = glm::vec4(1.0f, x, x * x, x * x * x) * M;
	return w[0] * p0 + w[1] * p1 + w[2] * p2 + w[3] * p3;
}
