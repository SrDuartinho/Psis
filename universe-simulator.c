#include <unistd.h>
#include <SDL2/SDL.h>
#include "SDL2/SDL2_gfxPrimitives.h"
#include "SDL2/SDL_pixels.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//Constants
#define PLANET_NUM 10

//Type definitions
typedef struct {
    float x;
    float y;
} Vector2D;

typedef struct {
    int x;
    int y;
    float mass;
    char name;
} Planet_t;

//Function prototypes
void planets_init(Planet_t* planets, int num_planets);
Uint32 SDL_ColorToUint(SDL_Color c);
SDL_Color random_color();

int main() {

    // To get a "random" seed, for the planets' position
    srand(time(NULL));
    // Initialize planets
    Planet_t planets[PLANET_NUM];
    planets_init(planets, PLANET_NUM);
    
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
    SDL_Color object_color;
    object_color.r = 12;
    object_color.g = 12; 
    object_color.b = 128;
    object_color.a = 255;

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
        sleep(1);

        for (int i = 0; i < PLANET_NUM; i++) {
            filledCircleColor(rend, planets[i].x, planets[i].y, 5, 
                                SDL_ColorToUint(object_color));
        }

        SDL_RenderPresent(rend);
        //10ms delay to simulate time unit
        SDL_Delay(1000);
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
}

Uint32 SDL_ColorToUint(SDL_Color c){
	return (Uint32)((c.a << 24) + (c.b << 16) + (c.g << 8)+ (c.r << 0));
}

SDL_Color random_color(){
    SDL_Color color;
    color.r = rand() % 256;
    color.g = rand() % 256;
    color.b = rand() % 256;
    color.a = rand() % 255;
    return color;
}
