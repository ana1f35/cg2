#define DR_WAV_IMPLEMENTATION
#include "include/dr_wav.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <iostream>
#include <vector>
#include <thread>

bool loadAudio(const std::string& fileName, ALuint& buffer) {
    drwav wav;
    if (!drwav_init_file(&wav, fileName.c_str(), NULL)) {
        std::cerr << "Failed to load audio file: " << fileName << std::endl;
        return false;
    }

    std::vector<char> data(wav.totalPCMFrameCount * wav.channels * sizeof(int16_t));
    drwav_read_pcm_frames_s16(&wav, wav.totalPCMFrameCount, reinterpret_cast<int16_t*>(data.data()));
    drwav_uninit(&wav);

    alGenBuffers(1, &buffer);
    alBufferData(buffer, wav.channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
                 data.data(), data.size(), wav.sampleRate);

    return true;
}

void playAudio(ALuint buffer) {
    std::thread([buffer]() {
        ALuint source;
        alGenSources(1, &source);
        alSourcei(source, AL_BUFFER, buffer); // Attach buffer to the source

        alSourcePlay(source); // Play the source

        // Wait for the audio to finish playing
        ALint state;
        do {
            alGetSourcei(source, AL_SOURCE_STATE, &state);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } while (state == AL_PLAYING);

        // Clean up the source
        alDeleteSources(1, &source);
    }).detach();
}

int inicializarSound(ALCdevice*& device, ALCcontext*& context, 
    ALuint& buffer, ALuint& buffer2, ALuint& buffer3, ALuint& source, ALuint& source2, ALuint& source3) {
    // Initialize OpenAL
    device = alcOpenDevice(NULL); // Open default device
    if (!device) {
        std::cerr << "Failed to open default audio device" << std::endl;
        return -1;
    }

    context = alcCreateContext(device, NULL);
    if (!alcMakeContextCurrent(context)) {
        std::cerr << "Failed to set OpenAL context" << std::endl;
        alcCloseDevice(device);
        return -1;
    }

    // Load audio into buffer
    if (!loadAudio("sound/intro.wav", buffer)) {
        alcMakeContextCurrent(NULL);
        alcDestroyContext(context);
        alcCloseDevice(device);
        return -1;
    }
    if (!loadAudio("sound/tie_fighter.wav", buffer2)) {
        alcMakeContextCurrent(NULL);
        alcDestroyContext(context);
        alcCloseDevice(device);
        return -1;
    }
    if (!loadAudio("sound/tie-fighter-roar.wav", buffer3)) {
        alcMakeContextCurrent(NULL);
        alcDestroyContext(context);
        alcCloseDevice(device);
        return -1;
    }


    // Generate a source and attach the buffer to it
    alGenSources(1, &source);
    alSourcei(source, AL_BUFFER, buffer);

    alGenSources(1, &source2);
    alSourcei(source2, AL_BUFFER, buffer2);

    alGenSources(1, &source3);
    alSourcei(source3, AL_BUFFER, buffer3);

    return 0;
}

