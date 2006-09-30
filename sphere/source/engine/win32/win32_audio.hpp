#ifndef WIN32_AUDIO_HPP
#define WIN32_AUDIO_HPP
#include <audiere.h>
audiere::OutputStream* SA_OpenSound(audiere::File* file, bool streaming);
#if defined(WIN32) && defined(USE_MIDI)
//audiere::MIDIStream* SA_OpenMIDI(audiere::File* file);
audiere::MIDIStream* SA_OpenMIDI(const char* filename);
#endif
#endif
