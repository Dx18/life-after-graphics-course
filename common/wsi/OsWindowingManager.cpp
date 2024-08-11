#include "OsWindowingManager.hpp"

#include <GLFW/glfw3.h>
#include <etna/Assert.hpp>

<<<<<<< HEAD
#include <tracy/Tracy.hpp>

=======
>>>>>>> f0b31be (Initial migration from vk_graphics_basic)

static OsWindowingManager* instance = nullptr;

void OsWindowingManager::onErrorCb(int /*errc*/, const char* message)
{
  spdlog::error("GLFW: {}", message);
}

void OsWindowingManager::onMouseScrollCb(GLFWwindow* window, double xoffset, double yoffset)
{
  if (auto it = instance->windows.find(window); it != instance->windows.end())
    it->second->mouse.scrollDelta = {xoffset, yoffset};
}

void OsWindowingManager::onWindowClosedCb(GLFWwindow* window)
{
  instance->windows.erase(window);
}

void OsWindowingManager::onWindowRefreshCb(GLFWwindow* window)
{
  if (auto it = instance->windows.find(window); it != instance->windows.end())
    if (it->second->onRefresh)
      it->second->onRefresh();
}

void OsWindowingManager::onWindowSizeCb(GLFWwindow* window, int width, int height)
{
  if (auto it = instance->windows.find(window); it != instance->windows.end())
    if (it->second->onResize)
<<<<<<< HEAD
      it->second->onResize({static_cast<glm::uint>(width), static_cast<glm::uint>(height)});
=======
      it->second->onResize({static_cast<glm::uint>(width),
        static_cast<glm::uint>(height)});
>>>>>>> f0b31be (Initial migration from vk_graphics_basic)
}

OsWindowingManager::OsWindowingManager()
{
<<<<<<< HEAD
  ETNA_VERIFY(glfwInit() == GLFW_TRUE);

  glfwSetErrorCallback(&OsWindowingManager::onErrorCb);

  ETNA_VERIFY(std::exchange(instance, this) == nullptr);
=======
  ETNA_ASSERT(glfwInit() == GLFW_TRUE);

  glfwSetErrorCallback(&OsWindowingManager::onErrorCb);

  ETNA_ASSERT(std::exchange(instance, this) == nullptr);
>>>>>>> f0b31be (Initial migration from vk_graphics_basic)
}

OsWindowingManager::~OsWindowingManager()
{
<<<<<<< HEAD
  ETNA_VERIFY(std::exchange(instance, nullptr) == this);
=======
  ETNA_ASSERT(std::exchange(instance, nullptr) == this);
>>>>>>> f0b31be (Initial migration from vk_graphics_basic)

  glfwTerminate();
}

void OsWindowingManager::poll()
{
<<<<<<< HEAD
  ZoneScoped;

  for (auto [_, window] : windows)
    window->mouse.scrollDelta = {0, 0};

  glfwPollEvents();

  for (auto [_, window] : windows)
=======
  glfwPollEvents();

  for (auto[_, window] : windows)
>>>>>>> f0b31be (Initial migration from vk_graphics_basic)
    updateWindow(*window);
}

double OsWindowingManager::getTime()
{
  return glfwGetTime();
}

<<<<<<< HEAD
std::unique_ptr<OsWindow> OsWindowingManager::createWindow(OsWindow::CreateInfo info)
{
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, info.resizeable ? GLFW_TRUE : GLFW_FALSE);

  auto glfwWindow = glfwCreateWindow(
    static_cast<int>(info.resolution.x),
    static_cast<int>(info.resolution.y),
    "Sample",
    nullptr,
    nullptr);
=======
std::unique_ptr<OsWindow> OsWindowingManager::createWindow(glm::uvec2 resolution,
  OsWindowRefreshCb refreshCb, OsWindowResizeCb resizeCb)
{
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  auto glfwWindow = glfwCreateWindow(static_cast<int>(resolution.x), static_cast<int>(resolution.y), "Sample", nullptr, nullptr);
>>>>>>> f0b31be (Initial migration from vk_graphics_basic)

  glfwSetScrollCallback(glfwWindow, &onMouseScrollCb);
  glfwSetWindowCloseCallback(glfwWindow, &onWindowClosedCb);
  glfwSetWindowRefreshCallback(glfwWindow, &onWindowRefreshCb);
  glfwSetWindowSizeCallback(glfwWindow, &onWindowSizeCb);

  auto result = std::unique_ptr<OsWindow>{new OsWindow};
  result->owner = this;
  result->impl = glfwWindow;
<<<<<<< HEAD
  result->onRefresh = std::move(info.refreshCb);
  result->onResize = std::move(info.resizeCb);
=======
  result->onRefresh = std::move(refreshCb);
  result->onResize = std::move(resizeCb);
>>>>>>> f0b31be (Initial migration from vk_graphics_basic)

  windows.emplace(glfwWindow, result.get());
  return result;
}

