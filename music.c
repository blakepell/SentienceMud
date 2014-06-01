#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"
#include "recycle.h"

void do_play(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *instrument;
    CHAR_DATA *mob;
    OBJ_DATA *obj;
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    int counter = 0;
    int this_class = 0;
    int chance = 0;

    argument = one_argument(argument,arg);
    argument = one_argument(argument,arg2);

    this_class = get_this_class(ch, gsn_music);

    if ((chance = get_skill(ch,gsn_music)) == 0)
    {
	send_to_char("You whistle a little tune to yourself.\n\r",ch);
	return;
    }

    if (ch->bashed > 0)
    {
	send_to_char("You must stand up first.\n\r", ch);
	return;
    }

    if ( arg[0] == '\0')
    {
	send_to_char("You know the following songs: \n\r\n\r", ch);
	send_to_char("{YSong Title                            Level         Mana{x\n\r", ch);
	send_to_char("{Y---------------------------------------------------------{x\n\r", ch);
	counter = 0;
	while( music_table[counter].name != NULL )
	{
	    if ( music_table[counter].level <= ch->level
	    || was_bard(ch))
	    {
		sprintf(buf, "%-30s %10d %13d\n\r",
			music_table[counter].name,
			music_table[counter].level,
			music_table[counter].mana );
		send_to_char(buf, ch);
	    }
	    counter++;
	}

	if (counter == 0)
	    send_to_char( "There are no songs you can play at this time.\n\r", ch );

        return;
    }

    obj = NULL;
    for (instrument = ch->carrying; instrument != NULL;
	 instrument = instrument->next_content )
    {
	if ( instrument->item_type == ITEM_INSTRUMENT )
	{
	    obj = instrument;
	    break;
	}
    }

    if (obj == NULL)
    {
	send_to_char("You don't have an instrument.\n\r", ch);
	return;
    }

    counter = 0;
    while( music_table[counter].name != NULL ) {
	if ( ( music_table[counter].level <= ch->level || was_bard(ch))
	&& !str_prefix(arg, music_table[counter].name))
	    break;
	else
	    counter++;
    }

    if (music_table[counter].name == NULL)
    {
	send_to_char("You don't know that song.\n\r", ch);
	return;
    }

    if ( arg2[0] != '\0' )
    {
	if ( ( mob = get_char_room(ch, NULL, arg2) ) == NULL )
	{
	    send_to_char("They aren't here.\n\r", ch);
	    return;
	}
	else if ( mob != NULL)
	{
	    switch (music_table[counter].target) {
		case TAR_IGNORE:
		    send_to_char("You can't harness the energies of that song onto one target.\n\r", ch );
		    return;
		    break;

		case TAR_CHAR_SELF:
		    if ( mob != ch ) {
			send_to_char("You can't harness the energies of that song onto anyone except yourself.\n\r", ch );
			return;
		    }
		    break;

		case TAR_CHAR_OFFENSIVE:
		case TAR_OBJ_CHAR_OFF:
		    if ( is_safe( ch, mob, TRUE ) )
		        return;
		    break;

		case TAR_CHAR_DEFENSIVE:
		case TAR_CHAR_FORMATION:
		case TAR_OBJ_CHAR_DEF:
		    if ( mob != ch
		    &&   mob->fighting != NULL
		    &&   ch->fighting != mob
		    &&   !IS_NPC(mob)
		    &&   !IS_NPC(mob->fighting)
		    &&   !is_pk(ch))
		    {
			send_to_char("You can't interfere in a PK battle if you are not PK.\n\r", ch );
			return;
		    }
		    break;
	    }

	    if ( mob == ch )
	    {
		send_to_char("{YYou begin to play the song softly to yourself...{X\n\r", ch);
		act( "{Y$n begins to play a song on $p{Y softly to $mself...{x", ch, NULL, NULL, obj, NULL, NULL, NULL, TO_ROOM);
	    }
	    else
	    {
		act("{YYou begin to play the song, sweetly exerting its influence on $N...{X", ch, mob, NULL, obj, NULL, NULL, NULL, TO_CHAR);
		act("{Y$n begins to play a song, exerting its influence on $N...{X", ch, mob, NULL, obj, NULL, NULL, NULL, TO_NOTVICT);
		act("{Y$n begins to play a song, exerting its influence on you...{X", ch, mob, NULL, obj, NULL, NULL, NULL, TO_VICT);
	    }
	}

	ch->music_target = str_dup(mob->name);
    }
    else
    {
	/* Syn- this fix is here to make sure offensive target songs cannot be played in safe rooms. */

	switch (music_table[counter].target) {
	    case TAR_CHAR_OFFENSIVE:
	    case TAR_OBJ_CHAR_OFF:
		if (IS_SET(ch->in_room->room_flags, ROOM_SAFE)) {
		    send_to_char("This room is sanctioned by the gods.\n\r", ch);
		    return;
		}

		break;
	}
	act( "{YYou begin to play a song on $p{Y...{x", ch, NULL, NULL, obj, NULL, NULL, NULL, TO_CHAR);
	act( "{Y$n begins to play a song on $p{Y...{x", ch, NULL, NULL, obj, NULL, NULL, NULL, TO_ROOM);
    }

    ch->song_num = counter;

    /* Sage has shorter playing time if Bard before */
    if (IS_SAGE(ch) && ch->pcdata->sub_class_thief == CLASS_THIEF_BARD)
	MUSIC_STATE(ch, (music_table[counter].beats * 2)/3);
    else
	MUSIC_STATE(ch, music_table[counter].beats);
}


