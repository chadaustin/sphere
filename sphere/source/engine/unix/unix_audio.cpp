#include <audiere.h>
#include <stdio.h>
#include <list>
#include "unix_audio.h"

static std::list<IFileSystem*> s_FileSystems;
static ADR_CONTEXT s_AudiereContext;


////////////////////////////////////////////////////////////////////////////////

bool InitAudio()
{
  // create the context
  ADR_CONTEXT_ATTR attr = AdrCreateContextAttr();
  AdrContextAttrSetOutputDevice(attr, "autodetect");
  AdrContextAttrSetFileCallbacks(
    attr, FileOpen, FileClose, FileRead, FileSeek, FileTell);

  s_AudiereContext = AdrCreateContext(attr);
  AdrDestroyContextAttr(attr);

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

#ifdef ADR_NEW
static ADR_BOOL ADR_CALL FileSeek(ADR_FILE file, int destination, ADR_SEEK_TYPE type)
{
  IFile* f = (IFile*)file;
  int pos;
  switch (type) {
    case ADR_BEGIN:   pos = destination; break;
    case ADR_CURRENT: pos = f->Tell() + destination; break;
    case ADR_END:     pos = f->Size() + destination; break;
    default:          return 0;
  }
  f->Seek(pos);
  return ADR_TRUE;
}
#else
static int ADR_CALL FileSeek(ADR_FILE file, int destination) {
  IFile* f = (IFile*)file;
  f->Seek(destination);
  return 1;
}
#endif

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
