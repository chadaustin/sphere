#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include "internal.h"
#include "../sphere.h"


////////////////////////////////////////////////////////////////////////////////

int main(int argc, const char** argv)
{
  char CommandLine[1024];
  int i;

  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    printf("Error: Could not initialize SDL (%s)\n", SDL_GetError());
    return EXIT_FAILURE;
  }

  // Initializes graphics driver
  if (InitGraphics() == false)
  {
    SDL_Quit();
    printf("Error: Could not initialize graphics subsystem\n");
    return EXIT_FAILURE;
  }

  // Initializes audio subsystem
  if (InitSoundSystem() == false)
  {
    SDL_Quit();
    CloseGraphics();
    puts("Error: Could not initialize sound subsystem\n");
    return EXIT_FAILURE;
  }

  // Initializes input subsystem
  if (InitInput() == false)
  {
    SDL_Quit();
    CloseSoundSystem();
    CloseGraphics();
    puts("Error: Could not initialize input subsystem\n");
    return EXIT_FAILURE;
  }

  // build the command line
  strcpy(CommandLine, "");
  for (i = 0; i < argc; i++)
  {
    strcat(CommandLine, argv[i]);
    strcat(CommandLine, " ");
  }

  // run the engine
  UpdateSystem();
  StartSphere(CommandLine);
  UpdateSystem();

  // Clean up
  CloseInput();
  CloseSoundSystem();
  CloseGraphics();
  UpdateSystem();  // process messages here before SDL is shut down
  SDL_Quit();

  return EXIT_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////

void QuitMessage(const char* message)
{
  UpdateSystem();
  CloseInput();
  CloseSoundSystem();
  CloseGraphics();
  UpdateSystem();
  SDL_Quit();
  
  printf("%s\n", message);
  exit(EXIT_FAILURE);
}

////////////////////////////////////////////////////////////////////////////////

void UpdateSystem(void)
{
  SDL_Event event;

  while (SDL_PollEvent(&event))
  {
    switch (event.type)
    {
      case SDL_KEYDOWN:
        CurrentKeyBuffer[event.key.keysym.scancode] = 1; 
        break;

      case SDL_KEYUP:
        CurrentKeyBuffer[event.key.keysym.scancode] = 0;
        break;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
