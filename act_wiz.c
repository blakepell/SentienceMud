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
 * ROM 2.4 is copyright 1993-1998 Russ Taylor                              *
 * ROM has been brought to you by the ROM consortium                       *
 *   Russ Taylor (rtaylor@hypercube.org)                                   *
 *   Gabrielle Taylor (gtaylor@hypercube.org)                              *
 *   Brian Moore (zump@rom.org)                                            *
 * By using this code, you have agreed to follow the terms of the          *
 * ROM license, in the file Rom24/doc/rom.license                          *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *    Scripting engine rebuilt by Michael Kurtz (Nibelung)                 *
 *    Used with permission.                                                *
 *                                                                         *
 **************************************************************************/

#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "merc.h"
#include "olc.h"
#include "interp.h"
#include "recycle.h"
#include "tables.h"
#include "olc_save.h"
#include "wilds.h"

extern void persist_save(void);
extern char *token_index_getvaluename(TOKEN_INDEX_DATA *token, int v);
extern void affect_fix_char(CHAR_DATA *ch);
extern bool newlock;
extern bool wizlock;
extern bool is_test_port;


int gconfig_read (void)
{
    FILE *fp;
    bool fMatch;
    char *word;
    char buf[MIL];
    extern GLOBAL_DATA gconfig;

    log_string("Loading configuration settings from gconfig.rc...");

    fp = fopen(CONFIG_FILE,"r");
    if (!fp)
    {
        bug("act_wiz.c, gconfig_read(): Unable to open gconfig.rc file for reading.",0);
        return(1); /* Failure*/
    }

    gconfig.next_mob_uid[0] = 1;	gconfig.next_mob_uid[1] = 0;
    gconfig.next_obj_uid[0] = 1;	gconfig.next_obj_uid[1] = 0;
    gconfig.next_token_uid[0] = 1;	gconfig.next_token_uid[1] = 0;
    gconfig.next_vroom_uid[0] = 1;	gconfig.next_vroom_uid[1] = 0;

    gconfig.next_church_uid = 1;

    for(;;)
    {
        word = feof (fp) ? "END" : fread_word(fp);
        fMatch = FALSE;

        switch (UPPER(word[0]))
        {
            case '*':
                fMatch = TRUE;
                fread_to_eol (fp);
            break;

           case 'E':
                if (!str_cmp(word, "END"))
                {
					gconfig.next_mob_uid[3] = gconfig.next_mob_uid[1];
					gconfig.next_mob_uid[2] = gconfig.next_mob_uid[0] + UID_INC - (gconfig.next_mob_uid[0] & UID_MASK);
					if(!gconfig.next_mob_uid[2]) gconfig.next_mob_uid[3]++;

					gconfig.next_obj_uid[3] = gconfig.next_obj_uid[1];
					gconfig.next_obj_uid[2] = gconfig.next_obj_uid[0] + UID_INC - (gconfig.next_obj_uid[0] & UID_MASK);
					if(!gconfig.next_obj_uid[2]) gconfig.next_obj_uid[3]++;

					gconfig.next_token_uid[3] = gconfig.next_token_uid[1];
					gconfig.next_token_uid[2] = gconfig.next_token_uid[0] + UID_INC - (gconfig.next_token_uid[0] & UID_MASK);
					if(!gconfig.next_token_uid[2]) gconfig.next_token_uid[3]++;

					gconfig.next_vroom_uid[3] = gconfig.next_vroom_uid[1];
					gconfig.next_vroom_uid[2] = gconfig.next_vroom_uid[0] + UID_INC - (gconfig.next_vroom_uid[0] & UID_MASK);
					if(!gconfig.next_vroom_uid[2]) gconfig.next_vroom_uid[3]++;


					if(!gconfig.next_church_uid) gconfig.next_church_uid++;
					fclose(fp);
					gconfig_write();
					return(0); /* Success*/
				}
	            break;

            case 'N':
            	if(!str_cmp(word,"NextMobUID")) {
					gconfig.next_mob_uid[0] = fread_number(fp);
					gconfig.next_mob_uid[1] = fread_number(fp);
					fMatch = TRUE;
					break;
				}
            	if(!str_cmp(word,"NextObjUID")) {
					gconfig.next_obj_uid[0] = fread_number(fp);
					gconfig.next_obj_uid[1] = fread_number(fp);
					fMatch = TRUE;
					break;
				}
            	if(!str_cmp(word,"NextTokenUID")) {
					gconfig.next_token_uid[0] = fread_number(fp);
					gconfig.next_token_uid[1] = fread_number(fp);
					fMatch = TRUE;
					break;
				}
            	if(!str_cmp(word,"NextVRoomUID")) {
					gconfig.next_vroom_uid[0] = fread_number(fp);
					gconfig.next_vroom_uid[1] = fread_number(fp);
					fMatch = TRUE;
					break;
				}
                KEY ("NextAreaUID", gconfig.next_area_uid, fread_number(fp));
                KEY ("NextWildsUID", gconfig.next_wilds_uid, fread_number(fp));
                KEY ("NextVlinkUID", gconfig.next_vlink_uid, fread_number(fp));
                KEY ("NextChurchUID", gconfig.next_church_uid, fread_number(fp));

                if(!str_cmp(word,"Newlock")) {
					newlock = TRUE;
					fMatch = TRUE;
					break;
				}
	            break;
			case 'T':
                if(!str_cmp(word,"Testport")) {
					is_test_port = TRUE;
					fMatch = TRUE;
					break;
				}
				break;
			case 'W':
                if(!str_cmp(word,"Wizlock")) {
					wizlock = TRUE;
					fMatch = TRUE;
					break;
				}
				break;

        } /* end switch */

        if (!fMatch)
        {
	    sprintf(buf, "act_wiz.c, gconfig_read(): no match for '%s'!", word);
	    bug(buf, 0);
            fread_to_eol(fp);
        }
    } /* end for */
}


int gconfig_write(void)
{
    FILE *fp;
    extern GLOBAL_DATA gconfig;

    fp = fopen(CONFIG_FILE,"w");
    if (!fp)
    {
        bug("act_wiz.c, gconfig_write(): Unable to open gconfig.rc file for writing.",0);
        return(1); /* Failure*/
    }

    fprintf(fp, "NextMobUID %ld %ld\n", gconfig.next_mob_uid[2], gconfig.next_mob_uid[3]);
    fprintf(fp, "NextObjUID %ld %ld\n", gconfig.next_obj_uid[2], gconfig.next_obj_uid[3]);
    fprintf(fp, "NextTokenUID %ld %ld\n", gconfig.next_token_uid[2], gconfig.next_token_uid[3]);
    fprintf(fp, "NextVRoomUID %ld %ld\n", gconfig.next_vroom_uid[2], gconfig.next_vroom_uid[3]);
    fprintf(fp, "NextAreaUID %ld\n", gconfig.next_area_uid);
    fprintf(fp, "NextWildsUID %ld\n", gconfig.next_wilds_uid);
    fprintf(fp, "NextVlinkUID %ld\n", gconfig.next_vlink_uid);
    fprintf(fp, "NextChurchUID %ld\n", gconfig.next_church_uid);
    if(newlock) fprintf(fp, "Newlock\n");
    if(wizlock) fprintf(fp, "Wizlock\n");
    if(is_test_port) fprintf(fp, "Testport\n");

    fprintf(fp, "END\n");
    fclose(fp);
/*    log_string("act_wiz.c, gconfig_write(): Config written to 'gconfig.rc'.");*/
    return(0); /* Success*/
}


void do_wiznet(CHAR_DATA *ch, char *argument)
{
    int flag;
    char buf[MAX_STRING_LENGTH];

    if (argument[0] == '\0')
    {
      	if (IS_SET(ch->wiznet,WIZ_ON))
      	{
            send_to_char("Signing off of Wiznet.\n\r",ch);
            REMOVE_BIT(ch->wiznet,WIZ_ON);
      	}
      	else
      	{
            send_to_char("Welcome to Wiznet!\n\r",ch);
            SET_BIT(ch->wiznet,WIZ_ON);
      	}
      	return;
    }

    if (!str_prefix(argument,"on"))
    {
	send_to_char("Welcome to Wiznet!\n\r",ch);
	SET_BIT(ch->wiznet,WIZ_ON);
	return;
    }

    if (!str_prefix(argument,"off"))
    {
	send_to_char("Signing off of Wiznet.\n\r",ch);
	REMOVE_BIT(ch->wiznet,WIZ_ON);
	return;
    }

    /* show wiznet status */
    if (!str_prefix(argument,"status"))
    {
	buf[0] = '\0';

	if (!IS_SET(ch->wiznet,WIZ_ON))
	    strcat(buf,"off ");

	for (flag = 0; wiznet_table[flag].name != NULL; flag++)
	    if (IS_SET(ch->wiznet,wiznet_table[flag].flag))
	    {
		strcat(buf,wiznet_table[flag].name);
		strcat(buf," ");
	    }

	strcat(buf,"\n\r");

	send_to_char("Wiznet status:\n\r",ch);
	send_to_char(buf,ch);
	return;
    }

    if (!str_prefix(argument,"show"))
    /* list of all wiznet options */
    {
	buf[0] = '\0';

	for (flag = 0; wiznet_table[flag].name != NULL; flag++)
	{
	    if (wiznet_table[flag].level <= get_trust(ch))
	    {
	    	strcat(buf,wiznet_table[flag].name);
	    	strcat(buf," ");
	    }
	}

	strcat(buf,"\n\r");

	send_to_char("Wiznet options available to you are:\n\r",ch);
	send_to_char(buf,ch);
	return;
    }

    flag = wiznet_lookup(argument);

    if (flag == -1 || get_trust(ch) < wiznet_table[flag].level)
    {
	send_to_char("No such option.\n\r",ch);
	return;
    }

    if (IS_SET(ch->wiznet,wiznet_table[flag].flag))
    {
	sprintf(buf,"You will no longer see %s on wiznet.\n\r",
	        wiznet_table[flag].name);
	send_to_char(buf,ch);
	REMOVE_BIT(ch->wiznet,wiznet_table[flag].flag);
    	return;
    }
    else
    {
    	sprintf(buf,"You will now see %s on wiznet.\n\r",
		wiznet_table[flag].name);
	send_to_char(buf,ch);
    	SET_BIT(ch->wiznet,wiznet_table[flag].flag);
	return;
    }

}


void wiznet(char *string, CHAR_DATA *ch, OBJ_DATA *obj,
	    long flag, long flag_skip, int min_level)
{
    DESCRIPTOR_DATA *d;

    for (d = descriptor_list; d != NULL; d = d->next)
    {
        if (d->connected == CON_PLAYING
	&&  IS_IMMORTAL(d->character)
	&&  IS_SET(d->character->wiznet,WIZ_ON)
	&&  (!flag || IS_SET(d->character->wiznet,flag))
	&&  (!flag_skip || !IS_SET(d->character->wiznet,flag_skip))
	&&  get_trust(d->character) >= min_level
	&&  d->character != ch)
        {
	    /* Higher level imms can see lower level imms sign on wizi, but not vice versa. */
            if (ch != NULL && flag == WIZ_LOGINS) {
		if (d->character->tot_level < ch->tot_level
		&&  ch->invis_level >= LEVEL_IMMORTAL)
		    continue;
	    }

	    if (IS_SET(d->character->wiznet,WIZ_PREFIX))
	  	send_to_char("{B({MSE{B){G-->{x ",d->character);
            act_new(string,d->character,ch,NULL,obj,NULL,NULL,NULL,TO_CHAR,POS_DEAD,NULL);
        }
    }
}


