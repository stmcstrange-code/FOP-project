#include "Block.h"
#include "Sprite.h"

void addBlock(ProgramManager& pm, BlockType t, float v) {
    pm.script.push_back({t, v, 0});
}

void executeNext(ProgramManager& pm, Sprite& s, int& currentStep) {
    if (currentStep >= (int)pm.script.size()) return;

    Block& b = pm.script[currentStep];
    bool jumped = false;

    switch (b.type) {
        case MOVE:     s.move(b.value); break;
        case TURN:     s.rotate(b.value); break;
        case PEN_DOWN: s.penDown = true; break;
        case PEN_UP:   s.penDown = false; break;
        case ERASE:    s.clearTrail(); break;

        case REPEAT:

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
    }


    if (!jumped) {
        currentStep++;
    }
}