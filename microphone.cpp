#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "soloud_microphone.h"
#include "SDL.h"
#include "../wav/dr_wav.h"

namespace SoLoud
{

    //Maximum number of supported recording devices
    const int MAX_RECORDING_DEVICES = 10;

    //Maximum recording time
    const int MAX_RECORDING_SECONDS = 5;

    //Maximum recording time plus padding
    const int RECORDING_BUFFER_SECONDS = MAX_RECORDING_SECONDS + 1;

    //The various recording actions we can take
    enum RecordingState
    {
        SELECTING_DEVICE,
        STOPPED,
        RECORDING,
        RECORDED,
        PLAYBACK,
        ERROR
    };

    static SDL_AudioSpec gActiveAudioSpec;
    static SDL_AudioDeviceID gAudioDeviceID;
    bool isRecording = false;
    Uint32 gBufferByteSize = 0;
    drwav converter;

    Uint8* audioHoldingPattern = NULL;

    MicrophoneInstance::MicrophoneInstance(Microphone *aParent)
    {
        //initialize any startup stuff (fix this later)
    }

    /*MicrophoneInstance::~MicrophoneInstance()
    {
        //dispose of the SDL device
    }*/

    unsigned int MicrophoneInstance::getAudio(float* aBuffer, unsigned int aSamplesToRead, unsigned int aBufferSize)
    {

        int size = aBufferSize;
        float *signal = reinterpret_cast<float*>(audioHoldingPattern);

        memcpy(aBuffer, signal, aBufferSize);

        return (unsigned int)sizeof(signal);
    }

    bool MicrophoneInstance::hasEnded()
    {
        return false;
    }

    result MicrophoneInstance::rewind()
    {
        return 0;
    }

    static void _sdl_cb(void * userdata, Uint8 * pcm, int len){
        if (SDL_GetAudioDeviceStatus(gAudioDeviceID) == SDL_AUDIO_PLAYING) {
            memcpy( &audioHoldingPattern[0], pcm, len );
        }
    }

    result Microphone::recordInit(SoLoud::Soloud *aSoloud, unsigned int aFlags, unsigned int aSamplerate, unsigned int aBuffer, unsigned int aChannels)
    {
        if (!SDL_WasInit(SDL_INIT_AUDIO))
        {
            if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
            {
                return UNKNOWN_ERROR;
            }
        }

        SDL_AudioSpec as;
        as.freq = aSamplerate;
        as.format = AUDIO_F32;
        as.channels = 1;
        as.samples = aBuffer;
        as.callback = _sdl_cb;
        as.userdata = (void*)aSoloud;
        gAudioDeviceID = SDL_OpenAudioDevice(NULL, 1, &as, &gActiveAudioSpec, SDL_AUDIO_ALLOW_ANY_CHANGE & ~(SDL_AUDIO_ALLOW_FORMAT_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE));
        if (gAudioDeviceID == 0)
        {
            as.format = AUDIO_S16;
            gAudioDeviceID = SDL_OpenAudioDevice(NULL, 1, &as, &gActiveAudioSpec, SDL_AUDIO_ALLOW_ANY_CHANGE & ~(SDL_AUDIO_ALLOW_FORMAT_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE));
            if (gAudioDeviceID == 0)
            {
                return UNKNOWN_ERROR;
            }
        }

        //Calculate per sample bytes
        int bytesPerSample = gActiveAudioSpec.channels * ( SDL_AUDIO_BITSIZE( gActiveAudioSpec.format ) / 8 );
        
        gBufferByteSize = bytesPerSample * aBuffer;


        audioHoldingPattern = new Uint8[ gBufferByteSize ];

        memset( audioHoldingPattern, 0, gBufferByteSize );

        SDL_PauseAudioDevice(gAudioDeviceID, 0);
        return 0;
    }


    Microphone::Microphone()
    {
        mBaseSamplerate = 44100;
    }

    Microphone::~Microphone()
    {
        stop();
    }


    AudioSourceInstance * Microphone::createInstance()
    {
        return new MicrophoneInstance(this);
    }

};
