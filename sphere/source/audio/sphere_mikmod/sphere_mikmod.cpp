#include "../common/audio.hpp"
#include <process.h>
#include <mikmod.h>
#include <mplayer.h>
#include <mdsfx.h>

enum SoundType { SOUND_MIDI, SOUND_MODULE, SOUND_WAVE, SOUND_NOMP3 };

struct SA_SOUNDimp {
  SoundType  sound_type;
  bool       is_playing;
  int        channel;

  // mikmod stuff
  UNIMOD*    module;
  MPLAYER*   module_player;
  MD_SAMPLE* sample;
};

HWND          SphereWindow;
bool dead       = false;
bool diedamnyou = false;
MDRIVER*      Mikmod_device = NULL;
MD_VOICESET*  Mikmod_voiceset = NULL;

FILE* Mikmod_FileOpen(const char* filename);
int Mikmod_FileClose(FILE* filep);
int Mikmod_FileRead(void* buffer, size_t size, size_t count, FILE* filep);
int Mikmod_FileWrite(const void* buffer, size_t size, size_t count, FILE* filep);
int Mikmod_FileGetC(FILE* filep);
int Mikmod_FilePutC(int c, FILE* filep);
int Mikmod_FileSeek(FILE* filep, long offset, int mode);
int Mikmod_FileTell(FILE* filep);

bool using_callbacks = false;
static void* s_data;
static SA_FileOpenCallback  s_open;
static SA_FileCloseCallback s_close;
static SA_FileReadCallback  s_read;
static SA_FileSeekCallback  s_seek;
static SA_FileTellCallback  s_tell;
static SA_FileSizeCallback  s_size;

/////////////////////////////////////////////////////////////////////////////

DWORD WINAPI SpherePluginThread(LPVOID pvoid)
{
  // in a normal application Mikmod has to be called 12-13 times to have a 
  // normal playback or sound breaks up... increase the sleep time if buffers 
  // are really good and decrease the sleep time if sounds breaks up
  while (!diedamnyou)
  {
    Mikmod_Update(Mikmod_device);
    Sleep(20);
  }

  dead = true;
  return 0;
}

/////////////////////////////////////////////////////////////////////////////

void EXPORT SA_GetDriverInfo(DRIVERINFO* driverinfo) 
{
  driverinfo->name   = "Mikmod Audio plugin";
  driverinfo->author = "Jacky Chong (Darklich)";
  driverinfo->date   = __DATE__;
  driverinfo->version = "0.52";
  driverinfo->description = "Jake Stine's Mikmod support for Sphere! Thanks Air!\nNote: No mp3 support!";
}

/////////////////////////////////////////////////////////////////////////////

void EXPORT SA_ConfigureDriver(HWND parent) 
{
  MessageBox(NULL, "I'm working on it! - DL", "Error!", MB_OK | MB_ICONERROR);
}

/////////////////////////////////////////////////////////////////////////////

bool EXPORT SA_InitDriver(HWND window) 
{ 
  SphereWindow = window;

  // register loaders
	Mikmod_RegisterLoader(load_it);
	Mikmod_RegisterLoader(load_xm);
	Mikmod_RegisterLoader(load_s3m);
	Mikmod_RegisterLoader(load_mod);
	Mikmod_RegisterLoader(load_stm);
	Mikmod_RegisterLoader(load_m15);

  // register appropriate driver
  Mikmod_RegisterDriver(drv_ds);

  // 
  int sample_rate = 44100;
  int ms_latency = 100;
  int mode = DMODE_16BITS | DMODE_INTERP | DMODE_NOCLICK;
  Mikmod_device = Mikmod_Init(sample_rate, ms_latency, NULL, MD_STEREO, 
    CPU_AUTODETECT, DMODE_16BITS | DMODE_INTERP | DMODE_NOCLICK);
  if (Mikmod_device == NULL)
    return false;

  // create global control voiceset
  Mikmod_voiceset = Voiceset_Create(Mikmod_device, NULL, 64, MDVS_DYNAMIC);
  if (Mikmod_voiceset == NULL)
  {
    Mikmod_Exit(Mikmod_device);
    return false;
  }

  DWORD threadID, some_param = 1;
  HANDLE stupidHandle;
  stupidHandle = CreateThread(NULL, 0, SpherePluginThread, &some_param, 0, &threadID);
  CloseHandle(stupidHandle);

  return true;
}

