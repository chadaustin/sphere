#include <windows.h>
#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <conio.h>  // yeah, this is evil, but getch() is nicer than getchar()
#include "../../../../acoustique/acoustique.h"


#define SA_CALL __stdcall

typedef int (SA_CALL *SAMPLE_READER)(
  void* opaque,
  int sample_count,
  void* samples);

typedef void* SA_STREAM;

struct SA_DRIVERINFO {
  const char* name;
  const char* author;
  const char* date;
  const char* version;
  const char* description;
};


HINSTANCE driver;
HWND      window;

void      (SA_CALL *SA_GetDriverInfo)(SA_DRIVERINFO* driverinfo);
void      (SA_CALL *SA_ConfigureDriver)(HWND parent);
bool      (SA_CALL *SA_OpenDriver)(HWND window);
void      (SA_CALL *SA_CloseDriver)(void);
void      (SA_CALL *SA_Update)(void);
SA_STREAM (SA_CALL *SA_OpenStream)(int channel_count, int sample_rate, int bits_per_sample, SAMPLE_READER source, void* opaque);
void      (SA_CALL *SA_CloseStream)(SA_STREAM stream);
void      (SA_CALL *SA_PlayStream)(SA_STREAM stream);
void      (SA_CALL *SA_StopStream)(SA_STREAM stream);
void      (SA_CALL *SA_SetVolume)(SA_STREAM stream, int volume);
void      (SA_CALL *SA_SetPan)(SA_STREAM stream, int pan);
int       (SA_CALL *SA_GetVolume)(SA_STREAM stream);
int       (SA_CALL *SA_GetPan)(SA_STREAM stream);


template<typename S, typename T>
void assign(S& dest, T source)
{
  dest = (S)source;
}


LRESULT CALLBACK WindowHandler(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
//  switch (message) {
//    default: {
      return DefWindowProc(window, message, wparam, lparam);
//    }
//  }
}


