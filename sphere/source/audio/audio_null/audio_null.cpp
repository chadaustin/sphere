#include "../common/audio.hpp"


/*** internal constants ***/

static const int VOLUME_MAX = 255;
static const int VOLUME_MIN = 0;

static const int PAN_LEFT = -255;
static const int PAN_RIGHT = 255;



struct SA_STREAMimp {
  int pan;
  int volume;
};


////////////////////////////////////////////////////////////////////////////////

void SA_CALL SA_GetDriverInfo(SA_DRIVERINFO* driverinfo)
{
  driverinfo->name        = "Null Audio Driver";
  driverinfo->author      = "Chad Austin";
  driverinfo->date        = __DATE__;
  driverinfo->version     = "1.00";
  driverinfo->description = "Does absolutely nothing!";
}

////////////////////////////////////////////////////////////////////////////////

void SA_CALL SA_ConfigureDriver(HWND parent)
{
  MessageBox(parent, "No options", "Null Audio Driver", MB_OK);
}

////////////////////////////////////////////////////////////////////////////////

bool SA_CALL SA_OpenDriver(HWND window)
{
  return true;
}

////////////////////////////////////////////////////////////////////////////////

void SA_CALL SA_CloseDriver()
{
}

////////////////////////////////////////////////////////////////////////////////

void SA_CALL SA_Update()
{
}

////////////////////////////////////////////////////////////////////////////////

SA_STREAM SA_CALL SA_OpenStream(
  int channel_count,
  int sample_rate,
  int bits_per_sample,
  SAMPLE_READER source,
  void* opaque)
{
  SA_STREAM s = new SA_STREAMimp;
  s->pan    = 0;
  s->volume = 255;
  return s;
}

////////////////////////////////////////////////////////////////////////////////

void SA_CALL SA_CloseStream(SA_STREAM stream)
{
  delete stream;
}

////////////////////////////////////////////////////////////////////////////////

void SA_CALL SA_PlayStream(SA_STREAM stream)
{
}

////////////////////////////////////////////////////////////////////////////////

void SA_CALL SA_StopStream(SA_STREAM stream)
{
}

////////////////////////////////////////////////////////////////////////////////

void SA_CALL SA_SetVolume(SA_STREAM stream, int volume)
{
  if (volume < VOLUME_MIN) {
    volume = VOLUME_MIN;
  } else if (volume > VOLUME_MAX) {
    volume = VOLUME_MAX;
  }

  stream->volume = volume;
}

////////////////////////////////////////////////////////////////////////////////

void SA_CALL SA_SetPan(SA_STREAM stream, int pan)
{
  if (pan < PAN_LEFT) {
    pan = PAN_LEFT;
  } else if (pan > PAN_RIGHT) {
    pan = PAN_RIGHT;
  }

  stream->pan = pan;
}

////////////////////////////////////////////////////////////////////////////////

int SA_CALL SA_GetVolume(SA_STREAM stream)
{
  return stream->volume;
}

////////////////////////////////////////////////////////////////////////////////

int SA_CALL SA_GetPan(SA_STREAM stream)
{
  return stream->pan;
}

////////////////////////////////////////////////////////////////////////////////
