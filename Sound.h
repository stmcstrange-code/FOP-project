#ifndef FOP_PROJECT_SOUND_H
#define FOP_PROJECT_SOUND_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

struct Sound {
    char name[50];
    SDL_AudioSpec spec;
    Uint8 *buffer;
    Uint32 length;
};

struct AudioSystem {
    Sound sounds[50];
    int soundCount;
    SDL_AudioDeviceID device;
};


void initAudio(AudioSystem* system);
void closeAudio(AudioSystem* system);

int addSound(AudioSystem* system , const char* name, const char* path);
Sound* getSound (AudioSystem* system, const char* name );

void playSound(AudioSystem* system,Sound* sound, float volume ,float pitch );

#endif