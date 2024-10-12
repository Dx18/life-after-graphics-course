include(${PROJECT_SOURCE_DIR}/cmake/common.cmake)

add_executable(myrenderer
  main.cpp
  Renderer.cpp
  WorldRenderer.cpp
  App.cpp
)

target_link_libraries(myrenderer
  PRIVATE glfw etna glm::glm wsi gui scene render_utils)

target_add_shaders(myrenderer
  shaders/g_buffer_pass.vert
  shaders/g_buffer_pass.frag
  shaders/resolve_pass.vert
  shaders/resolve_pass.frag
)

target_shader_include_directories(myrenderer ../common/scene/light)
