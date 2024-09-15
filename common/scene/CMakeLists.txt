<<<<<<< HEAD
<<<<<<< HEAD
add_library(scene
  SceneLights.cpp SceneManager.cpp SceneMaterials.cpp SceneMeshes.cpp)

target_include_directories(scene PUBLIC ..)

target_link_libraries(scene PUBLIC glm::glm tinygltf etna render_utils)
=======

add_library(scene SceneManager.cpp)

target_include_directories(scene PUBLIC ..)

target_link_libraries(scene PUBLIC glm::glm tinygltf etna)
>>>>>>> f0b31be (Initial migration from vk_graphics_basic)
=======
add_library(scene
  SceneLights.cpp SceneManager.cpp SceneMaterials.cpp SceneMeshes.cpp)

target_include_directories(scene PUBLIC ..)

target_link_libraries(scene PUBLIC glm::glm tinygltf etna render_utils)
>>>>>>> f2882a9 (Added task 1)
