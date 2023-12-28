#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <SDL/SDL.h>

void pause();
void gradient(SDL_Surface *screen);
void gradient2(SDL_Surface *screen);

void do_rooms(void);
void render_rooms(SDL_Surface *screen);

void do_passages(void);
void render_passages(SDL_Surface *screen);

void render_maze(SDL_Surface *screen);
void init_level(void);

void draw_level(SDL_Surface *screen);


int main(int argc, char *argv[])
{
    SDL_Surface *screen = NULL, *rectangle = NULL;
    SDL_Rect position;

    SDL_Init(SDL_INIT_VIDEO);

    screen = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE);
    // surface allocation
    rectangle = SDL_CreateRGBSurface(SDL_HWSURFACE, 220, 180, 32, 0, 0, 0, 0);

    SDL_WM_SetCaption("Rogue Maze", NULL);

    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 17, 206, 112));

    position.x = 0; // surface coordinate will be (0, 0)
    position.y = 0;


    // surface filling with the white color
    SDL_FillRect(rectangle, NULL, SDL_MapRGB(screen->format, 255, 255, 255));
    SDL_BlitSurface(rectangle, NULL, screen, &position); // blit the surface on screen

    SDL_Flip(screen); // update screen

    gradient(screen);
    position.x = (640 / 2) - (220 / 2);
    position.y = (480 / 2) - (180 / 2);
    //SDL_BlitSurface(rectangle, NULL, screen, &position);
    draw_level(screen);
    SDL_Flip(screen);

    pause();

    SDL_FreeSurface(rectangle); // surface freeing
    SDL_Quit();

    return EXIT_SUCCESS;
}


void pause()
{
    int paused  = 1;
    SDL_Event event;

    while (paused)
    {
        SDL_WaitEvent(&event);
        switch(event.type)
        {
            case SDL_QUIT:
            paused = 0;
        }
    }
}

void gradient(SDL_Surface *screen)
{
    SDL_Rect positions[256];
    SDL_Surface *surfaces[256] = {NULL};
    int i;

    for(i=0; i<256; i++)
    {
        //fprintf(stderr, "error : i=%d surfaces=%p \n", i, surfaces[i]);
        surfaces[i] = SDL_CreateRGBSurface(SDL_SWSURFACE, 640, 1, 32, 0, 0, 0, 0);
        SDL_FillRect(surfaces[i], NULL, SDL_MapRGB(screen->format, i, i, i));
        positions[i].x=0;
        positions[i].y=i;
        SDL_BlitSurface(surfaces[i], NULL, screen, &positions[i]);
        SDL_FreeSurface(surfaces[i]);
    }

    SDL_Flip(screen);
}

void gradient2(SDL_Surface *screen)
{
    SDL_Rect position;
    SDL_Surface *surface=SDL_CreateRGBSurface(SDL_SWSURFACE, 640, 1, 32, 0, 0, 0, 0);
    int i;

    for(i=0; i<256; i++)
    {
        SDL_FillRect(surface, NULL, SDL_MapRGB(screen->format, i, i, i));
        position.x=0;
        position.y=i;
        SDL_BlitSurface(surface, NULL, screen, &position);
    }

    SDL_Flip(screen);
    SDL_FreeSurface(surface);

    if (0)
    {
        fprintf(stderr, "error :%s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
}

void draw_level(SDL_Surface *screen)
{
    srand(time(0));
    init_level();
    do_rooms();
    render_rooms(screen);
    do_passages();
    render_passages(screen);
    render_maze(screen);
}