void music_end( CHAR_DATA *ch, sh_int song_num )
{
    CHAR_DATA *mob, *mob_next;
    int sn;

    act( "{YYou finish playing your song.{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);

    if ( ch->mana < music_table[song_num].mana )
    {
	send_to_char("You don't have enough mana.\n\r", ch);
        return;
    }

    ch->mana -= music_table[song_num].mana;

    act( "{Y$n finishes $s song.{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);

    // We are casting it on just one person only
    if (ch->music_target != NULL)
    {
	mob = get_char_room(ch, NULL, ch->music_target);
	if ( mob == NULL )
	{
	    send_to_char("They aren't here.\n\r", ch);
	    free_string(ch->music_target);
	    ch->music_target = NULL;
	    return;
	}
	else
	{
	    if ( music_table[song_num].spell1 != NULL )
	    {
		for (sn = 0; sn < MAX_SKILL; sn++ )
		    if ( !str_cmp(skill_table[sn].name, music_table[song_num].spell1 ))
			(*skill_table[sn].spell_fun) (sn, ch->tot_level, ch, mob, TARGET_CHAR);
	    }

	    if ( music_table[song_num].spell2 != NULL )
	    {
		for (sn = 0; sn < MAX_SKILL; sn++ )
		    if ( !str_cmp(skill_table[sn].name, music_table[song_num].spell2 ))
			(*skill_table[sn].spell_fun) (sn, ch->tot_level, ch, mob, TARGET_CHAR);
	    }

	    if ( music_table[song_num].spell3 != NULL )
	    {
		for (sn = 0; sn < MAX_SKILL; sn++ )
		    if ( !str_cmp(skill_table[sn].name, music_table[song_num].spell3 ))
			(*skill_table[sn].spell_fun) (sn, ch->tot_level, ch, mob, TARGET_CHAR);
	    }

	    if (mob != ch && !is_safe(ch, mob, FALSE)
	    &&  (music_table[song_num].target == TAR_CHAR_OFFENSIVE
	         || music_table[song_num].target == TAR_OBJ_CHAR_OFF))
		multi_hit(ch, mob, TYPE_UNDEFINED);

	    free_string(ch->music_target);
	    ch->music_target = NULL;
	    return;
	}
    }

    switch( music_table[song_num].target )
    {
	case TAR_CHAR_FORMATION:
	    for ( mob = ch->in_room->people; mob != NULL; mob = mob_next )
	    {
	        mob_next = mob->next_in_room;

		if ( !is_same_group(mob, ch))
		    continue;

		if ( mob != ch
		&&   mob->fighting != NULL
		&&   !IS_NPC(mob)
		&&   !IS_NPC(mob->fighting)
		&&   !is_pk(ch))
		    continue;

		if ( music_table[song_num].spell1 != NULL )
		{
		    int sn = 0;
		    for (sn = 0; sn < MAX_SKILL; sn++ )
			if ( !str_cmp(skill_table[sn].name, music_table[song_num].spell1 ))
			    (*skill_table[sn].spell_fun) (sn, ch->tot_level, ch, mob, TARGET_CHAR);
		}

		if ( music_table[song_num].spell2 != NULL )
		{
		    int sn = 0;
		    for (sn = 0; sn < MAX_SKILL; sn++ )
			if ( !str_cmp(skill_table[sn].name, music_table[song_num].spell2 ))
			    (*skill_table[sn].spell_fun) (sn, ch->tot_level, ch, mob, TARGET_CHAR);
		}

		if ( music_table[song_num].spell3 != NULL )
		{
		    int sn = 0;
		    for (sn = 0; sn < MAX_SKILL; sn++ )
			if ( !str_cmp(skill_table[sn].name, music_table[song_num].spell3 ))
			    (*skill_table[sn].spell_fun) (sn, ch->tot_level, ch, mob, TARGET_CHAR);
		}

		if (mob != ch && !is_safe(ch, mob, FALSE)
		&&  (music_table[song_num].target == TAR_CHAR_OFFENSIVE
		     || music_table[song_num].target == TAR_OBJ_CHAR_OFF))
		    set_fighting(mob, ch);
	    }

	    break;

	case TAR_CHAR_DEFENSIVE:
	case TAR_OBJ_CHAR_DEF:
	    for ( mob = ch->in_room->people; mob != NULL; mob = mob_next )
	    {
	        mob_next = mob->next_in_room;

		if ( mob != ch
		&&   mob->fighting != NULL
		&&   !IS_NPC(mob)
		&&   !IS_NPC(mob->fighting)
		&&   !is_pk(ch))
		    continue;

		if ( music_table[song_num].spell1 != NULL )
		{
		  int sn = 0;
		  for (sn = 0; sn < MAX_SKILL; sn++ )
		    if ( !str_cmp(skill_table[sn].name, music_table[song_num].spell1 ))
			(*skill_table[sn].spell_fun) (sn, ch->tot_level, ch, mob, TARGET_CHAR);
		}

		if ( music_table[song_num].spell2 != NULL )
		{
		  int sn = 0;
		  for (sn = 0; sn < MAX_SKILL; sn++ )
		    if ( !str_cmp(skill_table[sn].name, music_table[song_num].spell2 ))
			(*skill_table[sn].spell_fun) (sn, ch->tot_level, ch, mob, TARGET_CHAR);
		}

		if ( music_table[song_num].spell3 != NULL )
		{
		  int sn = 0;
		  for (sn = 0; sn < MAX_SKILL; sn++ )
		    if ( !str_cmp(skill_table[sn].name, music_table[song_num].spell3 ))
			(*skill_table[sn].spell_fun) (sn, ch->tot_level, ch, mob, TARGET_CHAR);
		}
	    }

	    break;

	case TAR_CHAR_OFFENSIVE:
	case TAR_OBJ_CHAR_OFF:
	    for ( mob = ch->in_room->people; mob != NULL; mob = mob_next)
	    {
	        mob_next = mob->next_in_room;

                if ( is_safe( ch, mob, FALSE ) )
		    continue;

		if ( music_table[song_num].spell1 != NULL
		&& !is_same_group(ch,mob))
		{
		  int sn = 0;
		  for (sn = 0; sn < MAX_SKILL; sn++ )
		    if ( !str_cmp(skill_table[sn].name, music_table[song_num].spell1 )
		    && mob != ch )
			(*skill_table[sn].spell_fun) (sn, ch->tot_level, ch, mob, TARGET_CHAR);
		}

		if ( music_table[song_num].spell2 != NULL
		&& !is_same_group(ch,mob))
		{
		  int sn = 0;
		  for (sn = 0; sn < MAX_SKILL; sn++ )
		    if ( !str_cmp(skill_table[sn].name, music_table[song_num].spell2 )
		    && mob != ch )
			(*skill_table[sn].spell_fun) (sn, ch->tot_level, ch, mob, TARGET_CHAR);
		}

		if ( music_table[song_num].spell3 != NULL
		&& !is_same_group(ch,mob))
		{
		  int sn = 0;
		  for (sn = 0; sn < MAX_SKILL; sn++ )
		    if ( !str_cmp(skill_table[sn].name, music_table[song_num].spell3 )
		    && mob != ch )
			(*skill_table[sn].spell_fun) (sn, ch->tot_level, ch, mob, TARGET_CHAR);
		}
	    }

	    break;

	case TAR_CHAR_SELF:
	    if ( music_table[song_num].spell1 != NULL )
	    {
	      int sn = 0;
	      for (sn = 0; sn < MAX_SKILL; sn++ )
		if ( !str_cmp(skill_table[sn].name, music_table[song_num].spell1 ))
		    (*skill_table[sn].spell_fun) (sn, ch->tot_level, ch, ch, TARGET_CHAR);
	    }
	    if ( music_table[song_num].spell2 != NULL )
	    {
	      int sn = 0;
	      for (sn = 0; sn < MAX_SKILL; sn++ )
		if ( !str_cmp(skill_table[sn].name, music_table[song_num].spell2 ))
		    (*skill_table[sn].spell_fun) (sn, ch->tot_level, ch, ch, TARGET_CHAR);
	    }
	    if ( music_table[song_num].spell3 != NULL )
	    {
	      int sn = 0;
	      for (sn = 0; sn < MAX_SKILL; sn++ )
		if ( !str_cmp(skill_table[sn].name, music_table[song_num].spell3 ))
		    (*skill_table[sn].spell_fun) (sn, ch->tot_level, ch, ch, TARGET_CHAR);
	    }

	    break;

	case TAR_IGNORE:
	    if ( music_table[song_num].spell1 != NULL )
	    {
	      int sn = 0;
	      for (sn = 0; sn < MAX_SKILL; sn++ )
		if ( !str_cmp(skill_table[sn].name, music_table[song_num].spell1 ))
		    (*skill_table[sn].spell_fun) (sn, ch->tot_level, ch, ch, TARGET_NONE);
	    }

	    if ( music_table[song_num].spell2 != NULL )
	    {
	      int sn = 0;
	      for (sn = 0; sn < MAX_SKILL; sn++ )
		if ( !str_cmp(skill_table[sn].name, music_table[song_num].spell2 ))
		    (*skill_table[sn].spell_fun) (sn, ch->tot_level, ch, ch, TARGET_NONE);
	    }

	    if ( music_table[song_num].spell3 != NULL )
	    {
	      int sn = 0;
	      for (sn = 0; sn < MAX_SKILL; sn++ )
		if ( !str_cmp(skill_table[sn].name, music_table[song_num].spell3 ))
		    (*skill_table[sn].spell_fun) (sn, ch->tot_level, ch, ch, TARGET_NONE);
	    }
	    break;
    }

    check_improve(ch, gsn_music, TRUE, 2);
}


bool was_bard( CHAR_DATA *ch )
{
    // they are a bard now, so they have to level to get the songs
    if ( ch->pcdata->sub_class_current == CLASS_THIEF_BARD )
	return FALSE;

    // They were a bard sometime in the past so they dont have to level.
    if ( ch->pcdata->sub_class_thief == CLASS_THIEF_BARD )
	return TRUE;

    return FALSE;
}
