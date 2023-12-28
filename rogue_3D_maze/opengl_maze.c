#include <SDL/SDL.h>
#include <SDL_image.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "rogue_maze.h"

char test_level[4][4] =
{ {0,1,1,0},
  {3,0,0,4},
  {3,0,0,4},
  {0,2,2,0}
};

GLdouble tile_size = 1;//(GLdouble)1/640*32;
GLdouble start_pos = (GLdouble)1/640*100;
#define pw_tile_size (GLdouble)1
#define pw_tile_half_size ((GLdouble)1/2)

GLuint wall_texture;
GLuint floor_texture;
GLuint door_texture;

typedef struct
{
    GLfloat x, y, z;//GLdouble x, y, z;

}Vertex3D, *Vertex3D_ptr;

typedef struct
{
    GLfloat x, y;//GLdouble x, y;

}Point2D, *Point2D_ptr;

typedef struct
{
    Point2D tex_coord;
    Vertex3D vertex;
}TexVertex3D, *TexVertex3D_ptr;

typedef struct
{
    Point2D p1, p2, p3, p4;

}Texture_Coord, *Texture_Coord_ptr;

typedef struct
{
    Vertex3D v1, v2, v3, v4;
    Texture_Coord tex_coord;
}Textured_Face, *Textured_Face_ptr;

typedef struct
{
    Vertex3D v1, v2, v3, v4;
}Flat_Face, *Flat_Face_ptr;

typedef struct
{
    union
    {
        GLfloat v[8][3];
        Vertex3D vertex[8];
    };
    Texture_Coord tex_coord;
}Cube, *Cube_ptr;

typedef struct
{
    Textured_Face* wall_array;
    Flat_Face* wall_flat_array;
    Textured_Face* floor_array;
    Textured_Face* door_array;

    int wall_count;
    int wall_flat_count;
    int floor_count;
    int door_count;

    int wall_vertices_count;
    int wall_flat_vertices_count;
    int floor_vertices_count;
    int door_vertices_count;

}GL_Level, *GL_Level_ptr;

GL_Level gl_level = {NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0};

typedef struct
{
    Vertex3D size;
    Vertex3D offset;
    Point2D tex_start;
    Point2D tex_end;
    Point2D image_size;
}Partial_Wall, *Partial_Wall_ptr;

Partial_Wall h_entrance_right_p_wall = {{pw_tile_half_size/2, pw_tile_half_size/2, pw_tile_half_size*3}, {pw_tile_half_size+pw_tile_half_size/2, 0, 0}, {48, 16}, {64, 64}, {64, 64}};
Partial_Wall h_entrance_left_p_wall = {{pw_tile_half_size/2, pw_tile_half_size/2, pw_tile_half_size*3}, {-pw_tile_half_size-pw_tile_half_size/2, 0, 0}, {0, 16}, {16, 64}, {64, 64}};
Partial_Wall h_entrance_top_p_wall = {{pw_tile_size, pw_tile_half_size/2, pw_tile_half_size}, {0, 0, pw_tile_half_size*3}, {0, 0}, {64, 16}, {64, 64}};
Partial_Wall h_entrance_door = {{pw_tile_half_size, pw_tile_half_size/4, pw_tile_half_size*3}, {0, 0, 0}, {0, 0}, {64, 64}, {64, 64}};
Partial_Wall v_entrance_right_p_wall = {{pw_tile_half_size/2, pw_tile_half_size/2, pw_tile_half_size*3}, {0, pw_tile_half_size+pw_tile_half_size/2, 0}, {48, 16}, {64, 64}, {64, 64}};
Partial_Wall v_entrance_left_p_wall = {{pw_tile_half_size/2, pw_tile_half_size/2, pw_tile_half_size*3}, {0, -pw_tile_half_size-pw_tile_half_size/2, 0}, {0, 16}, {16, 64}, {64, 64}};
Partial_Wall v_entrance_top_p_wall = {{pw_tile_half_size/2, pw_tile_size, pw_tile_half_size}, {0, 0, pw_tile_half_size*3}, {0, 0}, {64, 16}, {64, 64}};
Partial_Wall v_entrance_door = {{pw_tile_half_size/4,  pw_tile_half_size, pw_tile_half_size*3}, {0, 0, 0}, {0, 0}, {64, 64}, {64, 64}};

void gl_test_old(void)
{
    glClear(GL_COLOR_BUFFER_BIT);

    glBegin(GL_QUADS);
        glColor3ub(255,0,0);    glVertex2d(-0.75,-0.75);
        glColor3ub(255,255,255);glVertex2d(-0.75,0.75);
        glColor3ub(0,255,0);    glVertex2d(0.75,0.75);
        glColor3ub(0,0,255);    glVertex2d(0.75,-0.75);
    glEnd();

    glFlush();
    SDL_GL_SwapBuffers();
}

void gl_quad(void)
{
    GLdouble s = tile_size; //(GLdouble)1/640*32;
    glBegin(GL_QUADS);
        glColor3ub(255,0,0);    glVertex2d(-s,-s);
        glColor3ub(255,255,255);glVertex2d(-s,s);
        glColor3ub(0,255,0);    glVertex2d(s,s);
        glColor3ub(0,0,255);    glVertex2d(s,-s);
    glEnd();
}

void gl_color_quad(GLubyte* color)
{
    GLdouble s =  tile_size; //(GLdouble)1/640*32;
    glBegin(GL_QUADS);
        glColor3ubv(color);
        glVertex2d(-s,-s);
        glVertex2d(-s,s);
        glVertex2d(s,s);
        glVertex2d(s,-s);
    glEnd();
}

