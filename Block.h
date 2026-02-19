
#ifndef FOP_PROJECT_BLOCK_H
#define FOP_PROJECT_BLOCK_H

#include <vector>

enum BlockType { MOVE, TURN, PEN_DOWN, PEN_UP, ERASE, REPEAT, END_LOOP };

struct Block {
    BlockType type;
    float value;
    int iterations;
};

struct ProgramManager {
    std::vector<Block> script;
};

struct Sprite;

void addBlock(ProgramManager& pm, BlockType t, float v);
void executeNext(ProgramManager& pm, Sprite& s, int& currentStep);


#endif //FOP_PROJECT_BLOCK_H