/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
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
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "db.h"


void do_multi(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MSL];
    CHAR_DATA *mob;
    int pneuma_cost;
    int i;

    if (IS_NPC(ch))
	return;

    for (mob = ch->in_room->people; mob; mob = mob->next_in_room)
    {
	if (IS_NPC(mob) && IS_SET(mob->act, ACT_TRAIN))
	    break;
    }

    if (mob == NULL)
    {
	send_to_char("You must seek a trainer first.\n\r", ch);
	return;
    }

    if (ch->level < MAX_CLASS_LEVEL)
    {
	send_to_char("You must first complete your current career.\n\r",ch);
	return;
    }

    if (ch->tot_level == LEVEL_HERO || IS_IMMORTAL(ch))
    {
	send_to_char("You have learned all that you can at this time.\n\r",ch);
	return;
    }

    if (argument[0] == '\0')
    {
 	show_multiclass_choices(ch, ch);
	return;
    }

    i = 0;
    if (ch->pcdata->sub_class_mage 		!= -1) i++;
    if (ch->pcdata->sub_class_cleric 		!= -1) i++;
    if (ch->pcdata->sub_class_thief 		!= -1) i++;
    if (ch->pcdata->sub_class_warrior 		!= -1) i++;
    if (ch->pcdata->second_sub_class_mage 	!= -1) i++;
    if (ch->pcdata->second_sub_class_cleric	!= -1) i++;
    if (ch->pcdata->second_sub_class_thief 	!= -1) i++;
    if (ch->pcdata->second_sub_class_warrior 	!= -1) i++;

    switch (i)
    {
	case 1:		pneuma_cost = 0;	break;
	case 2:		pneuma_cost = 500;	break;
	case 3: 	pneuma_cost = 1000;	break;
	case 5: 	pneuma_cost = 2500;	break;
	case 6:		pneuma_cost = 5000;	break;
	case 7: 	pneuma_cost = 10000;	break;
        default:	pneuma_cost = 500;	break;
    }

    for (i = 0; i < MAX_SUB_CLASS; i++)
    {
	if (!str_cmp(argument, sub_class_table[i].name[ch->sex]))
	    break;
    }

    if (i == MAX_SUB_CLASS)
    {
	send_to_char("That's not a subclass.\n\r", ch);
	return;
    }

    if (!can_choose_subclass(ch, i))
    {
	send_to_char("You can't choose that subclass.\n\r", ch);
	return;
    }

    if (ch->pneuma < pneuma_cost)
    {
	sprintf(buf, "You need %d pneuma to multiclass!\n\r", pneuma_cost);
	send_to_char(buf, ch);
	return;
    }

    if (!str_cmp("archmage", argument)
    ||  !str_cmp("geomancer", argument)
    ||  !str_cmp("illusionist", argument))
    {
	ch->pcdata->class_current = CLASS_MAGE;
	ch->pcdata->second_class_mage = CLASS_MAGE;

	if (!str_cmp("archmage", argument))
	{
	    ch->pcdata->sub_class_current = CLASS_MAGE_ARCHMAGE;
	    ch->pcdata->second_sub_class_mage = CLASS_MAGE_ARCHMAGE;
	}

	if (!str_cmp("geomancer", argument))
	{
	    ch->pcdata->sub_class_current = CLASS_MAGE_GEOMANCER;
	    ch->pcdata->second_sub_class_mage = CLASS_MAGE_GEOMANCER;
	}

	if (!str_cmp("illusionist", argument))
	{
	    ch->pcdata->sub_class_current = CLASS_MAGE_ILLUSIONIST;
	    ch->pcdata->second_sub_class_mage = CLASS_MAGE_ILLUSIONIST;
	}
    }

    if (!str_cmp("alchemist", argument)
    ||  !str_cmp("ranger", argument)
    ||  !str_cmp("adept", argument))
    {
	ch->pcdata->class_current = CLASS_CLERIC;
	ch->pcdata->second_class_cleric = CLASS_CLERIC;

	if (!str_cmp("alchemist", argument))
	{
	    ch->pcdata->sub_class_current = CLASS_CLERIC_ALCHEMIST;
	    ch->pcdata->second_sub_class_cleric = CLASS_CLERIC_ALCHEMIST;
	}

	if (!str_cmp("ranger", argument))
	{
	    ch->pcdata->sub_class_current = CLASS_CLERIC_RANGER;
	    ch->pcdata->second_sub_class_cleric = CLASS_CLERIC_RANGER;
	}

	if (!str_cmp("adept", argument))
	{
	    ch->pcdata->sub_class_current = CLASS_CLERIC_ADEPT;
	    ch->pcdata->second_sub_class_cleric = CLASS_CLERIC_ADEPT;
	}
    }

    if (!str_cmp("highwayman", argument)
    || !str_cmp("ninja", argument)
    || !str_cmp("sage", argument))
    {
	ch->pcdata->class_current = CLASS_THIEF;
	ch->pcdata->second_class_thief = CLASS_THIEF;

	if (!str_cmp("highwayman", argument))
	{
	    ch->pcdata->sub_class_current = CLASS_THIEF_HIGHWAYMAN;
	    ch->pcdata->second_sub_class_thief = CLASS_THIEF_HIGHWAYMAN;
	}

	if (!str_cmp("ninja", argument))
	{
	    ch->pcdata->sub_class_current = CLASS_THIEF_NINJA;
	    ch->pcdata->second_sub_class_thief = CLASS_THIEF_NINJA;
	}

	if (!str_cmp("sage", argument))
	{
	    ch->pcdata->sub_class_current = CLASS_THIEF_SAGE;
	    ch->pcdata->second_sub_class_thief = CLASS_THIEF_SAGE;
	}
    }

    if (!str_cmp("warlord", argument)
    ||  !str_cmp("destroyer", argument)
    ||  !str_cmp("crusader", argument))
    {
	ch->pcdata->class_current = CLASS_WARRIOR;
	ch->pcdata->second_class_warrior = CLASS_WARRIOR;

	if (!str_cmp("warlord", argument))
	{
	    ch->pcdata->sub_class_current = CLASS_WARRIOR_WARLORD;
	    ch->pcdata->second_sub_class_warrior = CLASS_WARRIOR_WARLORD;
	}

	if (!str_cmp("destroyer", argument))
	{
	    ch->pcdata->sub_class_current = CLASS_WARRIOR_DESTROYER;
	    ch->pcdata->second_sub_class_warrior = CLASS_WARRIOR_DESTROYER;
	}

	if (!str_cmp("crusader", argument))
	{
	    ch->pcdata->sub_class_current = CLASS_WARRIOR_CRUSADER;
	    ch->pcdata->second_sub_class_warrior = CLASS_WARRIOR_CRUSADER;
	}
    }

    if (!str_cmp("necromancer", argument)
    || !str_cmp("sorcerer", argument)
    || !str_cmp("wizard", argument))
    {
	ch->pcdata->class_current = CLASS_MAGE;
	ch->pcdata->class_mage = CLASS_MAGE;

	if (!str_cmp("necromancer", argument))
	{
	    ch->pcdata->sub_class_current = CLASS_MAGE_NECROMANCER;
	    ch->pcdata->sub_class_mage = CLASS_MAGE_NECROMANCER;
	}

	if (!str_cmp("sorcerer", argument))
	{
	    ch->pcdata->sub_class_current = CLASS_MAGE_SORCERER;
	    ch->pcdata->sub_class_mage = CLASS_MAGE_SORCERER;
	}

	if (!str_cmp("wizard", argument))
	{
	    ch->pcdata->sub_class_current = CLASS_MAGE_WIZARD;
	    ch->pcdata->sub_class_mage = CLASS_MAGE_WIZARD;
	}
    }

    if (!str_cmp("witch", argument)
    ||  !str_cmp("warlock", argument)
    ||  !str_cmp("druid", argument)
    ||  !str_cmp("monk", argument))
    {
	ch->pcdata->class_current = CLASS_CLERIC;
	ch->pcdata->class_cleric = CLASS_CLERIC;
	if (!str_cmp("warlock", argument)
	||  !str_cmp("witch", argument))
	{
	    ch->pcdata->sub_class_current = CLASS_CLERIC_WITCH;
	    ch->pcdata->sub_class_cleric = CLASS_CLERIC_WITCH;
	}

	if (!str_cmp("druid", argument))
	{
	    ch->pcdata->sub_class_current = CLASS_CLERIC_DRUID;
	    ch->pcdata->sub_class_cleric = CLASS_CLERIC_DRUID;
	}

	if (!str_cmp("monk", argument))
	{
	    ch->pcdata->sub_class_current = CLASS_CLERIC_MONK;
	    ch->pcdata->sub_class_cleric = CLASS_CLERIC_MONK;
	}
    }

    if (!str_cmp("assassin", argument)
    || !str_cmp("rogue", argument)
    || !str_cmp("bard", argument))
    {
	ch->pcdata->class_current = CLASS_THIEF;
	ch->pcdata->class_thief = CLASS_THIEF;

	if (!str_cmp("assassin", argument))
	{
	    ch->pcdata->sub_class_current = CLASS_THIEF_ASSASSIN;
	    ch->pcdata->sub_class_thief = CLASS_THIEF_ASSASSIN;
	}

	if (!str_cmp("rogue", argument))
	{
	    ch->pcdata->sub_class_current = CLASS_THIEF_ROGUE;
	    ch->pcdata->sub_class_thief = CLASS_THIEF_ROGUE;
	}

	if (!str_cmp("bard", argument))
	{
	    ch->pcdata->sub_class_current = CLASS_THIEF_BARD;
	    ch->pcdata->sub_class_thief = CLASS_THIEF_BARD;
	}
    }

    if (!str_cmp("marauder", argument)
    || !str_cmp("gladiator", argument)
    || !str_cmp("paladin", argument))
    {
	ch->pcdata->class_current = CLASS_WARRIOR;
	ch->pcdata->class_warrior = CLASS_WARRIOR;

	if (!str_cmp("marauder", argument))
	{
	    ch->pcdata->sub_class_current = CLASS_WARRIOR_MARAUDER;
	    ch->pcdata->sub_class_warrior = CLASS_WARRIOR_MARAUDER;
	}

	if (!str_cmp("gladiator", argument))
	{
	    ch->pcdata->sub_class_current = CLASS_WARRIOR_GLADIATOR;
	    ch->pcdata->sub_class_warrior = CLASS_WARRIOR_GLADIATOR;
	}

	if (!str_cmp("paladin", argument))
	{
	    ch->pcdata->sub_class_current = CLASS_WARRIOR_PALADIN;
	    ch->pcdata->sub_class_warrior = CLASS_WARRIOR_PALADIN;
	}
    }

    ch->pneuma -= pneuma_cost;
    ch->level = 1;
    ch->exp = 0;
    ch->tot_level++;
    group_add(ch, class_table[ch->pcdata->class_current].base_group, TRUE);
    group_add(ch, sub_class_table[ch->pcdata->sub_class_current].default_group, TRUE);
    sprintf(buf2, "%s", sub_class_table[ch->pcdata->sub_class_current].name[ch->sex]);
    buf2[0] = UPPER(buf2[0]);
    sprintf(buf, "All congratulate %s, who is now a%s %s!",
        ch->name, (buf2[0] == 'A' || buf2[0] == 'I' || buf2[0] == 'E' || buf2[0] == 'U'
	    || buf2[0] == '0') ? "n" : "", buf2);
    crier_announce(buf);
    double_xp(ch);
}