void do_zot(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument(argument, arg);

    if (arg[0] == '\0')
    {
	send_to_char("Zot whom?\n\r", ch);
	return;
    }

    if (ch->tot_level == MAX_LEVEL && !str_cmp(arg, "room"))
    {
    	for (victim = ch->in_room->people; victim != NULL; victim = victim->next_in_room)
	{
	    if (victim != ch
	    &&   victim->tot_level < ch->tot_level)
	    {
		send_to_char("{Y***{R****** {WZOT {R******{Y***{x\n\r\n\r", victim);

		send_to_char("{YYou are struck by a bolt of lightning!\n\r{x", victim);

		act("{Y$n is struck by a bolt of lightning!{x", victim, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);

		sprintf(buf, "{Y***ZOT*** {xYou have zotted %s!\n\r",
			IS_NPC(victim) ? victim->short_descr : victim->name);
		send_to_char(buf, ch);
		send_to_char("{ROUCH! That really did hurt!{x\n\r", victim);

		victim->hit = 1;
		victim->mana = 1;
		victim->move = 1;

		sprintf(buf, "%s zotted %s!",
			ch->name,
			IS_NPC(victim) ? victim->short_descr : victim->name);
		wiznet(buf, NULL, NULL, WIZ_IMMLOG, 0, 0);

		log_string(buf);
	    }
	}

	return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (IS_NPC(victim) && ch->tot_level < MAX_LEVEL)
    {
	send_to_char("Try zotting players instead.\n\r", ch);
	return;
    }

   if (!IS_NPC(victim) && ch->tot_level <= victim->tot_level)
   {
	send_to_char("You may only punish those below you.\n\r", ch);
	return;
   }

    send_to_char("{Y***{R****** {WZOT {R******{Y***{x\n\r\n\r", victim);

    send_to_char("{YYou are struck by a bolt of lightning!\n\r{x", victim);

    act("{Y$n is struck by a bolt of lightning!{x", victim, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);

    sprintf(buf, "{Y***ZOT*** {xYou have zotted %s!\n\r",
		    IS_NPC(victim) ? victim->short_descr : victim->name);
    send_to_char(buf, ch);
    send_to_char("{ROUCH! That really did hurt!{x\n\r", victim);

    victim->hit = 1;
    victim->mana = 1;
    victim->move = 1;

    sprintf(buf, "%s zotted %s!",
        ch->name,
	IS_NPC(victim) ? victim->short_descr : victim->name);
    wiznet(buf, NULL, NULL, WIZ_IMMLOG, 0, 0);

    log_string(buf);
}


void do_nochannels(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument(argument, arg);

    if (arg[0] == '\0')
    {
        send_to_char("Nochannel whom?", ch);
        return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (get_trust(victim) >= get_trust(ch))
    {
        send_to_char("You failed.\n\r", ch);
        return;
    }

    if (IS_SET(victim->comm, COMM_NOCHANNELS))
    {
        REMOVE_BIT(victim->comm, COMM_NOCHANNELS);
        send_to_char("The gods have restored your channel priviliges.\n\r",
		      victim);
        send_to_char("NOCHANNELS removed.\n\r", ch);
	sprintf(buf,"$N restores channels to %s",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
        SET_BIT(victim->comm, COMM_NOCHANNELS);
        send_to_char("The gods have revoked your channel priviliges.\n\r",
		       victim);
        send_to_char("NOCHANNELS set.\n\r", ch);
	sprintf(buf,"$N revokes %s's channels.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
}


void do_bamfin(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    if (!IS_NPC(ch))
    {
	smash_tilde(argument);

	if (argument[0] == '\0')
	{
	    sprintf(buf,"Your poofin is %s\n\r",ch->pcdata->immortal->bamfin);
	    send_to_char(buf,ch);
	    return;
	}

	if (strstr(argument,ch->name) == NULL)
	{
	    send_to_char("You must include your name.\n\r",ch);
	    return;
	}

	free_string(ch->pcdata->immortal->bamfin);
	ch->pcdata->immortal->bamfin = str_dup(argument);

        sprintf(buf,"Your poofin is now %s\n\r",ch->pcdata->immortal->bamfin);
        send_to_char(buf,ch);
    }
}


void do_bamfout(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    if (!IS_NPC(ch))
    {
        smash_tilde(argument);

        if (argument[0] == '\0')
        {
            sprintf(buf,"Your poofout is %s\n\r",ch->pcdata->immortal->bamfout);
            send_to_char(buf,ch);
            return;
        }

        if (strstr(argument,ch->name) == NULL)
        {
            send_to_char("You must include your name.\n\r",ch);
            return;
        }

        free_string(ch->pcdata->immortal->bamfout);
        ch->pcdata->immortal->bamfout = str_dup(argument);

        sprintf(buf,"Your poofout is now %s\n\r",ch->pcdata->immortal->bamfout);
        send_to_char(buf,ch);
    }
}


void do_deny(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument(argument, arg);
    if (arg[0] == '\0')
    {
	send_to_char("Deny whom?\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (IS_NPC(victim))
    {
	send_to_char("Not on NPC's.\n\r", ch);
	return;
    }

    if (get_trust(victim) >= get_trust(ch))
    {
	send_to_char("You failed.\n\r", ch);
	return;
    }

    SET_BIT(victim->act, PLR_DENY);
    send_to_char("You are denied access!\n\r", victim);
    sprintf(buf,"$N denies access to %s",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    act("Denied access to $N.", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
    save_char_obj(victim);
    stop_fighting(victim,TRUE);
    do_function(victim, &do_quit, NULL);
}


void do_disconnect(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    one_argument(argument, arg);
    if (arg[0] == '\0')
    {
	send_to_char("Disconnect whom?\n\r", ch);
	return;
    }

    if (is_number(arg))
    {
	int desc;

	desc = atoi(arg);
    	for (d = descriptor_list; d != NULL; d = d->next)
    	{
            if (d->descriptor == desc)
            {
            	act("Disconnected $N.", ch, d->character, NULL, NULL, NULL, NULL, NULL, TO_CHAR);

				connection_remove(d);
				close_socket(d);
            	return;
            }
	}
    }

    if ((victim = get_char_world(ch, arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (victim->desc == NULL)
    {
	act("$N doesn't have a descriptor.", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
	return;
    }

    for (d = descriptor_list; d != NULL; d = d->next)
    {
	if (d == victim->desc)
	{
            act("Disconnected $N.", ch, d->character, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
			connection_remove(d);
	    close_socket(d);
	    return;
	}
    }

    bug("Do_disconnect: desc not found.", 0);
    send_to_char("Descriptor not found!\n\r", ch);
    return;
}


void do_echo(CHAR_DATA *ch, char *argument)
{
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0')
    {
	send_to_char("Global echo what?\n\r", ch);
	return;
    }

    for (d = descriptor_list; d; d = d->next)
    {
	if (d->connected == CON_PLAYING)
	{
	    if (get_trust(d->character) >= get_trust(ch))
		send_to_char("global> ",d->character);
	    send_to_char(argument, d->character);
	    send_to_char("\n\r",   d->character);
	}
    }
}


void do_recho(CHAR_DATA *ch, char *argument)
{
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0')
    {
	send_to_char("Local echo what?\n\r", ch);

	return;
    }

    for (d = descriptor_list; d; d = d->next)
    {
	if (d->connected == CON_PLAYING
	&&   d->character->in_room == ch->in_room)
	{
            if (get_trust(d->character) >= get_trust(ch))
                send_to_char("local> ",d->character);
	    send_to_char(argument, d->character);
	    send_to_char("\n\r",   d->character);
	}
    }

    return;
}


void do_zecho(CHAR_DATA *ch, char *argument)
{
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0')
    {
	send_to_char("Zone echo what?\n\r",ch);
	return;
    }

    for (d = descriptor_list; d; d = d->next)
    {
	if (d->connected == CON_PLAYING
	&&  d->character->in_room != NULL && ch->in_room != NULL
	&&  d->character->in_room->area == ch->in_room->area)
	{
	    if (get_trust(d->character) >= get_trust(ch))
		send_to_char("zone> ",d->character);
	    send_to_char(argument,d->character);
	    send_to_char("\n\r",d->character);
	}
    }
}


void do_pecho(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument(argument, arg);

    if (argument[0] == '\0' || arg[0] == '\0')
    {
	send_to_char("Personal echo what?\n\r", ch);
	return;
    }

    if  ((victim = get_char_world(ch, arg)) == NULL)
    {
	send_to_char("Target not found.\n\r",ch);
	return;
    }

    if (get_trust(victim) >= get_trust(ch) && get_trust(ch) != MAX_LEVEL)
        send_to_char("personal> ",victim);

    send_to_char(argument,victim);
    send_to_char("\n\r",victim);
    send_to_char("personal> ",ch);
    send_to_char(argument,ch);
    send_to_char("\n\r",ch);
}



void do_transfer(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *location;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);

    if (arg1[0] == '\0')
    {
	send_to_char("Transfer whom (and where)?\n\r", ch);
	return;
    }

    if (!str_cmp(arg1, "all"))
    {
	for (d = descriptor_list; d != NULL; d = d->next)
	{
	    if (d->connected == CON_PLAYING
	    &&   d->character != ch
	    &&   d->character->in_room != NULL
	    &&   can_see(ch, d->character)
	    &&   ch->tot_level >= d->character->tot_level)
	    {
		char buf[MAX_STRING_LENGTH];
		sprintf(buf, "%s %s", d->character->name, arg2);
		do_function(ch, &do_transfer, buf);
	    }
	}
	return;
    }

    if (arg2[0] == '\0')
	location = ch->in_room;
    else
    {
	if ((location = find_location(ch, arg2)) == NULL)
	{
	    send_to_char("No such location.\n\r", ch);
	    return;
	}

	if (!is_room_owner(ch,location) && room_is_private(location, ch)
	&&  get_trust(ch) < MAX_LEVEL)
	{
	    send_to_char("That room is private right now.\n\r", ch);
	    return;
	}
    }

    if ((victim = get_char_world(ch, arg1)) == NULL)
    {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (victim->in_room == NULL)
    {
	send_to_char("They are in limbo.\n\r", ch);
	return;
    }

    if (victim->tot_level > ch->tot_level && !IS_NPC(victim)) {
	send_to_char("You may not transfer those superior to you.\n\r", ch);
	return;
    }

    if (victim->fighting != NULL)
	stop_fighting(victim, TRUE);

    act("$n disappears.", victim, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
    char_from_room(victim);
	if(location->wilds)
		char_to_vroom(victim, location->wilds, location->x, location->y);
	else
		char_to_room(victim, location);

    if (victim->pet != NULL)
    {
    	char_from_room (ch->pet);
	if(location->wilds)
		char_to_vroom(victim->pet, location->wilds, location->x, location->y);
	else
		char_to_room(victim->pet, location);
    }

    if (ch != victim)
	act("$n has transferred you.", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_VICT);
    do_function(victim, &do_look, "auto");

	sprintf(buf, "Transferred $N to %s (%ld)",
	victim->in_room->name,
	victim->in_room->vnum);
	act(buf, ch, victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
}


void do_at(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *location;
	ROOM_INDEX_DATA *original;
	WILDS_DATA *wilds = NULL;
	OBJ_DATA *on;
	CHAR_DATA *wch;
	int x = 0;
	int y = 0;
	ITERATOR wit;

	argument = one_argument(argument, arg);

	if (ch->tot_level < 150) {
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!arg[0] || !argument[0]) {
		send_to_char("At where what?\n\r", ch);
		return;
	}

	if (!(location = find_location(ch, arg))) {
		send_to_char("No such location.\n\r", ch);
		return;
	}

	if (!is_room_owner(ch,location) && room_is_private(location, ch) &&
		get_trust(ch) < MAX_LEVEL) {
		send_to_char("That room is private right now.\n\r", ch);
		return;
	}

	original = ch->in_room;
	if(original->wilds) {
		wilds = original->wilds;
		x = original->x;
		y = original->y;
	}
	on = ch->on;
	char_from_room(ch);
	if(location->wilds)
		char_to_vroom(ch, location->wilds, location->x, location->y);
	else
		char_to_room(ch, location);
	interpret(ch, argument);

	/*
	* See if 'ch' still exists before continuing!
	* Handles 'at XXXX quit' case.
	*/

	iterator_start(&wit, loaded_chars);
	while(( wch = (CHAR_DATA *)iterator_nextdata(&wit)))
	{
		if (wch == ch) {
			char_from_room(ch);
			if(wilds)
				char_to_vroom(ch, wilds, x, y);
			else
				char_to_room(ch, original);
			ch->on = on;
			break;
		}
	}
	iterator_stop(&wit);
}

void do_startinvasion(CHAR_DATA *ch, char *argument)
{
    AREA_DATA *pArea;
    char arg1[MIL];
    char arg2[MIL];
    char arg3[MIL];
    char arg4[MIL];
    int max_level = 0;
    long leader_vnum = 0;
    long mob_vnum = 0;
    INVASION_QUEST *quest;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);
    argument = one_argument(argument, arg4);

    if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' || arg4[0] == '\0')
    {
	send_to_char("startinvasion AREA leader_vnum mob_vnum max_level\n\r", ch);
	return;
    }

  pArea = find_area(arg1);
  if (pArea == NULL) {
    send_to_char("Can't find that area.\n\r", ch);
    return;
  }

  leader_vnum = atol(arg2);
  mob_vnum = atol(arg3);
  max_level = atoi(arg4);

  if (get_mob_index(leader_vnum) == NULL || get_mob_index(mob_vnum) == NULL) {
    send_to_char("One or both of your mob vnums are wrong.\n\r", ch);
    return;
  }

  quest = create_invasion_quest(pArea, max_level, leader_vnum, mob_vnum);
  pArea->invasion_quest = quest;
}

void do_mapgoto(CHAR_DATA *ch, char *argument)
{
    ROOM_INDEX_DATA *pRoom;
    AREA_DATA *pArea;
    char arg1[MIL];
    char arg2[MIL];
    int dx, dy;
    long index = 0;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
	send_to_char("MapGoto X Y\n\r", ch);
	return;
    }

	dx = atoi(arg1);
	dy = atoi(arg2);

  pArea = find_area("Wilderness");

  index = (long)((long)dy * (long)pArea->map_size_x + dx + pArea->min_vnum + WILDERNESS_VNUM_OFFSET);

        if ((pRoom = get_room_index(index)) == NULL)
	{
	    send_to_char("Couldn't find room.\n\r", ch);
	    return;
	}

  char_from_room(ch);
  char_to_room(ch, pRoom);

    do_function(ch, &do_look, "auto");
}


void do_goto(CHAR_DATA *ch, char *argument)
{
    ROOM_INDEX_DATA *location;
    CHAR_DATA *rch;
    int count = 0;

    if (argument[0] == '\0')
    {
	send_to_char("Goto where?\n\r", ch);
	return;
    }

    if ((location = find_location(ch, argument)) == NULL)
    {
	send_to_char("No such location.\n\r", ch);
	return;
    }

    count = 0;
    for (rch = location->people; rch != NULL; rch = rch->next_in_room)
        count++;

    if (!is_room_owner(ch,location) && room_is_private(location, ch)
    &&  (count > 1 || get_trust(ch) < MAX_LEVEL))
    {
	send_to_char("That room is private right now.\n\r", ch);
	return;
    }

    if (ch->fighting != NULL)
	stop_fighting(ch, TRUE);

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
//	if (get_trust(rch) >= ch->invis_level)
//	{
	    if (ch->pcdata != NULL && ch->pcdata->immortal != NULL &&  ch->pcdata->immortal->bamfout[0] != '\0')
		act("$t",ch,rch, NULL, NULL, NULL,ch->pcdata->immortal->bamfout, NULL,TO_VICT);
	    else
		act("$n leaves in a swirling mist.",ch,rch, NULL, NULL, NULL, NULL, NULL,TO_VICT);
//	}
    }

    char_from_room(ch);
	if(location->wilds)
		char_to_vroom(ch, location->wilds, location->x, location->y);
	else
		char_to_room(ch, location);

    if (ch->pet != NULL)
    {
    	char_from_room (ch->pet);
	if(location->wilds)
		char_to_vroom(ch->pet, location->wilds, location->x, location->y);
	else
		char_to_room(ch->pet, location);
    }


    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (ch != rch /*&& get_trust(rch) >= ch->invis_level*/)
        {
            if (ch->pcdata != NULL && ch->pcdata->immortal != NULL && ch->pcdata->immortal->bamfin[0] != '\0')
                act("$t",ch,rch, NULL, NULL, NULL,ch->pcdata->immortal->bamfin, NULL,TO_VICT);
            else
                act("$n appears in a swirling mist.",ch,rch, NULL, NULL, NULL, NULL, NULL,TO_VICT);
        }
    }

    do_function(ch, &do_look, "auto");
}

void do_goxy (CHAR_DATA * ch, char *argument)
{
    AREA_DATA *pArea;
    WILDS_DATA *pWilds;
    CHAR_DATA *rch;
    char arg1[MIL];
    char arg2[MIL];
    char arg3[MIL];
    int x, y = 0;
    int wuid = 0;

    if (argument[0] == '\0')
    {
        send_to_char ("Goxy: Usage:\n\r", ch);
        send_to_char ("           : goxy <x coor> <y coor>        "
                      "- Standing in a wilds region, goto these coors.\n\r", ch);
        send_to_char ("           : goxy <x coor> <y coor> <wuid> "
                      "- The same, but specifying the target wilds uid.\n\r", ch);
        return;
    }

    if (!ch->in_room || (pArea = ch->in_room->area) == NULL)
    {
        send_to_char ("Goxy: You need to be in an area that has wilds defined.\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);

    if (!is_number(arg1) || (x=atoi(arg1)) < 0 || !is_number(arg2) || (y=atoi(arg2)) < 0)
    {
        send_to_char("Usage: goxy (x coordinate) (y coordinate).\n\r", ch);
        send_to_char("  e.g.   goxy 100 25\n\r", ch);
        return;
    }

    if (!str_cmp(arg3, ""))
    {
        if (!ch->in_wilds)
        {
            send_to_char("goxy: you're not in a wilds map. Type 'goxy' for usage info.\n\r", ch);
            return;
        }
        else
            pWilds = ch->in_wilds;
    }
    else
    {
        if (!is_number(arg3) || ((wuid=atoi(arg3)) <= 0))
        {
                send_to_char("goxy: You must specify a valid wuid. Type 'goxy' for usage info.\n\r", ch);
                return;
        }
        else
        {
            if ((pWilds = get_wilds_from_uid(NULL, wuid)) == NULL)
            {
                send_to_char("goxy: Could not find that wuid. Type 'goxy' for usage info.\n\r", ch);
                return;
            }
        }

    }

    /* Vizz - Remember, bottom right corner will have coors (map_size_x - 1, map_size_y - 1)
     *        because the map origin is at (0,0), not (1,1)
     */
    if (x > (pWilds->map_size_x - 1) || y > (pWilds->map_size_y - 1))
    {
        send_to_char("goxy: coordinate out of wilds range.\n\r", ch);
        return;
    }

    if (!check_for_bad_room(pWilds, x, y))
    {
        send_to_char("goxy: there isn't a room at that location!\n\r", ch);
        return;
    }

    if (ch->fighting != NULL)
        stop_fighting (ch, TRUE);

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust (rch) >= ch->invis_level)
        {
            if (ch->pcdata != NULL)
            {
                if (IS_IMMORTAL(ch) && ch->pcdata->immortal->bamfout[0] != '\0')
                    act ("$t", ch, rch, NULL, NULL, NULL, ch->pcdata->immortal->bamfout, NULL, TO_VICT);
                else
                    act ("$n leaves in a swirling mist.", ch, rch, NULL, NULL, NULL, NULL, NULL, TO_VICT);

/* Vizz - For later on... disabled for now.
                if (ch->pcdata->poofout_mspfile[0] != '\0')
                {
                    sprintf(buf, "!!SOUND(%s V=%d L=1 T=staff)",
                            ch->pcdata->poofout_mspfile,
                            ch->pcdata->poofout_mspvolume);
                    act( buf, ch, NULL, rch, TO_VICT_MSP );
                }
*/

            }

        }

    }

    char_from_room (ch);
    char_to_vroom (ch, pWilds, x, y);

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust (rch) >= ch->invis_level)
        {
            if (ch->pcdata != NULL && IS_IMMORTAL(ch) && ch->pcdata->immortal->bamfin[0] != '\0')
                act ("$t", ch, rch, NULL, NULL, NULL, ch->pcdata->immortal->bamfin, NULL, TO_VICT);
            else
                act ("$n appears in a swirling mist.", ch, rch, NULL, NULL, NULL, NULL, NULL,
                     TO_VICT);
        }
    }

    do_function (ch, &do_look, "auto");
    return;
}


void do_stat(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char *string;
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *location;
    CHAR_DATA *victim;

    string = one_argument(argument, arg);
    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  stat <name>\n\r",ch);
	send_to_char("  stat area <number>\n\r", ch);
	send_to_char("  stat wilds <wuid>\n\r", ch);
	send_to_char("  stat obj <name>\n\r",ch);
	send_to_char("  stat mob <name>\n\r",ch);
	send_to_char("  stat room <number>\n\r",ch);
	send_to_char("  stat aff <character or object>\n\r", ch);
	send_to_char("  stat token <character>\n\r", ch);
	return;
    }

    if (!str_cmp(arg,"room"))
    {
	do_function(ch, &do_rstat, string);
	return;
    }

    if (!str_cmp(arg,"obj"))
    {
	do_function(ch, &do_ostat, string);
	return;
    }

    if(!str_cmp(arg,"char")  || !str_cmp(arg,"mob"))
    {
	do_function(ch, &do_mstat, string);
	return;
    }

    if (!str_cmp (arg, "area"))
    {
        do_function (ch, &do_astat, string);
        return;
    }

    if (!str_cmp(arg, "token"))
    {
	do_function(ch, &do_tstat, string);
	return;
    }

    if (!str_cmp (arg, "wilds"))
    {
        do_function (ch, &do_wstat, string);
        return;
    }

    /* do it the old way */
    obj = get_obj_world(ch,argument);
    if (obj != NULL)
    {
	do_function(ch, &do_ostat, argument);
	return;
    }

    victim = get_char_world(ch,argument);
    if (victim != NULL)
    {
	do_function(ch, &do_mstat, argument);
	return;
    }

    location = find_location(ch,argument);
    if (location != NULL)
    {
	do_function(ch, &do_rstat, argument);
	return;
    }

    send_to_char("Nothing by that name found anywhere.\n\r",ch);
}

void do_astat (CHAR_DATA * ch, char *argument)
{
    AREA_DATA *pArea;
    WILDS_DATA *pWilds;
    BUFFER *output;
    char buf[MSL];
    char arg[MIL];
    int anum;

    one_argument(argument, arg);
    if (!str_cmp(arg, ""))
    {
        pArea = ch->in_room->area;
    }
    else
    {
        if (!is_number(arg))
        {
            send_to_char("Syntax:  stat area\n\r", ch);
            send_to_char("         stat area [uid]\n\r", ch);
            return;
        }
        else
        {
            anum = atoi(arg);
            pArea = get_area_from_uid(anum);
        }
    }

    if(!pArea) {
	    send_to_char("No such area exists.\n\r", ch);
	    return;
    }

    output = new_buf();
    add_buf (output, "\n\r{x[ {Wstat area{x ]\n\r\n\r");
    sprintf (buf, "Area    : [{W%3ld{x]  Uid	: [{W%6ld{x]\n\r", pArea->anum, pArea->uid);
    add_buf (output, buf);
    sprintf (buf, "Name    : [{W%s{x]\n\r", pArea->name ? pArea->name : "(not set)");
    add_buf (output, buf);
    sprintf (buf, "Filename: [{W%s{x]\n\r", pArea->file_name);
    add_buf (output, buf);
    sprintf (buf, "Vnums   : [{W%ld{x-{W%ld{x]\n\r", pArea->min_vnum, pArea->max_vnum);
    add_buf (output, buf);
    sprintf (buf, "Recall  : [{W%6ld{x] {W%s{x\n\r", pArea->recall.id[0],
             get_room_index (pArea->recall.id[0])
             ? get_room_index (pArea->recall.id[0])->name : "none");
    add_buf (output, buf);
    sprintf (buf, "Security: [{W%d{x]\n\r", pArea->security);
    add_buf (output, buf);
    sprintf (buf, "\n\r{C*Staff assigned*{x\n\r");
    add_buf (output, buf);
    sprintf (buf, "Builders: [{W%s{x]\n\r", pArea->builders ? pArea->builders : "(none set)");
    add_buf (output, buf);
    sprintf (buf, "Credits : [{W%s{x]\n\r", pArea->credits ? pArea->credits : "(none set)");
    add_buf (output, buf);
    add_buf (output, "\n\r{C*Wilds Sectors*{x\n\r");

    if (pArea->wilds)
    {
        int cnt=0;
        add_buf (output, "   UID       Dimensions     Name\n\r");
        for (pWilds = pArea->wilds;pWilds;pWilds = pWilds->next)
        {
            sprintf (buf, "  ({W%7ld{x) [{W%5d{x x {W%5d{x] '{W%s{x'\n\r",
                     pWilds->uid,
                     pWilds->map_size_x, pWilds->map_size_y,
                     pWilds->name);
            add_buf (output, buf);
            cnt++;
        }
    }
    else
    {
        add_buf (output, "    (None defined)\n\r");
    }

    add_buf (output, "\n\r{C*Current Attributes*{x\n\r");
    sprintf (buf, "Age     : [{W%d{x]\n\r", pArea->age);
    add_buf (output, buf);
    sprintf (buf, "Flags   : [{W%s{x]\n\r",
             flag_string (area_flags, pArea->area_flags));
    add_buf (output, buf);
    sprintf (buf, "Players : {W%d{x\n\r", pArea->nplayer);
    add_buf (output, buf);

    page_to_char(buf_string(output), ch);
    free_buf(output);
    return;
}


void do_rstat(CHAR_DATA *ch, char *argument)
{
    BUFFER *output;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location, *clone;
    OBJ_DATA *obj;
    CHAR_DATA *rch;
    int door;

    one_argument(argument, arg);
    location = (arg[0] == '\0') ? ch->in_room : find_location(ch, arg);
    if (location == NULL)
    {
	send_to_char("No such location.\n\r", ch);
	return;
    }

    if (!is_room_owner(ch,location) && ch->in_room != location
    &&  room_is_private(location, ch))
    {
	send_to_char("That room is private right now.\n\r", ch);
	return;
    }

    output = new_buf();
    add_buf (output, "{x\n\r[ {Wstat room{x ]\n\r");
    add_buf(output, "\n\r{C*Stats*{x\n\r");
    sprintf(buf, "{YName:{x %s\n\r"
                 "{YArea uid:{x %ld '%s'\n\r",
            location->name,
            location->area->uid,
            location->area->name);
    add_buf(output, buf);

    if (location->wilds == NULL)
    {

    sprintf(buf,
	"{BVnum:{x %ld  {BSector:{x %d  {BLight:{x %d  {BHealing:{x %d  {BMana:{x %d\n\r",
	location->vnum,
	location->sector_type,
                location->light,
                location->heal_rate,
                location->mana_rate);
    }
    else
    {
        sprintf(buf, "{YWilds uid:{x %ld '%s'\n\r"
                     "{YCoors:{x (%ld, %ld)  {YSector:{x %d  {YLight:{x %d  {YHealing:{x %d  {YMana:{x %d\n\r",
                location->wilds->uid,
                location->wilds->name,
                location->x,
                location->y,
                location->sector_type,
                location->light,
                location->heal_rate,
                location->mana_rate);
    }

    add_buf(output, buf);

    sprintf(buf,
            "{YRoom flags:{x %s.\n\r{YDescription:{x\n\r%s\n\r",
            flag_string(room_flags, location->room_flags),
            location->description);
    add_buf(output, buf);

    if (location->extra_descr != NULL)
    {
	EXTRA_DESCR_DATA *ed;

	sprintf(buf, "{YExtra description keywords: {x'");

	for (ed = location->extra_descr; ed; ed = ed->next)
	{
	    add_buf(output, ed->keyword);

	    if (ed->next != NULL)
                add_buf(output, " ");
	}

    }

    add_buf(output, "\n\r{C*Exits*{x\n\r");

    for (door = 0; door <= 9; door++)
    {
	EXIT_DATA *pexit;

	if ((pexit = location->exit[door]) != NULL)
	{
            if ((location->wilds == NULL && !IS_SET(pexit->exit_info, EX_VLINK))
                || (location->wilds != NULL && IS_SET(pexit->exit_info, EX_VLINK)))
            {
	        sprintf(buf,
                        "{x%s {Yto vnum {x%ld '%s' {Yin Area uid:{x %ld '%s'\n\r"
		        "    {YKey: {x%ld  {YExit flags: {x%s\n\r"
                        "    {YKeyword:{x '%s'  {YDescription: {x%s",
                        dir_name[door],
                        (pexit->u1.to_room == NULL ? -1 : pexit->u1.to_room->vnum),
                        pexit->u1.to_room->name,
                        pexit->u1.to_room->area->uid,
                        pexit->u1.to_room->area->name,
                        pexit->door.key_vnum,
                        flag_string(exit_flags, pexit->exit_info),
                        pexit->keyword,
                        pexit->short_desc[0] != '\0'
                        ? pexit->short_desc : "(none).\n\r");
            }
            else
            {
                if (location->wilds == NULL)
                    /* Exit goes to a wilds location from a static one*/
                    sprintf(buf,
                            "{x%s {Yto coors{x(%d, %d) {Yin Wilds uid:{x %ld{Y, Area uid:{x %ld\n\r"
                            "    {YKey: {x%ld  {YExit flags: {x%s\n\r"
                            "    {YKeyword:{x '%s'  {YDescription: {x%s",
                            dir_name[door],
                            pexit->wilds.x,
                            pexit->wilds.y,
                            pexit->wilds.wilds_uid,
                            pexit->wilds.area_uid,
                            pexit->door.key_vnum,
                            flag_string(exit_flags, pexit->exit_info),
                            pexit->keyword,
                            pexit->short_desc[0] != '\0'
                                                 ? pexit->short_desc : "(none).\n\r");
                else
                    /* Exit goes to a wilds location from another wilds one*/
                    sprintf(buf,
                            "{x%s {Yto coors{x (%d, %d)\n\r"
                            "    {YKey: {x%ld  {YExit flags: {x%s\n\r"
                            "    {YKeyword:{x '%s'  {YDescription: {x%s",
                            dir_name[door],
                            pexit->wilds.x,
                            pexit->wilds.y,
                            pexit->door.key_vnum,
                            flag_string(exit_flags, pexit->exit_info),
                            pexit->keyword,
                            pexit->short_desc[0] != '\0'
                                                 ? pexit->short_desc : "(none).\n\r");
            }

	    add_buf(output, buf);
	}
    }

    add_buf(output, "\n\r{C*Contents*{x\n\r");
    add_buf(output, "{YCharacters:{x");
    for (rch = location->people; rch; rch = rch->next_in_room)
    {
	if (can_see(ch,rch))
        {
	    add_buf(output, " ");
	    one_argument(rch->name, buf);
	    add_buf(output, buf);
	}
    }

    add_buf(output, ".\n\r{YObjects:{x");

    if (location->contents != NULL)
    {
        for (obj = location->contents; obj; obj = obj->next_content)
        {
            char buf2[MAX_STRING_LENGTH];

            add_buf(output, " ");
            one_argument(obj->name, buf);
            sprintf(buf2,"(%ld)", obj->pIndexData->vnum);
            strcat(buf, buf2);
            add_buf(output, buf);
        }
    }
    else
        add_buf(output, " (none)");

    add_buf(output, ".\n\r");

    if(location->clones) {
	    add_buf(output,"{CClones:{x\n\r");
	    for(clone = location->clones; clone; clone = clone->next) {
		    switch(clone->environ_type) {
		    case ENVIRON_ROOM: sprintf(buf,"{W%lu:%lu{x at Room [%ld:%lu:%lu]", clone->id[0], clone->id[1], clone->environ.room->vnum, clone->environ.room->id[0], clone->environ.room->id[1]); break;
		    case ENVIRON_MOBILE: sprintf(buf,"{W%lu:%lu{x in Mobile '%s' %ld [%lu:%lu]", clone->id[0], clone->id[1], clone->environ.mob->short_descr, VNUM(clone->environ.mob), clone->environ.mob->id[0], clone->environ.mob->id[1]); break;
		    case ENVIRON_OBJECT: sprintf(buf,"{W%lu:%lu{x in Object '%s' %ld [%lu:%lu]", clone->id[0], clone->id[1], clone->environ.obj->short_descr, VNUM(clone->environ.obj), clone->environ.obj->id[0], clone->environ.obj->id[1]); break;
		    case ENVIRON_TOKEN: sprintf(buf,"{W%lu:%lu{x in Token '%s' %ld [%lu:%lu]", clone->id[0], clone->id[1], clone->environ.token->name, VNUM(clone->environ.token), clone->environ.token->id[0], clone->environ.token->id[1]); break;
		    default: sprintf(buf,"{W%lu:%lu{x in ???", clone->id[0], clone->id[1]);
		    }

		    add_buf(output,buf);
		    add_buf(output,"\n\r");
	    }
    }

    page_to_char (buf_string(output), ch);
    free_buf(output);
    return;
}


/* VIZZWILDS */
void do_wstat (CHAR_DATA * ch, char *argument)
{
    AREA_DATA *pArea;
    WILDS_DATA *pWilds;
    BUFFER *output;
    char buf[MSL];
    char arg[MIL];
    int wuid = 0;

    one_argument (argument, arg);

    if (!str_cmp(arg, ""))
    {
        if ((pWilds = ch->in_wilds) == NULL)
        {
            send_to_char("Stat wilds: You're not in a wilds region. Use 'stat wilds <wuid> to specify.\n\r", ch);
            return;
        }

        pArea = pWilds->pArea;
    }
    else
    {
        if (!is_number(arg))
        {
            send_to_char("Syntax:  stat wilds\n\r", ch);
            send_to_char("         stat wilds <wuid>\n\r", ch);
            return;
        }

        wuid = atoi(arg);

        if ((pWilds = get_wilds_from_uid(NULL, wuid)) == NULL)
        {
            send_to_char("Stat wilds: Couldn't locate that wuid in any loaded area.\n\r", ch);
            return;
        }

        pArea = pWilds->pArea;
    }


    output = new_buf();
    add_buf (output, "{x\n\r[ {Wstat wilds{x ]\n\r");

    sprintf(buf, "\n\r{C*Stats*{x\n\r");
    add_buf(output, buf);


    sprintf(buf, "Wilds uid: ({W%ld{x), Name: '{W%s{x'\n\r", pWilds->uid, pWilds->name);
    add_buf(output, buf);

    sprintf(buf, "Defined in area {W%ld{x, '{W%s{x'\n\r",
                  pArea->anum, pArea->name);
    add_buf(output, buf);

    sprintf(buf, "Dimensions: {W%d{x x {W%d{x ({W%ld{x vrooms)\n\r",
                  pWilds->map_size_x,
                  pWilds->map_size_y,
                  (long)(pWilds->map_size_x * pWilds->map_size_y));
    add_buf(output, buf);

    sprintf(buf, "Repop age: {W%d{x\n\r", pWilds->repop);
    add_buf(output, buf);

    sprintf(buf, "\n\r{C*Current Info*{x\n\r");
    add_buf(output, buf);

    sprintf(buf, "Players: {W%d{x\n\r", pWilds->nplayer);
    add_buf(output, buf);

    sprintf(buf, "Loaded Vrooms: %d\n\r", list_size(pWilds->loaded_vrooms));
    add_buf(output, buf);

    sprintf(buf, "Loaded Mobiles: %d\n\r", pWilds->loaded_mobs);
    add_buf(output, buf);

    sprintf(buf, "Loaded Objects: %d\n\r", pWilds->loaded_objs);
    add_buf(output, buf);

    sprintf(buf, "Current Age: {W%d{x\n\r", pWilds->age);
    add_buf(output, buf);

    page_to_char (buf_string(output), ch);
    free_buf(output);
    return;
}

void do_ostat(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    OBJ_DATA *obj;
    EVENT_DATA *ev;
    ROOM_INDEX_DATA *room;

    one_argument(argument, arg);

    if (arg[0] == '\0')
    {
	send_to_char("Stat what?\n\r", ch);
	return;
    }

    if ((obj = get_obj_world(ch, argument)) == NULL)
    {
	send_to_char("Object not found.\n\r", ch);
	return;
    }

    sprintf(buf, "{BShort desc: {x%s {BName(s):{x %s\n\r",
	obj->short_descr, obj->name);
    send_to_char(buf, ch);

    sprintf(buf, "{BVnum:{x %ld {BArea: {x%s {BType:{x %s\n\r",
	obj->pIndexData->vnum, obj->pIndexData->area->name,
	item_name(obj->item_type));
    send_to_char(buf, ch);

    if (obj->loaded_by != NULL && ch->tot_level >= LEVEL_IMMORTAL)
    {
	sprintf(buf, "{BItem loaded by {x%s\n\r", obj->loaded_by);
	send_to_char(buf, ch);
    }

    sprintf(buf, "{BLong description:{x %s\n\r{BFull description:\n\r {x%s",
	obj->description, obj->full_description);
    send_to_char(buf, ch);

    sprintf(buf, "{BWear bits: {x%s\n\r{BExtra bits:{x %s\n\r{BExtra2 bits:{x %s\n\r",
	wear_bit_name(obj->wear_flags), extra_bit_name(obj->extra_flags),
	extra2_bit_name(obj->extra2_flags));
    send_to_char(buf, ch);

    sprintf(buf, "{BNumber:{x %d/%d {BWeight:{x %d\n\r",
	1,           get_obj_number(obj),
	get_obj_weight(obj));
    send_to_char(buf, ch);

    sprintf(buf, "{BLevel:{x %d {BCost:{x %ld {BCondition:{x %d {BTimer:{x %d {BOwner:{x %s\n\r",
	obj->level, obj->cost, obj->condition, obj->timer, obj->owner);
    send_to_char(buf, ch);

    if (obj->in_wilds == NULL)
    sprintf(buf,
	"{BIn room:{x %ld {BIn object:{x %s {BCarried by:{x %s {BIn mail:{x %s {BWear_loc:{x %d\n\r",
	obj->carried_by == NULL && obj->in_room != NULL ? obj->in_room->vnum : 0,
	obj->in_obj     == NULL ? "(none)" : obj->in_obj->short_descr,
	obj->carried_by == NULL ? "(none)" : obj->carried_by->name,
	obj->in_mail == NULL ? "No" : "Yes",
	obj->wear_loc);
    else
        sprintf(buf,
                "In wilds: %ld - '%s', at (%d, %d).\n\r",
                obj->in_wilds->uid, obj->in_wilds->name, obj->x, obj->y);

    send_to_char(buf, ch);

    sprintf(buf, "{BValues:{x %d %d %d %d %d %d %d %d\n\r",
	obj->value[0], obj->value[1], obj->value[2], obj->value[3],
	obj->value[4], obj->value[5], obj->value[6], obj->value[7]);
    send_to_char(buf, ch);

    if (obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL)
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char("{BExtra description keywords: {x", ch);

	for (ed = obj->extra_descr; ed != NULL; ed = ed->next)
	{
	    send_to_char(ed->keyword, ch);
	    if (ed->next != NULL)
	    	send_to_char(" ", ch);
	}

	for (ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next)
	{
	    send_to_char(ed->keyword, ch);
	    if (ed->next != NULL)
		send_to_char(" ", ch);
	}

	send_to_char("\n\r", ch);
    }

    for (paf = obj->affected; paf != NULL; paf = paf->next)
    {
	sprintf(buf, "{BAffects{x %-12s {Bby{x %3d{B, level{x %3d",
	    affect_loc_name(paf->location), paf->modifier,paf->level);
	send_to_char(buf,ch);
	if (paf->duration > -1)
	    sprintf(buf,", %d {Bhours.{x\n\r",paf->duration);
	else
	    sprintf(buf,"{B.{x\n\r");
	send_to_char(buf, ch);
    }

    for (ev = obj->events; ev != NULL; ev = ev->next_event) {
	sprintf(buf, "{M* {BEvent {x%-53.52s {B[{x%7.3f{B seconds{B]{x\n\r", ev->args, (float) ev->delay/2);
	send_to_char(buf, ch);
    }


    for (room = obj->clone_rooms; room; room = room->next_clone) {
	sprintf(buf, "{M* {CClone {W%ld {C[{W%lu{C:{W%lu{C]{x\n\r", room->source->vnum, room->id[0], room->id[1]);
	send_to_char(buf, ch);
    }
}


void do_mstat(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    CHAR_DATA *victim;
    EVENT_DATA *ev;

    one_argument(argument, arg);

    if (arg[0] == '\0')
    {
	send_to_char("Stat whom?\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, argument)) == NULL)
    {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    sprintf(buf, "{BName:{x %s\n\r", HANDLE(victim));
    send_to_char(buf, ch);

    if (victim->in_wilds == NULL)
    {
        sprintf(buf, "Area uid:{x %ld '%s'\n\r"
                     "{YIn_room:{x %ld '%s'\n\r",
                victim->in_room->area->uid,
                victim->in_room->area->name,
                victim->in_room->vnum,
                victim->in_room->name);
    }
    else
    {
        sprintf(buf, "{YArea uid:{x %ld '%s'\n\r"
                     "{YIn_wilds:{x %ld '%s', {Yat{x (%d, %d)\n\r",
	       	victim->in_room->area->uid,
                victim->in_room->area->name,
                victim->in_wilds->uid,
                victim->in_wilds->name,
		victim->at_wilds_x,
		victim->at_wilds_y);
    }

    sprintf(buf,
	"{BVnum:{x %ld  {BRace:{x %s  {BSex:{x %s  {BRoom:{x %ld\n\r",
	VNUM(victim),
	race_table[victim->race].name,
	sex_table[victim->sex].name,
	victim->in_room == NULL ? 0 : victim->in_room->vnum);
    send_to_char(buf, ch);

    sprintf(buf,
   	"{BStr:{x %d{W({x%d{W){x  {BInt:{x %d{W({x%d{W){x  {BWis:{x %d{W({x%d{W){x  {BDex:{x %d{W({x%d{W){x  {BCon:{x %d{W({x%d{W){x\n\r",
	victim->perm_stat[STAT_STR],
	get_curr_stat(victim,STAT_STR),
	victim->perm_stat[STAT_INT],
	get_curr_stat(victim,STAT_INT),
	victim->perm_stat[STAT_WIS],
	get_curr_stat(victim,STAT_WIS),
	victim->perm_stat[STAT_DEX],
	get_curr_stat(victim,STAT_DEX),
	victim->perm_stat[STAT_CON],
	get_curr_stat(victim,STAT_CON));
    send_to_char(buf, ch);

    sprintf(buf, "{BHp: {x%ld/%ld  {BMana: {x%ld/%ld  {BMove:{x %ld/%ld  {BPractices:{x %d {BTrains:{x %d\n\r",
	victim->hit,         victim->max_hit,
	victim->mana,        victim->max_mana,
	victim->move,        victim->max_move,
	IS_NPC(victim) ? 0 : victim->practice,
	IS_NPC(victim) ? 0 : victim->train);
    send_to_char(buf, ch);

    sprintf(buf,
	"{BLv:{x %d  {BAlign:{x %d  {BGold:{x %ld  {BSilver:{x %ld  {BExp:{x %ld\n\r",
	victim->tot_level,
	victim->alignment,
	victim->gold, victim->silver, victim->exp);
    send_to_char(buf, ch);

    if (!IS_NPC(ch)) {
	sprintf(buf, "{BKarma:{x %ld  {BPneuma:{x %ld  {BQuestPoints:{x %d{x\n\r",
	    victim->deitypoints, victim->pneuma, victim->questpoints);
	send_to_char(buf, ch);
    }

    sprintf(buf,"{BArmour:{x pierce: %d  bash: %d  slash: %d  magic: %d\n\r",
	    GET_AC(victim,AC_PIERCE), GET_AC(victim,AC_BASH),
	    GET_AC(victim,AC_SLASH),  GET_AC(victim,AC_EXOTIC));
    send_to_char(buf,ch);

    sprintf(buf,
	"{BHitroll:{x %d  {BDamroll:{x %f {W[{x%d{W]{x {BSize:{x %s {BPosition:{x %s {BWimpy:{x %d\n\r",
	GET_HITROLL(victim), 40*log(GET_DAMROLL(victim)), GET_DAMROLL(victim),
	size_table[victim->size].name, position_table[victim->position].name,
	victim->wimpy);
    send_to_char(buf, ch);

    sprintf(buf, "{BIdle:{x %d minutes ", victim->timer);
    send_to_char(buf, ch);

    if (IS_NPC(victim))
    {
		if( victim->damage.bonus > 0 )
			sprintf(buf, "{BDamage:{x %dd%d+%d  {BMessage:{x  %s\n\r",
			    victim->damage.number,victim->damage.size,victim->damage.bonus,
			    attack_table[victim->dam_type].noun);
		else
			sprintf(buf, "{BDamage:{x %dd%d  {BMessage:{x  %s\n\r",
			    victim->damage.number,victim->damage.size,
			    attack_table[victim->dam_type].noun);
	send_to_char(buf,ch);
    }

    if (IS_NPC(victim) && victim->hunting != NULL)
    {
        sprintf(buf, "Hunting victim: %s (%s)\n\r",
                IS_NPC(victim->hunting) ? victim->hunting->short_descr
                                        : victim->hunting->name,
                IS_NPC(victim->hunting) ? "MOB" : "PLAYER");
	send_to_char(buf, ch);
    }

    sprintf(buf, "{BFighting:{x %s\n\r",
	victim->fighting ? victim->fighting->name : "(none)");
    send_to_char(buf, ch);

    if (!IS_NPC(victim))
    {
	sprintf(buf,
	    "{BThirst:{x %d  {BHunger:{x %d  {BFull:{x %d  {BDrunk:{x %d\n\r",
	    victim->pcdata->condition[COND_THIRST],
	    victim->pcdata->condition[COND_HUNGER],
	    victim->pcdata->condition[COND_FULL],
	    victim->pcdata->condition[COND_DRUNK]);
	send_to_char(buf, ch);
    }

    sprintf(buf, "{BCarry number:{x %d  {BCarry weight:{x %ld\n\r",
	victim->carry_number, get_carry_weight(victim) / 10);
    send_to_char(buf, ch);

    if (!IS_NPC(victim))
    {
    	sprintf(buf,
	    "{BAge:{x %d  {BPlayed:{x %d  {BTimer:{x %d  {BCreated:{B %s{x",
	    get_age(victim),
	    (int) (victim->played + current_time - victim->logon) / 3600,
	    victim->timer,
	    ((char *) ctime((time_t *)&victim->pcdata->creation_date)));	/* @@@NIB : 20070123 : Moved the pc id to the creation date*/
    	send_to_char(buf, ch);
    }

    sprintf(buf, "{BAct :{x %s\n\r",act_bit_name((IS_NPC(victim) ? 1 : 3), victim->act));
    send_to_char(buf,ch);
    sprintf(buf, "{BAct2:{x %s\n\r",act_bit_name((IS_NPC(victim) ? 2 : 4), victim->act2));
    send_to_char(buf,ch);

    if (victim->comm)
    {
    	sprintf(buf,"{BComm:{x %s\n\r",comm_bit_name(victim->comm));
    	send_to_char(buf,ch);
    }

    if (IS_NPC(victim) && victim->off_flags)
    {
    	sprintf(buf, "{BOffense:{x %s\n\r",off_bit_name(victim->off_flags));
	send_to_char(buf,ch);
    }

    if (victim->imm_flags)
    {
	sprintf(buf, "{BImmune:{x %s\n\r",imm_bit_name(victim->imm_flags));
	send_to_char(buf,ch);
    }

    if (victim->res_flags)
    {
	sprintf(buf, "{BResist:{x %s\n\r", imm_bit_name(victim->res_flags));
	send_to_char(buf,ch);
    }

    if (victim->vuln_flags)
    {
	sprintf(buf, "{BVulnerable:{x %s\n\r", imm_bit_name(victim->vuln_flags));
	send_to_char(buf,ch);
    }

    if (victim->affected_by)
    {
	sprintf(buf, "{BAffected by{x %s\n\r",
	    affect_bit_name(victim->affected_by));
	send_to_char(buf,ch);
    }

    if (victim->affected_by2)
    {
	sprintf(buf, "{BAffected2 by{x %s\n\r",
	    affect2_bit_name(victim->affected_by2));
	send_to_char(buf,ch);
    }

    sprintf(buf, "{BMaster:{x %s  {BLeader:{x %s  {BPet:{x %s  {B%s:{x %s  {BCart:{x %s\n\r",
	victim->master      ? victim->master->name   : "(none)",
	victim->leader      ? victim->leader->name   : "(none)",
	victim->pet 	    ? victim->pet->name	     : "(none)",
	(victim->rider?"Rider":"Mount"),
	(victim->rider?victim->rider->name:(victim->mount?victim->mount->name : "(none)")),
	victim->pulled_cart ? victim->pulled_cart->short_descr : "(none)");

    send_to_char(buf, ch);

    if (!IS_NPC(victim))
    {
	sprintf(buf, "{BSecurity:{x %d.\n\r", victim->pcdata->security);
	send_to_char(buf, ch);
    }

    if (IS_NPC(victim)) {
	sprintf(buf, "{BShort description:{x %s\n\r{BLong description:{x %s",
		victim->short_descr,
		victim->long_descr[0] != '\0' ? victim->long_descr : "(none)\n\r");
	send_to_char(buf, ch);
    }

    if (!IS_NPC(victim))
    {
	sprintf(buf, "{BSubclasses: Mage:{x %s {BCleric:{x %s {BThief:{x %s {BWarrior:{x %s\n\r",
	    victim->pcdata->sub_class_mage < 0 ? "none" :
	        sub_class_table[victim->pcdata->sub_class_mage].name[victim->sex],
	    victim->pcdata->sub_class_cleric < 0 ? "none" :
	        sub_class_table[victim->pcdata->sub_class_cleric].name[victim->sex],
	    victim->pcdata->sub_class_thief < 0 ? "none" :
	        sub_class_table[victim->pcdata->sub_class_thief].name[victim->sex],
	    victim->pcdata->sub_class_warrior < 0 ? "none" :
	        sub_class_table[victim->pcdata->sub_class_warrior].name[victim->sex]);
	send_to_char(buf, ch);
	if (IS_REMORT(victim))
	{
	    sprintf(buf, "{BRemort Subclasses: Mage:{x %s {BCleric:{x %s {BThief:{x %s {BWarrior:{x %s\n\r",
		    victim->pcdata->second_sub_class_mage < 0 ? "none" :
		    sub_class_table[victim->pcdata->second_sub_class_mage].name[victim->sex],
		    victim->pcdata->second_sub_class_cleric < 0 ? "none" :
		    sub_class_table[victim->pcdata->second_sub_class_cleric].name[victim->sex],
		    victim->pcdata->second_sub_class_thief < 0 ? "none" :
		    sub_class_table[victim->pcdata->second_sub_class_thief].name[victim->sex],
		    victim->pcdata->second_sub_class_warrior < 0 ? "none" :
		    sub_class_table[victim->pcdata->second_sub_class_warrior].name[victim->sex]);
	    send_to_char(buf, ch);
	}
    }

    for (paf = victim->affected; paf != NULL; paf = paf->next) if(!paf->custom_name)
    {
	sprintf(buf,
	    "{C* {BLevel {W%3d {Baffect {x%-20.20s{B modifies {x%-12s{B by {x%2d{B for {x%2d{B hours with bits {x%s{B on slot {x%s\n\r",
	    paf->level,
	    skill_table[(int) paf->type].name,
	    affect_loc_name(paf->location),
	    paf->modifier,
	    paf->duration,
	    affects_bit_name(paf->bitvector, paf->bitvector2),
	    flag_string(wear_loc_names, paf->slot));
	send_to_char(buf, ch);
    }

    for (paf = victim->affected; paf != NULL; paf = paf->next) if(paf->custom_name)
    {
	sprintf(buf,
	    "{C* {BLevel {W%3d {Baffect {x%-20.20s{B modifies {x%-12s{B by {x%2d{B for {x%2d{B hours with bits {x%s{B on slot {x%s\n\r",
	    paf->level,
	    paf->custom_name,
	    affect_loc_name(paf->location),
	    paf->modifier,
	    paf->duration,
	    affects_bit_name(paf->bitvector, paf->bitvector2),
	    flag_string(wear_loc_names, paf->slot));
	send_to_char(buf, ch);
    }

    if (!IS_NPC(victim) && victim->pcdata->commands != NULL) {
	COMMAND_DATA *cmd;
	int i;

	send_to_char("{BGranted commands:{x\n\r", ch);

	i = 0;
	for (cmd = victim->pcdata->commands; cmd != NULL; cmd = cmd->next) {
	    i++;
	    sprintf(buf, "%-15s", cmd->name);
	    if (i % 4 == 0)
		strcat(buf, "\n\r");

	    send_to_char(buf, ch);
	}

    }

    for (ev = victim->events; ev != NULL; ev = ev->next_event) {
	sprintf(buf, "{M* {BEvent {x%-53.52s {B[{x%7.3f{B seconds{B]{x\n\r", ev->args, (float) ev->delay/2);
	send_to_char(buf, ch);
    }
}


void do_tstat(CHAR_DATA *ch, char *argument)
{
    char arg[MSL], buf[MSL], buf2[MSL], arg2[MSL], arg3[MSL];
    TOKEN_DATA *token = NULL;
    CHAR_DATA *victim;
    int i;
    long vnum = 0, count;

    BUFFER *buffer;

    argument = one_argument(argument, arg);
    argument = one_argument(argument, arg2);

    if (arg[0] == '\0') {
	send_to_char("Syntax:  stat token <character> [token vnum]\n\r", ch);
	return;
    }

    if ((victim = get_char_world(NULL, arg)) == NULL) {
	send_to_char("Character not found.\n\r", ch);
	return;
    }

	count = number_argument(arg2, arg3);
    if (arg3[0] != '\0') {
		vnum = atol(arg3);

		if (get_token_index(vnum) == NULL) {
			send_to_char("That token vnum does not exist.\n\r", ch);
			return;
		}

		if ((token = get_token_char(victim, vnum, count)) == NULL) {
			act("$N doesn't have that token.", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
			return;
		}
    }

    buffer = new_buf();

    sprintf(buf, "{Y%-9s %-20s %-6s ", "Vnum", "Token Name", "Timer");
    add_buf(buffer, buf);

    for (i = 0; i < MAX_TOKEN_VALUES; i++) {
		sprintf(buf2, "Value%d", i);
		sprintf(buf, "%-20s", buf2);
		add_buf(buffer, buf);
    }

    add_buf(buffer, "{x\n\r");

    if (token == NULL) {
	if (victim->tokens == NULL)
	    add_buf(buffer, "None.\n\r");
	else
	{
	    for (token = victim->tokens; token != NULL; token = token->next) {
		buf[0] = '\0';
		sprintf(buf2, "{Y[{x%7ld{Y]{x %-20.20s %-6d ",
			token->pIndexData->vnum, token->name, token->timer);

		strcat(buf, buf2);
		for (i = 0; i < MAX_TOKEN_VALUES; i++) {
		    sprintf(buf2, "{b[{x%-7.7s{b]{x %-9ld ", token_index_getvaluename(token->pIndexData, i), token->value[i]);
		    strcat(buf, buf2);
		}

		strcat(buf, "\n\r");
		add_buf(buffer, buf);
	    }
	}
    }
    else
    {
	buf[0] = '\0';
	sprintf(buf2, "{Y[{x%7ld{Y]{x %-20.20s %-6d ",
		token->pIndexData->vnum, token->name, token->timer);

	strcat(buf, buf2);
	for (i = 0; i < MAX_TOKEN_VALUES; i++) {
	    sprintf(buf2, "{b[{x%-7.7s{b]{x %-9ld ", token_index_getvaluename(token->pIndexData, i), token->value[i]);
	    strcat(buf, buf2);
	}

	strcat(buf, "\n\r");

	add_buf(buffer, buf);
    }

    page_to_char(buf_string(buffer), ch);
    free_buf(buffer);
}


void do_vnum(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char *string;

    string = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  vnum obj <name>\n\r",ch);
	send_to_char("  vnum mob <name>\n\r",ch);
	send_to_char("  vnum skill <skill or spell>\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"obj"))
    {
	do_function(ch, &do_ofind, string);
 	return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    {
	do_function(ch, &do_mfind, string);
	return;
    }

    /*
    if (!str_cmp(arg,"skill") || !str_cmp(arg,"spell"))
    {
	do_function (ch, &do_slookup, string);
	return;
    }
    */
    /* do both */
    do_function(ch, &do_mfind, argument);
    do_function(ch, &do_ofind, argument);
}


void do_mfind(CHAR_DATA *ch, char *argument)
{
    /* extern long top_mob_index; */
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    /* long vnum; */
    int nMatch, iHash;
    bool fAll;
    bool found;

    one_argument(argument, arg);
    if (arg[0] == '\0')
    {
	send_to_char("Find whom?\n\r", ch);
	return;
    }

    if (strlen(arg) < 2) {
	send_to_char("Your search must be at least 2 characters long.\n\r", ch);
	return;
    }

    fAll	= FALSE; /* !str_cmp(arg, "all"); */
    found	= FALSE;
    nMatch	= 0;

	for (iHash = 0; iHash < MAX_KEY_HASH; iHash++) {
		for (pMobIndex = mob_index_hash[iHash]; pMobIndex != NULL; pMobIndex = pMobIndex->next) {
			nMatch++;
			if (fAll || is_name(argument, pMobIndex->player_name)) {
				found = TRUE;
				sprintf(buf, "[%5ld] %s\n\r",
					pMobIndex->vnum, pMobIndex->short_descr);
				send_to_char(buf, ch);
			}
		}
	}

    if (!found)
	send_to_char("No mobiles by that name.\n\r", ch);
}


void do_ofind(CHAR_DATA *ch, char *argument)
{
    /* extern long top_obj_index; */
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    /* long vnum; */
    int nMatch, iHash;
    bool fAll;
    bool found;

    one_argument(argument, arg);
    if (arg[0] == '\0')
    {
	send_to_char("Find what?\n\r", ch);
	return;
    }

    fAll	= FALSE; /* !str_cmp(arg, "all"); */
    found	= FALSE;
    nMatch	= 0;

	for (iHash = 0; iHash < MAX_KEY_HASH; iHash++) {
		for (pObjIndex = obj_index_hash[iHash]; pObjIndex != NULL; pObjIndex = pObjIndex->next) {
			nMatch++;
			if (fAll || is_name(argument, pObjIndex->name)) {
				found = TRUE;
				sprintf(buf, "[%5ld] %s\n\r",
					pObjIndex->vnum, pObjIndex->short_descr);
				send_to_char(buf, ch);
			}
		}
	}

    if (!found)
	send_to_char("No objects by that name.\n\r", ch);
}


void do_rwhere(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_INPUT_LENGTH];
    BUFFER *buffer;
    ROOM_INDEX_DATA *room;
    bool found;
    int number, max_found;
    int hash;

    found = FALSE;
    number = 0;
    max_found = 200;

    buffer = new_buf();

    if (argument[0] == '\0')
    {
	send_to_char("Find what?\n\r",ch);
	return;
    }

    for (hash = 0; hash < MAX_KEY_HASH; hash++)
    {
	for (room = room_index_hash[hash]; room != NULL; room = room->next)
	{
	    if (can_see_room(ch, room)
	    &&  is_name(argument, room->name))
	    {
		if (number >= max_found)
		    break;

		number++;
		found = TRUE;
		sprintf(buf, "{Y%3d){x %s (vnum {W%ld{x)\n\r", number,
		    room->name, room->vnum);
		buf[0] = UPPER(buf[0]);
		add_buf(buffer,buf);
	    }
	}

	if (number >= max_found)
	    break;
    }

    if (!found)
        send_to_char("Nothing like that in heaven or earth.\n\r", ch);
    else
        page_to_char(buf_string(buffer),ch);

    free_buf(buffer);
}


void do_owhere(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_INPUT_LENGTH];
    BUFFER *buffer;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int number = 0, max_found;
    ITERATOR it;

    found = FALSE;
    number = 0;
    max_found = 200;

    buffer = new_buf();

    if (argument[0] == '\0')
    {
	send_to_char("Find what?\n\r",ch);
	return;
    }

	iterator_start(&it, loaded_objects);
	while(( obj = (OBJ_DATA *)iterator_nextdata(&it)))
    {
        if (!can_see_obj(ch, obj) || !is_name(argument, obj->name))
            continue;

        found = TRUE;
        number++;

        for (in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj) ;

        if (in_obj->carried_by != NULL &&
        	can_see(ch,in_obj->carried_by) &&
        	in_obj->carried_by->in_room != NULL) {
            sprintf(buf, "{Y%3d){x %s (vnum %ld) is carried by %s [Room %ld]\n\r",
                number, obj->short_descr,
			obj->pIndexData->vnum,
			pers(in_obj->carried_by, ch),
			in_obj->carried_by->in_room->vnum);
		} else if (in_obj->in_room != NULL && can_see_room(ch,in_obj->in_room)) {
            sprintf(buf, "{Y%3d){x %s (vnum %ld) is in %s [Room %ld]\n\r",
                number, obj->short_descr,
				obj->pIndexData->vnum,
				in_obj->in_room->name,
				in_obj->in_room->vnum);
		} else if (in_obj->in_mail != NULL) {
            sprintf(buf, "{Y%3d){x %s (vnum %ld) is in a mail package\n\r", number,
			    obj->short_descr, obj->pIndexData->vnum);
		} else {
            sprintf(buf, "{Y%3d){x %s (vnum %ld) is somewhere\n\r", number,
			    obj->short_descr, obj->pIndexData->vnum);
		}

        buf[0] = UPPER(buf[0]);
        add_buf(buffer,buf);

        if (number >= max_found)
            break;
    }

    if (!found)
        send_to_char("Nothing like that in heaven or earth.\n\r", ch);
    else
        page_to_char(buf_string(buffer),ch);

    free_buf(buffer);
}


void do_mwhere(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    BUFFER *buffer;
    CHAR_DATA *victim;
    bool found;
    int count = 0;
    ITERATOR vit;

    if (argument[0] == '\0') {
		DESCRIPTOR_DATA *d;

		buffer = new_buf();
		for (d = descriptor_list; d != NULL; d = d->next) {
		    if (d->character != NULL && d->connected == CON_PLAYING &&
		    	d->character->in_room != NULL && can_see(ch,d->character) &&
		    	can_see_room(ch,d->character->in_room)) {
                if (d->character->in_wilds == NULL) {
                    /* Victim is in a normal room, so report the vnum.*/
                    victim = d->character;
                    count++;

					if (d->original != NULL)
						sprintf(buf,"{Y%3d){x %s (in the body of %s) is in %s [%ld]\n\r",
							count, d->original->name,victim->short_descr,
							victim->in_room->name,victim->in_room->vnum);
					else
						sprintf(buf,"{Y%3d){x %s is in %s [%ld]\n\r",
							count, victim->name,victim->in_room->name,
							victim->in_room->vnum);
					add_buf(buffer,buf);
			    } else {
                    /* Victim is in a virtual room, so report the location and position.*/
                    victim = d->character;
                    count++;

                    if (d->original != NULL)
                        sprintf(buf,"{Y%3d){x %s (in the body of %s) is in wilds '%s', %s (%ld, %ld)\n\r",
							count, d->original->name,victim->short_descr,
							victim->in_room->name,
							victim->in_wilds->name, victim->in_room->x, victim->in_room->y);
                    else
						sprintf(buf,"{Y%3d){x %s is in wilds '%s', %s (%ld, %ld)\n\r",
							count, victim->name,
							victim->in_wilds->name,
							victim->in_room->name,
							victim->in_room->x, victim->in_room->y);

					add_buf(buffer,buf);
				}
			}
		}

		page_to_char(buf_string(buffer),ch);
		free_buf(buffer);
		return;
    }

    /* all the mobs without a room */
    if (!str_cmp(argument,"nowhere")) {
        buffer = new_buf();
        found=FALSE;
        count=0;

		iterator_start(&vit, loaded_chars);
		while(( victim = (CHAR_DATA *)iterator_nextdata(&vit)))
		{
            if (victim->in_room==NULL) {
                found = TRUE;
                count++;
                sprintf(buf, "{Y%3d){x [%5ld] %-28s %lx\n\r", count,
                    IS_NPC(victim) ? victim->pIndexData->vnum : 0,
                    IS_NPC(victim) ? victim->short_descr : victim->name,
                    (long)victim);
                add_buf(buffer,buf);
            }
		}
		iterator_stop(&vit);

        if (found)
            page_to_char(buf_string(buffer),ch);
        else
            send_to_char("No mobs without rooms found.\n\r",ch);
        free_buf(buffer);
        return;
    }

    /* ok - must be a mobname */
    found = FALSE;
    buffer = new_buf();

	iterator_start(&vit, loaded_chars);
	while(( victim = (CHAR_DATA *)iterator_nextdata(&vit)))
	{
		if (victim->in_room != NULL &&
			is_name(argument, victim->name)) {
			found = TRUE;
			count++;
			sprintf(buf, "{Y%3d){x [%5ld] %-28s [%5ld] %s\n\r", count,
			IS_NPC(victim) ? victim->pIndexData->vnum : 0,
			IS_NPC(victim) ? victim->short_descr : victim->name,
			victim->in_room->vnum,
			victim->in_room->name);
			add_buf(buffer,buf);
		}
	}
	iterator_stop(&vit);

    if (!found)
		act("You didn't find any $T.", ch, NULL, NULL, NULL, NULL, NULL, argument, TO_CHAR);
    else
    	page_to_char(buf_string(buffer),ch);

    free_buf(buffer);
}


void do_reboo(CHAR_DATA *ch, char *argument)
{
    send_to_char("If you want to REBOOT, spell it out.\n\r", ch);
}


void do_reckonin(CHAR_DATA *ch, char *argument)
{
    send_to_char("This command cannot be abbreviated!\n\r", ch);
}


void do_reboot(CHAR_DATA *ch, char *argument)
{
    int mins;
    int down_time;
    char buf[MSL];
    char arg[MSL];
    char arg2[MSL];
    struct tm *reboot_time;

    argument = one_argument(argument, arg);
    argument = one_argument(argument, arg2);

    if (reboot_timer > 0)
    {
	reboot_timer = 0;
	down_timer = 0;
	free_string(reboot_by);
	gecho("{WREBOOT COUNTDOWN DEACTIVATED.{x\n\r");
	return;
    }

    if (arg[0] == '\0')
    {
	send_to_char("Reboot in how many minutes?\n\r", ch);
	return;
    }

    if (arg2[0] == '\0')
    {
	send_to_char("What is the downtime?\n\r", ch);
	return;
    }

    if (!is_number(arg) || !is_number(arg2))
    {
        send_to_char("Invalid argument given.\n\r", ch);
	return;
    }

    mins = atoi(arg);
    if (mins < 1 || mins > 30)
    {
	send_to_char("Range for reboot time is 1 to 30 minutes.\n\r", ch);
	return;
    }

    down_time = atoi(arg2);
    if (down_time < 1 || down_time > 30)
    {
	send_to_char("Range for downtime is 1 to 30 minutes.\n\r", ch);
	return;
    }

    sprintf(buf, "{WSet reboot timer for %d minutes.{x\n\r", mins);
    send_to_char(buf, ch);

    reboot_time = localtime(&current_time);
    reboot_time->tm_min += mins;

    reboot_timer = mktime(reboot_time);
    down_timer = down_time;
    reboot_by = str_dup(ch->name);
}


void do_shutdow(CHAR_DATA *ch, char *argument)
{
    send_to_char("If you want to SHUTDOWN, spell it out.\n\r", ch);
}


void do_shutdown(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d,*d_next;
    CHAR_DATA *vch, *tch;
    TOKEN_DATA *token;
    ITERATOR cit, tit;

    if( IS_NULLSTR(argument) )
    	sprintf(buf, "Shutdown by %s.", ch->name);
    else
    {
		sprintf(buf, "Shutdown by %s, Reason: %s", ch->name, argument);
		append_file(ch, MAINTENANCE_FILE, buf);
	}
    append_file(ch, SHUTDOWN_FILE, buf);

    strcat(buf, "\n\r");
    do_function(ch, &do_echo, buf);

    /* remove any PURGE_REBOOT tokens on any characters */
	iterator_start(&cit, loaded_chars);
	while(( tch = (CHAR_DATA *)iterator_nextdata(&cit)))
	{
		iterator_start(&tit, tch->ltokens);
		while(( token = (TOKEN_DATA *)iterator_nextdata(&tit)))
		{
			if (IS_SET(token->flags, TOKEN_PURGE_REBOOT)) {
				sprintf(buf, "char update: token %s(%ld) char %s(%ld) was purged because of reboot",
					token->name, token->pIndexData->vnum, HANDLE(tch), IS_NPC(tch) ? tch->pIndexData->vnum : 0);
				log_string(buf);
				token_from_char(token);
				free_token(token);
			}
		}

		iterator_stop(&tit);
	}
	iterator_stop(&cit);

    merc_down = TRUE;
    for (d = descriptor_list; d != NULL; d = d_next) {
		d_next = d->next;
		vch = d->original ? d->original : d->character;
		if (vch != NULL) {
		    /* save their shift */
		    if (ch->shifted != SHIFTED_NONE) {
				shift_char(ch, TRUE);
				ch->shifted = IS_VAMPIRE(ch) ? SHIFTED_WEREWOLF : SHIFTED_SLAYER;
		    }

		    save_char_obj(vch);
		}

		close_socket(d);
    }

    write_churches_new();
    save_helpfiles_new();
    write_mail();
    write_chat_rooms();
    write_gq();
    write_permanent_objs();
    persist_save();
    save_projects();
}


void do_snoop(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    one_argument(argument, arg);

    if (arg[0] == '\0')
    {
	send_to_char("Snoop whom?\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (victim->desc == NULL)
    {
	send_to_char("No descriptor to snoop.\n\r", ch);
	return;
    }

    if (victim == ch)
    {
	send_to_char("Cancelling all snoops.\n\r", ch);
	wiznet("$N stops being such a snoop.",
		ch,NULL,WIZ_SNOOPS,WIZ_SECURE,get_trust(ch));
	for (d = descriptor_list; d != NULL; d = d->next)
	{
	    if (d->snoop_by == ch->desc)
		d->snoop_by = NULL;
	}
	return;
    }

    if (victim->desc->snoop_by != NULL)
    {
	send_to_char("Busy already.\n\r", ch);
	return;
    }

    if (!is_room_owner(ch,victim->in_room) && ch->in_room != victim->in_room
    &&  room_is_private(victim->in_room, ch))
    {
        send_to_char("That character is in a private room.\n\r",ch);
        return;
    }

    if (get_trust(victim) >= get_trust(ch))
    {
	send_to_char("You failed.\n\r", ch);
	return;
    }

    if (ch->desc != NULL)
    {
	for (d = ch->desc->snoop_by; d != NULL; d = d->snoop_by)
	{
	    if (d->character == victim || d->original == victim)
	    {
		send_to_char("No snoop loops.\n\r", ch);
		return;
	    }
	}
    }

    victim->desc->snoop_by = ch->desc;
    sprintf(buf,"$N starts snooping on %s",
	(IS_NPC(ch) ? victim->short_descr : victim->name));
    wiznet(buf,ch,NULL,WIZ_SNOOPS,WIZ_SECURE,get_trust(ch));
    act("Now snooping $N.", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
}


void do_switch(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument(argument, arg);

    if (arg[0] == '\0')
    {
	send_to_char("Switch into whom?\n\r", ch);
	return;
    }

    if (ch->desc == NULL)
	return;

    if (ch->desc->original != NULL)
    {
	send_to_char("You are already switched.\n\r", ch);
	return;
    }

    if( ch->desc->editor != ED_NONE )
    {
	send_to_char("You are currently in OLC.  Please exit before switching.\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (victim == ch)
    {
	send_to_char("That would be pointless.\n\r", ch);
	return;
    }

    if (!IS_NPC(victim))
    {
	send_to_char("You can only switch into mobiles.\n\r",ch);
	return;
    }

    if (!is_room_owner(ch,victim->in_room) && ch->in_room != victim->in_room
    &&  room_is_private(victim->in_room, ch))
    {
	send_to_char("That character is in a private room.\n\r",ch);
	return;
    }

    if (victim->desc != NULL)
    {
	send_to_char("Character in use.\n\r", ch);
	return;
    }

    sprintf(buf,"$N switches into %s",victim->short_descr);
    wiznet(buf,ch,NULL,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch));

    ch->desc->character = victim;
    ch->desc->original  = ch;
    victim->desc        = ch->desc;
    ch->desc            = NULL;
    /* change communications to match */
    if (ch->prompt != NULL)
        victim->prompt = str_dup(ch->prompt);
    victim->comm = ch->comm;
    victim->lines = ch->lines;
    act("Switched into $n.", victim, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
    SET_BIT(victim->act, PLR_COLOUR);
}


void do_return(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    if (ch->desc == NULL)
    {
        log_string("act_wiz.c, do_return: ch->desc is NULL.");
        return;
    }

    if (ch->desc->original == NULL)
    {
	send_to_char("You aren't switched.\n\r", ch);
	return;
    }

    send_to_char(
    "{RYou return to your original body. Type replay to see any missed tells.\n\r{x",
	ch);

    if (ch->prompt != NULL)
    {
	free_string(ch->prompt);
	ch->prompt = NULL;
    }

    if (IS_IMMORTAL(ch))
    {
        sprintf(buf,"$N returns from %s.",ch->short_descr);
        wiznet(buf,ch->desc->original,0,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch->desc->original));
    }

    REMOVE_BIT(ch->act, PLR_COLOUR);

    ch->desc->character       = ch->desc->original;
    ch->desc->original        = NULL;
    ch->desc->character->desc = ch->desc;
    ch->desc                  = NULL;
}


/* for clone, to insure that cloning goes many levels deep */
void recursive_clone(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *clone)
{
    OBJ_DATA *c_obj, *t_obj;

    for (c_obj = obj->contains; c_obj != NULL; c_obj = c_obj->next_content)
    {
	t_obj = create_object(c_obj->pIndexData,0, TRUE);
	clone_object(c_obj,t_obj);
	obj_to_obj(t_obj,clone);
	recursive_clone(ch,c_obj,t_obj);
    }
}


/* command that is similar to load */
void do_clone(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char *rest;
    CHAR_DATA *mob;
    OBJ_DATA  *obj;

    rest = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Clone what?\n\r",ch);
	return;
    }

    if (!str_prefix(arg,"object"))
    {
	mob = NULL;
	obj = get_obj_here(ch,NULL,rest);
	if (obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
    {
	obj = NULL;
	mob = get_char_room(ch,NULL, rest);
	if (mob == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else /* find both */
    {
	mob = get_char_room(ch,NULL, argument);
	obj = get_obj_here(ch,NULL,argument);
	if (mob == NULL && obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }

    /* clone an object */
    if (obj != NULL)
    {
	OBJ_DATA *clone;

	clone = create_object(obj->pIndexData,0, TRUE);
	clone_object(obj,clone);
	if (obj->carried_by != NULL)
	    obj_to_char(clone,ch);
	else
	    obj_to_room(clone,ch->in_room);
 	recursive_clone(ch,obj,clone);

	act("$n has created $p.",ch, NULL, NULL,clone,NULL, NULL, NULL,TO_ROOM);
	act("You clone $p.",ch, NULL, NULL,clone, NULL, NULL, NULL,TO_CHAR);
	wiznet("$N clones $p.",ch,clone,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
	return;
    }
    else if (mob != NULL)
    {
	CHAR_DATA *clone;
	OBJ_DATA *new_obj;
	char buf[MAX_STRING_LENGTH];

	if (!IS_NPC(mob))
	{
	    send_to_char("You can only clone mobiles.\n\r",ch);
	    return;
	}

	clone = clone_mobile(mob);

	for (obj = mob->carrying; obj != NULL; obj = obj->next_content)
	{
		new_obj = create_object(obj->pIndexData,0, TRUE);
		clone_object(obj,new_obj);
		recursive_clone(ch,obj,new_obj);
		obj_to_char(new_obj,clone);
		new_obj->wear_loc = obj->wear_loc;
	}
	char_to_room(clone,ch->in_room);
        act("$n has created $N.",ch,clone, NULL, NULL, NULL, NULL, NULL,TO_ROOM);
        act("You clone $N.",ch,clone, NULL, NULL, NULL, NULL, NULL,TO_CHAR);
	sprintf(buf,"$N clones %s.",clone->short_descr);
	wiznet(buf,ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
        return;
    }
}


void do_load(CHAR_DATA *ch, char *argument)
{
   char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  load mob <vnum>\n\r",ch);
	send_to_char("  load obj <vnum> <amt>\n\r",ch);
	/*send_to_char("  load ship <vnum> <room vnum>\n\r",ch);*/
	return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    {
	do_function(ch, &do_mload, argument);
	return;
    }

    if (!str_cmp(arg,"obj"))
    {
	do_function(ch, &do_oload, argument);
	return;
    }

/*    if (!str_cmp(arg, "ship"))
    {
	do_function(ch, &do_sload, argument);
	return;
    }*/

    /* echo syntax */
    do_function(ch, &do_load, "");
}


void do_mload(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *victim;

    one_argument(argument, arg);

    if (arg[0] == '\0' || !is_number(arg))
    {
	send_to_char("Syntax: load mob <vnum>.\n\r", ch);
	return;
    }

    if ((pMobIndex = get_mob_index(atol(arg))) == NULL)
    {
	send_to_char("No mob has that vnum.\n\r", ch);
	return;
    }

    if (!IS_BUILDER(ch, pMobIndex->area))
    {
	send_to_char("You arn't a builder in that area.\n\r", ch);
	return;
    }

    sprintf(buf, "Loaded %s (%ld)",
        pMobIndex->short_descr,
        pMobIndex->vnum);
    act(buf, ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);

    victim = create_mobile(pMobIndex, FALSE);

    if (ch->in_wilds == NULL)
        char_to_room(victim, ch->in_room);
    else
        char_to_vroom(victim, ch->in_wilds, ch->at_wilds_x, ch->at_wilds_y);
    p_percent_trigger(victim, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, TRIG_REPOP, NULL);

    act("$n has created $N!", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
    sprintf(buf,"$N loads %s.",victim->short_descr);
    wiznet(buf,ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
}


void do_oload(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    int amt = 1;
    int i;

    argument = one_argument(argument, arg1);
    one_argument(argument, arg2);

    if (arg1[0] == '\0' || !is_number(arg1))
    {
	send_to_char("Syntax: load obj <vnum> <amt>.\n\r", ch);
	return;
    }

    if (arg2[0] != '\0')
    {
	if (!is_number(arg2))
	{
	    send_to_char("Syntax: oload <vnum> <amt>.\n\r", ch);
	    return;
	}

	amt = atoi(arg2);
        if (amt < 1 || amt > 50)
	{
	    send_to_char("Range for amount is 1-50.\n\r",ch);
	    return;
	}
    }

    if ((pObjIndex = get_obj_index(atol(arg1))) == NULL)
    {
	send_to_char("No object has that vnum.\n\r", ch);
	return;
    }

    if (!has_access_area(ch, pObjIndex->area))
    {
	send_to_char("Insufficient security to load object - action logged.\n\r", ch);
	sprintf(buf, "do_oload: %s tried to load %s (vnum %ld) in area %s without permissions!",
	    ch->name,
	    pObjIndex->short_descr,
	    pObjIndex->vnum,
	    pObjIndex->area->name);
	log_string(buf);
	return;
    }

    if (amt == 1)
    {
	obj = create_object(pObjIndex, pObjIndex->level, TRUE);
	if (CAN_WEAR(obj, ITEM_TAKE))
	    obj_to_char(obj, ch);
	else
            if (ch->in_room->wilds == NULL)
            {
                plogf("act_wiz.c, do_oload(): Moving object to static room.");
	        obj_to_room(obj, ch->in_room);
            }
            else
            {
                plogf("act_wiz.c, do_oload(): Moving object to vroom.");
                obj_to_vroom(obj, ch->in_room->wilds, ch->at_wilds_x, ch->at_wilds_y);
            }

	act("$n has created $p!", ch, NULL, NULL, obj, NULL, NULL, NULL, TO_ROOM);
	sprintf(buf, "Loaded $p (%ld)", obj->pIndexData->vnum);
	act(buf, ch, NULL, NULL, obj, NULL, NULL, NULL, TO_CHAR);
	wiznet("$N loads $p.",ch,obj,WIZ_LOAD,WIZ_SECURE,get_trust(ch));

	obj->loaded_by = str_dup(ch->name);

	p_percent_trigger(NULL, obj, NULL, NULL, NULL, NULL, NULL, NULL, NULL, TRIG_REPOP, NULL);
    }
    else
    {
	for (i = 0; i < amt; i++)
	{
	    obj = create_object(pObjIndex, pObjIndex->level, TRUE);
	    if (CAN_WEAR(obj, ITEM_TAKE))
		obj_to_char(obj, ch);
	    else
		obj_to_room(obj, ch->in_room);
	    obj->loaded_by = str_dup(ch->name);

	    p_percent_trigger(NULL, obj, NULL, NULL, NULL, NULL, NULL, NULL, NULL, TRIG_REPOP, NULL);
	}

	sprintf(buf, "{Y({G%d{Y){x $n has created %s!", amt,
	    pObjIndex->short_descr);
	act(buf, ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
	sprintf(buf, "{Y({G%d{Y){x Loaded %s (%ld)",
	    amt, pObjIndex->short_descr, pObjIndex->vnum);
	act(buf, ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
	sprintf(buf, "{Y({G%d{Y){x $N loads %s.", amt, pObjIndex->short_descr);
	wiznet(buf, ch, NULL, WIZ_LOAD, WIZ_SECURE, get_trust(ch));
    }
}


void do_purge(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    argument = one_argument(argument, arg);
    argument = one_argument(argument, arg2);

    if (arg[0] == '\0')
    {
	CHAR_DATA *vnext;
	OBJ_DATA  *obj_next;

	for (victim = ch->in_room->people; victim != NULL; victim = vnext)
	{
	    vnext = victim->next_in_room;
	    if (IS_NPC(victim)
	    && !IS_SET(victim->act,ACT_NOPURGE)
	    && victim != ch /* safety precaution */
	    && victim != ch->rider
	    && victim != ch->mount) {
		extract_char(victim, TRUE);
	    }
	}

	for (obj = ch->in_room->contents; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;

	    if (obj->item_type == ITEM_CART) {
		    if(obj->pulled_by) {
			    obj->pulled_by->pulled_cart = NULL;
			    obj->pulled_by = NULL;
		    }
	    }

	    extract_obj(obj);
	}

	act("$n purges the room!", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
	act("Purged $T.", ch, NULL, NULL, NULL, NULL, NULL, ch->in_room->name, TO_CHAR);
	return;
    }

    if ((victim = get_char_room(ch, NULL, arg)) != NULL)
    {
	if (!IS_NPC(victim))
	{
	    send_to_char("You can't purge a player character.\n\r", ch);
	    return;
	}

	act("Extracted $N.", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
	extract_char(victim, TRUE);
	return;
    }
    else
    if ((obj = get_obj_list(ch, arg, ch->in_room->contents)) != NULL
    &&     !IS_SET(obj->extra_flags, ITEM_NOPURGE))
    {
	if (obj->item_type == ITEM_CART)
	{
		    if(obj->pulled_by) {
			    obj->pulled_by->pulled_cart = NULL;
			    obj->pulled_by = NULL;
		    }
	}

	act("Extracted $p.", ch, NULL, NULL, obj, NULL, NULL, NULL, TO_CHAR);
	extract_obj(obj);
	return;
    }
    else
    {
	act("Target not found.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
	return;
    }
}

/* Adding some new stuff to advance, for new immortals. It'll now display an intro screen to them. Perhaps the intro would be better as a helpfile, along the same lines as do_greeting? -- Areo 2006-08-23 */
void do_advance(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;
    int iLevel;
    int olevel;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (IS_NPC(ch)) {
        sprintf("do_advance: NPC %s(%ld) tried to advance", ch->pIndexData->short_descr, ch->pIndexData->vnum);
	log_string(buf);
	send_to_char("No.\n\r", ch);
	return;
    }

    if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number(arg2))
    {
	send_to_char("Syntax: advance <char> <level>.\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg1)) == NULL)
    {
	send_to_char("That player is not here.\n\r", ch);
	return;
    }

    if (IS_NPC(victim))
    {
	send_to_char("Not on NPC's.\n\r", ch);
	return;
    }

    if ((level = atoi(arg2)) < 1 || level > MAX_LEVEL)
    {
	sprintf(buf,"Level must be 1 to %d.\n\r", MAX_LEVEL);
	send_to_char(buf, ch);
	return;
    }

    if ((level > MAX_CLASS_LEVEL) && (level < LEVEL_IMMORTAL))
    {
	sprintf(buf,"Cannot advance to multiclass level range.\n\r");
	send_to_char(buf, ch);
	return;
    }

    if (level > get_trust(ch))
    {
	send_to_char("Limited to your trust level.\n\r", ch);
	return;
    }

    if (level == victim->level) return;

    /*
     * Lower level:
     *   Reset to level 1.
     *   Then raise again.
     *   Currently, an imp can lower another imp.
     *   -- Swiftest
     */
    if (level < victim->level)
    {
        int temp_prac;

	if(victim->pcdata->immortal) {
		IMMORTAL_DATA *immortal, *tmp, *last;

		immortal = victim->pcdata->immortal;

		if (victim->tot_level == MAX_LEVEL) {
			send_to_char("You may not delete implementors.\n\r", ch);
			return;
		}

		if(level < LEVEL_IMMORTAL) {
			act("$N has been deleted from the immortal list.", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
			/* Remove it from the global list */
			last = NULL;
			for (tmp = immortal_list; tmp != NULL; tmp = tmp->next) {
				if (tmp == immortal)
					break;

				last = tmp;
			}

			if (last != NULL)
				last->next = immortal->next;
			else
				immortal_list = immortal->next;
			free_immortal(immortal);
			victim->pcdata->immortal = NULL;
		}
	}

	send_to_char("Lowering a player's level!\n\r", ch);
	send_to_char("**** OOOOHHHHHHHHHH  NNNNOOOO ****\n\r", victim);
	temp_prac = victim->practice;
	victim->level    = 1;
	victim->tot_level    = 1;
	victim->exp      = 0; /*exp_per_level(victim,victim->pcdata->points);*/
	victim->max_hit  = 10;
	victim->max_mana = 100;
	victim->max_move = 100;
	victim->practice = 0;
	victim->hit      = victim->max_hit;
	victim->mana     = victim->max_mana;
	victim->move     = victim->max_move;
	advance_level(victim, TRUE);
	victim->practice = temp_prac;
    }
    else
    /* Only show this if we're not making them 150's.*/
    if (level != LEVEL_IMMORTAL)
    {
	send_to_char("Raising a player's level!\n\r", ch);
	send_to_char("**** OOOOHHHHHHHHHH  YYYYEEEESSS ****\n\r", victim);
    }

    if (level < LEVEL_IMMORTAL)
    for (iLevel = victim->level ; iLevel < level; iLevel++)
    {
	victim->level += 1;
	victim->tot_level += 1;
	advance_level(victim,TRUE);
    }
    else
    {
	/* Here's the big one. We'll take the victim's old level, and if they were below 149 (and advanced to 150+, we'll show them a nifty intro screen with some basic instructions. After that, we'll set them as wizi to their new level, give them a basic imm_flag, turn holylight on, and let them know what we've done. This should cut down on needed explanations, if only slightly. -- Areo */

	/* SYN -- add IMMORTAL_DATA here!! */

	olevel = victim->tot_level;
	if (olevel < (LEVEL_IMMORTAL - 1) && level >= LEVEL_IMMORTAL)
	{
		IMMORTAL_DATA *immortal = new_immortal();

		immortal->name = str_dup(victim->name);
		immortal->imm_flag = str_dup("{R  Immortal  {x");
		immortal->created = current_time;

		/* start them off as unassigned */
		immortal->next = immortal_list;
		immortal_list = immortal;

		victim->pcdata->immortal = immortal;

		send_to_char("{B================================================================================{x\n\r", victim);
		send_to_char("{B|{C****************************{WWelcome, new Immortal!{C****************************{B|{x\n\r",victim);
		send_to_char("{B================================================================================{x\n\r", victim);
		sprintf(buf,"\n\rWelcome to the Sentience Immortal Staff. Please read {WHELP IMMORTAL RULES{x now. In addition to this, please type wizhelp to see a full list of your available immortal commands. You may use '{Rimmtalk <message>{X' or '{R: <message>{X' to communicate on the immortal channel. {WHELP %d{x will list available helpfiles for your level.\n\r\n\r", level);
		send_to_char(buf,victim);
	victim->invis_level = level;
	do_function(victim, &do_holylight, "");
	sprintf(buf, "\n\rYou have been set to wizinvis level {W%d{x.\n\r", victim->invis_level);
	send_to_char(buf,victim);
	}
	victim->level = level;
	victim->tot_level = level;
    }


    if ((victim->level > MAX_CLASS_LEVEL) &&
	 (level < LEVEL_IMMORTAL))
    { /* set level to hero */
	victim->level = 31;
    }

	/* Again, only display if the victim is not a newly minted 150.*/
	if (level != LEVEL_IMMORTAL)
	{
    sprintf(buf,"You are now level %d.\n\r",victim->level);
    send_to_char(buf,victim);
	}
    victim->exp   = 0;/*exp_per_level(victim,victim->pcdata->points)*/
		  /** UMAX(1, victim->level);*/
    victim->trust = 0;
    save_char_obj(victim);
}


void do_trust(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int level;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number(arg2))
    {
	send_to_char("Syntax: trust <char> <level>.\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg1)) == NULL)
    {
	send_to_char("That player is not here.\n\r", ch);
	return;
    }

    if ((level = atoi(arg2)) < 0 || level > MAX_LEVEL)
    {
	sprintf(buf, "Level must be 0 (reset) or 1 to %d.\n\r",MAX_LEVEL);
	send_to_char(buf, ch);
	return;
    }

    if (level > get_trust(ch))
    {
	send_to_char("Limited to your trust.\n\r", ch);
	return;
    }

    victim->trust = level;
}


void do_restore(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *vch;
    DESCRIPTOR_DATA *d;

    one_argument(argument, arg);
    if (arg[0] == '\0' || !str_cmp(arg,"room"))
    {
    /* cure room */

        for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
        {
			restore_char(vch, ch, 100);
        }


        sprintf(buf, "$N restored room %ld.", ch->in_room->vnum);
        wiznet(buf, ch, NULL, WIZ_RESTORE, WIZ_SECURE, get_trust(ch));

        send_to_char("Room restored.\n\r",ch);
        return;
    }

    /* restore all */
    if ((ch->tot_level >= MAX_LEVEL - 2) && !str_cmp(arg,"all"))
    {
        for (d = descriptor_list; d != NULL; d = d->next)
        {

		    victim = d->character;

		    if (victim == NULL || IS_NPC(victim) || IS_IMMORTAL(victim))
				continue;
			restore_char(victim, ch, 100);
        }

		send_to_char("All active players restored.\n\r",ch);
		return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL)
    {
		send_to_char("They aren't here.\n\r", ch);
		return;
    }

	restore_char(victim, ch, 100);
    act("Restored $N.", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR);

    sprintf(buf, "$N restored %s.", IS_NPC(victim) ? victim->short_descr : victim->name);
    wiznet(buf,ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch));


}


void do_freeze(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument(argument, arg);

    if (arg[0] == '\0')
    {
	send_to_char("Freeze whom?\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (IS_NPC(victim))
    {
	send_to_char("Not on NPC's.\n\r", ch);
	return;
    }

    if (get_trust(victim) >= get_trust(ch))
    {
	send_to_char("You failed.\n\r", ch);
	return;
    }

    if (IS_SET(victim->act, PLR_FREEZE))
    {
	REMOVE_BIT(victim->act, PLR_FREEZE);
	send_to_char("You can play again.\n\r", victim);
	send_to_char("FREEZE removed.\n\r", ch);
	sprintf(buf,"$N thaws %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->act, PLR_FREEZE);
	send_to_char("You can't do ANYthing!\n\r", victim);
	send_to_char("FREEZE set.\n\r", ch);
	sprintf(buf,"$N puts %s in the deep freeze.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    save_char_obj(victim);
}


void do_log(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument(argument, arg);

    /* This command is strict */
    if (ch->tot_level < MAX_LEVEL)
    {
	send_to_char("Huh?\n\r", ch);
	return;
    }

    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r", ch);
	send_to_char("  log <character>\n\r", ch);
	send_to_char("  log all\n\r", ch);
	return;
    }

    if (!str_cmp(arg, "all"))
    {
	if (logAll)
	{
	    logAll = FALSE;
	    send_to_char("Log ALL off.\n\r", ch);
	}
	else
	{
	    logAll = TRUE;
	    send_to_char("Log ALL on.\n\r", ch);
	}
	return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (IS_NPC(victim))
    {
	send_to_char("Not on NPC's.\n\r", ch);
	return;
    }

    /*
     * No level check, gods can log anyone.
     */
    if (IS_SET(victim->act, PLR_LOG))
    {
	REMOVE_BIT(victim->act, PLR_LOG);
	send_to_char("LOG removed.\n\r", ch);
    }
    else
    {
	SET_BIT(victim->act, PLR_LOG);
	send_to_char("LOG set.\n\r", ch);
    }
}


void do_notell(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument(argument, arg);

    if (arg[0] == '\0')
    {
	send_to_char("Notell whom?\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (get_trust(victim) >= get_trust(ch))
    {
	send_to_char("You failed.\n\r", ch);
	return;
    }

    if (IS_SET(victim->comm, COMM_NOTELL))
    {
	REMOVE_BIT(victim->comm, COMM_NOTELL);
	send_to_char("You can tell again.\n\r", victim);
	send_to_char("NOTELL removed.\n\r", ch);
	sprintf(buf,"$N restores tells to %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOTELL);
	send_to_char("You can't tell!\n\r", victim);
	send_to_char("NOTELL set.\n\r", ch);
	sprintf(buf,"$N revokes %s's tells.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
}


void do_peace(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *rch;

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
	if (rch->fighting != NULL)
	    stop_fighting(rch, TRUE);
	if (IS_NPC(rch) && IS_SET(rch->act,ACT_AGGRESSIVE))
	    REMOVE_BIT(rch->act,ACT_AGGRESSIVE);
    }

    send_to_char("Done.\n\r", ch);
}


void do_wizlock(CHAR_DATA *ch, char *argument)
{
    wizlock = !wizlock;

    if (wizlock)
    {
	wiznet("$N has wizlocked the game.",ch,NULL,0,0,0);
	send_to_char("Game wizlocked.\n\r", ch);
    }
    else
    {
	wiznet("$N removes wizlock.",ch,NULL,0,0,0);
	send_to_char("Game un-wizlocked.\n\r", ch);
    }

	gconfig_write();

}


void do_newlock(CHAR_DATA *ch, char *argument)
{
    newlock = !newlock;

    if (newlock)
    {
	wiznet("$N locks out new characters.",ch,NULL,0,0,0);
        send_to_char("New characters have been locked out.\n\r", ch);
    }
    else
    {
	wiznet("$N allows new characters back in.",ch,NULL,0,0,0);
        send_to_char("Newlock removed.\n\r", ch);
    }

	gconfig_write();
}

void do_testport(CHAR_DATA *ch, char *argument)
{
    is_test_port = !is_test_port;

    if (is_test_port)
    {
		wiznet("$N enables Test Port Mode.",ch,NULL,0,0,0);
		send_to_char("Test Port Mode enabled.\n\r", ch);
    }
    else
    {
		wiznet("$N disables Test Port Mode.",ch,NULL,0,0,0);
		send_to_char("Test Port Mode disabled.\n\r", ch);
    }
    gconfig_write();
}

/*
void do_slookup(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int sn;

    one_argument(argument, arg);
    if (arg[0] == '\0')
    {
	send_to_char("Lookup which skill or spell?\n\r", ch);
	return;
    }

    if (!str_cmp(arg, "all"))
    {
	for (sn = 0; sn < MAX_SKILL; sn++)
	{
	    if (skill_table[sn].name == NULL)
		break;
	    sprintf(buf, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
		sn, skill_table[sn].slot, skill_table[sn].name);
	    send_to_char(buf, ch);
	}
    }
    else
    {
	if ((sn = skill_lookup(arg)) < 0)
	{
	    send_to_char("No such skill or spell.\n\r", ch);
	    return;
	}

	sprintf(buf, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
	    sn, skill_table[sn].slot, skill_table[sn].name);
	send_to_char(buf, ch);
    }
}
*/

void do_set(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MSL];

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set char  <name> <field> <value>\n\r",ch);
	send_to_char("  set obj   <name> <field> <value>\n\r",ch);
	send_to_char("  set room  <room> <field> <value>\n\r",ch);
	send_to_char("  set church <no.> <field> <value>\n\r",ch);
        send_to_char("  set skill <name> <spell or skill> <value>\n\r",ch);
	send_to_char("  set sky   <cloudless|cloudy|rainy|stormy>\n\r", ch);
	send_to_char("  set time  <hour|day|month|year> <#>\n\r", ch);
	send_to_char("  set token <char name> <token vnum> <v#|timer> <op> <value>\n\r", ch);
	return;
    }

    if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
    {
	do_function(ch, &do_mset, argument);
	return;
    }

    if (!str_prefix(arg,"skill") || !str_prefix(arg,"spell"))
    {
	do_function(ch, &do_sset, argument);
	return;
    }

    if (!str_prefix(arg,"object"))
    {
	do_function(ch, &do_oset, argument);
	return;
    }

    if (!str_prefix(arg,"room"))
    {
	do_function(ch, &do_rset, argument);
	return;
    }

    if (!str_prefix(arg,"church"))
    {
	do_function(ch, &do_chset, argument);
	return;
    }

    if (!str_prefix(arg, "sky"))
    {
	if (argument[0] == '\0')
	{
	    send_to_char("Syntax: set sky <cloudless|cloudy|rainy|stormy>\n\r", ch);
	    return;
	}

	if (!str_cmp(argument, "cloudless"))
	    weather_info.sky = SKY_CLOUDLESS;
	else if (!str_cmp(argument, "cloudy"))
	    weather_info.sky = SKY_CLOUDY;
	else if (!str_cmp(argument, "rainy"))
	    weather_info.sky = SKY_RAINING;
	else if (!str_cmp(argument, "stormy"))
	    weather_info.sky = SKY_LIGHTNING;
	else
	{
	    send_to_char("Invalid argument.\n\r", ch);
	    return;
	}

	sprintf(buf, "Set sky condition to %s.\n\r", argument);
	send_to_char(buf, ch);

	return;
    }

    if (!str_prefix(arg, "time"))
    {
	do_function(ch, &do_tset, argument);
	return;
    }

    if (!str_prefix(arg, "token"))
    {
	do_function(ch, &do_tkset, argument);
	return;
    }

    /* echo syntax */
    do_function(ch, &do_set, "");
}


/* set token <char name> <token vnum> <v#|timer> <operator> <value> */
void do_tkset(CHAR_DATA *ch, char *argument)
{
    char arg[MSL];
    char arg2[MSL], arg2b[MSL];
    char arg3[MSL];
    char arg4[MSL];
    char arg5[MSL];
    char buf[MSL];
    CHAR_DATA *victim;
    TOKEN_DATA *token;
    long vnum, value, count;
    int value_num;

    argument = one_argument(argument, arg);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);
    argument = one_argument(argument, arg4);
    argument = one_argument(argument, arg5);

    if (arg[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' || arg4[0] == '\0' || arg5[0] == '\0') {
		send_to_char("Syntax:\n\r  set token <char name> <token vnum> <v#|timer> <op> <value>\n\r", ch);
		return;
    }

    if ((victim = get_char_world(NULL, arg)) == NULL) {
		send_to_char("Character not found.\n\r", ch);
		return;
    }

	count = number_argument(arg2,arg2b);
    vnum = atol(arg2b);

    if ((token = get_token_char(victim, vnum, count)) == NULL) {
		send_to_char("Character doesn't have that token vnum.\n\r", ch);
		return;
    }

    if (!str_cmp(arg3, "timer"))
		value_num = -1;
    else if (is_number(arg3))
		value_num = atoi(arg3);
    else {
		send_to_char("Invalid value argument.\n\r", ch );
		return;
    }

    if (value_num < -1 || value_num >= MAX_TOKEN_VALUES) {
		send_to_char("Invalid value number.\n\r", ch);
		return;
    }

    value = atol(arg5);
    /*
    if (value < -2000000000 || value > 2000000000) {
		send_to_char("Value out of range.\n\r", ch);
		return;
    }
    */

    if (value_num == -1)
    {
	switch (arg4[0])
	{
	    case '+':
		token->timer += value;
		break;

	    case '-':
		token->timer -= value;
		break;

	    case '*':
		token->timer *= value;
		break;

	    case '/':
		if (value == 0) {
		    bug("do_tkset: adjust called with operator / and value 0", 0);
		    return;
		}
		token->timer /= value;
		break;

	    case '%':
		if (value == 0) {
		    bug("do_tkset: adjust called with operator %% and value 0", 0);
		    return;
		}
		token->timer %= value;
		break;

	    case '=':
		token->timer = value;
		break;

	    default:
		sprintf(buf, "do_tkset: bad operator %c", arg5[0]);
		bug(buf, 0);
	}

	sprintf(buf, "Adjusted token %s(%ld.%ld) on char %s, timer %c %ld\n\r",
	    token->name, count, token->pIndexData->vnum, HANDLE(victim),
	    arg4[0], value);
	send_to_char(buf, ch);
    }
    else
    {
	switch (arg4[0])
	{
	    case '+':
		token->value[value_num] += value;
		break;

	    case '-':
		token->value[value_num] -= value;
		break;

	    case '*':
		token->value[value_num] *= value;
		break;

	    case '/':
		if (value == 0) {
		    bug("do_tkset: adjust called with operator / and value 0", 0);
		    return;
		}
		token->value[value_num] /= value;
		break;

	    case '%':
		if (value == 0) {
		    bug("do_tkset: adjust called with operator % and value 0", 0);
		    return;
		}
		token->value[value_num] %= value;
		break;

	    case '=':
		token->value[value_num] = value;
		break;

	    default:
		sprintf(buf, "do_tkset: bad operator %c", arg5[0]);
		bug(buf, 0);
	}

	sprintf(buf, "Adjusted token %s(%ld.%ld) on char %s, value %s %c %ld\n\r",
	    token->name, count, token->pIndexData->vnum, HANDLE(victim),
	    token->pIndexData->value_name[value_num], arg4[0], value);
	send_to_char(buf, ch);
    }

    sprintf(buf, "stat token %s %ld", victim->name, vnum);
    interpret(ch, buf);
}

void set_moon_phase(void)
{
	int hours;

	hours = ((((time_info.year*12)+time_info.month)*35+time_info.day)*24+time_info.hour+MOON_OFFSET) % MOON_PERIOD;
	hours = (hours + MOON_PERIOD) % MOON_PERIOD;

	if(hours <= (MOON_CARDINAL_HALF)) time_info.moon = MOON_NEW;
	else if(hours < (MOON_CARDINAL_STEP - MOON_CARDINAL_HALF)) time_info.moon = MOON_WAXING_CRESCENT;
	else if(hours <= (MOON_CARDINAL_STEP + MOON_CARDINAL_HALF)) time_info.moon = MOON_FIRST_QUARTER;
	else if(hours < (2*MOON_CARDINAL_STEP - MOON_CARDINAL_HALF)) time_info.moon = MOON_WAXING_GIBBOUS;
	else if(hours <= (2*MOON_CARDINAL_STEP + MOON_CARDINAL_HALF)) time_info.moon = MOON_FULL;
	else if(hours < (3*MOON_CARDINAL_STEP - MOON_CARDINAL_HALF)) time_info.moon = MOON_WANING_GIBBOUS;
	else if(hours <= (3*MOON_CARDINAL_STEP + MOON_CARDINAL_HALF)) time_info.moon = MOON_LAST_QUARTER;
	else if(hours < (4*MOON_CARDINAL_STEP - MOON_CARDINAL_HALF)) time_info.moon = MOON_WANING_CRESCENT;
	else time_info.moon = MOON_NEW;
}

void do_tset(CHAR_DATA *ch, char *argument)
{
    char arg[MSL];
    char arg2[MSL];
    int value;

    argument = one_argument(argument, arg);
    argument = one_argument(argument, arg2);

    if (arg[0] == '\0' || arg2[0] == '\0'
    || (str_cmp(arg, "hour") && str_cmp(arg, "day") && str_cmp(arg, "month") && str_cmp(arg, "year")))
    {
	send_to_char("Syntax:\n\r", ch);
	send_to_char("  set time hour <0-23>\n\r", ch);
	send_to_char("  set time day <0-34>\n\r", ch);
	send_to_char("  set time month <0-11>\n\r", ch);
	send_to_char("  set time year <#>\n\r", ch);
	return;
    }

    if (!is_number(arg2))
    {
	send_to_char("Argument must be numeric.\n\r", ch);
	return;
    }

    value = atoi(arg2);
    if (!str_cmp(arg, "hour"))
    {
	if (value < 0 || value > 23)
	{
	    send_to_char("Invalid. Range is 0-23 hours.\n\r", ch);
	    return;
	}

        send_to_char("Time set.\n\r", ch);
        time_info.hour = value;
        set_moon_phase();
	return;
    }

    if (!str_cmp(arg, "day"))
    {
	if (value < 0 || value > 34)
	{
	    send_to_char("Invalid. Range is 0-34 days.\n\r", ch);
	    return;
	}

	send_to_char("Day set.\n\r", ch);
        set_moon_phase();
	time_info.day = value;
	return;
    }

    if (!str_cmp(arg, "month"))
    {
	if (value < 0 || value > 11)
	{
	    send_to_char("Invalid. Range is 0-11 months.\n\r", ch);
	    return;
	}

	send_to_char("Month set.\n\r", ch);
	time_info.month = value;
        set_moon_phase();
	return;
    }

    if (!str_cmp(arg, "year"))
    {
	if (value < 0 || value > 25000)
	{
	    send_to_char("Invalid. Range is 0-25000.\n\r", ch);
	    return;
	}

	send_to_char("Year set.\n\r", ch);
	time_info.year = value;
        set_moon_phase();
	return;
    }
}


void do_sset(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char buf[MSL];
    CHAR_DATA *victim;
    int value;
    int sn;
    bool fAll;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);

    if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set skill <name> <spell or skill> <value>\n\r", ch);
	send_to_char("  set skill <name> all <value>\n\r",ch);
	send_to_char("   (use the name of the skill, not the number)\n\r",ch);
	return;
    }

    if ((victim = get_char_world(ch, arg1)) == NULL)
    {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (IS_NPC(victim))
    {
	send_to_char("Not on NPC's.\n\r", ch);
	return;
    }

    fAll = !str_cmp(arg2, "all");

    sn   = 0;
    if (!fAll && (sn = skill_lookup(arg2)) < 0)
    {
	send_to_char("No such skill or spell.\n\r", ch);
	return;
    }

    /*
     * Get the value.
     */
    if (!is_number(arg3))
    {
	send_to_char("Value must be numeric.\n\r", ch);
	return;
    }

    value = atoi(arg3);
    if (value < 0 || value > 100)
    {
	send_to_char("Value range is 0 to 100.\n\r", ch);
	return;
    }

    if (fAll)
    {
		for (sn = 0; sn < MAX_SKILL; sn++)
		{
			if (skill_table[sn].name != NULL && str_cmp(skill_table[sn].name, "none")) {
				if( value == 0 ) {
					if( skill_table[sn].spell_fun == spell_null )
						skill_entry_removeskill(victim,sn, NULL);
					else
						skill_entry_removespell(victim,sn, NULL);
				} else if( skill_entry_findsn( ch->sorted_skills, sn) == NULL) {
					if( skill_table[sn].spell_fun == spell_null ) {
						skill_entry_addskill(ch, sn, NULL, SKILLSRC_NORMAL, SKILL_AUTOMATIC);
					} else {
						skill_entry_addspell(ch, sn, NULL, SKILLSRC_NORMAL, SKILL_AUTOMATIC);
					}
				}
			}
			victim->pcdata->learned[sn]	= value;
		}
    }
    else {
		if( value == 0 ) {
			if( skill_table[sn].spell_fun == spell_null )
				skill_entry_removeskill(victim,sn, NULL);
			else
				skill_entry_removespell(victim,sn, NULL);
		} else if( skill_entry_findsn( ch->sorted_skills, sn) == NULL) {
			if( skill_table[sn].spell_fun == spell_null ) {
				skill_entry_addskill(ch, sn, NULL, SKILLSRC_NORMAL, SKILL_AUTOMATIC);
			} else {
				skill_entry_addspell(ch, sn, NULL, SKILLSRC_NORMAL, SKILL_AUTOMATIC);
			}
		}
		victim->pcdata->learned[sn] = value;
	}

    if (!fAll)
	sprintf(buf, "Set %s's %s skill to %d%%\n\r", victim->name, skill_table[sn].name, value);
    else
	sprintf(buf, "Set all of %s's skills to %d%%\n\r", victim->name, value);

    send_to_char(buf, ch);
}


void do_chset(CHAR_DATA *ch, char *argument)
{
    CHURCH_DATA *church;
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

    argument = one_argument_norm(argument, arg );
    argument = one_argument_norm(argument, arg2);
    argument = one_argument_norm(argument, arg3);

    if (arg[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
    {
	send_to_char("Set church <no.> <field> <value>\n\r", ch);
	send_to_char("Fields: name founder pneuma dp gold\n\r", ch);
	send_to_char("        max size align recall treasure\n\r", ch);
	send_to_char("        flag key\n\r", ch);
	return;
    }

    if (!is_number(arg))
    {
	send_to_char("That's not even a number.\n\r", ch);
	return;
    }

    if ((church = find_church(atoi(arg))) == NULL)
    {
	send_to_char("No such church found.\n\r", ch);
	return;
    }

    if (!str_cmp(arg2, "name"))
    {
	CHURCH_PLAYER_DATA *member;

	sprintf(buf, "%s is now known as %s.\n\r", church->name,
		arg3);
	send_to_char(buf, ch);

	sprintf(buf, "{Y[%s will now be known as %s!]{x\n\r",
		church->name, arg3);
	msg_church_members(church, buf);

	free_string(church->name);
	church->name = str_dup(arg3);

	/* Any players with this church should be modified*/
	for (member = church->people; member != NULL; member = member->next)
	{
	    if (member->ch != NULL)
	    {
		free_string(ch->church_name);
		ch->church_name = str_dup(capitalize(arg3));
	    }
	}
	return;
    }

    if (!str_cmp(arg2, "flag"))
    {
	sprintf(buf, "%s's flag is now %s.\n\r", church->name,
		arg3);
	send_to_char(buf, ch);

	free_string(church->flag);
	church->flag = str_dup(arg3);
	return;
    }

    if (!str_cmp(arg2, "founder"))
    {
	sprintf(buf, "%s is now the founder of %s.\n\r",
		capitalize(arg3), church->name);
	send_to_char(buf, ch);

	free_string(church->founder);
	church->founder = str_dup(capitalize(arg3));
	return;
    }

    if (!str_cmp(arg2, "dp"))
    {
	if (!is_number(arg3))
	{
	    send_to_char("Invalid argument.\n\r", ch);
	    return;
	}

	sprintf(buf, "%s now has %ld dp.\n\r", church->name, atol(arg3));
	send_to_char(buf, ch);

	church->dp = atol(arg3);
	return;
    }

    if (!str_cmp(arg2, "pneuma"))
    {
	if (!is_number(arg3))
	{
	    send_to_char("Invalid argument.\n\r", ch);
	    return;
	}

	sprintf(buf, "%s now has %ld pneuma.\n\r", church->name,
		atol(arg3));
	send_to_char(buf, ch);

	church->pneuma = atol(arg3);
	return;
    }

    if (!str_cmp(arg2, "gold"))
    {
	if (!is_number(arg3))
	{
	    send_to_char("Invalid argument.\n\r", ch);
	    return;
	}

	sprintf(buf, "%s now has %ld gold.\n\r", church->name,
		atol(arg3));
	send_to_char(buf, ch);

	church->gold = atol(arg3);
	return;
    }

    if (!str_cmp(arg2, "max"))
    {
	if (!is_number(arg3))
	{
	    send_to_char("Invalid argument.\n\r", ch);
	    return;
	}

	sprintf(buf, "Set max positions in %s to %d.\n\r",
		church->name, atoi(arg3));
	send_to_char(buf, ch);

	church->max_positions = atoi(arg3);
	return;
    }

    if (!str_cmp(arg2, "size"))
    {
	if (!is_number(arg3))
	{
	    send_to_char("Invalid argument.\n\r", ch);
	    return;
	}

	if (atoi(arg3) < 1 || atoi(arg3) > 4)
	{
	    send_to_char("Invalid argument.\n\r", ch);
	    return;
	}

	church->size = atoi(arg3);
	sprintf(buf, "Set size of %s to %s.\n\r", church->name,
		get_chsize_from_number(church->size));
	send_to_char(buf, ch);

	return;
    }

    if (!str_prefix(arg2, "alignment"))
    {
	if (!str_cmp(arg3, "evil"))
	    church->alignment = CHURCH_EVIL;
	else if (!str_cmp(arg3, "good"))
	    church->alignment = CHURCH_GOOD;
	else if (!str_cmp(arg3, "neutral"))
	    church->alignment = CHURCH_NEUTRAL;
	else
	{
	    send_to_char("Invalid argument. Choose good, evil, or neutral.\n\r", ch);
	    return;
	}

	arg3[0] = UPPER(arg3[0]);

	sprintf(buf, "Set alignment of %s to %s.\n\r", church->name, arg3);
	send_to_char(buf, ch);
	return;
    }

    if (!str_cmp(arg2, "recall"))
    {
	if (get_room_index(atol(arg3)) == NULL)
	{
	    send_to_char("That room doesn't exist.\n\r", ch);
	    return;
	}

	sprintf(buf,
	    "You have set %s's temple recall point to %ld - %s.\n\r",
	    church->name,
	    atol(arg3),
	    get_room_index(atol(arg3))->name);
	send_to_char(buf, ch);
	church->recall_point.id[0] = atol(arg3);
	church->recall_point.id[1] = church->recall_point.id[2] = 0;
	church->recall_point.wuid = 0;
	return;
    }

    if (!str_cmp(arg2, "key"))
    {
	if (get_obj_index(atol(arg3)) == NULL)
	{
	    send_to_char("That object doesn't exist.\n\r", ch);
	    return;
	}

	sprintf(buf,
	    "You have set %s's key to %ld - %s.\n\r",
	    church->name,
	    atol(arg3),
	    get_obj_index(atol(arg3))->short_descr);
	send_to_char(buf, ch);
	church->key = atol(arg3);
	return;
    }
/*
    if (!str_cmp(arg2, "treasure"))
    {
	if (get_room_index(atol(arg3)) == NULL)
	{
	    send_to_char("Room number is not valid.\n\r", ch);
	    return;
	}

	church->treasure_room = atoi(arg3);
	sprintf(buf, "%s treasure room set to %ld - %s\n\r",
	    church->name, church->treasure_room,
	    get_room_index(church->treasure_room)->name);
	send_to_char(buf, ch);
	return;
    }*/
}


void do_mset(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *victim;
    int value;

    smash_tilde(argument);
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    strcpy(arg3, argument);

    if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set char <name> <field> <value>\n\r",ch);
	send_to_char("  Field being one of:\n\r",			ch);
	send_to_char("    str int wis dex con sex\n\r",	ch);
	send_to_char("    race gold silver hp mana move prac\n\r",ch);
	send_to_char("    align train thirst hunger drunk\n\r",	ch);
	send_to_char("    security pneuma dp qp title\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg1)) == NULL)
    {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = is_number(arg3) ? atoi(arg3) : -1;

    /*
     * Set something.
     */
    if (!str_cmp(arg2, "str"))
    {
	if (value < 3 || value > get_max_train(victim,STAT_STR))
	{
	    sprintf(buf,
		"Strength range is 3 to %d\n\r.",
		get_max_train(victim,STAT_STR));
	    send_to_char(buf,ch);
	    return;
	}

	set_perm_stat(victim, STAT_STR, value);
	return;
    }

    if (!str_cmp(arg2, "security"))	/* OLC */
    {
		int security = UMAX(9, ch->pcdata->security);
        if (IS_NPC(victim))
        {
            send_to_char("Not on NPC's.\n\r", ch);
            return;
        }



	if (value > security || value < 0) {
	    if (security > 0) {
			sprintf(buf, "Valid security is 0-%d.\n\r", security);
			send_to_char(buf, ch);
	    } else
			send_to_char("Valid security is 0 only.\n\r", ch);
	    return;
	}
	victim->pcdata->security = value;
	return;
    }

    if (!str_cmp(arg2, "qp"))
    {
	if (IS_NPC(victim))
	{
	    send_to_char("Can't set that on an NPC.\n\r", ch);
	    return;
	}

	if (arg3[0] == '\0')
	{
	    send_to_char("Set how much?\n\r", ch);
	    return;
	}

	if (atoi(arg3) < 0 || atoi(arg3) > 30000)
	{
	    send_to_char("Sorry, that's out of range.\n\r", ch);
	    return;
	}

	victim->questpoints = atoi(arg3);
	return;
    }

    if (!str_cmp(arg2, "pneuma"))
    {
	if (IS_NPC(victim))
	{
	    send_to_char("Can't set that on an NPC.\n\r", ch);
	    return;
	}

	if (arg3[0] == '\0')
	{
	    send_to_char("Set how much?\n\r", ch);
	    return;
	}

	if (atoi(arg3) < 0 || atoi(arg3) > 30000)
	{
	    send_to_char("Sorry, that's out of range.\n\r", ch);
	    return;
	}

	victim->pneuma = atoi(arg3);
	return;
    }

    if (!str_cmp(arg2, "dp"))
    {
	if (IS_NPC(victim))
	{
	    send_to_char("Can't set that on an NPC.\n\r", ch);
	    return;
	}

	if (arg3[0] == '\0')
	{
	    send_to_char("Set how much?\n\r", ch);
	    return;
	}

	if (atoi(arg3) < 0 || atoi(arg3) > 4300000)
	{
	    send_to_char("Sorry, that's out of range.\n\r", ch);
	    return;
	}

	victim->deitypoints = atoi(arg3);
	return;
    }

    /* SYN - now redundant
    if (!str_cmp(arg2, "imm_title"))
    {
	if (IS_NPC(victim))
	{
	    send_to_char("Can't set that on an NPC.\n\r", ch);
	    return;
	}

	if (victim->tot_level < LEVEL_IMMORTAL)
	{
	    send_to_char("Imm title is for imms only!\n\r", ch);
	    return;
	}

	if (arg3[0] == '\0')
	{
	    send_to_char("You must specify a title!\n\r", ch);
	    return;
	}

	if (strlen(arg3) > 45)
	{
	    send_to_char("That title is too long.\n\r", ch);
	    return;
	}

	free_string(victim->pcdata->imm_title);
	victim->pcdata->imm_title = str_dup(arg3);
	return;
    }
    */

    if (!str_cmp(arg2, "title"))
    {
	if (IS_NPC(victim))
	{
	    send_to_char("Can't set that on an NPC.\n\r", ch);
	    return;
	}

	if (arg3[0] == '\0')
	{
	    send_to_char("You must specify a title!\n\r", ch);
	    return;
	}

	if (strlen(arg3) > 91)
	{
	    send_to_char("That title is too long.\n\r", ch);
	    return;
	}

	free_string(victim->pcdata->title);
	victim->pcdata->title = str_dup(arg3);
	send_to_char("Title set.\n\r", ch);
	return;
    }

    if (!str_cmp(arg2, "int"))
    {
        if (value < 3 || value > get_max_train(victim,STAT_INT))
        {
            sprintf(buf,
		"Intelligence range is 3 to %d.\n\r",
		get_max_train(victim,STAT_INT));
            send_to_char(buf,ch);
            return;
        }

		set_perm_stat(victim, STAT_INT, value);
        return;
    }

    if (!str_cmp(arg2, "wis"))
    {
	if (value < 3 || value > get_max_train(victim,STAT_WIS))
	{
	    sprintf(buf,
		"Wisdom range is 3 to %d.\n\r",get_max_train(victim,STAT_WIS));
	    send_to_char(buf, ch);
	    return;
	}

	set_perm_stat(victim, STAT_WIS, value);
	return;
    }

    if (!str_cmp(arg2, "dex"))
    {
	if (value < 3 || value > get_max_train(victim,STAT_DEX))
	{
	    sprintf(buf,
		"Dexterity range is 3 to %d.\n\r",
		get_max_train(victim,STAT_DEX));
	    send_to_char(buf, ch);
	    return;
	}

	set_perm_stat(victim, STAT_DEX, value);
	return;
    }

    if (!str_cmp(arg2, "con"))
    {
	if (value < 3 || value > get_max_train(victim,STAT_CON))
	{
	    sprintf(buf,
		"Constitution range is 3 to %d.\n\r",
		get_max_train(victim,STAT_CON));
	    send_to_char(buf, ch);
	    return;
	}

	set_perm_stat(victim, STAT_CON, value);
	return;
    }

    if (!str_prefix(arg2, "sex"))
    {
	if (value < 0 || value > 2)
	{
	    send_to_char("Sex range is 0 to 2.\n\r", ch);
	    return;
	}
	victim->sex = value;
	if (!IS_NPC(victim))
	    victim->pcdata->true_sex = value;
	return;
    }

    if (!str_prefix(arg2, "class"))
    {
	int class;

	if (IS_NPC(victim))
	{
	    send_to_char("Mobiles have no class.\n\r",ch);
	    return;
	}

	class = class_lookup(arg3);
	if (class == -1)
	{
	    char buf[MAX_STRING_LENGTH];

        	strcpy(buf, "Possible classes are: ");
        	for (class = 0; class < MAX_CLASS; class++)
        	{
            	    if (class > 0)
                    	strcat(buf, " ");
            	    strcat(buf, class_table[class].name);
        	}
            strcat(buf, ".\n\r");

	    send_to_char(buf,ch);
	    return;
	}

	victim->pcdata->class_current = class;
	return;
    }

    if (!str_prefix(arg2, "level"))
    {
	if (!IS_NPC(victim))
	{
	    send_to_char("Not on PC's.\n\r", ch);
	    return;
	}

	if (value < 0 || value > MAX_LEVEL)
	{
	    sprintf(buf, "Level range is 0 to %d.\n\r", MAX_LEVEL);
	    send_to_char(buf, ch);
	    return;
	}
	victim->level = value;
	return;
    }

    if (!str_prefix(arg2, "gold"))
    {
	victim->gold = value;
	return;
    }

    if (!str_prefix(arg2, "silver"))
    {
	victim->silver = value;
	return;
    }

    if (!str_prefix(arg2, "hp"))
    {
	if (value < -10 || value > 30000)
	{
	    send_to_char("Hp range is -10 to 30,000 hit points.\n\r", ch);
	    return;
	}
	victim->max_hit = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_hit = value;
	return;
    }

    if (!str_prefix(arg2, "mana"))
    {
	if (value < 0 || value > 30000)
	{
	    send_to_char("Mana range is 0 to 30,000 mana points.\n\r", ch);
	    return;
	}
	victim->max_mana = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_mana = value;
	return;
    }

    if (!str_prefix(arg2, "move"))
    {
	if (value < 0 || value > 30000)
	{
	    send_to_char("Move range is 0 to 30,000 move points.\n\r", ch);
	    return;
	}
	victim->max_move = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_move = value;
	return;
    }

    if (!str_prefix(arg2, "practice"))
    {
	if (value < 0 || value > 250)
	{
	    send_to_char("Practice range is 0 to 250 sessions.\n\r", ch);
	    return;
	}
	victim->practice = value;
	return;
    }

    if (!str_prefix(arg2, "train"))
    {
	if (value < 0 || value > 50)
	{
	    send_to_char("Training session range is 0 to 50 sessions.\n\r",ch);
	    return;
	}
	victim->train = value;
	return;
    }

    if (!str_prefix("align", arg2))
    {
	if (value < -1000 || value > 1000)
	{
	    send_to_char("Alignment range is -1000 to 1000.\n\r", ch);
	    return;
	}
	victim->alignment = value;
	return;
    }

    if (!str_prefix(arg2, "thirst"))
    {
	if (IS_NPC(victim))
	{
	    send_to_char("Not on NPC's.\n\r", ch);
	    return;
	}

	if (value < -1 || value > 100)
	{
	    send_to_char("Thirst range is -1 to 100.\n\r", ch);
	    return;
	}

	victim->pcdata->condition[COND_THIRST] = value;
	return;
    }

    if (!str_prefix(arg2, "drunk"))
    {
	if (IS_NPC(victim))
	{
	    send_to_char("Not on NPC's.\n\r", ch);
	    return;
	}

	if (value < -1 || value > 100)
	{
	    send_to_char("Drunk range is -1 to 100.\n\r", ch);
	    return;
	}

	victim->pcdata->condition[COND_DRUNK] = value;
	return;
    }

    if (!str_prefix(arg2, "full"))
    {
	if (IS_NPC(victim))
	{
	    send_to_char("Not on NPC's.\n\r", ch);
	    return;
	}

	if (value < -1 || value > 100)
	{
	    send_to_char("Full range is -1 to 100.\n\r", ch);
	    return;
	}

	victim->pcdata->condition[COND_FULL] = value;
	return;
    }

    if (!str_prefix(arg2, "hunger"))
    {
        if (IS_NPC(victim))
        {
            send_to_char("Not on NPC's.\n\r", ch);
            return;
        }

        if (value < -1 || value > 100)
        {
            send_to_char("Full range is -1 to 100.\n\r", ch);
            return;
        }

        victim->pcdata->condition[COND_HUNGER] = value;
        return;
    }

    if (!str_cmp(arg2, "hunt"))
    {
        CHAR_DATA *hunted = 0;

        if (!IS_NPC(victim))
        {
            send_to_char("Not on PC's.\n\r", ch);
            return;
        }

        if (str_cmp(arg3, "."))
          if ((hunted = get_char_area(victim, arg3)) == NULL)
            {
              send_to_char("Mob couldn't locate the victim to hunt.\n\r", ch);
              return;
            }

        victim->hunting = hunted;
        return;
    }

    if (!str_prefix(arg2, "race"))
    {
	int race;

	race = race_lookup(arg3);

	if (race == 0)
	{
	    send_to_char("That is not a valid race.\n\r",ch);
	    return;
	}

	if (!IS_NPC(victim) && !race_table[race].pc_race)
	{
	    send_to_char("That is not a valid player race.\n\r",ch);
	    return;
	}

	victim->race = race;
	victim->affected_by_perm = race_table[victim->race].aff;
	victim->affected_by2_perm = race_table[victim->race].aff2;
    victim->imm_flags_perm = race_table[victim->race].imm;
    victim->res_flags_perm = race_table[victim->race].res;
    victim->vuln_flags_perm = race_table[victim->race].vuln;
    affect_fix_char(victim);

    victim->form        = race_table[victim->race].form;
    victim->parts       = race_table[victim->race].parts;
    victim->lostparts	= 0;

	return;
    }

    /* Syn -  unused
    if (!str_prefix(arg2,"group"))
    {
	if (!IS_NPC(victim))
	{
	    send_to_char("Only on NPCs.\n\r",ch);
	    return;
	}
	victim->group = value;
	return;
    }
    */

    /*
     * Generate usage message.
     */
    do_function(ch, &do_mset, "");
    return;
}


void do_string(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
//    char buf[MSL];
    OBJ_DATA *obj;

    smash_tilde(argument);
    argument = one_argument(argument, arg);
    argument = one_argument(argument, arg2);

    if (arg[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  string <object> <field> <string>\n\r",ch);
	send_to_char("  fields: name short long\n\r",ch);
	return;
    }

    if ((obj = get_obj_list(ch, arg, ch->carrying)) == NULL)
    {
	send_to_char("Nothing like that in your inventory.\n\r", ch);
	return;
    }

    if (!str_prefix(arg2, "name"))
    {
	free_string(obj->name);
	obj->name = str_dup(argument);
	act("Strung $p's name to '$t'.", ch, NULL, NULL, obj, NULL, argument, NULL, TO_CHAR);
	return;
    }

    if (!str_prefix(arg2, "short"))
    {
	free_string(obj->short_descr);
	obj->short_descr = str_dup(argument);
	act("Strung $p's short to '$t'.", ch, NULL, NULL, obj, NULL, argument, NULL, TO_CHAR);
	return;
    }

    if (!str_prefix(arg2, "long"))
    {
	free_string(obj->description);
	obj->description = str_dup(argument);
	act("Strung $p's long to '$t'.", ch, NULL, NULL, obj, NULL, argument, NULL, TO_CHAR);
	return;
    }

    do_function(ch, &do_string, "");
}


void do_oset(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int value;

    smash_tilde(argument);
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    strcpy(arg3, argument);

    if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set obj <object> <field> <value>\n\r",ch);
	send_to_char("  Field being one of:\n\r",				ch);
	send_to_char("    value0 value1 value2 value3 value4 (v1-v4)\n\r",	ch);
	send_to_char("    extra wear level weight cost timer\n\r",		ch);
	return;
    }

    if ((obj = get_obj_world(ch, arg1)) == NULL)
    {
	send_to_char("Nothing like that in heaven or earth.\n\r", ch);
	return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = atoi(arg3);

    /*
     * Set something.
     */
    if (!str_cmp(arg2, "value0") || !str_cmp(arg2, "v0"))
    {
	obj->value[0] = UMIN(50,value);
	return;
    }

    if (!str_cmp(arg2, "value1") || !str_cmp(arg2, "v1"))
    {
	obj->value[1] = value;
	return;
    }

    if (!str_cmp(arg2, "value2") || !str_cmp(arg2, "v2"))
    {
	obj->value[2] = value;
	return;
    }

    if (!str_cmp(arg2, "value3") || !str_cmp(arg2, "v3"))
    {
	obj->value[3] = value;
	return;
    }

    if (!str_cmp(arg2, "value4") || !str_cmp(arg2, "v4"))
    {
	obj->value[4] = value;
	return;
    }

    if (!str_prefix(arg2, "extra"))
    {
	obj->extra_flags = value;
	return;
    }

    if (!str_prefix(arg2, "wear"))
    {
	obj->wear_flags = value;
	return;
    }

    if (!str_prefix(arg2, "level"))
    {
	obj->level = value;
	return;
    }

    if (!str_prefix(arg2, "weight"))
    {
	obj->weight = value;
	return;
    }

    if (!str_prefix(arg2, "cost"))
    {
	obj->cost = value;
	return;
    }

    if (!str_prefix(arg2, "timer"))
    {
	obj->timer = value;
	return;
    }

    /*
     * Generate usage message.
     */
    do_function(ch, &do_oset, "");
    return;
}



void do_rset(CHAR_DATA *ch, char *argument)
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    int value;

    smash_tilde(argument);
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    strcpy(arg3, argument);

    if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set room <location> <field> <value>\n\r",ch);
	send_to_char("  Field being one of:\n\r",			ch);
	send_to_char("    flags sector\n\r",				ch);
	return;
    }

    if ((location = find_location(ch, arg1)) == NULL)
    {
	send_to_char("No such location.\n\r", ch);
	return;
    }

    if (!is_room_owner(ch,location) && ch->in_room != location
    &&  room_is_private(location, ch))
    {
        send_to_char("That room is private right now.\n\r",ch);
        return;
    }

    /*
     * Snarf the value.
     */
    if (!is_number(arg3))
    {
	send_to_char("Value must be numeric.\n\r", ch);
	return;
    }
    value = atoi(arg3);

    /*
     * Set something.
     */
    if (!str_prefix(arg2, "flags"))
    {
	location->room_flags	= value;
	return;
    }

    if (!str_prefix(arg2, "sector"))
    {
	location->sector_type	= value;
	return;
    }

    /*
     * Generate usage message.
     */
    do_function(ch, &do_rset, "");
    return;
}



/*void do_sockets(CHAR_DATA *ch, char *argument)
{
    char buf[2 * MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    int count;

    count = 0;
    buf[0] = '\0';

    one_argument(argument,arg);
    for (d = descriptor_list; d != NULL; d = d->next)
    {
	if (d->character != NULL && can_see(ch, d->character)
	&& (arg[0] == '\0' || is_name(arg,d->character->name)
			   || (d->original && is_name(arg,d->original->name))))
	{
	    count++;
	    if (d->character != NULL && (!str_cmp(d->character->name, "arlox")))
	      sprintf(buf + strlen(buf), "[%3d %2d] Arlox@136.17.156.20\n\r", d->descriptor, d->connected);
	    else
	    if (d->character != NULL && (!str_cmp(d->character->name, "zoron")))
	      sprintf(buf + strlen(buf), "[%3d %2d] Zoron@196.27.52.10\n\r", d->descriptor, d->connected);
	    else
	    sprintf(buf + strlen(buf), "[%3d %2d] %s@%s\n\r",
		d->descriptor,
		d->connected,
		d->original  ? d->original->name  :
		d->character ? d->character->name : "(none)",
		d->host
		);
	}
    }
    if (count == 0)
    {
	send_to_char("No one by that name is connected.\n\r",ch);
	return;
    }

    sprintf(buf2, "%d user%s\n\r", count, count == 1 ? "" : "s");
    strcat(buf,buf2);
    page_to_char(buf, ch);
    return;
}*/

/* New sockets command, tells the connected state of characters and aligns things better. -- Areo 2006-08-23 */
void do_sockets( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA       *vch;
    DESCRIPTOR_DATA *d;
    char            buf  [ 2 * MAX_STRING_LENGTH ];
    char            buf2 [ MAX_STRING_LENGTH ];
    char	    	arg	 [ MAX_INPUT_LENGTH ];
    int             count;
    char *          st;
    char            s[100];
    char            idle[10];


    count       = 0;
    buf[0]      = '\0';
    buf2[0]     = '\0';

    strcat( buf2, "\n\r{D[{xNum Connected_State Login@ Idl{D]{W Name{x         Host\n\r" );
    strcat( buf2,"{D--------------------------------------------------------------------------{x\n\r");

    one_argument(argument,arg);
    for ( d = descriptor_list; d; d = d->next )
    {
        if (d->character != NULL && can_see(ch, d->character)
	&& (arg[0] == '\0' || is_name(arg,d->character->name)
		   || (d->original && is_name(arg,d->original->name))))
        {
           /* NB: You may need to edit the CON_ values */
           switch( d->connected )
           {
              case CON_PLAYING:              st = "    PLAYING    ";    break;
              case CON_GET_NAME:             st = "   Get Name    ";    break;
              case CON_GET_OLD_PASSWORD:     st = "Get Old Passwd ";    break;
              case CON_CONFIRM_NEW_NAME:     st = " Confirm Name  ";    break;
              case CON_GET_NEW_PASSWORD:     st = "Get New Passwd ";    break;
              case CON_CONFIRM_NEW_PASSWORD: st = "Confirm Passwd ";    break;
              case CON_GET_NEW_RACE:         st = "  Get New Race ";    break;
              case CON_GET_NEW_SEX:          st = "  Get New Sex  ";    break;
              case CON_GET_NEW_CLASS:        st = " Get New Class ";    break;
              case CON_GET_ALIGNMENT:  	     st = " Get New Align ";	break;
	      	  case CON_READ_IMOTD:		     st = " Reading IMOTD "; 	break;
              case CON_READ_MOTD:            st = "  Reading MOTD ";    break;
	      	  case CON_BREAK_CONNECT:	     st = "   LINKDEAD    ";	break;
              case CON_GET_ASCII:		     st = "   Get ASCII   ";	break;
              case CON_GET_SUB_CLASS:	     st = "  Get Subclass ";	break;
              case CON_OLD_SUBCLASS:	     st = "  Get Old Sub  ";	break;
              case CON_SUBCLASS_CHOOSE:	     st = "Choose Subclass";	break;
              case CON_CHANGE_PASSWORD:	     st = "Change Password";	break;
              case CON_CHANGE_PASSWORD_CONFIRM:	st = "Confirm PassChg";	break;
              case CON_GET_EMAIL:			 st = "   Get Email   ";	break;
              default:                       st = "   !UNKNOWN!   ";    break;
           }
           count++;

           /* Format "login" value... */
           vch = d->original ? d->original : d->character;
           strftime( s, 100, "%I:%M%p", localtime( &vch->logon ) );

           if ( vch->timer > 0 )
              sprintf( idle, "%-2d", vch->timer );
           else
              sprintf( idle, "  " );

           sprintf(buf, "{D[{x%3d %s %7s{g %2s{D]{W %-12s{x %-50.50s\n\r",
              d->descriptor,
              st,
              s,
              idle,
              ( d->original ) ? d->original->name
                              : ( d->character )  ? d->character->name
                                                  : "(None!)",
              d->host );

           strcat( buf2, buf );

        }
    }

    if (count == 0)
    {
    send_to_char("No one by that name is connected.\n\r",ch);
    return;
    }

    sprintf( buf, "\n\r%d user%s\n\r", count, count == 1 ? "" : "s" );
    strcat( buf2, buf );
    strcat( buf2,"{D--------------------------------------------------------------------------{x\n\r");
    send_to_char( buf2, ch );
    return;
}


void do_force(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument(argument, arg);

    if (arg[0] == '\0' || argument[0] == '\0')
    {
	send_to_char("Force whom to do what?\n\r", ch);
	return;
    }

    one_argument(argument,arg2);

    if (!str_cmp(arg2,"delete") || !str_prefix(arg2,"mob"))
    {
	send_to_char("That will NOT be done.\n\r",ch);
	return;
    }

    sprintf(buf, "$n forces you to '%s'.", argument);

    if (!str_cmp(arg, "room"))
    {
	CHAR_DATA *victim_next;

	for (victim = ch->in_room->people; victim != NULL; victim = victim_next)
	{
	    victim_next = victim->next_in_room;

	    if (victim != ch && victim->tot_level < ch->tot_level) {
		act(buf, ch, victim, NULL, NULL, NULL, NULL, NULL, TO_VICT);
		interpret(victim, argument);
	    }
	}
	return;
    }

    if (!str_cmp(arg, "all"))
    {
	DESCRIPTOR_DATA *desc;
	DESCRIPTOR_DATA *desc_next;

	if (ch->tot_level < MAX_LEVEL - 2)
	{
	    send_to_char("Not at your level!\n\r",ch);
	    return;
	}

	for (desc = descriptor_list; desc != NULL; desc = desc_next)
	{
	    desc_next = desc->next;

	    if (desc->connected == CON_PLAYING
	    &&  get_trust(desc->character) < get_trust(ch))
	    {
		act(buf, ch, desc->character, NULL, NULL, NULL, NULL, NULL, TO_VICT);
		interpret(desc->character, argument);
	    }
	}
    }
    else if (!str_cmp(arg, "gods"))
    {
        DESCRIPTOR_DATA *desc,*desc_next;

        if (ch->tot_level < MAX_LEVEL - 1)
	{
            send_to_char("Not at your level!\n\r",ch);
	    return;
        }

        for (desc = descriptor_list; desc != NULL; desc = desc_next)
	{
            desc_next = desc->next;

	    if (desc->connected==CON_PLAYING
	    &&  get_trust(desc->character) < get_trust(ch)
            &&  desc->character->level >= LEVEL_HERO)
	    {
		act(buf, ch, desc->character, NULL, NULL, NULL, NULL, NULL, TO_VICT);
		interpret(desc->character, argument);
	    }
        }
    }
    else
    {
	CHAR_DATA *victim;

	if ((victim = get_char_world(ch, arg)) == NULL)
	{
	    send_to_char("They aren't here.\n\r", ch);
	    return;
	}

	if (victim == ch)
	{
	    send_to_char("Aye aye, right away!\n\r", ch);
	    return;
	}

    	if (!is_room_owner(ch,victim->in_room)
	&& ch->in_room != victim->in_room
        && room_is_private(victim->in_room, ch))
    	{
            send_to_char("That character is in a private room.\n\r",ch);
            return;
        }

	if (get_trust(victim) >= get_trust(ch)
	&&   ch->tot_level < MAX_LEVEL
	&&   !IS_NPC(victim))
	{
	    send_to_char("Do it yourself!\n\r", ch);
	    return;
	}

	act(buf, ch, victim, NULL, NULL, NULL, NULL, NULL, TO_VICT);
	interpret(victim, argument);
	act("Forced $N to \"$t\".", ch, victim, NULL, NULL, NULL, argument, NULL, TO_CHAR);
    }

    return;
}


void do_invis(CHAR_DATA *ch, char *argument)
{
    int level;
    char arg[MAX_STRING_LENGTH];

    one_argument(argument, arg);

    if (arg[0] == '\0')
    /* take the default path */

      if (ch->invis_level)
      {
	  ch->invis_level = 0;
	  act("$n slowly fades into existence.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
	  send_to_char("You slowly fade back into existence.\n\r", ch);
      }
      else
      {
	  ch->invis_level = get_trust(ch);
	  act("$n slowly fades into thin air.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
	  send_to_char("You slowly vanish into thin air.\n\r", ch);
      }
    else
    /* do the level thing */
    {
      level = atoi(arg);
      if (level < 2 || level > get_trust(ch))
      {
	send_to_char("Invis level must be between 2 and your level.\n\r",ch);
        return;
      }
      else
      {
	  ch->reply = NULL;
          ch->invis_level = level;
          act("$n slowly fades into thin air.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
          send_to_char("You slowly vanish into thin air.\n\r", ch);
      }
    }

    return;
}


void do_incognito(CHAR_DATA *ch, char *argument)
{
    int level;
    char arg[MAX_STRING_LENGTH];

    one_argument(argument, arg);

    if (arg[0] == '\0')
    /* take the default path */

      if (ch->incog_level)
      {
          ch->incog_level = 0;
          act("$n is no longer cloaked.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
          send_to_char("You are no longer cloaked.\n\r", ch);
      }
      else
      {
          ch->incog_level = get_trust(ch);
          act("$n cloaks $s presence.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
          send_to_char("You cloak your presence.\n\r", ch);
      }
    else
    /* do the level thing */
    {
      level = atoi(arg);
      if (level < 2 || level > get_trust(ch))
      {
        send_to_char("Incog level must be between 2 and your level.\n\r",ch);
        return;
      }
      else
      {
          ch->reply = NULL;
          ch->incog_level = level;
          act("$n cloaks $s presence.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
          send_to_char("You cloak your presence.\n\r", ch);
      }
    }

    return;
}


void do_holylight(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
	return;

    if (IS_SET(ch->act, PLR_HOLYLIGHT))
    {
	REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char("Holy light mode off.\n\r", ch);
    }
    else
    {
	SET_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char("Holy light mode on.\n\r", ch);
    }

    return;
}

void do_holyaura(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
	return;

    if (IS_SET(ch->act2, PLR_HOLYAURA))
    {
	REMOVE_BIT(ch->act2, PLR_HOLYAURA);
	send_to_char("Holy aura mode off.\n\r", ch);
    }
    else
    {
	SET_BIT(ch->act2, PLR_HOLYAURA);
	send_to_char("Holy aura mode on.\n\r", ch);
    }

    return;
}

void do_olevel(CHAR_DATA *ch, char *argument)
{
	ITERATOR it;
    char buf[MAX_INPUT_LENGTH];
    char min[MAX_INPUT_LENGTH];
    char max[MAX_INPUT_LENGTH];
    char type[MAX_INPUT_LENGTH];
    char wear_loc[MAX_INPUT_LENGTH];
    BUFFER *buffer;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int number = 0, max_found;
    found = FALSE;
    number = 0;
    max_found = 200;
    buffer = new_buf();

    argument = one_argument(argument, min);
    argument = one_argument(argument, max);
    argument = one_argument(argument, type);
    argument = one_argument(argument, wear_loc);

    if (min[0] == '\0')
    {
        send_to_char("Syntax: olevel <min> <max> <type> <wear_loc>\n\r", ch);
	return;
    }

	iterator_start(&it, loaded_objects);
	while(( obj = (OBJ_DATA *)iterator_nextdata(&it)))
    {
//	    if (next_obj != NULL
//	    && obj->pIndexData->vnum == next_obj->pIndexData->vnum)
//		    continue;

	    if (obj->level < atoi(min) || obj->level > atoi(max))
		    continue;

	    if (type[0] != '\0' && flag_value(type_flags, type) != obj->pIndexData->item_type)
		    continue;

	    if (wear_loc[0] != '\0' && !IS_SET(obj->wear_flags, flag_value(wear_flags, wear_loc)))
		    continue;

	    found = TRUE;
	    number++;
	    for (in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj);

	    if (in_obj->carried_by != NULL &&
	    	can_see(ch,in_obj->carried_by) &&
	    	in_obj->carried_by->in_room != NULL)
		    sprintf(buf, "%3d) %s (vnum %ld) is carried by %s [Room %ld]\n\r",
				number,
				obj->short_descr,
				obj->pIndexData->vnum,
				pers(in_obj->carried_by, ch),
				in_obj->carried_by->in_room->vnum);
	    else if (in_obj->in_room != NULL && can_see_room(ch,in_obj->in_room))
		    sprintf(buf, "%3d) %s (vnum %ld) is in %s [Room %ld]\n\r",
				number,
				obj->short_descr,
				obj->pIndexData->vnum,
				in_obj->in_room->name,
				in_obj->in_room->vnum);
	    else
		    sprintf(buf, "%3d) %s (vnum %ld) is somewhere\n\r",
				number,
				obj->short_descr,
				obj->pIndexData->vnum);

	    buf[0] = UPPER(buf[0]);
	    add_buf(buffer,buf);
	    if (number >= max_found)
		    break;
    }
    iterator_stop(&it);

    if (!found)
	    send_to_char("Nothing like that in heaven or earth.\n\r", ch);
    else
	    page_to_char(buf_string(buffer),ch);

    free_buf(buffer);
}


void do_mlevel(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_INPUT_LENGTH];
	BUFFER *buffer;
	CHAR_DATA *victim;
	bool found;
	int count = 0;
	ITERATOR vit;

	if (argument[0] == '\0')
	{
		send_to_char("Syntax: mlevel <level>\n\r",ch);
		return;
	}
	found = FALSE;
	buffer = new_buf();
	iterator_start(&vit, loaded_chars);
	while(( victim = (CHAR_DATA *)iterator_nextdata(&vit)))
	{
		if (victim->in_room != NULL &&
			atoi(argument) == victim->level) {
			found = TRUE;
			count++;
			sprintf(buf, "%3d) [%5ld] %-28s [%5ld] %s\n\r",
					count,
					IS_NPC(victim) ?
					victim->pIndexData->vnum : 0,
					IS_NPC(victim) ?
					victim->short_descr : victim->name,
					victim->in_room->vnum,
					victim->in_room->name);
			add_buf(buffer,buf);
		}
	}
	iterator_stop(&vit);

	if (!found)
		act("You didn't find any mob of level $T.",
				ch, NULL, NULL, NULL, NULL, NULL, argument, TO_CHAR);
	else
		page_to_char(buf_string(buffer),ch);

	free_buf(buffer);
	return;
}


void do_reckoning(CHAR_DATA *ch, char *argument)
{
    struct tm *reck_time;

    reck_time = (struct tm *) localtime(&current_time);
    reck_time->tm_min += 30;
    reckoning_timer = (time_t) mktime(reck_time);
    pre_reckoning = 1;

    send_to_char("{RLet the reckoning begin.{x\n\r", ch);
}


void do_immortalise(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    //OBJ_DATA *obj;
    char arg[MAX_INPUT_LENGTH];
    //char buf[MAX_STRING_LENGTH];
    //char buf2[MSL];
    int i;

    argument = one_argument(argument, arg);

    if (arg[0] == '\0')
    {
		send_to_char("Immortalise whom?\n\r", ch);
		send_to_char("Syntax: immortalise <person> <subclass>\n\r", ch);
		return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL)
    {
		send_to_char("They aren't online.\n\r", ch);
		return;
    }

    if (IS_NPC(victim))
    {
		send_to_char("You can't immortalise NPCs.\n\r", ch);
		return;
    }

    if (IS_REMORT(victim))
    {
		send_to_char("That person has already been immortalised.\n\r", ch);
		return;
    }

    if (victim->tot_level < LEVEL_HERO)
    {
		act("$N must be at max level to remort.", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
		return;
    }

    if (*argument == '\0')
    {
		show_multiclass_choices(victim, ch);
		return;
    }

    for (i = CLASS_WARRIOR_WARLORD; i < MAX_SUB_CLASS; i++) {
		if (!str_cmp(argument, sub_class_table[i].name[victim->sex]))
		    break;
    }

    if (i == MAX_SUB_CLASS) {
		send_to_char("Not a valid subclass.\n\r", ch);
		return;
    }

    if (!can_choose_subclass(victim, i))
    {
		act("$N cannot choose that subclass.", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
		return;
    }

	remort_player(victim, i);

#if 0
    sprintf(argument, "%s", sub_class_table[i].name[0]);

    i = 0;
    victim->race = get_remort_race(victim);
    sprintf(buf2, "%s", pc_race_table[victim->race].name);
    while (buf2[i] != '\0')
    {
	buf2[i] = UPPER(buf2[i]);
	i++;
    }

    if (victim->alignment < 0)
    {
        sprintf(buf, "{RHoly statues cry tears of blood and the sillhouettes "
		      "of winged horrors appear in the sky.{X\n\r{RA new %s has been born!{x\n\r", buf2);

	victim->alignment = -1000;

    send_to_char("Your mortal essence crumbles as you embrace your fate.\n\r", victim);
	send_to_char("You welcome the dark power as it flows through your divine veins.\n\r", victim);
	send_to_char("A dark influence clouds all that you once knew; your lifeless body\n\r", victim);
	send_to_char("lies slouched in front of you as part of you is torn into the Abyss.\n\r", victim);
	send_to_char("You feel complete, and wielding unfathomable power, you know you can\n\r", victim);
	send_to_char("manipulate it to suit your darkest desires.\n\r", victim);
    }
    else if (victim->alignment > 0)
    {
	sprintf(buf, "{WBrilliant white light radiates down from the heavens and thunder rolls through the valleys.\n\r"
	             "{WA new %s has been born!{x\n\r", buf2);

	victim->alignment = 1000;

 	send_to_char("Your mortal essence shines brightly, blinding your eyes.\n\r", victim);
	send_to_char("Images flash before you: sadness, grief, terror and hatred.\n\r", victim);
	send_to_char("Your life is played to you, from the beginning to the present.\n\r", victim);
	send_to_char("Your veins flow with the divine influence as you stand before your\n\r", victim);
	send_to_char("lifeless mortal vessel. It becomes clear to you that you have been\n\r", victim);
	send_to_char("reborn a divine power.\n\r", victim);
    }
    else
    {
	sprintf(buf, "{CThe cosmic energies of the world shift and the clouds speed overhead.{x\n\r"
				 "{CA new %s has been born!{x\n\r", buf2);

	victim->alignment = 0;
    }

    gecho(buf);

    /* take off equipment*/
    for (obj = victim->carrying; obj != NULL; obj = obj->next_content)
    {
		if (obj->wear_loc != WEAR_NONE)
		    unequip_char(victim, obj, FALSE);
    }

    /* take off remaining affects*/
    while (victim->affected)
		affect_remove(victim, victim->affected);

    /* lower their stats significantly*/
    for (i = 0; i < MAX_STATS; i++) {
		int val = victim->perm_stat[i] - number_range(4,6);
		set_perm_stat(victim, i, UMAX(val, 13));
	}

	victim->affected_by_perm = race_table[victim->race].aff;
	victim->affected_by2_perm = race_table[victim->race].aff2;
    victim->imm_flags_perm = race_table[victim->race].imm;
    victim->res_flags_perm = race_table[victim->race].res;
    victim->vuln_flags_perm = race_table[victim->race].vuln;

    victim->form        = race_table[victim->race].form;
    victim->parts       = race_table[victim->race].parts;
    victim->lostparts	= 0;	// Restore anything lost

    /* add skills for remort race*/
    for (i = 0; pc_race_table[victim->race].skills[i] != NULL; i++)
	group_add(victim,pc_race_table[victim->race].skills[i],FALSE);

    victim->pcdata->hit_before  = victim->pcdata->perm_hit;
    victim->pcdata->mana_before = victim->pcdata->perm_mana;
    victim->pcdata->move_before = victim->pcdata->perm_move;

    victim->pcdata->perm_hit  = 20;
    victim->pcdata->perm_mana = 20;
    victim->pcdata->perm_move = 20;

    victim->max_hit  = 20;
    victim->max_mana = 20;
    victim->max_move = 20;

    victim->hit  = 20;
    victim->mana = 20;
    victim->move = 20;

    victim->tot_level = 1;
    victim->level = 1;

	// Reset base affects - will reset affected_by, affected_by2, imm_flags, res_flags and vuln_flags
    affect_fix_char(victim);

    char_from_room(victim);
    char_to_room(victim, get_room_index(ROOM_VNUM_SCHOOL));

    /* mages*/
    if (!str_cmp("archmage", argument)
    ||  !str_cmp("geomancer", argument)
    ||  !str_cmp("illusionist", argument))
    {
	victim->pcdata->class_current = CLASS_MAGE;
	victim->pcdata->second_class_mage = CLASS_MAGE;

	if (!str_cmp("archmage", argument))
	{
	    victim->pcdata->sub_class_current = CLASS_MAGE_ARCHMAGE;
	    victim->pcdata->second_sub_class_mage = CLASS_MAGE_ARCHMAGE;
	}

	if (!str_cmp("geomancer", argument))
	{
	    victim->pcdata->sub_class_current = CLASS_MAGE_GEOMANCER;
	    victim->pcdata->second_sub_class_mage = CLASS_MAGE_GEOMANCER;
	}

	if (!str_cmp("illusionist", argument))
	{
	    victim->pcdata->sub_class_current = CLASS_MAGE_ILLUSIONIST;
	    victim->pcdata->second_sub_class_mage = CLASS_MAGE_ILLUSIONIST;
	}
    }

    /* clerics*/
    if (!str_cmp("alchemist", argument)
    ||  !str_cmp("ranger", argument)
    ||  !str_cmp("adept", argument))
    {
	victim->pcdata->class_current = CLASS_CLERIC;
	victim->pcdata->second_class_cleric = CLASS_CLERIC;

	if (!str_cmp("alchemist", argument))
	{
	    victim->pcdata->sub_class_current = CLASS_CLERIC_ALCHEMIST;
	    victim->pcdata->second_sub_class_cleric = CLASS_CLERIC_ALCHEMIST;
	}

	if (!str_cmp("ranger", argument))
	{
	    victim->pcdata->sub_class_current = CLASS_CLERIC_RANGER;
	    victim->pcdata->second_sub_class_cleric = CLASS_CLERIC_RANGER;
	}

	if (!str_cmp("adept", argument))
	{
	    victim->pcdata->sub_class_current = CLASS_CLERIC_ADEPT;
	    victim->pcdata->second_sub_class_cleric = CLASS_CLERIC_ADEPT;
	}
    }

    /* thieves*/
    if (!str_cmp("highwayman", argument)
    ||  !str_cmp("ninja", argument)
    ||  !str_cmp("sage", argument))
    {
	victim->pcdata->class_current = CLASS_THIEF;
	victim->pcdata->second_class_thief = CLASS_THIEF;

	if (!str_cmp("highwayman", argument))
	{
	    victim->pcdata->sub_class_current = CLASS_THIEF_HIGHWAYMAN;
	    victim->pcdata->second_sub_class_thief = CLASS_THIEF_HIGHWAYMAN;
	}

	if (!str_cmp("ninja", argument))
	{
	    victim->pcdata->sub_class_current = CLASS_THIEF_NINJA;
	    victim->pcdata->second_sub_class_thief = CLASS_THIEF_NINJA;
	}

	if (!str_cmp("sage", argument))
	{
	    victim->pcdata->sub_class_current = CLASS_THIEF_SAGE;
	    victim->pcdata->second_sub_class_thief = CLASS_THIEF_SAGE;
	}
    }

    /* warriors*/
    if (!str_cmp("warlord", argument)
    || !str_cmp("destroyer", argument)
    || !str_cmp("crusader", argument))
    {
	victim->pcdata->class_current = CLASS_WARRIOR;
	victim->pcdata->second_class_warrior = CLASS_WARRIOR;

	if (!str_cmp("warlord", argument))
	{
	    victim->pcdata->sub_class_current = CLASS_WARRIOR_WARLORD;
	    victim->pcdata->second_sub_class_warrior = CLASS_WARRIOR_WARLORD;
	}

	if (!str_cmp("destroyer", argument))
	{
	    victim->pcdata->sub_class_current = CLASS_WARRIOR_DESTROYER;
	    victim->pcdata->second_sub_class_warrior = CLASS_WARRIOR_DESTROYER;
	}

	if (!str_cmp("crusader", argument))
	{
	    victim->pcdata->sub_class_current = CLASS_WARRIOR_CRUSADER;
	    victim->pcdata->second_sub_class_warrior = CLASS_WARRIOR_CRUSADER;
	}
    }

    group_add(victim, class_table[victim->pcdata->class_current].base_group, TRUE);
    group_add(victim, sub_class_table[victim->pcdata->sub_class_current].default_group, TRUE);
    victim->exp = 0;

    sprintf(buf2, sub_class_table[victim->pcdata->sub_class_current].name[victim->sex]);
    buf2[0] = UPPER(buf2[0]);
    sprintf(buf, "All congratulate %s, who is now a%s %s!",
        victim->name, (buf2[0] == 'A' || buf2[0] == 'I' || buf2[0] == 'E' || buf2[0] == 'U'
	    || buf2[0] == 'O') ? "n" : "", buf2);
    crier_announce(buf);
    double_xp(victim);
#endif
}


void do_arealinks(CHAR_DATA *ch, char *argument)
{
    /*FILE *fp;*/
    BUFFER *buffer;
    AREA_DATA *parea;
    EXIT_DATA *pexit;
    ROOM_INDEX_DATA *to_room;
    ROOM_INDEX_DATA *from_room;
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    long vnum = 0;
    long iHash;
    int door;
    bool found = FALSE;

    /* To provide a convenient way to translate door numbers to words */
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    /* First, the 'all' option */
    if (!str_cmp(arg1,"all"))
    {
	/*
	 * If a filename was provided, try to open it for writing
	 * If that fails, just spit output to the screen.
	 */ /*
	if (arg2[0] != '\0')
	{
	    fclose(fpReserve);
	    if((fp = fopen(arg2, "w")) == NULL)
	    {
		send_to_char("Error opening file, printing to screen.\n\r",ch);
		fclose(fp);
		fpReserve = fopen(NULL_FILE, "r");
		fp = NULL;
	    }
	}
	else
	    fp = NULL; */

	/* Open a buffer if it's to be output to the screen */
	/*if (!fp)*/
	    buffer = new_buf();

	/* Loop through all the areas */
	for (parea = area_first; parea != NULL; parea = parea->next)
	{
	    /* First things, add area name  and vnums to the buffer */
	    sprintf(buf, "*** %s (%ld to %ld) ***\n\r",
			 parea->name, parea->min_vnum, parea->max_vnum);
	    /*fp ? fprintf(fp, buf) : */add_buf(buffer, buf);

	    /* Now let's start looping through all the rooms. */
	    found = FALSE;
	    for(iHash = 0; iHash < MAX_KEY_HASH; iHash++)
	    {
		for(from_room = room_index_hash[iHash];
		     from_room != NULL;
		     from_room = from_room->next)
		{
		    /*
		     * If the room isn't in the current area,
		     * then skip it, not interested.
		     */
		    if (from_room->vnum < parea->min_vnum
		    ||   from_room->vnum > parea->max_vnum)
			continue;

		    /* Aha, room is in the area, lets check all directions */
		    for (door = 0; door < 9; door++)
		    {
			/* Does an exit exist in this direction? */
			if((pexit = from_room->exit[door]) != NULL)
			{
			    to_room = pexit->u1.to_room;

			    /*
			     * If the exit links to a different area
			     * then add it to the buffer/file
			     */
			    if(to_room != NULL
			    &&  (to_room->vnum < parea->min_vnum
			    ||   to_room->vnum > parea->max_vnum))
			    {
				found = TRUE;
				sprintf(buf, "    (%ld) links %s to %s (%ld)\n\r",
				    from_room->vnum, dir_name[door],
				    to_room->area->name, to_room->vnum);

				/* Add to either buffer or file */
				/*if(fp == NULL)*/
				    add_buf(buffer, buf);
				/*else*/
				/*    fprintf(fp, buf);*/
			    }
			}
		    }
		}
	    }

	    /* Informative message for areas with no external links */
	    if (!found)
		add_buf(buffer, "    No links to other areas found.\n\r");
	}

	/* Send the buffer to the player */
	/*if (!fp)
	{*/
	    page_to_char(buf_string(buffer), ch);
	    free_buf(buffer);
	/*}*/
	/* Or just clean up file stuff */
	/*else
	{
	    fclose(fp);
	    fpReserve = fopen(NULL_FILE, "r");
	}*/

	return;
    }

    /* No argument, let's grab the char's current area */
    if(arg1[0] == '\0')
    {
	parea = ch->in_room ? ch->in_room->area : NULL;

	/* In case something wierd is going on, bail */
	if (parea == NULL)
	{
	    send_to_char("You aren't in an area right now, funky.\n\r",ch);
	    return;
	}
    }
    /* Room vnum provided, so lets go find the area it belongs to */
    else if(is_number(arg1))
    {
	vnum = atol(arg1);

	/* Hah! No funny vnums! I saw you trying to break it... */
	if (vnum <= 0 || vnum > 2147483647)
	{
	    send_to_char("The vnum must be between 1 and 2147483647.\n\r",ch);
	    return;
	}

	/* Search the areas for the appropriate vnum range */
	for (parea = area_first; parea != NULL; parea = parea->next)
	{
	    if(vnum >= parea->min_vnum && vnum <= parea->max_vnum)
		break;
	}

	/* Whoops, vnum not contained in any area */
	if (parea == NULL)
	{
	    send_to_char("There is no area containing that vnum.\n\r",ch);
	    return;
	}
    }
    /* Non-number argument, must be trying for an area name */
    else
    {
	/* Loop the areas, compare the name to argument */
	for(parea = area_first; parea != NULL; parea = parea->next)
	{
	    if(!str_prefix(arg1, parea->name))
		break;
	}

	/* Sorry chum, you picked a goofy name */
	if (parea == NULL)
	{
	    send_to_char("There is no such area.\n\r",ch);
	    return;
	}
    }

    /* Just like in all, trying to fix up the file if provided */
   /* if (arg2[0] != '\0')
    {
	fclose(fpReserve);
	if((fp = fopen(arg2, "w")) == NULL)
	{
	    send_to_char("Error opening file, printing to screen.\n\r",ch);
	    fclose(fp);
	    fpReserve = fopen(NULL_FILE, "r");
	    fp = NULL;
	}
    }
    else
	fp = NULL;*/

    /* And we loop the rooms */
    for(iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    {
	for(from_room = room_index_hash[iHash];
	     from_room != NULL;
	     from_room = from_room->next)
	{
	    /* Gotta make sure the room belongs to the desired area */
	    if (from_room->vnum < parea->min_vnum
	    ||   from_room->vnum > parea->max_vnum)
		continue;

	    /* Room's good, let's check all the directions for exits */
	    for (door = 0; door < 9; door++)
	    {
		if((pexit = from_room->exit[door]) != NULL)
		{
		    to_room = pexit->u1.to_room;

		    /* Found an exit, does it lead to a different area? */
		    if(to_room != NULL
		    &&  (to_room->vnum < parea->min_vnum
		    ||   to_room->vnum > parea->max_vnum))
		    {
			found = TRUE;
			sprintf(buf, "%s (%ld) links %s to %s (%ld)\n\r",
				    parea->name, from_room->vnum, dir_name[door],
				    to_room->area->name, to_room->vnum);

			/* File or buffer output? */
			/*if(fp == NULL)*/
			    send_to_char(buf, ch);
			/*else*/
			/*    fprintf(fp, buf);*/
		    }
		}
	    }
	}
    }

    /* Informative message telling you it's not externally linked */
    if(!found)
    {
	send_to_char("No links to other areas found.\n\r",ch);
	/* Let's just delete the file if no links found */
	/*if (fp)*/
	/*    unlink(arg2);*/
	return;
    }

    /* Close up and clean up file stuff */
    /*if(fp)
    {
	fclose(fp);
	fpReserve = fopen(NULL_FILE, "r");
    }*/

}


void do_sload(CHAR_DATA *ch, char *argument)
{
#if 0
/*
    char arg1[MAX_INPUT_LENGTH] ,arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *pRoom;
    NPC_SHIP_INDEX_DATA *pShip;
    NPC_SHIP_DATA *pNpcShip;
    long room_vnum;
    long ship_vnum;

    argument = one_argument(argument, arg1);
    one_argument(argument, arg2);

    if (arg1[0] == '\0' || !is_number(arg1))
    {
	send_to_char("Syntax: load ship <vnum> <room vnum>.\n\r", ch);
	return;
    }

    ship_vnum = atol(arg1);

    if (arg2[0] != '\0')
    {
	if (!is_number(arg2))
        {
	  send_to_char("Syntax: sload <vnum> <room vnum>.\n\r", ch);
	  return;
	}
        room_vnum = atol(arg2);
        if ((pRoom = get_room_index(room_vnum)) == NULL)
	{
	  send_to_char("Could not find room vnum.\n\r",ch);
  	  return;
	}
    }
    else {
      send_to_char("Syntax: sload <vnum> <room vnum>.\n\r", ch);
      return;
    }

    if ((pShip = get_npc_ship_index(ship_vnum)) == NULL)
    {
	send_to_char("No ship has that vnum.\n\r", ch);
	return;
    }

    pNpcShip = create_npc_sailing_boat(ship_vnum);

     If the npc airship then set airship
    if (pShip->npc_type == NPC_SHIP_AIR_SHIP)
    {
	    plith_airship = pNpcShip;
    }

    obj_to_room(pNpcShip->ship->ship, pRoom);

    if (pNpcShip->ship->ship->in_room == NULL)
    {
	    gecho("NULL already");
    }

    send_to_char("Ship created.\n\r", ch);
    return;
*/
#endif
}


/* Strip items of a vnum or name from a ch.*/
void do_junk(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    char arg[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    bool fAll = FALSE;
    bool found = FALSE;

    argument = one_argument(argument, arg);
    argument = one_argument(argument, arg2);
    if (arg[0] == '\0' || arg2[0] == '\0')
    {
	    send_to_char("Syntax: junk <person> <obj vnum or name> [all]\n\r",
			    ch);
	    return;
    }

    if ((victim = get_char_room(ch, NULL, arg)) == NULL)
    {
	    send_to_char("They aren't here.\n\r", ch);
	    return;
    }

    if (argument[0] != '\0'
    && !str_cmp(argument, "all"))
	    fAll = TRUE;

    if (is_number(arg2)
    && get_obj_index(atol(arg2)) == NULL)
    {
	    send_to_char("No such object even exists.\n\r", ch);
	    return;
    }

    for (obj = victim->carrying; obj != NULL; obj = obj_next)
    {
	    obj_next = obj->next_content;
	    if ((is_number(arg2)
 	         && obj->pIndexData->vnum == atol(arg2))
	    || (is_name(arg2, obj->name)))
	    {
		    act("Extracted $p from $N.", ch, victim, NULL, obj, NULL, NULL, NULL, TO_CHAR);
		    extract_obj(obj);
		    found = TRUE;
		    if (!fAll) break;
	    }
    }

    if (found)
	    send_to_char("Done.\n\r", ch);
    else
	    send_to_char("They are carrying no such object.\n\r", ch);

    return;
}


void do_alevel(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    long xp;

    one_argument(argument, arg);

    if (arg[0] == '\0')
    {
	send_to_char("Level whom?\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    xp = exp_per_level(victim, victim->pcdata->points) - victim->exp;
    gain_exp(victim, xp);

    return;
}


void do_areset(CHAR_DATA *ch, char *argument)
{
    AREA_DATA *area;
    char buf[MSL];

    if (argument[0] == '\0')
    {
	send_to_char("Reset which area?\n\r", ch);
	return;
    }

    area = NULL;
    for (area = area_first; area != NULL; area = area->next)
    {
	if (!str_infix(argument, area->name))
  	    break;
    }

    if (!area)
    {
	send_to_char("Couldn't find that area.\n\r", ch);
	return;
    }

    reset_area(area);
    area->age = 0;
    sprintf(buf, "Reset %s.\n\r", area->name);
    send_to_char(buf, ch);
}


void do_autosetname(CHAR_DATA *ch, char *argument)
{
    if (!IS_SET(ch->act, PLR_AUTOSETNAME))
    {
	send_to_char("AUTOSETNAME on. Your name keywords will now be automatically set when building.\n\r", ch);
	SET_BIT(ch->act, PLR_AUTOSETNAME);
    }
    else
    {
	send_to_char("AUTOSETNAME off. Your name keywords will no longer be automatically set.\n\r", ch);
	REMOVE_BIT(ch->act, PLR_AUTOSETNAME);
    }
}


void do_autowar(CHAR_DATA *ch, char *argument)
{
    char buf[MSL];
    char arg[MSL];
    char arg2[MSL];
    char arg3[MSL];
    char arg4[MSL];
    char arg5[MSL];
    int i;
    int min_players;
    int min;
    int max;
    int timer;

    argument = one_argument(argument, arg);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);
    argument = one_argument(argument, arg4);
    argument = one_argument(argument, arg5);

    if (!str_cmp(arg, "stop"))
    {
	if (auto_war == NULL) {
	    send_to_char("No war is going on.\n\r", ch);
	    return;
	}

	sprintf(buf, "{R%s has ended the autowar.{x\n\r", ch->name);
	war_channel(buf);
	free_auto_war(auto_war);
	return;
    }

    if (auto_war_timer > 0)
    {
	send_to_char("Auto-war already in progress.\n\r", ch);
	return;
    }

    if (arg[0] == '\0')
    {
	send_to_char("autowar <type> <min_players> <min_level> <max_level> <time_before_start>\n\r", ch);
	send_to_char("What type of autowar?\n\r{Y", ch);
	i = 0;
	while(auto_war_table[i].name != NULL)
	{
	    send_to_char(auto_war_table[i].name, ch);
	    send_to_char("\n\r", ch);
	    i++;
	}
	send_to_char("{x\n\r", ch);
    }

    if (arg2[0] == '\0')
    {
	send_to_char("What is the minimum number of players?\n\r", ch);
	return;
    }

    if (arg3[0] == '\0')
    {
	send_to_char("What is the minimum and maximum level?\n\r", ch);
	return;
    }

    if (arg4[0] == '\0')
    {
	send_to_char("What is the maximum level?\n\r", ch);
	return;
    }

    if (arg5[0] == '\0' || !is_number(arg5))
    {
	timer = 2;
    }
    else
    {
	timer = atoi(arg5);
    }

    i = 0;
    while(auto_war_table[i].name != NULL)
    {
	if (!str_prefix(auto_war_table[i].name, arg))
	{
	    break;
	}
	i++;
    }

    if (auto_war_table[i].name == NULL)
    {
	send_to_char("That isn't an auto-war type.\n\r", ch);
	return;
    }

    if (!is_number(arg2) || !is_number(arg3) || !is_number(arg4))
    {
	send_to_char("Invalid level range given.\n\r", ch);
	return;
    }

    min_players = atoi(arg2);
    if (min_players < 2) {
	send_to_char("You need at least two players to fight a war.\n\r", ch);
	return;
    }

    min = atoi(arg3);
    max = atoi(arg4);

    if (min >= max)
    {
	send_to_char("Invalid level range.\n\r", ch);
	return;
    }

    if (auto_war != NULL)
    {
	free_auto_war(auto_war);
    }

    auto_war = new_auto_war(i, min_players, min, max);
    auto_war_timer = timer;

    sprintf(buf, "{RGet Ready! {RA {Y%s{R war is about to begin for levels {Y%d{R to {Y%d{R, in {Y%d{R minutes!{x\n\r",
	    auto_war_table[i].name, min, max, auto_war_timer);
    gecho(buf);
    gecho("Type 'war join' to enter!\n\r");
}


void do_vislist(CHAR_DATA *ch, char *argument)
{
    char arg[MSL];
    char buf[MSL];
    char player_name[MSL];
    bool found_char;
    FILE *fp;
    STRING_DATA *string;
    STRING_DATA *string_prev;
    bool found;
    int i;

    if (IS_NPC(ch))
	return;

    argument = one_argument(argument, arg);
    arg[0] = UPPER(arg[0]);

    if (strlen(arg) > 12)
	arg[12] = '\0';

    /* show vislist */
    if (arg[0] == '\0' || !str_cmp(arg, "show"))
    {
	send_to_char("{YYou are currently visible to:{x\n\r", ch);
	line(ch, 45);
	i = 0;
	for (string = ch->pcdata->vis_to_people; string != NULL;
	      string = string->next)
	{
	    sprintf(buf, "{Y%2d):{x %s\n\r", i + 1, string->string);
	    send_to_char(buf, ch);
	    i++;
	}

	if (i == 0)
	    send_to_char("Nobody.\n\r", ch);

	line(ch, 45);

	return;
    }

    if (!str_cmp(arg, ch->name))
    {
	send_to_char("That would be pointless.\n\r", ch);
	return;
    }

    /* take everyone off */
    if (!str_cmp(arg, "clear"))
    {
	STRING_DATA *string_next;

	for (string = ch->pcdata->vis_to_people; string != NULL; string = string_next)
	{
	    string_next = string->next;
	    do_function(ch, &do_vislist, string->string);
	}

	send_to_char("Vislist cleared.\n\r", ch);
	return;
    }

    found = FALSE;
    string_prev = NULL;
    for (string = ch->pcdata->vis_to_people; string != NULL;
          string = string->next)
    {
	if (!str_prefix(arg, string->string))
	{
	    found = TRUE;
	    break;
	}

	string_prev = string;
    }

    if (found)
    {
	act("Removed $t from vis list.", ch, NULL, NULL, NULL, NULL, string->string, NULL, TO_CHAR);
	if (string_prev != NULL)
	    string_prev->next = string->next;
	else
	    ch->pcdata->vis_to_people = string->next;
	free_string_data(string);
	return;
    }
    else
    {
	CHAR_DATA *victim;

	i = 0;
	for (string = ch->pcdata->vis_to_people; string != NULL;
	      string = string->next)
	    i++;

	if (i > 14)
	{
	    send_to_char("Sorry, maximum is 15 people.\n\r", ch);
	    return;
	}

	if ((victim = get_char_world(ch, arg)) != NULL
	&& !IS_NPC(victim))
	{
	    found_char = TRUE;
	    sprintf(arg, "%s", capitalize(victim->name));
	}
	else
	{
	    sprintf(player_name, "%s%c/%s", PLAYER_DIR, tolower(arg[0]), capitalize(arg));
	    if ((fp = fopen(player_name, "r")) == NULL)
	    {
		found_char = FALSE;
	    }
	    else
	    {
		found_char = TRUE;
		fclose (fp);
	    }
	}

	if (!found_char)
	{
	    send_to_char("That player doesn't exist.\n\r", ch);
	    return;
	}

	string = new_string_data();
	string->string = str_dup(arg);
	act("Added $t to your vis list.", ch, NULL, NULL, NULL, NULL, string->string, NULL, TO_CHAR);
	string->next = ch->pcdata->vis_to_people;
	ch->pcdata->vis_to_people = string;
    }
}


/* dummy command for whatever, used in debugging only */
void do_test(CHAR_DATA *ch, char *argument)
{
}


void do_assignhelper(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_STRING_LENGTH];
    CHAR_DATA * victim;
    argument = one_argument(argument, arg);
    if (ch->tot_level < MAX_LEVEL - 1)
    {
	send_to_char("Huh?\n\r", ch);
	return;
    }

    if (arg[0] == '\0')
    {
	send_to_char ("Who do you want to make a helper?\n\r", ch);
	return;
    }

    victim = get_char_world(ch, arg);
    if (victim == NULL)
    {
	send_to_char("That player doesn't exist.\n\r", ch);
	return;
    }

    if (IS_NPC(victim))
    {
	send_to_char("That isn't a player!\n\r", ch);
	return;
    }

    if (IS_SET(victim->act, PLR_HELPER))
    {
	REMOVE_BIT(victim->act, PLR_HELPER);
	SET_BIT(ch->comm,COMM_NOHELPER);
	if (ch == victim)
	    send_to_char("You are no longer a helper.\n\r", ch);
	else
	{
	    act("$N is no longer a helper.", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
	    send_to_char("You are no longer a helper.\n\r", victim);
	}
    }
    else
    {
	SET_BIT(victim->act, PLR_HELPER);
	REMOVE_BIT(ch->comm,COMM_NOHELPER);
	if (ch == victim)
	    send_to_char("You are now a helper.\n\r", ch);
	else
	{
	    act("$N is now a helper.", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
	    send_to_char("You are now a helper.\n\r", victim);
	}
    }
}


void do_otransfer(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    /*char buf[MAX_STRING_LENGTH];*/
    ROOM_INDEX_DATA *location;
    OBJ_DATA *obj;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0')
    {
	send_to_char("Transfer what (and where)?\n\r", ch);
	return;
    }

    obj = get_obj_world(ch, arg1);

    if (obj == NULL)
    {
	send_to_char("No object.\n\r", ch);
	return;
    }

    if (obj->carried_by != NULL
    || obj->in_room == NULL)
    {
	act("$p isn't on the ground.", ch, NULL, NULL, obj, NULL, NULL, NULL, TO_CHAR);
	return;
    }

    act("Transferred $p.", ch, NULL, NULL, obj, NULL, NULL, NULL, TO_CHAR);

    if (arg2[0] == '\0')
    {
	location = ch->in_room;
    }
    else
    {
	if ((location = find_location(ch, arg2)) == NULL)
	{
	    send_to_char("No such location.\n\r", ch);
	    return;
	}
    }

    obj_from_room(obj);
    if(location->wilds)
    	obj_to_vroom(obj, location->wilds, location->x, location->y);
    else
	obj_to_room(obj, location);

    return;
}


/* Go unwizi for one command only*/
void do_uninvis(CHAR_DATA *ch, char *argument)
{
    int lev_wizi;
    int lev_incog;

    if (argument[0] == '\0')
    {
	send_to_char("Syntax: uninvis <command>\n\r", ch);
	return;
    }

    lev_wizi  = ch->invis_level;
    lev_incog = ch->incog_level;

    ch->invis_level = 0;
    ch->incog_level = 0;
    interpret(ch, argument);

    ch->invis_level = lev_wizi;
    ch->incog_level = lev_incog;
}


/* Allows custom granting of commands to people to do away with all those
   clumsy hacks. (Syn 2006-06-17) */
void do_addcommand(CHAR_DATA *ch, char *argument)
{
    char arg[MSL];
    char arg2[MSL];
    char buf[MSL];
    CHAR_DATA *vch;
    COMMAND_DATA *cmd;
    int i;
    bool found = FALSE;

    argument = one_argument(argument, arg);
    argument = one_argument(argument, arg2);

    if (arg[0] == '\0' || arg2[0] == '\0') {
        send_to_char("Syntax: addcommand [person] [command]\n\r", ch);
	return;
    }

    if ((vch = get_char_world(ch, arg)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (IS_NPC(vch)) {
        send_to_char("You can't give NPCs commands.\n\r", ch);
	return;
    }

    if (vch == ch) {
        send_to_char("You can't give yourself commands.\n\r", ch);
	return;
    }

    for (i = 0; cmd_table[i].name[0] != '\0'; i++)
    {
        if (!str_prefix(arg2, cmd_table[i].name)
	&&  cmd_table[i].level <= ch->tot_level)
	{
	    found = TRUE;
	    break;
	}
    }

    if (!found) {
        send_to_char("Command not found.\n\r", ch);
	return;
    }

    if (cmd_table[i].level <= vch->tot_level) {
        act("$N can already use that command due to $S level.", ch, vch, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
	return;
    }

    cmd = new_command();
    cmd->name = str_dup(cmd_table[i].name);
    cmd->next = vch->pcdata->commands;
    vch->pcdata->commands = cmd;

    sprintf(buf, "Granted command \"%s\" to %s.\n\r", cmd_table[i].name, vch->name);
    send_to_char(buf, ch);
}


void do_remcommand(CHAR_DATA *ch, char *argument)
{
    char arg[MSL];
    char arg2[MSL];
    char buf[MSL];
    CHAR_DATA *vch;
    COMMAND_DATA *cmd, *cmd_prev = NULL;
    bool found = FALSE;

    argument = one_argument(argument, arg);
    argument = one_argument(argument, arg2);

    if (arg[0] == '\0' || arg2[0] == '\0') {
        send_to_char("Syntax: remcommand [person] [command]\n\r", ch);
	return;
    }

    if ((vch = get_char_world(ch, arg)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (IS_NPC(vch)) {
        send_to_char("You can't remove commands from NPCs.\n\r", ch);
	return;
    }

    if (vch == ch) {
        send_to_char("That would be pointless.\n\r", ch);
	return;
    }

    for (cmd = vch->pcdata->commands; cmd != NULL; cmd = cmd->next)
    {
        if (!str_prefix(arg2, cmd->name))
	{
	    found = TRUE;
	    break;
	}

	cmd_prev = cmd;
    }

    if (!found) {
        send_to_char("Command not found.\n\r", ch);
	return;
    }

    if (cmd_prev != NULL)
	cmd_prev->next = cmd->next;
    else
    {
        vch->pcdata->commands = NULL;
	cmd->next = NULL;
    }

    sprintf(buf, "Removed command \"%s\" from %s.\n\r", cmd->name, vch->name);
    send_to_char(buf, ch);

    free_command(cmd);
}

/* Adjusted boost to allow for up to 7 days (10080 minutes) - Tieryo */
void do_boost(CHAR_DATA *ch, char *argument)
{
    char buf[MSL];
    char arg[MSL];
    char arg2[MSL];
    char arg3[MSL];
    int type;
    int mins;
    int percent;
    struct tm *timer;

    argument = one_argument(argument, arg);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);

    if (arg[0] == '\0' || arg2[0] == '\0') {
	send_to_char("Syntax:  boost <field> <#mins (1-10080 or off)> [percent]\n\rFields: experience damage qp pneuma\n\r", ch);
	return;
    }

    for (type = 0; boost_table[type].name != NULL; type++) {
	if (!str_prefix(arg, boost_table[type].name))
	    break;
    }
	/* Don't allow imms to set reckoning boost, this is to be done by the game's internal systems only -- Areo*/
    if (boost_table[type].name == NULL || strcmp(boost_table[type].name,  "reckoning") == 0) {
	send_to_char("Invalid boost field.\n\rFields: experience damage qp pneuma\n\r", ch);
	return;
    }

    if ((mins = atoi(arg2)) < 1 || mins > 10080) {
	if (!str_cmp(arg2, "off"))
	    mins = 0;
	else {
	    send_to_char("Invalid #mins.\n\rMust be 1-10080 minutes, or off.\n\r", ch);
	    return;
	}
    }

    if (arg3[0] == '\0')
	percent = 150;
    else
	if ((percent = atoi(arg3)) < 1 || percent > 200) {
	    send_to_char("Invalid boost percent.\n\rPercent must be 1-200%.\n\r", ch);
	    return;
	}

    if (mins == 0)
	sprintf(buf, "Turned off %s boost.\n\r", boost_table[type].name);
    else
	sprintf(buf, "Boosted %s to %+d%% for %d minutes.\n\r", boost_table[type].name, percent-100, mins);

    send_to_char(buf, ch);

    if (boost_table[type].timer == 0 || mins == 0)
	timer = localtime(&current_time);
    else
	timer = localtime(&boost_table[type].timer);

    if (mins >= 1) {
	timer->tm_min += mins;
	boost_table[type].timer = mktime(timer);
	boost_table[type].boost = percent;
	sprintf(buf, "{B({WBOOST{B)--> {W%d {Dminutes of %s {Dboost ({W%+d%%{D)!!!{x\n\r",
		mins, boost_table[type].colour_name, (percent - 100));
	gecho(buf);
	return;
	}
    else
	timer->tm_min = 0;
	boost_table[type].timer = mktime(timer);
}


/* Allows imms to fuck with tokens directly. */
void do_token(CHAR_DATA *ch, char *argument)
{
    char arg[MSL];
    char arg2[MSL];
    char arg3[MSL];
    char arg4[MSL], arg4b[MSL];
    char buf[MSL];
    long vnum, count;
    TOKEN_DATA *token;
    TOKEN_INDEX_DATA *token_index;

    argument = one_argument(argument, arg);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);
    argument = one_argument(argument, arg4);

    if (arg[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
		send_to_char("Syntax:  token <give|junk> char <character> [#.]<vnum>\n\r", ch);
		send_to_char("         token <give|junk> obj <object> [#.]<vnum>\n\r", ch);
		send_to_char("         token <give|junk> room [#.]<vnum>\n\r", ch);
		return;
    }

	if( !str_cmp(arg2, "char") && arg4[0] != '\0')
	{
	    CHAR_DATA *victim;

		if ((victim = get_char_world(NULL, arg3)) == NULL) {
			send_to_char("Character not found.\n\r", ch);
			return;
		}

		count = number_argument(arg4, arg4b);
	    vnum = atol(arg4b);

		if (!str_cmp(arg, "give")) {
			if(ch->tot_level < (MAX_LEVEL - 1) && ch != victim && !IS_NPC(victim)) {
				send_to_char("You may not give tokens to other players.\n\r",ch);
				return;
			}

			if ((token_index = get_token_index(vnum)) == NULL) {
				send_to_char("That token doesn't exist.\n\r", ch);
				return;
			}

			if (is_singular_token(token_index)) {
				if ((token = get_token_char(victim, vnum, 1)) != NULL) {
					send_to_char("Only one copy of this token can be given.\n\r", ch);
					return;
				}
			}

			if (IS_SET(token_index->flags, TOKEN_PERMANENT)) {

				if( ch->pcdata->security < 10 ) {
					if( !is_test_port ) {
						send_to_char("You may not give permanent tokens.\n\r", ch);
						return;
					}
					else
						send_to_char("{WWARNING: Assigning a permanent token.  This is only allowed while in Test Port Mode.{x\n\r", ch);
				}
			}

			give_token(token_index, victim, NULL, NULL);
			sprintf(buf, "Gave token %s(%ld) to character %s\n\r",
				token_index->name, token_index->vnum, HANDLE(victim));
			send_to_char(buf, ch);
			if (IS_SET(token_index->flags, TOKEN_PERMANENT))
			{
				send_to_char("{YWARNING:{R Token is {WPERMANENT{R.  It may only be removed by a system script or pfile editting.{x\n\r", ch);
			}

		} else if (!str_cmp(arg, "junk")) {
			if(ch->tot_level < (MAX_LEVEL - 1) && ch != victim && !IS_NPC(victim)) {
				send_to_char("You may not take tokens take other people.\n\r",ch);
				return;
			}

			if ((token = get_token_char(victim, vnum, count)) == NULL) {
				send_to_char("Token not found on victim.\n\r", ch);
				return;
			}

			if( token && IS_SET(token->flags, TOKEN_PERMANENT) ) {
				if( ch->pcdata->security < 10 ) {
					if( !is_test_port ) {
						send_to_char("Token is flagged permanent.  Only the server may remove it.\n\r", ch);
						return;
					}
					else
						send_to_char("{WWARNING: Removing a permanent token.  This is only allowed while in Test Port Mode.{x\n\r", ch);
				}
			}

			sprintf(buf, "Removed token %s(%ld.%ld) from character %s\n\r",
				token->name, count, token->pIndexData->vnum, HANDLE(victim));
			send_to_char(buf, ch);

			token_from_char(token);
			free_token(token);
		} else
			send_to_char("Syntax:  token <give|junk> char <character> [#.]<vnum>\n\r", ch);

	} else if(!str_cmp(arg2, "obj") && arg4[0] != '\0') {
		OBJ_DATA *obj;

		if( !(obj = get_obj_world(ch, arg3)) ) {
			send_to_char("Object not found.\n\r", ch);
			return;
		}

		count = number_argument(arg4, arg4b);
	    vnum = atol(arg4b);

		if (!str_cmp(arg, "give")) {
			if ((token_index = get_token_index(vnum)) == NULL) {
				send_to_char("That token doesn't exist.\n\r", ch);
				return;
			}

			if (is_singular_token(token_index)) {
				if ((token = get_token_obj(obj, vnum, 1)) != NULL) {
					send_to_char("Only one copy of this token can be given.\n\r", ch);
					return;
				}
			}

			give_token(token_index, NULL, obj, NULL);
			sprintf(buf, "Gave token %s(%ld) to object %s\n\r", token_index->name, token_index->vnum, obj->short_descr);
			send_to_char(buf, ch);

		} else if (!str_cmp(arg, "junk")) {
			if ((token = get_token_obj(obj, vnum, count)) == NULL) {
				send_to_char("Token not found on object.\n\r", ch);
				return;
			}

			sprintf(buf, "Removed token %s(%ld.%ld) from object %s\n\r",
				token->name, count, token->pIndexData->vnum, obj->short_descr);
			send_to_char(buf, ch);

			token_from_obj(token);
			free_token(token);
		} else
			send_to_char("Syntax:  token <give|junk> obj <object> [#.]<vnum>\n\r", ch);
	} else if(!str_cmp(arg2, "room") ) {
		count = number_argument(arg3, arg4b);
	    vnum = atol(arg4b);

		if (!str_cmp(arg, "give")) {
			if ((token_index = get_token_index(vnum)) == NULL) {
				send_to_char("That token doesn't exist.\n\r", ch);
				return;
			}

			if (is_singular_token(token_index)) {
				if ((token = get_token_room(ch->in_room, vnum, 1)) != NULL) {
					send_to_char("Only one copy of this token can be given.\n\r", ch);
					return;
				}
			}

			give_token(token_index, NULL, NULL, ch->in_room);
			if( ch->in_room->wilds && IS_SET(ch->in_room->room2_flags, ROOM_VIRTUAL_ROOM))
				sprintf(buf, "Gave token %s(%ld) to wilds room %ld @ (%ld, %ld)\n\r", token_index->name, token_index->vnum, ch->in_room->wilds->uid, ch->in_room->x, ch->in_room->y);
			else if( ch->in_room->source )
				sprintf(buf, "Gave token %s(%ld) to clone room %ld ID(%lu:%lu)\n\r", token_index->name, token_index->vnum, ch->in_room->source->vnum, ch->in_room->id[0], ch->in_room->id[1]);
			else
				sprintf(buf, "Gave token %s(%ld) to room %ld\n\r", token_index->name, token_index->vnum, ch->in_room->vnum);
			send_to_char(buf, ch);

		} else if (!str_cmp(arg, "junk")) {
			if ((token = get_token_room(ch->in_room, vnum, count)) == NULL) {
				send_to_char("Token not found on object.\n\r", ch);
				return;
			}

			if( ch->in_room->wilds && IS_SET(ch->in_room->room2_flags, ROOM_VIRTUAL_ROOM))
				sprintf(buf, "Removed token %s(%ld.%ld) from wilds room %ld @ (%ld, %ld)\n\r", token->name, count, token->pIndexData->vnum, ch->in_room->wilds->uid, ch->in_room->x, ch->in_room->y);
			else if( ch->in_room->source )
				sprintf(buf, "Removed token %s(%ld.%ld) from clone room %ld ID(%lu:%lu)\n\r", token->name, count, token->pIndexData->vnum, ch->in_room->source->vnum, ch->in_room->id[0], ch->in_room->id[1]);
			else
				sprintf(buf, "Removed token %s(%ld.%ld) from room %ld\n\r", token->name, count, token->pIndexData->vnum, ch->in_room->vnum);
			send_to_char(buf, ch);

			token_from_room(token);
			free_token(token);
		} else
			send_to_char("Syntax:  token <give|junk> room [#.]<vnum>\n\r", ch);

	}
	else
	{
		send_to_char("Syntax:  token <give|junk> char <character> [#.]<vnum>\n\r", ch);
		send_to_char("         token <give|junk> obj <object> [#.]<vnum>\n\r", ch);
		send_to_char("         token <give|junk> room [#.]<vnum>\n\r", ch);
	}

}


/* Load an area from an .are file into memory. The same exact function used
   in boot_db is used here. This allows us to build areas on the testport
   and import them without shutting down the game. Extreme care should be taken
   with this command, since it can be quite a performance drain.
*/
void do_aload(CHAR_DATA *ch, char *argument)
{
    char arg[MSL];
    FILE *fp;
    AREA_DATA *area;
    LLIST_AREA_DATA *link;

    argument = one_argument(argument, arg);

    /* Check to see if the area is loaded in already. If it is, free it
       from memory and reload it. Make sure to update all object and mob
       pIndexData pointers and room area pointers. */
    for (area = area_first; area != NULL; area = area->next) {
	if (!str_cmp(area->file_name, argument))
	    break;
    }

    /* The simpler case - the area is not a current area. */
    if (area == NULL) {
	if ((fp = fopen(arg, "r")) == NULL) {
	    send_to_char("Area file not found.\n\r", ch);
	    return;
	}

	link = (LLIST_AREA_DATA *)alloc_mem(sizeof(LLIST_AREA_DATA));
	if( list_appendlink(loaded_areas, link) && (area = read_area_new(fp))) {
		area->next = NULL;

		area_last->next = area;
		area_last = area;

		// Add to script usable list
		link->area = area;
		link->uid = area->uid;

		act("Loaded area $T.", ch, NULL, NULL, NULL, NULL, NULL, area->name, TO_CHAR);
	} else
		free_mem( link, sizeof(LLIST_AREA_DATA));
	fclose(fp);
    } else {
	/* Syn - will add in replacement of current area when I have time. */
	send_to_char("Area already exists.\n\r", ch);
    }
}

void do_immflag(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch)) {
       bug("NPC tried to change imm flag", 0);
       return;
    }

    if (argument[0] == '\0') {
	send_to_char("Syntax:  immflag [flag]\n\r", ch);
	return;
    }

    if (strlen_no_colours(argument) > 12)
    {
	send_to_char("That flag is too long. Must be no more than 12 characters, not counting colour codes.\n\r", ch);
	return;
    }

    free_string(ch->pcdata->immortal->imm_flag);
    ch->pcdata->immortal->imm_flag = str_dup(argument);
    act("Your immortal flag has been set to $T.", ch, NULL, NULL, NULL, NULL, NULL, argument, TO_CHAR);
}

