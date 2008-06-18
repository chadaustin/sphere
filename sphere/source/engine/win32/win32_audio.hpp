#ifndef WIN32_AUDIO_HPP
#define WIN32_AUDIO_HPP

#include <audiere.h>

audiere::OutputStream* SA_OpenSound(audiere::File* file, bool streaming);
audiere::SoundEffect*  SA_OpenSoundEffect(audiere::File* file, audiere::SoundEffectType type);

#if defined(WIN32) && defined(USE_MIDI)
audiere::MIDIStream* SA_OpenMIDI(const char* filename);
#endif


#endif
