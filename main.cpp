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
#include <SDL2/SDL_image.h>
#include "file_dialog.h"

enum Category { MOTION, LOOKS, SOUND, EVENTS, CONTROL, SENSING, OPERATORS, VARIABLES, MY_BLOCKS };
enum EditingField { NONE, FIELD_X, FIELD_Y, FIELD_SIZE, FIELD_DIR };

Category getCategory(BlockType type) {
    if (type == MOVE || type == TURN || type == GOTO_RANDOM || type == CHANGE_X || type == SET_X || type == CHANGE_Y || type == SET_Y || type == BOUNCE) return MOTION;
    if (type == PEN_DOWN || type == PEN_UP || type == ERASE || type == CHANGE_SIZE || type == SET_SIZE) return LOOKS;
    if (type == TOUCHING_EDGE) return SENSING;
    if (type == REPEAT || type == END_LOOP || type == WAIT || type == IF || type == ELSE || type == END_IF)
        return CONTROL;
    if (type == SET_VAR || type == CHANGE_VAR) return VARIABLES;
    if (type == PLAY_SOUND || type == SET_VOLUME || type == SET_PITCH) return SOUND;
    return MOTION;
}

void renderText(SDL_Renderer* ren, TTF_Font* font, std::string text, int x, int y, SDL_Color color, bool center = false) {
    if (!font || text.empty()) return;
    SDL_Surface* surf = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);
    SDL_Rect dst = {x, y, surf->w, surf->h};
    if (center) dst.x -= surf->w / 2;
    SDL_RenderCopy(ren, tex, nullptr, &dst);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
}

