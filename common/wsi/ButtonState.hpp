#pragma once


enum class ButtonState
{
<<<<<<< HEAD
  Low = 0,     //< Button not active this frame
  Rising = 1,  //< Button was pressed on this exact frame
  High = 2,    //< Button was pressed on one of the previous frames and is being held
=======
  Low = 0, //< Button not active this frame
  Rising = 1, //< Button was pressed on this exact frame
  High = 2, //< Button was pressed on one of the previous frames and is being held
>>>>>>> f0b31be (Initial migration from vk_graphics_basic)
  Falling = 3, //< Button was released on this frame
};

inline bool is_held_down(ButtonState state)
{
  return state == ButtonState::Rising || state == ButtonState::High;
}
