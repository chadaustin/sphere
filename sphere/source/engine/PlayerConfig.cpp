#include "PlayerConfig.hpp"
static __PLAYERCONFIG__ player_configurations[4];
void SetPlayerConfig(int player,
                     int key_up, int key_down, int key_left, int key_right,
                     bool use_keyboard, bool use_joypad)
{
    if (player >= 0 && player < 4)
    {

        player_configurations[player].key_up    = key_up;
        player_configurations[player].key_down  = key_down;
        player_configurations[player].key_left  = key_left;
        player_configurations[player].key_right = key_right;
        player_configurations[player].keyboard_input_allowed = use_keyboard;
        player_configurations[player].joypad_input_allowed   = use_joypad;
    }
}
struct __PLAYERCONFIG__* GetPlayerConfig(int player)
{

    if (player >= 0 && player < 4)
    {

        return &player_configurations[player];
    }
    return 0;
}