void gl_cube(void)
{
    GLdouble s =  tile_size; //(GLdouble)1/640*32;
    GLdouble zoff = s;
    GLdouble v[8][3] = {
        {-s, -s, s+zoff}, {-s ,s, s+zoff},{s, s, s+zoff}, {s, -s, s+zoff},
        {-s, -s, -s+zoff}, {-s ,s, -s+zoff},{s, s, -s+zoff}, {s, -s, -s+zoff},
    };
    glBegin(GL_QUADS);
        glColor3ub(255,255,0);
        glVertex3dv(v[0]);
        glVertex3dv(v[1]);
        glVertex3dv(v[2]);
        glVertex3dv(v[3]);

        glColor3ub(0,0,255);
        glVertex3dv(v[0]);
        glVertex3dv(v[3]);
        glVertex3dv(v[7]);
        glVertex3dv(v[4]);

        glColor3ub(0,255,0);
        glVertex3dv(v[3]);
        glVertex3dv(v[2]);
        glVertex3dv(v[6]);
        glVertex3dv(v[7]);

        glColor3ub(0,255,255);
        glVertex3dv(v[0]);
        glVertex3dv(v[1]);
        glVertex3dv(v[5]);
        glVertex3dv(v[4]);

        glColor3ub(255,0,255);
        glVertex3dv(v[1]);
        glVertex3dv(v[2]);
        glVertex3dv(v[6]);
        glVertex3dv(v[5]);

        glColor3ub(255,0,0);
        glVertex3dv(v[4]);
        glVertex3dv(v[5]);
        glVertex3dv(v[6]);
        glVertex3dv(v[7]);

    glEnd();
}

void gl_color_cube(GLubyte* color)
{
    GLdouble s = tile_size; //(GLdouble)1/640*32;
    GLdouble zoff = s;
    GLdouble v[8][3] = {
        {-s, -s, s+zoff}, {-s ,s, s+zoff},{s, s, s+zoff}, {s, -s, s+zoff},
        {-s, -s, -s+zoff}, {-s ,s, -s+zoff},{s, s, -s+zoff}, {s, -s, -s+zoff},
    };
    glBegin(GL_QUADS);

        glColor3ubv(color);

        glVertex3dv(v[0]);
        glVertex3dv(v[1]);
        glVertex3dv(v[2]);
        glVertex3dv(v[3]);

        glVertex3dv(v[0]);
        glVertex3dv(v[3]);
        glVertex3dv(v[7]);
        glVertex3dv(v[4]);

        glVertex3dv(v[3]);
        glVertex3dv(v[2]);
        glVertex3dv(v[6]);
        glVertex3dv(v[7]);

        glVertex3dv(v[0]);
        glVertex3dv(v[1]);
        glVertex3dv(v[5]);
        glVertex3dv(v[4]);

        glVertex3dv(v[1]);
        glVertex3dv(v[2]);
        glVertex3dv(v[6]);
        glVertex3dv(v[5]);

        glVertex3dv(v[4]);
        glVertex3dv(v[5]);
        glVertex3dv(v[6]);
        glVertex3dv(v[7]);

    glEnd();
}

void gl_test(void)
{
    glEnable(GL_DEPTH_TEST);
    //gluOrtho2D(0,640,0,480);
    glMatrixMode(GL_PROJECTION);glLoadIdentity( );
    gluPerspective(70,(double)640/480,1,1000);glMatrixMode(GL_MODELVIEW);
    //gluLookAt(0, 0, 1.5, 0, 0, 0, 0, 1 ,0);

    int x, y;
    GLdouble tile_size = (GLdouble)1/640*32, start_pos = (GLdouble)1/640*100, orientation = tile_size;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);glColor3ub(0,255,0);glTranslated(-1+start_pos,1-start_pos,0);

    for(y=0; y<4; y++)
    {
        glLoadIdentity();gluLookAt(0, 0, 1.5, 0, 0, 0, 0, 1 ,0);
        glTranslated(-1+start_pos,(1-start_pos)-(y*tile_size*2),0);

        for(x=0; x<4; x++)
        {
            if(test_level[y][x] == 0)
            {
                glColor3ub(255,0,0);
                glBegin(GL_QUADS);
                    glVertex2d(-tile_size, -tile_size);
                    glVertex2d(-tile_size, tile_size);
                    glVertex2d(tile_size, tile_size);
                    glVertex2d(tile_size, -tile_size);
                glEnd();
            }
            else if(test_level[y][x] < 3)
            {
                glColor3ub(0,0,255);
                if(test_level[y][x] == 1) orientation = -tile_size;
                else if(test_level[y][x] == 2) orientation = tile_size;
                glBegin(GL_QUADS);
                    glVertex3d(-tile_size, orientation, 0);
                    glVertex3d(-tile_size, orientation, tile_size*2);
                    glVertex3d(tile_size, orientation, tile_size*2);
                    glVertex3d(tile_size, orientation, 0);
                glEnd();
            }
            else if(test_level[y][x] > 2)
            {
                glColor3ub(0,255,0);
                glBegin(GL_QUADS);
                    if(test_level[y][x] == 3) orientation = tile_size;
                    else if(test_level[y][x] == 4) orientation = -tile_size;
                    glVertex3d(orientation, tile_size, 0);
                    glVertex3d(orientation, tile_size, tile_size*2);
                    glVertex3d(orientation, -tile_size, tile_size*2);
                    glVertex3d(orientation, -tile_size, 0);
                glEnd();
            }
            gl_cube();
            //gl_quad();
            glTranslated(tile_size*2,0,0);
        }
    }
    glTranslated(0,-tile_size*2,0);gl_cube();//gl_quad();

    glFlush();
    SDL_GL_SwapBuffers();
}

