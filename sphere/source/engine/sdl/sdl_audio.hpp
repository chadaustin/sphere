#ifndef SDL_AUDIO_HPP
#define SDL_AUDIO_HPP


#include "../../common/IFileSystem.hpp"

typedef struct SA_SOUNDimp* SA_SOUND;


#define SA_CALL __stdcall


typedef void* (SA_CALL * SA_FileOpenCallback)(const char* filename);
typedef void  (SA_CALL * SA_FileCloseCallback)(void* file);
typedef int   (SA_CALL * SA_FileReadCallback)(void* file, int bytes, void* buffer);
typedef void  (SA_CALL * SA_FileSeekCallback)(void* file, int position);
typedef int   (SA_CALL * SA_FileTellCallback)(void* file);
typedef int   (SA_CALL * SA_FileSizeCallback)(void* file);


extern void  SA_SetPrivate(void* data);
extern void* SA_GetPrivate();
extern void  SA_SetFileCallbacks(
  SA_FileOpenCallback  open,
  SA_FileCloseCallback close,
  SA_FileReadCallback  read,
  SA_FileSeekCallback  seek,
  SA_FileTellCallback  tell,
  SA_FileSizeCallback  size);

extern SA_SOUND SA_LoadSound(const char* filename);
extern void     SA_DestroySound(SA_SOUND sound);
extern void     SA_PlaySound(SA_SOUND sound, bool repeat);
extern void     SA_StopSound(SA_SOUND sound);
extern void     SA_SetVolume(SA_SOUND sound, int volume);
extern int      SA_GetVolume(SA_SOUND sound);
extern int      SA_GetLength(SA_SOUND sound);
extern void     SA_SetPosition(SA_SOUND sound, int position);
extern int      SA_GetPosition(SA_SOUND sound);
extern int      SA_IsPlaying(SA_SOUND sound);

#endif
