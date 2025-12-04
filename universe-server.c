#include <unistd.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "communication.h"
#include "universe-data.h"
#include "gravitation.h"
#include "display.h"
#include "SDL2/SDL2_gfxPrimitives.h"
#include "SDL2/SDL_pixels.h"


//Function prototypes
void planets_init(Planet_t* planets, int num_planets);
SDL_Color random_color();
Uint32 SDL_ColorToUint(SDL_Color c);


direction_t random_direction() {
    return rand() % 4;
}

void new_position(int* x, int* y, direction_t direction) {
    switch (direction) {
        case UP:
            (*x) = (*x)-5;
            if (*x < 0) {
                *x = WINDOW_SIZE + *x; 
            }
            break;
        case DOWN:
            (*x)= (*x)+5;
            if (*x > WINDOW_SIZE-1) {
                *x = *x - WINDOW_SIZE; 
            }
            break;
        case LEFT:
            (*y)= (*y)-5;
            if (*y < 0) {
                *y = WINDOW_SIZE + *y; 
            }
            break;
        case RIGHT:
            (*y)= (*y)+5;
            if (*y > WINDOW_SIZE-1) {
                *y = *y - WINDOW_SIZE; 
            }
            break;
    }
}

int find_ch_info(Ship arr[], int n, char ch) {
    for (int i = 0; i < n; i++)
        if (arr[i].ch == ch)
            return i;
    return -1;
}

void remove_trash(Trash_t trash[], int *n_trash, int index) {
    if (index < 0 || index >= *n_trash) return;

    // shift all elements left
    for (int j = index; j < *n_trash - 1; j++) {
        trash[j] = trash[j + 1];
    }

    (*n_trash)--;  // one less element
}

void transfer_trash_to_planet(Ship* ship, Planet_t* planet) {
    // move each trash item from ship to planet
    for (int i = 0; i < ship->trash_count; i++) {

        // Do not overflow the planet storage
        if (planet->trash_count < N_TRASH) {
            planet->trash[planet->trash_count] = ship->trash[i];
            planet->trash_count++;
        }
    }

    // Ship loses all trash
    ship->trash_count = 0;
}

void scatter_trash(Ship* ship, Trash_t trash[], int* n_trash, int window_size)
{
    for (int i = 0; i < ship->trash_count; i++) {

        // don't overflow global trash array
        if (*n_trash >= N_TRASH)
            break;

        // copy the trash
        trash[*n_trash] = ship->trash[i];

        // give it a NEW RANDOM POSITION
        trash[*n_trash].position.x = rand() % window_size;
        trash[*n_trash].position.y = rand() % window_size;

        (*n_trash)++;
    }

    // all trash thrown out -> ship is empty
    ship->trash_count = 0;
}




void draw_char(SDL_Renderer* r, TTF_Font* font, char c, int x, int y, SDL_Color ship_color, int trash_count) {
    SDL_Color color = { 0, 0, 0, 255 };

    char text[2] = {c, 0};
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(r, surface);

    SDL_Rect dest;
    dest.x = y;
    dest.y = x;
    dest.w = surface->w;
    dest.h = surface->h;
    filledCircleColor(r, dest.x + 10, dest.y +20, 20, SDL_ColorToUint(ship_color));
    SDL_FreeSurface(surface);
    SDL_RenderCopy(r, texture, NULL, &dest);
    SDL_DestroyTexture(texture);

    // render trash count below the ship
    char count_text[16];
    snprintf(count_text, sizeof(count_text), "%d", trash_count);
    SDL_Surface* cnt_surf = TTF_RenderText_Solid(font, count_text, color);
    if (cnt_surf) {
        SDL_Texture* cnt_tex = SDL_CreateTextureFromSurface(r, cnt_surf);
        if (cnt_tex) {
            SDL_Rect cnt_dst;
            cnt_dst.w = cnt_surf->w;
            cnt_dst.h = cnt_surf->h;
            // center under the ship circle
            int center_x = dest.x + 10;
            int center_y = dest.y + 20;
            cnt_dst.x = center_x - cnt_dst.w / 2;
            cnt_dst.y = center_y + 20 + 2; // circle radius + small gap
            SDL_RenderCopy(r, cnt_tex, NULL, &cnt_dst);
            SDL_DestroyTexture(cnt_tex);
        }
        SDL_FreeSurface(cnt_surf);
    }
}