/////////////////////////////////////////////////////////////////////////////

void EXPORT SA_CloseDriver(void) 
{
  diedamnyou = true;
  while(!dead)
  {
    Sleep(100);
  }
  Voiceset_Free(Mikmod_voiceset); // apparently Mikmod does it for you, aaah well...
  Mikmod_Exit(Mikmod_device);
}

/////////////////////////////////////////////////////////////////////////////

void EXPORT SA_SetPrivate(void* data) 
{
  s_data = data;
}

/////////////////////////////////////////////////////////////////////////////

void* EXPORT SA_GetPrivate(void) 
{ 
  return s_data; 
}

/////////////////////////////////////////////////////////////////////////////

void EXPORT SA_SetFileCallbacks(
  SA_FileOpenCallback  open,
  SA_FileCloseCallback close,
  SA_FileReadCallback  read,
  SA_FileSeekCallback  seek,
  SA_FileTellCallback  tell,
  SA_FileSizeCallback  size)
{
  s_open  = open;
  s_close = close;
  s_read  = read;
  s_seek  = seek;
  s_tell  = tell;
  s_size  = size;

  if (open == NULL || close == NULL || read == NULL || seek == NULL ||
      tell == NULL || size == NULL)
    using_callbacks = false;
  else
    using_callbacks = true;
}

/////////////////////////////////////////////////////////////////////////////

SA_SOUND EXPORT SA_LoadSound(const char* filename) { 
  SA_SOUND sound = new SA_SOUNDimp;
  sound->is_playing = false;

  if (stricmp(filename + strlen(filename) - 4, ".mid") == 0 ||
      stricmp(filename + strlen(filename) - 5, ".midi") == 0)
  {
    char temp[520];
    char ret[520];

    sprintf(temp, "open \"%s\" type sequencer alias midifile", filename);
    if (mciSendString(temp, ret, 512, NULL) != 0) // if it failed
      return NULL;

    sound->sound_type = SOUND_MIDI;
  }
  else if (stricmp(filename + strlen(filename) - 4, ".s3m") == 0 ||
           stricmp(filename + strlen(filename) - 4, ".mod") == 0 ||
           stricmp(filename + strlen(filename) - 4, ".stm") == 0 || // xtra format support
           stricmp(filename + strlen(filename) - 4, ".m15") == 0 || // xtra format support
           stricmp(filename + strlen(filename) - 3, ".it") == 0  ||
           stricmp(filename + strlen(filename) - 3, ".xm") == 0)
  {
    if (using_callbacks)
    {
      // build a file stream
      FILE* fp = Mikmod_FileOpen(filename);
      if (fp == NULL)
        return NULL;
      MMSTREAM* filestream = _mmstream_createfp(fp, 0);

      _mmstream_setapi(filestream, 
        Mikmod_FileRead,
        Mikmod_FileWrite,
        Mikmod_FileGetC,
        Mikmod_FilePutC,
        Mikmod_FileSeek,
        Mikmod_FileTell);

      sound->module = Unimod_LoadFP(Mikmod_device, filestream, filestream, MM_STATIC);
      SL_LoadSamples(Mikmod_device);

      // free the stream
      Mikmod_FileClose(filestream->fp);
      _mmstream_delete(filestream);
    }
    else
      sound->module = Unimod_Load(Mikmod_device, filename);
    
    if (sound->module == NULL)
      return NULL;

    sound->sound_type = SOUND_MODULE;
  }
  else if (stricmp(filename + strlen(filename) - 4, ".wav") == 0)
  {
    if (using_callbacks)
    {
      // build a file stream
      FILE* fp = Mikmod_FileOpen(filename);
      if (fp == NULL)
        return NULL;
      MMSTREAM* filestream = _mmstream_createfp(fp, 0);

      _mmstream_setapi(filestream, 
        Mikmod_FileRead,
        Mikmod_FileWrite,
        Mikmod_FileGetC,
        Mikmod_FilePutC,
        Mikmod_FileSeek,
        Mikmod_FileTell);

      sound->sample = mdsfx_loadwavfp(Mikmod_device, filestream);
      SL_LoadSamples(Mikmod_device);

      // free the stream
      Mikmod_FileClose(filestream->fp);
      _mmstream_delete(filestream);
    }
    else
      sound->sample = mdsfx_loadwav(Mikmod_device, filename);

    if (sound->sample == NULL)
      return NULL;
      
    sound->sound_type = SOUND_WAVE;
  }
  else // mp3
  {
    // don't do anything but accept it for now
    sound->sound_type = SOUND_NOMP3;
  }

  return sound; 
}