Sound meow;

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    TTF_Init();

    if ( Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT,2,2048) <0) {
        std::cerr << "Error opening audio mixer" << std::endl;
        Mix_GetError();
        return -1;
    }
    loadSound(meow ,"assets/sound.wav" );


    SDL_Window* win = SDL_CreateWindow("Scratch", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 768, 0);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    TTF_Font* font = TTF_OpenFont("assets/arial.ttf", 14);
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);

    Category currentCategory = MOTION;
    EditingField activeField = NONE;
    std::string paneBuffer = "";
    SDL_Texture* backgroundTexture = nullptr;

    // Layout Rects
    SDL_Rect sidebarArea   = {0, 0, 230, 768};
    SDL_Rect workSpace     = {230, 0, 420, 768};
    SDL_Rect stageArea     = {650, 0, 374, 450};
    SDL_Rect spritePane    = {650, 450, 374, 318};

    // Sprite Pane Input Boxes
    SDL_Rect xBox = {690, 470, 45, 25};
    SDL_Rect yBox = {765, 470, 45, 25};
    SDL_Rect sBox = {855, 470, 45, 25};
    SDL_Rect dBox = {955, 470, 45, 25};

    // Show/Hide Buttons
    SDL_Rect showBtn = {710, 510, 35, 25};
    SDL_Rect hideBtn = {750, 510, 35, 25};

  std::vector<VisualBlock> sidebarTemplates = {
    // --- MOTION (Blue) ---
    {{MOVE, 40, 0}, {95, 60, 120, 40}, {76, 151, 255, 255}, "MOVE "},
    {{TURN, 15, 0}, {95, 110, 120, 40}, {76, 151, 255, 255}, "TURN "},
    {{GOTO_RANDOM, 0, 0}, {95, 160, 120, 40}, {76, 151, 255, 255}, "GO RANDOM"},
    {{CHANGE_X, 10, 0}, {95, 210, 120, 40}, {76, 151, 255, 255}, "change x by "},
    {{SET_X, 0, 0}, {95, 260, 120, 40}, {76, 151, 255, 255}, "set x to "},
    {{CHANGE_Y, 10, 0}, {95, 310, 120, 40}, {76, 151, 255, 255}, "change y by "},
    {{SET_Y, 0, 0}, {95, 360, 120, 40}, {76, 151, 255, 255}, "set y to "},
    {{BOUNCE, 0, 0}, {95, 410, 120, 40}, {76, 151, 255, 255}, "if on edge,bounce"},

    // --- LOOKS (Purple) ---
    {{CHANGE_SIZE, 10, 0}, {95, 60, 120, 40}, {153, 102, 255, 255}, "change size by "},
    {{SET_SIZE, 100, 0}, {95, 110, 120, 40}, {153, 102, 255, 255}, "set size to "},
    {{PEN_DOWN, 0, 0}, {95, 160, 120, 40}, {64, 184, 158, 255}, "PEN DOWN"},
    {{PEN_UP, 0, 0}, {95, 210, 120, 40}, {64, 184, 158, 255}, "PEN UP"},
    {{ERASE, 0, 0}, {95, 260, 120, 40}, {255, 102, 102, 255}, "ERASE"},

    // --- CONTROL (Orange) ---
    {{WAIT, 1, 0}, {95, 60, 120, 40}, {255, 171, 25, 255}, "WAIT "},
    {{REPEAT, 4, 0}, {95, 110, 120, 40}, {255, 171, 25, 255}, "REPEAT "},
    {{END_LOOP, 0, 0}, {95, 160, 120, 40}, {255, 171, 25, 255}, "END LOOP"},
    {{IF, 10, 0}, {95, 210, 120, 40}, {255, 171, 25, 255}, "IF VAR > "},
    {{ELSE, 0, 0}, {95, 260, 120, 40}, {255, 171, 25, 255}, "ELSE"},
    {{END_IF, 0, 0}, {95, 310, 120, 40}, {255, 171, 25, 255}, "END IF"},

    // --- VARIABLES (Dark Orange) ---
    {{SET_VAR, 0, 0}, {95, 60, 120, 40}, {255, 140, 26, 255}, "SET TO "},
    {{CHANGE_VAR, 1, 0}, {95, 110, 120, 40}, {255, 140, 26, 255}, "CHANGE BY "},

    // --- SOUND (Pink) ---
    {{PLAY_SOUND, 0, 0}, {95, 60, 120, 40}, {207, 99, 207, 255}, "PLAY SOUND"},
    {{SET_VOLUME, 50, 0}, {95, 110, 120, 40}, {207, 99, 207, 255}, "SET VOLUME"},
    {{SET_PITCH, 1.0f, 0}, {95, 160, 120, 40}, {207, 99, 207, 255}, "SET PITCH"},

    // --- SENSING (Light Blue) ---
    {{TOUCHING_EDGE, 0, 0}, {95, 60, 120, 40}, {92, 177, 214, 255}, "Touching Edge?"}
};

    struct CategoryUI { std::string name; SDL_Color color; };
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
    Sprite cat;
    cat.loadBMP(ren, "assets/cat.bmp");
    cat.x = 837; cat.y = 225;

    ProgramManager manager;
    bool isRunning = false, quit = false, draggingCat = false;
    int currentStep = 0;

    while (!quit) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = true;

            if (activeField != NONE) {
                if (e.type == SDL_TEXTINPUT) {
                    if (isdigit(e.text.text[0]) || e.text.text[0] == '-' || e.text.text[0] == '.') paneBuffer += e.text.text;
                } else if (e.type == SDL_KEYDOWN) {
                    if (e.key.keysym.sym == SDLK_BACKSPACE && !paneBuffer.empty()) paneBuffer.pop_back();
                    if (e.key.keysym.sym == SDLK_RETURN) {
                        try {
                            float val = std::stof(paneBuffer);
                            if (activeField == FIELD_X) cat.x = val + 837;
                            else if (activeField == FIELD_Y) cat.y = 225 - val;
                            else if (activeField == FIELD_SIZE) cat.size = val;
                            else if (activeField == FIELD_DIR) cat.angle = val;
                        } catch (...) {}
                        activeField = NONE; SDL_StopTextInput();
                    }
                }
            }

            if (editingBlock) {
                if (e.type == SDL_TEXTINPUT) {
                    if (isdigit(e.text.text[0]) || e.text.text[0] == '.') editingBlock->editBuffer += e.text.text;
                } else if (e.type == SDL_KEYDOWN) {
                    if (e.key.keysym.sym == SDLK_BACKSPACE && !editingBlock->editBuffer.empty()) editingBlock->editBuffer.pop_back();
                    if (e.key.keysym.sym == SDLK_RETURN) {
                        if (!editingBlock->editBuffer.empty()) editingBlock->data.value = std::stof(editingBlock->editBuffer);
                        editingBlock->isEditing = false; editingBlock = nullptr; SDL_StopTextInput();
                    }
                }
            }

            if (e.type == SDL_MOUSEBUTTONDOWN) {
                SDL_Point p = {e.button.x, e.button.y};

                if (SDL_PointInRect(&p, &xBox)) { activeField = FIELD_X; paneBuffer = ""; SDL_StartTextInput(); }
                else if (SDL_PointInRect(&p, &yBox)) { activeField = FIELD_Y; paneBuffer = ""; SDL_StartTextInput(); }
                else if (SDL_PointInRect(&p, &sBox)) { activeField = FIELD_SIZE; paneBuffer = ""; SDL_StartTextInput(); }
                else if (SDL_PointInRect(&p, &dBox)) { activeField = FIELD_DIR; paneBuffer = ""; SDL_StartTextInput(); }
                else if (SDL_PointInRect(&p, &showBtn)) { cat.visible = true; }
                else if (SDL_PointInRect(&p, &hideBtn)) { cat.visible = false; }
                else { activeField = NONE; }

                SDL_Rect loadBgBtn = {880, 660, 120, 35};
                if (SDL_PointInRect(&p, &loadBgBtn)) {
                    char* path = openSpriteDialog();
                    if (path) {
                        SDL_Surface* img = IMG_Load(path);
                        if (img) {
                            if(backgroundTexture) SDL_DestroyTexture(backgroundTexture);
                            backgroundTexture = SDL_CreateTextureFromSurface(ren, img);
                            SDL_FreeSurface(img);
                        }
                    }
                }

                SDL_Rect loadImageBtn = {880, 700, 120, 35};
                if (SDL_PointInRect(&p, &loadImageBtn)) {
                    char* path = openSpriteDialog();
                    if (path) {
                        SDL_Surface* img = IMG_Load(path);
                        if (img) { cat.texture = SDL_CreateTextureFromSurface(ren, img); SDL_FreeSurface(img); }
                    }
                }

                for(int i = 0; i < (int)catUIs.size(); i++) {
                    int cX = 40, cY = 50 + (i * 65);
                    if (sqrt(pow(p.x - cX, 2) + pow(p.y - cY, 2)) <= 20) currentCategory = static_cast<Category>(i);
                }

                SDL_Rect runBtn = {95, 600, 120, 35}, saveBtn = {95, 645, 120, 35}, loadBtn = {95, 690, 120, 35};
                if (SDL_PointInRect(&p, &runBtn)) {
                    manager.script.clear();
                    std::sort(workspaceBlocks.begin(), workspaceBlocks.end(), [](const VisualBlock& a, const VisualBlock& b) { return a.rect.y < b.rect.y; });
                    for (auto& vb : workspaceBlocks) manager.script.push_back(vb.data);
                    preprocessScript(manager); isRunning = true; currentStep = 0;
                }
                else if (SDL_PointInRect(&p, &saveBtn)) saveProject(workspaceBlocks, "assets/project.txt");
                else if (SDL_PointInRect(&p, &loadBtn)) loadProject(workspaceBlocks, "assets/project.txt");

                SDL_Rect catR = {(int)cat.x - 30, (int)cat.y - 30, 60, 60};
                if (SDL_PointInRect(&p, &catR)) { draggingCat = true; dragOffsetX = p.x - (int)cat.x; dragOffsetY = p.y - (int)cat.y; }

                for (auto& temp : sidebarTemplates) {
                    if (getCategory(temp.data.type) == currentCategory && SDL_PointInRect(&p, &temp.rect)) {
                        VisualBlock newB = temp; newB.rect.x = p.x - 60; newB.rect.y = p.y - 20; newB.isDragging = true;
                        workspaceBlocks.push_back(newB); activeDragBlock = &workspaceBlocks.back();
                        dragOffsetX = p.x - activeDragBlock->rect.x; dragOffsetY = p.y - activeDragBlock->rect.y;
                    }
                }
                if (!activeDragBlock && !draggingCat) {
                    for (int i = (int)workspaceBlocks.size() - 1; i >= 0; i--) {
                        if (SDL_PointInRect(&p, &workspaceBlocks[i].rect)) {
                            if (p.x > workspaceBlocks[i].rect.x + 80) {
                                editingBlock = &workspaceBlocks[i];
                                editingBlock->isEditing = true;
                                editingBlock->editBuffer = "";
                                SDL_StartTextInput();
                            }
                            else {
                                activeDragBlock = &workspaceBlocks[i];
                                activeDragBlock->isDragging = true;
                                dragOffsetX = p.x - activeDragBlock->rect.x;
                                dragOffsetY = p.y - activeDragBlock->rect.y;
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
                    SDL_Point dropP = {e.button.x, e.button.y};
                    if (!SDL_PointInRect(&dropP, &workSpace)) {
                        for (auto it = workspaceBlocks.begin(); it != workspaceBlocks.end(); ++it) { if (&(*it) == activeDragBlock) { workspaceBlocks.erase(it); break; } }
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
            executeNext(manager, cat, meow, currentStep);
            //cat.checkBoundaries(1024, 450, 650);
            SDL_Delay(100);
        } else isRunning = false;

        SDL_SetRenderDrawColor(ren, 240, 240, 240, 255); SDL_RenderClear(ren);

        SDL_SetRenderDrawColor(ren, 255, 255, 255, 255); SDL_RenderFillRect(ren, &sidebarArea);
        SDL_SetRenderDrawColor(ren, 235, 235, 235, 255); SDL_RenderFillRect(ren, &workSpace);

        // Stage with Background logic
        SDL_SetRenderDrawColor(ren, 255, 255, 255, 255); SDL_RenderFillRect(ren, &stageArea);
        if (backgroundTexture) SDL_RenderCopy(ren, backgroundTexture, nullptr, &stageArea);

        SDL_SetRenderDrawColor(ren, 242, 243, 247, 255); SDL_RenderFillRect(ren, &spritePane);

        SDL_SetRenderDrawColor(ren, 200, 200, 200, 255);
        SDL_RenderDrawLine(ren, 230, 0, 230, 768); SDL_RenderDrawLine(ren, 650, 0, 650, 768);
        SDL_RenderDrawLine(ren, 650, 450, 1024, 450);

        // Input Boxes Rendering
        SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
        SDL_RenderFillRect(ren, &xBox); SDL_RenderFillRect(ren, &yBox); SDL_RenderFillRect(ren, &sBox); SDL_RenderFillRect(ren, &dBox);
        SDL_RenderFillRect(ren, &showBtn); SDL_RenderFillRect(ren, &hideBtn);

        SDL_SetRenderDrawColor(ren, 220, 220, 220, 255);
        SDL_RenderDrawRect(ren, &xBox); SDL_RenderDrawRect(ren, &yBox); SDL_RenderDrawRect(ren, &sBox); SDL_RenderDrawRect(ren, &dBox);
        SDL_RenderDrawRect(ren, &showBtn); SDL_RenderDrawRect(ren, &hideBtn);
        renderText(ren, font, (activeField == FIELD_X ? paneBuffer + "|" : std::to_string((int)cat.x - 837)), 695, 475, {0,0,0,255});
        renderText(ren, font, (activeField == FIELD_Y ? paneBuffer + "|" : std::to_string(225 - (int)cat.y)), 770, 475, {0,0,0,255});
        renderText(ren, font, "x", 675, 475, {150, 150, 150, 255});
        renderText(ren, font, "y", 750, 475, {150, 150, 150, 255});
        renderText(ren, font, "Size", 820, 475, {150, 150, 150, 255});
        renderText(ren, font, (activeField == FIELD_SIZE ? paneBuffer + "|" : std::to_string((int)cat.size)), 860, 475, {0,0,0,255});
        renderText(ren, font, "Dir", 925, 475, {150, 150, 150, 255});
        renderText(ren, font, (activeField == FIELD_DIR ? paneBuffer + "|" : std::to_string((int)cat.angle)), 960, 475, {0,0,0,255});

        renderText(ren, font, "Show", 665, 515, {150, 150, 150, 255});
        renderText(ren, font, "O", 722, 515, (cat.visible ? SDL_Color{0,150,255,255} : SDL_Color{100,100,100,255}));
        renderText(ren, font, "/", 762, 515, (!cat.visible ? SDL_Color{0,150,255,255} : SDL_Color{100,100,100,255}));

        for (int i = 0; i < (int)catUIs.size(); i++) {
            int cX = 40, cY = 50 + (i * 65);
            if (currentCategory == i) aacircleRGBA(ren, cX, cY, 23, 0, 0, 0, 100);
            filledCircleRGBA(ren, cX, cY, 20, catUIs[i].color.r, catUIs[i].color.g, catUIs[i].color.b, 255);
            renderText(ren, font, catUIs[i].name, cX, cY + 22, {80, 80, 80, 255}, true);
        }
        for (auto& b : sidebarTemplates) {
            if (getCategory(b.data.type) == currentCategory) {
                SDL_SetRenderDrawColor(ren, b.color.r, b.color.g, b.color.b, 255); SDL_RenderFillRect(ren, &b.rect);
                bool hasNum = (b.data.type == MOVE || b.data.type == TURN
                    || b.data.type == WAIT || b.data.type == REPEAT
                    || b.data.type == SET_VAR || b.data.type == CHANGE_VAR
                    || b.data.type == IF || b.data.type == CHANGE_X || b.data.type == SET_Y || b.data.type == CHANGE_Y
                    || b.data.type == SET_X|| b.data.type == SET_VOLUME|| b.data.type == SET_PITCH
                    || b.data.type == SET_SIZE || b.data.type == CHANGE_SIZE);
                std::string lbl = b.label + (hasNum && b.data.type != TOUCHING_EDGE ? std::to_string((int)b.data.value) : "");
                renderText(ren, font, lbl, b.rect.x + 10, b.rect.y + 10, {255, 255, 255, 255});
            }
        }
        for (auto& b : workspaceBlocks) {
            SDL_SetRenderDrawColor(ren, b.color.r, b.color.g, b.color.b, 255); SDL_RenderFillRect(ren, &b.rect);
            std::string t = b.label;
            bool hasNum = (b.data.type == MOVE || b.data.type == TURN
                || b.data.type == WAIT || b.data.type == REPEAT
                || b.data.type == SET_VAR || b.data.type == CHANGE_VAR
                || b.data.type == IF || b.data.type == CHANGE_X || b.data.type == SET_X
                || b.data.type == SET_Y || b.data.type == CHANGE_Y
                || b.data.type == SET_VOLUME|| b.data.type == SET_PITCH
                || b.data.type == SET_SIZE || b.data.type == CHANGE_SIZE);
            if (hasNum && b.data.value != 999) t += b.isEditing ? b.editBuffer + "|" : std::to_string((int)b.data.value);
            renderText(ren, font, t, b.rect.x + 10, b.rect.y + 10, {255, 255, 255, 255});
        }

        SDL_Rect runBtnRect = {95, 600, 120, 35}, saveBtnRect = {95, 645, 120, 35}, loadBtnRect = {95, 690, 120, 35};
        SDL_SetRenderDrawColor(ren, 50, 200, 50, 255); SDL_RenderFillRect(ren, &runBtnRect); renderText(ren, font, "GO / RUN", 155, 610, {255, 255, 255, 255}, true);
        SDL_SetRenderDrawColor(ren, 100, 100, 100, 255); SDL_RenderFillRect(ren, &saveBtnRect); renderText(ren, font, "SAVE", 155, 655, {255, 255, 255, 255}, true);
        SDL_RenderFillRect(ren, &loadBtnRect); renderText(ren, font, "LOAD", 155, 700, {255, 255, 255, 255}, true);

        SDL_Rect loadBgBtn = {880, 660, 120, 35};
        SDL_SetRenderDrawColor(ren, 255, 140, 26, 255); SDL_RenderFillRect(ren, &loadBgBtn);
        renderText(ren, font, "LOAD BACKGRND", 940, 670, {255, 255, 255, 255}, true);

        SDL_Rect loadImageBtn = {880, 700, 120, 35};
        SDL_SetRenderDrawColor(ren, 0, 150, 240, 255); SDL_RenderFillRect(ren, &loadImageBtn);
        renderText(ren, font, "LOAD COSTUME", 940, 710, {255, 255, 255, 255}, true);

        if (cat.visible) cat.draw(ren);
        SDL_RenderPresent(ren);
    }
    if (font) TTF_CloseFont(font);
    TTF_Quit();

    Mix_FreeChunk(meow.chunk);
    Mix_CloseAudio();
    SDL_Quit();
     return 0;
}