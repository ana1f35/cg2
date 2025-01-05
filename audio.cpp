#include "headers/audio.h"
#include <iostream>
#include <vector>
#include "include/dr_wav.h"

/**
 * @brief Carrega um ficheiro de áudio em um buffer OpenAL.
 *
 * Esta função usa a biblioteca dr_wav para ler um ficheiro de áudio WAV e carrega os dados de áudio para um buffer OpenAL.
 *
 * @param fileName O caminho para o ficheiro de áudio a ser carregado.
 * @param buffer Uma referência para um buffer OpenAL onde os dados de áudio serão armazenados.
 * @return true se o ficheiro de áudio foi carregado com sucesso, false caso contrário.
 */
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

/**
 * @brief Inicializa o sistema de som OpenAL e carrega ficheiros de áudio a partir de buffers.
 *
 * Esta função abre o dispositivo de áudio padrão, cria um contexto OpenAL e carrega
 * múltiplos arquivos de áudio em buffers separados. Ela também gera fontes e anexa
 * os buffers a essas fontes, definindo seus níveis de volume iniciais.
 *
 * @param device Uma referência para um ponteiro para o dispositivo OpenAL.
 * @param context Uma referência para um ponteiro para o contexto OpenAL.
 * @param buffer Uma referência para o primeiro buffer OpenAL.
 * @param buffer2 Uma referência para o segundo buffer OpenAL.
 * @param buffer3 Uma referência para o terceiro buffer OpenAL.
 * @param buffer4 Uma referência para o quarto buffer OpenAL.
 * @param source Uma referência para a primeira fonte OpenAL.
 * @param source2 Uma referência para a segunda fonte OpenAL.
 * @param source3 Uma referência para a terceira fonte OpenAL.W
 * @param source4 Uma referência para a quarta fonte OpenAL.
 * @return int Retorna 0 em caso de sucesso ou -1 em caso de falha.
 */
int inicializarSound(ALCdevice*& device, ALCcontext*& context, 
    ALuint& buffer, ALuint& buffer2, ALuint& buffer3, ALuint& buffer4, ALuint& source, ALuint& source2, ALuint& source3, ALuint& source4) {

    device = alcOpenDevice(NULL);
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
    if (!loadAudio("sound/speedUp.wav", buffer3)) {
        alcMakeContextCurrent(NULL);
        alcDestroyContext(context);
        alcCloseDevice(device);
        return -1;
    }
    if (!loadAudio("sound/explosion.wav", buffer4)) {
        alcMakeContextCurrent(NULL);
        alcDestroyContext(context);
        alcCloseDevice(device);
        return -1;
    }

    alGenSources(1, &source);
    alSourcei(source, AL_BUFFER, buffer);
    alGenSources(1, &source2);
    alSourcei(source2, AL_BUFFER, buffer2);
    alGenSources(1, &source3);
    alSourcei(source3, AL_BUFFER, buffer3);
    alGenSources(1, &source4);
    alSourcei(source4, AL_BUFFER, buffer4);

    alSourcef(source, AL_GAIN, 0.3f); 
    alSourcef(source2, AL_GAIN, 0.6f); 
    alSourcef(source3, AL_GAIN, 0.6f); 
    alSourcef(source4, AL_GAIN, 0.6f); 

    return 0;
}