void show_multiclass_choices(CHAR_DATA *ch, CHAR_DATA *looker)
{
    char buf[MSL];
    char buf2[MSL];
    int i;

    if (ch == looker)
	send_to_char("{YYou are skilled in the following subclasses:{x\n\r", looker);
    else
	act("{Y$N is skilled in the following subclasses:{x", looker, ch, NULL, NULL, NULL, NULL, NULL, TO_CHAR);

    sprintf(buf, "{x");

    for (i = 0; i < MAX_SUB_CLASS; i++)
    {
	switch (sub_class_table[i].class)
	{
	    case CLASS_MAGE:
		if (get_profession(ch, sub_class_table[i].remort ? SECOND_SUBCLASS_MAGE : SUBCLASS_MAGE) == i)
		{
		    sprintf(buf2, "{R%s{x\n\r", sub_class_table[i].name[ch->sex]);
		    buf2[2] = UPPER(buf2[2]);
		    strcat(buf, buf2);
		}

		break;

	    case CLASS_CLERIC:
		if (get_profession(ch, sub_class_table[i].remort ? SECOND_SUBCLASS_CLERIC : SUBCLASS_CLERIC) == i)
		{
		    sprintf(buf2, "{R%s{x\n\r", sub_class_table[i].name[ch->sex]);
		    buf2[2] = UPPER(buf2[2]);
		    strcat(buf, buf2);
		}

		break;

	    case CLASS_THIEF:
		if (get_profession(ch, sub_class_table[i].remort ? SECOND_SUBCLASS_THIEF : SUBCLASS_THIEF) == i)
		{
		    sprintf(buf2, "{R%s{x\n\r", sub_class_table[i].name[ch->sex]);
		    buf2[2] = UPPER(buf2[2]);
		    strcat(buf, buf2);
		}

		break;

	    case CLASS_WARRIOR:
		if (get_profession(ch, sub_class_table[i].remort ? SECOND_SUBCLASS_WARRIOR : SUBCLASS_WARRIOR) == i)
		{
		    sprintf(buf2, "{R%s{x\n\r", sub_class_table[i].name[ch->sex]);
		    buf2[2] = UPPER(buf2[2]);
		    strcat(buf, buf2);
		}

		break;
	}
    }

    send_to_char(buf, looker);

    if (ch == looker)
	sprintf(buf, "\n\r{YYou may multiclass to:{x\n\r");
    else
	sprintf(buf, "\n\r{Y%s may multiclass to:{x\n\r", ch->name);

    for (i = 0; i < MAX_SUB_CLASS; i++)
    {
	if (can_choose_subclass(ch, i))
	{
	    sprintf(buf2, "{x%s\n\r", sub_class_table[i].name[ch->sex]);
	    buf2[2] = UPPER(buf2[2]);
	    strcat(buf, buf2);
	}
    }

    send_to_char(buf, looker);
}


bool can_choose_subclass(CHAR_DATA *ch, int subclass)
{
    char buf[MSL];
    int prof;

    // 1st mort
    if (subclass >= CLASS_WARRIOR_MARAUDER && subclass <= CLASS_THIEF_BARD)
    {
	// Check if they've done it before
	switch (sub_class_table[subclass].class)
	{
	    case CLASS_MAGE:
	        if (get_profession(ch, CLASS_MAGE) != -1)
		    return FALSE;
		    break;

	    case CLASS_CLERIC:
	        if (get_profession(ch, CLASS_CLERIC) != -1)
		    return FALSE;
		    break;

	    case CLASS_THIEF:
	        if (get_profession(ch, CLASS_THIEF) != -1)
		    return FALSE;
		    break;

	    case CLASS_WARRIOR:
	        if (get_profession(ch, CLASS_WARRIOR) != -1)
		    return FALSE;
		    break;
	}

	// Check if they fit align
	switch (sub_class_table[subclass].alignment)
	{
	    case ALIGN_EVIL:
	        if (IS_GOOD(ch))
		    return FALSE;
		break;

            case ALIGN_GOOD:
		if (IS_EVIL(ch))
		    return FALSE;
		break;
	}

	return TRUE;
    }
    else // Remort
    {
	if (!IS_REMORT(ch) && ch->tot_level != LEVEL_HERO)
	    return FALSE;

	switch (sub_class_table[subclass].class)
	{
	    case CLASS_MAGE:
	        if (get_profession(ch, SECOND_SUBCLASS_MAGE) != -1)
		    return FALSE;

                prof = get_profession(ch, SUBCLASS_MAGE);

		if (prof == sub_class_table[subclass].prereq[0]
		||  prof == sub_class_table[subclass].prereq[1])
		    return TRUE;
		    break;

	    case CLASS_CLERIC:
	        if (get_profession(ch, SECOND_SUBCLASS_CLERIC) != -1)
		    return FALSE;

		prof = get_profession(ch, SUBCLASS_CLERIC);

		if (prof == sub_class_table[subclass].prereq[0]
		||  prof == sub_class_table[subclass].prereq[1])
		    return TRUE;
		    break;

	    case CLASS_THIEF:
	        if (get_profession(ch, SECOND_SUBCLASS_THIEF) != -1)
		    return FALSE;

		prof = get_profession(ch, SUBCLASS_THIEF);

		if (prof == sub_class_table[subclass].prereq[0]
		||  prof == sub_class_table[subclass].prereq[1])
		    return TRUE;
		    break;

	    case CLASS_WARRIOR:
	        if (get_profession(ch, SECOND_SUBCLASS_WARRIOR) != -1)
		    return FALSE;

		prof = get_profession(ch, SUBCLASS_WARRIOR);

		if (prof == sub_class_table[subclass].prereq[0]
		||  prof == sub_class_table[subclass].prereq[1])
		    return TRUE;
		    break;

	    default:
		    return FALSE;
	}

	return FALSE;
    }

    sprintf(buf, "can_choose_subclass: invalid subclass for %s[%d]", ch->name, subclass);
    bug(buf, 0);
    return FALSE;
}


