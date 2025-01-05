#ifndef AUDIO_H
#define AUDIO_H

#include <AL/al.h>
#include <AL/alc.h>
#include <string>
#include <thread>

void playAudio(ALuint buffer);
bool loadAudio(const std::string& fileName, ALuint& buffer);
int inicializarSound(ALCdevice*& device, ALCcontext*& context, 
    ALuint& buffer, ALuint& buffer2, ALuint& buffer3, ALuint& buffer4, ALuint& source, ALuint& source2, ALuint& source3, ALuint& source4);

#endif