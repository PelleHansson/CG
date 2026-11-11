# Set up the executable.
add_executable(raytracer)
target_sources(
  raytracer
  PRIVATE
    [[main.cpp]]
    [[stb_image.h]]
    [[stb_image_write.h]]
    [[swCamera.cpp]]
    [[swCamera.h]]
    [[swCamera.cpp]]
    [[swCamera.h]]
    [[swIntersection.cpp]]
    [[swIntersection.h]]
    [[swMaterial.h]] 
    [[swPrimitive.h]]
    [[swRay.h]]
    [[swScene.cpp]]
    [[swScene.h]]
    [[swSphere.cpp]]
    [[swSphere.h]]
    [[swTriangle.cpp]]
    [[swTriangle.h]]
    [[swVec3.h]]
)
target_compile_definitions(
  raytracer
  PRIVATE
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:MSVC>>:NOMINMAX>
)
target_compile_features(raytracer PRIVATE cxx_std_11)
target_compile_options(
  raytracer
  PRIVATE
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:MSVC>>:/utf-8;/Zc:__cplusplus>
)
copy_dlls (raytracer "${CMAKE_CURRENT_BINARY_DIR}")



# Add OpenMP support if available for optional exercise.
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(raytracer PUBLIC OpenMP::OpenMP_CXX)
endif()
