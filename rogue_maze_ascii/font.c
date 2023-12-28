#include <SDL/SDL.h>
#include <SDL_image.h>
#include "font.h"
#include "util.h"

SDL_Surface *font = NULL;

void load_font(void)
{
    if(font != NULL)
        SDL_FreeSurface(font);
    //font = IMG_Load("rogue_font_tileset/Nagidal24x24.png");
    font = IMG_Load("rogue_font_tileset/CGA16x16thick.png");
    //font = IMG_Load("rogue_font_tileset/CGA8x8thick.png");
    convert_surface(&font);
    SDL_SetColorKey(font, SDL_SRCCOLORKEY, getpixel32(font, 0, 0));
}

void free_font(void)
{
    SDL_FreeSurface(font);
}

void color_font(Uint32 color)
{
    Uint32 background_color = getpixel32(font, 0, 0);
    int x, y;
    for(y = 0; y < font->h; y++)
    {
        for(x = 0; x < font->w; x++)
        {
            if(getpixel32(font, x, y) != background_color)
            {
                setPixel32(font, x, y, color);
            }
        }
    }
}

void print_char(SDL_Surface *surface, int x, int y, unsigned char c)
{
    SDL_Rect image_pos = {x, y};
    SDL_Rect image_size = {0, 0, CHAR_WIDTH, CHAR_HEIGHT};

    image_size.x = ((c - CHAR_OFFSET) % CHAR_COLUMNS) * CHAR_WIDTH;
    image_size.y = ((c - CHAR_OFFSET) / CHAR_COLUMNS) * CHAR_HEIGHT;
    SDL_BlitSurface(font, &image_size, surface, &image_pos);
    //blit_colored(font, &image_size, surface, &image_pos, 0XFFFFFF);
    //blit_rotated(font, &image_size, surface, &image_pos, 30);
}

void print_string(SDL_Surface *surface, int x, int y, char *s)
{
    SDL_Rect image_pos = {x, y};
    SDL_Rect image_size = {0, 0, CHAR_WIDTH, CHAR_HEIGHT};
    int i; char c;
    for (i = 0; i < strlen(s); i++)
    {
        c = s[i];
        image_size.x = ((c - CHAR_OFFSET) % CHAR_COLUMNS) * CHAR_WIDTH;
        image_size.y = ((c - CHAR_OFFSET) / CHAR_COLUMNS) * CHAR_HEIGHT;
        SDL_BlitSurface(font, &image_size, surface, &image_pos);
        image_pos.x += CHAR_WIDTH;
    }
}

void print_char_colored(SDL_Surface *surface, int x, int y, unsigned char c, Uint32 color)
{
    SDL_Rect image_pos = {x, y};
    SDL_Rect image_size = {0, 0, CHAR_WIDTH, CHAR_HEIGHT};

    image_size.x = ((c - CHAR_OFFSET) % CHAR_COLUMNS) * CHAR_WIDTH;
    image_size.y = ((c - CHAR_OFFSET) / CHAR_COLUMNS) * CHAR_HEIGHT;
    blit_colored(font, &image_size, surface, &image_pos, color);
}

void print_string_colored(SDL_Surface *surface, int x, int y, char *s, Uint32 color)
{
    SDL_Rect image_pos = {x, y};
    SDL_Rect image_size = {0, 0, CHAR_WIDTH, CHAR_HEIGHT};
    int i; char c;
    for (i = 0; i < strlen(s); i++)
    {
        c = s[i];
        image_size.x = ((c - CHAR_OFFSET) % CHAR_COLUMNS) * CHAR_WIDTH;
        image_size.y = ((c - CHAR_OFFSET) / CHAR_COLUMNS) * CHAR_HEIGHT;
        image_size.w = CHAR_WIDTH;
        image_size.h = CHAR_HEIGHT;
        image_pos.x = x + (CHAR_WIDTH * i);
        image_pos.y = y;
        blit_colored(font, &image_size, surface, &image_pos, color);
    }
}