// Train a stat
void do_train(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *mob;
    AFFECT_DATA *af;
    OBJ_DATA *obj;
    sh_int stat = - 1;
    char *pOutput = NULL;
    int cost;
    int max_hit;
    int max_mana;
    int max_move;
    int mod_hit;
    int mod_mana;
    int mod_move;
    int sn;

    if (IS_NPC(ch))
	return;

    // Train up a skill for a ridiculous cost
    if ((str_cmp(argument, "str")
	 && str_cmp(argument, "wis")
	 && str_cmp(argument, "int")
	 && str_cmp(argument, "dex")
	 && str_cmp(argument, "wis")
	 && str_cmp(argument, "con")
	 && str_cmp(argument, "hp")
	 && str_cmp(argument, "mana")
	 && str_cmp(argument, "move")) &&
	(sn = skill_lookup(argument)) > 0)

    {
	for (mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room)
	{
	    if (IS_NPC(mob) && (mob->pIndexData->vnum == VNUM_QUESTOR_1
            ||  mob->pIndexData->vnum == VNUM_QUESTOR_2))
		break;
	}

        if (mob == NULL)
	{
	    send_to_char("There is nobody here to help you do that.\n\r", ch);
	    return;
	}

	if (ch->tot_level < 2 * MAX_CLASS_LEVEL + 1)
	{
	    sprintf(buf, "%s, you must be of at least the third class to do this.", pers(ch, mob));
	    do_say(mob, buf);
	    return;
	}

	if (get_skill(ch, sn) == 0)
	{
	    sprintf(buf, "You know nothing of that skill, %s!\n\r", pers(ch, mob));
	    do_say(mob, buf);
	    return;
	}
	else
	if (get_skill(ch, sn) < 75)
	{
	    sprintf(buf, "You must come back when you have studied this skill to the utmost through mundane means, %s.\n\r",
	        pers(ch, mob));
	    do_say(mob, buf);
	    return;
	}
	else
        if (get_skill(ch, sn) >= 90)
	{
	    sprintf(buf, "Even I can't help your mastery of %s past this point, %s.",
	        skill_table[sn].name, pers(ch, mob));
	    do_say(mob, buf);
	    return;
	}

	cost = ((get_skill(ch, sn)) * 3)/20;
	if (cost <= ch->train)
	{
	    ch->train -= cost;
	    act("$n trains $t with $N.", ch, mob, NULL, NULL, NULL, skill_table[sn].name, NULL, TO_ROOM);
	    act("You train $t with $N.", ch, mob, NULL, NULL, NULL, skill_table[sn].name, NULL, TO_CHAR);
	    act("{YYou feel your mastery of $t soaring to new heights!{x", ch, NULL, NULL, NULL, NULL, skill_table[sn].name, NULL, TO_CHAR);
	    ch->pcdata->learned[sn]++;
	}
	else
	{
	    sprintf(buf, "It would take %d trains to train that skill, %s.", cost, pers(ch, mob));
	    do_say(mob, buf);
	}

	return;
    }

    for (mob = ch->in_room->people; mob; mob = mob->next_in_room)
    {
	if (IS_NPC(mob) && IS_SET(mob->act, ACT_TRAIN))
	    break;
    }

    if (mob == NULL)
    {
	send_to_char("You can't do that here.\n\r", ch);
	return;
    }

    if(p_percent_trigger(mob, NULL, NULL, NULL, ch, NULL, NULL, NULL, NULL, TRIG_PRETRAIN, NULL))
	return;

    if (argument[0] == '\0')
    {
	sprintf(buf, "You have %d training sessions.\n\r", ch->train);
	send_to_char(buf, ch);
    }

    cost = 1;

    mod_hit = 0;
    mod_mana = 0;
    mod_move = 0;

    max_hit = pc_race_table[ch->race].max_vital_stats[MAX_HIT];
    max_mana = pc_race_table[ch->race].max_vital_stats[MAX_MANA];
    max_move = pc_race_table[ch->race].max_vital_stats[MAX_MOVE];

    for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
	if (obj->wear_loc != WEAR_NONE)
	{
	    for (af = obj->affected; af != NULL; af = af->next)
	    {
		if (af->location == APPLY_HIT)
		    mod_hit += af->modifier;
		if (af->location == APPLY_MANA)
		    mod_mana += af->modifier;
		if (af->location == APPLY_MOVE)
		    mod_move += af->modifier;
	    }
	}
    }

    max_hit += mod_hit;
    max_mana += mod_mana;
    mod_move += mod_move;

    if (!str_cmp(argument, "str"))
    {
	if (class_table[ch->pcdata->class_current].attr_prime == STAT_STR
	|| ch->pcdata->class_mage != -1)
	    cost    = 1;
	stat        = STAT_STR;
	pOutput     = "strength";
    }

    else if (!str_cmp(argument, "int"))
    {
	if (class_table[ch->pcdata->class_current].attr_prime == STAT_INT
	|| ch->pcdata->class_mage != -1)
	    cost    = 1;
	stat	    = STAT_INT;
	pOutput     = "intelligence";
    }

    else if (!str_cmp(argument, "wis"))
    {
	if (class_table[ch->pcdata->class_current].attr_prime == STAT_WIS
	|| ch->pcdata->class_cleric != -1)
	    cost    = 1;
	stat	    = STAT_WIS;
	pOutput     = "wisdom";
    }

    else if (!str_cmp(argument, "dex"))
    {
	if (class_table[ch->pcdata->class_current].attr_prime == STAT_DEX
	|| ch->pcdata->class_thief != -1)
	    cost    = 1;
	stat  	    = STAT_DEX;
	pOutput     = "dexterity";
    }

    else if (!str_cmp(argument, "con"))
    {
	if (class_table[ch->pcdata->class_current].attr_prime == STAT_CON)
	    cost    = 1;
	stat	    = STAT_CON;
	pOutput     = "constitution";
    }

    else if (!str_cmp(argument, "hp"))
	    cost = 1;

    else if (!str_cmp(argument, "mana"))
	    cost = 1;

    else if (!str_cmp(argument, "move"))
	    cost = 1;

    else
    {
	strcpy(buf, "You can train:");
	if (ch->perm_stat[STAT_STR] < get_max_train(ch,STAT_STR))
	    strcat(buf, " str");
	if (ch->perm_stat[STAT_INT] < get_max_train(ch,STAT_INT))
	    strcat(buf, " int");
	if (ch->perm_stat[STAT_WIS] < get_max_train(ch,STAT_WIS))
	    strcat(buf, " wis");
	if (ch->perm_stat[STAT_DEX] < get_max_train(ch,STAT_DEX))
	    strcat(buf, " dex");
	if (ch->perm_stat[STAT_CON] < get_max_train(ch,STAT_CON))
	    strcat(buf, " con");
	if (ch->max_hit < max_hit)
	    strcat(buf, " hp");
	if (ch->max_mana < max_mana)
	    strcat(buf, " mana");
        if (ch->max_move < max_move)
	    strcat(buf, " move");

	if (buf[strlen(buf)-1] != ':')
	{
	    strcat(buf, ".\n\r");
	    send_to_char(buf, ch);
	}
	else
	{
	    act("You have nothing left to train.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
	}

	return;
    }

    if (!str_cmp("hp",argument))
    {
    	if (cost > ch->train)
    	{
       	    send_to_char("You don't have enough training sessions.\n\r", ch);
            return;
        }

     	if (ch->max_hit >= max_hit) {
	        send_to_char("Your body can't get any tougher.\n\r", ch);
	        return;
        }

		ch->train -= cost;
        ch->pcdata->perm_hit += 10;
        ch->max_hit += 10;
        ch->hit += 10;
        act("Your health increases!",ch,NULL,NULL, NULL, NULL, NULL, NULL,TO_CHAR);
        act("$n's health increases!",ch,NULL,NULL, NULL, NULL, NULL, NULL,TO_ROOM);
        return;
    }

    if (!str_cmp("mana",argument))
    {
        if (cost > ch->train)
        {
            send_to_char("You don't have enough training sessions.\n\r", ch);
            return;
        }

	if (ch->max_mana >= max_mana) {
	    send_to_char("Your body can't get any tougher.\n\r", ch);
	    return;
	}

	ch->train -= cost;
        ch->pcdata->perm_mana += 10;
        ch->max_mana += 10;
        ch->mana +=10;
        act("Your mana increases!",ch,NULL,NULL, NULL, NULL, NULL, NULL,TO_CHAR);
        act("$n's mana increases!",ch,NULL,NULL, NULL, NULL, NULL, NULL,TO_ROOM);

	return;
    }

    if (!str_cmp("move",argument))
    {
        if (cost > ch->train)
        {
            send_to_char("You don't have enough training sessions.\n\r", ch);
            return;
        }

	if (ch->max_move >= max_move) {
	    send_to_char("Your body can't get any more durable.\n\r", ch);
	    return;
	}

	ch->train -= cost;
        ch->pcdata->perm_move += 10;
        ch->max_move += 10;
        ch->move += 10;
        act("Your stamina increases!",ch,NULL,NULL, NULL, NULL, NULL, NULL,TO_CHAR);
        act("$n's stamina increases!",ch,NULL,NULL, NULL, NULL, NULL, NULL,TO_ROOM);
        return;
    }

    if (ch->perm_stat[stat]  >= get_max_train(ch,stat))
    {
	act("Your $T is already at maximum.", ch, NULL, NULL, NULL, NULL, NULL, pOutput, TO_CHAR);
	return;
    }

    if (cost > ch->train)
    {
	send_to_char("You don't have enough training sessions.\n\r", ch);
	return;
    }

    ch->train		-= cost;

	add_perm_stat(ch, stat, 1);
    act("Your $T increases!", ch, NULL, NULL, NULL, NULL, NULL, pOutput, TO_CHAR);
    act("$n's $T increases!", ch, NULL, NULL, NULL, NULL, NULL, pOutput, TO_ROOM);
}


