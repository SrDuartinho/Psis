#include "display.h"
#include <time.h>
#include <SDL2/SDL_ttf.h>


SDL_Window* disp_init(){
    // initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
    }
    
    SDL_Window* win = SDL_CreateWindow("UNIVERSE SIMULATOR",
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED,
                                       WINDOW_SIZE, WINDOW_SIZE, 0);

    return win;                                
}

SDL_Renderer* rend_init(SDL_Window* win){
    Uint32 render_flags = SDL_RENDERER_ACCELERATED;
    SDL_Renderer* rend = SDL_CreateRenderer(win, -1, render_flags);
    return rend;
}

void planet_drawer(Planet_t* planets, int planets_num, SDL_Renderer* rend, SDL_Color planet_color, SDL_Color garbage_planet_color, TTF_Font* font){    
    SDL_Color backgroud_color = {255, 255, 255, 255};
    
    SDL_SetRenderDrawColor(rend, 
            backgroud_color.r, backgroud_color.g, backgroud_color.b, 
            backgroud_color.a);

    SDL_RenderClear(rend);

    for (int i = 0; i < planets_num; i++) {
        // Draw garbage collector planet differently
        if(planets[i].is_garbage == 1){
            filledCircleColor(rend, planets[i].x, planets[i].y, 30, 
                                SDL_ColorToUint(garbage_planet_color));
        }
        else{
            filledCircleColor(rend, planets[i].x, planets[i].y, 20, 
                                SDL_ColorToUint(planet_color));             
        }
        // Render label: "<name>:<trash_count>" below the planet
        if (font != NULL) {
            char label[32];
            snprintf(label, sizeof(label), "%c:%d", planets[i].name, planets[i].trash_count);
            SDL_Color text_color = {0, 0, 0, 255};
            SDL_Surface* surf = TTF_RenderText_Solid(font, label, text_color);
            if (surf) {
                SDL_Texture* tex = SDL_CreateTextureFromSurface(rend, surf);
                if (tex) {
                    SDL_Rect dst;
                    dst.w = surf->w;
                    dst.h = surf->h;
                    // center text horizontally at planet.x, place below planet
                    dst.x = planets[i].x - dst.w / 2;
                    dst.y = planets[i].y + 20 + 2; // planet radius (20) + small gap
                    SDL_RenderCopy(rend, tex, NULL, &dst);
                    SDL_DestroyTexture(tex);
                }
                SDL_FreeSurface(surf);
            }
        }
    }
}

void trash_drawer(Trash_t* trash, int trash_num, SDL_Renderer* rend, SDL_Color trash_color){
        for(int i = 0; i < trash_num; i++) {
            filledCircleColor(rend, trash[i].position.x, trash[i].position.y,
                                4, SDL_ColorToUint(trash_color));
        }
}

void end_game(SDL_Renderer* rend, SDL_Window* win){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Game Over", "Maximum trash capacity reached. Ending game.", NULL);
    SDL_Delay(2000); // wait for 2 seconds before closing
    disp_close(rend, win);
    exit(0);
}

void disp_close(SDL_Renderer* rend, SDL_Window* win){
    // destroy renderer
    SDL_DestroyRenderer(rend);

    // destroy window
    SDL_DestroyWindow(win);
    
    // close SDL
    SDL_Quit();
}

SDL_Color random_color(){
    SDL_Color color;
    color.r = rand() % 256;
    color.g = rand() % 256;
    color.b = rand() % 256;
    color.a = rand() % 255;
    return color;
}

Uint32 SDL_ColorToUint(SDL_Color c){
	return (Uint32)((c.a << 24) + (c.b << 16) + (c.g << 8)+ (c.r << 0));
}