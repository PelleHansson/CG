#include "interpolation.hpp"

glm::vec3
interpolation::evalLERP(glm::vec3 const& p0, glm::vec3 const& p1, float const x)
{
	//! \todo Implement this function


	//glm::mat2 m = glm::mat2(1, -1, 0, 1);
	//glm::vec2 w = glm::vec2(1, x);
	//glm::vec3 result = glm::vec3();
	//for (size_t i = 0; i < 3; i++)
	//{
	//	result[i] = glm::dot(w, (m * glm::vec2(p0[i], p1[i])));
	//}


	

	return mix(p0,p1,x);
}

glm::vec3
interpolation::evalCatmullRom(glm::vec3 const& p0, glm::vec3 const& p1,
                              glm::vec3 const& p2, glm::vec3 const& p3,
                              float const t, float const x)
{

	glm::mat4 m = glm::transpose(glm::mat4(
								0    , 1    , 0        ,  0 ,
								-t   , 0    , t        ,  0 ,
								2 * t, t - 3, 3 - 2 * t, -t ,
								-t	 , 2 - t, t - 2	   , t
	)); 
	//rotated for glm interfdace

	glm::vec4 w = glm::vec4(1, x ,  x * x,   x * x * x);
	glm::vec3 result = glm::vec3();
	for (size_t i = 0; i < 3; i++)
	{
		
		result[i] = glm::dot(w, (m * glm::vec4(p0[i], p1[i], p2[i], p3[i])));
	}
	//! \todo Implement this function
	return result;
}
