#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "merc.h"
#include "interp.h"
#include "recycle.h"
#include "tables.h"
#include "olc.h"
#include "wilds.h"

SHIP_DATA           *sailing_boat_free;
NPC_SHIP_DATA       *npc_sailing_boat_free;
NPC_SHIP_DATA	    *npc_ship_list;
long		    top_sailing_boat;

#if 0
/* Declare local prototypes*/
void set_correct_cannon_count_from_cargo( SHIP_DATA *ship );

extern ROOM_INDEX_DATA *new_room_index( void );

NPC_SHIP_DATA *create_npc_sailing_boat( long vnum )
{
    NPC_SHIP_DATA *npc_ship;
    NPC_SHIP_INDEX_DATA *npc_ship_index;
    MOB_INDEX_DATA *pMob;
    WAYPOINT_DATA *temp_waypoint;
    SHIP_CREW_DATA *crew;
	OBJ_DATA *cargo;
	OBJ_DATA *next_cargo;
    CHAR_DATA *pMobChar = NULL;
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *map;

    if ( (npc_ship_index = get_npc_ship_index(vnum)) == NULL)
    {
 	bug("Could not create npc ship vnum : ", vnum);
	return NULL;
    }

    /* Allocate memory for npc sailing boat*/
    if (!npc_sailing_boat_free)
    {
	npc_ship = alloc_perm( sizeof( *npc_ship) );
    }
    else
    {
	npc_ship = npc_sailing_boat_free;
	npc_sailing_boat_free = npc_sailing_boat_free->next;
    }

    /* Add to list*/
    if (!npc_ship_list)
    {
	npc_ship_list = npc_ship;
    }
    else
    {
	npc_ship->next = npc_ship_list;
	npc_ship_list = npc_ship;
    }

    /* Create actual ship (allocate vnums etc)*/
    npc_ship->ship = create_new_sailing_boat( npc_ship_index->name==NULL ? str_dup("None") : str_dup(npc_ship_index->name), npc_ship_index->ship_type);
    npc_ship->ship->pk = TRUE;

    /* Attach npc ship template*/
    npc_ship->pShipData = npc_ship_index;
    npc_ship->ship->npc_ship = npc_ship;
    npc_ship->ship->destination = NULL;
    npc_ship->ship->owner = NULL;
    npc_ship->ship->ship_name = str_dup(npc_ship->pShipData->name);
    npc_ship->ship->flag = str_dup(npc_ship->pShipData->flag);
    npc_ship->state = NPC_SHIP_STATE_STOPPED;

	/* Only add cannons if the cargo is empty*/
	if (npc_ship->pShipData->cargo == NULL)
	{
      /* Treasure ships always have all cannons*/
	if (npc_ship->pShipData->npc_type == NPC_SHIP_TRADER &&
      npc_ship->pShipData->npc_sub_type == NPC_SHIP_SUB_TYPE_TREASURE_BOAT) {
				/*create_and_add_cannons( npc_ship->ship, npc_ship->ship->ship->value[7]);*/
	}
  else {
				/*create_and_add_cannons( npc_ship->ship, number_range(npc_ship->ship->ship->value[7]-5, npc_ship->ship->ship->value[7]));*/
	}
  }

    for ( temp_waypoint = npc_ship->pShipData->waypoint_list; temp_waypoint != NULL; temp_waypoint = temp_waypoint->next)
    {
	WAYPOINT_DATA *new;
	log_string( "creating waypoint" );
	new = new_waypoint();
	new->x = temp_waypoint->x;
	new->y = temp_waypoint->y;
	new->hour = temp_waypoint->hour;
	new->day = temp_waypoint->day;
	new->month = temp_waypoint->month;
	new->next = NULL;

	/* Place new item at end of linked list*/
	if (npc_ship->ship->waypoint_list == NULL)
        {
	    npc_ship->ship->waypoint_list = new;
	}
	else
	{
	    WAYPOINT_DATA *cur;
	    cur = npc_ship->ship->waypoint_list;
	    while(cur->next != NULL)
            {
	        cur = cur->next;
            }
	    cur->next = new;
        }
    }

	if ( npc_ship->ship->waypoint_list != NULL ) {
		log_string("Loaded boat was reset and list wasn't null");
	}

    /* Create crew and place in ship*/
    for (crew = npc_ship->pShipData->crew; crew != NULL; crew = crew->next)
    {
	if ((pMob = get_mob_index( crew->vnum )) == NULL)
	{
	    bug("Couldn't find vnum for creating the crew in ship_crew_data.", 0);
	    break;
        }
   	pMobChar = create_mobile(pMob);
	char_to_crew(pMobChar, npc_ship->ship);
    }


    /* Create captain and put him at the helm*/
    if ((pMob = npc_ship->pShipData->captain) != NULL)
    {
    	pMobChar = create_mobile(pMob);
    	char_to_crew(pMobChar, npc_ship->ship);

	/* Give pirate a fancy name */
	if ( npc_ship->pShipData->npc_type == NPC_SHIP_PIRATE )
	{
		free_string(pMobChar->name);
		free_string(pMobChar->short_descr);
		free_string(pMobChar->long_descr);
		/*free_string(npc_ship->pShipData->current_name);*/

		/* Use current name if one exists*/
		if ( npc_ship->pShipData->current_name != NULL && str_cmp(npc_ship->pShipData->current_name, "(null)") && strlen(npc_ship->pShipData->current_name) > 0)
		{
			pMobChar->name = str_dup( npc_ship->pShipData->current_name );
		}
		else
		{
			if ( npc_ship->pShipData->current_name != NULL )
			{
		    	free_string(npc_ship->pShipData->current_name);
			}
			pMobChar->name = pirate_name_generator( );
			npc_ship->pShipData->current_name = str_dup( pMobChar->name );
		}

		pMobChar->short_descr = str_dup( pMobChar->name );

    pMobChar->ships_destroyed = npc_ship->pShipData->ships_destroyed;

		sprintf(buf, "The %s pirate %s stands here proudly.\n\r", rating_table[ get_rating( npc_ship->pShipData->ships_destroyed ) ].name, pMobChar->name );
		pMobChar->long_descr = str_dup( buf );
	}
        npc_ship->ship->owner_name = str_dup(pMobChar->short_descr);
    	npc_ship->captain = pMobChar;
/*    	char_to_room( npc_ship->captain, npc_ship->ship->ship_rooms[0] );*/

      if (number_percent() < 50) {
        map = create_treasure_map();
        if (map != NULL)
	    obj_to_char(map, pMobChar);
      }
    }

	/* Transfer cargo to new ship*/
	next_cargo = npc_ship->pShipData->cargo;
	while((cargo = next_cargo) != NULL)
	{
		next_cargo = cargo->next_content;

		obj_to_obj(cargo, npc_ship->ship->ship);
	}

  /* If trader then load up with cargo*/
	if (npc_ship->pShipData->npc_type == NPC_SHIP_TRADER) {
    OBJ_DATA *obj;
    int i = 0;
    int number_of_items = 0;

    switch(npc_ship->pShipData->npc_sub_type) {

  		/* If medium trader then load with some more cargo*/
      case NPC_SHIP_SUB_TYPE_MEDIUM_TRADER:
        number_of_items = 15;

      /* If light trader then load with some random cargo*/
      case NPC_SHIP_SUB_TYPE_LIGHT_TRADER:
        if (number_of_items == 0) {
        	number_of_items = 7;
        }

      for (i = 0; i < number_of_items; i++) {
        switch(number_range(0,4)) {
          case 0:
            /* iron ore*/
		        obj = create_object( get_obj_index( 100606 ), 0, FALSE);
            break;
          case 1:
            /* wood*/
		        obj = create_object( get_obj_index( 100607 ), 0, FALSE);
            break;
          case 2:
            /* paper*/
		        obj = create_object( get_obj_index( 100610 ), 0, FALSE);
            break;
          case 3:
            /* pigs*/
		        obj = create_object( get_obj_index( 100609 ), 0, FALSE);
            break;
          case 4:
            /* farm eq*/
		        obj = create_object( get_obj_index( 100604 ), 0, FALSE);
            break;
         }
      }
      break;

  		/* If treasure boat then load with treasure*/
      case NPC_SHIP_SUB_TYPE_TREASURE_BOAT:
      number_of_items = number_range(10, 20);
      for (i = 0; i < number_of_items; i++) {
        switch(number_range(0,4)) {
          case 0:
            /* gold*/
		        obj = create_object( get_obj_index( 100611 ), 0, FALSE);
            break;
          case 1:
            /* gems*/
		        obj = create_object( get_obj_index( 100605 ), 0, FALSE);
            break;
        }
      }
      break;
    }
  }

  return (NPC_SHIP_DATA *) npc_ship;
}

