#include "Sprite.h"
#include <iostream>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Initializing default values for the Sprite
Sprite::Sprite() {
    x = 837.0f;
    y = 225.0f;
    w = 60;
    h = 60;
    size = 100.0f;
    angle = 0.0f;
    visible = true;
    penDown = false;
    texture = nullptr;
}

bool Sprite::loadBMP(SDL_Renderer* ren, const std::string& path) {
    SDL_Surface* surface = SDL_LoadBMP(path.c_str());
    if (!surface) return false;

    // Standard magenta transparency
    SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 255, 0, 255));

    if (texture) SDL_DestroyTexture(texture);
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

    x = nextX;
    y = nextY;
}

void Sprite::rotate(float deg) {
    angle += deg;
}

void Sprite::clearTrail() {
    trail.clear();
}

void Sprite::clearPen() {
    trail.clear();
}

void Sprite::checkBoundaries(int screenW, int screenH, int sidebarW) {
    // Determine the current visual size of the cat
    int dW = (int)(w * (size / 100.0f));
    int dH = (int)(h * (size / 100.0f));

    // Safety cap for large dimensions
    if (w > 100 && size == 100.0f) { dW = 60; dH = 60; }

    int halfW = dW / 2;
    int halfH = dH / 2;

    // Static Boundary Logic
    if (x + halfW > screenW)  x = (float)(screenW - halfW);
    if (x - halfW < sidebarW) x = (float)(sidebarW + halfW);
    if (y + halfH > screenH)  y = (float)(screenH - halfH);
    if (y - halfH < 0)        y = (float)halfH;
}

void Sprite::draw(SDL_Renderer* ren) {

    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    for (size_t i = 0; i + 1 < trail.size(); i += 2) {
        SDL_RenderDrawLine(ren, (int)trail[i].x, (int)trail[i].y, (int)trail[i+1].x, (int)trail[i+1].y);
    }


    if (texture && visible) {

        float baseW = 100.0f;
        float baseH = 100.0f;
        int dW = (int)(baseW * (size / 100.0f));
        int dH = (int)(baseH * (size / 100.0f));

        SDL_Rect dest = { (int)x - dW/2, (int)y - dH/2, dW, dH };
        SDL_RenderCopyEx(ren, texture, nullptr, &dest, (double)angle, nullptr, SDL_FLIP_NONE);
    }
}