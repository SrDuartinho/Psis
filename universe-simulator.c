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


//Function prototypes
void planets_init(Planet_t* planets, int num_planets);
SDL_Color random_color();
Uint32 SDL_ColorToUint(SDL_Color c);


int main() {

    // To get a "random" seed, for the planets' position
    srand(time(NULL));

    // Initialize planets
    Planet_t planets[PLANET_NUM];
    planets_init(planets, PLANET_NUM);

    // Initialize trash
    Trash_t trash[N_TRASH];
    for (int i = 0; i < N_TRASH; i++) {
        trash[i].position.x = rand() % 1000;
        trash[i].position.y = rand() % 1000;
        trash[i].velocity.amplitude = 0;
        trash[i].velocity.angle = 0;
        trash[i].mass = 1.0;
    }

     // initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
    }

    SDL_Window* win = SDL_CreateWindow("UNIVERSE SIMULATOR",
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED,
                                       1000, 1000, 0);

    Uint32 render_flags = SDL_RENDERER_ACCELERATED;
    SDL_Renderer* rend = SDL_CreateRenderer(win, -1, render_flags);
    SDL_Color backgroud_color;
    backgroud_color.r = 255;
    backgroud_color.g = 255;
    backgroud_color.b = 255;
    backgroud_color.a = 255;

    SDL_Color planet_color;
    planet_color.r = 80;
    planet_color.g = 80; 
    planet_color.b = 186;
    planet_color.a = 255;

    SDL_Color trash_color;
    trash_color.r = 128;
    trash_color.g = 128; 
    trash_color.b = 0;
    trash_color.a = 255;

    //Draw initial trash
    SDL_RenderPresent(rend);

    
    //Main loop
    int close = 0;
    while(close == 0){
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT){
                close = 1;
            }
        }
    
        SDL_SetRenderDrawColor(rend, 
            backgroud_color.r, backgroud_color.g, backgroud_color.b, 
            backgroud_color.a);
        SDL_RenderClear(rend);

        for (int i = 0; i < PLANET_NUM; i++) {
            filledCircleColor(rend, planets[i].x, planets[i].y, 20, 
                                SDL_ColorToUint(planet_color));
        }

        for(int i = 0; i < N_TRASH; i++) {
            filledCircleColor(rend, trash[i].position.x, trash[i].position.y,
                                4, SDL_ColorToUint(trash_color));
        }
        
        new_trash_acceleration(planets, PLANET_NUM, trash, N_TRASH);
        new_trash_velocity(trash, N_TRASH);
        new_trash_position(trash, N_TRASH);

        SDL_RenderPresent(rend);
        //10ms delay to simulate time unit
        SDL_Delay(10);
    }

     // destroy renderer
    SDL_DestroyRenderer(rend);

    // destroy window
    SDL_DestroyWindow(win);
    
    // close SDL
    SDL_Quit();

    return 0;
}

void planets_init(Planet_t* planets, int num_planets) {
     for (int i = 0; i < num_planets; i++) {
        planets[i].x = rand() % 1000;
        planets[i].y = rand() % 1000;
        planets[i].mass = 10.0;
        planets[i].name = 'A' + i;
    }

     for (int i = 0; i < num_planets; i++) {
        for (int j = i + 1; j < num_planets; j++) {
            // Check if planet i and j have the same coordinates
            if (planets[i].x == planets[j].x && planets[i].y == planets[j].y) {
                // If they do, regenerate new coordinates.
                planets[j].x = rand() % 1000;
                planets[j].y = rand() % 1000;
                
                // Restart the check for the modified planet j from the beginning
                i = 0; 
                break;
            }
        }
    }
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
