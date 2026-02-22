#include "Block.h"
#include "Sprite.h"
#include <SDL2/SDL_mixer.h>
#include "Sound.h"


extern Mix_Chunk* gSound;


void addBlock(ProgramManager& pm, BlockType t, float v) {
    pm.script.push_back({t, v, 0});
}

void preprocessScript(ProgramManager& pm) {
    std::vector<int> stack;
    for (int i = 0; i < (int)pm.script.size(); i++) {
        if (pm.script[i].type == IF) {
            printf("Pairing IF at step %d to END_IF/ELSE at step %d\n", i, pm.script[i].jumpTo);
        }
        if (pm.script[i].type == REPEAT || pm.script[i].type == IF) {
            stack.push_back(i);
        }
        else if (pm.script[i].type == ELSE) {
            if (!stack.empty() && pm.script[stack.back()].type == IF) {
                int ifIdx = stack.back();
                pm.script[ifIdx].jumpTo = i;
                stack.pop_back();
                stack.push_back(i);
            }
        }
        else if (pm.script[i].type == END_LOOP || pm.script[i].type == END_IF) {
            if (!stack.empty()) {
                int startIdx = stack.back();
                stack.pop_back();


                if (pm.script[i].type == END_IF && pm.script[startIdx].type == IF) {
                    pm.script[startIdx].jumpTo = i;
                } else {
                    pm.script[startIdx].jumpTo = i;
                    pm.script[i].jumpTo = startIdx;
                }
            }
        }
    }
}

void executeNext(ProgramManager& pm, Sprite& s, SoundSystem& ss, int& currentStep) {
    if (currentStep >= (int)pm.script.size()) return;

    Block& b = pm.script[currentStep];
    bool jumped = false;

    switch (b.type) {
        case MOVE: {
            s.move(b.value);
            break;
        }
        case TURN:     s.rotate(b.value); break;
        case WAIT:
            SDL_Delay(static_cast<Uint32>(b.value * 1000));
            break;
        case PEN_DOWN: s.penDown = true; break;
        case PEN_UP:   s.penDown = false; break;
        case ERASE:    s.clearTrail(); break;
        case IF: {
            bool conditionMet = false;

            if (b.value == 999) { // Touching Edge Sensing
                // Sidebar area: 0-230 | Stage: 230-1024
                bool touchingLeft   = (s.x <= 280);
                bool touchingRight  = (s.x >= 970);
                bool touchingTop    = (s.y <= 50);
                bool touchingBottom = (s.y >= 710);

                if (touchingLeft || touchingRight || touchingTop || touchingBottom) {
                    conditionMet = true;
                }
            } else {
                conditionMet = (pm.variables.vars["my variable"] > b.value);
            }

            if (!conditionMet) {
                currentStep = b.jumpTo + 1;
                jumped = true;
            }
            break;
        }

        case ELSE:
            currentStep = b.jumpTo + 1;
            jumped = true;
            break;

        case END_IF:
            break;
        case REPEAT:
            if (b.iterations >= b.value) {
                b.iterations = 0;
                currentStep = b.jumpTo + 1;
                jumped = true;
            }
            break;
        case SET_VAR:

        pm.variables.vars["my variable"] = b.value;
            break;

        case CHANGE_VAR:
            pm.variables.vars["my variable"] += b.value;
            break;
        case END_LOOP:
            if (b.jumpTo != -1) {
                pm.script[b.jumpTo].iterations++;
                currentStep = b.jumpTo;
                jumped = true;
            }
            break;

       case CHANGE_X: {
            s.x += b.value;
            break;
        }

        // case PLAY_SOUND: Mix_PlayChannel(-1, gSound,0);
        case PLAY_SOUND: playSound(ss, b.soundName);
            break;
        case SET_VOLUME: setVolume(ss, b.soundName , (int)b.value );
            break;
        case SET_PITCH: setPitch(ss, b.soundName, b.value);
            break;
        case GOTO_RANDOM: {
    //Scratch-style coordinates
    int randomX = (rand() % 374) - 187;
    int randomY = (rand() % 450) - 225;
    s.x = 837 + randomX;
    s.y = 225 - randomY;
    break;
        case SET_X: {
            // If input is 0, screen x becomes 837
            s.x = 837 + b.value;
            break;
        }
}

    }
    if (!jumped) currentStep++;

    //Infinite Loop Watchdog

    static int instructionsThisFrame = 0;
    instructionsThisFrame++;
    if (instructionsThisFrame > 500) {
        SDL_Delay(1);
        instructionsThisFrame = 0;
    }
}