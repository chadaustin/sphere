#include <windows.h>
#include <stdio.h>
#include <fmod.h>
#include "../../common/types.h"
#include "../common/audio.hpp"
#include "resource.h"


enum SoundType { SOUND_STREAM, SOUND_MODULE };

struct SA_SOUNDimp {
  SoundType sound_type;
  long      channel;
  bool      is_playing;
  bool      repeat;

  FSOUND_STREAM* stream_loop;
  FSOUND_STREAM* stream_noloop;
  FMUSIC_MODULE* module;
};


struct CONFIGURATION {
  int output_driver;
  int mixer;
  int sample_rate;
  int bit_depth;
  int stereo;
};


static void LoadConfig();
static void SaveConfig();
static BOOL CALLBACK ConfigureDialogProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

static unsigned FMOD_OpenFile(const char* name);
static void     FMOD_CloseFile(unsigned handle);
static int      FMOD_ReadFile(void* buffer, int size, unsigned handle);
static int      FMOD_SeekFile(unsigned handle, int pos, signed char mode);
static int      FMOD_TellFile(unsigned handle);


// globals

static HWND  SphereWindow;
static dword BaseMode;
static bool  AudioEnabled;
static bool  UseFileCallback = false;
static CONFIGURATION config;


// private data and file callbacks

static void* s_data;
  
static SA_FileOpenCallback  s_open;
static SA_FileCloseCallback s_close;
static SA_FileReadCallback  s_read;
static SA_FileSeekCallback  s_seek;
static SA_FileTellCallback  s_tell;
static SA_FileSizeCallback  s_size;


////////////////////////////////////////////////////////////////////////////////

inline BOOL WritePrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, INT nInt, LPCTSTR lpFileName)
{
  char str[80];
  sprintf(str, "%d", nInt);
  return WritePrivateProfileString(lpAppName, lpKeyName, str, lpFileName);
}

////////////////////////////////////////////////////////////////////////////////

static void LoadConfig()
{ 
  char config_file_name[MAX_PATH];
  GetDriverConfigFile(config_file_name);

  config.output_driver = GetPrivateProfileInt("", "Output",     1,     config_file_name);
  config.mixer         = GetPrivateProfileInt("", "Mixer",      0,     config_file_name);
  config.sample_rate   = GetPrivateProfileInt("", "SampleRate", 44100, config_file_name);
  config.bit_depth     = GetPrivateProfileInt("", "BitDepth",   16,    config_file_name);
  config.stereo        = GetPrivateProfileInt("", "Stereo",     1,     config_file_name);
}

////////////////////////////////////////////////////////////////////////////////

