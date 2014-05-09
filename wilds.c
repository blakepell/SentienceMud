/* Author : Vizzini (sc0jack@yahoo.co.uk)
 * Wilds v2.x
 *
 * History:
 *
 * v2.x - Load/save wilds. Encapsulated wilds memory and area data structures.
 * v1.x - Load only wilds. The "just get something working" version.
 * v0.x - Initial alphas. Unstable and buggy. Testing concepts.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"
#include "recycle.h"
#include "wilds.h"
#include "olc_save.h"

/* external global variables */
extern bool fBootDb;
extern GLOBAL_DATA gconfig;

/* internal global variables */
long top_wilds;
long top_wilds_terrain;
long top_wilds_vlink;

/* These are pointers to linked lists of "freed" (but still allocated) structures.
   They are used in the routines as "heaps" of perms. In game, it is often more
   efficient performance-wise to not actually "free" memory allocated, but to store
   it on a heap of allocated structures to save time on reallocating later. This is
   known as "recycling" of data structures */
WILDS_DATA *wilds_free;
WILDS_VLINK *wilds_vlink_free;
WILDS_TERRAIN *wilds_terrain_free;


/* external routines */
extern EXIT_DATA *new_exit args ((void));
extern void free_exit args ((EXIT_DATA *pExit));
extern char *fix_string args ((const char *str));
extern char *fwrite_flag args ((long flags, char buf[]));
extern ROOM_INDEX_DATA *new_room_index args ((void));
extern void free_room_index args ((ROOM_INDEX_DATA * pRoomIndex));
extern void fwrite_room args ((FILE *fp, ROOM_INDEX_DATA *pRoomIndex));
extern AREA_DATA *get_area_from_uid args ((long uid));
extern ROOM_INDEX_DATA *read_room_new(FILE *fp, AREA_DATA *area, int roomtype);

/* local routines */
int		get_squares_to_show_x args ((int bonus_view));
int		get_squares_to_show_y args ((int bonus_view));
bool		map_char_cmp args ((WILDS_DATA *pWilds, int x, int y, char *check));
bool		check_for_bad_room args ((WILDS_DATA *pWilds, int x, int y));
ROOM_INDEX_DATA *create_vroom args ((WILDS_DATA *pWilds,
                                     int x, int y,
                                     WILDS_TERRAIN *pTerrain));
void link_vroom(ROOM_INDEX_DATA *pWildsRoom);
void            load_wilds args ((FILE *fp, AREA_DATA *area));
WILDS_DATA      *new_wilds args ((void));
void            free_wilds args ((WILDS_DATA *pWilds));
WILDS_DATA      *get_wilds_from_uid args ((AREA_DATA *pArea, long uid));
void            char_to_vroom args ((CHAR_DATA *ch, WILDS_DATA *pWilds, int x, int y));
ROOM_INDEX_DATA *get_wilds_vroom args ((WILDS_DATA *pWilds, int x, int y));
int             get_wilds_vroom_x_by_dir args ((WILDS_DATA *pWilds, int x, int y, int door));
int             get_wilds_vroom_y_by_dir args ((WILDS_DATA *pWilds, int x, int y, int door));
void            do_vlinks args ((CHAR_DATA *ch, char *argument));
WILDS_VLINK     *NEW_Vlink args ((void));
WILDS_VLINK     *fread_vlink args ((FILE *fp));
WILDS_VLINK     *get_vlink_from_uid args((WILDS_DATA *pWilds, long uid));
WILDS_VLINK     *get_vlink_from_index args((WILDS_DATA *pWilds, long index));
void            free_vlink args ((WILDS_VLINK *pVLink));
void            add_vlink args ((WILDS_DATA *pWilds, WILDS_VLINK *pVLink));
void            del_vlink args ((WILDS_DATA *pWilds, WILDS_VLINK *pVLink));
bool            link_vlink args ((WILDS_VLINK *pVLink));
void		fix_vlinks (void);
void            link_vlinks args ((WILDS_DATA *pWilds));
WILDS_TERRAIN   *new_terrain args ((WILDS_DATA *pWilds));
WILDS_TERRAIN   *fread_terrain args ((FILE *fp, WILDS_DATA *pWilds));
void            fwrite_terrain args ((FILE *fp, WILDS_TERRAIN *pTerrain));
void            free_terrain args ((WILDS_TERRAIN *pTerrain));
bool		add_terrain args ((WILDS_DATA *pWilds, WILDS_TERRAIN *pTerrain));
bool		del_terrain args ((WILDS_DATA *pWilds, WILDS_TERRAIN *pTerrain));
bool		check_terrain_exists args ((WILDS_DATA *pWilds, char token));


int dir_offsets[MAX_DIR][2] = {
	{0, -1},
	{1, 0},
	{0, 1},
	{-1, 0},
	{0, 0},
	{0, 0},
	{1, -1},
	{-1, -1},
	{1, 1},
	{-1, 1},
};

char *vlinkage_bit_name(int vlinkage)
{
    static char buf[512];

    buf[0] = '\0';

    if ((vlinkage & VLINK_TO_WILDS) && (vlinkage & VLINK_FROM_WILDS))
        strcat(buf, " two_way");
    else
        if (vlinkage & VLINK_TO_WILDS    ) strcat(buf, " to_wilds");
        else
            if (vlinkage & VLINK_FROM_WILDS  ) strcat(buf, " from_wilds");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}


void fix_vlinks(void)
{
    AREA_DATA *pArea;
    WILDS_DATA *pWilds;

    // Loop thru all loaded areas
    for (pArea = area_first; pArea; pArea = pArea->next)
    {
        // Does the area contain wilds sectors?
        if (pArea->wilds != NULL)
        {
            // Loop thru all wilds sectors
            for (pWilds = pArea->wilds; pWilds; pWilds = pWilds->next)
            {
                // Call mid-level function to link all vlinks in the wilds
                link_vlinks(pWilds);
            }
        }
    }

    return;
}

int get_squares_to_show_x(int bonus_view)
{
    int squares_to_show_x;

    squares_to_show_x = 10;

    switch (weather_info.sky)
    {
        case SKY_CLOUDLESS:
            squares_to_show_x = 16 + bonus_view;
            break;
        case SKY_CLOUDY:
            squares_to_show_x = 14 + bonus_view;
            break;
        case SKY_RAINING:
            squares_to_show_x = 12 + bonus_view;
            break;
        case SKY_LIGHTNING:
            squares_to_show_x = 10 + bonus_view;
            break;
    }

    if (weather_info.sky == SUN_SET)
        squares_to_show_x -= 1;

    if (weather_info.sky == SUN_DARK)
        squares_to_show_x -= 3;

    return squares_to_show_x;
}

int get_squares_to_show_y(int bonus_view)
{
    int squares_to_show_y;

    squares_to_show_y = 10;

    switch (weather_info.sky)
    {
        case SKY_CLOUDLESS:
            squares_to_show_y = 10 + bonus_view;
            break;
        case SKY_CLOUDY:
            squares_to_show_y = 10 + bonus_view;
            break;
        case SKY_RAINING:
            squares_to_show_y = 8 + bonus_view;
            break;
        case SKY_LIGHTNING:
            squares_to_show_y = 8 + bonus_view;
            break;
    }

    if (weather_info.sky == SUN_SET)
        squares_to_show_y -= 1;

    if (weather_info.sky == SUN_DARK)
        squares_to_show_y -= 3;

    return squares_to_show_y;
}

bool map_char_cmp(WILDS_DATA *pWilds, int x, int y, char *check)
{
    char map_char[2];

    // Get character on the map at these coordinates
    if ( x > pWilds->map_size_x )
        sprintf(map_char, "%c",
                pWilds->map[y * pWilds->map_size_x + x - pWilds->map_size_x]);
    else
        sprintf(map_char, "%c", pWilds->map[y * pWilds->map_size_x + x]);

    if (!str_cmp(map_char, check))
        return TRUE;
    else
        return FALSE;

}

bool check_for_bad_room(WILDS_DATA *pWilds, int x, int y)
{
    WILDS_TERRAIN *pTerrain;

    pTerrain = get_terrain_by_coors(pWilds, x, y);

    if (!pTerrain || pTerrain->nonroom)
        return FALSE;
    else
        return TRUE;

}

