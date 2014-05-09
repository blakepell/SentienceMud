#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "merc.h"
#include "recycle.h"
#include "tables.h"

SHIP_DATA           *sailing_boat_free;
SHIP_DATA           *cargo_ship_free;
SHIP_DATA           *galleon_free;
SHIP_DATA 	    *war_galleon_free;
SHIP_DATA           *frigate_free;
long		    top_sailing_boat;

SHIP_DATA *create_new_sailing_boat( CHAR_DATA *ch, long vnum)
{
    OBJ_DATA *pObjShip;
    OBJ_DATA *pMastObj;
    SHIP_DATA *pShip;
    ROOM_INDEX_DATA *pRoom;

    if (!sailing_boat_free)
    {
	pShip = alloc_perm( sizeof( *pShip) );
    }
    else
    {
	pShip = sailing_boat_free;
	sailing_boat_free = sailing_boat_free->next;
    }

    pObjShip = create_object( get_obj_index( OBJ_VNUM_SAILING_BOAT ), 1, TRUE);

    pShip->ship = pObjShip;
    pShip->owner_name = str_dup(ch->name);

    pShip->ship_rooms[0] = create_and_map_room( get_room_index(ROOM_VNUM_SAILING_BOAT_HELM), ROOM_VNUM_SAILING_BOAT_HELM+2+top_sailing_boat, pShip);

    pShip->ship->value[5] = ROOM_VNUM_SAILING_BOAT_HELM + 2 + top_sailing_boat;
    top_sailing_boat++;
    pShip->ship_rooms[1] = create_and_map_room( get_room_index(ROOM_VNUM_SAILING_BOAT_NEST), ROOM_VNUM_SAILING_BOAT_HELM+2+top_sailing_boat, pShip);
    top_sailing_boat++;

    pMastObj = create_object( get_obj_index( OBJ_VNUM_SAILING_BOAT_MAST ), 0, FALSE);
    obj_to_room(pMastObj, pShip->ship_rooms[0]);

    create_exit( pShip->ship_rooms[0], pShip->ship_rooms[1], DIR_UP);
    create_exit( pShip->ship_rooms[1], pShip->ship_rooms[0], DIR_DOWN);

    pShip->next = get_area_data( AREA_VNUM_SAILING_BOAT )->ship_list;
    get_area_data( AREA_VNUM_SAILING_BOAT )->ship_list = pShip;

    return pShip;
}

void create_exit( ROOM_INDEX_DATA *from, ROOM_INDEX_DATA *to, int door)
{
    EXIT_DATA *pExit;

    pExit = new_exit();
    pExit->u1.to_room = to;
    pExit->orig_door = door;
    from->exit[door] = pExit;
    pExit->from_room = from;
}

bool load_sailing_boats()
{
	FILE *fp;
	AREA_DATA *pArea;
	SHIP_DATA *pShip;
	OBJ_DATA *obj;
        ROOM_INDEX_DATA *pRoom;
        int iHash;
        int value;
	int count;
	int counter;

 	fp = fopen( SAILING_FILE, "r");

	if (fp == NULL)
	{
	    bug("Couldn't load sailing boat information.", 0);
	    exit(1);
	}

	pArea = area_first;
	while( pArea->vnum != AREA_VNUM_SAILING_BOAT )
	    pArea = pArea->next;

	count = fread_number( fp );

        value = 157002;

	for (counter = 0; counter < count; counter++)
	{
 	    pShip = alloc_perm( sizeof(*pShip));
	    pShip->owner_name = fread_string( fp );

            pRoom                       = new_room_index();
            pRoom->area                 = pArea;
            pRoom->vnum                 = value;
            if ( value > top_vnum_room )
              top_vnum_room = value;

            iHash                       = value % MAX_KEY_HASH;
            pRoom->next                 = room_index_hash[iHash];
            room_index_hash[iHash]      = pRoom;

	}
}

ROOM_INDEX_DATA * create_and_map_room( ROOM_INDEX_DATA *room, long vnum, SHIP_DATA *ship)
{
    ROOM_INDEX_DATA *pRoom;
    int iHash;
    int value;

    pRoom                       = new_room_index();
    pRoom->area                 = room->area;
    pRoom->vnum                 = vnum;
    pRoom->room_flags = room->room_flags;
    pRoom->sector_type = room->sector_type;
    pRoom->name = room->name;
    pRoom->description = room->description;
    pRoom->ship = ship;
    if ( vnum > top_vnum_room )
        top_vnum_room = vnum;

    iHash                       = vnum % MAX_KEY_HASH;
    pRoom->next                 = room_index_hash[iHash];
    room_index_hash[iHash]      = pRoom;

    return pRoom;
}

