#include "PlayerConfig.hpp"

static __PLAYERCONFIG__ player_configurations[4];

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
                     bool use_joypad)
{
    if (player >= 0 && player < 4)
    {
        player_configurations[player].key_menu               = key_menu;
        player_configurations[player].key_up                 = key_up;
        player_configurations[player].key_down               = key_down;
        player_configurations[player].key_left               = key_left;
        player_configurations[player].key_right              = key_right;
        player_configurations[player].key_a                  = key_a;
        player_configurations[player].key_b                  = key_b;
        player_configurations[player].key_x                  = key_x;
        player_configurations[player].key_y                  = key_y;
        player_configurations[player].keyboard_input_allowed = use_keyboard;
        player_configurations[player].joypad_input_allowed   = use_joypad;
    }
}

struct __PLAYERCONFIG__* GetPlayerConfig(int player)
{
    if (player >= 0 && player < 4)
        return &player_configurations[player];

    return 0;
}
