#ifndef AUDIO_HPP
#define AUDIO_HPP


#include <windows.h>


struct SA_DRIVERINFO
{
  const char* name;
  const char* author;
  const char* date;
  const char* version;
  const char* description;
};


extern HINSTANCE DriverInstance;


void GetDriverConfigFile(char* config_file);


// exports

#define SA_CALL __stdcall
typedef int (SA_CALL *SA_SAMPLE_SOURCE)(
  void* opaque,
  int sample_count,
  void* samples);


typedef struct SA_STREAMimp* SA_STREAM;


void SA_CALL SA_GetDriverInfo(SA_DRIVERINFO* driverinfo);
void SA_CALL SA_ConfigureDriver(HWND parent);

bool SA_CALL SA_OpenDriver(HWND window);
void SA_CALL SA_CloseDriver(void);

SA_STREAM SA_CALL SA_OpenStream(
  int channel_count,
  int sample_rate,
  int bits_per_sample,
  SA_SAMPLE_SOURCE source,
  void* opaque);
void      SA_CALL SA_CloseStream(SA_STREAM stream);
void      SA_CALL SA_WriteSamples(SA_STREAM stream, bool repeat);
void      SA_CALL SA_SetVolume(SA_STREAM stream, int volume);
void      SA_CALL SA_SetPan(SA_STREAM stream, int pan);
int       SA_CALL SA_GetVolume(SA_STREAM stream);
int       SA_CALL SA_GetPan(SA_STREAM stream);

// end exports


#endif
