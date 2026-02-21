#ifndef FOP_PROJECT_SOUND_H
#define FOP_PROJECT_SOUND_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <string>
#include <vector>

struct Sound {
    std::string name;
   Mix_Chunk* chunk;
    int volume;
    float pitch;
};

struct SoundSystem {
std::vector<Sound> sounds;
};

void setPitch( SoundSystem& ss , const std::string& name, float pitch);
void playSound(SoundSystem& ss, const std::string& name) ;
void setVolume(SoundSystem& ss, const std::string& name, int volume);


// void initAudio(AudioSystem* system);
// void closeAudio(AudioSystem* system);
//
// int addSound(AudioSystem* system , const char* name, const char* path);
// Sound* getSound (AudioSystem* system, const char* name );
//
// void playSound(AudioSystem* system,Sound* sound, float volume ,float pitch );

#endif