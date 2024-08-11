#pragma once

// X-macro to have single source of truth
<<<<<<< HEAD
#define ALL_KEYBOARD_KEYS                                                                          \
  X(kSpace, GLFW_KEY_SPACE)                                                                        \
  X(kApostrophe, GLFW_KEY_APOSTROPHE)                                                              \
  X(kComma, GLFW_KEY_COMMA)                                                                        \
  X(kMinus, GLFW_KEY_MINUS)                                                                        \
  X(kPeriod, GLFW_KEY_PERIOD)                                                                      \
  X(kSlash, GLFW_KEY_SLASH)                                                                        \
  X(k0, GLFW_KEY_0)                                                                                \
  X(k1, GLFW_KEY_1)                                                                                \
  X(k2, GLFW_KEY_2)                                                                                \
  X(k3, GLFW_KEY_3)                                                                                \
  X(k4, GLFW_KEY_4)                                                                                \
  X(k5, GLFW_KEY_5)                                                                                \
  X(k6, GLFW_KEY_6)                                                                                \
  X(k7, GLFW_KEY_7)                                                                                \
  X(k8, GLFW_KEY_8)                                                                                \
  X(k9, GLFW_KEY_9)                                                                                \
  X(kSemicolon, GLFW_KEY_SEMICOLON)                                                                \
  X(kEqual, GLFW_KEY_EQUAL)                                                                        \
  X(kA, GLFW_KEY_A)                                                                                \
  X(kB, GLFW_KEY_B)                                                                                \
  X(kC, GLFW_KEY_C)                                                                                \
  X(kD, GLFW_KEY_D)                                                                                \
  X(kE, GLFW_KEY_E)                                                                                \
  X(kF, GLFW_KEY_F)                                                                                \
  X(kG, GLFW_KEY_G)                                                                                \
  X(kH, GLFW_KEY_H)                                                                                \
  X(kI, GLFW_KEY_I)                                                                                \
  X(kJ, GLFW_KEY_J)                                                                                \
  X(kK, GLFW_KEY_K)                                                                                \
  X(kL, GLFW_KEY_L)                                                                                \
  X(kM, GLFW_KEY_M)                                                                                \
  X(kN, GLFW_KEY_N)                                                                                \
  X(kO, GLFW_KEY_O)                                                                                \
  X(kP, GLFW_KEY_P)                                                                                \
  X(kQ, GLFW_KEY_Q)                                                                                \
  X(kR, GLFW_KEY_R)                                                                                \
  X(kS, GLFW_KEY_S)                                                                                \
  X(kT, GLFW_KEY_T)                                                                                \
  X(kU, GLFW_KEY_U)                                                                                \
  X(kV, GLFW_KEY_V)                                                                                \
  X(kW, GLFW_KEY_W)                                                                                \
  X(kX, GLFW_KEY_X)                                                                                \
  X(kY, GLFW_KEY_Y)                                                                                \
  X(kZ, GLFW_KEY_Z)                                                                                \
  X(kLeftBracket, GLFW_KEY_LEFT_BRACKET)                                                           \
  X(kBackslash, GLFW_KEY_BACKSLASH)                                                                \
  X(kRightBracket, GLFW_KEY_RIGHT_BRACKET)                                                         \
  X(kGraveAccent, GLFW_KEY_GRAVE_ACCENT)                                                           \
  X(kWorld1, GLFW_KEY_WORLD_1)                                                                     \
  X(kWorld2, GLFW_KEY_WORLD_2)                                                                     \
  X(kEscape, GLFW_KEY_ESCAPE)                                                                      \
  X(kEnter, GLFW_KEY_ENTER)                                                                        \
  X(kTab, GLFW_KEY_TAB)                                                                            \
  X(kBackspace, GLFW_KEY_BACKSPACE)                                                                \
  X(kInsert, GLFW_KEY_INSERT)                                                                      \
  X(kDelete, GLFW_KEY_DELETE)                                                                      \
  X(kRight, GLFW_KEY_RIGHT)                                                                        \
  X(kLeft, GLFW_KEY_LEFT)                                                                          \
  X(kDown, GLFW_KEY_DOWN)                                                                          \
  X(kUp, GLFW_KEY_UP)                                                                              \
  X(kPageUp, GLFW_KEY_PAGE_UP)                                                                     \
  X(kPageDown, GLFW_KEY_PAGE_DOWN)                                                                 \
  X(kHome, GLFW_KEY_HOME)                                                                          \
  X(kEnd, GLFW_KEY_END)                                                                            \
  X(kCapsLock, GLFW_KEY_CAPS_LOCK)                                                                 \
  X(kScrollLock, GLFW_KEY_SCROLL_LOCK)                                                             \
  X(kNumLock, GLFW_KEY_NUM_LOCK)                                                                   \
  X(kPrintScreen, GLFW_KEY_PRINT_SCREEN)                                                           \
  X(kPause, GLFW_KEY_PAUSE)                                                                        \
  X(kF1, GLFW_KEY_F1)                                                                              \
  X(kF2, GLFW_KEY_F2)                                                                              \
  X(kF3, GLFW_KEY_F3)                                                                              \
  X(kF4, GLFW_KEY_F4)                                                                              \
  X(kF5, GLFW_KEY_F5)                                                                              \
  X(kF6, GLFW_KEY_F6)                                                                              \
  X(kF7, GLFW_KEY_F7)                                                                              \
  X(kF8, GLFW_KEY_F8)                                                                              \
  X(kF9, GLFW_KEY_F9)                                                                              \
  X(kF10, GLFW_KEY_F10)                                                                            \
  X(kF11, GLFW_KEY_F11)                                                                            \
  X(kF12, GLFW_KEY_F12)                                                                            \
  X(kF13, GLFW_KEY_F13)                                                                            \
  X(kF14, GLFW_KEY_F14)                                                                            \
  X(kF15, GLFW_KEY_F15)                                                                            \
  X(kF16, GLFW_KEY_F16)                                                                            \
  X(kF17, GLFW_KEY_F17)                                                                            \
  X(kF18, GLFW_KEY_F18)                                                                            \
  X(kF19, GLFW_KEY_F19)                                                                            \
  X(kF20, GLFW_KEY_F20)                                                                            \
  X(kF21, GLFW_KEY_F21)                                                                            \
  X(kF22, GLFW_KEY_F22)                                                                            \
  X(kF23, GLFW_KEY_F23)                                                                            \
  X(kF24, GLFW_KEY_F24)                                                                            \
  X(kF25, GLFW_KEY_F25)                                                                            \
  X(kKp0, GLFW_KEY_KP_0)                                                                           \
  X(kKp1, GLFW_KEY_KP_1)                                                                           \
  X(kKp2, GLFW_KEY_KP_2)                                                                           \
  X(kKp3, GLFW_KEY_KP_3)                                                                           \
  X(kKp4, GLFW_KEY_KP_4)                                                                           \
  X(kKp5, GLFW_KEY_KP_5)                                                                           \
  X(kKp6, GLFW_KEY_KP_6)                                                                           \
  X(kKp7, GLFW_KEY_KP_7)                                                                           \
  X(kKp8, GLFW_KEY_KP_8)                                                                           \
  X(kKp9, GLFW_KEY_KP_9)                                                                           \
  X(kKpDecimal, GLFW_KEY_KP_DECIMAL)                                                               \
  X(kKpDivide, GLFW_KEY_KP_DIVIDE)                                                                 \
  X(kKpMultiply, GLFW_KEY_KP_MULTIPLY)                                                             \
  X(kKpSubtract, GLFW_KEY_KP_SUBTRACT)                                                             \
  X(kKpAdd, GLFW_KEY_KP_ADD)                                                                       \
  X(kKpEnter, GLFW_KEY_KP_ENTER)                                                                   \
  X(kKpEqual, GLFW_KEY_KP_EQUAL)                                                                   \
  X(kLeftShift, GLFW_KEY_LEFT_SHIFT)                                                               \
  X(kLeftControl, GLFW_KEY_LEFT_CONTROL)                                                           \
  X(kLeftAlt, GLFW_KEY_LEFT_ALT)                                                                   \
  X(kLeftSuper, GLFW_KEY_LEFT_SUPER)                                                               \
  X(kRightShift, GLFW_KEY_RIGHT_SHIFT)                                                             \
  X(kRightControl, GLFW_KEY_RIGHT_CONTROL)                                                         \
  X(kRightAlt, GLFW_KEY_RIGHT_ALT)                                                                 \
  X(kRightSuper, GLFW_KEY_RIGHT_SUPER)                                                             \
  X(kMenu, GLFW_KEY_MENU)
