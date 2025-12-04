<<<<<<< HEAD
#include <ncurses.h>
#include <ctype.h> 
#include <string.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "SDL2/SDL2_gfxPrimitives.h"
#include "SDL2/SDL_pixels.h"
#include "communication.h"
#include "universe-data.h"

void initialize_screen(){
	initscr();			/* Start curses mode 		*/
	cbreak();				/* Line buffering disabled	*/
	keypad(stdscr, TRUE);		/* We get F1, F2 etc..		*/
	noecho();			/* Don't echo() while we do getch */
}



int main(int argc,  char** argv){
    if (argc ==1){
//        exit(-1);
    }
//    void * fd = create_client_channel(argv[1]);
    void * fd = create_client_channel("127.0.0.0");

    char ch;
    do{
        printf("what is your character(a..z)?: ");
        ch = getchar();
        ch = tolower(ch);  
    }while(!isalpha(ch));

    send_connection_message(fd, ch);
    char message[100];
    receive_response (fd, message);
    if (strcmp(message, "NOT OK") ==0){
        exit(-1);
    }

    initialize_screen();
    int n = 0;
    int key;
    direction_t direction;

    //SDL init 
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
    }

    SDL_Window* win = SDL_CreateWindow("DIRECTION SELECTED",
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED,
                                       400, 400 , 0);

    Uint32 render_flags = SDL_RENDERER_ACCELERATED;
    SDL_Renderer* rend = SDL_CreateRenderer(win, -1, render_flags);
    SDL_Color backgroud_color;
    backgroud_color.r = 255;
    backgroud_color.g = 255;
    backgroud_color.b = 255;
    backgroud_color.a = 255;

    SDL_Surface* up_surface = IMG_Load("icons/up.png");
    SDL_Surface* down_surface = IMG_Load("icons/down.png");
    SDL_Surface* left_surface = IMG_Load("icons/left.png");
    SDL_Surface* right_surface = IMG_Load("icons/right.png");

    if (up_surface == NULL || down_surface == NULL ||
        left_surface == NULL || right_surface == NULL) {
        printf("Error loading image: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Texture* up_texture = SDL_CreateTextureFromSurface(rend, up_surface);
    SDL_Texture* down_texture = SDL_CreateTextureFromSurface(rend, down_surface);
    SDL_Texture* left_texture = SDL_CreateTextureFromSurface(rend, left_surface);
    SDL_Texture* right_texture = SDL_CreateTextureFromSurface(rend, right_surface);

    SDL_FreeSurface(up_surface);
    SDL_FreeSurface(down_surface);
    SDL_FreeSurface(left_surface);
    SDL_FreeSurface(right_surface);

    SDL_Rect dst;
    dst.w = 128; // Set the width of the image
    dst.h = 128; // Set the height of the image
    dst.x = (400 - dst.w) / 2; // Center horizontally
    dst.y = (400 - dst.h) / 2; // Center vertically

    do
    {
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT){
                key = 27;
            }
        }

        if (key == 27){
            break;
        }
        else {
    	    key = getch();		
        }

        SDL_SetRenderDrawColor(rend, 
            backgroud_color.r, backgroud_color.g, backgroud_color.b, 
            backgroud_color.a);
        SDL_RenderClear(rend);

        n++;
        switch (key)
        {
        case KEY_LEFT:
            mvprintw(0,0,"%d Left arrow is pressed", n);
            direction = LEFT;
            SDL_RenderCopy(rend, left_texture, NULL, &dst);
            break;
        case KEY_RIGHT:
            mvprintw(0,0,"%d Right arrow is pressed", n);
            direction = RIGHT;
            SDL_RenderCopy(rend, right_texture, NULL, &dst);
            break;
        case KEY_DOWN:
            mvprintw(0,0,"%d Down arrow is pressed", n);
            direction = DOWN;
            SDL_RenderCopy(rend, down_texture, NULL, &dst);
            break;
        case KEY_UP:
            mvprintw(0,0,"%d :Up arrow is pressed", n);
            direction = UP;
            SDL_RenderCopy(rend, up_texture, NULL, &dst);   
            break;

        default:
            key = 'x'; 
            break;
        }
        if (key != 'x'){
            send_movement_message(fd, ch, direction);
            char message[100];
            receive_response (fd, message);
            if (strcmp(message, "WALL") ==0){
                mvprintw(1,0,"You hit a wall!            ");
                break;
            }
        }
        
        refresh();			/* Print it on to the real screen */
        
        SDL_RenderPresent(rend);
        SDL_Delay(10);
        
    }while(key != 27);
    
    // destroy renderer
    SDL_DestroyRenderer(rend);

    // destroy window
    SDL_DestroyWindow(win);
    
    // close SDL
    SDL_Quit();
    
  	endwin();			/* End curses mode		  */

	return 0;
=======
#include <stdio.h>

int main() {
    printf("Universe Client Running\n");
    return 0;
>>>>>>> 84d48c9db7be704eacb687906b681cc480a8e123
}