void do_convert(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *trainer;

    if (ch == NULL)
    {
        bug("do_convert: NULL ch", 0);
        return;
    }

    if (IS_IMMORTAL(ch))
    {
        send_to_char("Why would an immortal need to convert?\n\r", ch);
        return;
    }

    if (IS_NPC(ch))
	return;

    for (trainer = ch->in_room->people; trainer != NULL; trainer = trainer->next_in_room)
    {
	if (IS_NPC(trainer)
	&&  (IS_SET(trainer->act, ACT_TRAIN) || IS_SET(trainer->act, ACT_PRACTICE)))
	    break;
    }

    if (trainer == NULL || !can_see(ch,trainer))
    {
	send_to_char("You can't do that here.\n\r",ch);
	return;
    }

    one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	do_function(trainer, &do_say, "What is it that you would like to convert?");
	return;
    }

    if (!str_prefix(arg,"practices"))
    {

	if (ch->practice < 20)
	{
	    act("{R$N tells you 'You don't have enough practices. You must have 20 practices!'{x",
		ch,trainer, NULL, NULL, NULL, NULL, NULL,TO_CHAR);
	    return;
	}

	act("$N helps you apply your practice to training.",
		ch,trainer, NULL, NULL, NULL, NULL, NULL,TO_CHAR);
	ch->practice -= 20;
	ch->train++;
	return;
    }

    if (!str_prefix(arg,"trains"))
    {
	if (ch->train < 1)
	{
	    act("{R$N tells you 'You don't have any trains to convert into pracs!'{x",
		ch,trainer, NULL, NULL, NULL, NULL, NULL,TO_CHAR);
	    return;
	}

	act("$N helps you apply your practice to training.",
		ch,trainer, NULL, NULL, NULL, NULL, NULL,TO_CHAR);
	ch->practice += 20;
	ch->train--;
	return;
    }

    send_to_char("Syntax:  convert <practices|trains>\n\r", ch);
}


void do_spells(CHAR_DATA *ch, char *argument)
{
	BUFFER *buffer;

	bool found = FALSE;
	char buf[MAX_STRING_LENGTH];
	char arg[MSL];
	int i;
	char color, *name;
	int skill, rating, mod, level;
	SKILL_ENTRY *entry;

	if (ch == NULL) {
		bug("do_skills: NULL ch pointer.", 0);
		return;
	}

	if (IS_NPC(ch)) return;

	argument = one_argument( argument, arg );

	buffer = new_buf();
	add_buf(buffer, "\n\r{B [ {w# {B] [ {wLvl{B ]   [ {wSpell Name{B ]                 [ {w%{B ]{x\n\r");
	i = 1;

	// Show spells people lost
 	if (!str_cmp(arg, "negated")) {
		for(entry = ch->sorted_spells; entry; entry = entry->next) {
			rating = skill_entry_rating(ch, entry);

			if(rating < 0) {
				color = ( IS_IMMORTAL(ch) && IS_VALID(entry->token) ) ? 'G' : 'Y';

				sprintf(buf, " %3d     ---       {%c%-26s    {D%d%%{x\n\r", i++, color, skill_entry_name(entry), -rating);
				add_buf(buffer,buf);
				found = TRUE;
			}
		}
	} else {
		for(entry = ch->sorted_spells; entry; entry = entry->next) {
			skill = skill_entry_rating(ch, entry);
			mod = skill_entry_mod(ch, entry);
			level = skill_entry_level(ch, entry);	// Negate level implies they do not know it yet.
			name = skill_entry_name(entry);

			if( skill < 1 ) continue;

			if( !arg[0] || !str_prefix(arg, name) ) {

				color = ( IS_IMMORTAL(ch) && IS_VALID(entry->token) ) ? 'G' : 'Y';

				// Don't have it yet
				if( level < 0 )
					sprintf(buf, " %3d     %3d       {%c%-26s    {xN/A\n\r", i, -level, color, name);
				else {
					rating = skill + mod;
					rating = URANGE(0,rating,100);

					if( rating >= 100 ) {	// MASTER
						if( mod )
							sprintf(buf, " %3d     %3d       {%c%-26s    {MMaster {W(%+d%%){x\n\r", i, level, color, name, mod);
						else
							sprintf(buf, " %3d     %3d       {%c%-26s    {MMaster{x\n\r", i, level, color, name);
					} else if( mod )
						sprintf(buf, " %3d     %3d       {%c%-26s    {G%d%% {W(%+d%%){x\n\r", i, level, color, name, rating, mod);
					else
						sprintf(buf, " %3d     %3d       {%c%-26s    {G%d%%{x\n\r", i, level, color, name, rating);
				}

				add_buf(buffer,buf);
				i++;
				found = TRUE;
			}

		}
	}

	if (!found)
		send_to_char("No spells found.\n\r", ch );
	else
		page_to_char(buf_string(buffer),ch);
	free_buf(buffer);
}


