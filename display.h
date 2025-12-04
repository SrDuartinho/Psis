#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "SDL2/SDL2_gfxPrimitives.h"
#include "SDL2/SDL_pixels.h"
#include "universe-data.h"

#ifndef DISPLAY_H
#define DISPLAY_H

SDL_Window* disp_init();
SDL_Renderer* rend_init(SDL_Window* win);
void disp_close(SDL_Renderer* rend, SDL_Window* win);
SDL_Color random_color();
Uint32 SDL_ColorToUint(SDL_Color c);
void planet_drawer(Planet_t* planets, int planets_num, SDL_Renderer* rend, SDL_Color planet_color, SDL_Color garbage_planet_color, TTF_Font* font);
void trash_drawer(Trash_t* trash, int trash_num, SDL_Renderer* rend, SDL_Color trash_color);

#endif