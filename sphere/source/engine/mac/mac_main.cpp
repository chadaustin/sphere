#include <SDL.h>
#include <iostream>
#include <stdlib.h>
#include <string>

#include "mac_time.h"
#include "mac_internal.h"
#include "mac_filesystem.h"
#include "mac_audio.h"
#include "mac_input.h"
#include "mac_sphere_config.h"
#include "../PlayerConfig.hpp"
#include "../sphere.hpp"

#ifndef DATADIR
#define DATADIR "."
#endif

#define MAX_PATH 256

////////////////////////////////////////////////////////////////////////////////

int KeyStringToKeyCode(const char* key_string)
{
    if (strlen(key_string) >= strlen("KEY_X"))
    {
        if (strlen(key_string) == strlen("KEY_X"))
        {
            switch (key_string[strlen("KEY_")])
            {
                case 'A': return KEY_A;
                case 'B': return KEY_B;
                case 'C': return KEY_C;
                case 'D': return KEY_D;
                case 'E': return KEY_E;
                case 'F': return KEY_F;
                case 'G': return KEY_G;
                case 'H': return KEY_H;
                case 'I': return KEY_I;
                case 'J': return KEY_J;
                case 'K': return KEY_K;
                case 'L': return KEY_L;
                case 'M': return KEY_M;
                case 'N': return KEY_N;
                case 'O': return KEY_O;
                case 'P': return KEY_P;
                case 'Q': return KEY_Q;
                case 'R': return KEY_R;
                case 'S': return KEY_S;
                case 'T': return KEY_T;
                case 'U': return KEY_U;
                case 'V': return KEY_V;
                case 'W': return KEY_W;
                case 'X': return KEY_X;
                case 'Y': return KEY_Y;
                case 'Z': return KEY_Z;
                case '0': return KEY_0;
                case '1': return KEY_1;
                case '2': return KEY_2;
                case '3': return KEY_3;
                case '4': return KEY_4;
                case '5': return KEY_5;
                case '6': return KEY_6;
                case '7': return KEY_7;
                case '8': return KEY_8;
                case '9': return KEY_9;
            }

            return -1;
        }

        if (strlen(key_string) == strlen("KEY_NUM_X"))
        {
            if (strcmp(key_string, "KEY_NUM_0") == 0) return KEY_NUM_0;
            if (strcmp(key_string, "KEY_NUM_1") == 0) return KEY_NUM_1;
            if (strcmp(key_string, "KEY_NUM_2") == 0) return KEY_NUM_2;
            if (strcmp(key_string, "KEY_NUM_3") == 0) return KEY_NUM_3;
            if (strcmp(key_string, "KEY_NUM_4") == 0) return KEY_NUM_4;
            if (strcmp(key_string, "KEY_NUM_5") == 0) return KEY_NUM_5;
            if (strcmp(key_string, "KEY_NUM_6") == 0) return KEY_NUM_6;
            if (strcmp(key_string, "KEY_NUM_7") == 0) return KEY_NUM_7;
            if (strcmp(key_string, "KEY_NUM_8") == 0) return KEY_NUM_8;
            if (strcmp(key_string, "KEY_NUM_9") == 0) return KEY_NUM_9;
        }

        if (strcmp(key_string, "KEY_UP")            == 0) return KEY_UP;
        if (strcmp(key_string, "KEY_DOWN")          == 0) return KEY_DOWN;
        if (strcmp(key_string, "KEY_LEFT")          == 0) return KEY_LEFT;
        if (strcmp(key_string, "KEY_RIGHT")         == 0) return KEY_RIGHT;

        if (strcmp(key_string, "KEY_TILDE")         == 0) return KEY_TILDE;
        if (strcmp(key_string, "KEY_MINUS")         == 0) return KEY_MINUS;
        if (strcmp(key_string, "KEY_EQUALS")        == 0) return KEY_EQUALS;
        if (strcmp(key_string, "KEY_SPACE")         == 0) return KEY_SPACE;
        if (strcmp(key_string, "KEY_OPENBRACE")     == 0) return KEY_OPENBRACE;
        if (strcmp(key_string, "KEY_CLOSEBRACE")    == 0) return KEY_CLOSEBRACE;
        if (strcmp(key_string, "KEY_SEMICOLON")     == 0) return KEY_SEMICOLON;
        if (strcmp(key_string, "KEY_COMMA")         == 0) return KEY_COMMA;
        if (strcmp(key_string, "KEY_APOSTROPHE")    == 0) return KEY_APOSTROPHE;
        if (strcmp(key_string, "KEY_PERIOD")        == 0) return KEY_PERIOD;
        if (strcmp(key_string, "KEY_SLASH")         == 0) return KEY_SLASH;
        if (strcmp(key_string, "KEY_BACKSLASH")     == 0) return KEY_BACKSLASH;
    }

    return -1;
}

////////////////////////////////////////////////////////////////////////////////

static void LoadSphereConfiguration(SPHERECONFIG* config)
{
    // Loads configuration settings
    LoadSphereConfig(config, (GetSphereDirectory() + "/engine.ini").c_str());
}

////////////////////////////////////////////////////////////////////////////////

extern "C"
int main(int argc, char* argv[])
{
    SetSphereDirectory();

    // load the configuration settings, then save it for future reference
    SPHERECONFIG config;
    LoadSphereConfiguration(&config);

    for (int i = 0; i < 4; i++)
    {
        SetPlayerConfig(i,
            KeyStringToKeyCode(config.player_configurations[i].key_up_str.c_str()),
            KeyStringToKeyCode(config.player_configurations[i].key_down_str.c_str()),
            KeyStringToKeyCode(config.player_configurations[i].key_left_str.c_str()),
            KeyStringToKeyCode(config.player_configurations[i].key_right_str.c_str()),
            config.player_configurations[i].keyboard_input_allowed,
            config.player_configurations[i].joypad_input_allowed);
    }

    SaveSphereConfig(&config, (GetSphereDirectory() + "/engine.ini").c_str());

    // initialize screenshot directory
    std::string sphere_directory;
    char screenshot_directory[512];
    GetDirectory(sphere_directory);
    sprintf(screenshot_directory, "%s/screenshots", sphere_directory.c_str());
    SetScreenshotDirectory(screenshot_directory);

    // initialize video subsystem
    if (InitVideo(&config) == false)
    {
        printf("Video subsystem could not be initialized...\n");
        return 0;
    }

    // initialize input
    InitInput();

    // initialize audio
    if (!InitAudio(&config))
    {
        printf("Sound could not be initialized...\n");
    }

    RunSphere(argc, argv);
    CloseVideo();
    CloseAudio();
}

////////////////////////////////////////////////////////////////////////////////

void QuitMessage (const char* message)
{
    CloseVideo();
    CloseAudio();

    std::cerr << message << std::endl;
    exit(1);
}

////////////////////////////////////////////////////////////////////////////////