SHIP_DATA *create_new_sailing_boat( char *owner_name, int ship_type)
{
    OBJ_DATA *pObjShip;
    OBJ_DATA *pMastObj;
	/*OBJ_DATA *pCannons;*/
    bool room_create;
    SHIP_DATA *pShip;
    long offset;
    char *sname;
    char buf[ MAX_STRING_LENGTH ];

    offset = 100;   /* Beginning boat vnum number*/

	sprintf( buf, "top_sailing_boat = %ld", top_sailing_boat );
	log_string(buf);

    room_create = TRUE;

	/* If sailing boat available, if so dont try and find one*/
	pShip = NULL;
    if ( !sailing_boat_free )
    {
		pShip = alloc_perm( sizeof( *pShip) );
		log_string("boat bucket is empty, creating a new one");
    }
    else
    {
		/* Is a free sailing boat available which is of the type to be created*/
		SHIP_DATA *temp_ship;
		SHIP_DATA *prev_ship;

		prev_ship = sailing_boat_free;
		if (prev_ship->ship_type == ship_type)
		{
			pShip = prev_ship;
			sailing_boat_free = prev_ship->next;
			prev_ship->next = NULL;
			room_create = FALSE;
			log_string("First ship in bucket was what we needed, reusing ship");
		}
		else
        {
			for (temp_ship = prev_ship->next; temp_ship != NULL; prev_ship = temp_ship, temp_ship = temp_ship->next )
			{
				/* Found one*/
				if ( temp_ship->ship_type == ship_type )
				{
					pShip = temp_ship;
					prev_ship = temp_ship->next;
					temp_ship->next = NULL;
					room_create = FALSE;
					log_string("reusing old ship");
				}
			}
		}
	}

	/* If pShip wasn't set then an available boat wasn't available so make a new one*/
	if ( pShip == NULL)
	{
		pShip = alloc_perm( sizeof( *pShip) );
		log_string("Boat bucket had boats but none suitable. Creating a new one");
    }
	pShip->next = NULL;

	if (room_create)
	{
    	top_sailing_boat += 10;
	}

    switch( ship_type )
    {
    default:
  	bug("Ship type was wrong...... when creating boat.", ship_type);
    sprintf( buf, "%d", ship_type );
	bug( buf, 0 );
	return NULL;
    case SHIP_SAILING_BOAT:
        pObjShip = create_object( get_obj_index( OBJ_VNUM_SAILING_BOAT ), 1, TRUE);
	break;
    case SHIP_CARGO_SHIP:
        pObjShip = create_object( get_obj_index( OBJ_VNUM_CARGO_SHIP ), 1, TRUE);
	break;
    case SHIP_AIR_SHIP:
        pObjShip = create_object( get_obj_index( OBJ_VNUM_AIR_SHIP ), 1, TRUE);
	break;
    case SHIP_FRIGATE_SHIP:
        pObjShip = create_object( get_obj_index( OBJ_VNUM_FRIGATE_SHIP ), 1, TRUE);
	break;
    case SHIP_GALLEON_SHIP:
        pObjShip = create_object( get_obj_index( OBJ_VNUM_GALLEON_SHIP ), 1, TRUE);
	break;
    }

    pShip->ship = pObjShip;
    pObjShip->ship = pShip;
    pShip->speed = SHIP_SPEED_STOPPED;
    pShip->owner_name = str_dup(owner_name);
    pShip->flag = str_dup("");
    /*pShip->cannons = 10;*/
    pShip->hit = pShip->ship->value[6];
    pShip->ship_type = ship_type;
    pShip->current_waypoint = NULL;
    pShip->waypoint_list = NULL;
    pShip->scuttle_time = 0;

    pShip->min_crew = pObjShip->value[2];
    pShip->max_crew = pObjShip->value[4];

	/* Recreate description*/
    sname = pObjShip->description;
    sprintf( buf, sname, owner_name );
    pObjShip->description = str_dup( buf );
    free_string( sname );

	/* Recreate name*/
    sname = pObjShip->name;
    sprintf( buf, sname, owner_name );
    pObjShip->name = str_dup( buf );
    free_string( sname );

    pShip->last_room[0] = NULL;
    pShip->last_room[1] = NULL;
    pShip->last_room[2] = NULL;

    switch( ship_type)
    {
    case SHIP_AIR_SHIP:
    if (room_create)
    {
        pShip->ship_rooms[0] = (ROOM_INDEX_DATA *)create_and_map_room( get_room_index(ROOM_VNUM_AIR_SHIP_DECK), (long)157001+(long)offset+(long)top_sailing_boat, pShip);

        pShip->ship->value[5] = (long)(157000 + (long)offset + (long)top_sailing_boat);
        pShip->first_room = pShip->ship->value[5];
        /*top_sailing_boat++;*/
        pShip->ship_rooms[1] = (ROOM_INDEX_DATA *)create_and_map_room( get_room_index(ROOM_VNUM_AIR_SHIP_HELM), (long)157000+(long)offset+(long)top_sailing_boat, pShip);

        /*top_sailing_boat++;*/

        create_exit( pShip->ship_rooms[0], pShip->ship_rooms[1], DIR_WEST, 0);
        create_exit( pShip->ship_rooms[1], pShip->ship_rooms[0], DIR_EAST, 0);

        pShip->max_crew = 0;
        pShip->min_crew = 0;
		sprintf( buf, "Creating an Air Ship at room %ld", (long)157000 +
				(long)offset+ (long) top_sailing_boat );
		log_string(buf);

    }
    else
    {
	pShip->ship->value[5] = pShip->first_room;
    }
    break;

    case SHIP_SAILING_BOAT:
    if (room_create)
    {
        pShip->ship_rooms[0] = (ROOM_INDEX_DATA *)create_and_map_room( get_room_index(ROOM_VNUM_SAILING_BOAT_HELM), (long)157000+(long)offset+(long)top_sailing_boat, pShip);

        pShip->ship->value[5] = (long)157000 + (long)offset + top_sailing_boat;
        pShip->first_room = pShip->ship->value[5];
        /*top_sailing_boat++;*/
        pShip->ship_rooms[1] = create_and_map_room( get_room_index(ROOM_VNUM_SAILING_BOAT_NEST), (long)157001+offset+top_sailing_boat, pShip);
        /*top_sailing_boat++;*/

        create_exit( pShip->ship_rooms[0], pShip->ship_rooms[1], DIR_UP, 0);
        create_exit( pShip->ship_rooms[1], pShip->ship_rooms[0], DIR_DOWN, 0);

        pShip->max_crew = 10;
        pShip->min_crew = 2;
		sprintf( buf, "Creating a Sailing Ship at room %ld", (long)157000 +
				(long)offset+ (long) top_sailing_boat );
		log_string(buf);
    }
    else
    {
	pShip->ship->value[5] = pShip->first_room;
    }
    pMastObj = create_object( get_obj_index( OBJ_VNUM_SAILING_BOAT_MAST ), 0, FALSE);
    obj_to_room(pMastObj, pShip->ship_rooms[0]);

	/*
	pCannons = create_object( get_obj_index( OBJ_VNUM_SAILING_BOAT_CANNON), 0, FALSE);
	sprintf(buf, pCannons->description, pShip->cannons);
	free_string(pCannons->description);
	pCannons->description = str_dup(buf);
	obj_to_room( pCannons, pShip->ship_rooms[0]);

	pShip->cannon_obj = pCannons;
	*/

    break;

    case SHIP_CARGO_SHIP:
    if (room_create)
    {
        pShip->ship_rooms[0] = create_and_map_room( get_room_index(ROOM_VNUM_CARGO_SHIP_HELM), (long)157000+(long)offset+(long)top_sailing_boat, pShip);

        pShip->ship->value[5] = (long)157000 + (long)offset + (long)top_sailing_boat;
        pShip->first_room = pShip->ship->value[5];

        pShip->ship_rooms[1] = create_and_map_room( get_room_index(ROOM_VNUM_CARGO_SHIP_NEST), (long)157001+(long)offset+(long)top_sailing_boat, pShip);

        pShip->ship_rooms[2] = create_and_map_room( get_room_index(ROOM_VNUM_CARGO_SHIP_BOW), (long)157002+(long)offset+(long)top_sailing_boat, pShip);

        pShip->ship_rooms[3] = create_and_map_room( get_room_index(ROOM_VNUM_CARGO_SHIP_CARGO), (long)157004+(long)offset+(long)top_sailing_boat, pShip);

        pShip->ship_rooms[4] = create_and_map_room( get_room_index(ROOM_VNUM_CARGO_SHIP_STERN), (long)157003+(long)offset+(long)top_sailing_boat, pShip);
        /*top_sailing_boat++;*/

        /* Bow to Helm*/
        create_exit( pShip->ship_rooms[2], pShip->ship_rooms[0], DIR_EAST, 0);
        create_exit( pShip->ship_rooms[0], pShip->ship_rooms[2], DIR_WEST, 0);

        /* Helm to Stern*/
        create_exit( pShip->ship_rooms[0], pShip->ship_rooms[4], DIR_EAST, 0);
        create_exit( pShip->ship_rooms[4], pShip->ship_rooms[0], DIR_WEST, 0);

		/* Helm to Cargo*/
        create_exit( pShip->ship_rooms[0], pShip->ship_rooms[3], DIR_DOWN, EX_ISDOOR | EX_CLOSED );
        create_exit( pShip->ship_rooms[3], pShip->ship_rooms[0], DIR_UP,   EX_ISDOOR | EX_CLOSED );

        /* Helm to Nest*/
        create_exit( pShip->ship_rooms[0], pShip->ship_rooms[1], DIR_UP, 0);
        create_exit( pShip->ship_rooms[1], pShip->ship_rooms[0], DIR_DOWN, 0);

		sprintf( buf, "Creating a Cargo Ship at room %ld", (long)ROOM_VNUM_CARGO_SHIP_HELM +
				(long)offset+ (long) top_sailing_boat );
		log_string(buf);
    }
    else
    {
	pShip->ship->value[5] = pShip->first_room;
    }

    /* For the bough*/
    pMastObj = create_object( get_obj_index( OBJ_VNUM_SAILING_BOAT_MAST ), 0, FALSE);
    obj_to_room(pMastObj, pShip->ship_rooms[0]);

    pMastObj = create_object( get_obj_index( OBJ_VNUM_SAILING_BOAT_MAST ), 0, FALSE);
    obj_to_room(pMastObj, pShip->ship_rooms[2]);

    pMastObj = create_object( get_obj_index( OBJ_VNUM_SAILING_BOAT_MAST ), 0, FALSE);
    obj_to_room(pMastObj, pShip->ship_rooms[4]);

	/*
    pMastObj = create_object( get_obj_index( OBJ_VNUM_SAILING_BOAT_MAST ), 0, FALSE);
    obj_to_room(pMastObj, pShip->ship_rooms[2]);
	*/

    pMastObj = create_object( get_obj_index( OBJ_VNUM_SAILING_BOAT_WHEEL ), 0, FALSE);
    obj_to_room(pMastObj, pShip->ship_rooms[0]);

    pMastObj = create_object( get_obj_index( OBJ_VNUM_SAILING_BOAT_SEXTANT ), 0, FALSE);
    obj_to_room(pMastObj, pShip->ship_rooms[0]);

	/*
	pCannons = create_object( get_obj_index( OBJ_VNUM_SAILING_BOAT_CANNON), 0, FALSE);
	sprintf(buf, pCannons->description, pShip->cannons);
	free_string(pCannons->description);
	pCannons->description = str_dup(buf);
	obj_to_room( pCannons, pShip->ship_rooms[0]);

	pShip->cannon_obj = pCannons;
	*/

    break;

    case SHIP_FRIGATE_SHIP:
    if (room_create)
    {

        pShip->ship_rooms[0] = create_and_map_room( get_room_index(ROOM_VNUM_FRIGATE_HELM), (long)157001+(long)offset+(long)top_sailing_boat, pShip);

        pShip->ship->value[5] = (long)157002 + (long)offset + (long)top_sailing_boat;
        pShip->first_room = pShip->ship->value[5];

        pShip->ship_rooms[1] = create_and_map_room( get_room_index(ROOM_VNUM_FRIGATE_BOW), (long)157004+(long)offset+(long)top_sailing_boat, pShip);

        pShip->ship_rooms[2] = create_and_map_room( get_room_index(ROOM_VNUM_FRIGATE_DECK), (long)157002+(long)offset+(long)top_sailing_boat, pShip);

        pShip->ship_rooms[3] = create_and_map_room( get_room_index(ROOM_VNUM_FRIGATE_NEST), (long)157000+(long)offset+(long)top_sailing_boat, pShip);

        pShip->ship_rooms[4] = create_and_map_room( get_room_index(ROOM_VNUM_FRIGATE_CABIN), (long)157003+(long)offset+(long)top_sailing_boat, pShip);

        pShip->ship_rooms[5] = create_and_map_room( get_room_index(ROOM_VNUM_FRIGATE_CARGO), (long)157007+(long)offset+(long)top_sailing_boat, pShip);

        pShip->ship_rooms[6] = create_and_map_room( get_room_index(ROOM_VNUM_FRIGATE_STERN), (long)157005+(long)offset+(long)top_sailing_boat, pShip);

        pShip->ship_rooms[7] = create_and_map_room( get_room_index(ROOM_VNUM_FRIGATE_NEST), (long)157008+(long)offset+(long)top_sailing_boat, pShip);

        /*top_sailing_boat++;*/

        /* Bow to Deck*/
        create_exit( pShip->ship_rooms[1], pShip->ship_rooms[2], DIR_EAST, 0);
        create_exit( pShip->ship_rooms[2], pShip->ship_rooms[1], DIR_WEST, 0);

        /* Deck to Helm*/
        create_exit( pShip->ship_rooms[2], pShip->ship_rooms[0], DIR_EAST, 0);
        create_exit( pShip->ship_rooms[0], pShip->ship_rooms[2], DIR_WEST, 0);

  	    /* Deck to Nest*/
        create_exit( pShip->ship_rooms[2], pShip->ship_rooms[3], DIR_UP, 0);
        create_exit( pShip->ship_rooms[3], pShip->ship_rooms[2], DIR_DOWN, 0);

        /* Deck to Cargo*/
        create_exit( pShip->ship_rooms[2], pShip->ship_rooms[5], DIR_DOWN, EX_CLOSED | EX_NOPASS);
        create_exit( pShip->ship_rooms[5], pShip->ship_rooms[2], DIR_UP, EX_CLOSED | EX_NOPASS);

        /* Helm to Cabin*/
        create_exit( pShip->ship_rooms[0], pShip->ship_rooms[4], DIR_DOWN, EX_CLOSED | EX_NOPASS);
        create_exit( pShip->ship_rooms[4], pShip->ship_rooms[0], DIR_UP, EX_CLOSED | EX_NOPASS);

        /* Helm to Stern*/
        create_exit( pShip->ship_rooms[0], pShip->ship_rooms[6], DIR_EAST, 0);
        create_exit( pShip->ship_rooms[6], pShip->ship_rooms[0], DIR_WEST, 0);

        /* Helm to Nest*/
        create_exit( pShip->ship_rooms[0], pShip->ship_rooms[7], DIR_UP, 0);
        create_exit( pShip->ship_rooms[7], pShip->ship_rooms[0], DIR_DOWN, 0);

		sprintf( buf, "Creating a Frigate at room %ld", (long)ROOM_VNUM_FRIGATE_DECK +
				(long)offset+ (long) top_sailing_boat );
		log_string(buf);
    }
    else
    {
	pShip->ship->value[5] = pShip->first_room;
    }

    pMastObj = create_object( get_obj_index( OBJ_VNUM_SAILING_BOAT_MAST ), 0, FALSE);
    obj_to_room(pMastObj, pShip->ship_rooms[0]);

    pMastObj = create_object( get_obj_index( OBJ_VNUM_SAILING_BOAT_MAST ), 0, FALSE);
    obj_to_room(pMastObj, pShip->ship_rooms[2]);

    pMastObj = create_object( get_obj_index( OBJ_VNUM_SAILING_BOAT_MAST ), 0, FALSE);
    obj_to_room(pMastObj, pShip->ship_rooms[6]);

    pMastObj = create_object( get_obj_index( OBJ_VNUM_SAILING_BOAT_MAST ), 0, FALSE);
    obj_to_room(pMastObj, pShip->ship_rooms[1]);

    pMastObj = create_object( get_obj_index( OBJ_VNUM_SAILING_BOAT_WHEEL ), 0, FALSE);
    obj_to_room(pMastObj, pShip->ship_rooms[0]);

    pMastObj = create_object( get_obj_index( OBJ_VNUM_SAILING_BOAT_SEXTANT ), 0, FALSE);
    obj_to_room(pMastObj, pShip->ship_rooms[0]);

	/*
	pCannons = create_object( get_obj_index( OBJ_VNUM_SAILING_BOAT_CANNON), 0, FALSE);
	sprintf(buf, pCannons->description, (int)(pShip->cannons));
	free_string(pCannons->description);
	pCannons->description = str_dup(buf);
	obj_to_room( pCannons, pShip->ship_rooms[0]);

	pShip->cannon_obj = pCannons;
	*/

    break;

    case SHIP_GALLEON_SHIP:
    if (room_create)
    {

        pShip->ship_rooms[0] = create_and_map_room( get_room_index(ROOM_VNUM_GALLEON_HELM), (long)157001+(long)offset+(long)top_sailing_boat, pShip);

        pShip->ship->value[5] = (long)157002 + (long)offset + (long)top_sailing_boat;
        pShip->first_room = pShip->ship->value[5];

        pShip->ship_rooms[1] = create_and_map_room( get_room_index(ROOM_VNUM_GALLEON_BOW), (long)157004+(long)offset+(long)top_sailing_boat, pShip);

        pShip->ship_rooms[2] = create_and_map_room( get_room_index(ROOM_VNUM_GALLEON_DECK), (long)157002+(long)offset+(long)top_sailing_boat, pShip);

        pShip->ship_rooms[3] = create_and_map_room( get_room_index(ROOM_VNUM_GALLEON_NEST), (long)157000+(long)offset+(long)top_sailing_boat, pShip);

        pShip->ship_rooms[4] = create_and_map_room( get_room_index(ROOM_VNUM_GALLEON_CABIN), (long)157003+(long)offset+(long)top_sailing_boat, pShip);

        pShip->ship_rooms[5] = create_and_map_room( get_room_index(ROOM_VNUM_GALLEON_CARGO), (long)157007+(long)offset+(long)top_sailing_boat, pShip);

        pShip->ship_rooms[6] = create_and_map_room( get_room_index(ROOM_VNUM_GALLEON_STERN), (long)157005+(long)offset+(long)top_sailing_boat, pShip);

        pShip->ship_rooms[7] = create_and_map_room( get_room_index(ROOM_VNUM_GALLEON_TREASURE), (long)157008+(long)offset+(long)top_sailing_boat, pShip);

        pShip->ship_rooms[8] = create_and_map_room( get_room_index(ROOM_VNUM_GALLEON_NEST), (long)157009+(long)offset+(long)top_sailing_boat, pShip);

        /*top_sailing_boat++;*/

        /* Bow to Deck*/
        create_exit( pShip->ship_rooms[1], pShip->ship_rooms[2], DIR_EAST, 0);
        create_exit( pShip->ship_rooms[2], pShip->ship_rooms[1], DIR_WEST, 0);

        /* Deck to Helm*/
        create_exit( pShip->ship_rooms[2], pShip->ship_rooms[0], DIR_EAST, 0);
        create_exit( pShip->ship_rooms[0], pShip->ship_rooms[2], DIR_WEST, 0);

  	    /* Deck to Nest*/
        create_exit( pShip->ship_rooms[2], pShip->ship_rooms[3], DIR_UP, 0);
        create_exit( pShip->ship_rooms[3], pShip->ship_rooms[2], DIR_DOWN, 0);

        /* Deck to Cargo*/
        create_exit( pShip->ship_rooms[2], pShip->ship_rooms[5], DIR_DOWN, EX_CLOSED | EX_NOPASS);
        create_exit( pShip->ship_rooms[5], pShip->ship_rooms[2], DIR_UP, EX_CLOSED | EX_NOPASS);

        /* Helm to Cabin*/
        create_exit( pShip->ship_rooms[0], pShip->ship_rooms[4], DIR_DOWN, EX_CLOSED | EX_NOPASS);
        create_exit( pShip->ship_rooms[4], pShip->ship_rooms[0], DIR_UP, EX_CLOSED | EX_NOPASS);

        /* Helm to Stern*/
        create_exit( pShip->ship_rooms[0], pShip->ship_rooms[6], DIR_EAST, 0);
        create_exit( pShip->ship_rooms[6], pShip->ship_rooms[0], DIR_WEST, 0);

        /* Helm to Nest*/
        create_exit( pShip->ship_rooms[0], pShip->ship_rooms[8], DIR_UP, 0);
        create_exit( pShip->ship_rooms[8], pShip->ship_rooms[0], DIR_DOWN, 0);

        /* Cargo to Treasure*/
        create_exit( pShip->ship_rooms[5], pShip->ship_rooms[7], DIR_EAST, EX_CLOSED | EX_NOPASS);
        create_exit( pShip->ship_rooms[7], pShip->ship_rooms[5], DIR_WEST, EX_CLOSED | EX_NOPASS);

		sprintf( buf, "Creating a Galleon at room %ld", (long)ROOM_VNUM_GALLEON_DECK +
				(long)offset+ (long) top_sailing_boat );
		log_string(buf);
    }
    else
    {
	pShip->ship->value[5] = pShip->first_room;
    }

    pMastObj = create_object( get_obj_index( OBJ_VNUM_SAILING_BOAT_MAST ), 0, FALSE);
    obj_to_room(pMastObj, pShip->ship_rooms[0]);

    pMastObj = create_object( get_obj_index( OBJ_VNUM_SAILING_BOAT_MAST ), 0, FALSE);
    obj_to_room(pMastObj, pShip->ship_rooms[2]);

    pMastObj = create_object( get_obj_index( OBJ_VNUM_SAILING_BOAT_MAST ), 0, FALSE);
    obj_to_room(pMastObj, pShip->ship_rooms[6]);

    pMastObj = create_object( get_obj_index( OBJ_VNUM_SAILING_BOAT_MAST ), 0, FALSE);
    obj_to_room(pMastObj, pShip->ship_rooms[1]);

    pMastObj = create_object( get_obj_index( OBJ_VNUM_SAILING_BOAT_WHEEL ), 0, FALSE);
    obj_to_room(pMastObj, pShip->ship_rooms[0]);

    pMastObj = create_object( get_obj_index( OBJ_VNUM_SAILING_BOAT_SEXTANT ), 0, FALSE);
    obj_to_room(pMastObj, pShip->ship_rooms[0]);

	/*
	pCannons = create_object( get_obj_index( OBJ_VNUM_SAILING_BOAT_CANNON), 0, FALSE);
	sprintf(buf, pCannons->description, (int)(pShip->cannons));
	free_string(pCannons->description);
	pCannons->description = str_dup(buf);
	obj_to_room( pCannons, pShip->ship_rooms[0]);

	pShip->cannon_obj = pCannons;
	*/

    break;
    }

    pShip->next = ((AREA_DATA *) get_sailing_boat_area())->ship_list;
    ((AREA_DATA *) get_sailing_boat_area())->ship_list = pShip;

	sprintf( buf, "end top_sailing_boat = %ld", top_sailing_boat );
	log_string(buf);
    return pShip;
}

