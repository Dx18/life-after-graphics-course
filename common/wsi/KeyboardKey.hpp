#pragma once

#include "wsi/KeyToGlfw.hpp"


// This file allows us to avoid including GLFW everywhere
enum class KeyboardKey
{
#define X(key, glfwKey) key,
  ALL_KEYBOARD_KEYS
#undef X

<<<<<<< HEAD
    COUNT,
=======
  COUNT,
>>>>>>> f0b31be (Initial migration from vk_graphics_basic)
};