void gl_init(void)
{
    glEnable(GL_DEPTH_TEST);
    //gluOrtho2D(0,640,0,480);
    glMatrixMode(GL_PROJECTION);glLoadIdentity( );
    gluPerspective(70,(double)640/480,1,1000);glMatrixMode(GL_MODELVIEW);
    gluLookAt(0, 0, 1.5, 0, 0, 0, 0, 1 ,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void gl_flip(void)
{
    glFlush();
    SDL_GL_SwapBuffers();

}

void gl_draw_level_surface(void)
{
    GLdouble w = 80 * tile_size*2, h = 24 * tile_size*2;
    glPushMatrix();
    glLoadIdentity();
    FreeFlyCameraLookAt(&freeFlyCamera);
    glTranslated(-1+start_pos,1-start_pos,-0.01);
    glBegin(GL_QUADS);
        glColor3ub(17,206,112);
        glVertex2d(0,0);
        glVertex2d(w,0);
        glVertex2d(w,-h);
        glVertex2d(0,-h);
    glEnd();
    glPopMatrix();
}

SDL_Surface * flipSurface(SDL_Surface * surface)
{
    int current_line,pitch;
    SDL_Surface * fliped_surface = SDL_CreateRGBSurface(SDL_SWSURFACE,
                                   surface->w,surface->h,
                                   surface->format->BitsPerPixel,
                                   surface->format->Rmask,
                                   surface->format->Gmask,
                                   surface->format->Bmask,
                                   surface->format->Amask);



    SDL_LockSurface(surface);
    SDL_LockSurface(fliped_surface);

    pitch = surface->pitch;
    for (current_line = 0; current_line < surface->h; current_line ++)
    {
        memcpy(&((unsigned char* )fliped_surface->pixels)[current_line*pitch],
               &((unsigned char* )surface->pixels)[(surface->h - 1  -
                                                    current_line)*pitch],
               pitch);
    }

    SDL_UnlockSurface(fliped_surface);
    SDL_UnlockSurface(surface);
    return fliped_surface;
}

GLuint loadTexture(char* filename)
{
    char useMipMap = 1;
    GLuint glID;
    SDL_Surface * picture_surface = NULL;
    SDL_Surface *gl_surface = NULL;
    SDL_Surface * gl_fliped_surface = NULL;
    Uint32 rmask, gmask, bmask, amask;

    picture_surface = IMG_Load(filename);
    if (picture_surface == NULL)
        return 0;

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

    SDL_PixelFormat format = *(picture_surface->format);
    format.BitsPerPixel = 32;
    format.BytesPerPixel = 4;
    format.Rmask = rmask;
    format.Gmask = gmask;
    format.Bmask = bmask;
    format.Amask = amask;

    gl_surface = SDL_ConvertSurface(picture_surface,&format,SDL_SWSURFACE);

    gl_fliped_surface = flipSurface(gl_surface);

    glGenTextures(1, &glID);

    glBindTexture(GL_TEXTURE_2D, glID);


    if (useMipMap)
    {

        gluBuild2DMipmaps(GL_TEXTURE_2D, 4, gl_fliped_surface->w,
                          gl_fliped_surface->h, GL_RGBA,GL_UNSIGNED_BYTE,
                          gl_fliped_surface->pixels);

        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR_MIPMAP_LINEAR);

    }
    else
    {
        glTexImage2D(GL_TEXTURE_2D, 0, 4, gl_fliped_surface->w,
                     gl_fliped_surface->h, 0, GL_RGBA,GL_UNSIGNED_BYTE,
                     gl_fliped_surface->pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);


    SDL_FreeSurface(gl_fliped_surface);
    SDL_FreeSurface(gl_surface);
    SDL_FreeSurface(picture_surface);

    return glID;
}


void gl_texture_test(void)
{
    glClearColor (1, 1, 1, 0);
    glEnable(GL_TEXTURE_2D);
    gl_init();
    GLdouble s = 1;
    GLuint texture = loadTexture("dk_wall.png");
    glBindTexture(GL_TEXTURE_2D, texture);
    glLoadIdentity();
    FreeFlyCameraLookAt(&freeFlyCamera);
    glBegin(GL_QUADS);
        glColor3ub(255,255,255);
        glTexCoord2d(0,0); glVertex2d(-s,-s);
        glTexCoord2d(0,1); glVertex2d(-s,s);
        glTexCoord2d(1,1); glVertex2d(s,s);
        glTexCoord2d(1,0); glVertex2d(s,-s);
    glEnd();
    glFlush();
    SDL_GL_SwapBuffers();
}

void gl_init_textures(void)
{
    wall_texture = loadTexture("dk_wall.png");
    floor_texture = loadTexture("wolf_floor.png");
    door_texture = loadTexture("wolf_door.png");
    /*wall_texture = loadTexture("textures/brick_wall.png");
    floor_texture = loadTexture("textures/dungeon_ground.png");
    door_texture = loadTexture("textures/wooden_door.png");*/

}

void gl_wall(void)
{
    GLdouble s = tile_size;
    GLdouble zoff = s;
    GLdouble v[8][3] = {
        {-s, -s, s+zoff}, {s ,-s, s+zoff},{s, s, s+zoff}, {-s, s, s+zoff},
        {-s, -s, -s+zoff}, {s ,-s, -s+zoff},{s, s, -s+zoff}, {-s, s, -s+zoff},
    };
    glBegin(GL_QUADS);
        glColor3ub(196,88,24);
        // top face
        glVertex3dv(v[0]);
        glVertex3dv(v[1]);
        glVertex3dv(v[2]);
        glVertex3dv(v[3]);
        // bottom face
        glVertex3dv(v[5]);
        glVertex3dv(v[4]);
        glVertex3dv(v[7]);
        glVertex3dv(v[6]);
    glEnd();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, wall_texture);
    glColor3ub(255,255,255);

    glBegin(GL_QUADS);
        // front face
        glTexCoord2d(0,0); glVertex3dv(v[4]);
        glTexCoord2d(1,0); glVertex3dv(v[5]);
        glTexCoord2d(1,1); glVertex3dv(v[1]);
        glTexCoord2d(0,1); glVertex3dv(v[0]);
        // right face
        glTexCoord2d(0,0); glVertex3dv(v[5]);
        glTexCoord2d(1,0); glVertex3dv(v[6]);
        glTexCoord2d(1,1); glVertex3dv(v[2]);
        glTexCoord2d(0,1); glVertex3dv(v[1]);
        // left face
        glTexCoord2d(0,0); glVertex3dv(v[7]);
        glTexCoord2d(1,0); glVertex3dv(v[4]);
        glTexCoord2d(1,1); glVertex3dv(v[0]);
        glTexCoord2d(0,1); glVertex3dv(v[3]);
        // back face
        glTexCoord2d(0,0); glVertex3dv(v[6]);
        glTexCoord2d(1,0); glVertex3dv(v[7]);
        glTexCoord2d(1,1); glVertex3dv(v[3]);
        glTexCoord2d(0,1); glVertex3dv(v[2]);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}


void gl_floor(void)
{
    GLdouble s =  tile_size;
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, floor_texture);
    glColor3ub(255,255,255);
    glBegin(GL_QUADS);
        glTexCoord2d(0,0); glVertex2d(-s,-s);
        glTexCoord2d(1,0); glVertex2d(s,-s);
        glTexCoord2d(1,1); glVertex2d(s,s);
        glTexCoord2d(0,1); glVertex2d(-s,s);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void gl_door(byte pos)
{
    GLdouble s =  tile_size;
    GLdouble zoff = s;
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, door_texture);
    glColor3ub(255,255,255);
    if(pos == UHDOOR)
    {
        glBegin(GL_QUADS);
            glTexCoord2d(0,0); glVertex3d(-s,s,-s+zoff);
            glTexCoord2d(1,0); glVertex3d(s,s,-s+zoff);
            glTexCoord2d(1,1); glVertex3d(s,s,s+zoff);
            glTexCoord2d(0,1); glVertex3d(-s,s,s+zoff);
        glEnd();
    }
    else if(pos == LHDOOR)
    {
        glBegin(GL_QUADS);
            glTexCoord2d(0,0); glVertex3d(-s,-s,-s+zoff);
            glTexCoord2d(1,0); glVertex3d(s,-s,-s+zoff);
            glTexCoord2d(1,1); glVertex3d(s,-s,s+zoff);
            glTexCoord2d(0,1); glVertex3d(-s,-s,s+zoff);
        glEnd();
    }
    else if(pos == LVDOOR)// || pos == RVDOOR)//(pos == VWALL)
    {
        glBegin(GL_QUADS);
            glTexCoord2d(0,0); glVertex3d(-s,s,-s+zoff);
            glTexCoord2d(1,0); glVertex3d(-s,-s,-s+zoff);
            glTexCoord2d(1,1); glVertex3d(-s,-s,s+zoff);
            glTexCoord2d(0,1); glVertex3d(-s,s,s+zoff);
        glEnd();
    }
    else if(pos == RVDOOR)
    {
        glBegin(GL_QUADS);
            glTexCoord2d(0,0); glVertex3d(s,s,-s+zoff);
            glTexCoord2d(1,0); glVertex3d(s,-s,-s+zoff);
            glTexCoord2d(1,1); glVertex3d(s,-s,s+zoff);
            glTexCoord2d(0,1); glVertex3d(s,s,s+zoff);
        glEnd();
    }

    glDisable(GL_TEXTURE_2D);
}