ROOM_INDEX_DATA *create_vroom(WILDS_DATA *pWilds,
                              int x, int y,
                              WILDS_TERRAIN *pTerrain)
{
    ROOM_INDEX_DATA *pRoomIndex;
    WILDS_VLINK *pVLink;
    sh_int door;


// First check pointer parameters are valid
    if ( !pTerrain )
    {
        plogf("wilds.c, create_vroom(): pTerrain is NULL");
        abort();
    }

    if ( !pTerrain->template )
    {
        plogf("wilds.c, create_vroom(): pTerrain->template is NULL");
        abort();
    }

// Create a new room structure and load it up with sensible defaults
    pRoomIndex                  = new_room_index();
    pRoomIndex->area            = pWilds->pArea;
    pRoomIndex->wilds		= pWilds;
    pRoomIndex->vnum            = 0; /* Vizz - Wilds v2 doesn't use vnums */
    pRoomIndex->x               = x;
    pRoomIndex->y               = y;
    pRoomIndex->z		= 0;	// Fix to later to deal with heights
    pRoomIndex->name            = str_dup(pTerrain->template->name);
    pRoomIndex->description     = NULL;
    pRoomIndex->owner           = NULL;
    pRoomIndex->room_flags      = pTerrain->template->room_flags;
    pRoomIndex->room2_flags	= pTerrain->template->room2_flags|ROOM_VIRTUAL_ROOM;
    pRoomIndex->sector_type     = pTerrain->template->sector_type;
    pRoomIndex->parent_template = pTerrain;
    pRoomIndex->heal_rate       = 100;
    pRoomIndex->mana_rate       = 100;

// Check if there are already vrooms loaded - if so, simply add this one to the existing list
    if (pWilds->loaded_vroom != NULL)
        pRoomIndex->next = pWilds->loaded_vroom;

    pWilds->loaded_vroom = pRoomIndex;
    pWilds->loaded_rooms++;
    top_wilds_vroom++;

    for ( door = 0; door < MAX_DIR; door++ )
        pRoomIndex->exit[door] = NULL;

// Check for vlinks in this room before we create the exits
    for (pVLink = pWilds->pVLink; pVLink ; pVLink = pVLink->next)
    {
        if (pRoomIndex->x == pVLink->wildsorigin_x
            && pRoomIndex->y == pVLink->wildsorigin_y
	    && IS_SET(pVLink->default_linkage, VLINK_FROM_WILDS))
        {
            link_vlink(pVLink);
        }
    }

	// Nib - replaced the ugly repetitive code with this SAL (Simple Ass Loop) (tm)...
	link_vroom(pRoomIndex);

    return (pRoomIndex);
}

void destroy_wilds_vroom(ROOM_INDEX_DATA *pRoomIndex)
{
    WILDS_DATA *pWilds;
    ROOM_INDEX_DATA *pRoom;
    ROOM_INDEX_DATA *pLast_room;
    bool found = FALSE;

// Check pointer parameter is valid
    if (!pRoomIndex)
    {
        plogf("wilds.c, destroy_wilds_vroom(): pRoomIndex is NULL.");
        return;
    }

    pWilds = pRoomIndex->wilds;

    if (!pWilds)
    {
        plogf("wilds.c, destroy_wilds_vroom(): pRoomIndex->wilds is NULL.");
        return;
    }

    pLast_room = NULL;
    for (pRoom = pWilds->loaded_vroom;pRoom;pRoom = pRoom->next)
    {
        if (pRoomIndex == pRoom)
        {
            found = TRUE;
            break;
        }

        pLast_room = pRoom;
    }

    if (found)
    {
        if (pLast_room == NULL)
            pWilds->loaded_vroom = pRoom->next;
        else
            pLast_room->next = pRoom->next;

        --pWilds->loaded_rooms;
        free_room_index(pRoom);
    }
    else
        plogf("wilds.c, destroy_wilds_vroom(): FAILURE - Could not find vroom to destroy.");

    return;
}

char *allocate_wildsmap(int map_size_x, int map_size_y)
{
    char *tempmap;

// simply allocates enough space in memory to hold the specified wilds map array
    tempmap = calloc(sizeof(char), (map_size_x * map_size_y + 1));
    return (tempmap);
}

CHAR_DATA *allocate_char_matrix(int map_size_x, int map_size_y)
{
    CHAR_DATA *tempmap;

// simply allocates enough space in memory to hold the specified wilds map array
    tempmap = calloc(sizeof(CHAR_DATA *), (map_size_x * map_size_y ));
    return (tempmap);
}

OBJ_DATA *allocate_obj_matrix(int map_size_x, int map_size_y)
{
    OBJ_DATA *tempmap;

// simply allocates enough space in memory to hold the specified wilds map array
    tempmap = calloc(sizeof(OBJ_DATA *), (map_size_x * map_size_y ));
    return (tempmap);
}

void load_wilds( FILE *fp, AREA_DATA *pArea )
{

    WILDS_DATA *pWilds, *pLastWilds;
    WILDS_VLINK *temp_pVLink;
    WILDS_TERRAIN *pTerrain;
    long arraysize = 0;
    char      *word;
    bool      fMatch;
    int       y,j;

    pWilds = new_wilds();
    pWilds->pArea = pArea;

    for ( ; ; )
    {
        word   = feof( fp ) ? "End" : fread_word( fp );
        fMatch = FALSE;

        switch ( UPPER(word[0]) )
        {
            case '#':
                if ( !str_cmp( word, "#VMAP" ) )
                {
                    pWilds->map_size_x = fread_number(fp);
                    pWilds->map_size_y = fread_number(fp);
                    /* Vizz - answer in kilobytes */
                    arraysize = pWilds->map_size_x * pWilds->map_size_y / 1024;

                    if (arraysize < 1024) /* if less than 1 meg, output in Kb */
                        plogf("wilds.c, load_wilds(): Allocating %d x %d (%ld vrooms) = %ld Kb.",
                              pWilds->map_size_x, pWilds->map_size_y,
                              (pWilds->map_size_x * pWilds->map_size_y),
                              arraysize);
                    else /* Vizz - output in Mb */
                        plogf("wilds.c, load_wilds(): Allocating %d x %d (%ld vrooms) = %ld Mb.",
                              pWilds->map_size_x, pWilds->map_size_y,
                              (pWilds->map_size_x * pWilds->map_size_y),
                              arraysize / 1024);

                    pWilds->staticmap = allocate_wildsmap(pWilds->map_size_x, pWilds->map_size_y);
                    pWilds->map = allocate_wildsmap(pWilds->map_size_x, pWilds->map_size_y);

		// Instead of using the string space when this is going to be stored elsewhere... use freads
                    for (y = 0, j = 0; y < pWilds->map_size_y; y++, j+= pWilds->map_size_x) {
			    fread(pWilds->staticmap+j,1,pWilds->map_size_x,fp);
			    fread_to_eol(fp);
		    }

                /* Vizz - map a copy of the staticmap to apply things like vlinks, flooding etc to */
                    memcpy(pWilds->map, pWilds->staticmap,pWilds->map_size_x*pWilds->map_size_y);

                /* Vizz - allocate mob/obj wilds matrix */
//                    pWilds->char_matrix = allocate_char_matrix(pWilds->map_size_x, pWilds->map_size_y);
//                    pWilds->obj_matrix = allocate_obj_matrix(pWilds->map_size_x, pWilds->map_size_y);

                    if (pArea->wilds)
                    {
                        pLastWilds = pArea->wilds;

                        while(pLastWilds->next)
                            pLastWilds = pLastWilds->next;

                        pLastWilds->next = pWilds;
                    }
                    else
                    {
                        pArea->wilds = pWilds;
                    }
                }
		else
                if ( !str_cmp( word, "#-VMAP" ) )
                {
                    fMatch = TRUE;
                }
		else
                if ( !str_cmp( word, "#-WILDS" ) )
                {
                    if (pWilds->uid == 0)
                    {
                        plogf("wilds.c, load_wilds(): Wilds '%s' has no UID. Assigning next available one.",
                              pWilds->name);
                        pWilds->uid = ++gconfig.next_wilds_uid;
			gconfig_write();
                    }

                    return;
                }
		else
		if ( !str_cmp( word, "#TERRAIN" ) )
		{
                    pTerrain = fread_terrain( fp, pWilds );
                    add_terrain (pWilds, pTerrain);
                }
		else
                if ( !str_cmp( word, "#VLINK" ) )
                {
                    temp_pVLink = fread_vlink(fp);
                    add_vlink(pWilds, temp_pVLink);
                }


                break;

            case 'D':
                if ( !str_cmp( word, "DefaultTerrain" ) )
                {
                    pWilds->cDefaultTerrain = fgetc(fp);
                    plogf("wilds.c, load_wilds(): Default Terrain type is '%c'.", pWilds->cDefaultTerrain);
                }

                break;

            case 'N':
                if ( !str_cmp( word, "Name" ) )
                {
                    free_string(pWilds->name);
                    pWilds->name = fread_string(fp);
                }

                break;


            case 'R':
                if ( !str_cmp( word, "Repop" ) )
                    pWilds->repop = fread_number(fp);

                break;

            case 'U':
                if ( !str_cmp( word, "Uid" ) )
                    pWilds->uid = fread_number(fp);

                break;

        } /* end switch */
    } /* end for */

    return;
}

