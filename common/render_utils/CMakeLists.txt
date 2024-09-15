<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> f2882a9 (Added task 1)
add_library(render_utils
  LineRenderer.cpp QuadRenderer.cpp)

target_include_directories(render_utils PUBLIC ..)

# Allows C++ code to inlcude C++-GLSL compat
target_include_directories(render_utils INTERFACE shaders)
# Allow GLSL code to include helper files and compat
target_shader_include_directories(render_utils INTERFACE shaders)

target_link_libraries(render_utils PUBLIC etna glm::glm)


target_add_shaders(render_utils
  shaders/quad.vert
  shaders/quad.frag
  shaders/plain_color.vert
  shaders/plain_color.frag
)
=======

add_library(render_utils QuadRenderer.cpp)

target_include_directories(render_utils PUBLIC ..)

target_link_libraries(render_utils PUBLIC etna)
>>>>>>> f0b31be (Initial migration from vk_graphics_basic)