void create_exit( ROOM_INDEX_DATA *from, ROOM_INDEX_DATA *to, int door, int flags)
{
    EXIT_DATA *pExit;

    pExit = new_exit();
    pExit->u1.to_room = to;
    pExit->orig_door = door;
    pExit->rs_flags = flags;
    pExit->exit_info = flags;
    from->exit[door] = pExit;
    pExit->from_room = from;
}

void save_sailing_boats()
{
	SHIP_DATA *pShipData;
	CHAR_DATA *person;
	OBJ_DATA *pObj;
	FILE *fp;
	int counter;

	fp = fopen( SAILING_FILE, "w");

	if (fp == NULL)
	{
		bug("Couldn't save sailing boat information (couldn't open file for write).", 0);
		exit(1);
	}

	counter = 0;
	for( pShipData = ((AREA_DATA *) get_sailing_boat_area())->ship_list; pShipData != NULL; pShipData = pShipData->next )
	{
		if ( pShipData->npc_ship != NULL || pShipData->ship == NULL || pShipData->ship->in_room == NULL)
		{
			continue;
		}

		counter++;
	}
	fprintf( fp, "%d\n", counter);

	for( pShipData = ((AREA_DATA *) get_sailing_boat_area())->ship_list; pShipData != NULL; pShipData = pShipData->next )
	{
		if ( pShipData->npc_ship != NULL )
		{
			continue;
		}

		if ( pShipData->ship->in_room != NULL )
		{
			fprintf( fp, "%s~\n",          pShipData->owner_name );
			fprintf( fp, "%s~\n",          pShipData->flag );
			fprintf( fp, "%s~\n",          pShipData->ship_name );
			fprintf( fp, "%ld\n",          pShipData->hit );
			fprintf( fp, "%d\n",           pShipData->cannons );
			fprintf( fp, "%d\n",           pShipData->ship_type );
			fprintf( fp, "%ld\n",          pShipData->ship->in_room->vnum );
			fprintf( fp, "%d\n",           pShipData->pk == TRUE ? 1 : 0 );

			counter = 0;
			for (person = pShipData->crew_list; person != NULL; person = person->next_in_crew)
				counter++;
			fprintf( fp, "%d\n", counter);

			for (person = pShipData->crew_list; person != NULL; person = person->next_in_crew)
			{
				fprintf( fp, "%ld\n", person->pIndexData->vnum);
				fprintf( fp, "%ld\n", (long int)person->hired_to);
			}

			counter = 0;

			/* Count contents of object*/
			for ( pObj = pShipData->ship->contains; pObj != NULL; pObj = pObj->next_content )
			{
				counter++;
			}

			fprintf(fp, "%d\n", counter );

			/* Save contents of object*/
			for ( pObj = pShipData->ship->contains; pObj != NULL; pObj = pObj->next_content )
			{
				fprintf(fp, "%ld\n", pObj->pIndexData->vnum );
			}
		}
		else
		{
			char buf[MSL];
			sprintf(buf, "The ship '%s' could not be saved as it had no room.", pShipData->ship_name);
			log_string(buf);
		}
	}

	fclose(fp);
}

void load_sailing_boats()
{
	FILE *fp;
	SHIP_DATA *pShipData;
	char *owner_name;
	char *ship_name;
	char *flag;
	long hit;
	long cannons;
	long vnum;
	long counter;
	long count;
	long count2;
	int ship_type;
	long crew_counter;
	int cargo_counter;
	bool pk;
	CHAR_DATA *crew;
	OBJ_DATA *pObj;

	log_string( "Loading boats.");
 	fp = fopen( SAILING_FILE, "r");

	if (fp == NULL)
	{
	    bug("Couldn't load sailing boat information.", 0);
	    exit(1);
	}

	if ( find_area("Wilderness") == NULL )
	{
            log_string("load_sailing_boats(): couldn't load sailing boats because there is no wilderness!");
	    return;
	}

	counter = fread_number(fp);

	for ( count = 0; count < counter; count++ )
	{

      owner_name = fread_string(fp);
      flag = fread_string(fp);
	  ship_name = fread_string(fp);
	  hit = fread_number(fp);
	  cannons = fread_number(fp);
	  ship_type = fread_number(fp);
	  vnum = fread_number(fp);

	  if ( fread_number(fp) == 1 )
	  {
		  pk = TRUE;
      }
	  else
	  {
		  pk = FALSE;
	  }


	  pShipData = create_new_sailing_boat( owner_name, ship_type);
	  pShipData->hit = hit;
	  pShipData->cannons = cannons;
      pShipData->flag = flag;
	  pShipData->ship_name = ship_name;

      obj_to_room( pShipData->ship, get_room_index(vnum));

	  /*
	  cannon = create_object( get_obj_index( OBJ_VNUM_SAILING_BOAT_CANNON), 0, FALSE);
	  sprintf(buf, cannon->description, cannons);
	  free_string(cannon->description);
	  cannon->description = str_dup(buf);

	  obj_to_room( cannon, pShipData->ship_rooms[0]);
	  */

	  crew_counter = fread_number(fp);
	  for ( count2 = 0; count2 < crew_counter; count2++ )
	  {
	    crew = create_mobile( get_mob_index( fread_number(fp) ));
	    crew->hired_to = fread_number(fp);
	    char_to_crew( crew, pShipData);
	  }

	  cargo_counter = fread_number(fp);
	  for ( count2 = 0; count2 < cargo_counter; count2++ )
	  {
		  OBJ_INDEX_DATA *pObjIndex = get_obj_index( fread_number(fp) );
		  pObj = create_object( pObjIndex, pObjIndex->level, TRUE );
		  obj_to_obj( pObj, pShipData->ship );
      }

	  /*set_correct_cannon_count_from_cargo( pShipData );*/

     }
	 fclose(fp);
	 log_string("Loaded boats.");
}

void extract_npc_boat( NPC_SHIP_DATA *npc_ship)
{
	NPC_SHIP_DATA *temp_ship;
	extract_boat( npc_ship->ship );

	temp_ship = npc_ship_list;

	if (temp_ship == npc_ship)
	{
		npc_ship_list = temp_ship->next;
	}
	else
		while( temp_ship->next != NULL )
		{
			if (temp_ship == NULL)
			{
				bug("WHEN destroying npc boat, temp_ship was NULL.", 0);
				break;
			}

			if (temp_ship->next == npc_ship)
			{
				temp_ship->next = npc_ship->next;
				log_string("Actually extracted the npc boat." );
				break;
			}

			temp_ship = temp_ship->next;
		}

	npc_ship->next = npc_sailing_boat_free;
	npc_sailing_boat_free = npc_ship;
}

void extract_boat( SHIP_DATA *ship)
{
	OBJ_DATA *obj_content;
	OBJ_DATA *obj_next;
	CHAR_DATA *temp;
	CHAR_DATA *wch;
	SHIP_DATA *temp_ship;
	AREA_DATA *pArea;
	int counter;

	/*log_string("Extracting Boat...");*/


	/* If owner of ship online, reset their belongs_to_ship variable*/
	if ( ship->owner != NULL )
	{
		ship->owner->belongs_to_ship = NULL;
	}

	while( ship->crew_list != NULL )
		extract_char( ship->crew_list, TRUE );

	/* extract waypoint*/
	clear_waypoints( ship );

  /* Remove mobs from ship*/
	for (counter = 0; counter < MAX_SHIP_ROOMS; counter++)
	{
		if ( ship->ship_rooms[counter] != NULL )
		{
			for ( temp = ship->ship_rooms[counter]->people; temp != NULL; temp = temp->next_in_room )
			{
				if ( IS_NPC( temp ) )
				{
					extract_char( temp, TRUE );
				}
			}


			for ( obj_content = ship->ship_rooms[counter]->contents; obj_content; obj_content = obj_next )
			{
				obj_next = obj_content->next_content;
				extract_obj( obj_content );
			}
		}
	}

      /* Run through players and remove them from boat*/
			for ( wch = char_list; wch != NULL ; wch = wch->next )
			{
				if ( ON_SHIP(wch) && wch->in_room->ship == ship ) {
					if ( ship->ship != NULL && ship->ship->in_room != NULL ) {
            char_from_room(wch);
						char_to_room( wch, ship->ship->in_room );
					}
					else {
						/*damage( wch, wch, 30000, TYPE_UNDEFINED, DAM_DROWNING, TRUE);*/
    				raw_kill( wch, TRUE, TRUE, RAWKILL_NORMAL ); /* 1= has head, 0= no head */
					}
				}
			}

	if ( ship->ship != NULL && ship->ship->in_room != NULL )
	{
		obj_from_room(ship->ship);
	}

	extract_obj(ship->ship);

	free_string(ship->owner_name);
	ship->owner_name = NULL;

	if ( ship->flag != NULL )
	{
		free_string(ship->flag);
	}

	free_string(ship->ship_name);

	pArea = get_sailing_boat_area(); /*get_area_data( AREA_VNUM_SAILING_BOAT);*/
	temp_ship = pArea->ship_list;

	/*log_string("Starting removal...");*/

	if (temp_ship == ship)
	{
		/*log_string("Beginning removal...");*/
		pArea->ship_list = temp_ship->next;

	}
	else
		while( temp_ship->next != NULL )
		{
			if (temp_ship == NULL)
			{
				bug("WHEN destroying boat, temp_ship was NULL.", 0);
				break;
			}

			if (temp_ship->next == ship)
			{
				/*log_string("Found it ");*/

				temp_ship->next = ship->next;
				log_string("Actually extracted the sailing boat." );
				break;
			}

			temp_ship = temp_ship->next;
		}

	if (sailing_boat_free == ship) {
		log_string("boat being extracted is already free!!!!!");
	}
	if (sailing_boat_free != NULL && sailing_boat_free->next == ship) {
		log_string("The next boat being extracted is already free!!!!!");
	}
	ship->next = sailing_boat_free;
	sailing_boat_free = ship;
}

