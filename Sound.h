#ifndef FOP_PROJECT_SOUND_H
#define FOP_PROJECT_SOUND_H

#include <SDL2/SDL_mixer.h>

struct Sound{
    Mix_Chunk* chunk;
    int channel;
    int volume;
    float pitch;
};

extern Sound meow;


void loadSound (Sound& sound , const char* path);
void playSound(Sound& sound);
void stopSound(Sound& sound);
void setVolume(Sound& sound, int volume);
void setPitch( Sound& sound , float pitch);

#endif