#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <SDL/SDL.h>
#include "rogue_maze.h"

void pause(void);
void gradient(SDL_Surface *screen);
void gradient2(SDL_Surface *screen);
void sleep(void);

void do_rooms(void);
void render_rooms(SDL_Surface *screen);

void do_passages(void);
void render_passages(SDL_Surface *screen);

void render_maze(SDL_Surface *screen);
void init_level(void);

void build_level(void);
void draw_level(SDL_Surface *screen);

void gl_test(void);
void gl_init(void);
void gl_flip(void);


int main(int argc, char *argv[])
{
    SDL_Surface *screen = NULL, *rectangle = NULL;
    SDL_Rect position;
    SDL_Event event;

    SDL_Init(SDL_INIT_VIDEO);

    screen = SDL_SetVideoMode(640, 480, 32,SDL_OPENGL);
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
    //draw_level(screen);
    SDL_Flip(screen);//gl_test();

    //pause();

    int main_loop = 1;

    SDL_EnableKeyRepeat(400, 30);

    initFreeFlyCamera(&freeFlyCamera);
    initTrackballCamera(&trackBallCamera);printf("c=%c\n",getchar());
    gl_init_textures();
    gl_init_vertex_array();//gl_vertex_array_test();pause();

    build_level();gl_build_vertex_array(); draw_level(screen);


    while (main_loop)
    {
        /* We use PollEvent a(nd none
        WaitEvent pour to not block the program */
        if(SDL_PollEvent(&event) == 1)
        {
            switch(event.type)
            {
                case SDL_QUIT:
                    main_loop = 0;
                    break;
                // keyboard events
                case SDL_KEYDOWN:
                    switch(event.key.keysym.sym)
                    {
                        case SDLK_ESCAPE:
                            main_loop = 0;
                            break;
                        default:
                            FreeFlyCameraOnKeyboard(&freeFlyCamera, &event.key);
                            TrackBallCameraOnKeyboard(&trackBallCamera, &event.key);
                            //FreeFlyCameraAnimate(&freeFlyCamera, 1);
                            //draw_level(screen);
                    }
                    break;
                case SDL_KEYUP:
                    FreeFlyCameraOnKeyboard(&freeFlyCamera, &event.key);
                    break;
                case SDL_MOUSEMOTION:
                    //FreeFlyCameraOnMouseMotion(&freeFlyCamera, &event.motion);
                    TrackBallCameraOnMouseMotion(&trackBallCamera, &event.motion);
                    break;
                case SDL_MOUSEBUTTONUP:
                case SDL_MOUSEBUTTONDOWN:
                    FreeFlyCameraOnMouseButton(&freeFlyCamera, &event.button);
                    TrackBallCameraOnMouseButton(&trackBallCamera, & event.button);
                    break;
            }
        }
        FreeFlyCameraAnimate(&freeFlyCamera, 10);
        draw_level(screen);
        sleep();
    }

    gl_free_level();
    SDL_FreeSurface(rectangle); // surface freeing
    SDL_Quit();

    return EXIT_SUCCESS;
}


void pause(void)
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

void sleep(void)
{
    static int old_time = 0,  actual_time = 0;
    actual_time = SDL_GetTicks();
    if (actual_time - old_time < 16) // if less than 16 ms has passed
    {
        SDL_Delay(16 - (actual_time - old_time));
        old_time = SDL_GetTicks();
    }
    else
    {
        old_time = actual_time;
    }
}

void build_level(void)
{
    srand(time(0));
    init_level();
    do_rooms();
    do_passages();
}


void draw_level(SDL_Surface *screen)
{
    //srand(time(0));
    //init_level();
    //do_rooms();
    //do_passages();
    gl_init();
    gl_draw_level_surface();
    gl_draw_level();
    //gl_draw_level_surface();
    /*render_passages(screen);
    render_rooms(screen);
    render_maze(screen);*/
    gl_flip();
}


