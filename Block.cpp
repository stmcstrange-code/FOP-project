#include "Block.h"
#include "Sprite.h"
#include <SDL2/SDL_mixer.h>
#include "Sound.h"


extern Mix_Chunk* gSound;


void addBlock(ProgramManager& pm, BlockType t, float v) {
    pm.script.push_back({t, v, 0});
}

void executeNext(ProgramManager& pm, Sprite& s, SoundSystem& ss, int& currentStep) {
    if (currentStep >= (int)pm.script.size()) return;

    Block& b = pm.script[currentStep];
    bool jumped = false;

    switch (b.type) {
        case MOVE:     s.move(b.value); break;
        case TURN:     s.rotate(b.value); break;
        case WAIT:
            SDL_Delay(static_cast<Uint32>(b.value * 1000));
            break;
        case PEN_DOWN: s.penDown = true; break;
        case PEN_UP:   s.penDown = false; break;
        case ERASE:    s.clearTrail(); break;
        case REPEAT:   break;
        case SET_VAR:

        pm.variables.vars["my variable"] = b.value;
            break;

        case CHANGE_VAR:
            pm.variables.vars["my variable"] += b.value;
            break;
        case END_LOOP:
            for (int i = currentStep - 1; i >= 0; i--) {
                if (pm.script[i].type == REPEAT) {
                    pm.script[i].iterations++;
                    if (pm.script[i].iterations < pm.script[i].value) {
                        currentStep = i;
                        jumped = true;
                    } else {
                        pm.script[i].iterations = 0;
                    }
                    break;
                }
            }
            break;

        // case PLAY_SOUND: Mix_PlayChannel(-1, gSound,0);
        case PLAY_SOUND: playSound(ss, b.soundName);
            break;
        case SET_VOLUME: setVolume(ss, b.soundName , (int)b.value );
            break;
        case SET_PITCH: setPitch(ss, b.soundName, b.value);
            break;

    }
    if (!jumped) currentStep++;
}