void gl_init_vertex_array(void)
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    //glVertexPointer(3, GL_DOUBLE, 0, wall_v_array);
    //glTexCoordPointer(2, GL_DOUBLE, 0, wall_tex_array);

    //glVertexPointer(3, GL_DOUBLE, 6*sizeof(GLdouble), &(wall_array[0].v1));
    //glTexCoordPointer(2, GL_DOUBLE, 6*sizeof(GLdouble), &(wall_array[0].tex_coord));

    glClearColor (1, 1, 1, 0);
}

void gl_vertex_array_test(void)
{
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    //GLfloat light_position[] = {100.0, 0.0, 100.0, 1.0};
    GLfloat light_position[] = {10.0, 0.0, 10.0, 0.0};
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    GLfloat light_ambient[] = {1.0, 1.0, 1.0, 1.0};
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);

    glClearColor (1, 1, 1, 0);
    glColor3ub(255,255,255);
    glEnable(GL_TEXTURE_2D);
    gl_init();
    GLuint texture = loadTexture("dk_wall.png");
    glBindTexture(GL_TEXTURE_2D, texture);
    glLoadIdentity();
    gluLookAt(3, 3, 5, 0, 0, 0, 0,1,0);

    GLdouble s = 1;
    GLdouble zoff = s;
    GLdouble v[8][3] = {
        {-s, -s, s+zoff}, {s ,-s, s+zoff},{s, s, s+zoff}, {-s, s, s+zoff},
        {-s, -s, -s+zoff}, {s ,-s, -s+zoff},{s, s, -s+zoff}, {-s, s, -s+zoff},
    };
    GLdouble n[6][3] = {
        {0, 0, 1}, {0, 0, -1}, {0, -1, 0},
        {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}
    };

    GLdouble tex_coord[4][2] = {{0,0}, {1,0}, {1,1}, {0,1}};

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    //glVertexPointer(3, GL_DOUBLE, 0, v);
    //glTexCoordPointer(2, GL_DOUBLE, 0, tex_coord);

    GLubyte wall_faces_indices[24] = {0, 1, 2, 3,  // top face
                                      5, 4, 7, 6,  // bottom face
                                      4, 5, 1, 0,  // front face
                                      5, 6, 2, 1,  // right face
                                      7, 4, 0, 3,  // left face
                                      6, 7, 3, 2}; // back face

    GLubyte wall_normal_indices[24] = {0, 0, 0, 0,  // top face
                                       1, 1, 1, 1,  // bottom face
                                       2, 2, 2, 2,  // front face
                                       3, 3, 3, 3,  // right face
                                       4, 4, 4, 4,  // left face
                                       5, 5, 5, 5}; // back face

    GLdouble vertex_array[24][3];
    GLdouble texture_array[24][2];
    GLdouble normal_array[24][3];
    int i;
    for(i = 0; i < 24; i++)
    {
        vertex_array[i][0] = v[wall_faces_indices[i]][0];
        vertex_array[i][1] = v[wall_faces_indices[i]][1];
        vertex_array[i][2] = v[wall_faces_indices[i]][2];
        texture_array[i][0] = tex_coord[i%4][0];
        texture_array[i][1] = tex_coord[i%4][1];
        normal_array[i][0] = n[wall_normal_indices[i]][0];
        normal_array[i][1] = n[wall_normal_indices[i]][1];
        normal_array[i][2] = n[wall_normal_indices[i]][2];
    }

    glVertexPointer(3, GL_DOUBLE, 0, vertex_array);
    glTexCoordPointer(2, GL_DOUBLE, 0, texture_array);
    glNormalPointer(GL_DOUBLE, 0, normal_array);

    //glDrawElements(GL_QUADS, 24, GL_UNSIGNED_BYTE, wall_faces_indices);
    glDrawArrays(GL_QUADS, 0, 24);

    glFlush();
    SDL_GL_SwapBuffers();
}