=======
#define ALL_KEYBOARD_KEYS\
  X(kSpace, GLFW_KEY_SPACE)\
  X(kApostrophe, GLFW_KEY_APOSTROPHE)\
  X(kComma, GLFW_KEY_COMMA)\
  X(kMinus, GLFW_KEY_MINUS)\
  X(kPeriod, GLFW_KEY_PERIOD)\
  X(kSlash, GLFW_KEY_SLASH)\
  X(k0, GLFW_KEY_0)\
  X(k1, GLFW_KEY_1)\
  X(k2, GLFW_KEY_2)\
  X(k3, GLFW_KEY_3)\
  X(k4, GLFW_KEY_4)\
  X(k5, GLFW_KEY_5)\
  X(k6, GLFW_KEY_6)\
  X(k7, GLFW_KEY_7)\
  X(k8, GLFW_KEY_8)\
  X(k9, GLFW_KEY_9)\
  X(kSemicolon, GLFW_KEY_SEMICOLON)\
  X(kEqual, GLFW_KEY_EQUAL)\
  X(kA, GLFW_KEY_A)\
  X(kB, GLFW_KEY_B)\
  X(kC, GLFW_KEY_C)\
  X(kD, GLFW_KEY_D)\
  X(kE, GLFW_KEY_E)\
  X(kF, GLFW_KEY_F)\
  X(kG, GLFW_KEY_G)\
  X(kH, GLFW_KEY_H)\
  X(kI, GLFW_KEY_I)\
  X(kJ, GLFW_KEY_J)\
  X(kK, GLFW_KEY_K)\
  X(kL, GLFW_KEY_L)\
  X(kM, GLFW_KEY_M)\
  X(kN, GLFW_KEY_N)\
  X(kO, GLFW_KEY_O)\
  X(kP, GLFW_KEY_P)\
  X(kQ, GLFW_KEY_Q)\
  X(kR, GLFW_KEY_R)\
  X(kS, GLFW_KEY_S)\
  X(kT, GLFW_KEY_T)\
  X(kU, GLFW_KEY_U)\
  X(kV, GLFW_KEY_V)\
  X(kW, GLFW_KEY_W)\
  X(kX, GLFW_KEY_X)\
  X(kY, GLFW_KEY_Y)\
  X(kZ, GLFW_KEY_Z)\
  X(kLeftBracket, GLFW_KEY_LEFT_BRACKET)\
  X(kBackslash, GLFW_KEY_BACKSLASH)\
  X(kRightBracket, GLFW_KEY_RIGHT_BRACKET)\
  X(kGraveAccent, GLFW_KEY_GRAVE_ACCENT)\
  X(kWorld1, GLFW_KEY_WORLD_1)\
  X(kWorld2, GLFW_KEY_WORLD_2)\
  X(kEscape,GLFW_KEY_ESCAPE)\
  X(kEnter, GLFW_KEY_ENTER)\
  X(kTab, GLFW_KEY_TAB)\
  X(kBackspace, GLFW_KEY_BACKSPACE)\
  X(kInsert, GLFW_KEY_INSERT)\
  X(kDelete, GLFW_KEY_DELETE)\
  X(kRight, GLFW_KEY_RIGHT)\
  X(kLeft, GLFW_KEY_LEFT)\
  X(kDown, GLFW_KEY_DOWN)\
  X(kUp, GLFW_KEY_UP)\
  X(kPageUp, GLFW_KEY_PAGE_UP)\
  X(kPageDown, GLFW_KEY_PAGE_DOWN)\
  X(kHome, GLFW_KEY_HOME)\
  X(kEnd, GLFW_KEY_END)\
  X(kCapsLock, GLFW_KEY_CAPS_LOCK)\
  X(kScrollLock, GLFW_KEY_SCROLL_LOCK)\
  X(kNumLock, GLFW_KEY_NUM_LOCK)\
  X(kPrintScreen, GLFW_KEY_PRINT_SCREEN)\
  X(kPause, GLFW_KEY_PAUSE)\
  X(kF1, GLFW_KEY_F1)\
  X(kF2, GLFW_KEY_F2)\
  X(kF3, GLFW_KEY_F3)\
  X(kF4, GLFW_KEY_F4)\
  X(kF5, GLFW_KEY_F5)\
  X(kF6, GLFW_KEY_F6)\
  X(kF7, GLFW_KEY_F7)\
  X(kF8, GLFW_KEY_F8)\
  X(kF9, GLFW_KEY_F9)\
  X(kF10, GLFW_KEY_F10)\
  X(kF11, GLFW_KEY_F11)\
  X(kF12, GLFW_KEY_F12)\
  X(kF13, GLFW_KEY_F13)\
  X(kF14, GLFW_KEY_F14)\
  X(kF15, GLFW_KEY_F15)\
  X(kF16, GLFW_KEY_F16)\
  X(kF17, GLFW_KEY_F17)\
  X(kF18, GLFW_KEY_F18)\
  X(kF19, GLFW_KEY_F19)\
  X(kF20, GLFW_KEY_F20)\
  X(kF21, GLFW_KEY_F21)\
  X(kF22, GLFW_KEY_F22)\
  X(kF23, GLFW_KEY_F23)\
  X(kF24, GLFW_KEY_F24)\
  X(kF25, GLFW_KEY_F25)\
  X(kKp0, GLFW_KEY_KP_0)\
  X(kKp1, GLFW_KEY_KP_1)\
  X(kKp2, GLFW_KEY_KP_2)\
  X(kKp3, GLFW_KEY_KP_3)\
  X(kKp4, GLFW_KEY_KP_4)\
  X(kKp5, GLFW_KEY_KP_5)\
  X(kKp6, GLFW_KEY_KP_6)\
  X(kKp7, GLFW_KEY_KP_7)\
  X(kKp8, GLFW_KEY_KP_8)\
  X(kKp9, GLFW_KEY_KP_9)\
  X(kKpDecimal, GLFW_KEY_KP_DECIMAL)\
  X(kKpDivide, GLFW_KEY_KP_DIVIDE)\
  X(kKpMultiply, GLFW_KEY_KP_MULTIPLY)\
  X(kKpSubtract, GLFW_KEY_KP_SUBTRACT)\
  X(kKpAdd, GLFW_KEY_KP_ADD)\
  X(kKpEnter, GLFW_KEY_KP_ENTER)\
  X(kKpEqual, GLFW_KEY_KP_EQUAL)\
  X(kLeftShift, GLFW_KEY_LEFT_SHIFT)\
  X(kLeftControl, GLFW_KEY_LEFT_CONTROL)\
  X(kLeftAlt, GLFW_KEY_LEFT_ALT)\
  X(kLeftSuper, GLFW_KEY_LEFT_SUPER)\
  X(kRightShift, GLFW_KEY_RIGHT_SHIFT)\
  X(kRightControl, GLFW_KEY_RIGHT_CONTROL)\
  X(kRightAlt, GLFW_KEY_RIGHT_ALT)\
  X(kRightSuper, GLFW_KEY_RIGHT_SUPER)\
  X(kMenu, GLFW_KEY_MENU)\
>>>>>>> f0b31be (Initial migration from vk_graphics_basic)
