#include <iostream>
#include <stdlib.h>
#include "unix_input.h"

void UpdateSystem () {
  RefreshInput();
}

void QuitMessage (const char* message) {
  std::cerr << message << std::endl;
  exit(1);
}