// Looks for a loaded-up vroom matching the coordinates specified
ROOM_INDEX_DATA *get_wilds_vroom(WILDS_DATA *pWilds, int x, int y)
{
    ROOM_INDEX_DATA *pVroom;

    // Loop thru all loaded vrooms looking for a match
    for (pVroom = pWilds->loaded_vroom;pVroom!=NULL;pVroom = pVroom->next)
    {
        if (pVroom->x == x && pVroom->y == y)
        {
            return (pVroom);
        }
    }

    return (NULL);
}

int get_wilds_vroom_x_by_dir(WILDS_DATA *pWilds, int x, int y, int door)
{
    int rel_vroom_x;

    const int vector_offset[10][2]=
    {
/* Vizz - {  x,  y }           */
          {  0, -1 }, /* North */
          {  1,  0 }, /* East  */
          {  0,  1 }, /* South */
          { -1,  0 }, /* West  */
          {  0,  0 }, /* Up    */
          {  0,  0 }, /* Down  */
          {  1, -1 }, /* NorthEast */
          { -1, -1 }, /* NorthWest */
          {  1,  1 }, /* SouthEast */
          { -1,  1 }  /* SouthWest */
    };

    /* Vizz - work out coordinates of vroom using the vector table to translate the direction */
    rel_vroom_x = x + vector_offset[door][0];

    return (rel_vroom_x);
}

int get_wilds_vroom_y_by_dir(WILDS_DATA *pWilds, int x, int y, int door)
{
    int rel_vroom_y;

    const int vector_offset[10][2]=
    {
/* Vizz - {  x,  y }           */
          {  0, -1 }, /* North */
          {  1,  0 }, /* East  */
          {  0,  1 }, /* South */
          { -1,  0 }, /* West  */
          {  0,  0 }, /* Up    */
          {  0,  0 }, /* Down  */
          {  1, -1 }, /* NorthEast */
          { -1, -1 }, /* NorthWest */
          {  1,  1 }, /* SouthEast */
          { -1,  1 }  /* SouthWest */
    };

    /* Vizz - work out coordinates of vroom using the vector table to translate the direction */
    rel_vroom_y = y + vector_offset[door][1];

    return (rel_vroom_y);
}

ROOM_INDEX_DATA *create_wilds_vroom(WILDS_DATA *pWilds, int x, int y)
{
    ROOM_INDEX_DATA *pRoomIndex;
    WILDS_TERRAIN *pTerrain;
    bool nonroom = FALSE;

    if (!pWilds)
    {
        #ifdef DEBUG
            plogf("wilds.c, create_wilds_vroom(): pWilds = NULL");
        #endif
        return(NULL);
    }
    else
    {

        if ((pTerrain = get_terrain_by_coors (pWilds, x, y)) == NULL)
            nonroom = TRUE;

        if (!nonroom)
        {
            #ifdef DEBUG
                plogf("wilds.c, create_wilds_vroom(): INFO    - Terrain at coordinates is valid. Creating vroom.");
            #endif
            pRoomIndex = create_vroom(pWilds,
                                x + pWilds->startx,
                                y + pWilds->starty,
                                pTerrain);
        }
        else
        {
            #ifdef DEBUG
                plogf("wilds.c, create_wilds_vroom(): FAILURE -  Terrain at coordinates is invalid. Returning NULL");
            #endif
            return (NULL);
        }

    }

    return (pRoomIndex);
}



WILDS_TERRAIN *get_terrain_by_coors (WILDS_DATA *pWilds, int x, int y)
{
    WILDS_TERRAIN *pTerrain;
    bool found = FALSE;
    char j;

    /* Check pointer is valid */
    if (!pWilds)
    {
        plogf("wilds.c, get_terrain_by_coors(): Invalid pWilds pointer.");
        return(NULL);
    }

	j = pWilds->staticmap[(y * pWilds->map_size_x) + x];

    for(pTerrain = pWilds->pTerrain;pTerrain;pTerrain = pTerrain->next)
    {
        if (pTerrain->mapchar == j)
        {
            found = TRUE;
            break;
        }
    }

    if (!found)
    {
        #ifdef DEBUG
            plogf("wilds.c, get_terrain_by_coors(): Terrain type %c not found in list. Returning NULL.", j);
        #endif
        return (NULL);
    }

    #ifdef DEBUG
        plogf("wilds.c, get_terrain_by_coors(): SUCCESS - Terrain found. Returning pTerrain", j);
    #endif
    return (pTerrain);
}



WILDS_TERRAIN *get_terrain_by_token (WILDS_DATA *pWilds, char token)
{
    WILDS_TERRAIN *pTerrain;

    for (pTerrain = pWilds->pTerrain;pTerrain;pTerrain = pTerrain->next)
    {
        if (pTerrain->mapchar == token)
            return (pTerrain);
    }

    return NULL;
}

WILDS_VLINK *fread_vlink(FILE *fp)
{
    WILDS_VLINK *pVLink;
    char      *word;
    bool      fMatch;

    if (!fp)
    {
        plogf ("wilds.c, fread_vlink(): Invalid fp pointer.");
        abort();
    }

    pVLink = new_vlink();

    for ( ; ; )
    {
        word   = feof( fp ) ? "End" : fread_word( fp );
        fMatch = FALSE;

        switch ( UPPER(word[0]) )
        {
            case '#':
                if (!str_cmp(word, "#-VLINK")) {
			if(!pVLink->uid) {
				plogf("wilds.c, fread(): Vlink (%ld, %ld) has no UID. Assigning next available one.",
				      pVLink->wildsorigin_x, pVLink->wildsorigin_y);
				pVLink->uid = ++gconfig.next_vlink_uid;
				gconfig_write();
			}
                    return (pVLink);
	        }

            break;

            case 'D':
                if (!str_cmp(word, "Default_linkage"))
                {
                    pVLink->default_linkage = fread_flag (fp);
                    // This is the wrong place to put this... put it somewhere after reading in .are's
                    pVLink->current_linkage = VLINK_UNLINKED;
                }
                else
                if (!str_cmp(word, "Destvnum"))
                    pVLink->destvnum = fread_number (fp);
                else
                if (!str_cmp(word, "Door"))
                {
                    pVLink->door = fread_number (fp);

                    if (pVLink->door < 0 || pVLink->door > (MAX_DIR - 1))
                    {
                        plogf("wilds.c, fread_vlink(): vlink has bad door number.");
                        abort();
                    }
                }

            break;

            case 'M':
                if (!str_cmp(word, "Map_tile"))
                    pVLink->map_tile = fread_string_eol (fp);
            break;

            case 'O':
                if (!str_cmp(word, "Orig_desc"))
                    pVLink->orig_description = fread_string (fp);
                else
                if (!str_cmp(word, "Orig_keyword"))
                    pVLink->orig_keyword = fread_string (fp);
                else
                if (!str_cmp(word, "Orig_rsflags"))
                    pVLink->orig_rs_flags = fread_flag (fp);
                else
                if (!str_cmp(word, "Orig_key"))
                    pVLink->orig_key = fread_number (fp);

            break;

            case 'R':
                if (!str_cmp(word, "Rev_desc"))
                    pVLink->rev_description = fread_string (fp);
                else
                if (!str_cmp(word, "Rev_keyword"))
                    pVLink->rev_keyword = fread_string (fp);
                else
                if (!str_cmp(word, "Rev_rsflags"))
                    pVLink->rev_rs_flags = fread_flag (fp);
                else
                if (!str_cmp(word, "Rev_key"))
                    pVLink->rev_key = fread_number (fp);

            break;

            case 'U':
                if (!str_cmp(word, "Uid"))
                    pVLink->uid = fread_number (fp);
            break;

            case 'W':
                if (!str_cmp(word, "Wildsorigin_x"))
                    pVLink->wildsorigin_x = fread_number (fp);
                else
                if (!str_cmp(word, "Wildsorigin_y"))
                    pVLink->wildsorigin_y = fread_number (fp);
        } /* end switch */
    } /* end for */
}


