#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include "Sprite.h"
#include "Block.h"
#include "FileHandler.h"
#include <SDL2/SDL2_gfx.h>
#include <SDL2/SDL_mixer.h>

#include "Sound.h"

// Categories
enum Category { MOTION, LOOKS, SOUND, EVENTS, CONTROL, SENSING, OPERATORS, VARIABLES, MY_BLOCKS };

// which category a block belongs to
Category getCategory(BlockType type) {
    if (type == MOVE || type == TURN) return MOTION;
    if (type == PEN_DOWN || type == PEN_UP || type == ERASE) return LOOKS;
    if (type == REPEAT || type == END_LOOP || type == WAIT) return CONTROL;
    if (type == SET_VAR || type == CHANGE_VAR) return VARIABLES;
    if (type == PLAY_SOUND || type == SET_VOLUME ||type == SET_PITCH ) return SOUND;
    return MOTION;
}

void renderText(SDL_Renderer* ren, TTF_Font* font, std::string text, int x, int y, SDL_Color color, bool center = false) {
    if (!font || text.empty()) return;
    SDL_Surface* surf = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);
    SDL_Rect dst = {x, y, surf->w, surf->h};
    if (center) {
        dst.x -= surf->w / 2;
    }
    SDL_RenderCopy(ren, tex, nullptr, &dst);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
}

Mix_Chunk* gSound= nullptr;


