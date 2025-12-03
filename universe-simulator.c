#include <unistd.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "SDL2/SDL2_gfxPrimitives.h"
#include "SDL2/SDL_pixels.h"
#include "universe-data.h"
#include "gravitation.h"
#include "display.h"


int main() {

    // Initialize planets
    Planet_t planets[PLANET_NUM];
    //planets[0] = garbage collector planet; -> planets[0].is_garbage = 1;
    planets_init(planets, PLANET_NUM);

    // Initialize trash
    Trash_t trash[N_TRASH];
    trash_init(trash, N_TRASH);

    // Initialize display
    SDL_Window* win = disp_init();
    SDL_Renderer* rend = rend_init(win);
    SDL_RenderPresent(rend);   
    
    SDL_Color planet_color = {80, 80, 186, 255};
    SDL_Color garbage_planet_color = {20, 20, 186, 255};
    SDL_Color trash_color = {128, 128, 0, 255};

    //Main loop
    int close = 0;
    while(close == 0){

        SDL_Event event;
        while(SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT){
                close = 1;
            }
        }
        
        planet_drawer(planets, PLANET_NUM, rend, planet_color, garbage_planet_color);   
        trash_drawer(trash, N_TRASH, rend, trash_color);
        
        new_trash_acceleration(planets, PLANET_NUM, trash, N_TRASH);
        new_trash_velocity(trash, N_TRASH);
        new_trash_position(trash, N_TRASH);
    }

    disp_close(rend, win);
    return 0;
}