void fwrite_vlink (FILE *fp, WILDS_VLINK *pVLink)
{
    if (!fp)
    {
        plogf("wilds.c, fwrite_vlink(): Invalid fp");
        return;
    }

    if (!pVLink)
    {
        plogf("wilds.c, fwrite_vlink(): Invalid pVLink");
        return;
    }

    fprintf(fp, "#VLINK\n");
    fprintf(fp, "Uid %ld\n", pVLink->uid);
    fprintf(fp, "Wildsorigin_x %d\n", pVLink->wildsorigin_x);
    fprintf(fp, "Wildsorigin_y %d\n", pVLink->wildsorigin_y);
    fprintf(fp, "Door %d\n", pVLink->door);
    fprintf(fp, "Destvnum %ld\n", pVLink->destvnum);

    // Vizz - maptile is a raw string, so we deliberately don't use fix_string().
    fprintf(fp, "Map_tile %s\n", pVLink->map_tile);
    fprintf(fp, "Default_linkage %d\n", pVLink->default_linkage);

    // Vizz - pVLink->current_linkage is not stored, because it's a runtime only variable.
    fprintf(fp, "Orig_desc %s~\n", fix_string(pVLink->orig_description));
    fprintf(fp, "Orig_keyword %s~\n", fix_string(pVLink->orig_keyword));
    fprintf(fp, "Orig_rsflags %ld\n", pVLink->orig_rs_flags);
    fprintf(fp, "Orig_key %ld\n", pVLink->orig_key);
    fprintf(fp, "Rev_desc %s~\n",  fix_string(pVLink->rev_description));
    fprintf(fp, "Rev_keyword %s~\n",  fix_string(pVLink->rev_keyword));
    fprintf(fp, "Rev_rsflags %ld\n", pVLink->rev_rs_flags);
    fprintf(fp, "Rev_key %ld\n", pVLink->rev_key);
    fprintf(fp, "#-VLINK\n\n");

    return;
}


WILDS_TERRAIN *fread_terrain (FILE *fp, WILDS_DATA *pWilds)
{
    WILDS_TERRAIN *pTerrain;
    char      *word;
    bool      fMatch;

/*
 * Check pointers are valid.
 */
    if(!fp)
    {
        plogf("wilds.c, fread_terrain(): Invalid fp");
        abort();
    }

    if(!pWilds)
    {
        plogf("wilds.c, fread_terrain(): Invalid pWilds");
        abort();
    }

    pTerrain = new_terrain(pWilds);

    for ( ; ; )
    {
        word   = feof( fp ) ? "End" : fread_word( fp );
        fMatch = FALSE;

        switch ( UPPER(word[0]) )
        {
            case '#':
                if ( !str_cmp( word, "#-TERRAIN" ) )
                {
                    plogf("wilds.c, fread_terrain(): Finished reading terrain record.");
                    return (pTerrain);
                }
                else
                if ( !str_cmp( word, "#ROOM" ) )
                {
                    plogf("wilds.c, fread_terrain(): Found #ROOM record.");
                    // need to use sent's current room reading logic
                    pTerrain->template = read_room_new(fp, NULL, ROOMTYPE_TERRAIN);
                }

                break;

            case 'B':
                if ( !str_cmp( word, "Briefdesc"))
                {
                    pTerrain->briefdesc = fread_string(fp);
                    plogf("wilds.c, fread_terrain(): Briefdesc = '%s'.", pTerrain->briefdesc);
                }

                break;

            case 'N':
                if ( !str_cmp( word, "Nonroom"))
                {
                    pTerrain->nonroom = fread_number(fp);
                    plogf("wilds.c, fread_terrain(): Nonroom = %d.", pTerrain->nonroom);
                }

                break;

            case 'S':
                if ( !str_cmp( word, "Showchar"))
                {
                    pTerrain->showchar = fread_string_eol(fp);
                    plogf("wilds.c, fread_terrain(): Showchar = '%s'.", pTerrain->showchar);
                }
                else
                if ( !str_cmp( word, "Showname"))
                {
                    pTerrain->showname = fread_string(fp);
                    plogf("wilds.c, fread_terrain(): Showname = '%s'.", pTerrain->showname);
                }

                break;

            case 'T':
                if (!str_cmp(word, "Tile"))
                {
                    fgetc(fp);
                    pTerrain->mapchar = fgetc(fp);
                    plogf("wilds.c, fread_terrain(): Tile = '%c'.", pTerrain->mapchar);

                }

                break;

        } /* End switch */

    } /* End for */

}


void fwrite_terrain (FILE *fp, WILDS_TERRAIN *pTerrain)
{
    if (!fp)
    {
        plogf("wilds.c, fwrite_terrain(): Invalid fp");
        return;
    }

    if (!pTerrain)
    {
        plogf("wilds.c, fwrite_terrain(): Invalid pTerrain");
        return;
    }

    fprintf(fp, "#TERRAIN\n");
    fprintf(fp, "Tile %c\n", pTerrain->mapchar);
    fprintf(fp, "Showchar %s\n", pTerrain->showchar);
    fprintf(fp, "Showname %s~\n", fix_string(pTerrain->showname));
    fprintf(fp, "Briefdesc %s~\n", fix_string(pTerrain->briefdesc));
    fprintf(fp, "Nonroom %d\n", pTerrain->nonroom);
    save_room_new(fp, pTerrain->template, ROOMTYPE_TERRAIN);
    fprintf(fp, "#-TERRAIN\n\n");
    return;
}

void link_vroom(ROOM_INDEX_DATA *pWildsRoom)
{
	EXIT_DATA *pexit;
	int door;
	long dest_x;
	long dest_y;

	if(check_for_bad_room(pWildsRoom->wilds,pWildsRoom->x,pWildsRoom->y)) {
		for(door = 0; door < MAX_DIR; door++) if(!pWildsRoom->exit[door]) {
			dest_x = pWildsRoom->x + dir_offsets[door][0];
			dest_y = pWildsRoom->y + dir_offsets[door][1];

			if((dest_x == pWildsRoom->x && dest_y == pWildsRoom->y) || dest_x < 0 || dest_x >= pWildsRoom->wilds->map_size_x ||
				dest_y < 0 || dest_y >= pWildsRoom->wilds->map_size_y ||
				!check_for_bad_room(pWildsRoom->wilds,dest_x,dest_y)) continue;

			pexit                   = new_exit();
			pexit->u1.vnum          = 0;
			pexit->orig_door        = door;
			pexit->from_room	= pWildsRoom;
			pexit->wilds.x          = dest_x;
			pexit->wilds.y          = dest_y;
			pexit->wilds.area_uid   = pWildsRoom->wilds->pArea->uid;
			pexit->wilds.wilds_uid  = pWildsRoom->wilds->uid;
			pWildsRoom->exit[door]  = pexit;
		}
	}
}

bool link_vlink(WILDS_VLINK *pVLink)
{
    WILDS_DATA *pWilds = NULL;
    ROOM_INDEX_DATA *pWildsRoom = NULL;
    ROOM_INDEX_DATA *pRevRoom = NULL;
    EXIT_DATA *pExit = NULL;
    bool found = FALSE;
    long portal_x = 0;
    long portal_y = 0;
    int rev = 0;

    // Check vlink pointer is valid
    if (!pVLink)
    {
        plogf("wilds.c, link_vlink(): pVLink is NULL");
        return (FALSE);
    }

    pWilds = pVLink->pWilds;
    pWildsRoom = get_wilds_vroom (pWilds, pVLink->wildsorigin_x, pVLink->wildsorigin_y);
        portal_x = get_wilds_vroom_x_by_dir(pWilds,
                                            pVLink->wildsorigin_x,
                                            pVLink->wildsorigin_y,
                                            pVLink->door);

        portal_y = get_wilds_vroom_y_by_dir(pWilds,
                                            pVLink->wildsorigin_x,
                                            pVLink->wildsorigin_y,
                                            pVLink->door);

        // Mark the vlink entrance on the wildsmap
        pWilds->map[(portal_y * pWilds->map_size_x)+ portal_x] = '0';


	if (IS_SET(pVLink->default_linkage, VLINK_FROM_WILDS)) {
		// if the wilds room happens to be loaded up
		if (pWildsRoom) {
			if (!(pExit = pWildsRoom->exit[pVLink->door])) {
				pExit = new_exit ();
			} else if(!IS_SET(pExit->exit_info,EX_VLINK)) {
				if(pExit->long_desc) { free_string(pExit->long_desc); pExit->long_desc=NULL; }
				if(pExit->short_desc) { free_string(pExit->short_desc); pExit->short_desc=NULL; }
				if(pExit->keyword) { free_string(pExit->keyword); pExit->keyword=NULL; }
			} else {
				plogf("wilds.c, link_vlink(): Wilds-side vlink exit already exists.");
				return (FALSE);
			}

			found = TRUE;
			pExit->short_desc = str_dup("");
			pExit->long_desc = str_dup(pVLink->orig_description);
			pExit->keyword = str_dup(pVLink->orig_keyword);
			pExit->rs_flags = pVLink->orig_rs_flags | EX_VLINK;
			pExit->exit_info = pExit->rs_flags;
			pExit->door.key_vnum = pVLink->orig_key;
			pExit->u1.vnum = pVLink->destvnum;
			pExit->u1.to_room = get_room_index(pExit->u1.vnum);
			pExit->orig_door = pVLink->door;    /* OLC */
			pExit->wilds.x = 0;
			pExit->wilds.y = 0;
			pExit->wilds.area_uid = 0;
			pExit->wilds.wilds_uid = 0;

			pWildsRoom->exit[pVLink->door] = pExit;
			pExit->from_room = pWildsRoom;
			SET_BIT(pVLink->current_linkage, VLINK_FROM_WILDS);
		}
	}

    if (IS_SET(pVLink->default_linkage, VLINK_TO_WILDS))
    {
        // if the static room happens to be loaded up
        if ((pRevRoom=get_room_index(pVLink->destvnum))!=NULL)
        {
            rev = rev_dir[pVLink->door];

            if (pRevRoom->exit[rev]==NULL)
            {
                found = TRUE;
                pExit = new_exit ();
                pExit->long_desc = str_dup(pVLink->rev_description);
                pExit->keyword = str_dup(pVLink->rev_keyword);
                pExit->rs_flags = pVLink->rev_rs_flags | EX_VLINK;
                pExit->exit_info = pExit->rs_flags;
                pExit->door.key_vnum = pVLink->rev_key;
                pExit->u1.vnum = 0;
                pExit->u1.to_room = NULL;
                pExit->wilds.x = pVLink->wildsorigin_x;
                pExit->wilds.y = pVLink->wildsorigin_y;
                pExit->wilds.area_uid = pVLink->pWilds->pArea->uid;
                pExit->wilds.wilds_uid = pVLink->pWilds->uid;
                pExit->orig_door = rev;    /* OLC */

                pRevRoom->exit[rev] = pExit;
                pExit->from_room = pRevRoom;
                SET_BIT(pVLink->current_linkage, VLINK_TO_WILDS);
            }
            else
		return (FALSE);
        }
        else
        {
            plogf("wilds.c, link_vlink(): Static room not found.");
        }

    }

    if (found)
        return (TRUE);
    else
        return (FALSE);
}