int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    SoundSystem  soundSystem;
    gSound = Mix_LoadWAV("assets/sound.wav");
    TTF_Init();

    SDL_Window* win = SDL_CreateWindow("Scratch Pro", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 768, 0);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    TTF_Font* font = TTF_OpenFont("assets/arial.ttf", 14);

    Category currentCategory = MOTION;

    // Sidebar Templates
    std::vector<VisualBlock> sidebarTemplates = {
        {{MOVE, 40, 0}, {95, 60, 120, 40}, {76, 151, 255, 255}, "MOVE "},
        {{TURN, 15, 0}, {95, 110, 120, 40}, {76, 151, 255, 255}, "TURN "},
        {{PEN_DOWN, 0, 0}, {95, 60, 120, 40}, {64, 184, 158, 255}, "PEN DOWN"},
        {{PEN_UP, 0, 0}, {95, 110, 120, 40}, {64, 184, 158, 255}, "PEN UP"},
        {{ERASE, 0, 0}, {95, 160, 120, 40}, {255, 102, 102, 255}, "ERASE"},
        {{WAIT, 1, 0}, {95, 60, 120, 40}, {255, 171, 25, 255}, "WAIT "},
        {{REPEAT, 4, 0}, {95, 110, 120, 40}, {255, 171, 25, 255}, "REPEAT "},
        {{END_LOOP, 0, 0}, {95, 160, 120, 40}, {255, 171, 25, 255}, "END LOOP"},
        {{SET_VAR, 0, 0}, {95, 60, 120, 40}, {255, 140, 26, 255}, "SET TO "},
        {{CHANGE_VAR, 1, 0}, {95, 110, 120, 40}, {255, 140, 26, 255}, "CHANGE BY "},
        {{PLAY_SOUND, 0 ,0}, {95, 60, 120, 40} ,{207, 99, 207, 255}, "PLAY SOUND"},
        {{SET_VOLUME, 50 ,0}, {95, 110, 120, 40} ,{207, 99, 207, 255}, "SET VOLUME"},
        {{SET_PITCH, 1.0f ,0}, {95, 160, 120, 40} ,{207, 99, 207, 255}, "SET PITCH"},
    };

    struct CategoryUI {
        std::string name;
        SDL_Color color;
    };
    std::vector<CategoryUI> catUIs = {
        {"Motion", {60, 131, 253, 255}}, {"Looks", {153, 102, 255, 255}},
        {"Sound", {207, 99, 207, 255}}, {"Events", {255, 191, 0, 255}},
        {"Control", {255, 171, 25, 255}}, {"Sensing", {92, 177, 214, 255}},
        {"Operators", {89, 192, 89, 255}}, {"Variables", {255, 140, 26, 255}},
        {"My Blocks", {255, 102, 128, 255}}
    };

    std::vector<VisualBlock> workspaceBlocks;
    VisualBlock *activeDragBlock = nullptr, *editingBlock = nullptr;
    int dragOffsetX = 0, dragOffsetY = 0;
    Sprite cat; cat.loadBMP(ren, "assets/cat.bmp");
    ProgramManager manager;
    bool isRunning = false, quit = false, draggingCat = false;
    int currentStep = 0;

    while (!quit) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = true;

            if (editingBlock) {
                if (e.type == SDL_TEXTINPUT) {
                    if (isdigit(e.text.text[0]) || e.text.text[0] == '.') editingBlock->editBuffer += e.text.text;
                } else if (e.type == SDL_KEYDOWN) {
                    if (e.key.keysym.sym == SDLK_BACKSPACE && !editingBlock->editBuffer.empty()) editingBlock->editBuffer.pop_back();
                    if (e.key.keysym.sym == SDLK_RETURN) {
                        if (!editingBlock->editBuffer.empty()) {
                            try { editingBlock->data.value = std::stof(editingBlock->editBuffer); } catch (...) { editingBlock->data.value = 0; }
                        }
                        editingBlock->isEditing = false; editingBlock = nullptr; SDL_StopTextInput();
                    }
                }
            }

            if (e.type == SDL_MOUSEBUTTONDOWN) {
                SDL_Point p = {e.button.x, e.button.y};

                for(int i = 0; i < (int)catUIs.size(); i++) {
                    int cX = 40, cY = 50 + (i * 65);
                    if (sqrt(pow(p.x - cX, 2) + pow(p.y - cY, 2)) <= 20) currentCategory = static_cast<Category>(i);
                }

                SDL_Rect runBtn = {95, 600, 120, 35}, saveBtn = {95, 645, 120, 35}, loadBtn = {95, 690, 120, 35};
                if (SDL_PointInRect(&p, &runBtn)) {
                    manager.script.clear();
                    std::sort(workspaceBlocks.begin(), workspaceBlocks.end(), [](const VisualBlock& a, const VisualBlock& b) { return a.rect.y < b.rect.y; });
                    for (auto& vb : workspaceBlocks) manager.script.push_back(vb.data);
                    isRunning = true; currentStep = 0;
                }
                else if (SDL_PointInRect(&p, &saveBtn)) saveProject(workspaceBlocks, "assets/project.txt");
                else if (SDL_PointInRect(&p, &loadBtn)) loadProject(workspaceBlocks, "assets/project.txt");

                SDL_Rect catR = {(int)cat.x-30, (int)cat.y-30, 60, 60};
                if (SDL_PointInRect(&p, &catR)) { draggingCat = true; dragOffsetX = p.x - (int)cat.x; dragOffsetY = p.y - (int)cat.y; }

                for (auto& temp : sidebarTemplates) {
                    if (getCategory(temp.data.type) == currentCategory && SDL_PointInRect(&p, &temp.rect)) {
                        VisualBlock newB = temp; newB.isDragging = true;
                        workspaceBlocks.push_back(newB); activeDragBlock = &workspaceBlocks.back();
                        dragOffsetX = p.x - newB.rect.x; dragOffsetY = p.y - newB.rect.y;
                    }
                }
                if (!activeDragBlock && !draggingCat) {
                    for (int i = (int)workspaceBlocks.size() - 1; i >= 0; i--) {
                        if (SDL_PointInRect(&p, &workspaceBlocks[i].rect)) {
                            if (p.x > workspaceBlocks[i].rect.x + 80) {
                                editingBlock = &workspaceBlocks[i]; editingBlock->isEditing = true;
                                editingBlock->editBuffer = ""; SDL_StartTextInput();
                            } else {
                                activeDragBlock = &workspaceBlocks[i]; activeDragBlock->isDragging = true;
                                dragOffsetX = p.x - activeDragBlock->rect.x; dragOffsetY = p.y - activeDragBlock->rect.y;
                            }
                            break;
                        }
                    }
                }
            }

            if (e.type == SDL_MOUSEMOTION) {
                if (activeDragBlock) { activeDragBlock->rect.x = e.motion.x - dragOffsetX; activeDragBlock->rect.y = e.motion.y - dragOffsetY; }
                if (draggingCat) { cat.x = (float)(e.motion.x - dragOffsetX); cat.y = (float)(e.motion.y - dragOffsetY); }
            }

            if (e.type == SDL_MOUSEBUTTONUP) {
                if (activeDragBlock) {
                    if (activeDragBlock->rect.x < 230) {
                        for (auto it = workspaceBlocks.begin(); it != workspaceBlocks.end(); ++it) {
                            if (&(*it) == activeDragBlock) { workspaceBlocks.erase(it); break; }
                        }
                    } else {
                        for (auto& other : workspaceBlocks) {
                            if (&other == activeDragBlock) continue;
                            if (abs(activeDragBlock->rect.x - other.rect.x) < 30 && abs(activeDragBlock->rect.y - (other.rect.y + 40)) < 25) {
                                activeDragBlock->rect.x = other.rect.x; activeDragBlock->rect.y = other.rect.y + 40; break;
                            }
                        }
                    }
                }
                activeDragBlock = nullptr; draggingCat = false;
            }
        }

        if (isRunning && currentStep < (int)manager.script.size()) {
            executeNext(manager, cat , soundSystem, currentStep); cat.checkBoundaries(800, 768, 230); SDL_Delay(100);
        } else isRunning = false;

        SDL_SetRenderDrawColor(ren, 240, 240, 240, 255); SDL_RenderClear(ren);

        SDL_Rect toolbar = {0,0,80,768}, blocksArea = {80,0,150,768}, stage = {800,0,224,768};
        SDL_SetRenderDrawColor(ren, 255,255,255,255); SDL_RenderFillRect(ren, &toolbar);
        SDL_SetRenderDrawColor(ren, 225,225,225,255); SDL_RenderFillRect(ren, &blocksArea);
        SDL_SetRenderDrawColor(ren, 245,245,245,255); SDL_RenderFillRect(ren, &stage);



        for (int i = 0; i < (int)catUIs.size(); i++) {
            int cX = 40, cY = 50 + (i * 65);
            if (currentCategory == i) aacircleRGBA(ren, cX, cY, 23, 0, 0, 0, 100);
            filledCircleRGBA(ren, cX, cY, 20, catUIs[i].color.r, catUIs[i].color.g, catUIs[i].color.b, 255);
            renderText(ren, font, catUIs[i].name, cX, cY + 22, {80, 80, 80, 255}, true);
        }

        for (auto& b : sidebarTemplates) {
            if (getCategory(b.data.type) == currentCategory) {
                SDL_SetRenderDrawColor(ren, b.color.r, b.color.g, b.color.b, 255); SDL_RenderFillRect(ren, &b.rect);
                bool hasNum = (b.data.type == MOVE || b.data.type == TURN || b.data.type == WAIT || b.data.type == REPEAT || b.data.type == SET_VAR || b.data.type == CHANGE_VAR);
                std::string lbl = b.label + (hasNum ? std::to_string((int)b.data.value) : "");
                renderText(ren, font, lbl, b.rect.x + 10, b.rect.y + 10, {255, 255, 255, 255});
            }
        }

        for (auto& b : workspaceBlocks) {
            SDL_SetRenderDrawColor(ren, b.color.r, b.color.g, b.color.b, 255); SDL_RenderFillRect(ren, &b.rect);
            std::string t = b.label;
            bool hasNum = (b.data.type == MOVE || b.data.type == TURN || b.data.type == WAIT || b.data.type == REPEAT || b.data.type == SET_VAR || b.data.type == CHANGE_VAR);
            if (hasNum) t += b.isEditing ? b.editBuffer + "|" : std::to_string((int)b.data.value);
            renderText(ren, font, t, b.rect.x + 10, b.rect.y + 10, {255, 255, 255, 255});
        }

        SDL_Rect runBtnRect = {95, 600, 120, 35}, saveBtnRect = {95, 645, 120, 35}, loadBtnRect = {95, 690, 120, 35};
        SDL_SetRenderDrawColor(ren, 50, 200, 50, 255); SDL_RenderFillRect(ren, &runBtnRect);
        renderText(ren, font, "GO / RUN", 155, 610, {255, 255, 255, 255}, true);
        SDL_SetRenderDrawColor(ren, 100, 100, 100, 255); SDL_RenderFillRect(ren, &saveBtnRect);
        renderText(ren, font, "SAVE", 155, 655, {255, 255, 255, 255}, true);
        SDL_RenderFillRect(ren, &loadBtnRect);
        renderText(ren, font, "LOAD", 155, 700, {255, 255, 255, 255}, true);

        cat.draw(ren);
        SDL_RenderPresent(ren);
    }
    if(font) TTF_CloseFont(font);
    TTF_Quit(); SDL_Quit(); return 0;
}