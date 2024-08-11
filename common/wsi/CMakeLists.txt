
add_library(wsi OsWindow.cpp OsWindowingManager.cpp)

target_include_directories(wsi PUBLIC ..)

<<<<<<< HEAD
target_link_libraries(wsi PUBLIC glm::glm function2::function2)
target_link_libraries(wsi PRIVATE glfw etna Tracy::TracyClient)
=======
target_link_libraries(wsi PUBLIC glm::glm)
target_link_libraries(wsi PRIVATE glfw etna)
>>>>>>> f0b31be (Initial migration from vk_graphics_basic)