void do_skills(CHAR_DATA *ch, char *argument)
{
	BUFFER *buffer;

	bool found = FALSE;
	char buf[MAX_STRING_LENGTH];
	char arg[MSL];
	int i;
	char color, *name;
	int skill, rating, mod, level;
	SKILL_ENTRY *entry;

	if (ch == NULL) {
		bug("do_skills: NULL ch pointer.", 0);
		return;
	}

	if (IS_NPC(ch)) return;

	argument = one_argument( argument, arg );

	buffer = new_buf();
	add_buf(buffer, "\n\r{B [ {w# {B] [ {wLvl{B ]   [ {wSkill Name{B ]                 [ {w%{B ]{x\n\r");
	i = 1;

	// Show skills people lost
 	if (!str_cmp(arg, "negated")) {
		for(entry = ch->sorted_skills; entry; entry = entry->next) {
			rating = skill_entry_rating(ch, entry);

			if(rating < 0) {
				color = ( IS_IMMORTAL(ch) && IS_VALID(entry->token) ) ? 'G' : 'Y';

				sprintf(buf, " %3d     ---       {%c%-26s    {D%d%%{x\n\r", i++, color, skill_entry_name(entry), -rating);
				add_buf(buffer,buf);
				found = TRUE;
			}
		}
	} else {
		for(entry = ch->sorted_skills; entry; entry = entry->next) {
			skill = skill_entry_rating(ch, entry);
			mod = skill_entry_mod(ch, entry);
			level = skill_entry_level(ch, entry);	// Negate level implies they do not know it yet.
			name = skill_entry_name(entry);

			if( skill < 1 ) continue;

			if( !arg[0] || !str_prefix(arg, name) ) {

				color = ( IS_IMMORTAL(ch) && IS_VALID(entry->token) ) ? 'G' : 'Y';

				// Don't have it yet
				if( level < 0 )
					sprintf(buf, " %3d     %3d       {%c%-26s    {xN/A\n\r", i, -level, color, name);
				else {
					rating = skill + mod;
					rating = URANGE(0,rating,100);

					if( rating >= 100 ) {	// MASTER
						if( mod )
							sprintf(buf, " %3d     %3d       {%c%-26s    {MMaster {W(%+d%%){x\n\r", i, level, color, name, mod);
						else
							sprintf(buf, " %3d     %3d       {%c%-26s    {MMaster{x\n\r", i, level, color, name);
					} else if( mod )
						sprintf(buf, " %3d     %3d       {%c%-26s    {G%d%% {W(%+d%%){x\n\r", i, level, color, name, rating, mod);
					else
						sprintf(buf, " %3d     %3d       {%c%-26s    {G%d%%{x\n\r", i, level, color, name, rating);
				}

				add_buf(buffer,buf);
				i++;
				found = TRUE;
			}

		}
	}

	if (!found)
		send_to_char("No skills found.\n\r", ch );
	else
		page_to_char(buf_string(buffer),ch);
	free_buf(buffer);
}


long exp_per_level(CHAR_DATA *ch, long points)
{
    double expl;

    if (IS_NPC(ch))
	return 1000;

    expl = exp_per_level_table[ch->tot_level].exp;

    if (IS_REMORT(ch))
        expl = 3 * expl / 2;

    if (ch->level == MAX_CLASS_LEVEL)
	expl = 0;

    return expl;
}


void check_improve( CHAR_DATA *ch, int sn, bool success, int multiplier )
{
    int chance;
    char buf[100];
    int this_class;

    if (IS_NPC(ch))
	return;

    if (IS_SOCIAL(ch))
	return;

    this_class = get_this_class(ch, sn);

    if (get_skill(ch, sn) == 0
    ||  skill_table[sn].rating[this_class] == 0
    ||  ch->pcdata->learned[sn] == 0
    ||  ch->pcdata->learned[sn] == 100)
	return;

    // check to see if the character has a chance to learn
    chance      = 10 * int_app[get_curr_stat(ch, STAT_INT)].learn;
    multiplier  = UMAX(multiplier,1);
//    multiplier  = UMIN(multiplier + 3, 8);
    chance     /= (multiplier * skill_table[sn].rating[this_class] * 4);
    chance     += ch->level;

    if (number_range(1,1000) > chance)
	return;

    // now that the character has a CHANCE to learn, see if they really have
    if (success)
    {
	chance = URANGE(2, 100 - ch->pcdata->learned[sn], 25);
	if (number_percent() < chance)
	{
	    sprintf(buf,"{WYou have become better at %s!{x\n\r", skill_table[sn].name);
	    send_to_char(buf,ch);
	    ch->pcdata->learned[sn]++;
	    gain_exp(ch, 2 * skill_table[sn].rating[this_class]);
	}
    }
    else
    {
	chance = URANGE(5, ch->pcdata->learned[sn]/2, 30);
	if (number_percent() < chance)
	{
	    sprintf(buf,
		"{WYou learn from your mistakes, and your %s skill improves.{x\n\r",
		skill_table[sn].name);
	    send_to_char(buf, ch);
	    ch->pcdata->learned[sn] += number_range(1,3);
	    ch->pcdata->learned[sn] = UMIN(ch->pcdata->learned[sn],100);
	    gain_exp(ch,2 * skill_table[sn].rating[ch->pcdata->class_current]);
	}
    }
}


int group_lookup( const char *name )
{
    int gn;

    for ( gn = 0; gn < MAX_GROUP; gn++ )
    {
        if ( group_table[gn].name == NULL )
            break;
        if ( LOWER(name[0]) == LOWER(group_table[gn].name[0])
        &&   !str_prefix( name, group_table[gn].name ) )
            return gn;
    }

    return -1;
}


void gn_add(CHAR_DATA *ch, int gn)
{
    int i;

    ch->pcdata->group_known[gn] = TRUE;
    for (i = 0; i < MAX_IN_GROUP; i++)
    {
        if (group_table[gn].spells[i] == NULL)
            break;

        group_add(ch,group_table[gn].spells[i],FALSE);
    }
}


void gn_remove( CHAR_DATA *ch, int gn)
{
    int i;

    ch->pcdata->group_known[gn] = FALSE;

    for ( i = 0; i < MAX_IN_GROUP; i ++)
    {
	if (group_table[gn].spells[i] == NULL)
	    break;

	group_remove(ch,group_table[gn].spells[i]);
    }
}


void group_add( CHAR_DATA *ch, const char *name, bool deduct)
{
    int sn;
    int gn;

    if (IS_NPC(ch))
	return;

    sn = skill_lookup(name);

    if (sn != -1)
    {
		if (ch->pcdata->learned[sn] <= 0) { /* i.e. not known */
			ch->pcdata->learned[sn] = 1;

			if( skill_table[sn].spell_fun == spell_null ) {
				if( skill_entry_findsn( ch->sorted_skills, sn) == NULL)
					skill_entry_addskill(ch, sn, NULL);
			} else {
				if( skill_entry_findsn( ch->sorted_spells, sn) == NULL)
					skill_entry_addspell(ch, sn, NULL);
			}
		}

		return;
    }

    /* now check groups */
    gn = group_lookup(name);
    if (gn != -1)
    {
	if (ch->pcdata->group_known[gn] == FALSE)
	    ch->pcdata->group_known[gn] = TRUE;

	gn_add(ch,gn); /* make sure all skills in the group are known */
    }
}


void group_remove(CHAR_DATA *ch, const char *name)
{
    int sn;
    int gn;

    sn = skill_lookup(name);

    if (sn != -1)
    {
	ch->pcdata->learned[sn] = 0;
	if( skill_table[sn].spell_fun == spell_null )
		skill_entry_removeskill(ch,sn, NULL);
	else
		skill_entry_removespell(ch,sn, NULL);
	return;
    }

    gn = group_lookup(name);

    if (gn != -1 && ch->pcdata->group_known[gn] == TRUE)
    {
	ch->pcdata->group_known[gn] = FALSE;
	gn_remove(ch,gn);  /* be sure to call gn_add on all remaining groups */
    }
}


