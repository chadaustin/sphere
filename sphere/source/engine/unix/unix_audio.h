#ifndef UNIX_AUDIO_H
#define UNIX_AUDIO_H

#include "../../common/IFileSystem.hpp"

typedef void* ADR_CONTEXT;

void SA_PushFileSystem(IFileSystem* fs);
void SA_PopFileSystem();
ADR_CONTEXT SA_GetAudiereContext();

#endif /* UNIX_AUDIO_H */
