#ifndef WIN32_AUDIO_HPP
#define WIN32_AUDIO_HPP


#include <audiere.h>
#include "../../common/IFileSystem.hpp"


typedef void* SA_CONTEXT;


extern void SA_PushFileSystem(IFileSystem* fs);
extern void SA_PopFileSystem();
extern ADR_CONTEXT SA_GetAudiereContext();


#endif
