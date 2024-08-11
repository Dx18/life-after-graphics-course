#pragma once

#include "wsi/MouseButtonToGlfw.hpp"


enum class MouseButton
{
#define X(mb, glfwMb) mb,
  ALL_MOUSE_BUTTONS
#undef X

<<<<<<< HEAD
    COUNT,
=======
  COUNT,
>>>>>>> f0b31be (Initial migration from vk_graphics_basic)
};
