#include "OsWindow.hpp"

#include <GLFW/glfw3.h>
#include <etna/Assert.hpp>

#include "OsWindowingManager.hpp"


OsWindow::~OsWindow()
{
  OsWindowingManager::onWindowDestroyed(impl);
}

void OsWindow::askToClose()
{
  glfwSetWindowShouldClose(impl, GLFW_TRUE);
}

bool OsWindow::isBeingClosed()
{
<<<<<<< HEAD
  return glfwWindowShouldClose(impl) == GLFW_TRUE;
=======
  return glfwWindowShouldClose(impl);
>>>>>>> f0b31be (Initial migration from vk_graphics_basic)
}

glm::uvec2 OsWindow::getResolution()
{
  glm::ivec2 result;
  glfwGetWindowSize(impl, &result.x, &result.y);
<<<<<<< HEAD
  ETNA_VERIFY(result.x >= 0 && result.y >= 0);
=======
  ETNA_ASSERT(result.x >= 0 && result.y >= 0);
>>>>>>> f0b31be (Initial migration from vk_graphics_basic)
  return glm::uvec2(result);
}

vk::UniqueSurfaceKHR OsWindow::createVkSurface(vk::Instance instance)
{
  VkSurfaceKHR surface;
  auto cres = glfwCreateWindowSurface(instance, impl, nullptr, &surface);
  ETNA_CHECK_VK_RESULT(static_cast<vk::Result>(cres));

  return vk::UniqueSurfaceKHR{
<<<<<<< HEAD
    vk::SurfaceKHR{surface},
    vk::ObjectDestroy<vk::Instance, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE>{instance}};
=======
    surface,
    vk::ObjectDestroy<vk::Instance, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE>{instance}
  };
>>>>>>> f0b31be (Initial migration from vk_graphics_basic)
}
