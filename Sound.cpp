#include "Sound.h"

#include <cmath>
#include <iostream>

bool loadSound (SoundSystem& ss , const std::string& name , const std::string& path) {
Mix_Chunk *chunk = Mix_LoadWAV(path.c_str());
    if (!chunk) {
        std::cout << "Error loading sound: " << path << std::endl;
        return false;
    }
    Sound s;
    s.name = name;
    s.chunk = chunk;
    s.volume = MIX_MAX_VOLUME;

    ss.sounds.push_back(s);
    return true;
}

Sound* findSound( SoundSystem& ss , const std::string& name ) {
    for (auto& s : ss.sounds) {
        if (s.name == name) {
            return &s;
        }
    }
return nullptr;
}

void setPitch( SoundSystem& ss , const std::string& name, float pitch) {

    Sound* s = findSound(ss, name);
    if (!s) return;

    s->pitch = pitch;
}

Mix_Chunk* pitchChunk(Mix_Chunk* original,float pitch ) {
if (!original || pitch<=0.0f) return nullptr;

    Uint32 newLength = static_cast<Uint32> (original-> alen/pitch);
    Uint8* newBuffer = new Uint8[newLength];

    for ( Uint32 i = 0; i < newLength; i++) {
        Uint32 srcIndex = static_cast<Uint32> (i* pitch);
        if (srcIndex < original->alen) {
            newBuffer[i]= original->abuf[srcIndex];
        }
        else {newBuffer[i]=0;}
    }
    Mix_Chunk* newChunk = Mix_QuickLoad_RAW(newBuffer , newLength);
    return newChunk;
}

    void playSound(SoundSystem& ss, const std::string& name) {
   Sound* s = findSound(ss, name);
    if (!s) return;

    Mix_VolumeChunk(s->chunk, s->volume);

    if (std::fabs(s->pitch - 1.0f)< 0.01f) {
        Mix_PlayChannel(-1, s->chunk, 0);
    }
    else {
        Mix_Chunk* pitched = pitchChunk(s->chunk, s->pitch);
        Mix_PlayChannel(-1, pitched, 0);
    }
}

void setVolume(SoundSystem& ss, const std::string& name, int volume) {
    Sound* s = findSound(ss, name);
    if (!s) return;

    s->volume = volume;
}



// void initAudio(AudioSystem* system) {
//     system-> soundCount=0;
//     SDL_AudioSpec want;
//     want.freq = 44100;
//     want.format = AUDIO_S16SYS;
//     want.channels = 2;
//     want.samples = 1024;
//     want.callback = NULL;
//
//     system->device = SDL_OpenAudioDevice(NULL,0,&want,NULL,0);
// }
//
// void closeAudio(AudioSystem *system) {
//     for (int i=0; i < system->soundCount; i++) {
//         SDL_FreeWAV (system-> sounds[i].buffer);
//     }
//     SDL_CloseAudioDevice(system->device);
// }
//
// int addSound(AudioSystem* system , const char* name, const char* path) {
//     if (system-> soundCount >= 50) return -1;
//
//     Sound* s= &system-> sounds[system-> soundCount];
//     strcpy (s->name, name);
//
//     if (SDL_LoadWAV(path , &s->spec , &s->buffer , &s->length) == NULL) return -1;
//
//     system-> soundCount++;
//     return system-> soundCount - 1;
// }
//
// Sound* getSound (AudioSystem* system, const char* name ) {
//     for (int i=0; i < system->soundCount; i++) {
//         if (strcmp(system-> sounds[i].name , name) ==0) { return &system-> sounds[i];}
//     }
//     return NULL;
// }
//
//
// //
// // void setPitch(AudioSystem* system,Sound* sound, float volume ,float pitch) {
// //     if (!sound) return;
// //
// //     if (pitch> 0.1f && pitch < 4.0f) {sound-> pitch = pitch;}
// // }