bool LoadDriver(const char* filename)
{
  // open the DLL
  driver = LoadLibrary(filename);
  if (driver == NULL) {
    return false;
  }

  // get the exported symbols
#define get(name)                                          \
  assign(SA_##name, GetProcAddress(driver, "SA_" #name));  \
  if (SA_##name == NULL) {                                 \
    FreeLibrary(driver);                                   \
    return false;                                          \
  }

  get(GetDriverInfo);
  get(ConfigureDriver);
  get(OpenDriver);
  get(CloseDriver);
  get(Update);
  get(OpenStream);
  get(CloseStream);
  get(PlayStream);
  get(StopStream);
  get(SetVolume);
  get(SetPan);
  get(GetVolume);
  get(GetPan);

  // display driver information
  SA_DRIVERINFO driverinfo;
  SA_GetDriverInfo(&driverinfo);
  printf(
    "name:        %s\n"
    "author:      %s\n"
    "date:        %s\n"
    "version:     %s\n"
    "description: %s\n"
    "\n",
    driverinfo.name,
    driverinfo.author,
    driverinfo.date,
    driverinfo.version,
    driverinfo.description
  );

  // register window class
  WNDCLASS wc;
  memset(&wc, 0, sizeof(wc));
  wc.lpfnWndProc = WindowHandler;
  wc.hInstance = GetModuleHandle(NULL);
  wc.lpszClassName = "audio_test_window";
  if (RegisterClass(&wc) == 0) {
    puts("Window class registration failed");
    FreeLibrary(driver);
    return false;
  }

  // create window, but don't show it  
  window = CreateWindow(
    "audio_test_window", "audio_test_window",
    WS_POPUP, 0, 0, 0, 0,
    NULL, NULL, GetModuleHandle(NULL), NULL
  );
  if (window == NULL) {
    puts("Window creation failed");
    FreeLibrary(driver);
    return false;
  }

  // initialize driver
  if (!SA_OpenDriver(window)) {
    puts("SA_OpenDriver() failed");
    FreeLibrary(driver);
    return false;
  }

  return true;
}


void FreeDriver()
{
  SA_CloseDriver();
  DestroyWindow(window);
  FreeLibrary(driver);
}


int file_read(void* opaque, void* bytes, int byte_count)
{
  return fread(bytes, 1, byte_count, (FILE*)opaque);
}


void file_reset(void* opaque)
{
  fseek((FILE*)opaque, 0, SEEK_SET);
}


int SA_CALL Source(void* opaque, int sample_count, void* samples)
{
  ACQ_STREAM* stream = (ACQ_STREAM*)opaque;
  return AcqReadStream(stream, samples, sample_count);
}


int main(int argc, char** argv)
{
  if (argc != 2) {
    puts("usage: audio_test <wav_file>");
    return 1;
  }

  const char* filename = argv[1];

  // open audio driver
  if (!LoadDriver("audio_directsound.dll")) {
    puts("Could not load audio driver");
    return 1;
  }

  puts(AcqGetVersion());

  // controls
  puts(
    "\n"
    "controls:\n"
    "\n"
    "r  reset\n"
    "q  quit\n"
    "p  pause\n"
    ">  pan right\n"
    "<  pan left\n"
    "+  volume up\n"
    "-  volume down\n"
  );

  // open input stream
  FILE* file = fopen(filename, "rb");
  if (file == NULL) {
    puts("Could not open input stream");
    return 1;
  }

  // open input decoder stream
  ACQ_STREAM decoder = AcqOpenStream(
    file,
    file_read,
    file_reset,
    ACQ_STREAM_AUTODETECT
  );
  if (decoder == NULL) {
    fclose(file);
    puts("Could not open decoder stream");
    return 1;
  }

  // get input stream parameters
  int channel_count;
  int bits_per_sample;
  int sample_rate;
  AcqGetStreamInformation(
    decoder,
    &channel_count,
    &bits_per_sample,
    &sample_rate
  );

  printf(
    "Stream information:\n"
    "  channel_count: %d\n"
    "  bits per sample: %d\n"
    "  sample rate: %d\n"
    "\n",
    channel_count,
    bits_per_sample,
    sample_rate
  );

  // open output stream
  SA_STREAM output = SA_OpenStream(
    channel_count,
    sample_rate,
    bits_per_sample,
    Source,
    decoder
  );
  if (output == NULL) {
    AcqCloseStream(decoder);
    fclose(file);
    FreeDriver();
    puts("could not open output stream");
    return 0;
  }
  SA_PlayStream(output);
  bool playing = true;


  // main loop
  bool done = false;
  while (!done) {

    int pan    = SA_GetPan(output);
    int volume = SA_GetVolume(output);

    cprintf("status: pan (%d)  volume (%d)           \r", pan, volume);

    while (kbhit()) {
      switch (getch()) {

        // quit
        case 'q': {
          done = true;
        } break;

        case 'r': {
          AcqResetStream(decoder);
        } break;

        case 'p': {
          if (playing) {
            SA_StopStream(output);
          } else {
            SA_PlayStream(output);
          }
          playing = !playing;
        } break;

        // pan right
        case '>': {
          SA_SetPan(output, pan + 1);
        } break;

        // pan left
        case '<': {
          SA_SetPan(output, pan - 1);
        } break;

        // volume up
        case '+': {
          SA_SetVolume(output, volume + 1);
        } break;

        // volume down
        case '-': {
          SA_SetVolume(output, volume - 1);
        } break;

      }
    }

    // call update 20 times a second (twice as fast as the spec says we need to)
    Sleep(10);
    SA_Update();
  }

  putchar('\n');

  // close output stream
  SA_CloseStream(output);

  // close decoder stream
  AcqCloseStream(decoder);

  // close input stream
  fclose(file);

  // close audio driver
  FreeDriver();
  return 0;
}
