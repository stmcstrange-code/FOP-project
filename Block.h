
#ifndef FOP_PROJECT_BLOCK_H
#define FOP_PROJECT_BLOCK_H

#include <vector>
#include <string>
#include <SDL2/SDL.h>
#include <map>
#pragma once
#include <string>
#include "Sound.h"


enum BlockType {
    MOVE,
    TURN,
    PEN_DOWN,
    PEN_UP,
    ERASE,
    REPEAT,
    END_LOOP,
    WAIT,
    SET_VAR,
    CHANGE_VAR,
    PLAY_SOUND,
    SET_VOLUME,
    SET_PITCH,
    IF,
    ELSE,
    END_IF,
    TOUCHING_EDGE,
    GOTO_RANDOM,
    CHANGE_X
};


struct Block {
    BlockType type;
    float value;
    int iterations = 0;
    int jumpTo = -1;
    std::string soundName;
};


struct VisualBlock {
    Block data;
    SDL_Rect rect;
    SDL_Color color;
    std::string label;
    bool isDragging = false;
    bool isEditing = false;
    std::string editBuffer;
};


struct VariableManager {
    std::map<std::string, float> vars;
    VariableManager() { vars["my variable"] = 0.0f; }
};

struct ProgramManager {
    std::vector<Block> script;
    VariableManager variables;
};

struct Sprite;


void addBlock(ProgramManager& pm, BlockType t, float v);
void executeNext(ProgramManager& pm, Sprite& s, SoundSystem& soundSystem, int& currentStep);
void preprocessScript(ProgramManager& pm);


#endif //FOP_PROJECT_BLOCK_H