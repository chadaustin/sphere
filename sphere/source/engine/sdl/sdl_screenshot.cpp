#include <string>
#include <time.h>
#include "../win32/win32_filesystem.hpp"
#include "sdl_internal.hpp"
#include "sdl_video.hpp"
#include "../../common/Image32.hpp"


static std::string s_ScreenshotDirectory;


bool ShouldTakeScreenshot;


////////////////////////////////////////////////////////////////////////////////

void SetScreenshotDirectory(const char* directory)
{
  s_ScreenshotDirectory = directory;
}

////////////////////////////////////////////////////////////////////////////////

void TakeScreenshot()
{
  CImage32 screen(GetScreenWidth(), GetScreenHeight());
  DirectGrab(0, 0, GetScreenWidth(), GetScreenHeight(), screen.GetPixels());

  // try to create the screenshot directory
  MakeDirectory(s_ScreenshotDirectory.c_str());

  // build a filename based on current date and time
  time_t current_time;
  tm*    today;
  time(&current_time);
  today = localtime(&current_time);

  char filename[512];
  strftime(filename, 512, "screenshot %Y.%m.%d.%H.%M.%S.png", today);

  // now write the screenshot to disk
  std::string save_path;
  save_path =  s_ScreenshotDirectory + '/';
  save_path += filename;
  screen.Save(save_path.c_str());
}

////////////////////////////////////////////////////////////////////////////////