void do_practice( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	char arg[MSL];
	int sn;
	int this_class;
	CHAR_DATA *mob;

	if (IS_NPC(ch))
		return;

	argument = one_argument( argument, arg );

	if (!arg[0]) {
		do_function(ch, &do_skills, "");
		send_to_char("\n\r", ch);
		return;
	}

	for (mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room) {
		if (IS_NPC(mob) && IS_SET(mob->act, ACT_PRACTICE))
			break;
	}

	if (!mob) {
		send_to_char("You can't do that here.\n\r", ch);
		return;
	}

	sn = find_spell(ch, arg);

	if (sn <= 0 || !can_practice( ch, sn )) {
		if(!p_percent_trigger(mob, NULL, NULL, NULL, ch, NULL, NULL, NULL, NULL, TRIG_PREPRACTICEOTHER,arg))
			send_to_char("You can't practice that.\n\r", ch);
		return;
	}

	// If it makes it this far, it is a standard spell

	if(p_percent_trigger(mob, NULL, NULL, NULL, ch, NULL, NULL, NULL, NULL, TRIG_PREPRACTICE, skill_table[sn].name))
		return;

	if (ch->practice <= 0) {
		send_to_char("You have no practice sessions left.\n\r", ch);
		return;
	}

	if (ch->pcdata->learned[sn] >= 75) {
		sprintf(buf, "There is nothing more that you can learn about %s here.\n\r", skill_table[sn].name);
		send_to_char(buf, ch);
	} else {
		this_class = get_this_class(ch, sn);
		--ch->practice;
		ch->pcdata->learned[sn] += int_app[get_curr_stat(ch, STAT_INT)].learn /
			(skill_table[sn].rating[this_class] == 0 ? 10 : skill_table[sn].rating[this_class]);

		if (ch->pcdata->learned[sn] < 75) {
			act("You practice $T.", ch, NULL, NULL, NULL, NULL, NULL, skill_table[sn].name, TO_CHAR);
			act("$n practices $T.", ch, NULL, NULL, NULL, NULL, NULL, skill_table[sn].name, TO_ROOM);
		} else {
			ch->pcdata->learned[sn] = 75;
			act("{WYou are now learned at $T.{x", ch, NULL, NULL, NULL, NULL, NULL, skill_table[sn].name, TO_CHAR);
			act("{W$n is now learned at $T.{x", ch, NULL, NULL, NULL, NULL, NULL, skill_table[sn].name, TO_ROOM);
		}
	}
}

void do_rehearse( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	char arg[MSL];
	int sn;
	CHAR_DATA *mob;
	bool wasbard;
	bool found = FALSE;

	if (IS_NPC(ch))
		return;

	if( ch->pcdata->sub_class_thief != CLASS_THIEF_BARD)
	{
		send_to_char( "You wouldn't even know how to rehearse.\n\r", ch );
		return;
	}

	wasbard = was_bard(ch);

	argument = one_argument( argument, arg );

	if (!arg[0]) {
		// List all the songs the can learn but don't know
		BUFFER *buffer = new_buf();

		add_buf(buffer, "You can learn the following songs: \n\r\n\r");
		add_buf(buffer, "{YSong Title                            Level {x\n\r");
		add_buf(buffer, "{Y--------------------------------------------x\n\r");

		for(sn = 0; (sn < MAX_SONGS) && music_table[sn].name; sn++)
		{
			if( !ch->pcdata->songs_learned[sn] &&
				(music_table[sn].level <= ch->level || wasbard))
			{
				sprintf(buf, "%-30s %10d\n\r",
					music_table[sn].name,
					music_table[sn].level);
				add_buf(buffer, buf);
				found = TRUE;
			}
		}

		if (found)
			page_to_char(buf_string(buffer), ch);
		else
			send_to_char( "There are no songs you can learn at this time.\n\r", ch );

		free_buf(buffer);
		return;
	}

	for (mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room) {
		if (IS_NPC(mob) && IS_SET(mob->act, ACT_PRACTICE))
			break;
	}

	if (!mob) {
		send_to_char("You can't do that here.\n\r", ch);
		return;
	}

	for(sn = 0; (sn < MAX_SONGS) && music_table[sn].name; sn++)
	{
		if( !ch->pcdata->songs_learned[sn] &&
			(music_table[sn].level <= ch->level || wasbard) &&
			!str_prefix(arg, music_table[sn].name))
		{
			found = TRUE;
			break;
		}
	}

	if( !found )
	{
		send_to_char("You can't rehearse that.\n\r", ch);
		return;
	}

	if(p_percent_trigger(mob, NULL, NULL, NULL, ch, NULL, NULL, NULL, NULL, TRIG_PREREHEARSE, music_table[sn].name))
		return;

	if (ch->practice < 3) {
		send_to_char("You need 3 practices sessions to rehearse a song.\n\r", ch);
		return;
	}

	ch->pcdata->songs_learned[sn] = TRUE;
	skill_entry_addsong(ch, sn, NULL);
	ch->practice -= 3;

	act("You rehearse {W$T{x.", ch, NULL, NULL, NULL, NULL, NULL, music_table[sn].name, TO_CHAR);
	act("{+$n rehearses {x$T{x.", ch, NULL, NULL, NULL, NULL, NULL, music_table[sn].name, TO_ROOM);
}


// Is sn a racial skill for a given race?
bool is_racial_skill(int race, int sn)
{
    int i;

    for (i = 0; pc_race_table[race].skills[i] != NULL; i++)
    {
	if (!str_cmp(skill_table[sn].name, pc_race_table[race].skills[i]))
	    return TRUE;
    }

    return FALSE;
}


// This monster determines if a person can practice a skill
bool can_practice( CHAR_DATA *ch, int sn )
{
    int this_class;

    if (sn < 0)
        return FALSE;

    this_class = get_this_class(ch, sn);

    // Is it a racial skill ?
    if (!IS_NPC(ch))
    {
	if (is_racial_skill(ch->race, sn)
	&& (ch->level >= skill_table[sn].skill_level[this_class] || had_skill(ch, sn)))
	    return TRUE;
    }

    // Does *everyone* get the skill? (such as hand to hand)
    if (is_global_skill(sn))
	return TRUE;

    // Have they had the skill in a previous subclass or class?
    if (had_skill(ch,sn))
	return TRUE;

    // Is the skill in the person's *current* class and the person
    // is of high enough level for it?
    if (has_class_skill(ch->pcdata->class_current, sn)
    &&  ch->level >= skill_table[sn].skill_level[this_class])
	return TRUE;

    // Ditto for subclass
    if (has_subclass_skill(ch->pcdata->sub_class_current, sn)
    &&  ch->level >= skill_table[sn].skill_level[this_class])
	return TRUE;

    // For old skills which already got practiced
    if (ch->pcdata->learned[sn] > 2)
	return TRUE;

    return FALSE;
}


