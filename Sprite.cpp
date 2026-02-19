#include "Sprite.h"
#include <iostream>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

bool Sprite::loadBMP(SDL_Renderer* ren, const std::string& path) {
    SDL_Surface* surface = SDL_LoadBMP(path.c_str());
    if (!surface) return false;
    SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 255, 0, 255));
    texture = SDL_CreateTextureFromSurface(ren, surface);
    if (surface->w > 0) w = surface->w;
    if (surface->h > 0) h = surface->h;
    SDL_FreeSurface(surface);
    return texture != nullptr;
}

void Sprite::move(float steps) {
    float rad = angle * (M_PI / 180.0f);
    float nextX = x + std::cos(rad) * steps;
    float nextY = y + std::sin(rad) * steps;
    if (penDown) {
        trail.push_back({x, y});
        trail.push_back({nextX, nextY});
    }
    x = nextX; y = nextY;
}

void Sprite::rotate(float deg) { angle += deg; }
void Sprite::clearTrail() { trail.clear(); }
void Sprite::clearPen() { trail.clear(); }

void Sprite::checkBoundaries(int screenW, int screenH, int sidebarW) {
    int halfW = (w > 100) ? 30 : w / 2;
    int halfH = (h > 100) ? 30 : h / 2;

    // Bounce off Right/Left
    if (x + halfW > screenW) { x = screenW - halfW; angle = 180.0f - angle; }
    else if (x - halfW < sidebarW) { x = sidebarW + halfW; angle = 180.0f - angle; }

    // Bounce off Top/Bottom
    if (y + halfH > screenH) { y = screenH - halfH; angle = -angle; }
    else if (y - halfH < 0) { y = halfH; angle = -angle; }
}

void Sprite::draw(SDL_Renderer* ren) {
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    for (size_t i = 0; i + 1 < trail.size(); i += 2) {
        SDL_RenderDrawLine(ren, (int)trail[i].x, (int)trail[i].y, (int)trail[i+1].x, (int)trail[i+1].y);
    }
    if (texture) {
        int dW = (w > 100) ? 60 : w;
        int dH = (h > 100) ? 60 : h;
        SDL_Rect dest = { (int)x - dW/2, (int)y - dH/2, dW, dH };
        SDL_RenderCopyEx(ren, texture, nullptr, &dest, (double)angle, nullptr, SDL_FLIP_NONE);
    }
}