ROOM_INDEX_DATA * create_and_map_room( ROOM_INDEX_DATA *room, long vnum, SHIP_DATA *ship)
{
    ROOM_INDEX_DATA *pRoom;
    long iHash;

    pRoom                       = new_room_index();
    pRoom->area                 = room->area;
    pRoom->vnum                 = vnum;
    pRoom->room_flags = room->room_flags;
    pRoom->sector_type = room->sector_type;
    pRoom->name = str_dup(room->name);
    pRoom->description = room->description;
    pRoom->ship = ship;
    if ( vnum > top_vnum_room )
        top_vnum_room = vnum;

    iHash                       = vnum % MAX_KEY_HASH;
    pRoom->next                 = room_index_hash[iHash];
    room_index_hash[iHash]      = pRoom;

    return pRoom;
}

bool move_boat_success(CHAR_DATA *ch)
{
	char buf[MAX_STRING_LENGTH];
	ROOM_INDEX_DATA *in_room = NULL;
	ROOM_INDEX_DATA *to_room = NULL;
	SHIP_DATA *ship;
	OBJ_DATA *obj;
	int door;
	int msg;
	int storm_type;


	in_room = ch->in_room->ship->ship->in_room;
	ship = ch->in_room->ship;
	obj = ch->in_room->ship->ship;
	door = ship->dir;

	sprintf(buf, "The ship name is %s. Character is %s", ship->ship_name, ch->name);
	log_string(buf);

	if (!in_room) {
		gecho("WARNING: IN_ROOM was null in boat_move_success");
		bug("WARNING: IN_ROOM was null in boat_move_success, removing ship", 0);
		return FALSE;
	}

	/*
	if ( (IS_NPC_SHIP(ship) && ship->npc_ship->pShipData->npc_type != NPC_SHIP_AIR_SHIP) || !IS_NPC_SHIP(ship))
	{
	if ( ( pexit   = in_room->exit[door] ) == NULL
	||   ( to_room = pexit->u1.to_room   ) == NULL
	||   ( get_room_index(to_room->parent)->sector_type != SECT_WATER_NOSWIM &&
	get_room_index(to_room->parent)->sector_type != SECT_WATER_SWIM )
	||   !can_see_room(ch,pexit->u1.to_room))
	{
	*/

	if ( (IS_NPC_SHIP(ship) && ship->npc_ship->pShipData->npc_type != NPC_SHIP_AIR_SHIP) || !IS_NPC_SHIP(ship))
	{
	if (/* ( to_room = get_wilderness_room_for_exit(in_room, door)   ) == NULL
	||*/   ( to_room->sector_type != SECT_WATER_NOSWIM &&
	to_room->sector_type != SECT_WATER_SWIM )
	||   !can_see_room(ch,to_room))
	{

	/*act("The vessel has run aground.", ch, NULL, NULL, TO_CHAR);*/
	boat_echo(ship, "The vessel has run aground.");
	ship->speed = SHIP_SPEED_STOPPED;
	ship->last_room[0] = NULL;
	ship->last_room[1] = NULL;
	ship->last_room[2] = NULL;
	return FALSE;
	}
	}

	storm_type = get_storm_for_room(in_room);

	msg = number_range(0, 3);

	if (number_percent() < 5)
	switch ( storm_type ) {
	case WEATHER_NONE:
	switch( number_range(0, 7) ) {
	case 0 :
	send_to_char("The vessel gently glides through the smooth ocean waves.\n\r", ch);
	break;
	case 1 :
	send_to_char("A spray of water whips up the side of the vessel and sparkles in the sun.\n\r", ch);
	break;
	case 2 :
	send_to_char("The sun reflects blindingly off the shiny clean deck.\n\r", ch);
	break;
	case 3 :
	send_to_char("The sails above flutter as the cool wind changes.\n\r", ch);
	break;
	case 4 :
	send_to_char("The wind rushes through your hair has the vessel glides through the waves.\n\r", ch);
	break;
	case 5 :
	send_to_char("The vessel crashes up and down through the ocean waves.\n\r", ch);
	break;
	case 6 :
	send_to_char("Water splashes up around the sides of the vessel as it cuts through the ocean waves.\n\r", ch);
	break;
	case 7 :
	send_to_char("The sun beams down lighting the calm seas ahead.\n\r", ch);
	break;
	}
	break;
	case WEATHER_RAIN_STORM:
	switch( number_range(0, 3) ) {
	case 0 :
	send_to_char("The vessel sways haphazardly up and down in the rough ocean waves. The rain pours mercilessly down upon you.\n\r", ch);
	break;
	case 1 :
	send_to_char("The vessel crashes up and down through the unforgiving ocean waves.\n\r", ch);
	break;
	case 2 :
	send_to_char("Rain splashes down upon the deck as the vessel crashes through the waves.\n\r", ch);
	break;
	case 3 :
	send_to_char("The vessel crashes up and down through the waves as the rain pours upon you.\n\r", ch);
	break;
	}
	break;
	case WEATHER_LIGHTNING_STORM:
	switch( number_range(0,3) ) {
	case 0 :
	send_to_char("The vessel groans as it is thrown mercilessly from side to side through the huge waves.\n\r", ch);
	break;
	case 1 :
	send_to_char("A large wave crashes into the side of the vessel hurtling the crew across the deck.\n\r", ch);
	break;
	case 2 :
	send_to_char("The vessel crashes through the waves, the cargo onboard thrown around the cabins.\n\r", ch);
	break;
	case 3 :
	send_to_char("A splatter of salty water showers upon you as your boat smashes into yet another wave.\n\r", ch);
	break;
	}
	case WEATHER_HURRICANE:
	switch( number_range(0,6) ) {
	case 0 :
	send_to_char("The waves grow to the size of mountains, dwarfing the vessel!\n\r", ch);
	break;
	case 1 :
	send_to_char("The vessel plummets tens of meters as a wave crashes into another!\n\r", ch);
	boat_damage( ship, number_range(800, 1000), SHIP_DAMAGE_GRIND );
	break;
	case 2 :
	send_to_char("A wave crashes into the vessel splintering the ship and sending crew overboard!\n\r", ch);
	boat_damage( ship, number_range(200, 600), SHIP_DAMAGE_GRIND );
	break;
	case 3 :
	send_to_char("The massive waves curls and dumps on the vessel pushing it under momentarily!\n\r", ch);
	boat_damage( ship, number_range(1000, 1500), SHIP_DAMAGE_GRIND );
	break;
	case 4 :
	send_to_char("A sail snaps from the mast and disappears into the storm!\n\r", ch);
	break;
	case 5 :
	send_to_char("The powerful wind changes, pushing the ship sideways as the crew struggle to turn the vessel!\n\r", ch);
	boat_damage( ship, number_range(400, 800), SHIP_DAMAGE_GRIND );
	break;
	case 6 :
	send_to_char("You hear a scream through the roaring wind as a crew member is sucked overboard!\n\r", ch);
	break;
	}
	case WEATHER_TORNADO:
	switch( number_range(0,4) ) {
	case 0 :
	send_to_char("The swirling column of devestation sucks up water from the seas!\n\r", ch);
	break;
	case 1 :
	send_to_char("A crew member screams as they are sucked up into the tornado!\n\r", ch);
	break;
	case 2 :
	send_to_char("The waves smash into the vessel, splintering the hull!\n\r", ch);
	boat_damage( ship, number_range(200, 600), SHIP_DAMAGE_GRIND );
	break;
	case 3 :
	send_to_char("The vessel groans as parts of the vessel are torn and sucked into the tornado!\n\r", ch);
	boat_damage( ship, number_range(800, 1000), SHIP_DAMAGE_GRIND );
	break;
	case 4 :
	send_to_char("A sail snaps from the mast and disappears into the storm!\n\r", ch);
	boat_damage( ship, number_range(100, 300), SHIP_DAMAGE_GRIND );
	break;
	}
	}

	obj = ship->ship;

	/* keep last_rooms populated*/
	ship->last_room[2] = ship->last_room[1];
	ship->last_room[1] = ship->last_room[0];
	ship->last_room[0] = obj->in_room;

	sprintf(buf, "{W%s sails away to the %s.{x\n\r", capitalize(obj->short_descr), dir_name[ship->dir]);
	room_echo(in_room, buf);

	obj_from_room(obj);
	obj_to_room(obj, to_room);

	sprintf(buf, "{W%s sails in from the %s.{x\n\r", capitalize(obj->short_descr), dir_name[rev_dir[ship->dir]]);
	room_echo(in_room, buf);

	return TRUE;
}

void boat_move( CHAR_DATA *ch )
{
    SHIP_DATA *ship;
    bool success;

    if (ch == NULL || ch->in_room == NULL || ch->in_room->ship == NULL)
    {
	return;
    }

    ship = ch->in_room->ship;
    if ( ship->speed == SHIP_SPEED_STOPPED )
    {
	boat_echo(ship, "{WThe vessel has stopped.{x");
        ship->last_room[0] = NULL;
        ship->last_room[1] = NULL;
        ship->last_room[2] = NULL;
	return;
    }

    success = move_boat_success(ch);

    if (!success)
	{
	return;
	}

    /*if (!IS_SET(ch->comm, COMM_BRIEF))*/
     if (IS_SET(ch->act2, PLR_AUTOSURVEY) && IS_AWAKE(ch))
        do_function(ch, &do_survey, "auto" );

    if ( ship->speed == SHIP_SPEED_FULL_SPEED )
    {
   	/*success = move_boat_success(ch);*/
	SHIP_STATE(ch, ship->ship->value[1]);
    }
    else
	SHIP_STATE(ch, ship->ship->value[1]*2);
}

void boat_echo( SHIP_DATA *ship, char *str )
{
    DESCRIPTOR_DATA *d;

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
       CHAR_DATA *victim;

       victim = d->original ? d->original : d->character;

       if ( d->connected == CON_PLAYING && victim->in_room != NULL && victim->in_room->ship == ship )
 	   act(str, victim, NULL, NULL, TO_CHAR);
    }
}

void boat_explode( SHIP_DATA *ship )
{
        DESCRIPTOR_DATA *d;
	ROOM_INDEX_DATA *location;
        OBJ_DATA *obj;
		bool explode = FALSE;

        if (number_percent() < 50)
	{
        	boat_echo(ship, "{RWith a loud bang, the vessel explodes into debris!{x");
			explode = TRUE;
	}
	else
	{
		boat_echo(ship, "{BWith a loud gurgle, the vessel sinks into the sea!{x");
	}

	location = ship->ship->in_room;

        for ( d = descriptor_list; d != NULL; d = d->next )
        {
           CHAR_DATA *victim;

           victim = d->original ? d->original : d->character;

           if ( d->connected == CON_PLAYING && victim->in_room != NULL && victim->in_room->ship == ship )
	   {
	       if (IS_MSP(victim))
	           send_to_char( sound_table[ SOUND_SINK ].tag, victim);

		   if ( !explode )
		   {
	       		char_from_room(victim);
	       		char_to_room(victim, location);
		   }
		   else
		   {
	            damage( victim, victim, 30000, TYPE_UNDEFINED, DAM_FIRE, TRUE);
		   }
	   		}
        }

	obj = create_object( get_obj_index( OBJ_VNUM_SAILING_BOAT_DEBRIS ), 0, FALSE);
	obj->timer = 20;

	if ( !location ) {
		bug( "BOAT HAD NO LOCATION IN EXPLODE!", 0 );
		extract_obj( obj );
		return;
	}

	obj_to_room(obj, location);

	extract_boat(ship);
}

void boat_damage( SHIP_DATA *ship, long amount, int type )
{
        DESCRIPTOR_DATA *d;

	ship->hit -= amount;

	if ( type == SHIP_DAMAGE_FIRE )
	{
            for ( d = descriptor_list; d != NULL; d = d->next )
            {
       		CHAR_DATA *victim;

       		victim = d->original ? d->original : d->character;

      		 if ( d->connected == CON_PLAYING && victim->in_room != NULL && victim->in_room->ship == ship )
	    		damage( victim, victim, number_percent(), TYPE_UNDEFINED, DAM_FIRE, FALSE);
	    }
	}

	if (ship->hit <= 0)
	{
	    boat_explode( ship );
	    return;
	}

}

