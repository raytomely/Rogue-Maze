#include <dirent.h>
#include <SDL/SDL.h>
#include <math.h>
#include "util.h"

#define RMASK 0XFF0000
#define GMASK 0XFF00
#define BMASK 0XFF

#define RSHIFT 16
#define GSHIFT 8
#define BSHIFT 0

#define UNPACK_RGB(r, g, b, rgb) r = (rgb & RMASK) >> RSHIFT; g = (rgb & GMASK) >> GSHIFT; b = (rgb & BMASK) >> BSHIFT
#define PACK_RGB(r, g, b) (r << RSHIFT) | (g << GSHIFT) | (b << BSHIFT)

int listdir(void)
{
  DIR *dp;
  struct dirent *ep;
  dp = opendir ("./");
  if (dp != NULL)
  {
    while ((ep = readdir (dp)) != NULL)
      puts (ep->d_name);

    (void) closedir (dp);
    return 0;
  }
  else
  {
    perror ("Couldn't open the directory");
    return -1;
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

SDL_Surface *create_surface32(int width, int height)
{
    SDL_Surface *surface;
    Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif
    surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, rmask, gmask, bmask, amask);
    convert_surface(&surface);
    return surface;
}

SDL_Surface *copy_surface(SDL_Surface *surface)
{
    return SDL_ConvertSurface(surface, surface->format, SDL_SWSURFACE);
}

void setPixel32(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    if (x >= surface->w || y >= surface->h || x < 0 || y < 0)
        return;
    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
    *(Uint32 *)p = pixel;
}

Uint32 getpixel32(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
    return *(Uint32 *)p;
}

void convert_surface(SDL_Surface **surface)
{
    SDL_Surface *temp_surf = &(**surface);
    *surface = SDL_DisplayFormat (temp_surf);
    SDL_FreeSurface(temp_surf);
}

void convert_surface_alpha(SDL_Surface **surface)
{
    SDL_Surface *temp_surf = &(**surface);
    *surface = SDL_DisplayFormatAlpha(temp_surf);
    SDL_FreeSurface(temp_surf);
}

int get_file_size(FILE* file)
{
    fseek (file, 0, SEEK_END);
    int file_size = ftell(file);
    fseek (file, 0, SEEK_SET);
    return file_size;
}

void blit_colored(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color)
{
    int x, y, x2, bpp;
    Uint32 pixel, alpha_pixel = src->format->colorkey;

    // first do trivial rejections of bitmap, is it totally invisible?
    if ((dstrect->x >= dst->w) || (dstrect->y >= dst->h) ||
       ((dstrect->x + srcrect->w) <= 0) || ((dstrect->y + srcrect->h) <= 0))
            return;

    // clip rectangles
    // upper left hand corner first
    if (dstrect->x < 0)
    {
        srcrect->x -= dstrect->x;
        srcrect->w += dstrect->x;
        dstrect->x = 0;
    }

    if (dstrect->y < 0)
    {
        srcrect->y -= dstrect->y;
        srcrect->h += dstrect->y;
        dstrect->y = 0;
    }

    // now lower left hand corner
    if (dstrect->x + srcrect->w > dst->w)
    {
        //srcrect->x -= dstrect->x;
        srcrect->w = dst->w - dstrect->x;
        dstrect->w = dst->w;
    }

    if (dstrect->y + srcrect->h > dst->h)
    {
        srcrect->h = dst->h - dstrect->y;
        dstrect->h = dst->h;
    }

    // compute starting address in dst surface
    bpp = dst->format->BytesPerPixel;
    Uint32 *dst_buffer = (Uint32 *)((Uint8 *)dst->pixels + dstrect->y * dst->pitch + dstrect->x * bpp);

    // compute starting address in src surface to scan data from
    bpp = src->format->BytesPerPixel;
    Uint32 *src_bitmap = (Uint32 *)((Uint8 *)src->pixels + srcrect->y * src->pitch + srcrect->x * bpp);

    for(y = 0; y < srcrect->h; y++)
    {
        x2 = 0;
        for(x = 0; x < srcrect->w; x++)
        {
            pixel = src_bitmap[x];
            if(pixel != alpha_pixel)
                dst_buffer[x2] = color;
            x2++;
        }
        dst_buffer += dst->w;
        src_bitmap += src->w;
    }
}

void blit_blended(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect, Uint8 alpha)
{
    int x, y, bpp;
    Uint32 pixel, alpha_pixel = src->format->colorkey;
    Uint8 r1, g1, b1, r2, g2, b2;
    float alpha_ratio1 = (float) alpha / 0XFF;
    float alpha_ratio2 = 1 - alpha_ratio1;

    // first do trivial rejections of bitmap, is it totally invisible?
    if ((dstrect->x >= dst->w) || (dstrect->y >= dst->h) ||
       ((dstrect->x + srcrect->w) <= 0) || ((dstrect->y + srcrect->h) <= 0))
            return;

    // clip rectangles
    // upper left hand corner first
    if (dstrect->x < 0)
    {
        srcrect->x -= dstrect->x;
        srcrect->w += dstrect->x;
        dstrect->x = 0;
    }

    if (dstrect->y < 0)
    {
        srcrect->y -= dstrect->y;
        srcrect->h += dstrect->y;
        dstrect->y = 0;
    }

    // now lower left hand corner
    if (dstrect->x + srcrect->w > dst->w)
    {
        //srcrect->x -= dstrect->x;
        srcrect->w = dst->w - dstrect->x;
        dstrect->w = dst->w;
    }

    if (dstrect->y + srcrect->h > dst->h)
    {
        srcrect->h = dst->h - dstrect->y;
        dstrect->h = dst->h;
    }

    // compute starting address in dst surface
    bpp = dst->format->BytesPerPixel;
    Uint32 *dst_buffer = (Uint32 *)((Uint8 *)dst->pixels + dstrect->y * dst->pitch + dstrect->x * bpp);

    // compute starting address in src surface to scan data from
    bpp = src->format->BytesPerPixel;
    Uint32 *src_bitmap = (Uint32 *)((Uint8 *)src->pixels + srcrect->y * src->pitch + srcrect->x * bpp);

    for(y = 0; y < srcrect->h; y++)
    {
        for(x = 0; x < srcrect->w; x++)
        {
            pixel = src_bitmap[x];
            if(pixel != alpha_pixel)
            {
                UNPACK_RGB(r1, g1, b1, src_bitmap[x]);
                UNPACK_RGB(r2, g2, b2, dst_buffer[x]);
                r1 *= alpha_ratio1; g1 *= alpha_ratio1; b1 *= alpha_ratio1;
                r2 *= alpha_ratio2; g2 *= alpha_ratio2; b2 *= alpha_ratio2;
                dst_buffer[x] = PACK_RGB((r1 + r2), (g1 + g2), (b1 + b2));
            }
        }
        dst_buffer += dst->w;
        src_bitmap += src->w;
    }
}

#define PI 3.1415927
#define RADIANS(deg) deg * PI / 180
#define DEGREES(rad) rad * 180 / PI

void rotate_pixel(SDL_Surface *dst, Uint32 pixel, float px, float py, float cx, float cy, float angle)
{
    //x = x*cos(a) - y*sin(a)
    //y = x*sin(a) + y*cos(a)
    //printf("old px=%f py=%f \n",px, py);
    px -= cx;
    py -= cy;
    //printf("new px=%d py=%d \n",px, py);
    float x = px;
    float y = py;
    //printf("x=%f y=%f \n",x, y);
    px = x*cos(RADIANS(angle))-y*sin(RADIANS(angle));
    py = x*sin(RADIANS(angle))+y*cos(RADIANS(angle));
    px += cx;
    py += cy;
    //printf("new px=%f py=%f \n",px, py);
    setPixel32(dst, px, py, pixel);
}

void rotate_pixel_shears(SDL_Surface *dst, Uint32 pixel, float px, float py, float cx, float cy, float angle)
{
    //Rotation By The Three shears matrices:
    //First Shear: up and down raster columns shift
    //x = x
    //y = x*(-tan(a/2)) + y
    //Second Shear: left to right raster rows shift
    //x = x + y*sin(a)
    //y = y
    //Third Shear: another up and down raster columns shift
    //x = x
    //y = x*(-tan(a/2)) + y

    px -= cx;
    py -= cy;
    float x = px;
    float y = py;
    y = x*(-tan(RADIANS(angle/2))) + y;
    x = x + y*sin(RADIANS(angle));
    y = x*(-tan(RADIANS(angle/2))) + y;
    px = x;
    py = y;
    px += cx;
    py += cy;
    setPixel32(dst, px, py, pixel);
}
Uint32 GetSmoothColor(float xOriginal, float yOriginal, SDL_Surface *OriginalBitmap);
void blit_rotated_sincos(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect, float angle)
{
    int x, y, x2, bpp;
    Uint32 pixel, alpha_pixel = src->format->colorkey;

    // first do trivial rejections of bitmap, is it totally invisible?
    if ((dstrect->x >= dst->w) || (dstrect->y >= dst->h) ||
       ((dstrect->x + srcrect->w) <= 0) || ((dstrect->y + srcrect->h) <= 0))
            return;

    // clip rectangles
    // upper left hand corner first
    if (dstrect->x < 0)
    {
        srcrect->x -= dstrect->x;
        srcrect->w += dstrect->x;
        dstrect->x = 0;
    }

    if (dstrect->y < 0)
    {
        srcrect->y -= dstrect->y;
        srcrect->h += dstrect->y;
        dstrect->y = 0;
    }

    // now lower left hand corner
    if (dstrect->x + srcrect->w > dst->w)
    {
        //srcrect->x -= dstrect->x;
        srcrect->w = dst->w - dstrect->x;
        dstrect->w = dst->w;
    }

    if (dstrect->y + srcrect->h > dst->h)
    {
        srcrect->h = dst->h - dstrect->y;
        dstrect->h = dst->h;
    }

    // compute starting address in dst surface
    bpp = dst->format->BytesPerPixel;
    Uint32 *dst_buffer = (Uint32 *)((Uint8 *)dst->pixels + dstrect->y * dst->pitch + dstrect->x * bpp);

    // compute starting address in src surface to scan data from
    bpp = src->format->BytesPerPixel;
    Uint32 *src_bitmap = (Uint32 *)((Uint8 *)src->pixels + srcrect->y * src->pitch + srcrect->x * bpp);

    float cx = (float)(dstrect->x + (srcrect->w / 2));
    float cy=  (float)(dstrect->y + (srcrect->h / 2));

    for(y = 0; y < srcrect->h; y++)
    {
        x2 = 0;
        for(x = 0; x < srcrect->w; x++)
        {
            pixel = src_bitmap[x];
            if(pixel != alpha_pixel)
            {
                //dst_buffer[x2] = pixel;
                rotate_pixel_shears(dst, pixel, x+dstrect->x, y+dstrect->y, cx, cy, angle);
                //setPixel32(dst, x+dstrect->x, y+dstrect->y, pixel);
            }
            x2++;
        }
        dst_buffer += dst->w;
        src_bitmap += src->w;
    }
}

void DDA(float x1, float y1, float x2, float y2, Uint32 color, SDL_Surface *dst)
{
    // Digital Differential Analyzer Algorithm
    float dx = x2 - x1;
    float dy = y2 - y1;
    float steps;
    if(abs(dx) > abs(dy))
       steps = abs(dx);
    else
       steps = abs(dy);
    float Xincrement = dx / steps;
    float Yincrement = dy / steps;
    float x=x1;
    float y=y1;
    int i;
    for(i = 0; i < steps; i++)
    {
        setPixel32(dst, x, y, color);
        x = x + Xincrement;
        y = y + Yincrement;
    }
}

void DDA2(int x1, int y1, int x2, int y2, Uint32 color, SDL_Surface *dst)
{
    // Digital Differential Analyzer Algorithm with integers only
    int dx = x2 - x1;
    int dy = y2 - y1;
    int x_inc, y_inc;
    if (dx < 0) { x_inc = -1; dx = -dx; } else { x_inc = 1; }
    if (dy < 0) { y_inc = -1; dy = -dy; } else { y_inc = 1; }
    //if (dy < 0) { y_inc = -dst->pitch >> 2; dy = -dy; } else { y_inc = dst->pitch >> 2; }
    //Uint32 *vb_start = (Uint32 *)((Uint8 *)dst->pixels + y1 * dst->pitch + x1 * dst->format->BytesPerPixel);
    int x = x1;
    int y = y1;
    int i;
    int fraction = 0;
    if(dx > dy)
    {
        fraction = -dx;
        for(i = 0; i < dx; i++)
        {
            setPixel32(dst, x, y, color);
            //*vb_start = color;
            x += x_inc;
            //vb_start += x_inc;
            fraction += dy;
            //if(fraction >= dx)
            if(fraction >= 0)
            {
                y += y_inc;
                //vb_start += y_inc;
                fraction -= dx;
            }
        }
    }
    else
    {
        fraction = -dy;
        for(i = 0; i < dy; i++)
        {
            setPixel32(dst, x, y, color);
            //*vb_start = color;
            y += y_inc;
            //vb_start += y_inc;
            fraction += dx;
            //if(fraction >= dy)
            if(fraction >= 0)
            {
                x += x_inc;
                //vb_start += x_inc;
                fraction -= dy;
            }
        }
    }
}

int Draw_Line(int x0, int y0, int x1, int y1, Uint32 color, SDL_Surface *dst)
{
    // this function draws a line from xo,yo to x1,y1 using differential error
    // terms (based on Bresenahams work)

    int dx,             // difference in x's
        dy,             // difference in y's
        dx2,            // dx,dy * 2
        dy2,
        x_inc,          // amount in pixel space to move during drawing
        y_inc,          // amount in pixel space to move during drawing
        error,          // the discriminant i.e. error i.e. decision variable
        index;          // used for looping

    // pre-compute first pixel address in video buffer
    int bpp = dst->format->BytesPerPixel;
    Uint32 *vb_start = (Uint32 *)((Uint8 *)dst->pixels + y0 * dst->pitch + x0 * bpp);

    // compute horizontal and vertical deltas
    dx = x1-x0;
    dy = y1-y0;

    // test which direction the line is going in i.e. slope angle
    if (dx>=0)
    {
        x_inc = 1;

    } // end if line is moving right
    else
    {
        x_inc = -1;
        dx    = -dx;  // need absolute value

    } // end else moving left

    // test y component of slope

    if (dy>=0)
    {
        y_inc = dst->pitch>>2;
    } // end if line is moving down
    else
    {
        y_inc = -dst->pitch>>2;
        dy    = -dy;  // need absolute value

    } // end else moving up

    // compute (dx,dy) * 2
    dx2 = dx << 1;
    dy2 = dy << 1;

    // now based on which delta is greater we can draw the line
    if (dx > dy)
    {
        // initialize error term
        error = dy2 - dx;

        // draw the line
        for (index=0; index <= dx; index++)
        {
            // set the pixel
            *vb_start = color;

            // test if error has overflowed
            if (error >= 0)
            {
                error-=dx2;

                // move to next line
                vb_start+=y_inc;

            } // end if error overflowed

            // adjust the error term
            error+=dy2;

            // move to the next pixel
            vb_start+=x_inc;

        } // end for

    } // end if |slope| <= 1
    else
    {
        // initialize error term
        error = dx2 - dy;

        // draw the line
        for (index=0; index <= dy; index++)
        {
            // set the pixel
            *vb_start = color;

            // test if error overflowed
            if (error >= 0)
            {
                error-=dy2;

                // move to next line
                vb_start+=x_inc;

            } // end if error overflowed

            // adjust the error term
            error+=dx2;

            // move to the next pixel
            vb_start+=y_inc;

        } // end for

    } // end else |slope| > 1

    // return success
    return(1);

} // end Draw_Line


void rotate_point(float *px, float *py, float cx, float cy, float angle)
{
    *px -= cx;
    *py -= cy;
    float x = *px;
    float y = *py;
    *px = x*cos(RADIANS(angle))-y*-sin(RADIANS(angle));
    *py = x*-sin(RADIANS(angle))+y*cos(RADIANS(angle));
    *px += cx;
    *py += cy;
}

void blit_rotated_rotozoom(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect, float angle)
{
    int x, y, bpp;
    Uint32 pixel, alpha_pixel = src->format->colorkey;

    // first do trivial rejections of bitmap, is it totally invisible?
    if ((dstrect->x >= dst->w) || (dstrect->y >= dst->h) ||
       ((dstrect->x + srcrect->w) <= 0) || ((dstrect->y + srcrect->h) <= 0))
            return;

    // clip rectangles
    // upper left hand corner first
    if (dstrect->x < 0)
    {
        srcrect->x -= dstrect->x;
        srcrect->w += dstrect->x;
        dstrect->x = 0;
    }

    if (dstrect->y < 0)
    {
        srcrect->y -= dstrect->y;
        srcrect->h += dstrect->y;
        dstrect->y = 0;
    }

    // now lower left hand corner
    if (dstrect->x + srcrect->w > dst->w)
    {
        //srcrect->x -= dstrect->x;
        srcrect->w = dst->w - dstrect->x;
        dstrect->w = dst->w;
    }

    if (dstrect->y + srcrect->h > dst->h)
    {
        srcrect->h = dst->h - dstrect->y;
        dstrect->h = dst->h;
    }

    // compute starting address in dst surface
    bpp = dst->format->BytesPerPixel;
    Uint32 *dst_buffer = (Uint32 *)((Uint8 *)dst->pixels + dstrect->y * dst->pitch + dstrect->x * bpp);

    // compute starting address in src surface to scan data from
    bpp = src->format->BytesPerPixel;
    Uint32 *src_bitmap = (Uint32 *)((Uint8 *)src->pixels + srcrect->y * src->pitch + srcrect->x * bpp);

    float x_increment = cos(RADIANS(angle));
    float y_increment = -sin(RADIANS(angle));
    float x_start = dstrect->x;
    float y_start = dstrect->y;
    float cx = (float)(dstrect->x + (srcrect->w / 2));
    float cy = (float)(dstrect->y + (srcrect->h / 2));
    rotate_point(&x_start, &y_start, cx, cy, angle);
    float px;
    float py;
    //DDA(dst, 100, 100, 400, 400);

    for(y = 0; y < srcrect->h; y++)
    {
        px = x_start;
        py = y_start;
        for(x = 0; x < srcrect->w; x++)
        {
            pixel = src_bitmap[x];
            if(pixel != alpha_pixel)
            {
                setPixel32(dst, px, py, pixel);
                //printf("px=%f py=%f dpx=%d dpy=%d \n",px,py,(int)px,(int)py);
            }
            px += x_increment; py += y_increment;
        }
        dst_buffer += dst->w;
        src_bitmap += src->w;
        x_start += -y_increment;
        y_start += x_increment;
    }
}

void rotate_scanline(SDL_Surface *dst, Uint32 *src_bitmap, Uint32 alpha_pixel, int x0, int y0, int x1, int y1)
{
    int dx, dy, dx2, dy2, x_inc, y_inc, error, index;
    int px = 0; Uint32 pixel;

    int bpp = dst->format->BytesPerPixel;
    Uint32 *vb_start = (Uint32 *)((Uint8 *)dst->pixels + y0 * dst->pitch + x0 * bpp);

    dx = x1-x0;
    dy = y1-y0;

    if (dx>=0)
    {
        x_inc = 1;

    }
    else
    {
        x_inc = -1;
        dx    = -dx;
    }

    if (dy>=0)
    {
        y_inc = dst->pitch >> 2;
    }
    else
    {
        y_inc = -dst->pitch >> 2;
        dy    = -dy;
    }

    dx2 = dx << 1;
    dy2 = dy << 1;

    if (dx > dy)
    {
        error = dy2 - dx;
        for (index=0; index <= dx; index++)
        {
            pixel = src_bitmap[px];
            if(pixel != alpha_pixel)
            {
                *vb_start = pixel;
            }
            if (error >= 0)
            {
                error-=dx2;
                vb_start+=y_inc;
            }
            error+=dy2;
            vb_start+=x_inc;
            px++;
        }
    }
    else
    {
        error = dx2 - dy;
        for (index=0; index <= dy; index++)
        {
            pixel = src_bitmap[px];
            if(pixel != alpha_pixel)
            {
                *vb_start = pixel;
            }
            if (error >= 0)
            {
                error-=dy2;
                vb_start+=x_inc;
            }
            error+=dx2;
            vb_start+=y_inc;
            px++;
        }
    }
}

void DDA_scanline(SDL_Surface *dst, Uint32 *src_bitmap, Uint32 alpha_pixel, float x1, float y1, float x2, float y2)
{
    // Digital Differential Analyzer Algorithm
    float dx = x2 - x1;
    float dy = y2 - y1;
    float steps;
    if(abs(dx) > abs(dy))
       steps = abs(dx);
    else
       steps = abs(dy);
    float Xincrement = dx / steps;
    float Yincrement = dy / steps;
    float x=x1;
    float y=y1;
    int i, px = 0;
    Uint32 pixel;
    for(i = 0; i < steps; i++)
    {
        pixel = src_bitmap[px];
        if(pixel != alpha_pixel)
        {
            setPixel32(dst, x, y, pixel);
        }
        x = x + Xincrement;
        y = y + Yincrement;
        px++;
    }
}

void blit_rotated_dda(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect, float angle)
{
    int y, bpp;
    Uint32 alpha_pixel = src->format->colorkey;

    // first do trivial rejections of bitmap, is it totally invisible?
    if ((dstrect->x >= dst->w) || (dstrect->y >= dst->h) ||
       ((dstrect->x + srcrect->w) <= 0) || ((dstrect->y + srcrect->h) <= 0))
            return;

    // clip rectangles
    // upper left hand corner first
    if (dstrect->x < 0)
    {
        srcrect->x -= dstrect->x;
        srcrect->w += dstrect->x;
        dstrect->x = 0;
    }

    if (dstrect->y < 0)
    {
        srcrect->y -= dstrect->y;
        srcrect->h += dstrect->y;
        dstrect->y = 0;
    }

    // now lower left hand corner
    if (dstrect->x + srcrect->w > dst->w)
    {
        //srcrect->x -= dstrect->x;
        srcrect->w = dst->w - dstrect->x;
        dstrect->w = dst->w;
    }

    if (dstrect->y + srcrect->h > dst->h)
    {
        srcrect->h = dst->h - dstrect->y;
        dstrect->h = dst->h;
    }

    // compute starting address in dst surface
    bpp = dst->format->BytesPerPixel;
    Uint32 *dst_buffer = (Uint32 *)((Uint8 *)dst->pixels + dstrect->y * dst->pitch + dstrect->x * bpp);

    // compute starting address in src surface to scan data from
    bpp = src->format->BytesPerPixel;
    Uint32 *src_bitmap = (Uint32 *)((Uint8 *)src->pixels + srcrect->y * src->pitch + srcrect->x * bpp);

    float x_increment = cos(RADIANS(angle));
    float y_increment = -sin(RADIANS(angle));
    float x_start = dstrect->x;
    float y_start = dstrect->y;
    float x_end = dstrect->x + srcrect->w;
    float y_end = dstrect->y;
    //printf("xs=%f ys=%f xe=%f ye=%f \n",x_start,y_start,x_end,y_end);
    float cx = (float)(dstrect->x + (srcrect->w / 2));
    float cy = (float)(dstrect->y + (srcrect->h / 2));
    rotate_point(&x_start, &y_start, cx, cy, angle);
    rotate_point(&x_end, &y_end, cx, cy, angle);
    //printf("xs=%f ys=%f xe=%f ye=%f \n",x_start,y_start,x_end,y_end);

    //DDA(dst, 197.071793, 105.071800, 213.071793, 105.071800);

    for(y = 0; y < srcrect->h; y++)
    {
        //DDA(dst, x_start, y_start, x_end, y_end, 0XFFFF00);
        //rotate_scanline(dst, src_bitmap, alpha_pixel, x_start+100, y_start, x_end+100, y_end);
        DDA_scanline(dst, src_bitmap, alpha_pixel, x_start, y_start, x_end, y_end);
        dst_buffer += dst->w;
        src_bitmap += src->w;
        x_start += -y_increment;
        y_start += x_increment;
        x_end += -y_increment;
        y_end += x_increment;
    }
}

#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))

void get_rotated_image_size(int image_width, int image_height, int *width, int *height, float angle, int *xmin, int *ymin)
{
    float half_width = (float)image_width/2;
    float half_height = (float)image_height/2;
    float x1 = -half_width, y1 = -half_height;
    float x2 = half_width, y2 = -half_height;
    float x3 = half_width, y3 = half_height;
    float x4 = -half_width, y4 = half_height;
    rotate_point(&x1, &y1, 0, 0, angle);
    rotate_point(&x2, &y2, 0, 0, angle);
    rotate_point(&x3, &y3, 0, 0, angle);
    rotate_point(&x4, &y4, 0, 0, angle);
    *width = 2 * round(MAX(MAX(x1, x2), MAX(x3, x4)));
    *height = 2 * round(MAX(MAX(y1, y2), MAX(y3, y4)));
    *xmin = round(MIN(MIN(x1, x2), MIN(x3, x4)))+half_width;
    *ymin = round(MIN(MIN(y1, y2), MIN(y3, y4)))+half_height;
    /*printf("x1=%f x2=%f x3=%f x4=%f -- y1=%f y2=%f y3=%f y4=%f \n",
           x1, x2, x3, x4,
           y1, y2, y3, y4);
    printf("w=%d h=%d \n", *width, *height);
    printf("xmin=%d ymin=%d \n", *xmin, *ymin);*/
}

#define FRAC(a) ((a) - (trunc(a)))
#define RGB_RED(rgb) ((rgb & RMASK) >> RSHIFT)
#define RGB_GREEN(rgb) ((rgb & GMASK) >> GSHIFT)
#define RGB_BLUE(rgb) ((rgb & BMASK) >> BSHIFT)

Uint32 GetSmoothColor(float xOriginal, float yOriginal, SDL_Surface *OriginalBitmap)
{
    //Bilinear interpolation
    float f0, f1, f2, f3;
    Uint32 P0, P1, P2, P3, pixel;
    float r, g, b;
    //Get fractional parts
    f0 = (1 - FRAC(xOriginal))*(1 - FRAC(yOriginal));
    f1 = FRAC(xOriginal)*(1 - FRAC(yOriginal));
    f2 = FRAC(xOriginal)*FRAC(yOriginal);
    f3 = (1 - FRAC(xOriginal))*FRAC(yOriginal);
    //Get surrounding points
    P0 = getpixel32(OriginalBitmap, trunc(xOriginal), trunc(yOriginal));
    P1 = getpixel32(OriginalBitmap, ceil(xOriginal), trunc(yOriginal));
    P2 = getpixel32(OriginalBitmap, trunc(xOriginal), ceil(yOriginal));
    P3 = getpixel32(OriginalBitmap, ceil(xOriginal), ceil(yOriginal));
    //Calculate result color
    r = round(RGB_RED(P0)*f0 + RGB_RED(P1)*f1 + RGB_RED(P2)*f2 + RGB_RED(P3)*f3);
    g = round(RGB_GREEN(P0)*f0 + RGB_GREEN(P1)*f1 + RGB_GREEN(P2)*f2 + RGB_GREEN(P3)*f3);
    b = round(RGB_BLUE(P0)*f0 + RGB_BLUE(P1)*f1 + RGB_BLUE(P2)*f2 + RGB_BLUE(P3)*f3);
    pixel = PACK_RGB((Uint8)r, (Uint8)g, (Uint8)b);
    return pixel;
}

void blit_rotated_areamap(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect, float angle)
{
    int x, y, xPrime, yPrime, xRotationAxis, yRotationAxis, xOriginal, yOriginal;
    float cosTheta, sinTheta;
    Uint32 pixel;
    //SDL_Surface *result = create_surface32(24, 24);
    SDL_Surface *BitmapOriginal = create_surface32(srcrect->w, srcrect->h);
    SDL_BlitSurface(src, srcrect, BitmapOriginal, NULL);
    //SDL_BlitSurface(BitmapOriginal, NULL, dst, dstrect);
    xRotationAxis = srcrect->w / 2;
    yRotationAxis = srcrect->h / 2;
    cosTheta = cos(RADIANS(angle));
    sinTheta = sin(RADIANS(angle));
    int width, height, xmin,ymin;
    get_rotated_image_size(srcrect->w, srcrect->h, &width, &height, angle, &xmin, &ymin);
    //An easy way to calculate the non-clipping rectangle
    //width = abs(round(BitmapOriginal->h * sinTheta)) + abs(round(BitmapOriginal->w * cosTheta));
    //height = abs(round(BitmapOriginal->w * sinTheta)) + abs(round(BitmapOriginal->h * cosTheta));
    SDL_Surface *result = create_surface32(width, height);
    //int xPrimeRotated, yPrimeRotated;
    for(y = result->h-1;  y >= ymin;  y--)
    {
        yPrime = y - yRotationAxis;
        //yPrime = 2*(y - (result->h - BitmapOriginal->h) / 2 - yRotationAxis) + 1 ;
        for(x = result->w-1;  x >= xmin;  x--)
        {
            xPrime = x - xRotationAxis;
            //xPrime = 2*(x - (result->w - BitmapOriginal->w) / 2 - xRotationAxis) + 1;
            xOriginal = xRotationAxis + round(xPrime * cosTheta - yPrime * sinTheta);
            yOriginal = yRotationAxis + round(xPrime * sinTheta + yPrime * cosTheta);

            //xPrimeRotated = round(xPrime * cosTheta - yPrime * sinTheta);
            //yPrimeRotated = round(xPrime * sinTheta + yPrime * cosTheta);
            //xOriginal = (xPrimeRotated - 1) / 2 + xRotationAxis;
            //yOriginal = (yPrimeRotated - 1) / 2 + yRotationAxis;

            //printf("xorg=%d yorg=%d \n",xOriginal,yOriginal);
            // Make sure (iOriginal, jOriginal) is in BitmapOriginal. If not,
            // assign blue color to corner points.
            if((xOriginal >= 0) && (xOriginal <= BitmapOriginal->w-1) &&
               (yOriginal >= 0) && (yOriginal <= BitmapOriginal->h-1))
            {
                // Assign pixel from rotated space to current pixel in BitmapRotated
                pixel = getpixel32(BitmapOriginal, xOriginal, yOriginal);
                //pixel = GetSmoothColor(xOriginal, yOriginal, BitmapOriginal);
                setPixel32(result, x-xmin, y-ymin, pixel);
                //printf("x=%d y=%d \n",x,y);
            }
            else
            {
                // assign "corner" color
                setPixel32(result, x-xmin, y-ymin, 0X0000FF);
                //printf("blue x=%d y=%d \n",x,y);
            }
        }
    }
    SDL_BlitSurface(result, NULL, dst, dstrect);
    SDL_FreeSurface(result);
    SDL_FreeSurface(BitmapOriginal);
}

void blit_rotated_rotozoom2(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect, float angle)
{
    int x, y, x_final, y_final; //bpp;
    Uint32 pixel; //alpha_pixel = src->format->colorkey;

    // compute starting address in dst surface
    //bpp = dst->format->BytesPerPixel;
    //Uint32 *dst_buffer = (Uint32 *)((Uint8 *)dst->pixels + dstrect->y * dst->pitch + dstrect->x * bpp);

    // compute starting address in src surface to scan data from
    //bpp = src->format->BytesPerPixel;
    //Uint32 *src_bitmap = (Uint32 *)((Uint8 *)src->pixels + srcrect->y * src->pitch + srcrect->x * bpp);

    angle = -angle;
    int scale = 1;
    int x_flip = 0;
    int y_flip = 0;
    float cosTheta = cos(RADIANS(angle));
    float sinTheta = sin(RADIANS(angle));
    float x_increment = cosTheta/scale;
    float y_increment = -sinTheta/scale;
    float px;
    float py;

    //SDL_Surface *result = create_surface32(24, 24);
    SDL_Surface *BitmapOriginal = create_surface32(srcrect->w, srcrect->h);
    SDL_BlitSurface(src, srcrect, BitmapOriginal, NULL);
    int xOriginal, yOriginal;
    int width = abs(round(BitmapOriginal->h * sinTheta)) + abs(round(BitmapOriginal->w * cosTheta));
    int height = abs(round(BitmapOriginal->w * sinTheta)) + abs(round(BitmapOriginal->h * cosTheta));
    SDL_Surface *result = create_surface32(width*scale, height*scale);
    float x_start = (float)(srcrect->w/2)-(float)(width/2)+0.5/scale;
    float y_start = (float)(srcrect->h/2)-(float)(height/2)+0.5/scale;
    rotate_point(&x_start, &y_start, (float)(srcrect->w/2), (float)(srcrect->w/2), angle);

    //printf("width=%d height=%d \n", width, height);
    //printf("x_start=%f y_start=%f \n", x_start, y_start);
    //printf("xinc=%f yinc=%f \n\n", x_increment, y_increment);

    for(y = 0;  y < result->h;  y++)
    {
        px = x_start;
        py = y_start;
        //printf("\nx_start=%f y_start=%f \n", x_start, y_start);
        for(x = 0;  x < result->w;  x++)
        {
            /*pixel = src_bitmap[x];
            if(pixel != alpha_pixel)
            {
                setPixel32(dst, px, py, pixel);
            }
            px += x_increment; py += y_increment;*/

            xOriginal = floor(px);
            yOriginal = floor(py);
            //xOriginal = round(px);
            //yOriginal = round(py);

            if (x_flip) x_final = (width*scale-1)-x; else x_final = x;
            if (y_flip) y_final = (height*scale-1)-y; else y_final = y;

            //printf("x=%d y=%d -- xorg=%d yorg=%d -- px=%f py=%f \n", x, y, xOriginal, yOriginal, px, py);

            if((xOriginal >= 0) && (xOriginal <= BitmapOriginal->w-1) &&
               (yOriginal >= 0) && (yOriginal <= BitmapOriginal->h-1))
            {
                pixel = getpixel32(BitmapOriginal, xOriginal, yOriginal);
                setPixel32(result, x_final, y_final, pixel);
                //printf("pixhit \n");
            }
            else
            {
                setPixel32(result, x_final, y_final, 0X0000FF);
            }
            //xOriginal += x_increment; yOriginal += y_increment;
            px += x_increment; py += y_increment;

        }
        //dst_buffer += dst->w;
        //src_bitmap += src->w;
        x_start += -y_increment;
        y_start += x_increment;
    }
    SDL_BlitSurface(result, NULL, dst, dstrect);
    SDL_FreeSurface(result);
    SDL_FreeSurface(BitmapOriginal);
}

void blit_rotated(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect, float angle)
{
    //blit_rotated_sincos(src, srcrect, dst, dstrect, angle);
    //blit_rotated_rotozoom(src, srcrect, dst, dstrect, angle);
    //blit_rotated_dda(src, srcrect, dst, dstrect, angle);
    //blit_rotated_areamap(src, srcrect, dst, dstrect, angle);
    blit_rotated_rotozoom2(src, srcrect, dst, dstrect, angle);
}