int main() {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL init error: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() != 0) {
        printf("TTF init error: %s\n", TTF_GetError());
        return 1;
    }
    // To get a "random" seed, for the planets' position
    srand(time(NULL));

    init_recycle_index();

    // Initialize planets
    Planet_t planets[PLANET_NUM];
    planets_init(planets, PLANET_NUM);

    // Initialize trash
    Trash_t trash[N_TRASH];
    trash_init(trash, N_TRASH);
    int n_trash = N_TRASH;   // start full or whatever number you want
    int aux_trash_recycle = 0; //auxiliar variable for printing information
    int aux_trash_spill = 0; //auxiliar variable for printing information

    // Initialize display
    SDL_Window* win = disp_init();
    SDL_Renderer* rend = rend_init(win);
    SDL_RenderPresent(rend);   
    
    SDL_Color planet_color = {80, 80, 186, 255};
    SDL_Color garbage_planet_color = {20, 186, 20, 255};
    SDL_Color trash_color = {128, 128, 0, 255};
    SDL_Color ship_color = {186, 80, 80, 100};

    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 32);
    if (!font) {
        printf("Font error: %s\n", TTF_GetError());
        return 1;
    }

    void* fd = create_server_channel();

    Ship char_data[100];
    int n_chars = 0;

    char message_type[100];
    char c;
    direction_t d;

    int close = 0;
    while(close == 0){
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT){
                close = 1;
            }
        }
        
        planet_drawer(planets, PLANET_NUM, rend, planet_color, garbage_planet_color, font);   
        trash_drawer(trash, n_trash, rend, trash_color);

        read_message(fd, message_type, &c, &d);

        if (strcmp(message_type, "CONNECT") == 0) {

            int pos = find_ch_info(char_data, n_chars, c);

            if (pos == -1) {
                send_response(fd, "OK");
            } else {
                send_response(fd, "NOT OK");
                continue;
            }

            char_data[n_chars].ch = c;
            char_data[n_chars].position.x = WINDOW_SIZE / 2;
            char_data[n_chars].position.y = WINDOW_SIZE / 2;
            char_data[n_chars].trash_count = 0;
            n_chars++;

        } else if (strcmp(message_type, "MOVE") == 0) {

            int pos = find_ch_info(char_data, n_chars, c);
            if (pos != -1) {

                int x = char_data[pos].position.x;
                int y = char_data[pos].position.y;

                new_position(&x, &y, d);

                char_data[pos].position.x = x;
                char_data[pos].position.y = y;
                
                //printf("%d %d\n", x, y);
                
                send_response(fd, "OK");
            }
        }

        

        // Draw all characters
        for (int i = 0; i < n_chars; i++)
            draw_char(rend, font,
                      char_data[i].ch,
                      char_data[i].position.x,
                      char_data[i].position.y, ship_color, char_data[i].trash_count);
        // Trash interaction
        for (int i = 0; i < n_chars; i++){
            for (int j = 0; j < n_trash; j++){
                //printf("%f %f\n", trash[j].position.y, trash[j].position.x);
                //fflush(stdout);
                float dx = (char_data[i].position.x + 20) - trash[j].position.y;
                float dy = (char_data[i].position.y + 10) - trash[j].position.x;
                float distance = sqrt(dx * dx + dy * dy);
                if (distance <= 20.0f) {  // within radius of 20
                // store trash in ship
                if (char_data[i].trash_count < N_TRASH) {
                    char_data[i].trash[char_data[i].trash_count] = trash[j];
                    char_data[i].trash_count++;
                }

                remove_trash(trash, &n_trash, j);
                j--;  // adjust index since we removed an element
                aux_trash_spill = n_trash;
                aux_trash_recycle = N_TRASH - n_trash;
                printf("Amount of trash in client %c: %d\n", char_data[i].ch, char_data[i].trash_count);
                fflush(stdout);
            }
            }
        }

        // Planet interaction
        for (int i = 0; i < n_chars; i++){
            //printf("%d", char_data[i].trash);
            for (int j = 0; j < PLANET_NUM; j++){
                //printf("%d %d\n", planets[j].y, planets[j].x);
                //fflush(stdout);
                float dx = (char_data[i].position.x + 20) - planets[j].y;
                float dy = (char_data[i].position.y + 10) - planets[j].x;
                float distance = sqrt(dx * dx + dy * dy);
                if (distance <= 20.0f) {  // within radius of 20
                    if (j == RECYCLE_PLANET_INDEX) {
                        // move trash from ship to planet
                        transfer_trash_to_planet(&char_data[i], &planets[j]);
                        
                        if (aux_trash_recycle == planets[j].trash_count){
                            printf("Ship %c delivered trash to planet %c! Planet now has %d pieces.\n",
                                char_data[i].ch, planets[j].name, planets[j].trash_count);
                            aux_trash_recycle = 0;
                        }
                    } else {
                        // ship crashes into other planet, scatter trash                       
                        scatter_trash(&char_data[i], trash, &n_trash, WINDOW_SIZE);

                        if (aux_trash_spill != n_trash){
                            printf("Ship %c crashed into %c and scattered its trash!\n",
                                char_data[i].ch, planets[j].name);
                            aux_trash_spill = n_trash;    
                        }
                    }
                }
            }
        }
        SDL_RenderPresent(rend);

        SDL_Delay(10);
    }

    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();
    return 0;
}