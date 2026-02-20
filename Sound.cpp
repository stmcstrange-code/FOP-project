#include "Sound.h"
#include <cstring>
#include <iostream>

void initAudio(AudioSystem* system) {
    system-> soundCount=0;
    SDL_AudioSpec want;
    want.freq = 44100;
    want.format = AUDIO_S16SYS;
    want.channels = 2;
    want.samples = 1024;
    want.callback = NULL;

    system->device = SDL_OpenAudioDevice(NULL,0,&want,NULL,0);
}

void closeAudio(AudioSystem *system) {
    for (int i=0; i < system->soundCount; i++) {
        SDL_FreeWAV (system-> sounds[i].buffer);
    }
    SDL_CloseAudioDevice(system->device);
}

int addSound(AudioSystem* system , const char* name, const char* path) {
    if (system-> soundCount >= 50) return -1;

    Sound* s= &system-> sounds[system-> soundCount];
    strcpy (s->name, name);

    if (SDL_LoadWAV(path , &s->spec , &s->buffer , &s->length) == NULL) return -1;

    system-> soundCount++;
    return system-> soundCount - 1;
}

Sound* getSound (AudioSystem* system, const char* name ) {
    for (int i=0; i < system->soundCount; i++) {
        if (strcmp(system-> sounds[i].name , name) ==0) { return &system-> sounds[i];}
    }
    return NULL;
}

void playSound(AudioSystem* system,Sound* sound, float volume ,float pitch) {
    if (!sound) return;

    Uint8* tempBuffer = (Uint8*) malloc ( sound -> length);

    for (Uint32 i = 0; i < sound -> length; i++) {
        tempBuffer[i] = sound -> buffer[i]* volume;
    }
    SDL_QueueAudio(system->device, tempBuffer, sound -> length);
    free(tempBuffer);
}
//
// void setPitch(AudioSystem* system,Sound* sound, float volume ,float pitch) {
//     if (!sound) return;
//
//     if (pitch> 0.1f && pitch < 4.0f) {sound-> pitch = pitch;}
// }