/////////////////////////////////////////////////////////////////////////////

void EXPORT SA_DestroySound(SA_SOUND sound) 
{
  switch(sound->sound_type)
  {
    case SOUND_MIDI:
      char ret[520];
      mciSendString("close midifile", ret, 512, NULL);
      break;

    case SOUND_MODULE:
      if (sound->is_playing)
        SA_StopSound(sound);

      Unimod_Free(sound->module);
      break;

    case SOUND_WAVE:
      if (sound->is_playing)
        SA_StopSound(sound);

      mdsfx_free(sound->sample);
      break;

    case SOUND_NOMP3:
      break;
  }

  delete sound;
}

/////////////////////////////////////////////////////////////////////////////

void EXPORT SA_PlaySound(SA_SOUND sound, bool repeat) 
{
  switch(sound->sound_type)
  {
    case SOUND_MIDI:
      char ret[512];
      mciSendString("seek midifile to start", ret, 500, SphereWindow);
      mciSendString("play midifile notify", ret, 500, SphereWindow);
      break;

    case SOUND_MODULE:
      {
        if (sound->is_playing)
          SA_StopSound(sound);

        UINT repeatFlag = (repeat ? PF_LOOP : 0);
        sound->module_player = Player_InitSong(sound->module, NULL, repeatFlag, 64);
        Player_Start(sound->module_player);
        sound->is_playing = true;
      }
      break;

    case SOUND_WAVE:
      if (sound->is_playing)
        SA_StopSound(sound);
      
      if (repeat)
      {
        sound->sample->flags |= SL_LOOP;
        sound->sample->reppos = 0;
        sound->sample->repend = sound->sample->length - 1;
      }
      else
        sound->sample->flags &= ~SL_LOOP;

      sound->channel = mdsfx_playeffect(sound->sample, Mikmod_voiceset, SF_START_BEGIN, 0);
      sound->is_playing = true;
      break;

    case SOUND_NOMP3:
      break;
  }
}

/////////////////////////////////////////////////////////////////////////////

void EXPORT SA_StopSound(SA_SOUND sound) 
{
  switch(sound->sound_type)
  {
    case SOUND_MIDI:
      char ret[512];
      mciSendString("stop midifile", ret, 500, NULL);
      mciSendString("seek midifile to start", ret, 500, NULL);
      break;

    case SOUND_MODULE:
      if (sound->is_playing)
      {
        Player_Stop(sound->module_player);
        Player_FreeSong(sound->module_player);
        sound->is_playing = false;
      }
      break;

    case SOUND_WAVE:
      if (sound->is_playing)
      {
        if (!Voice_Stopped(Mikmod_voiceset, sound->channel))
          Voice_Stop(Mikmod_voiceset, sound->channel);
        sound->is_playing = false;
      }
      break;

    case SOUND_NOMP3:
      break;
  }
}

/////////////////////////////////////////////////////////////////////////////

bool EXPORT SA_IsPlaying(SA_SOUND sound)
{
  return false;
}

/////////////////////////////////////////////////////////////////////////////

