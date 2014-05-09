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

// Puts some treasure in the wilds, buries it and creates a map
OBJ_DATA* create_treasure_map() {
  long vnum;
  int i;
  ROOM_INDEX_DATA *pRoom = NULL;
  char *map;
  OBJ_DATA *scroll;
  OBJ_DATA *treasure;
  char buf[MSL];
  AREA_DATA *pArea;
  AREA_DATA *closestArea;
  int distance;
  AREA_DATA *bestArea = NULL;
  int bestDistance = 1000;

  // Wilderness
  pArea = find_area("Wilderness");


  // find treasure
  i = number_range(0, MAX_TREASURES-1);

  // create object
  treasure = create_object(get_obj_index(treasure_table[i]), 
		 0, TRUE);

  // Find location for map 
	while(TRUE) {
    vnum = number_range(pArea->min_vnum, pArea->max_vnum);

	    if ( ( pRoom = get_room_index(vnum) ) != NULL) 
	    {
        if (pRoom->sector_type != SECT_WATER_SWIM && pRoom->sector_type != SECT_WATER_NOSWIM)
        {
            obj_to_room(treasure, pRoom);
            break;
        }
      }
   }

   // Get closest area
    for (closestArea = area_first; closestArea != NULL; closestArea = closestArea->next) {

		 distance = (int) sqrt( 					\
							( closestArea->x - pRoom->x ) *	\
							( closestArea->x - pRoom->x ) +	\
							( closestArea->y - pRoom->y ) *	\
							( closestArea->y - pRoom->y ) );

     if (distance < bestDistance) {
       bestDistance = distance; 
       bestArea = closestArea;
     }
   }

  // Get map
  map = get_wilderness_map(pArea, pRoom->x, pRoom->y, 0, 0);

    // create the scroll
    scroll = create_object(get_obj_index(OBJ_VNUM_TREASURE_MAP), 0, TRUE); 
    if (scroll == NULL)
	return NULL;
    if (scroll->full_description != NULL)
	free_string(scroll->full_description);

    sprintf(buf,  
      "%s\n\r"
      "X marks the spot! The location be near %s.",
	    map,
      bestArea->name);

    scroll->full_description = str_dup(buf);

  return scroll;
}