bool unlink_vlink(WILDS_VLINK *pVLink)
{
    WILDS_DATA *pWilds = NULL;
    ROOM_INDEX_DATA *pWildsRoom = NULL;
    ROOM_INDEX_DATA *pRevRoom = NULL;
    EXIT_DATA *pExit = NULL;
    bool found = FALSE;
    long portal_x = 0;
    long portal_y = 0;
    int rev = 0;

    // Check vlink pointer is valid
    if (!pVLink)
    {
        plogf("wilds.c, unlink_vlink(): pVLink is NULL");
        return (FALSE);
    }

    if (pVLink->current_linkage == VLINK_UNLINKED)
    {
        plogf("wilds.c, unlink_vlink(): current_linkage is VLINK_UNLINKED, so can't unlink.");
        return(FALSE);
    }
    pWilds = pVLink->pWilds;

portal_x = get_wilds_vroom_x_by_dir(pWilds,
				    pVLink->wildsorigin_x,
				    pVLink->wildsorigin_y,
				    pVLink->door);

portal_y = get_wilds_vroom_y_by_dir(pWilds,
				    pVLink->wildsorigin_x,
				    pVLink->wildsorigin_y,
				    pVLink->door);

/* Remove the vlink entrance from the wildsmap, restoring the original terrain tile */
pWilds->map[(portal_y * pWilds->map_size_x) + portal_x] =
    pWilds->staticmap[(portal_y * pWilds->map_size_x) + portal_x];

    if (IS_SET(pVLink->current_linkage, VLINK_FROM_WILDS))
    {
        /* First, check if wilds-side exit exists */
        if ((pWildsRoom = get_wilds_vroom(pVLink->pWilds,
                                          pVLink->wildsorigin_x,
                                          pVLink->wildsorigin_y)) != NULL)
        {
            if ((pExit = pWildsRoom->exit[pVLink->door]) != NULL)
            {
		    pWildsRoom->exit[pVLink->door] = NULL;
                free_exit(pExit);
                found = TRUE;
		link_vroom(pWildsRoom);
            }
            else
                plogf("wilds.c, unlink_vlink(): wilds side of vlink - exit missing!");
        }
	/* Mark vlink as unlinked in from_wilds direction */
	REMOVE_BIT(pVLink->current_linkage, VLINK_FROM_WILDS);
    }

    if (IS_SET(pVLink->current_linkage, VLINK_TO_WILDS))
    {
        /* Check if reverse-side exit exists */
        if ((pRevRoom = get_room_index(pVLink->destvnum)) !=NULL)
        {
            rev = rev_dir[pVLink->door];
            if ((pExit = pRevRoom->exit[rev]) != NULL)
            {
		    pRevRoom->exit[rev] = NULL;
                free_exit(pExit);
                found = TRUE;
            }
            else
                plogf("wilds.c, unlink_vlink(): reverse side of vlink - exit missing!");
        }
	REMOVE_BIT(pVLink->current_linkage, VLINK_TO_WILDS);
    }

    if (found)
    {
        pVLink->current_linkage = VLINK_UNLINKED;
        return (TRUE);
    }
    else
        return (FALSE);
}

WILDS_VLINK *find_vlink_to_coord(WILDS_DATA *pWilds, int x, int y)
{
	WILDS_VLINK *pVLink;

	for(pVLink=pWilds->pVLink;pVLink;pVLink = pVLink->next) {
		if((x == get_wilds_vroom_x_by_dir(pWilds, pVLink->wildsorigin_x, pVLink->wildsorigin_y, pVLink->door)) &&
			(y == get_wilds_vroom_y_by_dir(pWilds, pVLink->wildsorigin_x, pVLink->wildsorigin_y, pVLink->door)))
			break;
	}

	return pVLink;
}

