#include <SDL.h>
#include "../sphere.hpp"
#include "sdl_internal.hpp"
#include "sdl_filesystem.hpp"


void ProcessEvent(SDL_Event event);

////////////////////////////////////////////////////////////////////////////////

extern "C"
int main(int argc, char* argv[])
{
  // initialize
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("Unable to initialize SDL: %s\n", SDL_GetError());
    return 1;
  }

  atexit(SDL_Quit);

  // initialize video
  if (InitVideo(GetDefaultWidth(), GetDefaultHeight()) == false) {
    printf("Unable to initialize video subsystem");
    return 1;
  }

  // initialize audio
  if (InitAudio() == false) {
    printf("Unable to initialize audio subsystem");
    return 1;
  }

  if (InitInput() == false) {
    printf("Unable to initialize input subsystem");
    return 1;
  }

  if (InitNetworkSystem() == false) {
    printf("Unable to initialize network subsystem");
    return 1;
  }

  // initialize screenshot directory
  std::string sphere_directory;
  char screenshot_directory[512];
  GetDirectory(sphere_directory);
  sprintf(screenshot_directory, "%s\\screenshots", sphere_directory.c_str());
  SetScreenshotDirectory(screenshot_directory);

  // run Sphere!
  UpdateSystem();
  RunSphere(argc, const_cast<const char**>(argv));
  UpdateSystem();

  CloseInput();
  CloseAudio();
  CloseVideo();
  CloseNetworkSystem();
  puts("Goodbye from SDL Sphere!");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

void UpdateSystem()
{
  //printf("Puming events\n");
  //SDL_PumpEvents();

  static SDL_Event event;

  if (SDL_PollEvent(&event)) {
    // do something with the event (not sure what yet)
    ProcessEvent(event);
  }
}

////////////////////////////////////////////////////////////////////////////////

void QuitMessage(const char* message)
{
  puts(message);
  exit(1);
}

////////////////////////////////////////////////////////////////////////////////

void ProcessEvent(SDL_Event event)
{
  switch(event.type)
  {
    case SDL_QUIT:
      // the most important message if you *ever* want to quit ;)
      CloseInput();
      CloseVideo();
      CloseAudio();
      puts("Goodbye from SDL Sphere!");

      exit(0);
      break;

    ////////////////////////////////////////////////////////////////////////////

    case SDL_KEYDOWN:
      //char buf[80];
      //sprintf(buf, "Key pressed: %i", event.key.keysym.sym);
      //puts(buf);
      
      if (event.key.keysym.sym == SDLK_F11) {
        ToggleFPS();
      }
      else if (event.key.keysym.sym == SDLK_F12) {
        ShouldTakeScreenshot = true;
      }
      else {
        OnKeyDown(event.key.keysym.sym);
      }
      break;

    ////////////////////////////////////////////////////////////////////////////

    case SDL_KEYUP:
      //char buf[80];
      //sprintf(buf, "Key released: %i", event.key.keysym.sym);
      //puts(buf);

      OnKeyUp(event.key.keysym.sym);
      break;

    ////////////////////////////////////////////////////////////////////////////

    case SDL_MOUSEMOTION:
      OnMouseMove(event.motion.x, event.motion.y);
      break;

    case SDL_MOUSEBUTTONDOWN:
      OnMouseDown(event.button.button);
      break;

    case SDL_MOUSEBUTTONUP:
      OnMouseUp(event.button.button);
      break;
  }
}