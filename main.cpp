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



void renderText(SDL_Renderer* ren, TTF_Font* font, std::string text, int x, int y, SDL_Color color) {
    if (!font) return;
    SDL_Surface* surf = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);
    SDL_Rect dst = {x, y, surf->w, surf->h};
    SDL_RenderCopy(ren, tex, nullptr, &dst);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window* win = SDL_CreateWindow("Scratch Pro - Drag & Drop", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920, 1080, 0);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    TTF_Font* font = TTF_OpenFont("assets/arial.ttf", 16);
    if (!font) std::cout << "Warning: Font not found. Text will not display." << std::endl;


    std::vector<VisualBlock> sidebarTemplates = {
        {{MOVE, 40, 0}, {95, 60, 120, 40}, {76, 151, 255, 255}, "MOVE 40"},
        {{TURN, 15, 0}, {95, 110, 120, 40}, {76, 151, 255, 255}, "TURN 15"},
        {{PEN_DOWN, 0, 0}, {95, 160, 120, 40}, {64, 184, 158, 255}, "PEN DOWN"},
        {{PEN_UP, 0, 0}, {95, 210, 120, 40}, {64, 184, 158, 255}, "PEN UP"},
        {{ERASE, 0, 0}, {95, 260, 120, 40}, {255, 102, 102, 255}, "ERASE"},
        {{REPEAT, 4, 0}, {95, 310, 120, 40}, {255, 171, 25, 255}, "REPEAT 4"},
        {{END_LOOP, 0, 0}, {95, 360, 120, 40}, {255, 171, 25, 255}, "END LOOP"}
    };

    std::vector<VisualBlock> workspaceBlocks;
    VisualBlock* activeDragBlock = nullptr;
    int dragOffsetX = 0, dragOffsetY = 0;

    Sprite cat;
     cat.loadBMP(ren, "assets/cat.bmp");
    ProgramManager manager;
    int currentStep = 0;
    bool isRunning = false, quit = false;

    while (!quit) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = true;

            if (e.type == SDL_MOUSEBUTTONDOWN) {
                SDL_Point p = {e.button.x, e.button.y};


                for (auto& temp : sidebarTemplates) {
                    if (SDL_PointInRect(&p, &temp.rect)) {
                        VisualBlock newB = temp;
                        newB.isDragging = true;
                        workspaceBlocks.push_back(newB);
                        activeDragBlock = &workspaceBlocks.back();
                        dragOffsetX = p.x - newB.rect.x;
                        dragOffsetY = p.y - newB.rect.y;
                    }
                }


                if (!activeDragBlock) {
                    for (int i = workspaceBlocks.size() - 1; i >= 0; i--) {
                        if (SDL_PointInRect(&p, &workspaceBlocks[i].rect)) {
                            activeDragBlock = &workspaceBlocks[i];
                            activeDragBlock->isDragging = true;
                            dragOffsetX = p.x - activeDragBlock->rect.x;
                            dragOffsetY = p.y - activeDragBlock->rect.y;
                            break;
                        }
                    }
                }


                SDL_Rect runBtn = {95, 600, 120, 40};
                if (SDL_PointInRect(&p, &runBtn)) {
                    manager.script.clear();

                    std::sort(workspaceBlocks.begin(), workspaceBlocks.end(), [](const VisualBlock& a, const VisualBlock& b) {
                        return a.rect.y < b.rect.y;
                    });
                    for (auto& vb : workspaceBlocks) manager.script.push_back(vb.data);
                    isRunning = true;
                    currentStep = 0;
                }
            }

            if (e.type == SDL_MOUSEMOTION && activeDragBlock) {
                activeDragBlock->rect.x = e.motion.x - dragOffsetX;
                activeDragBlock->rect.y = e.motion.y - dragOffsetY;
            }

            if (e.type == SDL_MOUSEBUTTONUP && activeDragBlock) {

                if (activeDragBlock->rect.x < 150) {
                    for (auto it = workspaceBlocks.begin(); it != workspaceBlocks.end(); ++it) {
                        if (&(*it) == activeDragBlock) {
                            workspaceBlocks.erase(it);
                            break;
                        }
                    }
                }
                else {

                    for (auto& other : workspaceBlocks) {
                        if (&other == activeDragBlock) continue;
                        if (abs(activeDragBlock->rect.x - other.rect.x) < 30 &&
                            abs(activeDragBlock->rect.y - (other.rect.y + 40)) < 25) {
                            activeDragBlock->rect.x = other.rect.x;
                            activeDragBlock->rect.y = other.rect.y + 40;
                            break;
                            }
                    }
                    activeDragBlock->isDragging = false;
                }
                activeDragBlock = nullptr;
            }
        }


        if (isRunning && currentStep < (int)manager.script.size()) {
            executeNext(manager, cat, currentStep);
            cat.checkBoundaries(800, 768, 150);
            SDL_Delay(150);
        } else isRunning = false;


        SDL_SetRenderDrawColor(ren, 240, 240, 240, 255);
        SDL_RenderClear(ren);


        SDL_Rect sideBarRect = {80, 0, 150, 1080};
        SDL_SetRenderDrawColor(ren, 200, 200, 200, 255);
        SDL_RenderFillRect(ren, &sideBarRect);

        SDL_Rect stageRect = {800, 0, 224, 1080};
        SDL_SetRenderDrawColor(ren, 230, 230, 230, 255);
        SDL_RenderFillRect(ren, &stageRect);


        filledEllipseRGBA(ren, 40, 60, 15, 15, 60, 131, 253, 255);
        filledEllipseRGBA(ren, 40, 120, 15, 15, 176, 60, 254, 255);
        filledEllipseRGBA(ren, 40, 180, 15, 15, 251, 60, 250, 255);
        filledEllipseRGBA(ren, 40, 240, 15, 15, 255, 192, 0, 255);
        filledEllipseRGBA(ren, 40, 300, 15, 15, 255, 153, 0, 255);
        filledEllipseRGBA(ren, 40, 360, 15, 15, 0, 220, 255, 255);
        filledEllipseRGBA(ren, 40, 420, 15, 15, 28, 216, 8, 255);
        filledEllipseRGBA(ren, 40, 480, 15, 15, 255, 114, 0, 255);
        filledEllipseRGBA(ren, 40, 540, 15, 15, 255, 62, 142, 255);

        renderText(ren, font, "Motion", 20, 80, {0, 0, 0, 255});
        renderText(ren, font, "Looks", 20, 140, {0, 0, 0, 255});
        renderText(ren, font, "Sound", 17, 200, {0, 0, 0, 255});
        renderText(ren, font, "Events", 16, 260, {0, 0, 0, 255});
        renderText(ren, font, "Control", 13, 320, {0, 0, 0, 255});
        renderText(ren, font, "Sensing", 12, 380, {0, 0, 0, 255});
        renderText(ren, font, "Operators", 10, 440, {0, 0, 0, 255});
        renderText(ren, font, "Variables", 10, 500, {0, 0, 0, 255});
        renderText(ren, font, "My Blocks", 7, 560, {0, 0, 0, 255});



        for (auto& b : sidebarTemplates) {
            SDL_SetRenderDrawColor(ren, b.color.r, b.color.g, b.color.b, 255);
            SDL_RenderFillRect(ren, &b.rect);
            renderText(ren, font, b.label, b.rect.x + 10, b.rect.y + 10, {255, 255, 255, 255});
        }


        for (auto& b : workspaceBlocks) {
            SDL_SetRenderDrawColor(ren, b.color.r, b.color.g, b.color.b, 255);
            SDL_RenderFillRect(ren, &b.rect);
            SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
            SDL_RenderDrawRect(ren, &b.rect); // Outline
            renderText(ren, font, b.label, b.rect.x + 10, b.rect.y + 10, {255, 255, 255, 255});
        }


        SDL_Rect runBtnRect = {95, 600, 120, 40};
        SDL_SetRenderDrawColor(ren, 50, 200, 50, 255);
        SDL_RenderFillRect(ren, &runBtnRect);
        renderText(ren, font, "GO / RUN", 115, 610, {255, 255, 255, 255});

        cat.draw(ren);
        SDL_RenderPresent(ren);
    }

    if(font) TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();
    return 0;
}