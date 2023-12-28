#include <stdlib.h>
#include <stdio.h>
#include <SDL/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "camera.h"

#define abs(x) ((x)<0?-(x):(x))
//#define NULL	0
#define TRUE 	1
#define	FALSE	0

/* flags for rooms */
#define ISDARK	 0x0001		/* room is dark */
#define ISGONE	 0x0002		/* room is gone (a corridor) */
#define	ISMAZE	 0x0004		/* room is a maze */

/*
 * Things that appear on the screens
 */
#define PASSAGE		(0xb1)
#define DOOR		(0xce)
#define FLOOR		(0xfa)
#define VWALL	(0xba)
#define HWALL	(0xcd)
#define ULWALL	(0xc9)
#define URWALL	(0xbb)
#define LLWALL	(0xc8)
#define LRWALL	(0xbc)

#define UHDOOR	(0xce)
#define LHDOOR	(0xde)
#define RVDOOR	(0xee)
#define LVDOOR  (0xfe)

#define MAXLINES	27//25	/* maximum number of screen lines used */
#define MAXCOLS		80	/* maximum number of screen columns used */

/*
 * Flags for level map
 */
#define F_PASS		0x040		/* is a passageway */
#define F_MAZE		0x020		/* have seen this corridor before */
#define F_REAL		0x010		/* what you see is what you get */
#define F_PNUM		0x00f		/* passage number mask */
#define F_TMASK		0x007		/* trap number mask */

typedef unsigned char byte;

extern byte _level[(MAXLINES-3)*MAXCOLS];
extern byte _flags[(MAXLINES-3)*MAXCOLS];

#define INDEX(y,x) ((x) + ((y) * MAXCOLS))

/*
 * All the fun defines
 */
#define shint		int		/* short integer (for very small #s) */
#define when		break;case
#define otherwise	break;default
#define chat(y,x)	(_level[INDEX(y,x)])
#define flat(y,x)	(_flags[INDEX(y,x)])
#define ce(a,b)		((a).x == (b).x && (a).y == (b).y)

extern int level;				/* What level rogue is on */
extern int maxrow;			/* Last Line used for map */
extern int COLS;      /* Globals for curses */

/*
 * Coordinate data type
 */
typedef struct {
    shint x;
    shint y;
} coord;

/*
 * Room structure
 */
struct room {
    coord r_pos;			/* Upper left corner */
    coord r_max;			/* Size of room */
    coord r_gold;			/* Where the gold is */
    int r_goldval;			/* How much the gold is worth */
    short r_flags;			/* Info about the room */
    shint r_nexits;			/* Number of exits */
    coord r_exit[12];		/* Where the exits are */
};

#define MAXROOMS	9

struct room rooms[MAXROOMS];		/* One for each room -- A level */

#define GOLDGRP 1

// from room.c
int rnd(int range);
int rnd_room(void);
void draw_room(struct room *rp);
void vert(struct room *rp, int startx);
void horiz(struct room *rp, int starty);
void rnd_pos(struct room *rp, coord *cp);

// from passages.c
void psplat(int y, int x);
void door(struct room *rm, coord *cp);
byte door_pos(struct room *rm, coord *cp);

// from maze.c
void draw_maze(struct room *rp);
void new_frontier(int y, int x);
void add_frnt(int y, int x);
void con_frnt(void);
int maze_at(int y, int x);
void splat(int y, int x);
int inrange(int y, int x);
int offmap(int y, int x);

//from opegl_maze.c
void gl_test_old(void);
void gl_cube(void);
void gl_init(void);
void gl_quad(void);
void gl_color_cube(GLubyte* color);
void gl_color_quad(GLubyte* color);
void gl_draw_level_surface(void);
void gl_texture_test(void);
void gl_init_textures(void);
void gl_wall(void);
void gl_floor(void);
void gl_door(byte pos);
void gl_vertex_array_test(void);
void gl_init_vertex_array(void);
void gl_build_vertex_array(void);
void gl_free_level(void);
void gl_draw_level(void);



#define TILE_SIZE 5
extern SDL_Rect tile_rect;
extern SDL_Rect background_rect;

extern FreeFlyCamera freeFlyCamera;
extern TrackBallCamera trackBallCamera;
extern GLdouble tile_size, start_pos;
