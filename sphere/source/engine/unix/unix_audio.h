#ifndef WIN32_AUDIO_HPP
#define WIN32_AUDIO_HPP


#include <audiere.h>


audiere::OutputStream* SA_OpenSound(audiere::File* file, bool streaming);


#endif
