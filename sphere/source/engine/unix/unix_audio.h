#ifndef UNIX_AUDIO_H
#define UNIX_AUDIO_H

#include "../../common/IFileSystem.hpp"

#include "audiere.h"
#define ADR_TRUE  1
#define ADR_FALSE 0

bool InitAudio();
void CloseAudio();

static ADR_FILE ADR_CALL FileOpen(void* opaque, const char* filename);
static void     ADR_CALL FileClose(ADR_FILE file);
static int      ADR_CALL FileRead(ADR_FILE file, void* buffer, int size);
#ifdef ADR_NEW
static ADR_BOOL ADR_CALL FileSeek(ADR_FILE file, int destination, ADR_SEEK_TYPE type);
#else
static int      ADR_CALL FileSeek(ADR_FILE file, int destination);
#endif
static int      ADR_CALL FileTell(ADR_FILE file);

void SA_PushFileSystem(IFileSystem* fs);
void SA_PopFileSystem();
ADR_CONTEXT SA_GetAudiereContext();

#endif /* UNIX_AUDIO_H */
