/*
 * Create the layout for the new level
 *
 * rooms.c	1.4 (A.I. Design)	12/16/84
 */

//#include "rogue.h"
//#include "curses.h"
#include "rogue_maze.h"

byte _level[(MAXLINES-3)*MAXCOLS];
byte _flags[(MAXLINES-3)*MAXCOLS];

int level = 100;//1;			/* What level rogue is on */
int maxrow = 24;//23;			/* Last Line used for map */
int LINES=25, COLS=80;      /* Globals for curses */

SDL_Rect tile_rect = {0, 0, TILE_SIZE, TILE_SIZE};
SDL_Rect background_rect = {0, 0, MAXCOLS*TILE_SIZE, (MAXLINES-3)*TILE_SIZE};

/*
 * rnd:
 *	Pick a very random number.
 */
int rnd(int range)
{
    return range < 1 ? 0 : ((rand() + rand())&0x7fffffffl) % range;
}

/*
 * rnd_room:
 *	Pick a room that is really there
 */
int rnd_room()
{
    int rm;

    do
        rm = rnd(MAXROOMS);
    while (!((rooms[rm].r_flags & ISGONE)==0||(rooms[rm].r_flags & ISMAZE)));
    return rm;
}

/*
 * do_rooms:
 *	Create rooms and corridors with a connectivity graph
 */
void do_rooms(void)
{
    int i, rm;
    struct room *rp;
    //register THING *tp;
    int left_out;
    coord top;
    coord bsze;
    //coord mp;
    //int old_lev;
    int endline;

	endline = maxrow + 1;

    //old_lev = level;
    /*
     * bsze is the maximum room size
     */
    bsze.x = COLS/3;
    bsze.y = endline/3;
    /*
     * Clear things for a new level
     */
    for (rp = rooms; rp < &rooms[MAXROOMS]; rp++)
		rp->r_goldval = rp->r_nexits = rp->r_flags = 0;
    /*
     * Put the gone rooms, if any, on the level
     */
    left_out = rnd(4); //rooms[4].r_flags |= ISGONE | ISMAZE;
    for (i = 0; i < left_out; i++) {
		do
		    rp = &rooms[(rm = rnd_room())];
		while (rp->r_flags & ISMAZE);
		rp->r_flags |= ISGONE;

		if (rm > 2 && level > 10 && rnd(20) < level - 9)
		    rp->r_flags |= ISMAZE;
    }
    /*
     * dig and populate all the rooms on the level
     */
    for (i = 0, rp = rooms; i < MAXROOMS; rp++, i++) {
		/*
		 * Find upper left corner of box that this room goes in
		 */
		top.x = (i%3)*bsze.x + 1;
		top.y = i/3*bsze.y;
		if (rp->r_flags & ISGONE) {
		    /*
		     * If the gone room is a maze room, draw the maze and set the
		     * size equal to the maximum possible.
		     */
		    if (rp->r_flags&ISMAZE) {
				rp->r_pos.x = top.x;
				rp->r_pos.y = top.y;
				draw_maze(rp);
		    } else {
				/*
				 * Place a gone room.  Make certain that there is a blank line
				 * for passage drawing.
				 */
				do {
				    rp->r_pos.x = top.x + rnd(bsze.x-2) + 1;
				    rp->r_pos.y = top.y + rnd(bsze.y-2) + 1;
				    rp->r_max.x = -COLS;
				    rp->r_max.y = -endline;
				} while (!(rp->r_pos.y > 0 && rp->r_pos.y < endline-1));
		    }
		    continue;
		}
		if (rnd(10) < (level - 1))
		    rp->r_flags |= ISDARK;
		/*
		 * Find a place and size for a random room
		 */
		do {
		    rp->r_max.x = rnd(bsze.x - 4) + 4;
		    rp->r_max.y = rnd(bsze.y - 4) + 4;
		    rp->r_pos.x = top.x + rnd(bsze.x - rp->r_max.x);
		    rp->r_pos.y = top.y + rnd(bsze.y - rp->r_max.y);
		    //printf("y=%d %d \n",(rp->r_pos.y+rp->r_max.y),endline-1);
		} while (rp->r_pos.y == 0 || (rp->r_pos.y+rp->r_max.y >= endline-1));
		draw_room(rp);
    }
}

/*
 * draw_room:
 *	Draw a box around a room and lay down the floor
 */
