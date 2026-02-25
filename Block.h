
#ifndef FOP_PROJECT_BLOCK_H
#define FOP_PROJECT_BLOCK_H

#include <vector>
#include <SDL2/SDL.h>
#include <map>
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
    CHANGE_X,
    SET_X,
    CHANGE_Y,
    SET_Y,
    BOUNCE,
    CHANGE_SIZE,
    SET_SIZE,
    SHOW,
    HIDE,
    GOTO_MOUSE,
    OP_ADD, OP_SUB, OP_MUL, OP_DIV,
    OP_GT, OP_LT, OP_EQU,
    MOUSE_X,
    MOUSE_Y,
    DISTANCE_TO_MOUSE,
    TOUCHING_MOUSE,
    MOUSE_DOWN,
    THINK,
    SAY
};

struct Block {
    BlockType type;
    float value;
    float value2;
    int iterations = 0;
    int jumpTo = -1;
    std::string soundName;
    std::string text="";

};


struct VisualBlock {
    Block data;
    SDL_Rect rect;
    SDL_Color color;
    std::string label;
    bool isEditing = false;
    bool isDragging = false;
    bool isEditingValue2 = false;
    std::string editBuffer;
    std::string editBuffer2;
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
void executeNext(ProgramManager& pm, Sprite& s, Sound& sound, int& currentStep);
void preprocessScript(ProgramManager& pm);


#endif //FOP_PROJECT_BLOCK_H