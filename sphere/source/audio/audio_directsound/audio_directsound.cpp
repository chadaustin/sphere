/*
  Sphere DirectSound driver

  Terminology

  block 
  - set of bytes for the smallest block of audio in a stream
    (including all of the streams)

  buffer
  - a circular audio buffer which is constantly refilled with
    new data from the stream source

  segment
  - buffers are split into a set of segments data flows from
    the stream source into the buffer a segment at a time

*/


#include "../common/audio.hpp"
#include <dsound.h>
#include <list>


/*** internal constants ***/

static const int VOLUME_MAX = 255;
static const int VOLUME_MIN = 0;

static const int PAN_LEFT = -255;
static const int PAN_RIGHT = 255;

static const int SEGMENT_COUNT  = 4;     // number of segments per buffer


/*** internal data types ***/

struct SA_STREAMimp {

  // output
  IDirectSoundBuffer* buffer;
  int next_read;  // offset (in samples) where we will read next

  int segment_length;  // in samples

  // input
  SA_SAMPLE_SOURCE source;
  void*            opaque;

  // convenience
  int block_length;  // bits per sample * channel count / 8

  // state
  int volume;  // 0 to 255
  int pan;     // -255 to 255

  BYTE* last_sample; // the last sample read (used for clickless silence)
};

typedef std::list<SA_STREAM> StreamList;


/*** internal functions ***/
static void FillStream(SA_STREAM stream);


/*** internal variables ***/
static IDirectSound8* s_DirectSound;
static StreamList     s_OpenStreams;


////////////////////////////////////////////////////////////////////////////////

void SA_CALL SA_GetDriverInfo(SA_DRIVERINFO* driverinfo)
{
  driverinfo->name        = "sphere_directsound";
  driverinfo->author      = "Chad Austin";
  driverinfo->date        = __DATE__;
  driverinfo->version     = "0.01";
  driverinfo->description = "DirectSound 8 Audio Driver";
}

////////////////////////////////////////////////////////////////////////////////

void SA_CALL SA_ConfigureDriver(HWND parent)
{
  MessageBox(parent, "Nothing yet!", "Coming soon", MB_OK);
}

////////////////////////////////////////////////////////////////////////////////

