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
*	    Gabrielle Taylor (gtaylor@efn.org)				   *
*	    Brian Moore (zump@rom.org)					   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *    Scripting engine rebuilt by Michael Kurtz (Nibelung)                 *
 *    Used with permission.                                                *
 *                                                                         *
 **************************************************************************/

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "interp.h"
#include "wilds.h"


void do_disembark( CHAR_DATA *ch, char *argument)
{
    ROOM_INDEX_DATA *location;
    OBJ_DATA *ship_obj;

    if ( ch->fighting != NULL )
    {
	act("You can't disembark while fighting.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
	return;
    }

    if ( ch->in_room->area->ship_list == NULL )
    {
	act("You arn't on a vessel.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
	return;
    }

    if ( ch->in_room->ship->ship_type == SHIP_AIR_SHIP &&
    ch->in_room->ship->speed != SHIP_SPEED_STOPPED )
    {
	act( "The doors of the airship are locked!", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR );
	return;
    }


    location = ch->in_room->ship->ship->in_room;
    ship_obj = ch->in_room->ship->ship;

    char_from_room(ch);
    char_to_room(ch, location);

    act("{WYou disembark from $p.{x", ch, NULL, NULL, ship_obj, NULL, NULL, NULL, TO_CHAR);
    act("{W$n disembarks from $p.{x", ch, NULL, NULL, ship_obj, NULL, NULL, NULL, TO_ROOM);

/*
    if ( IS_SET( ch->in_room->room_flags, ROOM_SHIP))
	act("{W$n disembarks from the.{x", ch,
*/

	move_cart(ch,location,TRUE);
}


void do_enter( CHAR_DATA *ch, char *argument)
{
    ROOM_INDEX_DATA *location;

    if ( ch->fighting != NULL )
	return;

    if (argument[0] != '\0')
    {
	ROOM_INDEX_DATA *old_room;
	OBJ_DATA *portal;
	CHAR_DATA *fch, *fch_next;

	old_room = ch->in_room;

	portal = get_obj_list( ch, argument,  ch->in_room->contents );

	if (portal == NULL)
	{
	    int i;
	    ROOM_INDEX_DATA *r2;
	    for ( i = 0; i < MAX_DIR; i++ ) {
		if ( ch->in_room->exit[ i ] != NULL && (r2 = ch->in_room->exit[ i ]->u1.to_room)) {
		    portal = get_obj_list( ch, argument,  r2->contents );
		    if ( portal != NULL && portal->item_type == ITEM_SHIP ) {
			break;
		    }
		}
	    }

	    if ( portal == NULL ) {
		send_to_char("You don't see that here.\n\r",ch);
		return;
	    }
	}

/* - Temporary allowance of relics through portals, until ships are fixed. -- Areo
if (PULLING_CART(ch) && portal->item_type != ITEM_SHIP)
	{
	    send_to_char("You must drop what you are currently pulling.\n\r", ch);
	    return;
	}
*/

	if (portal->item_type == ITEM_SHIP)
	{
	    if (MOUNTED(ch))
	    {
		act("You can't board this vessel while mounted.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
		return;
	    }

	    if (portal->ship->speed != SHIP_SPEED_STOPPED)
	    {
		act("You can't board a moving vessel.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
		return;
	    }

	    if ( portal->value[5] != -1 && (location = get_room_index(portal->value[5])))
	    {
		/* CHAR_DATA *pMob; */
		act("{WYou board $p.{x\n\r", ch, NULL, NULL, portal, NULL, NULL, NULL, TO_CHAR);
		act("{W$n boards $p.{x\n\r", ch, NULL, NULL, portal, NULL, NULL, NULL, TO_ROOM);

		move_cart(ch,location,TRUE);

		char_from_room(ch);
		char_to_room(ch, location);

		act("{W$n boards $p.{x", ch, NULL, NULL, portal, NULL, NULL, NULL, TO_ROOM);

                /* For now this makes airship captain kill people.
		   disable it for now as i dont know how this thing works
		if ( !IS_NPC(ch)
		&& portal->ship->owner != ch
		&& portal->ship->crew_list != NULL)
		{
		    boat_echo(portal->ship, "{YThe ship crew charge into combat!{x");

		     if boarding other persons boat then everyone wants to kill the person
		    for (pMob = portal->ship->crew_list; pMob != NULL;
		         pMob = pMob->next_in_crew)
		    {
			if ( pMob->fighting == NULL )
			{
			    char_from_room(pMob);
			    char_to_room(pMob, get_room_index(portal->ship->first_room));

				p_percent_trigger( ch,NULL, NULL, NULL, pMob, NULL, NULL, NULL, NULL,TRIG_BOARD , NULL);

			    set_fighting(pMob, ch);
			}
		    }
		}
		*/

		return;
	    }
	}

	if (portal->item_type != ITEM_PORTAL
		|| IS_SET(portal->value[1],EX_CLOSED))
	{
	    send_to_char("You can't seem to find a way in.\n\r",ch);
	    return;
	}

 	/* @@@NIB : 20070126 : Changed the polarity of nocurse
 		It had a NOT.  But that's backwards to the name of
 		the flag.*/
 	if (IS_SET(portal->value[2],GATE_NOCURSE)
  		&&  (IS_AFFECTED(ch,AFF_CURSE)))
	    /*
	       ||   IS_SET(old_room->room_flags,ROOM_NO_RECALL)))
	     */
	{
	    send_to_char("Something prevents you from leaving...\n\r",ch);
	    return;
	}

	if (IS_SET(portal->value[1],EX_ENVIRONMENT) && old_room && old_room->source) {
		location = get_environment(old_room);
	} else if (IS_SET(portal->value[2],GATE_RANDOM) || portal->value[4] == -1 || (IS_SET(portal->value[2],GATE_BUGGY) && (number_percent() < 5)))
		location = get_random_room( ch, 0 );
	else if (IS_SET(portal->value[2],GATE_AREARANDOM) || portal->value[3] == -1) {
		ROOM_INDEX_DATA *here;

		here = obj_room(portal);

		if(here) {
			if(here->wilds) {
				int x,y;
				x = number_range(0,here->wilds->map_size_x-1);
				y = number_range(0,here->wilds->map_size_y-1);
				location = get_wilds_vroom(here->wilds,x,y);
				if(!location)
					location = create_wilds_vroom(here->wilds,x,y);
			} else
				location = get_random_room_area(ch, here->area);
		} else
			location = get_random_room_area(ch, find_area("Plith"));
	} else if (portal->value[5] > 0) {
		WILDS_DATA *wilds = get_wilds_from_uid(NULL,portal->value[5]);
		location = get_wilds_vroom(wilds,portal->value[6],portal->value[7]);
		if(!location)
			location = create_wilds_vroom(wilds,portal->value[6],portal->value[7]);
	}
	else
	{
		location = get_room_index(portal->value[3]);
		// Check if this portal points to a clone room, if so, find it
		if( location != NULL && (portal->value[6] > 0 || portal->value[7] > 0))
			location = get_clone_room(location, (unsigned long)portal->value[6], (unsigned long)portal->value[7]);
	}

  	if (!location || location == old_room || !can_see_room(ch,location) ||
  		(!IS_SET(portal->value[2],GATE_NOPRIVACY) && room_is_private(location, ch))) {
	    act("$p doesn't seem to go anywhere.",ch, NULL, NULL,portal,NULL, NULL, NULL,TO_CHAR);
	    return;
	}

	if(p_percent_trigger(NULL, NULL, location, NULL, ch, NULL, NULL,portal, NULL,TRIG_PREENTER, "portal"))
		return;

	portal->value[3] = location->vnum;
	portal->value[4] = location->area->uid;
	portal->value[5] = location->wilds ? location->wilds->uid : 0;
	portal->value[6] = location->wilds ? location->x : 0;
	portal->value[7] = location->wilds ? location->y : 0;

 	/* @@@NIB : 20070126 : added the check */
 	if(!IS_SET(portal->value[2],GATE_SILENTENTRY))
  		act("$n steps into $p.",ch, NULL, NULL,portal, NULL, NULL,NULL,TO_ROOM);

	if (IS_SET(portal->value[2],GATE_NORMAL_EXIT))
	    act("{YYou enter $p.{x",ch, NULL, NULL,portal, NULL, NULL,NULL,TO_CHAR);
	else if (!IS_SET(portal->value[2], GATE_RANDOM))
	    act("{YYou walk through $p and find yourself in $T.{x",
		    ch, NULL, NULL,portal, NULL, NULL,location->name,TO_CHAR);
	else
	    act("{YYou walk through $p and find yourself somewhere else...{x",
		    ch, NULL, NULL,portal, NULL, NULL,location->name,TO_CHAR);

	move_cart(ch,location,TRUE);

	char_from_room(ch);
	char_to_room(ch, location);

        /* Let portals cast spells */
	{
		SPELL_DATA *spell;

		for (spell = portal->spells; spell; spell = spell->next)
			obj_cast_spell(spell->sn, spell->level, ch, ch, NULL);
	}

	if (IS_SET(portal->value[2],GATE_GOWITH)) /* take the gate along */
	{
	    obj_from_room(portal);
	    obj_to_room(portal,location);
	}

	/* @@@NIB : 20070127 : strip off if portal is nosneak
			Right now, it does not mix with "sneak". */
	if(IS_SET(portal->value[2],GATE_NOSNEAK)) {
		affect_strip(ch, gsn_sneak);
		REMOVE_BIT(ch->affected_by, AFF_SNEAK);

	/* @@@NIB : 20070127 : if portal is sneak, attempt autosneak IF they can do it!
			Maybe in the future if permitted, this can do it regardless of
			whether they can do it or not.  For now, normal rules for "sneak"
			apply.  If they are already sneaking, that's a different story.
			Improvement is not done here and if you fail, it doesn't say
			anything. */
	} else if(IS_SET(portal->value[2],GATE_SNEAK)) {
		if(!MOUNTED(ch) && !ch->fighting && !IS_AFFECTED(ch,AFF_SNEAK) &&
			(number_percent() < get_skill(ch,gsn_sneak))) {
			AFFECT_DATA af;
			memset(&af,0,sizeof(af));
			af.where     = TO_AFFECTS;
			af.group     = AFFGROUP_PHYSICAL;
			af.type      = gsn_sneak;
			af.level     = ch->level;
			af.duration  = ch->level;
			af.location  = APPLY_NONE;
 			af.modifier  = 0;
			af.bitvector = AFF_SNEAK;
			af.bitvector2 = 0;
			af.slot	= WEAR_NONE;
			affect_to_char(ch, &af);
			send_to_char("You assume a sneaking posture.\n\r", ch);
		}
	}

	/* @@@NIB : 20070126 : added the check */
	if(!IS_SET(portal->value[2],GATE_SILENTEXIT)) {
  		if (IS_SET(portal->value[2],GATE_NORMAL_EXIT))
  		    act("$n has arrived.",ch, NULL, NULL,portal, NULL, NULL,NULL,TO_ROOM);
  		else
  		    act("$n has arrived through $p.",ch, NULL, NULL,portal, NULL, NULL,NULL,TO_ROOM);
	}

	do_function(ch, &do_look, "auto");

	/* charges */
	if (portal->value[0] > 0)
	{
	    portal->value[0]--;
	    if (portal->value[0] == 0)
		portal->value[0] = -1;
	}

	/* protect against circular follows */
	if (old_room == location)
	    return;

	for ( fch = old_room->people; fch != NULL; fch = fch_next )
	{
	    fch_next = fch->next_in_room;

	    if (portal == NULL || portal->value[0] == -1)
		/* no following through dead portals */
		continue;

	    if ( fch->master == ch && IS_AFFECTED(fch,AFF_CHARM)
		    &&   fch->position < POS_STANDING)
		do_function(fch, &do_stand, "");

	    if ( fch->master == ch && fch->position == POS_STANDING)
	    {
		act( "You follow $N.", fch, ch, NULL, NULL, NULL, NULL, NULL, TO_CHAR );
		do_function(fch, &do_enter, argument);
	    }
	}

	if (portal != NULL && portal->value[0] == -1)
	{
	    act("$p fades out of existence.",ch, NULL, NULL,portal,NULL, NULL, NULL,TO_CHAR);
	    if (ch->in_room == old_room)
		act("$p fades out of existence.",ch, NULL, NULL,portal,NULL, NULL, NULL,TO_ROOM);
	    else if (old_room->people != NULL)
	    {
		act("$p fades out of existence.",
			old_room->people, NULL, NULL,portal,NULL, NULL, NULL,TO_CHAR);
		act("$p fades out of existence.",
			old_room->people, NULL, NULL,portal,NULL, NULL, NULL,TO_ROOM);
	    }
	    extract_obj(portal);
	}

	/*
	 * If someone is following the char, these triggers get activated
	 * for the followers before the char, but it's safer this way...
	 */
	if (IS_NPC(ch))
	    p_percent_trigger( ch, NULL, NULL, NULL,NULL, NULL, NULL, NULL, NULL, TRIG_ENTRY , NULL);
	if ( !IS_NPC( ch ) ) {
	    p_greet_trigger( ch, PRG_MPROG );
	    p_greet_trigger( ch, PRG_OPROG );
	    p_greet_trigger( ch, PRG_RPROG );
	}
	return;
    }

    send_to_char("Nope, can't do it.\n\r",ch);
}