std::span<const char*> OsWindowingManager::getRequiredVulkanInstanceExtensions()
{
  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
  return {glfwExtensions, glfwExtensionCount};
}

void OsWindowingManager::onWindowDestroyed(GLFWwindow* impl)
{
  instance->windows.erase(impl);
  glfwDestroyWindow(impl);
}

void OsWindowingManager::updateWindow(OsWindow& window)
{
<<<<<<< HEAD
  auto transitionState = [](ButtonState& state, bool glfw_state) {
    switch (state)
    {
    case ButtonState::Low:
      if (glfw_state)
        state = ButtonState::Rising;
      break;
    case ButtonState::Rising:
      state = ButtonState::High;
      break;
    case ButtonState::High:
      if (!glfw_state)
        state = ButtonState::Falling;
      break;
    case ButtonState::Falling:
      state = ButtonState::Low;
      break;
    default:
      break;
    }
  };

  auto processMb = [&window, &transitionState](MouseButton mb, int glfw_mb) {
    const bool pressed = glfwGetMouseButton(window.impl, glfw_mb) == GLFW_PRESS;
    transitionState(window.mouse.buttons[static_cast<std::size_t>(mb)], pressed);
  };

#define X(mb, glfwMb) processMb(MouseButton::mb, glfwMb);
  ALL_MOUSE_BUTTONS
#undef X

  auto processKey = [&window, &transitionState](KeyboardKey key, int glfw_key) {
    const bool pressed = glfwGetKey(window.impl, glfw_key) == GLFW_PRESS;
    transitionState(window.keyboard.keys[static_cast<std::size_t>(key)], pressed);
  };

#define X(mb, glfwMb) processKey(KeyboardKey::mb, glfwMb);
  ALL_KEYBOARD_KEYS
#undef X
=======
  auto transitionState = [](ButtonState& state, bool glfwState) {
      switch (state)
      {
        case ButtonState::Low: if (glfwState) state = ButtonState::Rising; break;
        case ButtonState::Rising: state = ButtonState::High; break;
        case ButtonState::High: if (!glfwState) state = ButtonState::Falling; break;
        case ButtonState::Falling: state = ButtonState::Low; break;
        default: break;
      }
    };

  auto processMb = [&window, &transitionState](MouseButton mb, int glfwMb) {
    const bool pressed = glfwGetMouseButton(window.impl, glfwMb) == GLFW_PRESS;
    transitionState(window.mouse.buttons[static_cast<std::size_t>(mb)], pressed);
  };

  #define X(mb, glfwMb) processMb(MouseButton::mb, glfwMb);
  ALL_MOUSE_BUTTONS
  #undef X

  auto processKey = [&window, &transitionState](KeyboardKey key, int glfwKey) {
    const bool pressed = glfwGetKey(window.impl, glfwKey) == GLFW_PRESS;
    transitionState(window.keyboard.keys[static_cast<std::size_t>(key)], pressed);
  };

  #define X(mb, glfwMb) processKey(KeyboardKey::mb, glfwMb);
  ALL_KEYBOARD_KEYS
  #undef X
>>>>>>> f0b31be (Initial migration from vk_graphics_basic)

  if (window.captureMouse)
  {
    // Skip first frame on which mouse was captured so as not to
    // "jitter" the camera due to big offset from 0,0
    if (window.mouseWasCaptured)
    {
      double x;
      double y;
      glfwGetCursorPos(window.impl, &x, &y);
      glfwSetCursorPos(window.impl, 0, 0);

<<<<<<< HEAD
      window.mouse.capturedPosDelta = {x, y};
=======
      window.mouse.posDelta = {x, y};
>>>>>>> f0b31be (Initial migration from vk_graphics_basic)
    }
    else
    {
      glfwSetInputMode(window.impl, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      glfwSetCursorPos(window.impl, 0, 0);
      window.mouseWasCaptured = true;
    }
<<<<<<< HEAD

    window.mouse.freePos = {0, 0};
=======
>>>>>>> f0b31be (Initial migration from vk_graphics_basic)
  }
  else
  {
    glfwSetInputMode(window.impl, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    window.mouseWasCaptured = false;
<<<<<<< HEAD

    double x;
    double y;
    glfwGetCursorPos(window.impl, &x, &y);
    window.mouse.freePos = {x, y};
    window.mouse.capturedPosDelta = {0, 0};
=======
>>>>>>> f0b31be (Initial migration from vk_graphics_basic)
  }
}
