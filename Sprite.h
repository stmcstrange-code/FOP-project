#ifndef FOP_PROJECT_SPRITE_H
#define FOP_PROJECT_SPRITE_H

#include <SDL2/SDL.h>
#include <vector>
#include <string>

struct Point {
    float x, y;
};

struct Sprite{
    float x = 400.0f, y = 300.0f;
    float angle = 0.0f;
    bool penDown = false;
    std::vector<Point> trail;
    SDL_Texture* texture = nullptr;
    int w = 50, h = 50;

    void draw(SDL_Renderer* ren);
    void move(float steps);
    void rotate(float deg);
    void clearPen();
    void clearTrail();

    void checkBoundaries(int screenW, int screenH, int sidebarW);

    bool loadBMP(SDL_Renderer* ren, const std::string& path);


    bool isClicked(int mx, int my) {
        int dW = (w > 100) ? 60 : w;
        int dH = (h > 100) ? 60 : h;
        SDL_Rect rect = { (int)x - dW/2, (int)y - dH/2, dW, dH };
        SDL_Point p = {mx, my};
        return SDL_PointInRect(&p, &rect);
    }
};

#endif