bool SA_CALL SA_OpenDriver(HWND window)
{
  // create the DirectSound device
  HRESULT result = DirectSoundCreate8(
    &DSDEVID_DefaultPlayback,
    &s_DirectSound,
    NULL
  );
  if (FAILED(result) || !s_DirectSound) {
    return false;
  }

  // set the cooperative level
  result = s_DirectSound->SetCooperativeLevel(window, DSSCL_NORMAL);
  if (FAILED(result)) {
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void SA_CALL SA_CloseDriver()
{
  // close any open streams
  StreamList::iterator i = s_OpenStreams.begin();
  while (i != s_OpenStreams.end()) {
    SA_CloseStream(*i);
    ++i;
  }
  s_OpenStreams.clear();

  // shut down DirectSound
  s_DirectSound->Release();
}

////////////////////////////////////////////////////////////////////////////////

// read as much as possible from the stream source, fill the rest with 0
inline void stream_read(SA_STREAM stream, int sample_count, void* samples)
{
  // try to read from the stream
  int samples_read = stream->source(
    stream->opaque,
    sample_count,
    samples
  );

  // read the last sample
  if (samples_read > 0) {
    int block_length = stream->block_length;
    memcpy(
      stream->last_sample,
      (BYTE*)samples + (samples_read - 1) * block_length,
      block_length
    );
  }

  // fill the rest with silence
  BYTE* out = (BYTE*)samples + stream->block_length * samples_read;
  int c = sample_count - samples_read;
  while (c--) {
    memcpy(out, stream->last_sample, stream->block_length);
    out += stream->block_length;
  }
}

////////////////////////////////////////////////////////////////////////////////

void SA_CALL SA_Update()
{
  // enumerate all open streams
  StreamList::iterator i = s_OpenStreams.begin();
  while (i != s_OpenStreams.end()) {
    SA_STREAM s = *i++;
    const int segment_length = s->segment_length;
    const int buffer_length  = s->segment_length * SEGMENT_COUNT;

    /* if a stream needs more data, read it */

    // read the stream's play and write cursors
    DWORD play;
    DWORD write;
    HRESULT result = s->buffer->GetCurrentPosition(&play, &write);
    if (FAILED(result)) {
      continue;
    }

    // find out what segments we can write to
    int start_segment = s->next_read           / segment_length;
    int stop_segment  = play / s->block_length / segment_length;

    // calculate how many segments we'll be locking
    // and update the input cursor
    int segment_count = 0;
    int next_read = s->next_read;
    while (start_segment != stop_segment) {
      segment_count++;
      next_read = (next_read + segment_length) % buffer_length;
      start_segment = (start_segment + 1) % SEGMENT_COUNT;
    }

    // lock the buffer
    void* buffer1 = NULL;
    void* buffer2 = NULL;
    DWORD buffer1_length = 0;
    DWORD buffer2_length = 0;
    result = s->buffer->Lock(
      s->next_read * s->block_length,
      segment_count * segment_length * s->block_length,
      &buffer1,
      &buffer1_length,
      &buffer2,
      &buffer2_length,
      0
    );
    if (FAILED(result) || (!buffer1 && !buffer2)) {
      continue;
    }

    // fill from the file
    int samples_to_read1 = buffer1_length / s->block_length;
    stream_read(s, samples_to_read1, buffer1);
    int samples_to_read2 = buffer2_length / s->block_length;
    stream_read(s, samples_to_read2, buffer2);

    // unlock
    s->buffer->Unlock(buffer1, buffer1_length, buffer2, buffer2_length);

    s->next_read = next_read;

  } // next stream
}


////////////////////////////////////////////////////////////////////////////////

SA_STREAM SA_CALL SA_OpenStream(
  int channel_count,
  int sample_rate,
  int bits_per_sample,
  SA_SAMPLE_SOURCE source,
  void* opaque)
{
  // create the internal stream object
  SA_STREAM stream = new SA_STREAMimp;
  stream->source = source;
  stream->opaque = opaque;
  stream->block_length = channel_count * bits_per_sample / 8;
  stream->next_read = 0;
  stream->last_sample = new BYTE[stream->block_length];
  memset(stream->last_sample, 0, stream->block_length);

  // calculate an ideal buffer size
  // a buffer is made of SEGMENT_COUNT segments
  // SA_Update should be called at least 20 times a second
  // therefore, the buffer needs to be at least 50 milliseconds
  // use 100 (it skips otherwise) for a little breathing room
  // XXX investigate skippage later...
  stream->segment_length = sample_rate * 100 / 1000 / SEGMENT_COUNT;

  // define the wave format
  WAVEFORMATEX wfx;
  memset(&wfx, 0, sizeof(wfx));
  wfx.wFormatTag      = WAVE_FORMAT_PCM;
  wfx.nChannels       = channel_count;
  wfx.nSamplesPerSec  = sample_rate;
  wfx.nAvgBytesPerSec = sample_rate * stream->block_length;
  wfx.nBlockAlign     = stream->block_length;
  wfx.wBitsPerSample  = bits_per_sample;
  wfx.cbSize          = sizeof(wfx);

  // define the DirectSound buffer type
  DSBUFFERDESC dsbd;
  memset(&dsbd, 0, sizeof(dsbd));
  dsbd.dwSize          = sizeof(dsbd);
  dsbd.dwFlags     = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS;
  dsbd.dwBufferBytes   = stream->segment_length *
                         stream->block_length *
                         SEGMENT_COUNT;
  dsbd.lpwfxFormat     = &wfx;
  dsbd.guid3DAlgorithm = GUID_NULL;

  // create the DirectSound buffer
  HRESULT result = s_DirectSound->CreateSoundBuffer(
    &dsbd, &stream->buffer, NULL
  );
  if (FAILED(result) || !stream) {
    delete[] stream->last_sample;
    delete stream;
    return NULL;
  }

  // fill the buffer with data
  FillStream(stream);

  // add ourselves to the list of streams and return
  s_OpenStreams.push_back(stream);
  return stream;
}

////////////////////////////////////////////////////////////////////////////////

void FillStream(SA_STREAM stream)
{
  // we know the stream is stopped, so just lock the buffer and fill it

  const int length = stream->segment_length * SEGMENT_COUNT;

  void* buffer = NULL;
  DWORD buffer_length = 0;

  // lock
  HRESULT result = stream->buffer->Lock(
    0,
    length * stream->block_length,
    &buffer,
    &buffer_length,
    NULL,
    NULL,
    0
  );
  if (FAILED(result) || !buffer) {
    return;
  }

  // fill
  int samples_to_read = buffer_length / stream->block_length;
  stream_read(stream, samples_to_read, buffer);

  // unlock
  stream->buffer->Unlock(buffer, buffer_length, NULL, 0);

  // set initial state
  SA_SetVolume(stream, 255);
  SA_SetPan(stream, 0);
}

////////////////////////////////////////////////////////////////////////////////

void SA_CALL SA_CloseStream(SA_STREAM stream)
{
  // remove ourself from the list
  StreamList::iterator i = s_OpenStreams.begin();
  while (i != s_OpenStreams.end()) {
    if (*i == stream) {
      s_OpenStreams.erase(i);
      break;
    }
    ++i;
  }

  // destroy the sound buffer interface
  stream->buffer->Release();
  delete[] stream->last_sample;
  delete stream;
}

////////////////////////////////////////////////////////////////////////////////

void SA_CALL SA_PlayStream(SA_STREAM stream)
{
  stream->buffer->Play(0, 0, DSBPLAY_LOOPING);
}

////////////////////////////////////////////////////////////////////////////////

void SA_CALL SA_StopStream(SA_STREAM stream)
{
  stream->buffer->Stop();
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
  stream->buffer->SetVolume(
    DSBVOLUME_MIN + (DSBVOLUME_MAX - DSBVOLUME_MIN) * volume / 255
  );
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
  stream->buffer->SetPan(
    pan * (DSBPAN_RIGHT - DSBPAN_LEFT) / 255
  );
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