void boat_attack( CHAR_DATA *ch )
{
    SHIP_DATA *ship;
    CHAR_DATA *victim = NULL;
	char buf[MSL];

    if (ch == NULL || ch->in_room == NULL || ch->in_room->ship == NULL)
	return;

    ship = ch->in_room->ship;

    if ( ship->attack_position == SHIP_ATTACK_STOPPED )
    {
	boat_echo(ship, "The vessel comes to a stop.");
	return;
    }

    if (ship->attack_position == SHIP_ATTACK_LOADING )
    {
	bool in_range;
	int x, y;

	in_range = FALSE;

        x = get_squares_to_show_x(0);
        y = get_squares_to_show_y(0);

        /* Check for ship in range*/
        if ( ship->ship_attacked != NULL && ship->ship_attacked->ship != NULL)
	  if (ship->ship_attacked->ship->in_room != NULL)
        if (
	( ship->ship_attacked->ship->in_room->x < ship->ship->in_room->x + x &&
	 ship->ship_attacked->ship->in_room->x > ship->ship->in_room->x - x) &&
	( ship->ship_attacked->ship->in_room->y < ship->ship->in_room->x + x &&
	 ship->ship_attacked->ship->in_room->y > ship->ship->in_room->y - y))
            {
                  in_range = TRUE;
            }

        /* Check for player in range*/
        if ( ship->char_attacked != NULL)
	  if (ship->char_attacked->in_room != NULL && IN_WILDERNESS(ship->char_attacked))
        if (
	( ship->char_attacked->in_room->x < ship->ship->in_room->x + x &&
	 ship->char_attacked->in_room->x > ship->ship->in_room->x - x) &&
	( ship->char_attacked->in_room->y < ship->ship->in_room->x + x &&
	 ship->char_attacked->in_room->y > ship->ship->in_room->y - y))
            {
                  in_range = TRUE;
	    }

        if (in_range == (bool)NULL)
        {
            send_to_char("That target isn't in range.\n\r", ch);
	    ship->attack_position = SHIP_ATTACK_STOPPED;
	    ship->ship_attacked = NULL;
	    ship->char_attacked = NULL;
            return;
        }


	boat_echo(ship, "The crew start loading the cannons.");
	ship->attack_position = SHIP_ATTACK_FIRING;
        SHIP_ATTACK_STATE(ch, 12);
	return;
    }

    if (ship->attack_position == SHIP_ATTACK_FIRING )
    {
	boat_echo(ship, "{RThere is a tremendous boom as the cannons fire.{x");

        if (ship->ship_attacked != NULL)
	    boat_echo(ship->ship_attacked, "{RYou hear the cannons of a distant vessel fire.{x");

        if (ship->char_attacked != NULL)
	    send_to_char("{RYou hear the cannons of a vessel fire.{x\n\r", ship->char_attacked);

	ship->attack_position = SHIP_ATTACK_FIRED;
        SHIP_ATTACK_STATE(ch, 12);

        if (IS_MSP(ch))
	   send_to_char( sound_table[ SOUND_CANNON ].tag, ch);

	return;
    }

    if (ship->attack_position == SHIP_ATTACK_FIRED )
    {
        int chance=0;
        long dam;

		if ( IS_SHIP_IN_HARBOUR( ship ) )
	{
		boat_echo(ship, "{MThe projectile drops out of the sky due to magical interference.{x" );

        if (ship->ship_attacked != NULL)
		boat_echo(ship->ship_attacked, "{MThe projectile drops out of the sky due to magical interference.{x" );

        if (ship->char_attacked != NULL)
		send_to_char( "{MThe projectile drops out of the sky due to magical interference.{x", ship->char_attacked );
		return;
	}

	switch(ship->speed)
	{
	case SHIP_SPEED_STOPPED:
	    chance = 70;
	    break;
	case SHIP_SPEED_HALF_SPEED:
	    chance = 50;
	    break;
	case SHIP_SPEED_FULL_SPEED:
	    chance = 30;
	    break;
	}

        if (ship->ship_attacked != NULL)
	switch(ship->ship_attacked->speed)
	{
	case SHIP_SPEED_STOPPED:
	    break;
	case SHIP_SPEED_HALF_SPEED:
	    chance -= 20;
	    break;
	case SHIP_SPEED_FULL_SPEED:
	    chance -= 30;
	    break;
	}
        else
	  chance -=40;

        dam = ship->cannons * (number_range(1,20)) + 25;

        if (number_percent() < chance)
	{
            if ( number_percent() < 5 && ship->char_attacked != NULL)
            {
	        CHAR_DATA *random;
                random = (CHAR_DATA *)get_random_char( NULL, NULL, ship->char_attacked->in_room, NULL);
		act("{RWhat luck! Your head is taken off by a stray cannon ball!{x", random, NULL, NULL, TO_CHAR);
		act("{RA cannon ball appears out of nowhere taking $n's head off!{x", random, NULL, NULL, TO_ROOM);
		damage( ch, random, 30000, TYPE_UNDEFINED, DAM_FIRE, FALSE);
	        ship->attack_position = (sh_int)SHIP_ATTACK_LOADING;
                SHIP_ATTACK_STATE(ch, 8);
	        return;
	    }

            if (ship->ship_attacked != NULL)
            {
	        if (number_percent() < 25)
                {
		    boat_echo(ship->ship_attacked, "{Y*{R**{W*{YBBBOOOOOOOOM{W*{R***{Y* {xA ball of flame engulfs the vessel from a direct hit!{x");
                }
	        else
	        if (number_percent() < 50)
                {
		    boat_echo(ship->ship_attacked, "{RA cannon ball skims up from a wave smacking into the hull!{x");
		    boat_echo(ship->ship_attacked, "Splinters erupt from the vessel!");
                }
	        else
	        if (number_percent() < 75)
                {
		    boat_echo(ship->ship_attacked, "{RAn explosion lights up the deck as a cannon ball plummets into the vessel!{x");
                }
	        else
                {
		    boat_echo(ship->ship_attacked, "{RThe boat is splintered as a cannon ball thuds into the deck.{x");
                }
            }

	    boat_echo(ship, "{YThe sea lights up as a distant vessel flashes from a direct hit.{x");

	    /*sprintf(buf, "Damage taken by %s damage %ld hp %d cannons  \n\r", ship->ship_name, dam, ship->cannons);*/
		/*log_string(buf);*/

			sprintf(buf, "The vessel is struck for {Y%ld{x hit points damage!", dam);
			boat_echo(ship->ship_attacked, buf);

            if (ship->ship_attacked != NULL)
	        boat_damage(ship->ship_attacked, dam, SHIP_DAMAGE_FIRE);

	    if (ship->char_attacked != NULL)
		damage( ch, victim, dam, TYPE_UNDEFINED, DAM_FIRE, FALSE);


            if (IS_MSP(ch))
	        send_to_char( sound_table[ SOUND_CANNON_EXPLODE ].tag, ch);
	}
	else
	{
            if (ship->ship_attacked != NULL)
	    {
	    if (number_percent() < 25)
		boat_echo(ship->ship_attacked, "{B*{C*{WSPLASH{C*{B* {xWater splashes the boat as a cannon ball barely misses.{x");
	    else
	    {
		boat_echo(ship->ship_attacked, "{CA cannon ball splashes into the water close by.{x");
	    }
		if (IS_MSP(ch))
		    send_to_char( sound_table[ SOUND_CANNON_SPLASH ].tag, ch);
	    }

	    if (ship->char_attacked != NULL)
            {
	    if (number_percent() < 25)
		send_to_char("A mighty big cannon ball barely misses you!\n\r", ship->char_attacked);
	    else
		send_to_char("There is a loud thud as a cannon ball hits the ground not far away.\n\r", ship->char_attacked);
	    }
	}

	ship->attack_position = SHIP_ATTACK_LOADING;
        SHIP_ATTACK_STATE(ch, 8);
	return;
    }
}

