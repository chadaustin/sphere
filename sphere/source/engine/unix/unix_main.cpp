#include "unix_time.h"
#include "../sphere.hpp"
#include <stdlib.h>

int main(int argc, const char** argv) {
	srand((unsigned)GetTime);
	RunSphere(argc, argv);
}