void EXPORT SA_SetVolume(SA_SOUND sound, int volume) 
{
  float vol;

  // contrain values to 0%-100% and translate to Mikmod's volume range
  // and yeah... me using evil windoze min and max...
  vol = __min(volume, 255);
  vol = __max(vol, 0);
  vol = (vol / 100.00f * 2.56f);

  switch (sound->sound_type)
  {
    case SOUND_MIDI:
      break;

    case SOUND_MODULE:
      Player_SetVolume(sound->module_player, volume);
      break;

    case SOUND_WAVE:
      Voice_SetVolume(Mikmod_voiceset, sound->channel, volume);
      break;

    case SOUND_NOMP3:
      break;
  }
}

/////////////////////////////////////////////////////////////////////////////

int EXPORT SA_GetVolume(SA_SOUND sound)
{ 
  int volume;

  switch (sound->sound_type)
  {
    case SOUND_MIDI:
      volume = 0;
      break;

    case SOUND_MODULE:
      // note: dunno if this is done right... addressing internal structs directly
      volume = sound->module_player->vs->volume;
      volume = (volume / 2.56f * 2.55f);
      break;

    case SOUND_WAVE:
      // note: dunno if this is done right... addressing internal structs directly
      volume = Mikmod_voiceset->vdesc[sound->channel].volume;
      volume = (volume / 2.56f * 2.55f);
      break;

    case SOUND_NOMP3:
      volume = 0;
      break;
  }

  return volume; 
}

/////////////////////////////////////////////////////////////////////////////

int EXPORT SA_GetLength(SA_SOUND sound) 
{
  switch (sound->sound_type)
  {
    case SOUND_MIDI:
      break;

    case SOUND_MODULE:
      break;

    case SOUND_WAVE:
      break;

    case SOUND_NOMP3:
      break;
  }

  return 1; 
}

/////////////////////////////////////////////////////////////////////////////

void EXPORT SA_SetPosition(SA_SOUND sound, int position) 
{
  switch (sound->sound_type)
  {
    case SOUND_MIDI:
      break;

    case SOUND_MODULE:
      break;

    case SOUND_WAVE:
      break;

    case SOUND_NOMP3:
      break;
  }
}

/////////////////////////////////////////////////////////////////////////////

int EXPORT SA_GetPosition(SA_SOUND sound) 
{ 
  switch (sound->sound_type)
  {
    case SOUND_MIDI:
      break;

    case SOUND_MODULE:
      break;

    case SOUND_WAVE:
      break;

    case SOUND_NOMP3:
      break;
  }

  return 0;
}

/////////////////////////////////////////////////////////////////////////////

FILE* Mikmod_FileOpen(const char* filename)
{
  return (FILE*)s_open(filename);
}

/////////////////////////////////////////////////////////////////////////////

int Mikmod_FileClose(FILE* filep)
{
  s_close((void*)filep);
  return 0;
}

/////////////////////////////////////////////////////////////////////////////

int Mikmod_FileRead(void* buffer, size_t size, size_t count, FILE* filep)
{
  return s_read((void*)filep, size*count, buffer);
}

/////////////////////////////////////////////////////////////////////////////

int Mikmod_FileWrite(const void* buffer, size_t size, size_t count, FILE* filep)
{
  // sphere and this plugin has no write capabilities!
  return size*count;
}

/////////////////////////////////////////////////////////////////////////////

int Mikmod_FileGetC(FILE* filep)
{
  char c;
  s_read((void*)filep, 1, &c);
  return c;
}

/////////////////////////////////////////////////////////////////////////////

int Mikmod_FilePutC(int c, FILE* filep)
{
  // sphere and this plugin has no write capabilities!
  return c;
}

/////////////////////////////////////////////////////////////////////////////

int Mikmod_FileSeek(FILE* filep, long offset, int mode)
{
  int new_pos = 0;
  switch (mode)
  {
    case SEEK_SET: new_pos = offset;                        break;
    case SEEK_CUR: new_pos = offset + s_tell((void*)filep); break;
    case SEEK_END: new_pos = offset + s_size((void*)filep); break;
  }

  s_seek((void*)filep, new_pos);
  return 0;
}

/////////////////////////////////////////////////////////////////////////////

int Mikmod_FileTell(FILE* filep)
{
  return s_tell((void*)filep);
}