void gl_copy_face_vertex(GLdouble *face, GLdouble (*v0)[3], GLdouble (*v1)[3], GLdouble (*v2)[3], GLdouble (*v3)[3])
{
    //int a[2][3]={{8,7,3}, {4,9,6}};
    //int (*b)[3]=&a[0];printf("%d\n",*((*(b+1))+1) );
    //printf("%d\n",(*b)[2]);
    *face = (*v0)[0]; *(face+1) = (*v0)[1]; *(face+2) = (*v0)[2];
    *(face+3) = (*v1)[0]; *(face+3+1) = (*v1)[1]; *(face+3+2) = (*v1)[2];
    *(face+3*2) = (*v2)[0]; *(face+3*2+1) = (*v2)[1]; *(face+3*2+2) = (*v2)[2];
    *(face+3*3) = (*v3)[0]; *(face+3*3+1) = (*v3)[1]; *(face+3*3+2) = (*v3)[2];
}

void gl_copy_face_tex(GLdouble *face_tex, GLdouble (*tex_coord)[4][2])
{
    *face_tex = ((*tex_coord)[0])[0]; *(face_tex+1) = ((*tex_coord)[0])[1];
    *(face_tex+2) = ((*tex_coord)[1])[0]; *(face_tex+2+1) = ((*tex_coord)[1])[1];
    *(face_tex+2*2) = ((*tex_coord)[2])[0]; *(face_tex+2*2+1) = ((*tex_coord)[2])[1];
    *(face_tex+2*3) = ((*tex_coord)[3])[0]; *(face_tex+2*3+1) = ((*tex_coord)[3])[1];
}

void gl_translate_cube(GLfloat (*cube)[8][3], GLdouble x, GLdouble y, GLdouble z)
{
    int i;//printf("i=%d c=%f\n",i,((*cube)[6])[2]);
    for(i = 0; i < 8; i++)
    {
        //printf("i=%d c=%f\n",i,*cube[i][0]);
        ((*cube)[i])[0] += x;
        ((*cube)[i])[1] += y;
        ((*cube)[i])[2] += z;
    }
}

void gl_load_cube(GLfloat (*cube)[8][3], GLdouble x, GLdouble y, GLdouble z)
{
    GLdouble s = tile_size;
    GLdouble zoff = s;
    GLdouble v[8][3] = {
        {-s, -s, s+zoff}, {s ,-s, s+zoff},{s, s, s+zoff}, {-s, s, s+zoff},
        {-s, -s, -s+zoff}, {s ,-s, -s+zoff},{s, s, -s+zoff}, {-s, s, -s+zoff},
    };
    int i;
    for(i = 0; i < 8; i++)
    {
        ((*cube)[i])[0] = v[i][0] + x;
        ((*cube)[i])[1] = v[i][1] + y;
        ((*cube)[i])[2] = v[i][2] + z;
    }
}

void gl_init_cube(Cube *cube)
{
    GLdouble s = tile_size;
    GLdouble zoff = s;
    GLdouble v[8][3] = {
        {-s, -s, s+zoff}, {s ,-s, s+zoff},{s, s, s+zoff}, {-s, s, s+zoff},
        {-s, -s, -s+zoff}, {s ,-s, -s+zoff},{s, s, -s+zoff}, {-s, s, -s+zoff},
    };
    int i;
    for(i = 0; i < 8; i++)
    {
        cube->vertex[i].x = v[i][0];
        cube->vertex[i].y = v[i][1];
        cube->vertex[i].z = v[i][2];
    }
    Texture_Coord tex_coord = {{0,0}, {1,0}, {1,1}, {0,1}};
    cube->tex_coord = tex_coord;
}

// Cube Faces
enum{FRONT_FACE, BACK_FACE, LEFT_FACE, RIGHT_FACE, TOP_FACE, BOTTOM_FACE};
int cube_faces[6][4]={{4,5,1,0}, {6,7,3,2}, {7,4,0,3}, {5,6,2,1}, {0,1,2,3}, {5,4,7,6}};

void gl_copy_face_interleaved(Cube *cube, Textured_Face *face, int face_num)
{
    TexVertex3D *tex_vertex_ptr = (TexVertex3D*) face;

    int i, *cube_face = cube_faces[face_num];
    Point2D_ptr cube_tex_coord = &(cube->tex_coord.p1);
    for(i = 0; i < 4; i++)
    {
        tex_vertex_ptr[i].vertex = cube->vertex[cube_face[i]];
        tex_vertex_ptr[i].tex_coord = cube_tex_coord[i];
    }
}

void gl_copy_face_interleaved_reverse(Cube *cube, Textured_Face *face, int face_num)
{
    TexVertex3D *tex_vertex_ptr = (TexVertex3D*) face;

    int i, *cube_face = cube_faces[face_num];
    Point2D_ptr cube_tex_coord = &(cube->tex_coord.p1);
    int revered[4] = {1, 0, 3, 2};
    for(i = 0; i < 4; i++)
    {
        tex_vertex_ptr[revered[i]].vertex = cube->vertex[cube_face[i]];
        tex_vertex_ptr[i].tex_coord = cube_tex_coord[i];
    }
}

void gl_print_vertex_array(Textured_Face *vertex_array, int count, int num_face)
{
    printf("\n\n*******PRINTING VERTEX ARRAY*******\n\n");
    int i, j;
    for(i = 0; i < count*num_face; i++)
    {
        printf("i=%d\n",i);
        TexVertex3D *tex_vertex_ptr = (TexVertex3D*) &(vertex_array[i]);
        for(j = 0; j < 4; j++)
        {
            Vertex3D_ptr v= &tex_vertex_ptr[j].vertex;
            Point2D_ptr t= &tex_vertex_ptr[j].tex_coord;
            printf("v%d=(x=%f, y=%f, z=%f) t%d=(x=%f, y=%f)\n",j, v->x, v->y, v->z,  j, t->x, t->y);
            printf("****************\n");
        }
    }
}

void gl_draw_vertex_array(Textured_Face *vertex_array, int count, int num_face)
{
    glVertexPointer(3, GL_FLOAT, 5*sizeof(GLfloat), &(((GLfloat*)vertex_array)[2]));
    glTexCoordPointer(2, GL_FLOAT, 5*sizeof(GLfloat), vertex_array);
    int i;
    for(i = 0; i < count*num_face; i++)
    {
        glBegin(GL_QUADS);
            glArrayElement(i*4); glArrayElement(i*4+1);
            glArrayElement(i*4+2); glArrayElement(i*4+3);
        glEnd();
    }
}

