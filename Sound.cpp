#include "Sound.h"
#include <cmath>
#include <iostream>
#include <vector>

void loadSound (Sound& sound , const char* path) {

    sound.chunk = Mix_LoadWAV(path);
    if (!sound.chunk) {
        std::cout << "Error loading sound: " << path << std::endl;
        Mix_GetError();
    }

    sound.volume = MIX_MAX_VOLUME;
    sound.pitch = 1.0f;
}

void playSound(Sound& sound) {
    sound.channel = Mix_PlayChannel(-1, sound.chunk, 0);
}

void stopSound(Sound& sound) {
    Mix_HaltChannel(sound.channel);
}

void setVolume(Sound& sound, int volume) {
    if (volume<0) volume=0;
    if (volume>128) volume=128;

    sound.volume = volume;
    Mix_VolumeChunk(sound.chunk,volume);
}

void setPitch(Sound& sound, float pitch) {
    if (!sound.chunk) return;
    if (sound.channel != -1) Mix_HaltChannel(sound.channel);

    int sampleSize = 2;
    int channels = 2;

    Uint16* samples = (Uint16*)sound.chunk->abuf;
    int lenSamples = sound.chunk->alen / (sampleSize * channels);

    int newLenSamples = static_cast<int>(lenSamples / pitch);
    std::vector<Uint16> newBuffer(newLenSamples * channels);

    // linear interpolation
    for (int i = 0; i < newLenSamples; i++) {
        float srcPos = i * pitch;
        int idx = (int)srcPos;
        float frac = srcPos - idx;

        if (idx + 1 < lenSamples) {
            for (int c = 0; c < channels; c++) {
                int s0 = samples[idx * channels + c];
                int s1 = samples[(idx + 1) * channels + c];
                newBuffer[i * channels + c] = static_cast<Uint16>(s0 + (s1 - s0) * frac);
            }
        } else {
            for (int c = 0; c < channels; c++)
                newBuffer[i * channels + c] = samples[idx * channels + c];
        }
    }

    Mix_Chunk tempChunk = *sound.chunk;
    tempChunk.abuf = (Uint8*)newBuffer.data();
    tempChunk.alen = newLenSamples * channels * sampleSize;

    sound.channel = Mix_PlayChannel(-1, &tempChunk, 0);
    Mix_VolumeChunk(&tempChunk, sound.volume);
}
