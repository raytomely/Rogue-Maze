
/*
 * Draw the connecting passages
 *
 * passages.c	1.4 (A.I. Design)	12/14/84
 */

//#include "rogue.h"
//#include "curses.h"
#include "rogue_maze.h"


/*
 * conn:
 *	Draw a corridor from a room in a certain direction.
 */
void conn(int r1, int r2)
{
	struct room *rpf, *rpt;
	int rmt, rm;
	int distance, turn_spot, turn_distance;//, index;
	int direc;
	coord del, curr, turn_delta, spos, epos;

	if (r1 < r2) {
		rm = r1;
		if (r1 + 1 == r2)
			direc = 'r';
		else
			direc = 'd';
	} else {
		rm = r2;
		if (r2 + 1 == r1)
			direc = 'r';
		else
			direc = 'd';
    }
	rpf = &rooms[rm];
	/*
	 * Set up the movement variables, in two cases:
	 * first drawing one down.
	 */
	if (direc == 'd') {
		rmt = rm + 3;				/* room # of dest */
		rpt = &rooms[rmt];			/* room pointer of dest */
		del.x = 0;				/* direction of move */
		del.y = 1;
		/*
		 * If we are drawing from/to regular or maze rooms, we have
		 * to pick the spot we draw from/to
		 */
		if ((rpf->r_flags & ISGONE) == 0 || (rpf->r_flags & ISMAZE)) {
			spos.y = rpf->r_pos.y + rpf->r_max.y - 1;
			do {
				spos.x = rpf->r_pos.x + rnd(rpf->r_max.x - 2) + 1;
			} while (chat(spos.y,spos.x) == ' ');
		} else {
			spos.x = rpf->r_pos.x;
			spos.y = rpf->r_pos.y;
		}
		epos.y = rpt->r_pos.y;
		if ((rpt->r_flags & ISGONE) == 0 || (rpt->r_flags & ISMAZE)) {
			do {
				epos.x = rpt->r_pos.x + rnd(rpt->r_max.x - 2) + 1;
			} while (chat(epos.y,epos.x) == ' ');
		} else
			epos.x = rpt->r_pos.x;
		distance = abs(spos.y - epos.y) - 1;	/* distance to move */
		turn_delta.y = 0;			/* direction to turn */
		turn_delta.x = (spos.x < epos.x ? 1 : -1);
		turn_distance = abs(spos.x - epos.x);	/* how far to turn */
	} else if (direc == 'r') {			/* setup for moving right */
		rmt = rm + 1;
		rpt = &rooms[rmt];
		del.x = 1;
		del.y = 0;
		if ((rpf->r_flags & ISGONE) == 0 || (rpf->r_flags & ISMAZE)) {
			spos.x = rpf->r_pos.x + rpf->r_max.x-1;
			do {
				spos.y = rpf->r_pos.y + rnd(rpf->r_max.y-2)+1;
			} while (chat(spos.y,spos.x) == ' ');
		} else {
			spos.x = rpf->r_pos.x;
			spos.y = rpf->r_pos.y;
		}
		epos.x = rpt->r_pos.x;
		if ((rpt->r_flags & ISGONE) == 0 || (rpt->r_flags & ISMAZE)) {
			do {
				epos.y = rpt->r_pos.y + rnd(rpt->r_max.y-2)+1;
			} while (chat(epos.y, epos.x) == ' ');
		} else
			epos.y = rpt->r_pos.y;
		distance = abs(spos.x - epos.x) - 1;
		turn_delta.y = (spos.y < epos.y ? 1 : -1);
		turn_delta.x = 0;
		turn_distance = abs(spos.y - epos.y);
	}

    turn_spot = rnd(distance-1) + 1;
    /*
     * Draw in the doors on either side of the passage or just put #'s
     * if the rooms are gone.
     */
    if (!(rpf->r_flags & ISGONE))
	door(rpf, &spos);
    else
	psplat(spos.y, spos.x);
    if (!(rpt->r_flags & ISGONE))
	door(rpt, &epos);
    else
	psplat(epos.y, epos.x);
    /*
     * Get ready to move...
     */
    curr.x = spos.x;
    curr.y = spos.y;
    while (distance)
    {
	/*
	 * Move to new position
	 */
	curr.x += del.x;
	curr.y += del.y;
	/*
	 * Check if we are at the turn place, if so do the turn
	 */
	if (distance == turn_spot)
	{
	    while (turn_distance--)
	    {
		psplat(curr.y, curr.x);
		curr.x += turn_delta.x;
		curr.y += turn_delta.y;
	    }
	}
	/*
	 * Continue digging along
	 */
	psplat(curr.y, curr.x);
	distance--;
    }
    curr.x += del.x;
    curr.y += del.y;
    if (!ce(curr, epos)) {
	epos.x -= del.x;
	epos.y -= del.y;
	psplat(epos.y, epos.x);
    }
}

