#include <windows.h>
#include <stdio.h>
#include <list>
#include "win32_audio.hpp"
#include "win32_internal.hpp"
#include "win32_sphere_config.hpp"


static std::list<IFileSystem*> s_FileSystems;
static ADR_CONTEXT s_AudiereContext;


static ADR_FILE ADR_CALL FileOpen(void* opaque, const char* filename);
static void     ADR_CALL FileClose(ADR_FILE file);
static int      ADR_CALL FileRead(ADR_FILE file, void* buffer, int size);
static int      ADR_CALL FileSeek(ADR_FILE file, int destination);
static int      ADR_CALL FileTell(ADR_FILE file);


////////////////////////////////////////////////////////////////////////////////

bool InitAudio(HWND window, SPHERECONFIG* config)
{
  // create the context
  s_AudiereContext = AdrCreateContext(
    "autodetect", "",
    NULL,
    FileOpen,
    FileClose,
    FileRead,
    FileSeek,
    FileTell
  );

  return (s_AudiereContext != NULL);
}

////////////////////////////////////////////////////////////////////////////////

void CloseAudio()
{
  // destroy the context
  AdrDestroyContext(s_AudiereContext);
}

////////////////////////////////////////////////////////////////////////////////

static ADR_FILE ADR_CALL FileOpen(void* opaque, const char* filename)
{
  // for each file system...
  std::list<IFileSystem*>::iterator i = s_FileSystems.begin();
  while (i != s_FileSystems.end()) {
    
    IFileSystem* fs = *i;
    IFile* file = fs->Open(filename, IFileSystem::read);
    if (file) {
      return (ADR_FILE)file;
    }

    ++i;
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////

static void ADR_CALL FileClose(ADR_FILE file)
{
  IFile* f = (IFile*)file;
  f->Close();
}

////////////////////////////////////////////////////////////////////////////////

static int ADR_CALL FileRead(ADR_FILE file, void* buffer, int size)
{
  IFile* f = (IFile*)file;
  return f->Read(buffer, size);
}

////////////////////////////////////////////////////////////////////////////////

static int ADR_CALL FileSeek(ADR_FILE file, int destination)
{
  IFile* f = (IFile*)file;
  f->Seek(destination);
  return 1;
}

////////////////////////////////////////////////////////////////////////////////

static int ADR_CALL FileTell(ADR_FILE file)
{
  IFile* f = (IFile*)file;
  return f->Tell();
}

////////////////////////////////////////////////////////////////////////////////

void SA_PushFileSystem(IFileSystem* fs)
{
  s_FileSystems.push_front(fs);
}

////////////////////////////////////////////////////////////////////////////////

void SA_PopFileSystem()
{
  s_FileSystems.pop_front();
}

////////////////////////////////////////////////////////////////////////////////

ADR_CONTEXT SA_GetAudiereContext()
{
  return s_AudiereContext;
}

////////////////////////////////////////////////////////////////////////////////
