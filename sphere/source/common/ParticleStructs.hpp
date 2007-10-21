#ifndef PARTICLE_STRUCTS_HPP
#define PARTICLE_STRUCTS_HPP

#include "rgb.hpp"
#include "VectorStructs.hpp"

#define STRUCT_NAME PARTICLE
#define STRUCT_BODY     \
	VECTOR_INT scr_pos;   \
	VECTOR_FLO cur_pos;	  \
	VECTOR_FLO vel;	      \
	RGBA       color;	  \
	int        length;	  \
	float      life;	  \
	float      aging;
#include "packed_struct.h"

#endif

