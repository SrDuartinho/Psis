#include "display.h"

SDL_Window* disp_init(){
    // initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
    }
    
    SDL_Window* win = SDL_CreateWindow("UNIVERSE SIMULATOR",
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED,
                                       1000, 1000, 0);

    return win;                                
}

SDL_Renderer* rend_init(SDL_Window* win){
    Uint32 render_flags = SDL_RENDERER_ACCELERATED;
    SDL_Renderer* rend = SDL_CreateRenderer(win, -1, render_flags);
    return rend;
}

void planet_drawer(Planet_t* planets, int planets_num, SDL_Renderer* rend, SDL_Color planet_color, SDL_Color garbage_planet_color){    
    SDL_Color backgroud_color = {255, 255, 255, 255};
    
    SDL_SetRenderDrawColor(rend, 
            backgroud_color.r, backgroud_color.g, backgroud_color.b, 
            backgroud_color.a);

    SDL_RenderClear(rend);

    for (int i = 0; i < planets_num; i++) {
        // Draw garbage collector planet differently
        if(planets[i].is_garbage == 1){
            filledCircleColor(rend, planets[i].x, planets[i].y, 30, SDL_ColorToUint(garbage_planet_color));
        }
        else{
            filledCircleColor(rend, planets[i].x, planets[i].y, 20, SDL_ColorToUint(planet_color));              
        }
    }
}

void trash_drawer(Trash_t* trash, int trash_num, SDL_Renderer* rend, SDL_Color trash_color){
        for(int i = 0; i < trash_num; i++) {
            filledCircleColor(rend, trash[i].position.x, trash[i].position.y,
                                4, SDL_ColorToUint(trash_color));
        }
        SDL_RenderPresent(rend);
        //10ms delay to simulate time unit
        SDL_Delay(10);
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