void draw_room(struct room *rp)
{
    int y, x;

    /*
     * Here we draw normal rooms, one side at a time
     */
    vert(rp, rp->r_pos.x);			/* Draw left side */
    vert(rp, rp->r_pos.x + rp->r_max.x - 1);	/* Draw right side */
    horiz(rp, rp->r_pos.y);			/* Draw top */
    horiz(rp, rp->r_pos.y + rp->r_max.y - 1);	/* Draw bottom */
	chat(rp->r_pos.y,rp->r_pos.x) = ULWALL;
	chat(rp->r_pos.y,rp->r_pos.x+rp->r_max.x - 1) = URWALL;
	chat(rp->r_pos.y+rp->r_max.y-1,rp->r_pos.x) = LLWALL;
	chat(rp->r_pos.y+rp->r_max.y-1,rp->r_pos.x+rp->r_max.x - 1) = LRWALL;

    /*
     * Put the floor down
     */
    for (y = rp->r_pos.y + 1; y < rp->r_pos.y + rp->r_max.y - 1; y++)
		for (x = rp->r_pos.x + 1; x < rp->r_pos.x + rp->r_max.x - 1; x++)
		    chat(y, x) = FLOOR;
}

/*
 * vert:
 *	Draw a vertical line
 */
void vert(struct room *rp, int startx)
{
    int y;

    for (y = rp->r_pos.y + 1; y <= rp->r_max.y + rp->r_pos.y - 1; y++)
		chat(y, startx) = VWALL;
}

/*
 * horiz:
 *	Draw a horizontal line
 */
void horiz(struct room *rp, int starty)
{
    int x;

    for (x = rp->r_pos.x; x <= rp->r_pos.x + rp->r_max.x - 1; x++)
		chat(starty, x) = HWALL;
}

/*
 * rnd_pos:
 *	Pick a random spot in a room
 */
void rnd_pos(struct room *rp, coord *cp)
{
    cp->x = rp->r_pos.x + rnd(rp->r_max.x - 2) + 1;
    cp->y = rp->r_pos.y + rnd(rp->r_max.y - 2) + 1;
}

void render_rooms(SDL_Surface *screen)
{
    int x, y; //coord rpos = rooms[4].r_pos;
    //int ymax = rpos.y  + (maxrow + 1)/3,  xmax= rpos.x + COLS/3;
    SDL_FillRect(screen, &background_rect, SDL_MapRGB(screen->format, 17, 206, 112));

    for(y = 0; y < MAXLINES-3; y++)
    {
        for(x = 0; x < MAXCOLS; x ++)
        {
            tile_rect.x = x*5; tile_rect.y = y*5;
            //if(y >= rpos.y && y < ymax && x >= rpos.x && x < xmax)
                //SDL_FillRect(screen, &tile_rect, SDL_MapRGB(screen->format, 255, 255, 0));
            switch(chat(y,x))
            {
                case VWALL: SDL_FillRect(screen, &tile_rect, SDL_MapRGB(screen->format, 0, 0, 255));break;
                case HWALL: SDL_FillRect(screen, &tile_rect, SDL_MapRGB(screen->format, 255, 0, 0));break;
                case ULWALL: SDL_FillRect(screen, &tile_rect, SDL_MapRGB(screen->format, 0, 255, 0));break;
                case URWALL: SDL_FillRect(screen, &tile_rect, SDL_MapRGB(screen->format, 255, 255, 0));break;
                case LLWALL: SDL_FillRect(screen, &tile_rect, SDL_MapRGB(screen->format, 255, 0, 255));break;
                case LRWALL: SDL_FillRect(screen, &tile_rect, SDL_MapRGB(screen->format, 0, 255, 255));break;
                case FLOOR: SDL_FillRect(screen, &tile_rect, SDL_MapRGB(screen->format, 255, 255, 255));break;

                /*case VWALL:
                case HWALL:
                case ULWALL:
                case URWALL:
                case LLWALL:
                case LRWALL:
                case FLOOR:

                //case 1:case 2:case 3:case 5:case 6:case 7:case 8:case 9:
                    //printf("O");
                    printf("%d",chat(y,x));
                    tile_rect.x = x*5; tile_rect.y = y*5;
                    SDL_FillRect(screen, &tile_rect, SDL_MapRGB(screen->format, 255, 255, 255));
                    break;*/
                default:
                    //printf("-");
                    break;
            }
            //printf("%d ",INDEX(y,x));
        }
        //printf("\n");
    }
}