// Has a person had a skill in their previous classes/subclasses?
bool had_skill( CHAR_DATA *ch, int sn )
{
    if (sn < 0)
	return FALSE;

    if (IS_IMMORTAL(ch))
	return TRUE;

    if (ch->pcdata->learned[sn] > 1)
	return TRUE;

    if (!IS_REMORT(ch))
    {
	switch (ch->pcdata->class_current)
	{
	    case CLASS_MAGE:
		if (has_subclass_skill( ch->pcdata->sub_class_cleric, sn )
		||  has_subclass_skill( ch->pcdata->sub_class_thief, sn )
		||  has_subclass_skill( ch->pcdata->sub_class_warrior, sn )
		||  has_class_skill( get_profession(ch, CLASS_CLERIC), sn )
		||  has_class_skill( get_profession(ch, CLASS_THIEF), sn )
		||  has_class_skill( get_profession(ch, CLASS_WARRIOR), sn ))
		    return TRUE;
		break;

	    case CLASS_CLERIC:
		if (has_subclass_skill( ch->pcdata->sub_class_mage, sn )
		||  has_subclass_skill( ch->pcdata->sub_class_thief, sn )
		||  has_subclass_skill( ch->pcdata->sub_class_warrior, sn )
		||  has_class_skill( get_profession(ch, CLASS_MAGE), sn )
		||  has_class_skill( get_profession(ch, CLASS_THIEF), sn )
		||  has_class_skill( get_profession(ch, CLASS_WARRIOR), sn ))
		    return TRUE;
		break;

	    case CLASS_THIEF:
		if (has_subclass_skill( ch->pcdata->sub_class_mage, sn )
		||  has_subclass_skill( ch->pcdata->sub_class_cleric, sn )
		||  has_subclass_skill( ch->pcdata->sub_class_warrior, sn )
		||  has_class_skill( get_profession(ch, CLASS_MAGE), sn )
		||  has_class_skill( get_profession(ch, CLASS_CLERIC), sn )
		||  has_class_skill( get_profession(ch, CLASS_WARRIOR), sn ))
		    return TRUE;
		break;

	    case CLASS_WARRIOR:
		if (has_subclass_skill( ch->pcdata->sub_class_mage, sn )
		||  has_subclass_skill( ch->pcdata->sub_class_cleric, sn )
		||  has_subclass_skill( ch->pcdata->sub_class_thief, sn )
		||  has_class_skill( get_profession(ch, CLASS_MAGE), sn )
		||  has_class_skill( get_profession(ch, CLASS_CLERIC), sn )
		||  has_class_skill( get_profession(ch, CLASS_THIEF), sn ))
		    return TRUE;
		break;
	}
    }
    else // for remorts
    {
	if (has_class_skill( CLASS_MAGE, sn )
	||  has_class_skill( CLASS_CLERIC, sn )
	||  has_class_skill( CLASS_THIEF, sn )
	||  has_class_skill( CLASS_WARRIOR, sn )
	||  has_subclass_skill( ch->pcdata->sub_class_mage, sn )
	||  has_subclass_skill( ch->pcdata->sub_class_cleric, sn )
	||  has_subclass_skill( ch->pcdata->sub_class_thief, sn )
	||  has_subclass_skill( ch->pcdata->sub_class_warrior, sn ))
	    return TRUE;

	switch( ch->pcdata->sub_class_current )
	{
	    case CLASS_MAGE_ARCHMAGE:
	    case CLASS_MAGE_ILLUSIONIST:
	    case CLASS_MAGE_GEOMANCER:
		if (has_subclass_skill( ch->pcdata->second_sub_class_cleric, sn )
		||  has_subclass_skill( ch->pcdata->second_sub_class_thief, sn )
		||  has_subclass_skill( ch->pcdata->second_sub_class_warrior, sn ))
		    return TRUE;
		break;

	    case CLASS_CLERIC_RANGER:
	    case CLASS_CLERIC_ADEPT:
	    case CLASS_CLERIC_ALCHEMIST:
		if (has_subclass_skill( ch->pcdata->second_sub_class_mage, sn )
		||  has_subclass_skill( ch->pcdata->second_sub_class_thief, sn )
		||  has_subclass_skill( ch->pcdata->second_sub_class_warrior, sn ))
		    return TRUE;
		break;

	    case CLASS_THIEF_HIGHWAYMAN:
	    case CLASS_THIEF_NINJA:
	    case CLASS_THIEF_SAGE:
		if (has_subclass_skill( ch->pcdata->second_sub_class_mage, sn )
		||  has_subclass_skill( ch->pcdata->second_sub_class_cleric, sn )
		||  has_subclass_skill( ch->pcdata->second_sub_class_warrior, sn ))
		    return TRUE;
		break;

	   case CLASS_WARRIOR_WARLORD:
	   case CLASS_WARRIOR_DESTROYER:
	   case CLASS_WARRIOR_CRUSADER:
		if (has_subclass_skill( ch->pcdata->second_sub_class_mage, sn )
		||  has_subclass_skill( ch->pcdata->second_sub_class_cleric, sn )
		||  has_subclass_skill( ch->pcdata->second_sub_class_thief, sn ))
		    return TRUE;
		break;
	}
    }

    return FALSE;
}


// Does *everyone* get the sn?
bool is_global_skill( int sn )
{
    int i;

    if ( sn < 0 )
	return FALSE;

    // "global skills" is always 1st in group list
    for (i = 0; group_table[0].spells[i] != NULL; i++)
    {
	if (!str_cmp(group_table[0].spells[i], skill_table[sn].name))
	    return TRUE;
    }

    return FALSE;
}


// Equalizes skills on a character. Removes ones they arn't supposed to have.
void update_skills( CHAR_DATA *ch )
{
    char buf[MSL];
    int sn;
    int reward = 0;

    for (sn = 0; sn < MAX_SKILL && skill_table[sn].name; sn++)
    {
	if (ch->pcdata->learned[sn] > 0 && !should_have_skill(ch, sn)
	&&  str_cmp(skill_table[sn].name, "reserved"))
	{
	    sprintf(buf, "You shouldn't have skill %s (reward of {Y%d{x quest points)\n\r",
	        skill_table[sn].name, 7 * ch->pcdata->learned[sn]);
	    send_to_char(buf, ch);
	    reward += (7 * ch->pcdata->learned[sn]);
	    ch->pcdata->learned[sn] = -ch->pcdata->learned[sn];
	}
    }

    if (reward > 0)
    {
	sprintf(buf, "Total reward is {Y%d{x quest points!\n\r", reward);
	send_to_char(buf, ch);
    }

    ch->questpoints += reward;
}


// Return true if the skill belongs to the subclass.
bool has_subclass_skill( int subclass, int sn )
{
    char *skill_name;
    char *group_name;
    int i;
    int n;

    if (sn < 0)
	return FALSE;

    if (subclass < CLASS_WARRIOR_MARAUDER || subclass > CLASS_THIEF_SAGE)
	return FALSE;

    skill_name = skill_table[sn].name;
    group_name = sub_class_table[subclass].default_group;

    for (i = 0; group_table[i].name != NULL; i++)
    {
	if (!str_cmp(group_name, group_table[i].name))
	    break;
    }

    for (n = 0; group_table[i].spells[n] != NULL; n++)
    {
	if (!str_cmp(skill_name, group_table[i].spells[n]))
	    return TRUE;
    }

    return FALSE;
}


// Returns true if the class has the skill.
bool has_class_skill( int class, int sn )
{
    char *group_name;
    int i;
    int n;

    if (sn < 0)
	return FALSE;

    if (class < CLASS_MAGE || class > CLASS_WARRIOR)
	return FALSE;

    switch (class)
    {
	case CLASS_MAGE: 	group_name = "mage skills"; break;
	case CLASS_CLERIC: 	group_name = "cleric skills"; break;
	case CLASS_THIEF: 	group_name = "thief skills"; break;
	case CLASS_WARRIOR: 	group_name = "warrior skills"; break;
	default: 		group_name = "global skills"; break;
    }

    for (i = 0; group_table[i].name != NULL; i++)
    {
	if (!str_cmp(group_name, group_table[i].name))
	    break;
    }

    for (n = 0; group_table[i].spells[n]; n++)
    {
	if (!str_cmp( skill_table[sn].name, group_table[i].spells[n]))
	    return TRUE;
    }

    return FALSE;
}


// Should a player have a skill?
// Used for old players having skills they shouldn't.
bool should_have_skill( CHAR_DATA *ch, int sn )
{
    char buf[MSL];

    buf[0] = '\0';

    if (ch == NULL)
    {
    	bug("should_have_skill: null ch", 0 );
	return FALSE;
    }

    if (sn < 0 || sn > MAX_SKILL)
    {
    	bug("should_have_skill: bad sn", 0 );
	return FALSE;
    }

    if (is_racial_skill(ch->race, sn))
    	return TRUE;

    if (is_global_skill(sn))
	return TRUE;

    if (has_class_skill( get_profession(ch, CLASS_MAGE), sn )
    ||  has_class_skill( get_profession(ch, CLASS_CLERIC), sn )
    ||  has_class_skill( get_profession(ch, CLASS_THIEF), sn )
    ||  has_class_skill( get_profession(ch, CLASS_WARRIOR), sn ))
	return TRUE;

    if (has_subclass_skill( ch->pcdata->sub_class_mage, sn )
    ||  has_subclass_skill( ch->pcdata->sub_class_cleric, sn )
    ||  has_subclass_skill( ch->pcdata->sub_class_thief, sn )
    ||  has_subclass_skill( ch->pcdata->sub_class_warrior, sn ))
	return TRUE;

    if (has_subclass_skill( ch->pcdata->second_sub_class_mage, sn )
    ||  has_subclass_skill( ch->pcdata->second_sub_class_cleric, sn )
    ||  has_subclass_skill( ch->pcdata->second_sub_class_thief, sn )
    ||  has_subclass_skill( ch->pcdata->second_sub_class_warrior, sn ))
	return TRUE;

    return FALSE;
}