void gl_copy_face(Cube *cube ,void *face, int face_num, int textured)
{
    int *cube_face = cube_faces[face_num];

    ((Flat_Face*)face)->v1 = cube->vertex[cube_face[0]];
    ((Flat_Face*)face)->v2 = cube->vertex[cube_face[1]];
    ((Flat_Face*)face)->v3 = cube->vertex[cube_face[2]];
    ((Flat_Face*)face)->v4 = cube->vertex[cube_face[3]];

    if(textured)
        ((Textured_Face*)face)->tex_coord = cube->tex_coord;
}

void gl_resize_cube(GLfloat (*cube)[8][3], GLdouble x, GLdouble y, GLdouble z)
{
    GLdouble s = tile_size;
    GLdouble zoff = s;
    GLdouble v[8][3] = {
        {-s+x, -s+y, s+zoff-z}, {s-x ,-s+y, s+zoff-z},{s-x, s-y, s+zoff-z}, {-s+x, s-y, s+zoff-z},
        {-s+x, -s+y, -s+zoff}, {s-x ,-s+y, -s+zoff},{s-x, s-y, -s+zoff}, {-s+x, s-y, -s+zoff},
    };
    int i;
    for(i = 0; i < 8; i++)
    {
        ((*cube)[i])[0] -= v[i][0];
        ((*cube)[i])[1] -= v[i][1];
        ((*cube)[i])[2] -= v[i][2];
    }
}

void gl_get_texture_coordinate(Texture_Coord *tex_coord, Point2D *tex_start, Point2D *tex_end, Point2D *image_size)
{
    GLfloat x1 = (1 / image_size->x) * tex_start->x;
    GLfloat y1 = (1 / image_size->y) * (image_size->y - tex_end->y);
    GLfloat x2 = (1 / image_size->x) * tex_end->x;
    GLfloat y2 = (1 / image_size->y) * (image_size->y - tex_start->y);
    tex_coord->p1.x = x1; tex_coord->p1.y = y1;
    tex_coord->p2.x = x2; tex_coord->p2.y = y1;
    tex_coord->p3.x = x2; tex_coord->p3.y = y2;
    tex_coord->p4.x = x1; tex_coord->p4.y = y2;
}

void gl_partial_wall(Cube *cube, Cube *p_wall, Partial_Wall *pw_data)
{
    *p_wall = *cube;
    gl_resize_cube(&p_wall->v, pw_data->size.x, pw_data->size.y, pw_data->size.z);
    gl_translate_cube(&p_wall->v, pw_data->offset.x, pw_data->offset.y, pw_data->offset.z);
    gl_get_texture_coordinate(&p_wall->tex_coord, &pw_data->tex_start, &pw_data->tex_end, &pw_data->image_size);
}

void gl_copy_h_partial_wall(Cube *cube, Partial_Wall *p_wall_data, int *wall_index, int *flat_wall_index, int *door_index, int door)
{
    int num_face = 2;
    Cube p_wall;
    if(!door)
    {
        gl_partial_wall(cube, &p_wall, p_wall_data);
        gl_copy_face_interleaved(&p_wall, &gl_level.wall_array[*wall_index*num_face], FRONT_FACE);
        gl_copy_face_interleaved_reverse(&p_wall, &gl_level.wall_array[*wall_index*num_face+1], BACK_FACE);
        (*wall_index)++;
    }
    else
    {
        gl_partial_wall(cube, &p_wall, p_wall_data);
        gl_copy_face_interleaved(&p_wall, &gl_level.door_array[*door_index*num_face], FRONT_FACE);
        gl_copy_face_interleaved_reverse(&p_wall, &gl_level.door_array[*door_index*num_face+1], BACK_FACE);
        (*door_index)++;
    }
    gl_copy_face(&p_wall, &gl_level.wall_flat_array[*flat_wall_index*num_face], TOP_FACE, 0);
    gl_copy_face(&p_wall, &gl_level.wall_flat_array[*flat_wall_index*num_face+1], BOTTOM_FACE, 0);
    (*flat_wall_index)++;
    gl_copy_face(&p_wall, &gl_level.wall_flat_array[*flat_wall_index*num_face], LEFT_FACE, 0);
    gl_copy_face(&p_wall, &gl_level.wall_flat_array[*flat_wall_index*num_face+1], RIGHT_FACE, 0);
    (*flat_wall_index)++;
}

void gl_copy_v_partial_wall(Cube *cube, Partial_Wall *p_wall_data, int *wall_index, int *flat_wall_index, int *door_index, int door)
{
    int num_face = 2;
    Cube p_wall;
    if(!door)
    {
        gl_partial_wall(cube, &p_wall, p_wall_data);
        gl_copy_face_interleaved(&p_wall, &gl_level.wall_array[*wall_index*num_face], LEFT_FACE);
        gl_copy_face_interleaved_reverse(&p_wall, &gl_level.wall_array[*wall_index*num_face+1], RIGHT_FACE);
        (*wall_index)++;
    }
    else
    {
        gl_partial_wall(cube, &p_wall, p_wall_data);
        gl_copy_face_interleaved(&p_wall, &gl_level.door_array[*door_index*num_face], LEFT_FACE);
        gl_copy_face_interleaved_reverse(&p_wall, &gl_level.door_array[*door_index*num_face+1], RIGHT_FACE);
        (*door_index)++;
    }
    gl_copy_face(&p_wall, &gl_level.wall_flat_array[*flat_wall_index*num_face], TOP_FACE, 0);
    gl_copy_face(&p_wall, &gl_level.wall_flat_array[*flat_wall_index*num_face+1], BOTTOM_FACE, 0);
    (*flat_wall_index)++;
    gl_copy_face(&p_wall, &gl_level.wall_flat_array[*flat_wall_index*num_face], FRONT_FACE, 0);
    gl_copy_face(&p_wall, &gl_level.wall_flat_array[*flat_wall_index*num_face+1], BACK_FACE, 0);
    (*flat_wall_index)++;
}

#define H_ENTRANCE 0
#define V_ENTRANCE 1