void do_damage( CHAR_DATA *ch, char *argument)
{
	char buf[MSL];
	CHAR_DATA *victim;
	int counter = 0;

	if (ch->in_room == NULL)
		return;

	if (ch->in_room->ship == NULL)
	{
		act("You arn't on a vessel.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	sprintf(buf, "{MThe %s you are on ", boat_table[ch->in_room->ship->ship_type].name);
	send_to_char(buf, ch);

	if ( ch->in_room->ship->scuttle_time > 0 )
		send_to_char("is sinking.{x\n\r", ch);
	else
	if ( ch->in_room->ship->hit < ch->in_room->ship->ship->value[6]/4)
		send_to_char("is about to sink.{x\n\r", ch);
	else
	if ( ch->in_room->ship->hit < ch->in_room->ship->ship->value[6]/3)
		send_to_char("is on fire!{x\n\r", ch);
	else
	if ( ch->in_room->ship->hit < ch->in_room->ship->ship->value[6]/2)
		send_to_char("is badly damaged.{x\n\r", ch);
	else
	if ( ch->in_room->ship->hit < ch->in_room->ship->ship->value[6]/2 +
				      ch->in_room->ship->ship->value[6]/4)
		send_to_char("has some structural damage.{x\n\r", ch);
	else
	if ( ch->in_room->ship->hit < ch->in_room->ship->ship->value[6]/2 +
				      ch->in_room->ship->ship->value[6]/3)
		send_to_char("is in good condition.{x\n\r", ch);
	else
		send_to_char("is in excellent condition.{x\n\r", ch);

	/* Count crew*/
	for ( victim = ch->in_room->ship->crew_list; victim != NULL; victim = victim->next_in_crew)
	{
		counter++;
	}

	sprintf(buf, "{YHit Points: {x%ld {Yof {x%d \n\r"
				 "{YCrew Left : {x%d\n\r",
				ch->in_room->ship->hit, ch->in_room->ship->ship->value[6],
				counter);
	send_to_char(buf, ch);
	return;
}

AREA_DATA *get_sailing_boat_area()
{
	AREA_DATA *temp;

        for (temp = area_first; temp != NULL; temp = temp->next)
        {
            if (temp->name != NULL && !str_cmp(temp->name, "Sailing Ships"))
                break;
        }

        if ( temp == NULL )
        {
            bug("Couldn't find area Sailing Ships.", 0);
        }

        return temp;
}

void do_crew( CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    SHIP_DATA *ship;
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int  counter;

    argument = one_argument( argument, arg);
    argument = one_argument( argument, arg2);

    if ( arg[0] == '\0')
    {
        for ( ship = ((AREA_DATA *) get_sailing_boat_area())->ship_list; ship !=NULL; ship = ship->next)
	{
	    if ( !str_cmp(ship->owner_name, ch->name))
		break;
	}

	if ( ship == NULL )
	{
	    send_to_char("You don't have any crew because you don't own a boat.\n\r", ch);
	    return;
	}

	send_to_char("{YYour current crew consist of:{x\n\r", ch);
	send_to_char("{YNo.    Name                 Race       Level        Hired till{x\n\r", ch);
	send_to_char("{B--------------------------------------------------------------------{x\n\r", ch);

	counter = 0;
	for ( victim = ship->crew_list; victim != NULL; victim = victim->next_in_crew)
	{
	    counter++;
	    sprintf(buf, "{G%-6d{x %-20s %-10s %-12d %s", counter, victim->short_descr, race_table[victim->race].name, victim->tot_level, (char *) ctime( &victim->hired_to) );
	    send_to_char(buf, ch);
	}

	sprintf(buf, "\n\rCrew found: %d\n\r", counter);
	send_to_char( buf, ch );
	return;
    }
}

/* Return npc mobs back to their own ship after they have won */
void make_ship_crew_return_to_ship( SHIP_DATA *boarding_ship )
{
	CHAR_DATA *mob;
	CHAR_DATA *captain;

	if (IS_NPC_SHIP(boarding_ship))
	{
		captain = boarding_ship->npc_ship->captain;
	}
	else
	{
		captain = boarding_ship->owner;
	}

	boat_echo( boarding_ship, "Victorious, the excited crew plunder what they can before returning to their vessel!{x" );

	/* Transfer mobs to boarding ship*/
	for (mob = boarding_ship->crew_list; mob != NULL; mob = mob->next_in_crew)
	{
		/* If its the player then don't send him back*/
		if ( !IS_NPC(mob) )
		{
			continue;
		}
		char_from_room(mob);
		char_to_room(mob, get_room_index(boarding_ship->first_room));
	}

	if ( captain != NULL && !IS_DEAD(captain) )
	{
		char_from_room(captain);
		char_to_room(captain, get_room_index(boarding_ship->first_room));
	}
}

void stop_boarding( SHIP_DATA *ship )
{
    SHIP_DATA *boarding_ship;

    if (ship->boarded_by == NULL)
    {
	bug("Boarded by ship when stop_boarding called was null!",0);
	return;
    }

    boarding_ship = ship->boarded_by;

    make_ship_crew_return_to_ship( boarding_ship );

    /* Send boarding crew back to their ship */
	/*
    for ( crew = ship->crew_list; crew != NULL; crew = crew->next_in_crew)
    {
        if ( crew->belongs_to_ship == boarding_ship )
        {
            act("$n returns to $s ship.", crew, NULL, NULL, TO_ROOM);
            char_from_room(crew);
            char_to_room(crew, get_room_index(boarding_ship->first_room));
        }
	crew->boarded_ship = NULL;
    }
    */

/*gecho("setting boarded_by");*/
/*gecho("currently...");*/
   /*if (ship->boarded_by != NULL) gecho("not null"); else gecho("null");*/

    if ( IS_NPC_SHIP(boarding_ship) )
    {
		boarding_ship->npc_ship->state = NPC_SHIP_STATE_STOPPED;
    }

    ship->boarded_by->boarded_by = NULL;
    ship->boarded_by = NULL;

}

void do_scuttle( CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *temp_char;
    SHIP_DATA *ship;

    /* Is player on a boat? */
    if ( ( ship = ch->in_room->ship ) == NULL )
    {
		send_to_char("You arn't on a vessel.\n\r", ch);
		return;
    }

    if ( ship->scuttle_time > 0 )
    {
		send_to_char("The vessel has already been scuttled!\n\r", ch);
		return;
    }

	if ( is_boat_safe( ch, ship, ship ) )
	{
		send_to_char("The vessel is docked in a protected area.\n\r", ch);
		return;
	}

    /* Is player on a boarded boat? */
    if ( IS_NPC_SHIP(ship) )
    {
	/* Is captain on the ship or dead */
        if ( ship->npc_ship->captain != NULL )
        {
           send_to_char("You can't scuttle this vessel, the captain is not dead yet!\n\r", ch);
	   return;
        }
    }
    else
    {
	for ( temp_char = ship->crew_list; temp_char != NULL; temp_char = temp_char->next_in_crew)
        {
	    if ( temp_char == ship->owner )
            {
		break;
            }
        }

        if ( temp_char != NULL )
        {
	    send_to_char("The captain is still on the vessel!\n\r", ch);
	    return;
        }
    }

    sprintf(buf, "%s douses the vessel with fuel and ignites it!", ch->name);
    boat_echo(ship, buf);

    if ( ( ship->ship->in_room->vnum == ROOM_VNUM_SEA_PLITH_HARBOUR ||
		 ship->ship->in_room->vnum == ROOM_VNUM_SEA_SOUTHERN_HARBOUR ||
		 ship->ship->in_room->vnum == ROOM_VNUM_SEA_NORTHERN_HARBOUR ) &&
		 ship->owner != ch )
	{
		boat_echo(ship, "{MThe flames are extinguished by a mysterious protective magic.{x");
		return;
	}

    ship->scuttle_time = 5;

    if ( ship->boarded_by != NULL )
    {
        ship->boarded_by->ship_attacked = NULL;
        ship->boarded_by->ship_chased = NULL;
        ship->boarded_by->destination = NULL;
    }
    ship->ship_attacked = NULL;
    ship->ship_chased = NULL;
    ship->destination = NULL;

    stop_boarding(ship);
}

void do_ship_flag( CHAR_DATA *ch, char *argument )
{
	/*char arg[MAX_INPUT_LENGTH];*/
	char buf[MAX_STRING_LENGTH];
	SHIP_DATA *ship;

	/*argument = one_argument(argument, arg);*/

    for ( ship = ((AREA_DATA *) get_sailing_boat_area())->ship_list; ship !=NULL; ship = ship->next)
    {
	if (ship->owner == ch)
        {
	    break;
        }
    }

    if (ship == NULL)
    {
		send_to_char("You don't own a vessel!\n\r", ch);
		return;
    }


	if ( argument[0] == '\0' )
	{
		sprintf(buf, "The flag flying on your vessel is, '%s{x'.\n\r", ship->flag);
		send_to_char(buf, ch);
		return;
	}

	free_string(ship->flag);
	smash_tilde(argument);
	ship->flag = str_dup(argument);
	sprintf(buf, "Your vessel's flag has been changed to, '%s{x'.\n\r", ship->flag);
	send_to_char(buf, ch);
	return;
}

void do_cargo( CHAR_DATA *ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *pObj = NULL;
	OBJ_DATA *cart = NULL;

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	/* Is player on a boat? */
	if ( ch->in_room->ship == NULL )
	{
		send_to_char("You arn't on a vessel.\n\r", ch);
		return;
	}

	if ( ch->in_room->ship->ship == NULL )
	{
		bug( "Cargo called but the ship had no object ship attached to it!?.", 0 );
		return;
	}

	if ( arg1[0] == '\0' )
	{
		send_to_char("The cargo hold contains:\n\r", ch);
		show_list_to_char(ch->in_room->ship->ship->contains, ch, TRUE, TRUE);
		return;
	}

	if ( !str_cmp( arg1, "get" ) )
	{
		if ( !str_cmp( ch->name, ch->in_room->ship->owner_name ) && ch->in_room->ship->scuttle_time > 0 )
		{
			send_to_char( "This isn't your vessel. It must be scuttled before you can take cargo.\n\r", ch );
			return;

		}

		if ( arg2[0] == '\0' )
		{
			send_to_char("Get what from the cargo hold?\n\r", ch );
			return;
		}

		if ( (cart = PULLING_CART(ch)) == NULL )
		{
			send_to_char("You arn't pulling a cart.\n\r", ch );
			return;
		}

		if ( str_cmp( arg2, "all" ) && str_prefix( "all.", arg2 ) )
		{
			pObj = get_obj_list( ch, arg2, ch->in_room->ship->ship->contains );

			if ( pObj == NULL
					|| ( pObj != NULL && IS_SET(pObj->extra_flags, ITEM_HIDDEN)))
			{
				act( "There is no $T in the cargo hold.", ch, NULL, arg2, TO_CHAR );
				return;
			}

			if ( get_obj_weight( pObj ) + get_obj_weight_container( cart )
					> ( cart->value[0])
					||  (get_obj_number_container( cart) >= cart->value[3]))
			{
				send_to_char( "Your cart is full.\n\r", ch );
				return;
			}

			obj_from_obj( pObj );
			obj_to_obj( pObj, cart );

			act( "You get $p from the vessels cargo hold and place it in your cart.", ch, pObj, NULL, TO_CHAR );
			act( "$n gets $p from the vessels cargo hold and places it in $s cart.", ch, pObj, NULL, TO_ROOM );

			return;
		}
		else
		{
			if ( !str_cmp( arg2, "all" ) )
			{
	    OBJ_DATA *obj;
	    bool found = TRUE;

		    if (ch->in_room->ship->ship->contains == NULL) {
				  send_to_char("There is nothing to transfer.\n\r", ch);
          return;
	      }

			while ( found )
			{
		    found = FALSE;
        obj = ch->in_room->ship->ship->contains;

				if ( obj != NULL )
				{
					found = TRUE;

					if ( !IS_SET( obj->extra_flags, ITEM_HIDDEN ) )
					{
						if ( get_obj_weight( obj ) + get_obj_weight_container( cart )
								> ( cart->value[0])
								||  (get_obj_number_container( cart) >= cart->value[3]))
						{
							send_to_char( "Your cart is full.", ch );
							break;
						}

						obj_from_obj( obj );
						obj_to_obj( obj, cart );

						act( "You get $p from the vessels cargo hold and place it in your cart.", ch, obj, NULL, TO_CHAR );
						act( "$n gets $p from the vessels cargo hold and places it in $s cart.", ch, obj, NULL, TO_ROOM );
					}
					}
				}
			}
			else
			{
				if ( !str_prefix( arg2, "all." ) )
				{
					int counter = 0;

					pObj = ch->in_room->ship->ship->contains;
					strcpy(arg2, arg2+4);

					while ( pObj != NULL )
					{
						if ( !IS_SET( pObj->extra_flags, ITEM_HIDDEN ) &&
								is_name( pObj->name, arg2 ) )
						{
							if ( get_obj_weight( pObj ) + get_obj_weight_container( cart )
									> ( cart->value[0])
									||  (get_obj_number_container( cart) >= cart->value[3]))
							{
								break;
							}

							obj_from_obj( pObj );
							obj_to_obj( pObj, cart );
							counter++;
						}

						pObj = pObj->next_content;
					}

					if ( counter == 0 )
					{
						send_to_char( "There arn't any in the cargo hold.", ch );
					}
					else
					{
						sprintf( buf, "{Y({G%2d{Y) {xYou get $p from the vessels cargo hold and place it in your cart.", counter );
						act( buf, ch, pObj, NULL, TO_CHAR );
						sprintf( buf, "{Y({G%2d{Y) {x$n gets $p from the vessels cargo hold and places it in $s cart.", counter );
						act( buf, ch, pObj, NULL, TO_CHAR );
					}
				}
			}
		}
	}
	else
	if ( !str_cmp( arg1, "put" ) )
	{
		OBJ_DATA *pShipObj = NULL;

		pShipObj = ch->in_room->ship->ship;

		if ( arg2[0] == '\0' )
		{
			send_to_char("Put what in the cargo hold?\n\r", ch );
			return;
		}

		if ( (cart = PULLING_CART(ch)) == NULL )
		{
			send_to_char("You arn't pulling a cart.\n\r", ch );
			return;
		}

		if ( str_cmp( arg2, "all" ) && str_prefix( "all.", arg2 ) )
		{
			pObj = get_obj_list( ch, arg2, cart->contains );

			if ( pObj == NULL
					|| ( pObj != NULL && IS_SET(pObj->extra_flags, ITEM_HIDDEN)))
			{
				act( "There is no $T in your cart.", ch, NULL, arg2, TO_CHAR );
				return;
			}

			if ( get_obj_weight( pObj ) + get_obj_weight_container( pShipObj )
					> ( pShipObj->value[0])
					||  (get_obj_number_container( pShipObj ) >= pShipObj->value[3]))
			{
				send_to_char( "The vessels cargo hold is full.", ch );
				return;
			}

      if ( pObj->pIndexData->vnum == OBJ_VNUM_TRADE_CANNON &&
           pShipObj->ship->cannons >= pShipObj->value[7] ) {
				send_to_char( "The vessels can not hold any more cannons.", ch );
				return;
      }

			obj_from_obj( pObj );
			obj_to_obj( pObj, pShipObj );

			act( "$n puts $p from $s cart into the vessels cargo hold.", ch, pObj, NULL, TO_ROOM );
			act( "You put $p from your cart into the vessels cargo hold.", ch, pObj, NULL, TO_CHAR );
		}
		else
			if ( !str_cmp( arg2, "all" ) )
			{
	    OBJ_DATA *obj;
	    bool found = TRUE;

		    if (cart->contains == NULL) {
				  send_to_char("There is nothing to transfer.\n\r", ch);
          return;
	      }

			while ( found )
			{
		    found = FALSE;
        obj = cart->contains;

				if ( obj != NULL )
				{
					found = TRUE;

						if ( !IS_SET( obj->extra_flags, ITEM_HIDDEN ) )
						{
							if ( get_obj_weight( obj ) + get_obj_weight_container( pShipObj )
									> ( pShipObj->value[0])
									||  (get_obj_number_container( pShipObj ) >= pShipObj->value[3]))
							{
								send_to_char( "The vessels cargo hold is full.\n\r", ch );
								break;
							}

      if ( obj->pIndexData->vnum == OBJ_VNUM_TRADE_CANNON &&
           pShipObj->ship->cannons >= pShipObj->value[7] ) {
				send_to_char( "The vessels can not hold any more cannons.", ch );
				break;
      }

							obj_from_obj( obj );
							obj_to_obj( obj, pShipObj );

							act( "$n puts $p from $s cart into the vessels cargo hold.", ch, obj, NULL, TO_ROOM );
							act( "You put $p from your cart into the vessels cargo hold.", ch, obj, NULL, TO_CHAR );
						}
			    }
			  }
		  }
			else
				if ( !str_prefix( arg2, ".all" ) )
				{
					int counter = 0;

					strcpy(arg2, arg2+4);

					pObj = cart->contains;

					while ( pObj != NULL )
					{
						if ( !IS_SET( pObj->extra_flags, ITEM_HIDDEN ) &&
								is_name( pObj->name, arg2 ) )
						{
							if ( get_obj_weight( pObj ) + get_obj_weight_container( pShipObj )
									> ( pShipObj->value[0])
									||  (get_obj_number_container( pShipObj ) >= pShipObj->value[3]))
							{
								send_to_char( "The vessels cargo hold is full.\n\r", ch );
								break;
							}

      if ( pObj->pIndexData->vnum == OBJ_VNUM_TRADE_CANNON &&
           pShipObj->ship->cannons >= pShipObj->value[7] ) {
				send_to_char( "The vessels can not hold any more cannons.", ch );
				break;
      }

							obj_from_obj( pObj );
							obj_to_obj( pObj, pShipObj );
							counter++;
						}

						pObj = pObj->next_content;
					}

					if ( counter == 0 )
					{
						send_to_char( "There arn't any in the cargo hold.\n\r", ch );
					}
					else
					{
						sprintf( buf, "{Y({G%2d{Y) {xYou put $p from your cart into the vessels cargo hold.", counter );
						act( buf, ch, pObj, NULL, TO_CHAR );
						sprintf( buf, "{Y({G%2d{Y) {x$n puts $p from $s cart into the vessels cargo hold.", counter );
						act( buf, ch, pObj, NULL, TO_CHAR );
					}
				}
	}
	else
	{
		send_to_char( "What do you mean?\n\r", ch );
		return;
	}

	/* Process CANNONs. Cannons should appear on the deck.*/
	/*set_correct_cannon_count_from_cargo( ch->in_room->ship );*/

	return;
}

void do_boat_chase( CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    SHIP_DATA *orig_ship;
    SHIP_DATA *ship;
    SHIP_DATA *attack;
    int x, y;

    argument = one_argument(argument, arg);

    if ( arg[0] == '\0' )
    {
	send_to_char("Which ship do you want to chase?\n\r", ch);
	return;
    }

    /* Is player on a boat? */
    if ( ch->in_room->ship == NULL )
    {
        send_to_char("You arn't on a vessel.\n\r", ch);
        return;
    }

    if ( ch->in_room->ship->owner != ch )
    {
	send_to_char("This isn't your vessel!\n\r", ch);
	return;
    }

	if ( !has_enough_crew( ch->in_room->ship ) ) {
		send_to_char( "There isn't enough crew to order that command!\n\r", ch );
		return;
	}

    orig_ship = ch->in_room->ship;

        x = get_squares_to_show_x(0);
        y = get_squares_to_show_y(0);

        attack = NULL;

            /* Check for sailing ship to attack*/
            for ( ship = ((AREA_DATA *) get_sailing_boat_area())->ship_list; ship != NULL; ship = ship->next)
                if (ship != orig_ship)
                if (
                  (ship->ship->in_room->x < orig_ship->ship->in_room->x + x &&

                     ship->ship->in_room->x > orig_ship->ship->in_room->x - x)
                 && (ship->ship->in_room->y < orig_ship->ship->in_room->y + y &&                    ship->ship->in_room->y > orig_ship->ship->in_room->y - y) &&
                     is_name(ship->ship_name, arg) &&
					 str_cmp(ship->owner_name, ch->name))
                 {
                      attack = ship;
                      break;
                 }

        if (attack == NULL)
        {
	    send_to_char("Theres no ship nearby matching that name.\n\r", ch);
            return;
        }

        orig_ship->destination = attack->ship->in_room;
        orig_ship->ship_chased = attack;
        if (ch->ship_move <= 0)
        {
            SHIP_STATE(ch, 16);
        }
        orig_ship->speed = SHIP_SPEED_FULL_SPEED;
        sprintf(buf, "%s gives the order to chase the ship, '%s'", ch->name, attack->ship_name);
	boat_echo(orig_ship, buf);
}

void do_board( CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    SHIP_DATA *ship;
    SHIP_DATA *orig_ship;
    CHAR_DATA *mob;

    one_argument(argument, arg);

    /* If not on ship then do do_enter */
    if ( ch->in_room->ship == NULL)
    {
        do_function(ch, &do_enter, argument );
		return;
    }

    if ( ch != ch->in_room->ship->owner)
    {
		send_to_char("You don't own this vessel!\n\r", ch);
		return;
    }

    for ( ship = ((AREA_DATA *) get_sailing_boat_area())->ship_list; ship !=NULL; ship = ship->next)
    {
		if ( is_name(ship->ship_name, arg) && ship->ship->in_room == ch->in_room->ship->ship->in_room && str_cmp( ship->owner_name, ch->name ))
        {
		    break;
        }
    }

    if ( ship == NULL )
    {
		send_to_char("There is no vessel by that name close enough to board.\n\r", ch);
		return;
    }

	if ( is_boat_safe( ch, ch->in_room->ship, ship ) )
	{
		return;
	}

    if ( ch->boarded_ship != NULL )
    {
		send_to_char("You are already boarding a vessel!\n\r", ch);
		return;
    }

    send_to_char("You give the order to board the enemy vessel!\n\r", ch);

    orig_ship = ch->in_room->ship;

	/* Make sure ship isn't chasing anything*/
	orig_ship->destination = NULL;
	orig_ship->ship_chased = NULL;
	orig_ship->speed = SHIP_SPEED_STOPPED;
    orig_ship->last_room[0] = NULL;
    orig_ship->last_room[1] = NULL;
    orig_ship->last_room[2] = NULL;

    /* Transfer mobs to boarded ship*/
    for (mob = orig_ship->crew_list; mob != NULL; mob = mob->next_in_crew)
    {
        char_from_room(mob);
        char_to_room(mob, get_room_index(ship->first_room));
        mob->boarded_ship = ship;
    }

    char_from_room(ch);
    char_to_room(ch, get_room_index(ship->first_room));
    send_to_char("{YYou jump across to the enemy vessel!{x\n\r", ch);

    ship->boarded_by = orig_ship;

	/* Transfer enemy captain into battle if they are on the ship*/
	if (ship->owner != NULL &&
		ship->owner->in_room != NULL &&
		ship->owner->in_room->ship == ship &&
		ship->owner->in_room != get_room_index(ship->first_room))
	{
		send_to_char("{YYou confront your aggressors!{x\n\r", ship->owner);
		char_from_room(ship->owner);
		char_to_room(ship->owner, get_room_index(ship->first_room));
	}

    /* All mobs on attacked ship must know they are being attacked*/
    for (mob = ship->crew_list; mob != NULL; mob = mob->next_in_crew)
    {
        /* if crew not at first room then come running in*/
        if (mob->in_room != get_room_index(ship->first_room))
        {
            char_from_room(mob);
            char_to_room(mob, get_room_index(ship->first_room));

            act("$n runs in from elsewhere on the vessel.", mob, NULL, NULL, TO_ROOM);
        }
        mob->boarded_ship = ship;
    }
	log_string("end of do_board");
}

void do_waypoint( CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char arg3[MAX_STRING_LENGTH];
    char arg4[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    SHIP_DATA *ship;
    WAYPOINT_DATA *waypoint;
    WAYPOINT_DATA *waypoint2;
    ROOM_INDEX_DATA *pRoom;
    AREA_DATA *pArea;
    long index;
    long dx, dy;
    int count;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);
    argument = one_argument(argument, arg4);


    if ( arg1[0] == '\0' )
    {
	send_to_char("Waypoint: list, add, delete, go, stop\n\r", ch);
 	return;
    }

    for ( ship = ((AREA_DATA *) get_sailing_boat_area())->ship_list; ship !=NULL; ship = ship->next)
    {
	if (ship->owner == ch)
        {
	    break;
        }
    }

    if (ship == NULL)
    {
	send_to_char("You don't own a vessel!\n\r", ch);
	return;
    }

    /* List waypoints */
    if ( !str_prefix(arg1, "list") )
    {
    send_to_char("{GWaypoints:{x\n\r", ch);

    count = 0;
    for (waypoint = ship->waypoint_list; waypoint != NULL; waypoint = waypoint->next)
    {
        sprintf(buf, "[%s%3d{x] x:%3d y:%3d\n\r", (ship->current_waypoint == waypoint ? "{G" : "{Y"), count, waypoint->x, waypoint->y);
        send_to_char(buf, ch);
        count++;
    }
    return;
    }

    if ( !str_prefix(arg1, "add") )
    {
        if (arg2[0] == '\0' || arg3[0] == '\0' || !is_number(arg2) || !is_number(arg3))
        {
            send_to_char("That is not valid.\n\r", ch);
            return;
        }

        dx = atoi(arg2);
        dy = atoi(arg3);

        pArea = ship->ship->in_room->area;
        index = (long)((long)dy * (long)pArea->map_size_x + dx + pArea->min_vnum + WILDERNESS_VNUM_OFFSET);

        if ((pRoom = get_room_index(index)) == NULL)
        {
            send_to_char("Couldn't find room.\n\r", ch);
            return ;
        }

        if (pRoom->sector_type != SECT_WATER_SWIM && pRoom->sector_type != SECT_WATER_NOSWIM)
        {
            send_to_char("Boat must be in the water.\n\r", ch);
            return ;
        }
        waypoint = new_waypoint();
        waypoint->x = dx;
        waypoint->y = dy;

    /* Add waypoint*/
    if ( !ship->waypoint_list )
    {
        ship->waypoint_list = waypoint;
    }
    else
    {
        WAYPOINT_DATA *temp;
        temp = ship->waypoint_list;
        while(temp->next != NULL)
        {
            temp = temp->next;
        }
        temp->next = waypoint;
        waypoint->next = NULL;
    }
    send_to_char("Waypoint added.\n\r", ch);
    return;
    }

    if ( !str_prefix(arg1, "delete") )
    {
    if ( !is_number(arg2) )
    {
        send_to_char("That is not a valid number.\n\r", ch);
        return;
    }

    count = 0;
    waypoint = NULL;
    waypoint2 = NULL;
    for (waypoint = ship->waypoint_list; waypoint != NULL; waypoint2 = waypoint, waypoint = waypoint->next)
    {
        if (atol(arg2) == count)
        {
            if (waypoint == ship->waypoint_list)
            {
                ship->waypoint_list = ship->waypoint_list->next;
            }
            else
            {
                waypoint2->next = waypoint->next;
                waypoint->next = NULL;
            }
        }
        count++;
    }
    if (waypoint != NULL)
    {
        free_waypoint(waypoint);
    }
    send_to_char("Waypoint removed from vessel.\n\r", ch);
    return;
    }
    if ( !str_prefix(arg1, "go") )
    {
    if ( !is_number(arg2) )
    {
        send_to_char("That is not a valid number.\n\r", ch);
        return;
    }

    if ( ship->destination != NULL )
    {
	send_to_char("Your vessel already has a destination!\n\r", ch);
	return;
    }

    count = 0;
    waypoint = NULL;
    ship->current_waypoint = NULL;
    for (waypoint = ship->waypoint_list; waypoint != NULL; waypoint2 = waypoint, waypoint = waypoint->next)
    {
        if (atol(arg2) == count)
        {
	    ship->current_waypoint = waypoint;
	    break;
        }
        count++;
    }

    if ( ship->current_waypoint == NULL)
    {
	send_to_char("That isn't a valid waypoint.\n\r", ch);
	return;
    }

    sprintf(buf, "Setting waypoint for %d, %d!\n\r", ship->current_waypoint->x, ship->current_waypoint->y);
    send_to_char(buf, ch);

    ship->destination = get_room_index((long)((long)ship->current_waypoint->y * (long)ship->ship->in_room->area->map_size_x + ship->current_waypoint->x + ship->ship->in_room->area->min_vnum + WILDERNESS_VNUM_OFFSET));

    if (ship->destination == NULL)
    {
	send_to_char("That waypoint destination is invalid! No room exists there!\n\r", ch);
	return;
    }

    ship->speed = SHIP_SPEED_FULL_SPEED;

    SHIP_STATE(ch, 16);
    send_to_char("Now progressing to waypoint.\n\r", ch);
    return;
    }

    if ( !str_prefix(arg1, "stop") )
    {
	if ( ship->current_waypoint == NULL )
        {
	    send_to_char("Your vessel isn't following a waypoint!\n\r", ch);
	    return;
        }

        send_to_char("{RCurrent waypoint cancelled!\n\r", ch);
        ship->current_waypoint = NULL;
        ship->destination = NULL;
        return;
    }

    send_to_char("Waypoint: list, add, delete, go, stop\n\r", ch);
    return;
}

bool is_boat_safe(CHAR_DATA *ch, SHIP_DATA *ship, SHIP_DATA *ship2)
{
	if ( ship2 == NULL )
	{
		if (   ship->ship->in_room == NULL ) {
			return FALSE;
		}

		if (   ship->ship->in_room->vnum == ROOM_VNUM_SEA_PLITH_HARBOUR ||
			   ship->ship->in_room->vnum == ROOM_VNUM_SEA_NORTHERN_HARBOUR ||
			   ship->ship->in_room->vnum == ROOM_VNUM_SEA_SOUTHERN_HARBOUR )
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	if ( ship->boarded_by != NULL && ship->boarded_by != ship2 )
	{
		return TRUE;
	}

	if ( ship->owner != NULL && IS_IMMORTAL( ship->owner ) &&
			ship->owner->level > LEVEL_IMMORTAL )
	{
		return FALSE;
	}

    if ( ( ship->ship->in_room->vnum == ROOM_VNUM_SEA_PLITH_HARBOUR ||
           ship->ship->in_room->vnum == ROOM_VNUM_SEA_NORTHERN_HARBOUR ||
           ship->ship->in_room->vnum == ROOM_VNUM_SEA_SOUTHERN_HARBOUR )
		 && ( ship2->ship->in_room->vnum == ROOM_VNUM_SEA_PLITH_HARBOUR  ||
              ship2->ship->in_room->vnum == ROOM_VNUM_SEA_NORTHERN_HARBOUR ||
              ship2->ship->in_room->vnum == ROOM_VNUM_SEA_SOUTHERN_HARBOUR ) )

	{
		send_to_char("Vessels in harbours can not be attacked.\n\r", ch);
		return TRUE;
	}

    /* first, is it a reckoning? */
    if ( pre_reckoning == 0 && reckoning_timer > 0 )
    {
        if ( ( ship->owner != NULL && ship->owner->tot_level <= 30 ) ||
				( ship2->owner != NULL && ship2->owner->tot_level <= 30 ) )
        {
			if ( ship->owner != NULL )
			{
	    		send_to_char("Only players level 31 and above are affected by 'The Reckoning'.\n\r",
						ship->owner );
			}
	    	return TRUE;
        }

		return FALSE;
    }

	/* NPC ships can fight people and people can fight NPC ships */
	if ( IS_NPC_SHIP( ship ) || IS_NPC_SHIP( ship2 ) )
	{
		return FALSE;
	}

	if ( ch != NULL && !IS_PK(ch) )
	{
		send_to_char("You must be PK to attack another vessel.\n\r", ch);
		return TRUE;
	}

/*
	if ( ship->pk == FALSE )
	{
		if ( ship->owner != NULL )
		{
 			send_to_char("The vessel is not PK.\n\r", ship->owner );
		}
    	return TRUE;
	}

	if ( ship2->pk == FALSE )
	{
		if ( ship->owner != NULL )
		{
 			send_to_char("The vessel is not PK.\n\r", ship->owner );
		}
    	return TRUE;
	}
*/

    return FALSE;
}

/* used for npcs to judge which reputation they should be*/
sh_int get_rating( int ships_destroyed )
{
	if ( ships_destroyed < 5 )
	{
		return NPC_SHIP_RATING_UNKNOWN;
	}
	else
	if ( ships_destroyed <  15)
	{
		return NPC_SHIP_RATING_WELLKNOWN;
	}
	else
	if ( ships_destroyed < 25 )
	{
		return NPC_SHIP_RATING_FAMOUS;
	}
	else
	if ( ships_destroyed < 50 )
	{
		return NPC_SHIP_RATING_NOTORIOUS;
	}
	else
		return NPC_SHIP_RATING_INFAMOUS;
}

void clear_waypoints( SHIP_DATA *ship )
{
    WAYPOINT_DATA *waypoint = NULL;

    /* extract waypoints*/
    while( ship->waypoint_list != NULL )
    {
    	waypoint = ship->waypoint_list;
	ship->waypoint_list = waypoint->next;
	free_waypoint( waypoint );
    }
}

void add_move_waypoint( SHIP_DATA *ship, int x, int y )
{
    WAYPOINT_DATA *waypoint;

    waypoint = new_waypoint();
    waypoint->x = x;
	  waypoint->y = y;

    /* Add waypoint*/
    if ( !ship->waypoint_list )
    {
		ship->waypoint_list = waypoint;
    }
    else
    {
 		WAYPOINT_DATA *temp;
		temp = ship->waypoint_list;
		while(temp->next != NULL)
    {
	    temp = temp->next;
    }
		temp->next = waypoint;
    }
}

/*
 * Transfers all the cargo from one ship to another.
 */
void transfer_cargo( SHIP_DATA *source, SHIP_DATA *destination )
{
	OBJ_DATA *pObj = NULL;

	while( ( pObj = source->ship->contains ) != NULL )
	{
		obj_from_obj( pObj );
		obj_to_obj( pObj, destination->ship );

		if (IS_NPC_SHIP(destination))
		{
			destination->npc_ship->pShipData->plunder_captured++;
		}
	}

	/* Process CANNONs. Cannons should appear on the deck.*/
	/*set_correct_cannon_count_from_cargo( destination );*/
}

/**
 * Group commands for ships.
 */
void do_ship( CHAR_DATA *ch, char *argument )
{
	char arg[ MAX_INPUT_LENGTH ];

	argument = one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_char( "Type {YHelp Ship {xfor ship commands.\n\r", ch );
		return;
	}

	if ( !str_cmp( arg, "sail" ) )
	{
		do_function(ch, &do_steer, argument );
	}
	else
	if ( !str_cmp( arg, "steer" ) )
	{
		do_function(ch, &do_steer, argument );
	}
	else
	if ( !str_cmp( arg, "scuttle" ) )
	{
		do_function(ch, &do_scuttle, argument );
	}
	else
	if ( !str_cmp( arg, "speed" ) )
	{
		do_function(ch, &do_speed, argument );
	}
	else
	if ( !str_cmp( arg, "chase" ) )
	{
		do_function(ch, &do_boat_chase, argument );
	}
	else
	if ( !str_cmp( arg, "board" ) )
	{
		do_function(ch, &do_board, argument );
	}
	else
	if ( !str_cmp( arg, "aim" ) )
	{
		do_function(ch, &do_aim, argument );
	}
	else
	if ( !str_cmp( arg, "crew" ) )
	{
		do_function(ch, &do_crew, argument );
	}
	else
	if ( !str_cmp( arg, "shiplist" ) )
	{
		do_function(ch, &do_ship_list, argument );
	}
	else
	if ( !str_cmp( arg, "damage" ) )
	{
		do_function(ch, &do_damage, argument );
	}
	else
	if ( !str_cmp( arg, "cargo" ) )
	{
		do_function(ch, &do_cargo, argument );
	}
	else
	if ( !str_cmp( arg, "flag" ) )
	{
		do_function(ch, &do_ship_flag, argument );
	}
	else
	if ( !str_cmp( arg, "npcstatus" ) )
	{
		do_function(ch, &do_npcstatus, argument );
	}
	else
	{
		send_to_char( "Type {YHelp Ship {xfor ship commands.\n\r", ch );
	}
	return;
}

void do_npcstatus( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	SHIP_DATA *ship;
	long vnum = 0;

	argument = one_argument( argument, arg );

	if (arg[0] == '\0' || !is_number(arg))
	{
		send_to_char("Which vnum do you want to inspect?\n\rUsage: ship npcstatus vnum\n\r", ch);
		return;
	}

	vnum = atol(arg);


    for ( ship = ((AREA_DATA *) get_sailing_boat_area())->ship_list; ship !=NULL; ship = ship->next)
    {
		if ( IS_NPC_SHIP( ship ) && ship->npc_ship->captain != NULL && vnum == ship->npc_ship->pShipData->vnum )
		{
			break;
		}
	}

	if (ship == NULL)
	{
		send_to_char("That vnum could not be found.\n\r", ch);
		return;
	}

	sprintf( buf, "{YNPC Ship Status Information\n\r"
				  "-------------------------------{x\n\r"
				  "VNum          %ld\n\r"
				  "Captain Name  %s\n\r"
		          "Type          %s\n\r"
			      "Ship Type     %s\n\r"
			      "Waypoint Type %s\n\r"
				  "Waypoint Dest %d, %d\n\r"
			      "Kills         %d\n\r"
				  "Plunder Capt  %ld\n\r"
				  "State         %s\n\r"
				  "Rank          %s\n\r"
                  "Room          %ld\n\r",
					ship->npc_ship->pShipData->vnum,
					ship->npc_ship->captain->short_descr,
					boat_table[ ship->npc_ship->pShipData->ship_type ].name,
					npc_boat_table[ ship->npc_ship->pShipData->npc_type ].name,
					( ship->current_waypoint != NULL ? ( ( ship->current_waypoint->x == 0 &&
														 ship->current_waypoint->y == 0 ) ?
														 "{YWaiting{x" : "{GTravelling{x" ) : "{RNone{x" ),
					( ship->current_waypoint != NULL ? ship->current_waypoint->x : 0),
					( ship->current_waypoint != NULL ? ship->current_waypoint->y : 0),
					ship->npc_ship->pShipData->ships_destroyed,
					ship->npc_ship->pShipData->plunder_captured,
					ship_state_table[ship->npc_ship->state].name,
					rating_table[ get_rating( ship->npc_ship->pShipData->ships_destroyed ) ].name,
					ship->ship->in_room->vnum
					);
			send_to_char( buf, ch );
}

void do_ship_list( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	SHIP_DATA *ship;

  if (!IS_IMMORTAL(ch)) {
    send_to_char( "Huh?\n\r", ch);
    return;
  }

	send_to_char( "NPC ships in game:\n\r", ch );

	send_to_char( "{YVNum Captain Name                     Type              Ship Type  Waypoint Type  Waypoint Destination  Kills State   Room{x\n\r", ch );
    for ( ship = ((AREA_DATA *) get_sailing_boat_area())->ship_list; ship !=NULL; ship = ship->next)
    {
		if ( IS_NPC_SHIP( ship ) && ship->npc_ship->captain != NULL )
		{
			sprintf( buf, "%-4ld %-32s %-17s %-10s %-18s %-8d %-12d %-5d %-5s %-5ld\n\r",
					ship->npc_ship->pShipData->vnum,
					ship->npc_ship->captain->short_descr,
					boat_table[ ship->npc_ship->pShipData->ship_type ].name,
					npc_boat_table[ ship->npc_ship->pShipData->npc_type ].name,
					( ship->current_waypoint != NULL ? ( ( ship->current_waypoint->x == 0 &&
														 ship->current_waypoint->y == 0 ) ?
														 "{YWaiting{x" : "{GTravelling{x" ) : "{RNone{x" ),
					( ship->current_waypoint != NULL ? ship->current_waypoint->x : 0),
					( ship->current_waypoint != NULL ? ship->current_waypoint->y : 0),
					ship->npc_ship->pShipData->ships_destroyed,
					ship_state_table[ship->npc_ship->state].name,
					ship->ship->in_room->vnum
					);
			send_to_char( buf, ch );
		}
	}


	send_to_char( "\n\r\n\rPlayer ships in game:\n\r", ch );
	send_to_char( "{YOwner Name                Type{x\n\r", ch );
    for ( ship = ((AREA_DATA *) get_sailing_boat_area())->ship_list; ship !=NULL; ship = ship->next)
    {
		if ( !IS_NPC_SHIP( ship ) )
		{
			sprintf( buf, "%-25s %-8s\n\r",
					ship->owner_name,
					boat_table[ ship->ship_type ].name
					);
			send_to_char( buf, ch );
		}
	}
}

/*
 * Does the ship have enough crew to perform any actions?
 */
bool has_enough_crew( SHIP_DATA *ship )
{
	CHAR_DATA *person = NULL;
	int counter = 0;
	bool result = FALSE;

	for (person = ship->crew_list; person != NULL; person = person->next_in_crew)
	{
	    counter++;
	}

	if ( counter >= ship->min_crew ) {
		result = TRUE;
	}

	return result;
}

/*
 * Make someone a pirate.
 * Region is Seralia, Olaria or Achaeus.
 * If ship is set then work out the bounty from the ship.
void set_pirate_status( CHAR_DATA *ch, int region, int bounty )
{
	char buf[MAX_STRING_LENGTH];

	if ( ch->pcdata->reputation[ region ] == NPC_SHIP_RATING_WANTED )
	{
		sprintf(buf, "{R%s now regards you as a Pirate!!!{x\n\r", (region == CONT_SERALIA ? "Seralia" : "Athemia") );
		send_to_char( buf, ch );
	}
	else
	{
		ch->pcdata->rank[ region ] = NPC_SHIP_RANK_PIRATE;
    ch->pcdata->ship_quest_points[ region ] = 0;
	}

	 If particular kinds of NPC, then much worse*/
	ch->pcdata->bounty[ region ] += bounty;
}

CHAR_DATA *get_captain( SHIP_DATA *ship )
{
	if ( IS_NPC_SHIP( ship ) )
	{
		return ship->npc_ship->captain;
	}
	else
	{
		return ship->owner;
	}
}

void do_pardon( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *mob;
    char arg[MAX_INPUT_LENGTH];
	short region_type;

	if ( IS_NPC(ch) )
	{
		return;
	}

	mob = NULL;

    for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room)
    {
		if ( IS_SET( mob->act2, ACT2_SHIP_QUESTMASTER) && IS_NPC(mob) )
		{
			break;
		}
    }

	if ( mob == NULL )
	{
		send_to_char("No one can pardon you here.\n\r", ch);
		return;
	}

	if ( IS_SET(ch->in_room->area->place_flags, PLACE_FIRST_CONTINENT))
	{
		region_type = CONT_SERALIA;
	}
	else
	{
		region_type = CONT_ATHEMIA;
	}

    if ( argument[0] == '\0' )
    {
		if ( IS_PIRATE_IN_AREA(ch) )
		{
	    	sprintf( buf, "%s, you are lucky to have made it this far! You may be pardoned for %ld gold.", pers( ch, mob ), ch->pcdata->bounty[region_type]);
			do_say( mob, buf );
		}
		else
		{
	    	sprintf( buf, "%s, you need no pardon, you havn't done anything wrong!", pers( ch, mob ) );
	   		do_say( mob, buf );
		}
		return;
	}

	argument = one_argument( argument, arg );

	if ( !str_cmp( arg, "pay" ) )
	{
    long cost = 0;

    cost = ch->pcdata->bounty[region_type];
		if ( ch->silver + (100*ch->gold) < cost*100 )
		{
			sprintf( buf, "%s, don't fool with me. The price is %ld gold coins.",
					pers(ch, mob), cost );
			do_say( mob, buf );
			return;
		}
		else
		{
  		deduct_cost( ch, cost );

		  sprintf( buf, "$N takes {Y%ld{x gold coins.", cost );
		  act( buf, ch, NULL, mob, TO_CHAR );

			ch->pcdata->bounty[region_type] = 0;
			ch->pcdata->rank[region_type] = NPC_SHIP_RANK_NONE;

			sprintf( buf, "%s, you are now pardoned of your crimes.", pers(ch, mob) );
			do_say( mob, buf );
			return;
		}
	}
}

void create_and_add_cannons( SHIP_DATA *ship, int number )
{
	OBJ_DATA *pCannons;
	int i;

	for ( i = 0; i < number; i++ )
	{
		pCannons = create_object( get_obj_index( OBJ_VNUM_TRADE_CANNON ), 0, FALSE);
		obj_to_obj( pCannons, ship->ship);
	}

	/* Process CANNONs. Cannons should appear on the deck.*/
	set_correct_cannon_count_from_cargo( ship );
}

void set_correct_cannon_count_from_cargo( SHIP_DATA *ship )
{
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *pObj = NULL;
	int counter = 0;

	/* Process CANNONs. Cannons should appear on the deck.*/
	pObj = ship->ship->contains;

	while ( pObj != NULL )
	{
		if ( pObj->value[0] == TRADE_CANNONS )
		{
			counter++;
		}
		pObj = pObj->next_content;
	}

	if ( counter > 0 )
	{
		extract_obj( ship->cannons_obj );

		ship->cannons = counter;

		pObj = create_object( get_obj_index( OBJ_VNUM_SAILING_BOAT_CANNON), 0, FALSE);
		sprintf(buf, pObj->description, ship->cannons);
		free_string(pObj->description);
		pObj->description = str_dup(buf);
		obj_to_room( pObj, ship->ship_rooms[0]);

		ship->cannons_obj = pObj;
	}
}

void award_ship_quest_points(int area_type, CHAR_DATA *ch, int points) {
  char buf[MSL];
	bool promotion = FALSE;

					ch->pcdata->ship_quest_points[area_type] = ch->pcdata->ship_quest_points[area_type] + points;

          /* Check if a promotion is due*/
          if (rank_table[ch->pcdata->rank[area_type]+1].qp <= ch->pcdata->ship_quest_points[area_type]) {
            promotion = TRUE;
          }

		if (promotion) {
            /* increase rank*/
            ch->pcdata->rank[area_type]++;

      sprintf(buf, "All congratulate %s who has attained the rank of %s for %s!",
				ch->name,
				rank_table[ ch->pcdata->rank[ area_type ] ].name,
        area_type == IN_SERALIA(ch->in_room->area) ? "Seralia" : (IN_ATHEMIA(ch->in_room->area) ? "Athemia" : "the pirates"));
      crier_announce(buf);

      sprintf(buf, "You have been promoted to %s!\n\r", rank_table[ ch->pcdata->rank[ area_type ] ].name);
	    send_to_char(buf, ch);
    }
}

void award_reputation_points(int area_type, CHAR_DATA *ch, int points) {
  char buf[MSL];
	bool promotion = FALSE;
  int reputation = 0;

    reputation = get_player_reputation(ch->pcdata->reputation[area_type]);
		ch->pcdata->reputation[area_type] = ch->pcdata->reputation[area_type] + points;

    /* Check if a promotion is due*/
    if (reputation > get_player_reputation(ch->pcdata->reputation[area_type])) {
        promotion = TRUE;
    }

		if (promotion) {

      sprintf(buf, "Your reputation is now {W%s{x in {Y%s{x!",
				rating_table[ get_player_reputation(ch->pcdata->reputation[ area_type ]) ].name,
        IN_SERALIA(ch->in_room->area) ? "Seralia" : (IN_ATHEMIA(ch->in_room->area) ? "Athemia" : "the pirates"));
	    send_to_char(buf, ch);
    }
}
 */

#endif
