#include <unistd.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "SDL2/SDL2_gfxPrimitives.h"
#include "SDL2/SDL_pixels.h"


//Constants
#define PLANET_NUM 10
#define N_TRASH 1

//Type definitions
typedef struct {
    float amplitude;
    float angle;
} Vector;

typedef struct {
    float x;
    float y;
} Position_t;

typedef struct {
    int x;
    int y;
    float mass;
    char name;
} Planet_t;

typedef struct {
    Position_t position;
    Vector velocity;
    Vector acceleration;
    float mass;
} Trash_t;

//Function prototypes
void planets_init(Planet_t* planets, int num_planets);
SDL_Color random_color();
Uint32 SDL_ColorToUint(SDL_Color c);
void new_trash_acceleration(Planet_t planets[], int total_planets, Trash_t trash[], int total_trash);
void new_trash_velocity(Trash_t trash[], int total_trash);
void new_trash_position(Trash_t trash[], int total_trash);
Vector make_vector(float x, float y);
Vector add_vectors(Vector v1, Vector v2);
void correct_position(float *pos);

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

void new_trash_acceleration(Planet_t planets[], int total_planets,
                            Trash_t trash[], int total_trash) {
    Vector total_vector_force;
    Vector local_vector_force;
    for (int n_trash = 0; n_trash < N_TRASH; n_trash++) {
        total_vector_force.amplitude = 0;
        total_vector_force.angle = 0;
        for (int n_planet = 0; n_planet < PLANET_NUM; n_planet++) {
            // This assumes your Trash_t struct has x and y, but it has a Vector 'position'
            // You may need to convert the position vector to cartesian coordinates first.
            float force_vector_x = planets[n_planet].x - trash[n_trash].position.x;
            float force_vector_y = planets[n_planet].y - trash[n_trash].position.y;
            local_vector_force = make_vector(force_vector_x, force_vector_y);
            local_vector_force.amplitude = planets[n_planet].mass /
                                           pow(local_vector_force.amplitude, 2);
            total_vector_force = add_vectors(local_vector_force,
                                             total_vector_force);
            trash[n_trash].acceleration = total_vector_force;
        }
    }
}

void new_trash_velocity(Trash_t trash[], int total_trash) {
    for (int n_trash = 0; n_trash < total_trash; n_trash++) {
        trash[n_trash].velocity.amplitude *= 0.99;
        trash[n_trash].velocity = add_vectors(trash[n_trash].velocity,
                                              trash[n_trash].acceleration);
    }
}

void new_trash_position(Trash_t trash[], int total_trash) {
    for (int n_trash = 0; n_trash < total_trash; n_trash++) {
        // This assumes your Trash_t struct has x and y, but it has a Vector 'position'
        // You may need to update the position vector's components instead.
        trash[n_trash].position.x += trash[n_trash].velocity.amplitude * 2 *
                                   cos(trash[n_trash].velocity.angle);
        trash[n_trash].position.y += trash[n_trash].velocity.amplitude *
                                   sin(trash[n_trash].velocity.angle);
        correct_position(&trash[n_trash].position.x);
        correct_position(&trash[n_trash].position.y);
    }
}

Vector make_vector(float x, float y) {
    Vector v;
    v.amplitude = sqrt(x * x + y * y);
    v.angle = atan2(y, x);
    return v;
}

Vector add_vectors(Vector v1, Vector v2) {
    // Convert polar to cartesian
    float x1 = v1.amplitude * cos(v1.angle);
    float y1 = v1.amplitude * sin(v1.angle);
    float x2 = v2.amplitude * cos(v2.angle);
    float y2 = v2.amplitude * sin(v2.angle);

    // Add cartesian components
    float sum_x = x1 + x2;
    float sum_y = y1 + y2;

    // Convert back to polar and return
    return make_vector(sum_x, sum_y);
}

void correct_position(float *pos) {
    if (*pos < 0) {
        *pos = 1000 + *pos;
    } else if (*pos >= 1000) {
        *pos = *pos - 1000;
    }
}
