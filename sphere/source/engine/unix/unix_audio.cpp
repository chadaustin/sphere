#include "unix_audio.h"

/* all this file is going to do is intercept audio calls and
   fake like it's doing work, perhaps once Audiere is ported
   this will change */

void SA_PushFileSystem (IFileSystem* fs) {
}

void SA_PopFileSystem () {
}

ADR_CONTEXT SA_GetAudiereContext () {
}
