#ifndef UNIX_AUDIO_H
#define UNIX_AUDIO_H

#include "../../common/IFileSystem.hpp"

typedef void* ADR_CONTEXT;
typedef void* ADR_STREAM;
typedef int ADR_BOOL;
#define ADR_TRUE  1
#define ADR_FALSE 0

ADR_STREAM AdrOpenStream(ADR_CONTEXT context, const char* name);
int AdrCloseStream(ADR_STREAM stream);
void AdrSetStreamRepeat(ADR_STREAM stream, ADR_BOOL repeat);
void AdrPlayStream(ADR_STREAM stream);
int AdrPauseStream(...);
int AdrResetStream(...);
int AdrSetStreamVolume(...);
int AdrGetStreamVolume(...);
int AdrSetStreamPan(...);
int AdrGetStreamPan(...);
void SA_PushFileSystem(IFileSystem* fs);
void SA_PopFileSystem();
ADR_CONTEXT SA_GetAudiereContext();

#endif /* UNIX_AUDIO_H */