void show_map_to_char(CHAR_DATA * ch,
                      CHAR_DATA * to,
                      int bonus_view_x,
		      int bonus_view_y,
                      bool olc)
{
    WILDS_DATA *pWilds;
    WILDS_TERRAIN *pTerrain;
    WILDS_VLINK *pVLink;
    int x, y;
    long index;
    DESCRIPTOR_DATA * d;
    bool found = FALSE;
    bool foundterrain = FALSE;
    char j[5];
    char last_terrain[5];
    char temp[5];
    int squares_to_show_x;
    int squares_to_show_y;
    bool last_char_same;
    char last_char;
    char last_colour_char;
    char edit_mapstring[80];
    char buf[MIL];
    char padding1[MIL];
    char padding2[MIL];
    char tlcoor[MIL];
    char trcoor[MIL];
    char blcoor[MIL];
    char brcoor[MIL];
    int cString;
    int vp_startx, vp_starty, vp_endx, vp_endy;
    int i, pad;

    if (olc)
        pWilds = ch->desc->pEdit;
    else
        pWilds = ch->in_wilds;

    squares_to_show_x = get_squares_to_show_x(bonus_view_x);
    squares_to_show_y = get_squares_to_show_y(bonus_view_y);
    last_char_same = FALSE;
    last_char = ' ';
    last_colour_char = ' ';
    edit_mapstring[0] = '\0';
    send_to_char("\n\r", ch);

    vp_startx = ch->in_room->x - squares_to_show_x;
    vp_endx   = ch->in_room->x + squares_to_show_x;
    vp_starty = ch->in_room->y - squares_to_show_y;
    vp_endy   = ch->in_room->y + squares_to_show_y;

    if (olc)
    {
        if (vp_startx < 0)
        {
            vp_startx = ch->in_room->x;
            vp_endx   = ch->in_room->x + (squares_to_show_x * 2);
        }
        else
            if (vp_startx > pWilds->map_size_x)
            {
                vp_startx = pWilds->map_size_x - (squares_to_show_x * 2);
                vp_endx   = pWilds->map_size_x;
            }

        if (vp_starty < 0)
        {
            vp_starty = ch->in_room->y;
            vp_endy   = ch->in_room->y + (squares_to_show_y * 2);
        }
        else
            if (vp_startx > pWilds->map_size_x)
            {
                vp_startx = pWilds->map_size_x - (squares_to_show_x * 2);
                vp_endx   = pWilds->map_size_x;
            }

        send_to_char("View Window                      Edit Window\n\r", to);

        sprintf(tlcoor, "(%d, %d)", vp_startx, vp_starty);
        sprintf(trcoor, "(%d, %d)", vp_endx, vp_starty);
        pad = squares_to_show_x * 2 + 7;

        for( i=0; i < pad ; i++ )
        {
           padding1[i] = ' ';
        }

        padding1[i] = 0;
        pad = ((squares_to_show_x * 2) - strlen(tlcoor)) - strlen(trcoor);

        for( i=0; i < pad ; i++ )
        {
           padding2[i] = ' ';
        }

        padding2[i] = 0;
        sprintf(buf, "%s%s%s%s\n\r",
                padding1, tlcoor, padding2, trcoor);
        send_to_char(buf, to);
    }

    for (y = vp_starty;y <= vp_endy;y++)
    {
        cString = 0;
        for (x = vp_startx;x <= vp_endx;x++)

        {
            found = FALSE;
            index = y * pWilds->map_size_x + x;

            if (x >= 0
                && x < pWilds->map_size_x
                && y >= 0
                && y < pWilds->map_size_y)
            {

		if((pVLink = find_vlink_to_coord(pWilds,x,y)) && pVLink->map_tile && pVLink->map_tile[0]) {
			strcpy(temp,pVLink->map_tile);
			found = TRUE;
		}

                if (ch->in_room->x == x
                    && ch->in_room->y == y)
                {
                    sprintf(temp, "{M@{x");
                    found = TRUE;
                }

                for (d = descriptor_list; d != NULL;d = d->next)
                {
                    if (d->connected == CON_PLAYING && d->character != ch &&
                    	can_see(ch, d->character) &&
                    	(d->character->in_room->wilds == pWilds ||
                    		(d->character->in_room->viewwilds == pWilds && IS_SET(d->character->in_room->room2_flags,ROOM_VISIBLE_ON_MAP))) &&
                    	d->character->in_room->x == x &&
                    	d->character->in_room->y == y)
                    {
                        sprintf(temp, "{W@{x");
                        found = TRUE;
                    }
                }

/* Vizz - if no PC found in the room, display the terrain char */
                if (!found)
                {
                    sprintf(j, "%c",pWilds->map[index]);
                    if (!str_cmp(j, last_terrain))
                    {
                        sprintf(temp, last_terrain);
                    }
                    else
                    {
/* Vizz - Search the terrain list linearly for now at least. could index this later for speed */
                        foundterrain = FALSE;
                        for(pTerrain = pWilds->pTerrain;pTerrain;pTerrain = pTerrain->next)
                        {
                            if (pWilds->map[index] == pTerrain->mapchar)
                            {
                                sprintf(temp, pTerrain->showchar);
                                sprintf(last_terrain, temp);
                                foundterrain = TRUE;
                            }

                        }

                        if (!foundterrain)
                        {
                            /* Vizz - highlight vlink entrances */
                            if (!strcmp(j, "0"))
                                sprintf(temp, "{YO");
                            else
                                /* Vizz - allow for non-terrain defined characters - display verbatim */
                                sprintf(temp, j);
                        }
                    }
                }

                if (last_char_same
                    && (temp[2] != last_char
                        || temp[1] != last_colour_char))
                {
                    last_char_same = FALSE;
                }

                if (temp[2] == last_char && temp[1] == last_colour_char)
                {
                     last_char_same = TRUE;
                }

                if (last_char_same)
                {
                     sprintf(temp, "%c", temp[2]);
                }

                send_to_char(temp, to);

                if (olc)
                {
                    edit_mapstring[cString] = j[0];
                    cString++;
                }

                if (last_char_same)
                {
                    last_char = temp[0];
                }
                else
                {
                    last_char = temp[2];
                    last_colour_char = temp[1];
                }
            }
            else
            {
                /* If we're displaying outside the map bounds, fill in with starfield */
                if (!olc)
                    if (x % 5 + y % 6 == 0 && x % 2 + y % 3 == 0)
                    {
                        last_char = '.'; last_colour_char = 'x';
                        send_to_char("{x.", to);
                    }
                    else
                        send_to_char(" ", to);
                else
                    send_to_char(" ", to);
                    edit_mapstring[cString] = ' ';
                    cString++;
            }
        }

        if (olc)
        {
            char buf[81];

            edit_mapstring[cString] = '\0';
            sprintf(buf, "       {x%s{%c", edit_mapstring, last_colour_char);
            send_to_char(buf, to);
        }

        send_to_char("\n\r", to);
    }

    if (olc)
    {
        sprintf(blcoor, "(%d, %d)", vp_startx, vp_endy);
        sprintf(brcoor, "(%d, %d)", vp_endx, vp_endy);
        pad = squares_to_show_x * 2 + 7;

        for( i=0; i < pad ; i++ )
        {
           padding1[i] = ' ';
        }

        padding1[i] = 0;
        pad = ((squares_to_show_x * 2) - strlen(blcoor)) - strlen(brcoor);

        for( i=0; i < pad ; i++ )
        {
           padding2[i] = ' ';
        }

        padding2[i] = 0;
        sprintf(buf, "{x%s%s%s%s\n\r",
                padding1, blcoor, padding2, brcoor);
        send_to_char(buf, to);
    }

    send_to_char("{x", to);
    return;
}

void save_wilds (FILE * fp, AREA_DATA * pArea)
{
    WILDS_DATA *	pWilds;
    WILDS_TERRAIN *	pTerrain;
    WILDS_VLINK *       pVLink;
    int			y, j;

    if (pArea->wilds == NULL)
    {
        return;
    }

    for (pWilds = pArea->wilds;pWilds;pWilds = pWilds->next)
    {
        fprintf(fp, "#WILDS\n");
        fprintf(fp, "Uid %ld\n", pWilds->uid);
        fprintf(fp, "Name %s~\n", pWilds->name);
        fprintf(fp, "Repop %d~\n", pWilds->repop);
        fprintf(fp, "#VMAP %d %d\n", pWilds->map_size_x, pWilds->map_size_y);

        for (y = 0, j = 0; y < pWilds->map_size_y; y++, j+=pWilds->map_size_x)
        {
	    fwrite(pWilds->staticmap+j,pWilds->map_size_x,1,fp);

            if (y < (pWilds->map_size_y - 1))
                fprintf(fp, "\n");
            else
                fprintf(fp, "~\n");

        }

        fprintf(fp, "#-VMAP\n\n");

        for(pTerrain = pWilds->pTerrain;pTerrain;pTerrain = pTerrain->next)
            fwrite_terrain(fp, pTerrain);

        fprintf(fp, "\n");

        for(pVLink = pWilds->pVLink;pVLink != NULL;pVLink = pVLink->next)
        {
             fwrite_vlink(fp, pVLink);
        }

        fprintf(fp, "#-WILDS\n\n");
    }

    return;
}

void do_vlinks(CHAR_DATA *ch, char *argument)
{
    WILDS_VLINK *pVLink;
    char buf[MSL];

    if (!ch->in_wilds)
    {
        plogf("wilds.c, do_vlinks(): ch->in_wilds invalid.");
        send_to_char("Vlinks: You don't appear to be in a wilds region.\n\r", ch);
        return;
    }

    send_to_char("{w[{WVLinks{w]{x\n\r\n\r", ch);

    if (ch->in_wilds->pVLink)
    {
        BUFFER *buffer;

        buffer=new_buf();
        add_buf(buffer, "[   Uid] [x coor] [y coor] [direction] [destvnum] [default state] [current state]{x\n\r");
        for(pVLink=ch->in_wilds->pVLink;pVLink!=NULL;pVLink = pVLink->next)
        {
            sprintf(buf, "{x({W%6ld{x)  {W%6d   %6d   %9s    %7ld   %7s   %7s{x\n\r",
                           pVLink->uid,
                           pVLink->wildsorigin_x,
                           pVLink->wildsorigin_y,
                           dir_name[pVLink->door],
                           pVLink->destvnum,
                           (IS_SET(pVLink->default_linkage, VLINK_TO_WILDS) &&
			    IS_SET(pVLink->default_linkage, VLINK_FROM_WILDS)) ? "two-way" :
			        IS_SET(pVLink->default_linkage, VLINK_TO_WILDS) ? "to wilds" :
			            IS_SET(pVLink->default_linkage, VLINK_FROM_WILDS) ? "from wilds" :
                                        "not set",
                           (IS_SET(pVLink->current_linkage, VLINK_TO_WILDS) &&
			    IS_SET(pVLink->current_linkage, VLINK_FROM_WILDS)) ? "two-way" :
			        IS_SET(pVLink->current_linkage, VLINK_TO_WILDS) ? "to wilds" :
			            IS_SET(pVLink->current_linkage, VLINK_FROM_WILDS) ? "from wilds" :
                                        "not set");
            add_buf(buffer, buf);
        }

        page_to_char(buf_string(buffer), ch);
        free_buf(buffer);

    }
    else
        send_to_char("    None defined.\n\r", ch);

    if (!IS_SET(ch->comm, COMM_COMPACT))
    {
        send_to_char("\n\r", ch);
    }

    return;
}

