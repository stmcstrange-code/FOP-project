#include "Block.h"
#include "Sprite.h"
#include "Sound.h"
#include <iostream>


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

void executeNext(ProgramManager& pm, Sprite& s, Sound& meow, int& currentStep) {
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

            if (b.value == 999) {
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
       case CHANGE_SIZE: {
    s.size += b.value;
    if (s.size < 5) s.size = 5;
    break;
}
      case SET_SIZE: {
    s.size = b.value;
    if (s.size < 5) s.size = 5;
    break;
}

        case PLAY_SOUND: playSound(meow);
            break;
        case SET_VOLUME: setVolume(meow, b.value);
            break;
        case SET_PITCH: setPitch(meow,  b.value);
            break;
        case GOTO_RANDOM: {
    //Scratch-style coordinates
    int randomX = (rand() % 374) - 187;
    int randomY = (rand() % 450) - 225;
    s.x = 837 + randomX;
    s.y = 225 - randomY;
    break;

        }
         case CHANGE_Y: {
    s.y -= b.value;
    break;
                }
case BOUNCE: {
    bool hit = false;
    // LEFT Wall (650)
    if (s.x <= 650) {
        s.angle = 180 - s.angle;
        s.x = 651;
        hit = true;
    }
    // RIGHT Wall (1024)
    else if (s.x >= 974) {
        s.angle = 180 - s.angle;
        s.x = 973;
        hit = true;
    }

    // TOP Wall (0)
    if (s.y <= 0) {
        s.angle = -s.angle;
        s.y = 1;
        hit = true;
    }
    // BOTTOM Wall (450)
    else if (s.y >= 400) {
        s.angle = -s.angle;
        s.y = 399;
        hit = true;
    }
    break;
}
    case SET_Y: {
    s.y = 225 - b.value;
    break;
                 }
    case SET_X: {
            // If input is 0, screen x becomes 837
            s.x = 837 + b.value;
            break;
                    }
           case SHOW: {
    s.isVisible = true;
    break;
                    }
           case HIDE: {
    s.isVisible = false;
    break;
                    }
        case GOTO_MOUSE: {
            int mx, my;
            SDL_GetMouseState(&mx, &my);
            if (mx >= 650 && mx <= 1024 && my >= 0 && my <= 450) {
                s.x = (float)mx - (s.size / 2.0f);
                s.y = (float)my - (s.size / 2.0f);
            }
            break;
        }
        case OP_ADD:
            pm.variables.vars["my variable"] = b.value + b.value2;
            break;
        case OP_SUB:
            pm.variables.vars["my variable"] = b.value - b.value2;
            break;
        case OP_MUL:
            pm.variables.vars["my variable"] = b.value * b.value2;
            break;
        case OP_DIV:
            if (b.value2 != 0)
                pm.variables.vars["my variable"] = b.value / b.value2;
            else
                pm.variables.vars["my variable"] = 0;
            break;
        case OP_GT:
            std::cout << (b.value > b.value2 ? "True" : "False") << std::endl;
            break;
        case OP_LT:
            std::cout << (b.value < b.value2 ? "True" : "False") << std::endl;
            break;
        case OP_EQU:
            std::cout << (b.value == b.value2 ? "True" : "False") << std::endl;
            break;
        case THINK: {
            s.bubbleText = b.text;
            break;
        }
        case SAY:
        {
            s.bubbleText = b.text;
            break;
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