/*
 * do_passages:
 *	Draw all the passages on a level.
 */
void do_passages(void)
{
    int i, j;
    int roomcount;
    static struct rdes
    {
	char	conn[MAXROOMS];		/* possible to connect to room i? */
	char	isconn[MAXROOMS];	/* connection been made to room i? */
	char	ingraph;		/* this room in graph already? */
    } rdes[MAXROOMS] = {
	{ { 0, 1, 0, 1, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 },
	{ { 1, 0, 1, 0, 1, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 },
	{ { 0, 1, 0, 0, 0, 1, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 },
	{ { 1, 0, 0, 0, 1, 0, 1, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 },
	{ { 0, 1, 0, 1, 0, 1, 0, 1, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 },
	{ { 0, 0, 1, 0, 1, 0, 0, 0, 1 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 },
	{ { 0, 0, 0, 1, 0, 0, 0, 1, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 },
	{ { 0, 0, 0, 0, 1, 0, 1, 0, 1 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 },
	{ { 0, 0, 0, 0, 0, 1, 0, 1, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 }
    };
    struct rdes *r1 = NULL, *r2 = NULL ;

    /*
     * reinitialize room graph description
     */
    for (r1 = rdes; r1 < &rdes[MAXROOMS]; r1++)
    {
	for (j = 0; j < MAXROOMS; j++)
	    r1->isconn[j] = FALSE;
	r1->ingraph = FALSE;
    }

    /*
     * starting with one room, connect it to a random adjacent room and
     * then pick a new room to start with.
     */
    roomcount = 1;
    r1 = &rdes[rnd(MAXROOMS)];
    r1->ingraph = TRUE;
    do
    {
	/*
	 * find a room to connect with
	 */
	j = 0;
	for (i = 0; i < MAXROOMS; i++)
	    if (r1->conn[i] && !rdes[i].ingraph && rnd(++j) == 0)
		r2 = &rdes[i];
	/*
	 * if no adjacent rooms are outside the graph, pick a new room
	 * to look from
	 */
	if (j == 0)
	{
	    do
		r1 = &rdes[rnd(MAXROOMS)];
	    while (!r1->ingraph);
	}
	/*
	 * otherwise, connect new room to the graph, and draw a tunnel
	 * to it
	 */
	else
	{
	    r2->ingraph = TRUE;
	    i = r1 - rdes;
	    j = r2 - rdes;
	    conn(i, j);
	    r1->isconn[j] = TRUE;
	    r2->isconn[i] = TRUE;
	    roomcount++;
	}
    } while (roomcount < MAXROOMS);

    /*
     * attempt to add passages to the graph a random number of times so
     * that there isn't always just one unique passage through it.
     */
    for (roomcount = rnd(5); roomcount > 0; roomcount--)
    {
	r1 = &rdes[rnd(MAXROOMS)];	/* a random room to look from */
	/*
	 * find an adjacent room not already connected
	 */
	j = 0;
	for (i = 0; i < MAXROOMS; i++)
	    if (r1->conn[i] && !r1->isconn[i] && rnd(++j) == 0)
		r2 = &rdes[i];
	/*
	 * if there is one, connect it and look for the next added
	 * passage
	 */
	if (j != 0)
	{
	    i = r1 - rdes;
	    j = r2 - rdes;
	    conn(i, j);
	    r1->isconn[j] = TRUE;
	    r2->isconn[i] = TRUE;
	}
    }
    //passnum();
}


/*
 * door:
 *	Add a door or possibly a secret door.  Also enters the door in
 *	the exits array of the room.
 */
void door(struct room *rm, coord *cp)
{
    int index, xit;

    index = INDEX(cp->y, cp->x);
    if (rnd(10) + 1 < level && rnd(5) == 0)
    {
	_level[index] = (cp->y == rm->r_pos.y || cp->y == rm->r_pos.y + rm->r_max.y - 1) ? HWALL : VWALL;
	_flags[index] &= ~F_REAL;//printf("real\n");
    }
    else{
	_level[index] = DOOR;
    _level[index] = door_pos(rm, cp);}//printf("door\n");}
    xit = rm->r_nexits++;
    rm->r_exit[xit].y = cp->y;
    rm->r_exit[xit].x = cp->x;
}

void psplat(int y, int x)
{
    int idx;

    _level[idx = INDEX(y, x)] = PASSAGE;
    _flags[idx] |= F_PASS;
}

byte door_pos(struct room *rm, coord *cp)
{
    byte pos = 0;
    //printf("cp_y=%d cp_x=%d;  rm_y=%d rm_x=%d;  rm_max_y=%d rm_max_x=%d;\n",
    //                    cp->y,cp->x,rm->r_pos.y,rm->r_pos.x,rm->r_max.y,rm->r_max.x);
    if (cp->y == rm->r_pos.y) {pos=UHDOOR;}// printf("UHDOOR\n");}
    else if (cp->y == rm->r_pos.y + rm->r_max.y - 1) {pos=LHDOOR;}// printf("LHDOOR\n");}
    else if (cp->x == rm->r_pos.x) {pos=LVDOOR;}// printf("LVDOOR\n");}
    else if (cp->x == rm->r_pos.x + rm->r_max.x - 1) {pos=RVDOOR;}// printf("RVDOOR\n");}
    return pos;
    //pos = (cp->y == rm->r_pos.y || cp->y == rm->r_pos.y + rm->r_max.y - 1) ? UHDOOR : LVDOOR;
    //printf("pos=%d\n",pos);
    //return pos;
}

void render_passages(SDL_Surface *screen)
{
    int x, y;

    //GLdouble tile_size = (GLdouble)1/640*32, start_pos = (GLdouble)1/640*100;
    //gl_init();
    glColor3ub(0,255,0);glTranslated(-1+start_pos,1-start_pos,0);
    glLoadIdentity();
    FreeFlyCameraLookAt(&freeFlyCamera);
    //glDisable(GL_DEPTH_TEST);

    for(y = 0; y < MAXLINES-3; y++)
    {
        glLoadIdentity();
        FreeFlyCameraLookAt(&freeFlyCamera);
        glTranslated(-1+start_pos,(1-start_pos)-(y*tile_size*2),0);

        for(x = 0; x < MAXCOLS; x ++)
        {
            tile_rect.x = x*5; tile_rect.y = y*5;

            switch(chat(y,x))
            {
                //case PASSAGE: SDL_FillRect(screen, &tile_rect, SDL_MapRGB(screen->format, 255, 255, 255));break;
                //case DOOR: SDL_FillRect(screen, &tile_rect, SDL_MapRGB(screen->format, 255, 128, 0));break;
                case PASSAGE:gl_floor();break;//{GLubyte color[3]={255,255,255};gl_color_quad(color);break;}
                case LHDOOR:
                case LVDOOR:
                case RVDOOR:
                //case DOOR:{GLubyte color[3]={255,182,108};gl_color_cube(color);break;}
                case DOOR:gl_floor();gl_door(chat(y,x));break;//{GLubyte color[3]={255,182,108};gl_color_cube(color);break;}
            }
            glTranslated(tile_size*2,0,0);
        }
    }
    //glFlush();
    //SDL_GL_SwapBuffers();
}