WILDS_DATA *new_wilds (void)
{
    static WILDS_DATA pwilds_zero;
    WILDS_DATA *pWilds;

    if(!wilds_free)
    {
        pWilds = alloc_perm (sizeof (*pWilds));
        top_wilds++;
    }
    else
    {
        pWilds = wilds_free;
        wilds_free = wilds_free->next;
    }

    // Initialise wilds_data structure in memory referenced by pWilds pointer
    *pWilds = pwilds_zero;

    pWilds->next = NULL;
    pWilds->uid = 0; /* Vizz - UID 0 is invalid - helps us know when to set the UID */
    pWilds->name = str_dup("New Wilds");
    pWilds->map = &str_empty[0];
    pWilds->staticmap = &str_empty[0];
    pWilds->map_size_x = 0;
    pWilds->map_size_y = 0;
    pWilds->startx = 0;
    pWilds->starty = 0;
    pWilds->pTerrain = NULL;
    pWilds->cDefaultTerrain = 'S'; // Arbitrary default terrain char
    pWilds->pVLink = NULL;
//    pWilds->char_matrix = NULL;
//    pWilds->obj_matrix = NULL;
    VALIDATE (pWilds);

    return pWilds;
}



void free_wilds (WILDS_DATA * pWilds)
{
    WILDS_VLINK *pVLink, *pVLink_next;
    WILDS_TERRAIN *pTerrain, *pTerrain_next;

    if (!IS_VALID (pWilds))
        return;

    free_string (pWilds->name);

/* Vizz - calloc'ed, to avoid MAX_PERM_BLOCK having to be huge
 *        if we were using alloc_perm and free_string
 */
    free(pWilds->map);
    free(pWilds->staticmap);

    if (pWilds->pVLink)
    {
// Loop thru VLinks list and free all indirect structures
        for(pVLink = pWilds->pVLink;pVLink != NULL;pVLink = pVLink_next)
        {
            pVLink_next = pVLink->next;
            free_vlink (pVLink);
        }
    }

    if (pWilds->pTerrain)
    {
// Loop thru Terrain list and free all indirect structures
        for(pTerrain = pWilds->pTerrain;pTerrain != NULL;pTerrain = pTerrain_next)
        {
            pTerrain_next = pTerrain->next;
            free_terrain (pTerrain);
        }
    }

    pWilds->pVLink = NULL;
    pWilds->pTerrain = NULL;
    INVALIDATE (pWilds);

    pWilds->next = wilds_free->next;
    wilds_free = pWilds;
    return;
}

void char_to_vroom (CHAR_DATA *ch, WILDS_DATA *pWilds, int x, int y)
{
    ROOM_INDEX_DATA *room;
    OBJ_DATA *obj;

    // Check arguments are valid.
    if (pWilds == NULL)
    {
        plogf ("wilds.c, char_to_vroom(): pWilds is NULL.");

	// No wilds pointer, so send the char to the default room.
        if ((room = get_room_index (ROOM_VNUM_DEFAULT)) != NULL)
        {
            char_to_room (ch, room);
            return;
        }

	// Just in case...
        plogf("wilds.c, char_to_vroom(): Default room could not be found!");
        return;
    }

	// Dude... mounts? :P
    if (MOUNTED(ch) && MOUNTED(ch)->in_room == ch->in_room
    && MOUNTED(ch)->in_room == NULL)
	char_to_vroom(MOUNTED(ch), pWilds, x, y);


    ch->in_wilds = pWilds;
    ch->at_wilds_x = x;
    ch->at_wilds_y = y;

	// Check if there's a loaded vroom for them, or load one up.
	if(!(room = get_wilds_vroom(pWilds, x, y)))
	    room = create_wilds_vroom(pWilds, x, y);

	ch->in_room = room;
	ch->next_in_room = room->people;
	room->people = ch;

    // Is the character a player?
    if (!IS_NPC (ch))
    {
        if (ch->in_room->area->empty)
        {
            ch->in_room->area->empty = FALSE;
            ch->in_room->area->age = 0;
        }

        ++ch->in_room->area->nplayer;

        if (ch->in_wilds)
        {
            if (ch->in_wilds->empty)
            {
                ch->in_wilds->empty = FALSE;
                ch->in_wilds->age = 0;
            }

            ++ch->in_wilds->nplayer;
        }

    }
    else
        ++ch->in_wilds->loaded_mobs;

    if ((obj = get_eq_char (ch, WEAR_LIGHT)) != NULL
        && obj->item_type == ITEM_LIGHT && obj->value[2] != 0)
        ++ch->in_room->light;

    if (IS_AFFECTED (ch, AFF_PLAGUE))
    {
        AFFECT_DATA *af, plague;
        CHAR_DATA *vch;

        for (af = ch->affected; af != NULL; af = af->next)
        {
            if (af->type == gsn_plague)
                break;
        }

        if (af == NULL)
        {
            REMOVE_BIT (ch->affected_by, AFF_PLAGUE);
            return;
        }

        if (af->level == 1)
            return;

        plague.where = TO_AFFECTS;
        plague.group = af->group;
        plague.type = gsn_plague;
        plague.level = af->level - 1;
        plague.duration = number_range (1, 2 * plague.level);
        plague.location = APPLY_STR;
        plague.modifier = -5;
        plague.bitvector = AFF_PLAGUE;

        for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
        {
            if (!saves_spell (plague.level - 2, vch, DAM_DISEASE)
                && !IS_IMMORTAL (vch) &&
                !IS_AFFECTED (vch, AFF_PLAGUE) && number_bits (6) == 0)
            {
                send_to_char ("You feel hot and feverish.\n\r", vch);
                act ("$n shivers and looks very ill.", vch, NULL, NULL,
                     TO_ROOM);
                affect_join (vch, &plague);
            }
        }
    }

    return;
}

void add_vlink (WILDS_DATA *pWilds, WILDS_VLINK *pVLink)
{
    /*
     * Check pointers are valid
     */
    if (!pWilds)
    {
        plogf("wilds.c, add_vlink(): Invalid pWilds pointer.");
        abort();
    }

    if (!pVLink)
    {
        plogf("wilds.c, add_vlink(): Invalid pVLink pointer.");
        abort();
    }

	pVLink->pWilds = pWilds;
	pVLink->next = pWilds->pVLink;
	pWilds->pVLink = pVLink;

    return;
}

WILDS_VLINK *new_vlink ()
{
    static WILDS_VLINK pvlink_zero;
    WILDS_VLINK *pVLink;

    if(!wilds_vlink_free)
    {
        pVLink = alloc_perm (sizeof (*pVLink));
        top_wilds_vlink++;
    }
    else
    {
        pVLink = wilds_vlink_free;
        wilds_vlink_free = wilds_vlink_free->next;
    }

    *pVLink = pvlink_zero;

    pVLink->next = NULL;
    pVLink->pWilds = NULL;
    VALIDATE (pVLink);

    return pVLink;
}



void free_vlink (WILDS_VLINK *pVLink)
{
    if (!IS_VALID(pVLink))
        return;

    free_string (pVLink->orig_description);
    free_string (pVLink->orig_keyword);
    free_string (pVLink->rev_description);
    free_string (pVLink->rev_keyword);
    INVALIDATE (pVLink);

    pVLink->next = wilds_vlink_free;
    wilds_vlink_free = pVLink;
    return;
}

bool add_terrain (WILDS_DATA *pWilds, WILDS_TERRAIN *pTerrain)
{
    if (!IS_VALID(pTerrain))
        return FALSE;

    if (pWilds->pTerrain != NULL)
    {
        /* Point back from the previous to the new list head struct */
        pWilds->pTerrain->prev = pTerrain;

        /* Point forward from the new to the previous list head struct */
        pTerrain->next = pWilds->pTerrain;

        /* Put new struct in list */
        pWilds->pTerrain = pTerrain;

        return TRUE;
    }
    else
    {
        /* Put new struct in list */
        pWilds->pTerrain = pTerrain;
        return TRUE;
    }
}

bool del_terrain (WILDS_DATA *pWilds, WILDS_TERRAIN *pTerrain)
{
    WILDS_TERRAIN *prev_pTerrain, *next_pTerrain;

    if (!IS_VALID(pTerrain))
        return FALSE;

    prev_pTerrain = pTerrain->prev;
    next_pTerrain = pTerrain->next;

    if (prev_pTerrain)
    {
        plogf("prev_pTerrain->mapchar: '%c'", prev_pTerrain->mapchar);
        prev_pTerrain->next = next_pTerrain;
    }

    if (next_pTerrain)
    {
        plogf("next_pTerrain->mapchar: '%c'", next_pTerrain->mapchar);
        next_pTerrain->prev = prev_pTerrain;
    }

    plogf("pTerrain->mapchar: '%c'", pTerrain->mapchar);

    if (pWilds->pTerrain == pTerrain)
        pWilds->pTerrain = next_pTerrain;

    free_terrain(pTerrain);
    return TRUE;
}

WILDS_TERRAIN *new_terrain (WILDS_DATA *pWilds)
{
    static WILDS_TERRAIN pterrain_zero;
    WILDS_TERRAIN *pTerrain;

    if(!wilds_terrain_free)
    {
        pTerrain = alloc_perm (sizeof (*pTerrain));
        top_wilds_terrain++;
    }
    else
    {
        pTerrain = wilds_terrain_free;
        wilds_terrain_free = wilds_terrain_free->next;
    }

    *pTerrain = pterrain_zero;

    pTerrain->prev = NULL;
    pTerrain->next = NULL;
    pTerrain->showname = NULL;
    pTerrain->briefdesc = NULL;
    pTerrain->template = new_room_index();
    pTerrain->pWilds = pWilds;
    VALIDATE (pTerrain);

    return pTerrain;
}

