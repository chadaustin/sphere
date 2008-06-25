#ifndef PLAYER_CONFIG_HPP
#define PLAYER_CONFIG_HPP


struct __PLAYERCONFIG__
{
    int key_menu;
    int key_up;
    int key_down;
    int key_left;
    int key_right;
    int key_a;
    int key_b;
    int key_x;
    int key_y;
    bool keyboard_input_allowed;
    bool joypad_input_allowed;
};

void SetPlayerConfig(int player,
                     int key_menu,
                     int key_up,
                     int key_down,
                     int key_left,
                     int key_right,
                     int key_a,
                     int key_b,
                     int key_x,
                     int key_y,
                     bool use_keyboard,
                     bool use_joypad);

struct __PLAYERCONFIG__* GetPlayerConfig(int player);


#endif