char *skill_entry_name (SKILL_ENTRY *entry)
{
	if( entry ) {
		if ( IS_VALID(entry->token) ) return entry->token->name;

		if ( entry->sn > 0 ) return skill_table[entry->sn].name;
		if ( entry->song >= 0 ) return music_table[entry->song].name;
	}

	return &str_empty[0];
}

int skill_entry_compare (SKILL_ENTRY *a, SKILL_ENTRY *b)
{
	char *an = skill_entry_name(a);
	char *bn = skill_entry_name(b);
	int cmp = str_cmp(an, bn);

	if( !cmp ) {
		if( (a->sn > 0 || a->song >= 0) && IS_VALID(b->token)) cmp = -1;
		else if( IS_VALID(a->token) && (b->sn > 0 || b->song >= 0)) cmp = 1;
	}

	//log_stringf("skill_entry_compare: a(%s) %s b(%s)", an, ((cmp < 0) ? "<" : ((cmp > 0) ? ">" : "==")), bn);

	return cmp;
}

void skill_entry_insert (SKILL_ENTRY **list, int sn, int song, TOKEN_DATA *token)
{
	SKILL_ENTRY *cur, *prev, *entry;

	entry = new_skill_entry();
	if( !entry ) return;

	entry->sn = sn;
	entry->token = token;
	entry->song = song;

	cur = *list;
	prev = NULL;

	while( cur ) {
		if( skill_entry_compare(entry, cur) < 0 )
			break;

		prev = cur;
		cur = cur->next;
	}

	if( prev )
		prev->next = entry;
	else
		*list = entry;
	entry->next = cur;
}

void skill_entry_remove (SKILL_ENTRY **list, int sn, int song, TOKEN_DATA *token)
{
	SKILL_ENTRY *cur, *prev;

	cur = *list;
	prev = NULL;
	while(cur) {
		if ( (IS_VALID(token) && (cur->token == token)) ||
			(sn > 0 && (cur->sn == sn)) ||
			((song >= 0) && (cur->song == song))) {

			if(prev)
				prev->next = cur->next;
			else
				*list = cur->next;

			free_skill_entry(cur);
		}

		prev = cur;
		cur = cur->next;
	}
}

SKILL_ENTRY *skill_entry_findname( SKILL_ENTRY *list, char *str )
{
	int count;
	char name[MIL];

	count = number_argument(str, name);

	if(count < 1) count = 1;

	while (list) {
		// Name match, until count predecrements to 0
		if( is_name(str, skill_entry_name(list)) && !--count )
			return list;

		list = list->next;
	}

	return NULL;
}

SKILL_ENTRY *skill_entry_findsn( SKILL_ENTRY *list, int sn )
{
	if( sn < 1 || sn >= MAX_SKILL ) return NULL;

	while (list && list->sn != sn)
		list = list->next;

	return list;
}

SKILL_ENTRY *skill_entry_findsong( SKILL_ENTRY *list, int song )
{
	if( song < 0 ) return NULL;

	while (list && list->song != song)
		list = list->next;

	return list;
}

SKILL_ENTRY *skill_entry_findtoken( SKILL_ENTRY *list, TOKEN_DATA *token )
{
	if( !IS_VALID(token) ) return NULL;

	while (list && list->token != token)
		list = list->next;

	return list;
}

void skill_entry_addskill (CHAR_DATA *ch, int sn, TOKEN_DATA *token)
{
	if( !ch ) return;

/*
	if(token)
		log_stringf("skill_entry_addskill: ch(%s) sn(%d) token(%ld, %s, %s)", (ch->name ? ch->name : "(unknown)"), sn, token->pIndexData->vnum, token->name, (token->type == TOKEN_SKILL) ? "SKILL" : "!SKILL");
	else
		log_stringf("skill_entry_addskill: ch(%s) sn(%d) token(0)", (ch->name ? ch->name : "(unknown)"), sn);
*/
	if( !sn && (!token || token->type != TOKEN_SKILL)) return;

	skill_entry_insert( &ch->sorted_skills, sn, -1, token );
}

void skill_entry_addspell (CHAR_DATA *ch, int sn, TOKEN_DATA *token)
{
	if( !ch ) return;

	if( !sn && (!token || token->type != TOKEN_SPELL)) return;

	skill_entry_insert( &ch->sorted_spells, sn, -1, token );
}

void skill_entry_addsong (CHAR_DATA *ch, int song, TOKEN_DATA *token)
{
	if( !ch ) return;

	if( song < 0 && (!token || token->type != TOKEN_SONG)) return;

	skill_entry_insert( &ch->sorted_songs, 0, song, token );
}

void skill_entry_removeskill (CHAR_DATA *ch, int sn, TOKEN_DATA *token)
{
	if( !ch ) return;

	if( !sn && (!token || token->type != TOKEN_SKILL)) return;

	skill_entry_remove( &ch->sorted_skills, sn, -1, token );
}

void skill_entry_removespell (CHAR_DATA *ch, int sn, TOKEN_DATA *token)
{
	if( !ch ) return;

	if( !sn && (!token || token->type != TOKEN_SPELL)) return;

	skill_entry_remove( &ch->sorted_spells, sn, -1, token );
}

void skill_entry_removesong (CHAR_DATA *ch, int song, TOKEN_DATA *token)
{
	if( !ch ) return;

	if( song < 0 && (!token || token->type != TOKEN_SONG)) return;

	skill_entry_remove( &ch->sorted_songs, 0, song, token );
}

int token_skill_rating( CHAR_DATA *ch, TOKEN_DATA *token)
{
	int percent;

	// Make sure the tokens are skill/spell tokens
	if( ((token->type == TOKEN_SKILL) || (token->type == TOKEN_SPELL)) &&
		(token->type != token->pIndexData->type))
		return 0;

	// Value 0
	if(token->pIndexData->value[TOKVAL_SPELL_RATING] > 0)
		percent = 100 * token->value[TOKVAL_SPELL_RATING] / token->pIndexData->value[TOKVAL_SPELL_RATING];
	else
		percent = token->value[TOKVAL_SPELL_RATING];

	return percent;
}

int skill_entry_rating (CHAR_DATA *ch, SKILL_ENTRY *entry)
{
	if( IS_VALID(entry->token) ) {
		return token_skill_rating(ch, entry->token);
	} else if( entry->sn > 0) {
		if( IS_NPC(ch) ) {
			if ((skill_table[entry->sn].race != -1 && ch->race != skill_table[entry->sn].race) || ch->tot_level < 10)
				return 0;

			return mob_skill_table[ch->tot_level];
		} else
			return ch->pcdata->learned[entry->sn];
	} else
		return 0;
}

int skill_entry_mod(CHAR_DATA *ch, SKILL_ENTRY *entry)
{
	if( IS_VALID(entry->token) ) {
		return 0;	// No mods for TOKEN entries yet!
	} else if( entry->sn > 0) {
		if( IS_NPC(ch) ) return 0;

		return ch->pcdata->mod_learned[entry->sn];
	} else
		return 0;
}

int skill_entry_level (CHAR_DATA *ch, SKILL_ENTRY *entry)
{
	int this_class;

	if( IS_IMMORTAL(ch) )
		return LEVEL_IMMORTAL;

	if( IS_VALID(entry->token) ) {
		// Make sure the tokens are skill/spell tokens
		if( (entry->token->type == TOKEN_SKILL || entry->token->type == TOKEN_SPELL || entry->token->type == TOKEN_SONG) &&
			entry->token->type != entry->token->pIndexData->type)
			return 0;

		// All token abilities register as level 1 (for now)
		return 1;
	} else if( entry->sn > 0) {
		this_class = get_this_class(ch, entry->sn);

		// Not ready yet
		if( !had_skill( ch, entry->sn ) && (ch->level < skill_table[entry->sn].skill_level[this_class]) )
			return -skill_table[entry->sn].skill_level[this_class];

		return skill_table[entry->sn].skill_level[this_class];
	} else if( entry->song >= 0 ) {
		return music_table[entry->song].level;
	} else
		return 0;
}