void gl_door_entrance(Cube *cube, int *wall_index, int *flat_wall_index, int *door_index, int entrance_type)
{
    if(entrance_type == H_ENTRANCE)
    {
        gl_copy_h_partial_wall(cube, &h_entrance_left_p_wall, wall_index, flat_wall_index, door_index, 0);
        gl_copy_h_partial_wall(cube, &h_entrance_right_p_wall, wall_index, flat_wall_index, door_index, 0);
        gl_copy_h_partial_wall(cube, &h_entrance_top_p_wall, wall_index, flat_wall_index, door_index, 0);
        gl_copy_h_partial_wall(cube, &h_entrance_door, wall_index, flat_wall_index, door_index, 1);
    }
    else if(entrance_type == V_ENTRANCE)
    {
        gl_copy_v_partial_wall(cube, &v_entrance_left_p_wall, wall_index, flat_wall_index, door_index, 0);
        gl_copy_v_partial_wall(cube, &v_entrance_right_p_wall, wall_index, flat_wall_index, door_index, 0);
        gl_copy_v_partial_wall(cube, &v_entrance_top_p_wall, wall_index, flat_wall_index, door_index, 0);
        gl_copy_v_partial_wall(cube, &v_entrance_door, wall_index, flat_wall_index, door_index, 1);
    }
}

void gl_clear_level(void)
{
    gl_level.wall_array = NULL;
    gl_level.wall_flat_array = NULL;
    gl_level.floor_array = NULL;
    gl_level.door_array = NULL;

    gl_level.wall_count = 0;
    gl_level.wall_flat_count = 0;
    gl_level.floor_count = 0;
    gl_level.door_count = 0;

    gl_level.wall_vertices_count = 0;
    gl_level.wall_flat_vertices_count = 0;
    gl_level.floor_vertices_count = 0;
    gl_level.door_vertices_count = 0;
}

void gl_free_level(void)
{
    if(gl_level.wall_array != NULL)
    {
        free(gl_level.wall_array);
    }
    if(gl_level.wall_flat_array != NULL)
    {
        free(gl_level.wall_flat_array);
    }
    if(gl_level.floor_array != NULL)
    {
        free(gl_level.floor_array);
    }
    if(gl_level.door_array != NULL)
    {
        free(gl_level.door_array);
    }
    gl_clear_level();
}

void gl_malloc_level(int wall_count, int wall_flat_count, int floor_count, int door_count, int num_face)
{
    gl_free_level();
    gl_level.wall_count = wall_count;
    gl_level.wall_vertices_count = wall_count*num_face*4;
    gl_level.wall_flat_vertices_count = wall_flat_count*num_face*4;
    gl_level.floor_count = floor_count;
    gl_level.floor_vertices_count = floor_count*4;
    gl_level.door_count = door_count;
    gl_level.door_vertices_count = door_count*num_face*4;
    gl_level.wall_array = malloc(wall_count*num_face*sizeof(Textured_Face));
    if(gl_level.wall_array == NULL){printf("\n couldn't allocate wall_array \n"); return;};
    gl_level.wall_flat_array = malloc(wall_flat_count*num_face*sizeof(Flat_Face));
    if(gl_level.wall_flat_array == NULL){printf("\n couldn't allocate wall_flat_array \n"); return;};
    gl_level.floor_array = malloc(floor_count*sizeof(Textured_Face));
    if(gl_level.floor_array == NULL){printf("\n couldn't allocate floor_array \n"); return;};
    gl_level.door_array = malloc(door_count*num_face*sizeof(Textured_Face));
    if(gl_level.door_array == NULL){printf("\n couldn't allocate door_array \n"); return;};
}