void free_terrain (WILDS_TERRAIN *pTerrain)
{
    if (!IS_VALID(pTerrain))
        return;

    free_string (pTerrain->showchar);
    free_room_index (pTerrain->template);
    pTerrain->pWilds = NULL;
    INVALIDATE (pTerrain);

    pTerrain->prev = NULL;
    pTerrain->next = wilds_terrain_free;
    wilds_terrain_free = pTerrain;
    return;
}



void link_vlinks (WILDS_DATA *pWilds)
{
    ROOM_INDEX_DATA *pRevLinkRoomIndex;
    WILDS_VLINK *pVLink = NULL;

    if (pWilds == NULL)
    {
        plogf("wilds.c, link_vlinks(): Failed to link vlinks - pWilds pointer is NULL");
        return;
    }

    if (pWilds->pVLink == NULL)
    {
        plogf("wilds.c, link_vlinks(): No Vlinks found.");
        return;
    }

    for (pVLink = pWilds->pVLink;pVLink;pVLink = pVLink->next)
    {
        if ((pRevLinkRoomIndex = get_room_index (pVLink->destvnum)) == NULL)
        {
            plogf("wilds.c, apply_vlink(): destvnum %ld does not exist.", pVLink->destvnum);
            continue;
        }

        if (IS_SET(pVLink->default_linkage, VLINK_FROM_WILDS))
        {
            if (pVLink->wildsorigin_x >= 0 && pVLink->wildsorigin_y >= 0
                && pVLink->wildsorigin_x < pWilds->map_size_x
                && pVLink->wildsorigin_y < pWilds->map_size_y)
            {
                if (link_vlink(pVLink))
                    plogf("link_vlinks(): VLink %s from (%d, %d) to %ld Linked Successfully.",
                          dir_name[pVLink->door],
                          pVLink->wildsorigin_x,
			  pVLink->wildsorigin_y,
			  pVLink->destvnum);
                else
                    plogf("wilds.c, link_vlinks(): VLink failed.");
                    continue;
            }
            else
                plogf("wilds.c, link_vlinks(): VLink failed - coordinates are invalid.");
                continue;

        }
    } /* end for */

    return;
}

WILDS_DATA *get_wilds_from_uid (AREA_DATA *pArea, long uid)
{

    WILDS_DATA *pWilds;

//    printf("get_wilds_from_uid (%08X[%s],%ld)\n\r", pArea, (pArea?pArea->file_name:"(null)"), uid);

    // Vizz - If a NULL pArea pointer is passed, search all areas (slower)
    if (!pArea)
    {
        for (pArea = area_first; pArea; pArea = pArea->next)
        {
//	    printf("get_wilds_from_uid -> checking %08X[%s]\n\r", pArea, (pArea?pArea->file_name:"(null)"));
            for (pWilds = pArea->wilds; pWilds; pWilds = pWilds->next)
            {
                if (pWilds->uid == uid)
                    return (pWilds);
            }
        }
    }
    else
    {
        // Vizz - Search the supplied area (faster for when area is known)
        for (pWilds = pArea->wilds; pWilds; pWilds = pWilds->next)
        {
            if (pWilds->uid == uid)
                return (pWilds);
        }
    }

    plogf("wilds.c, get_wilds_from_uid(): Could not find wilds pointer from uid.");
    return (NULL);
}

WILDS_VLINK *get_vlink_from_uid (WILDS_DATA *pWilds, long uid)
{

    AREA_DATA *pArea;
    WILDS_VLINK *pVLink;

    // Vizz - If a NULL pWilds pointer is passed, search all wilds (slower)
    if (!pWilds)
    {
        for (pArea = area_first; pArea; pArea = pArea->next)
        {
            for (pWilds = pArea->wilds; pWilds; pWilds = pWilds->next)
            {
                for (pVLink = pWilds->pVLink; pVLink; pVLink = pVLink->next)
                {
                    if (pVLink->uid == uid)
                        return (pVLink);
                }
            }
        }
    }
    else
    {
        // Vizz - Search the supplied Wilds (faster for when wilds is known)
        for (pVLink = pWilds->pVLink; pVLink; pVLink = pVLink->next)
        {
            if (pVLink->uid == uid)
                return (pVLink);
        }
    }

    plogf("wilds.c, get_vlink_from_uid(): Could not find vlink pointer from uid.");
    return (NULL);
}

WILDS_VLINK *get_vlink_from_index (WILDS_DATA *pWilds, long index)
{
	WILDS_VLINK *pVLink;
	long idx;

	if(!pWilds) return NULL;

	for (idx = 0, pVLink = pWilds->pVLink; pVLink && idx < index; idx++, pVLink = pVLink->next);

	return pVLink;
}

CHAR_DATA *get_people_from_wilds(WILDS_DATA *pWilds, int x, int y)
{
	ROOM_INDEX_DATA *room;
	register CHAR_DATA *people;

	if ((room = get_wilds_vroom(pWilds, x, y)))
		return room->people;
	else {
		for(people = pWilds->char_list; people; people = people->next_in_wilds) {
			if(people->at_wilds_x == x && people->at_wilds_y == y) return people;

			if(people->at_wilds_y > y || (people->at_wilds_y == y && people->at_wilds_x > x)) break;
		}
	}

	return NULL;
}

OBJ_DATA *get_contents_from_wilds(WILDS_DATA *pWilds, int x, int y)
{
	ROOM_INDEX_DATA *room;
	register OBJ_DATA *obj;

	if ((room = get_wilds_vroom(pWilds, x, y)))
		return room->contents;
	else {
		for(obj = pWilds->obj_list; obj; obj = obj->next_in_wilds) {
			if(obj->x == x && obj->y == y) return obj;

			if(obj->y > y || (obj->y == y && obj->x > x)) break;

		}
	}

	return NULL;
}

bool link_contents_wilds(WILDS_DATA *pWilds, int x, int y, OBJ_DATA *contents)
{
	register OBJ_DATA *prev, *obj;

	if(!contents) return FALSE;

	for(prev = NULL, obj = pWilds->obj_list; obj; prev = obj, obj = obj->next_in_wilds) {
		if(obj->x == x && obj->y == y) return FALSE;

		if(obj->y > y || (obj->y == y && obj->x > x)) break;
	}

	if(prev) {
		prev->next_in_wilds = contents;
		contents->prev_in_wilds = prev;
	} else if(pWilds->obj_list) {
		pWilds->obj_list = contents;
		contents->prev_in_wilds = NULL;
	}
	if(obj) {
		obj->prev_in_wilds = contents;
		contents->next_in_wilds = obj;
	}

	return TRUE;
}

OBJ_DATA *unlink_contents_wilds(WILDS_DATA *pWilds, int x, int y)
{
	register OBJ_DATA *obj;

	for(obj = pWilds->obj_list; obj; obj = obj->next_in_wilds) {
		if(obj->x == x && obj->y == y) {
			// Change A->B->C to A->C
			// or
			// Change HEAD(B)->C to HEAD(C)
			if(obj->prev_in_wilds)
				obj->prev_in_wilds->next_in_wilds = obj->next_in_wilds;
			else
				pWilds->obj_list = obj->next_in_wilds;

			// Change A<-B<-C to A<-C
			// or
			// nothing (B is the TAIL)
			if(obj->next_in_wilds)
				obj->next_in_wilds->prev_in_wilds = obj->prev_in_wilds;

			obj->prev_in_wilds = obj->next_in_wilds = NULL;
			return obj;
		}

		if(obj->y > y || (obj->y == y && obj->x > x)) return NULL;
	}

	return NULL;
}

// This will search the rooms
ROOM_INDEX_DATA *wilds_seek_down(register WILDS_DATA *wilds, register int x, register int y, register int z, bool ground)
{
	register ROOM_INDEX_DATA *room, *highest = NULL, *vroom;
	register int i;

	for(i = 0; i < MAX_KEY_HASH; i++)
		for(room = room_index_hash[i]; room; room = room->next) {
			if((room->wilds == wilds || room->viewwilds == wilds) && room->x == x && room->y == y && room->z <= z) {
				if(!highest || (room->z > highest->z)) highest = room;
			}
		}

	// Limit the search to above ground...
	if(ground) {
		vroom = get_wilds_vroom(wilds, x, y);
		if(!vroom) vroom = create_wilds_vroom(wilds, x, y);

		if(highest->z < vroom->z)
			highest = vroom;
	}

	return highest;
}

