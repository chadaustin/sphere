#include "../common/stub.hpp"


int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR command_line, int)
{
  return Run("config.exe", instance, command_line);
}
