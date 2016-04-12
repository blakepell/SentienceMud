/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1998 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@hypercube.org)				   *
*	    Gabrielle Taylor (gtaylor@hypercube.org)			   *
*	    Brian Moore (zump@rom.org)					   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "wilds.h"

char *const distance[7] =
{
    "right here.",
    "one step to the %s.",
    "nearby to the %s.",
    "not far %s.",
    "quite far %s.",
    "barely visible to the %s.",
    "off in the distance %s."
};


/* local functions */
void scan_list(ROOM_INDEX_DATA *scan_room, CHAR_DATA *ch, int depth, int door);
void scan_char(CHAR_DATA *victim, CHAR_DATA *ch, int depth, int door );

void scan_direction(CHAR_DATA *ch, ROOM_INDEX_DATA *start_room, int max_depth, int door)
{
	int depth;
	int to_x, to_y;
	DESTINATION_DATA dest;
	EXIT_DATA *pExit;
	WILDS_TERRAIN *pTerrain;
	WILDS_VLINK *pVLink = NULL;

	// Initialize current destination data to the start room
	dest.room = start_room;
	dest.wilds = NULL;
	dest.wx = 0;
	dest.wy = 0;

	for( depth = 1; depth < max_depth; depth++) {

		if( pVLink != NULL ) {
			// TODO: VLINKS need FROM-WILD side exit flags

			// Hidden exits that haven't been found
			// Closed exits

			// No room there actually!
			if( !pVLink->pDestRoom ) {
				dest.room = get_room_index(pVLink->destvnum);

				if(!dest.room)
					return;

				if(can_see_room (ch, dest.room))
					scan_list(dest.room, ch, depth, door);

			} else
				dest.room = pVLink->pDestRoom;

			pVLink = NULL;

		} else if( dest.room ) {
			// We have an actual room (static, clone or existing wilds)
			if ((pExit = dest.room->exit[door])) {
				// Hidden exits that haven't been found
				if(IS_SET(pExit->exit_info,EX_HIDDEN) && !IS_SET(pExit->exit_info,EX_FOUND))
					return;

				// Closed exits
				if(IS_SET(pExit->exit_info, EX_CLOSED))
					return;

				if(!exit_destination_data(pExit, &dest))
					return;

				// We have an actual room
				if(dest.room) {

					if(can_see_room (ch, dest.room))
						scan_list(dest.room, ch, depth, door);
				}
			}
		} else if(dest.wilds) {
			// We have a wilds location, the room has not been loaded

			pVLink = vroom_get_to_vlink(dest.wilds, dest.wx, dest.wy, door);
			if( pVLink != NULL ) {
				continue;

			} else {
				to_x = get_wilds_vroom_x_by_dir(dest.wilds, dest.wx, dest.wy, door);
				to_y = get_wilds_vroom_y_by_dir(dest.wilds, dest.wx, dest.wy, door);

				// Nothing here to reach, so stop
				if( !check_for_bad_room(dest.wilds, to_x, to_y) )
					return;

				dest.room = get_wilds_vroom(dest.wilds, to_x, to_y);
				dest.wx = to_x;
				dest.wy = to_y;

				if(dest.room) {

					if(can_see_room (ch, dest.room))
						scan_list(dest.room, ch, depth, door);
				}

			}
		} else
			return;

	}
}


void do_scan(CHAR_DATA *ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *scan_room, *dest;
	EXIT_DATA *pExit;
	int door;
	int depth;
	int max_depth;
	int skill;

	argument = one_argument(argument, arg1);

	skill = get_skill(ch,gsn_scan);

	if (skill < 1)		max_depth = 4;
	else if (skill < 85)	max_depth = 5;
	else if (skill < 100)	max_depth = 6;
	else			max_depth = 7;

	if (!arg1[0]) {
		act("$n looks all around.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
		send_to_char("{YLooking around, you see:{x\n\r", ch);
		scan_list(ch->in_room, ch, 0, -1);

		for (door = 0; door < MAX_DIR; door++ ) {
			scan_direction(ch, ch->in_room, max_depth, door);
			/*
			scan_room = ch->in_room;

			for (depth = 1; depth < max_depth; depth++) {
				if ((pExit = scan_room->exit[door])) {
					// Hidden exits that haven't been found
					if(IS_SET(pExit->exit_info,EX_HIDDEN) && !IS_SET(pExit->exit_info,EX_FOUND))
						continue;

					// Closed exits
					if(IS_SET(pExit->exit_info, EX_CLOSED))
						continue;

					if(!(dest = exit_destination(pExit)))
						continue;

					if(!can_see_room (ch, dest))
						continue;

					scan_room = dest;
					scan_list(dest, ch, depth, door);
				}
			}
			*/
		}

		return;
	}

	door = find_door(NULL, arg1, FALSE);
	if(door < 0) {
		send_to_char("Which way do you want to scan?\n\r", ch);
		return;
	}

	act("{YLooking $T, you see:{x", ch, NULL, NULL, NULL, NULL, NULL, dir_name[door], TO_CHAR);
	act("$n peers intently $T.", ch, NULL, NULL, NULL, NULL, NULL, dir_name[door], TO_ROOM);

	scan_direction(ch, ch->in_room, max_depth, door);

	/*
	scan_room = ch->in_room;
	for (depth = 1; depth < max_depth; depth++) {
		if ((pExit = scan_room->exit[door])) {
			// Hidden exits that haven't been found
			if(IS_SET(pExit->exit_info,EX_HIDDEN) && !IS_SET(pExit->exit_info,EX_FOUND))
				continue;

			// Closed exits
			if(IS_SET(pExit->exit_info, EX_CLOSED))
				continue;

			if(!(dest = exit_destination(pExit)))
				continue;

			if(!can_see_room (ch, dest))
				continue;

			scan_room = dest;
			scan_list(dest, ch, depth, door);
		}

	}
	*/

	if (skill > 0)
		check_improve( ch, gsn_scan, TRUE, 1 );
}


void scan_list(ROOM_INDEX_DATA *scan_room, CHAR_DATA *ch, int depth, int door )
{
   CHAR_DATA *rch;

   if ( scan_room == NULL )
       return;

   for (rch = scan_room->people; rch != NULL; rch=rch->next_in_room)
   {
       if (rch == ch)
           continue;

       if (!IS_NPC(rch) && rch->invis_level > get_trust(ch))
	   continue;

       if (can_see(ch, rch) && rch->position != POS_FEIGN)
	   scan_char( rch, ch, depth, door );
   }
}


void scan_char(CHAR_DATA *victim, CHAR_DATA *ch, int depth, int door)
{
    char buf[MAX_INPUT_LENGTH];
    char buf2[MAX_INPUT_LENGTH];

    buf[0] = '\0';

    strcat(buf, pers(victim, ch));
    buf[0] = UPPER(buf[0]);
    strcat(buf, ", ");
    sprintf(buf2, distance[depth], dir_name[door]);
    strcat(buf, buf2);
    strcat(buf, "\n\r");

    send_to_char(buf, ch);
}
