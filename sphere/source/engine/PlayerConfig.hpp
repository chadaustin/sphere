#ifndef PLAYER_CONFIG_HPP
#define PLAYER_CONFIG_HPP


struct __PLAYERCONFIG__ {
  int key_up;
  int key_down;
  int key_left;
  int key_right;
  bool keyboard_input_allowed;
  bool joypad_input_allowed;
};

void SetPlayerConfig(int player, int key_up, int key_down, int key_left, int key_right, bool use_keyboard, bool use_joypad);
struct __PLAYERCONFIG__* GetPlayerConfig(int player);

#endif