void gl_build_vertex_array(void)
{
    int x, y;
    int wall_count = 0, wall_flat_count = 0, floor_count = 0, maze_count = 0, door_count = 0;
    for(y = 0; y < MAXLINES-3; y++)
    {
        for(x = 0; x < MAXCOLS; x ++)
        {
            switch(chat(y,x))
            {
                case VWALL:
                case HWALL:
                case ULWALL:
                case URWALL:
                case LLWALL:
                case LRWALL:wall_count++;wall_flat_count++;break;
                case PASSAGE:
                case FLOOR:floor_count++;break;
                case LHDOOR:
                case LVDOOR:
                case RVDOOR:
                //case DOOR:wall_count++;floor_count++;door_count++;break;
                case DOOR:wall_count+=4;floor_count++;door_count++,wall_flat_count+=8;break;
            }
            if(chat(y,x) == PASSAGE && (flat(y, x) & (F_MAZE|F_REAL)) == (F_MAZE|F_REAL))
                maze_count++;
        }
    }
    int num_face = 2;

    gl_malloc_level(wall_count, wall_flat_count, floor_count, door_count, num_face);

    Cube cube; gl_init_cube(&cube);

    int i = 0, j = 0, k = 0, n = 0;
    for(y = 0; y < MAXLINES-3; y++)
    {
        gl_load_cube(&cube.v, -1+start_pos, (1-start_pos)-(y*tile_size*2), 0);
        for(x = 0; x < MAXCOLS; x ++)
        {
            switch(chat(y,x))
            {
                case VWALL:
                    gl_copy_face_interleaved(&cube, &gl_level.wall_array[i*num_face], LEFT_FACE);
                    gl_copy_face_interleaved(&cube, &gl_level.wall_array[i*num_face+1], RIGHT_FACE);
                    i++;
                    gl_copy_face(&cube, &gl_level.wall_flat_array[n*num_face], TOP_FACE, 0);
                    gl_copy_face(&cube, &gl_level.wall_flat_array[n*num_face+1],BOTTOM_FACE, 0);
                    n++;
                    break;
                case HWALL:
                    gl_copy_face_interleaved(&cube, &gl_level.wall_array[i*num_face], FRONT_FACE);
                    gl_copy_face_interleaved(&cube, &gl_level.wall_array[i*num_face+1], BACK_FACE);
                    i++;
                    gl_copy_face(&cube, &gl_level.wall_flat_array[n*num_face], TOP_FACE, 0);
                    gl_copy_face(&cube, &gl_level.wall_flat_array[n*num_face+1],BOTTOM_FACE, 0);
                    n++;
                    break;
                case ULWALL:
                    gl_copy_face_interleaved(&cube, &gl_level.wall_array[i*num_face], BACK_FACE);
                    gl_copy_face_interleaved(&cube, &gl_level.wall_array[i*num_face+1], LEFT_FACE);
                    i++;
                    gl_copy_face(&cube, &gl_level.wall_flat_array[n*num_face], TOP_FACE, 0);
                    gl_copy_face(&cube, &gl_level.wall_flat_array[n*num_face+1],BOTTOM_FACE, 0);
                    n++;
                    break;
                case URWALL:
                    gl_copy_face_interleaved(&cube, &gl_level.wall_array[i*num_face], BACK_FACE);
                    gl_copy_face_interleaved(&cube, &gl_level.wall_array[i*num_face+1], RIGHT_FACE);
                    i++;
                    gl_copy_face(&cube, &gl_level.wall_flat_array[n*num_face], TOP_FACE, 0);
                    gl_copy_face(&cube, &gl_level.wall_flat_array[n*num_face+1],BOTTOM_FACE, 0);
                    n++;
                    break;
                case LLWALL:
                    gl_copy_face_interleaved(&cube, &gl_level.wall_array[i*num_face], FRONT_FACE);
                    gl_copy_face_interleaved(&cube, &gl_level.wall_array[i*num_face+1], LEFT_FACE);
                    i++;
                    gl_copy_face(&cube, &gl_level.wall_flat_array[n*num_face], TOP_FACE, 0);
                    gl_copy_face(&cube, &gl_level.wall_flat_array[n*num_face+1],BOTTOM_FACE, 0);
                    n++;
                    break;
                case LRWALL:
                    gl_copy_face_interleaved(&cube, &gl_level.wall_array[i*num_face], FRONT_FACE);
                    gl_copy_face_interleaved(&cube, &gl_level.wall_array[i*num_face+1], RIGHT_FACE);
                    i++;
                    gl_copy_face(&cube, &gl_level.wall_flat_array[n*num_face], TOP_FACE, 0);
                    gl_copy_face(&cube, &gl_level.wall_flat_array[n*num_face+1],BOTTOM_FACE, 0);
                    n++;
                    break;
                case PASSAGE:
                case FLOOR:
                    gl_copy_face_interleaved_reverse(&cube, &gl_level.floor_array[j], BOTTOM_FACE);
                    j++;
                    break;
                 case UHDOOR:
                    gl_door_entrance(&cube, &i, &n, &k, H_ENTRANCE);
                    //gl_copy_face_interleaved(&cube, &gl_level.door_array[k*num_face], BACK_FACE);
                    //k++;
                    gl_copy_face_interleaved(&cube, &gl_level.floor_array[j], BOTTOM_FACE);
                    j++;
                    gl_copy_face_interleaved_reverse(&cube, &gl_level.wall_array[i*num_face], LEFT_FACE);
                    gl_copy_face_interleaved_reverse(&cube, &gl_level.wall_array[i*num_face+1],RIGHT_FACE);
                    i++;
                    break;
                case LHDOOR:
                    gl_door_entrance(&cube, &i, &n, &k, H_ENTRANCE);
                    //gl_copy_face_interleaved(&cube, &gl_level.door_array[k*num_face], FRONT_FACE);
                    //k++;
                    gl_copy_face_interleaved(&cube, &gl_level.floor_array[j], BOTTOM_FACE);
                    j++;
                    gl_copy_face_interleaved_reverse(&cube, &gl_level.wall_array[i*num_face], LEFT_FACE);
                    gl_copy_face_interleaved_reverse(&cube, &gl_level.wall_array[i*num_face+1],RIGHT_FACE);
                    i++;
                    break;
                case LVDOOR:
                    gl_door_entrance(&cube, &i, &n, &k, V_ENTRANCE);
                    //gl_copy_face_interleaved(&cube, &gl_level.door_array[k*num_face], LEFT_FACE);
                    //k++;
                    gl_copy_face_interleaved(&cube, &gl_level.floor_array[j], BOTTOM_FACE);
                    j++;
                    gl_copy_face_interleaved_reverse(&cube, &gl_level.wall_array[i*num_face], FRONT_FACE);
                    gl_copy_face_interleaved_reverse(&cube, &gl_level.wall_array[i*num_face+1],BACK_FACE);
                    i++;
                    break;
                case RVDOOR:
                    gl_door_entrance(&cube, &i, &n, &k, V_ENTRANCE);
                    //gl_copy_face_interleaved(&cube, &gl_level.door_array[k*num_face], RIGHT_FACE);
                    //k++;
                    gl_copy_face_interleaved(&cube, &gl_level.floor_array[j], BOTTOM_FACE);
                    j++;
                    gl_copy_face_interleaved_reverse(&cube, &gl_level.wall_array[i*num_face], FRONT_FACE);
                    gl_copy_face_interleaved_reverse(&cube, &gl_level.wall_array[i*num_face+1],BACK_FACE);
                    i++;
                    break;
            }
            gl_translate_cube(&cube.v, tile_size*2, 0, 0);
        }
    }
}

void gl_draw_level(void)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, wall_texture);
    glColor3ub(255,255,255);
    glLoadIdentity();
    FreeFlyCameraLookAt(&freeFlyCamera);

    glInterleavedArrays(GL_T2F_V3F, 0, gl_level.wall_array);
    glDrawArrays(GL_QUADS, 0, gl_level.wall_vertices_count);

    glBindTexture(GL_TEXTURE_2D, floor_texture);
    glVertexPointer(3, GL_FLOAT, 5*sizeof(GLfloat), &(((GLfloat*)gl_level.floor_array)[2]));
    glTexCoordPointer(2, GL_FLOAT, 5*sizeof(GLfloat), gl_level.floor_array);
    glDrawArrays(GL_QUADS, 0, gl_level.floor_vertices_count);

    glBindTexture(GL_TEXTURE_2D, door_texture);
    glVertexPointer(3, GL_FLOAT, 5*sizeof(GLfloat), &(((GLfloat*)gl_level.door_array)[2]));
    glTexCoordPointer(2, GL_FLOAT, 5*sizeof(GLfloat), gl_level.door_array);
    glDrawArrays(GL_QUADS, 0, gl_level.door_vertices_count);

    glDisable(GL_TEXTURE_2D);
    glColor3ub(196,88,24);
    glInterleavedArrays(GL_V3F, 0, gl_level.wall_flat_array);
    glDrawArrays(GL_QUADS, 0, gl_level.wall_flat_vertices_count);

    //gl_draw_vertex_array(gl_level.wall_array, gl_level.wall_count, 2);

    glDisable(GL_TEXTURE_2D);
}

