#include <unistd.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include "SDL2/SDL2_gfxPrimitives.h"
#include "SDL2/SDL_pixels.h"

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
SDL_Color random_color();

int main() {

    // Initialize planets
    Planet_t planets[10];
    planets_init(planets, 10);

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
    for(int i = 0; i < 10; i++) {
        object_color = random_color();
        filledCircleColor(rend, planets[i].x, planets[i].y, 20, 
                                SDL_ColorToUint(object_color));
    }
    

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
}

SDL_Color random_color(){
    SDL_Color color;
    color.r = rand() % 256;
    color.g = rand() % 256;
    color.b = rand() % 256;
    color.a = rand() % 255;
    return color;
}
