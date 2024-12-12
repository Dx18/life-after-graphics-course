include(${PROJECT_SOURCE_DIR}/cmake/common.cmake)

set(GENERATED_CPP_INCLUDE_DIRECTORY
  ${CMAKE_CURRENT_BINARY_DIR}/generated/cpp)
set(GENERATED_CPP_DIRECTORY
  ${GENERATED_CPP_INCLUDE_DIRECTORY}/generated)

set(GENERATED_GLSL_INCLUDE_DIRECTORY
  ${CMAKE_CURRENT_BINARY_DIR}/generated/glsl)
set(GENERATED_GLSL_DIRECTORY
  ${GENERATED_GLSL_INCLUDE_DIRECTORY}/generated)

file(MAKE_DIRECTORY
  ${GENERATED_CPP_DIRECTORY}
  ${GENERATED_GLSL_DIRECTORY}
)

add_executable(diffuse_baking
  main.cpp
  Renderer.cpp
  DiffuseBakingDemoRenderer.cpp
  App.cpp
)

target_include_directories(diffuse_baking
  PRIVATE ${GENERATED_CPP_INCLUDE_DIRECTORY})

target_link_libraries(diffuse_baking
  PRIVATE glfw etna glm::glm wsi gui scene render_utils)

add_custom_target(sphere_mesh_inc
  python3 ${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen_sphere_mesh.py
  18 36
  ${GENERATED_GLSL_DIRECTORY}/SphereMesh.inc
  ${GENERATED_CPP_DIRECTORY}/SphereMesh.hpp
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen_sphere_mesh.py
)

add_custom_target(random_table_inc
  python3 ${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen_random_table.py
  1024
  ${GENERATED_GLSL_DIRECTORY}/RandomTable.inc
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen_random_table.py
)

add_custom_target(sphere_samples_inc
  python3 ${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen_sphere_samples.py
  36
  ${GENERATED_GLSL_DIRECTORY}/SphereSamples.inc
  ${GENERATED_CPP_DIRECTORY}/SphereSamples.hpp
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen_sphere_samples.py
)

add_custom_target(spherical_harmonic_samples_inc
  python3 ${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen_spherical_harmonic_samples.py
  48 2
  ${GENERATED_GLSL_DIRECTORY}/SphericalHarmonicSamples.inc
  ${GENERATED_CPP_DIRECTORY}/SphericalHarmonicSamples.hpp
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen_spherical_harmonic_samples.py
)

target_add_shaders(diffuse_baking
  shaders/baking/diffuse_spherical_harmonics.comp
  shaders/shading/dynamic_diffuse.frag
  shaders/shading/spherical_harmonics_diffuse.frag
  shaders/cubemap.vert
  shaders/cubemap.frag
  shaders/scene_blend.vert
  shaders/scene_blend.frag
  shaders/sphere.vert
)

# target_shader_include_directories(diffuse_baking ../../common/scene/light)
target_shader_include_directories(diffuse_baking ${GENERATED_GLSL_INCLUDE_DIRECTORY})

add_dependencies(diffuse_baking_shaders sphere_mesh_inc)
add_dependencies(diffuse_baking_shaders random_table_inc)
add_dependencies(diffuse_baking_shaders sphere_samples_inc)
add_dependencies(diffuse_baking_shaders spherical_harmonic_samples_inc)