static void SaveConfig()
{
  char config_file_name[MAX_PATH];
  GetDriverConfigFile(config_file_name);

  WritePrivateProfileInt("", "Output",     config.output_driver, config_file_name);
  WritePrivateProfileInt("", "Mixer",      config.mixer,         config_file_name);
  WritePrivateProfileInt("", "SampleRate", config.sample_rate,   config_file_name);
  WritePrivateProfileInt("", "BitDepth",   config.bit_depth,     config_file_name);
  WritePrivateProfileInt("", "Stereo",     config.stereo,        config_file_name);
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT SA_GetDriverInfo(DRIVERINFO* driverinfo)
{
  driverinfo->name        = "FMod Audio Driver";
  driverinfo->author      = "Jacky Chong (Darklich), Chad Austin";
  driverinfo->date        = __DATE__;
  driverinfo->version     = "0.92";
  driverinfo->description = "Audio interface from Sphere to FMod\nNote: uses a lot of resource!";
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT SA_ConfigureDriver(HWND parent)
{
  LoadConfig();
  DialogBox(DriverInstance, MAKEINTRESOURCE(IDD_CONFIGURE), parent, ConfigureDialogProc);
  SaveConfig();
}

////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK ConfigureDialogProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
  switch (message) {
    case WM_INITDIALOG: {
      // output driver
      SendDlgItemMessage(window, IDC_AUDIO_DRIVERCOMBO, CB_ADDSTRING, 0, (LPARAM)"No sound");
      SendDlgItemMessage(window, IDC_AUDIO_DRIVERCOMBO, CB_ADDSTRING, 0, (LPARAM)"Windows Multimedia");
      SendDlgItemMessage(window, IDC_AUDIO_DRIVERCOMBO, CB_ADDSTRING, 0, (LPARAM)"DirectSound");
      SendDlgItemMessage(window, IDC_AUDIO_DRIVERCOMBO, CB_SETCURSEL, config.output_driver, 0);

      // mixers
      SendDlgItemMessage(window, IDC_AUDIO_MIXERCOMBO, CB_ADDSTRING, 0, (LPARAM)"Autodetect");
      SendDlgItemMessage(window, IDC_AUDIO_MIXERCOMBO, CB_ADDSTRING, 0, (LPARAM)"Interpolation - Autodetect");
      SendDlgItemMessage(window, IDC_AUDIO_MIXERCOMBO, CB_ADDSTRING, 0, (LPARAM)"Standard");
      SendDlgItemMessage(window, IDC_AUDIO_MIXERCOMBO, CB_ADDSTRING, 0, (LPARAM)"Pentium MMX");
      SendDlgItemMessage(window, IDC_AUDIO_MIXERCOMBO, CB_ADDSTRING, 0, (LPARAM)"PPro/P5/P6 MMX");
      SendDlgItemMessage(window, IDC_AUDIO_MIXERCOMBO, CB_ADDSTRING, 0, (LPARAM)"Interpolation - FPU");
      SendDlgItemMessage(window, IDC_AUDIO_MIXERCOMBO, CB_ADDSTRING, 0, (LPARAM)"Interpolation - MMX P5");
      SendDlgItemMessage(window, IDC_AUDIO_MIXERCOMBO, CB_ADDSTRING, 0, (LPARAM)"Interpolation - MMX P6");
      SendDlgItemMessage(window, IDC_AUDIO_MIXERCOMBO, CB_SETCURSEL, config.mixer, 0);

      switch (config.sample_rate) {
        case 44100: {
          CheckDlgButton(window, IDC_AUDIO_44KHZ, BST_CHECKED);
          break;
        }

        case 22050: {
          CheckDlgButton(window, IDC_AUDIO_22KHZ, BST_CHECKED);
          break;
        }

        case 11025: {
          CheckDlgButton(window, IDC_AUDIO_11KHZ, BST_CHECKED);
          break;
        }

        default: { // 8000
          CheckDlgButton(window, IDC_AUDIO_8KHZ, BST_CHECKED);
          break;
        }
      }

      CheckDlgButton(window, config.stereo ? IDC_AUDIO_STEREO : IDC_AUDIO_MONO, BST_CHECKED);
      CheckDlgButton(window, config.bit_depth == 16 ? IDC_AUDIO_16BIT : IDC_AUDIO_8BIT, BST_CHECKED);
      return TRUE;
    }

    case WM_COMMAND: {
      switch (LOWORD(wparam)) {
        case IDOK: {
          if (IsDlgButtonChecked(window, IDC_AUDIO_16BIT) == BST_CHECKED) {
            config.bit_depth = 16;
          } else {
            config.bit_depth = 8;
          }

          if (IsDlgButtonChecked(window, IDC_AUDIO_44KHZ) == BST_CHECKED) {
            config.sample_rate = 44100;
          } else if (IsDlgButtonChecked(window, IDC_AUDIO_22KHZ) == BST_CHECKED) {
            config.sample_rate = 22050;
          } else if (IsDlgButtonChecked(window, IDC_AUDIO_11KHZ) == BST_CHECKED) {
            config.sample_rate = 11025;
          } else {
            config.sample_rate = 8000;
          }

          config.stereo = (IsDlgButtonChecked(window, IDC_AUDIO_STEREO) == BST_CHECKED);
          config.output_driver = SendDlgItemMessage(window, IDC_AUDIO_DRIVERCOMBO, CB_GETCURSEL, 0, 0);
          config.mixer         = SendDlgItemMessage(window, IDC_AUDIO_MIXERCOMBO,  CB_GETCURSEL, 0, 0);
                
          EndDialog(window, 0);
          return TRUE;
        }

        case IDCANCEL: {
          EndDialog(window, 0);
          return TRUE;
        }
      }
    }

    default: {
      return FALSE;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

bool EXPORT SA_InitDriver(HWND window)
{
  LoadConfig();

  SphereWindow = window;

  // set output type
  int output_device;
  switch (config.output_driver) {
    case 2:  output_device = FSOUND_OUTPUT_DSOUND;  break;
    case 1:  output_device = FSOUND_OUTPUT_WINMM;   break;
    default: output_device = FSOUND_OUTPUT_NOSOUND; break;
  }

retry:

  FSOUND_SetOutput(output_device);

  // set mixer
  int mixer = FSOUND_MIXER_AUTODETECT;
  switch (config.mixer) {
    case 0: mixer = FSOUND_MIXER_AUTODETECT;         break;
    case 1: mixer = FSOUND_MIXER_QUALITY_AUTODETECT; break;
    case 2: mixer = FSOUND_MIXER_BLENDMODE;          break;
    case 3: mixer = FSOUND_MIXER_MMXP5;              break;
    case 4: mixer = FSOUND_MIXER_MMXP6;              break;
    case 5: mixer = FSOUND_MIXER_QUALITY_FPU;        break;
    case 6: mixer = FSOUND_MIXER_QUALITY_MMXP5;      break;
    case 7: mixer = FSOUND_MIXER_QUALITY_MMXP6;      break;
  }
  FSOUND_SetMixer(mixer);

  // initialize audio
  if (!FSOUND_Init(config.sample_rate, 64, 0)) 
  {
    // try reinitialization with no sound
    if (output_device != FSOUND_OUTPUT_NOSOUND)
    {
      output_device = FSOUND_OUTPUT_NOSOUND;
      goto retry;  // I know...  :)
    }

    return false;
  }

  // calculate modes for MP3 streaming
  bool Stereo = config.stereo ? true : false;
  BaseMode = FSOUND_2D;
  BaseMode |= (config.stereo ? FSOUND_STEREO : FSOUND_MONO);
  BaseMode |= (config.sample_rate > 11025 ? FSOUND_16BITS : FSOUND_8BITS);

  AudioEnabled = (output_device != FSOUND_OUTPUT_NOSOUND ? true : false);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT SA_CloseDriver()
{
  FSOUND_Close();
}

////////////////////////////////////////////////////////////////////////////////

SA_SOUND EXPORT SA_LoadSound(const char* filename)
{
  SA_SOUND sound = new SA_SOUNDimp;
   sound->repeat = true;
   sound->is_playing = false;
   
  // If file is a midi
  if (stricmp(filename + strlen(filename) - 5, ".midi") == 0 ||
      stricmp(filename + strlen(filename) - 4, ".mid") == 0)
  {
    byte* memblock;
    long  size;

    if (!UseFileCallback)
    {
      FILE* file = fopen(filename, "rb");
      if (file == NULL) return NULL;
      fseek(file, 0, SEEK_END);
      size = ftell(file);
      memblock = new byte[size];
      fseek(file, 0, SEEK_SET);
      fread(memblock, 1, size, file);
      fclose(file);
    }
    else
    {
      void* file = s_open(filename);
      if (file == NULL) return NULL;
      size = s_size(file);
      memblock = new byte[size];
      s_read(file, size, memblock);
      s_close(file);
    }

    sound->module = FMUSIC_LoadSongMemory(memblock, size);
    delete[] memblock;
    if (sound->module == NULL)
      return NULL;

    sound->sound_type = SOUND_MODULE;
  }
  else if (stricmp(filename + strlen(filename) - 4, ".rmi") == 0 ||
      stricmp(filename + strlen(filename) - 4, ".sgt") == 0 ||
      stricmp(filename + strlen(filename) - 4, ".s3m") == 0 ||
      stricmp(filename + strlen(filename) - 4, ".mod") == 0 ||
      stricmp(filename + strlen(filename) - 3, ".it") == 0  ||
      stricmp(filename + strlen(filename) - 3, ".xm") == 0)
  {
    sound->module = FMUSIC_LoadSong((char*)filename);
    if (sound->module == NULL)
      return NULL;

    sound->sound_type = SOUND_MODULE;
  }
  else if (stricmp(filename + strlen(filename) - 4, ".mp3") == 0 ||
           stricmp(filename + strlen(filename) - 4, ".mp2") == 0 ||
           stricmp(filename + strlen(filename) - 4, ".wav") == 0 ||
           stricmp(filename + strlen(filename) - 4, ".raw") == 0 ||
           stricmp(filename + strlen(filename) - 4, ".ogg") == 0 ||
           stricmp(filename + strlen(filename) - 4, ".wma") == 0 ||
           stricmp(filename + strlen(filename) - 4, ".asf") == 0)
  {
    sound->stream_loop = FSOUND_Stream_OpenFile(filename, BaseMode | FSOUND_LOOP_NORMAL, 0);
    sound->stream_noloop = FSOUND_Stream_OpenFile(filename, BaseMode | FSOUND_LOOP_OFF, 0);
    if (sound->stream_loop == NULL || sound->stream_noloop == NULL)
      return NULL;

    sound->sound_type = SOUND_STREAM;
  }
  else
    return NULL;

  return sound;
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT SA_DestroySound(SA_SOUND sound)
{
  if (!AudioEnabled)
    return;

  //Sound_Stop(sound);
  switch (sound->sound_type)
  {
    case SOUND_MODULE:
      FMUSIC_FreeSong(sound->module);
      break;

    case SOUND_STREAM:
      FSOUND_Stream_Close(sound->stream_loop);
      FSOUND_Stream_Close(sound->stream_noloop);
      break;
  }

  delete sound;
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT SA_PlaySound(SA_SOUND sound, bool repeat)
{
  if (!AudioEnabled)
    return;

  if (sound->is_playing)
    SA_StopSound(sound);

  switch (sound->sound_type)
  {
    case SOUND_MODULE:
      FMUSIC_PlaySong(sound->module);
      break;

    case SOUND_STREAM:
      switch(repeat)
      {
        case false: sound->channel = FSOUND_Stream_Play(FSOUND_FREE, sound->stream_noloop); break;
        case true:  sound->channel = FSOUND_Stream_Play(FSOUND_FREE, sound->stream_loop);   break;
      }
      sound->repeat = repeat;
      break;
  }

  sound->is_playing = true;
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT SA_StopSound(SA_SOUND sound)
{
  if (!AudioEnabled)
    return;

  if (!sound->is_playing)
    return;

  switch (sound->sound_type)
  {
    case SOUND_MODULE:
      FMUSIC_StopSong(sound->module);
      break;

      
    case SOUND_STREAM:
      switch(sound->repeat)
      {
        case true:  FSOUND_Stream_Stop(sound->stream_loop);   break;
        case false: FSOUND_Stream_Stop(sound->stream_noloop); break;
      }
      break;
   }
    
  sound->is_playing = false;
}

////////////////////////////////////////////////////////////////////////////////

int EXPORT SA_GetVolume(SA_SOUND sound)
{
  int volume = 0;

  if (sound->sound_type == SOUND_MODULE) {
    volume = FMUSIC_GetMasterVolume(sound->module);
  } else if (sound->sound_type == SOUND_STREAM) {
    volume = FSOUND_GetVolume(sound->channel);
  }

  volume = (int)(volume / 2.55f);
  return volume;
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT SA_SetVolume(SA_SOUND sound, int volume)
{
  if (!AudioEnabled)
    return;

  if (!sound->is_playing)
    return;

  // using evil MS's max and min stuff
  float vol;
  vol = (float)__min(volume, 100);
  vol = __max(vol, 0);
  vol = vol * 2.55f;

  if (sound->sound_type == SOUND_MODULE)
    FMUSIC_SetMasterVolume(sound->module, (int)vol);
  else if (sound->sound_type == SOUND_STREAM)
    FSOUND_SetVolume(sound->channel, (int)vol);
}

////////////////////////////////////////////////////////////////////////////////

int EXPORT SA_GetLength(SA_SOUND sound)
{
  // measurement units in FMOD is not uniform (streams use bytes, modules 
  // use something else)... therefore not possible :(
  // aahh screw it :)

  if (!AudioEnabled || !sound->is_playing)
    return 1;

  switch (sound->sound_type)
  {
    case SOUND_MODULE:
      // this is the closest FMod can get with the pattern length stuff
      return FMUSIC_GetNumOrders(sound->module);
      break;

    case SOUND_STREAM:
      return FSOUND_Stream_GetLengthMs(sound->stream_loop);
      break;
  }

  // VC++ is stupid. All control paths are returned already!
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT SA_SetPosition(SA_SOUND sound, int position)
{
  if (!AudioEnabled)
    return;

  switch (sound->sound_type)
  {
    case SOUND_MODULE:
      // this is the closest FMod can get with the pattern length stuff
      FMUSIC_SetOrder(sound->module, position);
      break;

    case SOUND_STREAM:
      if (sound->repeat) 
        FSOUND_Stream_SetTime(sound->stream_loop, position);
      else 
        FSOUND_Stream_SetTime(sound->stream_noloop, position);
      break;
  }
}

////////////////////////////////////////////////////////////////////////////////

int EXPORT SA_GetPosition(SA_SOUND sound)
{
  // measurement units in FMOD is not uniform (streams use bytes, modules 
  // use something else)... therefore not possible :(
  // aah fudge it.

  if (!AudioEnabled || !sound->is_playing)
    return 1;

  switch (sound->sound_type)
  {
    case SOUND_MODULE:
      // this is the closest FMod can get with the pattern length stuff
      return FMUSIC_GetOrder(sound->module);
      break;

    case SOUND_STREAM:
      if (sound->repeat) 
        return FSOUND_Stream_GetTime(sound->stream_loop);
      else 
        return FSOUND_Stream_GetTime(sound->stream_noloop);
      break;
  }

  // VC++ is stupid. All control paths are returned already!
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

int EXPORT SA_IsPlaying(SA_SOUND sound)
{
  // not sure...
  if (!sound->is_playing)
    return 0;

  switch(sound->sound_type)
  {
    // stream has bug... keeps thinking the channel is still going even 
    // when finished... don't ask why. (FMod 3.2)
    case SOUND_STREAM: if (!FSOUND_IsPlaying(sound->channel)) return 0; break;
    case SOUND_MODULE: if (!FMUSIC_IsPlaying(sound->module)) return 0; break;
  }

  return 1;
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT SA_SetPrivate(void* data)
{
  s_data = data;
}

////////////////////////////////////////////////////////////////////////////////

void* EXPORT SA_GetPrivate()
{
  return s_data;
}

////////////////////////////////////////////////////////////////////////////////

void EXPORT SA_SetFileCallbacks(
  SA_FileOpenCallback  open,
  SA_FileCloseCallback close,
  SA_FileReadCallback  read,
  SA_FileSeekCallback  seek,
  SA_FileTellCallback  tell,
  SA_FileSizeCallback  size)
{
  if (open == NULL || close == NULL || read == NULL || seek == NULL || 
      tell == NULL || size == NULL)
  {
    FSOUND_File_SetCallbacks(NULL, NULL, NULL, NULL, NULL);

    UseFileCallback = false;
    return;
  }

  FSOUND_File_SetCallbacks(
    FMOD_OpenFile,
    FMOD_CloseFile,
    FMOD_ReadFile,
    FMOD_SeekFile,
    FMOD_TellFile);

  s_open  = open;
  s_close = close;
  s_read  = read;
  s_seek  = seek;
  s_tell  = tell;
  s_size  = size;

  UseFileCallback = true;
}

////////////////////////////////////////////////////////////////////////////////

unsigned FMOD_OpenFile(const char* name)
{
  return (unsigned)s_open(name);
}

////////////////////////////////////////////////////////////////////////////////

void FMOD_CloseFile(unsigned handle)
{
  s_close((void*)handle);
}

////////////////////////////////////////////////////////////////////////////////

int FMOD_ReadFile(void* buffer, int size, unsigned handle)
{
  return s_read((void*)handle, size, buffer);
}

////////////////////////////////////////////////////////////////////////////////

int FMOD_SeekFile(unsigned handle, int pos, signed char mode)
{
  int position = 0;
  switch (mode) {
    case SEEK_SET: position = pos;                         break;
    case SEEK_CUR: position = pos + s_tell((void*)handle); break;
    case SEEK_END: position = pos + s_size((void*)handle); break;
  }
  s_seek((void*)handle, position);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

int FMOD_TellFile(unsigned handle)
{
  return s_tell((void*)handle);
}

////////////////////////////////////////////////////////////////////////////////
