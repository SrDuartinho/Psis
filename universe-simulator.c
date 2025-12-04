#include <unistd.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "SDL2/SDL2_gfxPrimitives.h"
#include "SDL2/SDL_pixels.h"
#include "universe-data.h"
#include "gravitation.h"
#include "display.h"
#include <SDL2/SDL_ttf.h>


int main() {
    // To get a "random" seed, for the planets' position
    srand(time(NULL));
    init_recycle_index();
    
    // Initialize planets
    Planet_t planets[PLANET_NUM];
    //planets[0] = garbage collector planet; -> planets[0].is_garbage = 1;
    planets_init(planets, PLANET_NUM);

    // Initialize trash
    Trash_t trash[MAX_TRASH_WORLD];
    if(N_TRASH > MAX_TRASH_WORLD){
        printf("Error: N_TRASH exceeds maximum capacity of MAX_TRASH_WORLD.\n");
        return 1;
    }
    trash_init(trash, N_TRASH);
    int trash_counter = N_TRASH;

    // Initialize display
    SDL_Window* win = disp_init();
    SDL_Renderer* rend = rend_init(win);
    SDL_RenderPresent(rend);

    // initialize TTF and font for labels
    if (TTF_Init() != 0) {
        printf("TTF init error: %s\n", TTF_GetError());
        return 1;
    }
    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 14);
    if (!font) {
        printf("Font error: %s\n", TTF_GetError());
        // continue without labels
        font = NULL;
    }

    SDL_Color planet_color = {80, 80, 186, 255};
    SDL_Color garbage_planet_color = {20, 20, 186, 255};
    SDL_Color trash_color = {128, 128, 0, 255};

    //Arrays for collision detection, and frame counting, so that undesired trash generation is avoided
    
    //int trash_collided[MAX_TRASH_WORLD];
    //int frame_counter[MAX_TRASH_WORLD];
    /*
    for(int i = 0; i < MAX_TRASH_WORLD; i++){
        trash_collided[i] = 0;
        frame_counter[i] = 0;
    }
    */

    int ret;
    //Main loop
    int close = 0;
    
    while(close == 0){

        SDL_Event event;
        while(SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT){
                close = 1;
            }
        }
                
        for(int i = 0; i < trash_counter; i++){
            for(int j = 0; j < PLANET_NUM; j++){
                if(trash_planet_collision(&trash[i],&planets[j]) == 1 /*&& trash_collided[i] == 0*/){
                    ret = generate_new_trash(trash, trash_counter, planets[j].x, planets[j].y);
                    if (ret == -1) {
                        printf("Ending game.\n");
                        end_game(rend, win);
                        close = 1;
                    }
                    else{
                        //trash_collided[i] = 1;
                        trash_counter++;
                        printf("Trash counter incremented! Current trash count: %d\n", trash_counter);
                    }
                    
                }
            }   
        }

        new_trash_acceleration(planets, PLANET_NUM, trash, trash_counter);
        new_trash_velocity(trash, trash_counter);
        new_trash_position(trash, trash_counter);


        //10 frames delay for collision flag reset
        //To avoid multiple collisions being detected for the same trash object
        /*for(int i = 0; i < trash_counter; i++){
            if(trash_collided[i] == 1){
                frame_counter[i]++;
                if(frame_counter[i] >= 10){
                    trash_collided[i] = 0;
                    frame_counter[i] = 0;
                }
            }
        }*/
        

        SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
        SDL_RenderClear(rend);
        planet_drawer(planets, PLANET_NUM, rend, planet_color, garbage_planet_color, font);
        trash_drawer(trash, trash_counter, rend, trash_color);
        SDL_RenderPresent(rend);
        SDL_Delay(10);

    }

    if (font) {
        TTF_CloseFont(font);
        TTF_Quit();
    }
    disp_close(rend, win);
    return 0;
}
