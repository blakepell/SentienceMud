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
// Note from Whisp
// In the spells, theres a segregated level modifier. If a level is above 9000, its an object and should be permanent
// so it subtracts this from the level so 9100 is lvl 100 with a permanent duration.
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "wilds.h"
#include "scripts.h"

// Lookup a skill by name.
int skill_lookup(const char *name)
{
    int sn;

    for (sn = 0; sn < MAX_SKILL; sn++)
    {
	if (skill_table[sn].name == NULL)
	    break;
	if (LOWER(name[0]) == LOWER(skill_table[sn].name[0])
	&&   !str_prefix(name, skill_table[sn].name))
	    return sn;
    }

    return -1;
}


// finds a spell the character can cast if possible
int find_spell(CHAR_DATA *ch, const char *name)
{
    int sn, found = -1;
    int this_class;
    int level;

    if (IS_NPC(ch))
	return skill_lookup(name);

    for (sn = 0; sn < MAX_SKILL; sn++)
    {
	if (skill_table[sn].name == NULL)
	    break;
	if (LOWER(name[0]) == LOWER(skill_table[sn].name[0])
	&&  !str_prefix(name,skill_table[sn].name))
	{
	    if (found == -1)
		found = sn;
        this_class = 9999;
        if (ch->pcdata->class_mage != -1
	    && (level = skill_table[sn].skill_level[ch->pcdata->class_mage]) < 31)
        {
                this_class = ch->pcdata->class_mage;
        }
        else
        if (ch->pcdata->class_cleric != -1
	    && (level = skill_table[sn].skill_level[ch->pcdata->class_cleric]) < 31)
        {
                this_class = ch->pcdata->class_cleric;
        }
        else
        if (ch->pcdata->class_thief != -1
	    && (level = skill_table[sn].skill_level[ch->pcdata->class_thief]) < 31)
        {
                this_class = ch->pcdata->class_thief;
        }
        else
        if (ch->pcdata->class_warrior != -1
	    && (level = skill_table[sn].skill_level[ch->pcdata->class_warrior]) < 31)
        {
                this_class = ch->pcdata->class_warrior;
	}

	    if (ch->level >= skill_table[sn].skill_level[this_class]
	    &&  ch->pcdata->learned[sn] > 0)
		    return sn;
	}
    }

    return found;
}


// Utter mystical words for an sn.
void say_spell(CHAR_DATA *ch, int sn)
{
    char buf  [MAX_STRING_LENGTH];
    char buf2 [MAX_STRING_LENGTH];
    CHAR_DATA *rch;
    char *pName;
    int iSyl;
    int length;

    struct syl_type
    {
	char *old;
	char *new;
    };

    static const struct syl_type syl_table[] =
    {
	{ " ",		" "		},
	{ "ar",		"bar"		},
	{ "au",		"nai"		},
	{ "bless",	"kam"		},
	{ "blind",	"miz"		},
	{ "bur",	"ras"		},
	{ "cu",		"lai"		},
	{ "de",		"moi"		},
	{ "en",		"ief"		},
	{ "light",	"lei"		},
	{ "lo",		"ao"		},
	{ "mor",	"zel"		},
	{ "move",	"pea"		},
	{ "ness",	"eox"		},
	{ "ning",	"deu"		},
	{ "per",	"in"		},
	{ "ra",		"lov"		},
	{ "fresh",	"ima"		},
	{ "re",		"nia"		},
	{ "son",	"sei"		},
	{ "tect",	"eui"		},
	{ "tri",	"mav"		},
	{ "ven",	"noa"		},
	{ "a", "a" }, { "b", "b" }, { "c", "q" }, { "d", "e" },
	{ "e", "z" }, { "f", "y" }, { "g", "o" }, { "h", "p" },
	{ "i", "u" }, { "j", "y" }, { "k", "t" }, { "l", "r" },
	{ "m", "w" }, { "n", "i" }, { "o", "a" }, { "p", "s" },
	{ "q", "d" }, { "r", "f" }, { "s", "g" }, { "t", "h" },
	{ "u", "j" }, { "v", "z" }, { "w", "x" }, { "x", "n" },
	{ "y", "l" }, { "z", "k" },
	{ "", "" }
    };

    buf[0]	= '\0';
    for (pName = skill_table[sn].name; *pName != '\0'; pName += length)
    {
	for (iSyl = 0; (length = strlen(syl_table[iSyl].old)) != 0; iSyl++)
	{
	    if (!str_prefix(syl_table[iSyl].old, pName))
	    {
		strcat(buf, syl_table[iSyl].new);
		break;
	    }
	}

	if (length == 0)
	    length = 1;
    }

    sprintf(buf2, "$n utters the words, '%s'.", buf);
    sprintf(buf,  "$n utters the words, '%s'.", skill_table[sn].name);

    for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
    {
	if (rch != ch)
	    act((!IS_NPC(rch) && ch->pcdata->class_current == rch->pcdata->class_current) ? buf : buf2,
	        ch, NULL, rch, TO_VICT);
    }
}


/* FALSE - dispel spell. ch = caster, victim = person being dispelled. */
bool saves_dispel(CHAR_DATA *ch, CHAR_DATA *victim, int spell_level)
{
	int save;
	int dif;

	if (spell_level > MAGIC_SCRIPT_SPELL)
		spell_level -= MAGIC_SCRIPT_SPELL;

	else if (spell_level > MAGIC_WEAR_SPELL)
		spell_level -= MAGIC_WEAR_SPELL;

	if (IS_IMMORTAL(ch) && !IS_NPC(ch))
		return FALSE;

	// Base save, 100% chance it gos through
	save = 100;

	dif = (victim&&ch) ? abs(victim->tot_level - ch->tot_level) : 0;

	if (dif < 10)		save = number_range(85, 100);
	else if (dif < 20)	save = number_range(80, 84);
	else if (dif < 30)	save = number_range(70, 79);
	else if (dif < 40)	save = number_range(65, 69);
	else if (dif < 50)	save = number_range(50, 59);
	else if (dif < 100)	save = number_range(40, 49);
	else if (dif < 150)	save = number_range(30, 39);
	else if (dif < 200)	save = number_range(20, 29);
	else			save = number_range(5, 19);

	if (ch && victim) {
		if (victim->tot_level < ch->tot_level)
			save = 100 - save;
	}

	if (victim)
	switch(check_immune(victim, DAM_MAGIC)) {
	case IS_IMMUNE: return FALSE;
	case IS_RESISTANT: save /= 2; break;
	case IS_VULNERABLE: save *= 2; break;
	default: break;
	}

	/* remorts get a higher base save */
	if (victim && IS_REMORT(victim)) save -= 10;

	if (ch && IS_REMORT(ch)) save += 10;

	/* if you are asleep it's harder */
	if (victim && victim->position < POS_RESTING) save *= 2;

	if (ch == victim) save = number_range(75,90);

	save = URANGE(4, save, 96);

	return (number_percent() > save);
}


/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool saves_spell(int level, CHAR_DATA *victim, sh_int dam_type)
{
    int chance;
    //char buf[MSL];

    if (abs(victim->tot_level - level) < 15)
        chance = 15;
    else
        chance = (victim->tot_level - level)/4;

    /* remorts get a higher base save */
    if (IS_REMORT(victim))
        chance += 5;

    switch(check_immune(victim,dam_type))
    {
	case IS_IMMUNE:		return TRUE;
	case IS_RESISTANT:	chance += 10; break;
	case IS_VULNERABLE:	chance -= 10; break;
    }

    /* if you are asleep you get less */
    if (victim->position < POS_RESTING)
    {
	chance /= 2;
    }

    //sprintf(buf, "%d", chance);send_to_char(buf,victim);
    chance = URANGE(4, chance, 96);
    //sprintf(buf, "%d", chance);send_to_char(buf,victim);

    if (number_percent() < chance)
        return TRUE;
    else
	return FALSE;
}


/* co-routine for dispel magic and cancellation */
bool check_dispel(CHAR_DATA *ch, CHAR_DATA *victim, int sn)
{
	AFFECT_DATA *af;

	if (is_affected(victim, sn)) {
		for (af = victim->affected; af != NULL; af = af->next) {
			if (af->type == sn) {
				if (!saves_dispel(ch, victim, af->level)) {
					affect_strip(victim,sn);
					if (skill_table[sn].msg_off) {
						send_to_char(skill_table[sn].msg_off, victim);
						send_to_char("\n\r", victim);
					}
					if (skill_table[sn].msg_disp && skill_table[sn].msg_disp[0])
						act(skill_table[sn].msg_disp,victim,NULL,NULL,TO_ROOM);

					return TRUE;
				} else
					af->level--;
			}
		}
	}

	return FALSE;
}

bool validate_spell_target(CHAR_DATA *ch,int type,char *arg,int *t,CHAR_DATA **v, OBJ_DATA **o)
{
	CHAR_DATA *victim = NULL;
	OBJ_DATA *obj = NULL;
	int target = TARGET_NONE;

	// Preset
	*v = victim;
	*o = obj;
	*t = target;

	switch(type) {
	case TAR_IGNORE: target = TARGET_NONE; break;

	case TAR_CHAR_OFFENSIVE:
		if (!arg[0]) victim = ch->fighting;
		else victim = get_char_room(ch, NULL, arg);

		if (!victim) {
			if (!arg[0])
				send_to_char("Cast it on whom?\n\r", ch);
			else
				send_to_char("They aren't here.\n\r", ch);
			return FALSE;
		}

		if (is_safe(ch, victim, TRUE) ||
			(victim->fighting && !is_same_group(ch, victim->fighting) &&
			ch != victim && !IS_SET(ch->in_room->room2_flags, ROOM_MULTIPLAY))) {
			send_to_char("Not on that target.\n\r", ch);
			return FALSE;
		}

		if (ch->fighting && !is_same_group(victim, ch->fighting) && ch != victim && !IS_NPC(victim)) {
			act("You must finish your fight before attacking $N.", ch, NULL, victim, TO_CHAR);
			return FALSE;
		}

		target = TARGET_CHAR;
		break;

	case TAR_CHAR_DEFENSIVE:
		if (arg[0]) {
			victim = get_char_room(ch, NULL, arg);
			if (!victim) {
				send_to_char("They aren't here.\n\r", ch);
				return FALSE;
			}

			if (victim != ch && victim->fighting && victim->fighting != ch &&
				!is_same_group(ch, victim->fighting) && !IS_NPC(victim) &&
				!IS_NPC(victim->fighting) && !is_pk(ch) && !IS_SET(ch->in_room->room_flags, ROOM_ARENA)) {
				send_to_char("You can't interfere in a PK battle if you are not PK.\n\r", ch);
				return FALSE;
			}
		} else
		victim = ch;

		target = TARGET_CHAR;
		break;

	case TAR_CHAR_SELF:
		if (arg[0] && str_cmp(arg, "me") && str_cmp(arg, "self") && str_prefix(arg, ch->name)) {
			send_to_char("You may not cast this spell on another.\n\r", ch);
			return FALSE;
		}

		victim = ch;
		target = TARGET_CHAR;
		break;

	case TAR_OBJ_INV:
		if (!arg[0]) {
			send_to_char("Cast it on what?\n\r", ch);
			return FALSE;
		}

		if (!(obj = get_obj_list(ch, arg, ch->carrying))) {
			send_to_char("You're not carrying that item.\n\r", ch);
			return FALSE;
		}

		target = TARGET_OBJ;
		break;

	case TAR_OBJ_GROUND:
		if (!arg[0]) {
			send_to_char("Cast it on what?\n\r", ch);
			return FALSE;
		}

		obj = get_obj_list(ch, arg, ch->in_room->contents);
		if (!obj) {
			send_to_char("It's not anywhere around here.\n\r", ch);
			return FALSE;
		}

		target = TARGET_OBJ;
		break;

	case TAR_OBJ_CHAR_OFF:
		if (!arg[0] && !ch->fighting) {
			send_to_char("Cast it on whom or what?\n\r", ch);
			return FALSE;
		}

		if (ch->fighting && !arg[0])
			victim = ch->fighting;
		else
			victim = get_char_room(ch, NULL, arg);

		obj = get_obj_list(ch, arg, ch->carrying);
		if (victim) target = TARGET_CHAR;
		else if (obj) target = TARGET_OBJ;
		else {
			send_to_char("You don't see that here.\n\r", ch);
			return FALSE;
		}

		if (target == TARGET_CHAR && (is_safe(ch, victim, TRUE) ||
			(victim->fighting && ch != victim && !is_same_group(ch, victim->fighting) &&
			!IS_SET(ch->in_room->room2_flags, ROOM_MULTIPLAY)))) {
			send_to_char("Not on that target.\n\r", ch);
			return FALSE;
		}
		break;

	case TAR_OBJ_CHAR_DEF:
		if (!arg[0])
			victim = ch;
		else {
			victim = get_char_room(ch, NULL, arg);
			obj = get_obj_list(ch, arg, ch->carrying);
		}

		if (victim) {
			if (victim != ch && victim->fighting && victim->fighting != ch &&
				!is_same_group(ch, victim->fighting) && !IS_NPC(victim) &&
				!IS_NPC(victim->fighting) && !is_pk(ch) && !IS_SET(ch->in_room->room_flags, ROOM_ARENA)) {
				send_to_char("You can't interfere in a PK battle if you are not PK.\n\r", ch);
				return FALSE;
			}

			target = TARGET_CHAR;
		} else if (obj)
			target = TARGET_OBJ;
		else {
			send_to_char("They aren't here.\n\r", ch);
			return FALSE;
		}
		break;

	case TAR_IGNORE_CHAR_DEF:
		if (!arg[0]) {
			send_to_char("Cast it on whom?\n\r", ch);
			return FALSE;
		}

		victim = get_char_world(ch, arg);
		if (!victim) {
			send_to_char("They aren't anywhere in Sentience.\n\r", ch);
			return FALSE;
		}

		target = TARGET_CHAR;
		break;

	default:
		target = TARGET_NONE;
		break;
	}

	*v = victim;
	*o = obj;
	*t = target;

	return TRUE;
}

bool check_mana_cost(CHAR_DATA *ch, int cost)
{
	return true;
}

void do_cast(CHAR_DATA *ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	TOKEN_DATA *token;
	SCRIPT_DATA *script = NULL;
	PROG_LIST *prg;
	int beats;
	int mana;
	int sn;
	int target;
	int i;
	char buf[MSL];
	char temp[MSL];

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (IS_AFFECTED2(ch, AFF2_SILENCE)) {
		send_to_char("You open your mouth but nothing comes out.\n\r", ch);
		act("$n opens $s mouth but nothing comes out.", ch, NULL, NULL, TO_ROOM);
		return;
	}

	if (IS_DEAD(ch)) {
		send_to_char("You begin casting the spell, but your magic fizzles and dies.\n\r", ch);
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_NOMAGIC)) {
		send_to_char("Your magical energies are powerless here.\n\r", ch);
		return;
	}

	if (!arg1[0]) {
		send_to_char("Cast which what where?\n\r", ch);
		return;
	}

	/* 09-25-2006 Syn
	I put in a fix which makes sure to check all matches, rather than
	just the first one. For example, if someone who doesn't know deathbarbs
	but does know death grip tries "c death", it assumes deathbarbs and tells
	the person they don't know that spell.
	*/
	for (sn = 0; sn < MAX_SKILL; sn++) {
		if (!str_prefix(arg1, skill_table[sn].name) &&
			get_skill(ch, sn) > 0 && skill_table[sn].spell_fun != spell_null)
			break;
	}

	mana = 0;
	if(ch->manastore < 0) ch->manastore = 0;
	if (sn >= MAX_SKILL) {
		// Check for TOKEN spells
		script = NULL;
		for(token = ch->tokens; token; token = token->next) {
//			sprintf(buf,"TOKEN: '%s' -> '%s' -> %s, %s\n\r", token->pIndexData->name, arg1, !str_prefix(arg1, token->pIndexData->name) ? "TRUE" : "FALSE", token->pIndexData->progs ? "TRUE" : "FALSE");
//			send_to_char(buf,ch);
			if(token->pIndexData->type == TOKEN_SPELL && token->pIndexData && !str_prefix(arg1, token->pIndexData->name) && token->pIndexData->progs) {
				for(prg = token->pIndexData->progs[TRIGSLOT_SPELL]; prg; prg = prg->next) {
//					sprintf(buf,"TOKEN->SCRIPT: '%s' -> %s\n\r", token->pIndexData->name, trigger_table[prg->trig_type].name);
//					send_to_char(buf,ch);

					if(is_trigger_type(prg->trig_type,TRIG_SPELL)) {
						mana = atoi(prg->trig_phrase);
						script = prg->script;
						break;
					}
				}
				if(script) break;
			}
		}

		if(!script) {
			send_to_char("You don't know any spells of that name.\n\r", ch);
			return;
		}

		// Check minimum position
		if (ch->position < token->pIndexData->value[TOKVAL_SPELL_POSITION]) {
			send_to_char("You can't concentrate enough.\n\r", ch);
			return;
		}

		if ((ch->mana + ch->manastore) < mana) {
			send_to_char("You don't have enough mana.\n\r", ch);
			return;
		}

		// Setup targets.
		ch->cast_sn = -1;
		ch->cast_token = token;
		ch->cast_script = script;
		ch->cast_mana = mana;

		if(!validate_spell_target(ch,token->pIndexData->value[TOKVAL_SPELL_TARGET],arg2,&target,&victim,&obj)) {
			return;
		}

		ch->tempstore[0] = 0;

		// Precheck for the spell token - set the cast beats in here!
		if(p_percent_trigger(NULL,NULL,NULL,token,ch,NULL,NULL, TRIG_PRESPELL)) {
			return;
		}

		beats = ch->tempstore[0];
	} else {

		if (ch->position < skill_table[sn].minimum_position) {
			send_to_char("You can't concentrate enough.\n\r", ch);
			return;
		}

		mana = skill_table[sn].min_mana;

		if ((ch->mana + ch->manastore) < mana) {
			send_to_char("You don't have enough mana.\n\r", ch);
			return;
		}

		// Setup targets.
		ch->cast_token = NULL;
		ch->cast_script = NULL;
		ch->cast_sn = sn;
		ch->cast_mana = mana;

		if(!validate_spell_target(ch,skill_table[sn].target,arg2,&target,&victim,&obj))
			return;

		if(p_percent_trigger_phrase(NULL,NULL,ch->in_room,NULL,ch,NULL,NULL, TRIG_PRECAST,"check"))
			return;

		beats = skill_table[sn].beats;
	}

	send_to_char("{WYou begin to speak the words of the spell...\n\r{x", ch);
	act("{W$n begins casting a spell...{x", ch, NULL, NULL, TO_ROOM);

	// this bit makes sure that if there are 2 mobs in the room with the same
	//   name, cast_end is performed on the correct target.
	if (target == TARGET_CHAR) {
		if ((i = number_argument(arg2, temp)) != 1) {
			sprintf(buf, "%i.%s", i, temp);
			ch->cast_target_name = str_dup(buf);
		} else
			ch->cast_target_name = str_dup(victim->name);
	} else if (target == TARGET_OBJ) {
		if ((i = number_argument(arg2, temp)) != 1) {
			sprintf(buf, "%i.%s", i, temp);
			ch->cast_target_name = str_dup(buf);
		} else
			ch->cast_target_name = str_dup(obj->name);
	} else
		ch->cast_target_name = str_dup(arg2);

	// @@@NIB : 20070126 : Slow magic, 2-3 times as long
	//	room2:slow_magic adds 0.5-1x
	//	sector:cursed_sanctum adds 0.5-1x
	if(!IS_NPC(ch)) {
		ch->tempstore[0] = 1000;
		p_percent_trigger_phrase(NULL,NULL,ch->in_room,NULL,ch,NULL,NULL, TRIG_PRECAST,"beats");
		i = ch->tempstore[0];
		if(IS_SET(ch->in_room->room2_flags,ROOM_SLOW_MAGIC)) i += number_range(500,1000);
		if(ch->in_room->sector_type == SECT_CURSED_SANCTUM) i += number_range(500,1000);
	} else
		i = 1000;

	beats = beats * i / 1000;

	CAST_STATE(ch, beats);
}

void deduct_mana(CHAR_DATA *ch,int cost)
{
	if(ch->manastore >= cost)
		ch->manastore -= cost;
	else {
		ch->mana -= (cost - ch->manastore);
		ch->manastore = 0;
	}
}

void cast_end(CHAR_DATA *ch)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	OBJ_DATA *trap;
	TOKEN_DATA *token = NULL;
	SCRIPT_DATA *script = NULL;
	int mana;
	void *vo;
	unsigned long id[2];
	int type;
	int sn;
	int target;
	int chance;	// @@@NIB : 20070126 : hard_magic

	send_to_char("{WYou have completed your casting.{x\n\r", ch);
	act("{W$n has completed $s casting.{x", ch , NULL, NULL, TO_ROOM);

	if(ch->cast_token) {
		token = ch->cast_token;
		script = ch->cast_script;
		ch->cast_token = NULL;
		ch->cast_script = NULL;
		type = token->pIndexData->value[TOKVAL_SPELL_TARGET];
		sn = -1;
	} else {
		sn = ch->cast_sn;
		ch->cast_sn = -1;
		type = skill_table[sn].target;
	}

	mana = ch->cast_mana;
	victim = NULL;
	obj = NULL;
	vo = NULL;
	target	= TARGET_NONE;

	switch (type) {
	case TAR_IGNORE:
		vo = (void *) ch->cast_target_name;
		target = TARGET_NONE;
		break;

	case TAR_CHAR_OFFENSIVE:
		if (!(victim = get_char_room(ch, NULL, ch->cast_target_name))) {
			send_to_char("They've left the room.\n\r", ch);
			victim = NULL;
		}

		vo = (void *) victim;
		target = TARGET_CHAR;
		break;

	case TAR_CHAR_DEFENSIVE:
		if (!(victim = get_char_room(ch, NULL, ch->cast_target_name))) {
			send_to_char("They've left the room.\n\r", ch);
			victim = NULL;
		}

		vo = (void *) victim;
		target = TARGET_CHAR;
		break;

	case TAR_CHAR_SELF:
		victim = ch;
		vo = (void *) victim;
		target = TARGET_CHAR;
		break;

	case TAR_OBJ_INV:
		obj = get_obj_list(ch, ch->cast_target_name, ch->carrying);
		if (!obj) {
			send_to_char("Your target seems to have vanished.\n\r", ch);
			obj = NULL;
		}

		vo = (void *) obj;
		target = TARGET_OBJ;
		break;

	case TAR_OBJ_GROUND:
		obj = get_obj_list(ch, ch->cast_target_name, ch->in_room->contents);
		if (!obj) {
			send_to_char("Your target seems to have vanished.\n\r", ch);
			obj = NULL;
		}

		vo = (void *) obj;
		target = TARGET_OBJ;
		break;

	case TAR_OBJ_CHAR_OFF:
		if ((victim = get_char_room(ch, NULL, ch->cast_target_name))) {
			target = TARGET_CHAR;
			vo = (void *) victim;
		} else if ((obj = get_obj_list(ch, ch->cast_target_name, ch->carrying))) {
			target = TARGET_OBJ;
			vo = (void *) obj;
		} else {
			send_to_char("Your target is no longer here.\n\r", ch);
			target = TARGET_CHAR;
			victim = NULL;
		}
		break;

	case TAR_OBJ_CHAR_DEF:
		if ((victim = get_char_room(ch, NULL, ch->cast_target_name))) {
			target = TARGET_CHAR;
			vo = (void *) victim;
		} else if ((obj = get_obj_list(ch, ch->cast_target_name, ch->carrying))) {
			target = TARGET_OBJ;
			vo = (void *) obj;
		} else {
			send_to_char("Your target is no longer here.\n\r", ch);
			target = TARGET_CHAR;
			victim = NULL;
		}
		break;

	case TAR_IGNORE_CHAR_DEF:
		victim = get_char_world(ch, ch->cast_target_name);
		if (!victim) {
			send_to_char("They aren't in the world of Sentience.\n\r", ch);
			victim = NULL;
		}

		vo = (void *) victim;
		break;
	}

	// The targets weren't found. So the spell isn't cast.
	if ((target == TARGET_CHAR && !victim) || (target == TARGET_OBJ && !obj)) {
		free_string(ch->cast_target_name);
		ch->cast_target_name = NULL;
		return;
	}

	/* Spell trap in the room ? */
	for (trap = ch->in_room->contents; trap; trap = trap->next_content) {
		if (trap->item_type == ITEM_SPELL_TRAP) {
			trap->level -= ch->tot_level/4;
			act("{Y$p sucks up $n's spell!{x", ch, trap, NULL, TO_ROOM);
			act("{Y$p sucks up your spell!{x", ch, trap, NULL, TO_CHAR);
			if (trap->level <= 0) {
				CHAR_DATA *dam_vict;

				act("{R$p shatters explosively!{x", ch, trap, NULL, TO_ROOM);
				act("{R$p shatters explosively!{x", ch, trap, NULL, TO_CHAR);
				if (!IS_SET(ch->in_room->room_flags, ROOM_SAFE)) {
					for (dam_vict = ch->in_room->people; dam_vict; dam_vict = dam_vict->next_in_room) {
						act("{RYou are struck by $p's shards!", dam_vict, trap, NULL, TO_CHAR);
						act("{R$n is struck by $p's shards!", dam_vict, trap, NULL, TO_ROOM);
						damage(dam_vict, dam_vict, dice(60, 8), 0, DAM_PIERCE, FALSE);
					}
				}
				extract_obj(trap);
			}

			deduct_mana(ch,mana);
			stop_casting(ch, FALSE);
			return;
		}
	}

	if(token) {
		if(!IS_SET(token->pIndexData->flags,TOKEN_NOSKILLTEST)) {
			chance = 0;
			if (IS_SET(ch->in_room->room2_flags, ROOM_HARD_MAGIC)) chance += 2;
			if (ch->in_room->sector_type == SECT_CURSED_SANCTUM) chance += 2;
			if (!IS_NPC(ch) && chance > 0 && number_range(1,chance) > 1) {
				send_to_char("You can't seem to focus your mana into the spell.\n\r", ch);
				deduct_mana(ch,mana / 3);
				return;
			} else {
				if (number_range(0,token->pIndexData->value[TOKVAL_SPELL_RATING]) > token->value[TOKVAL_SPELL_RATING]) {
					send_to_char("You lost your concentration.\n\r", ch);
					token_skill_improve(ch,token,FALSE,1);
					deduct_mana(ch,mana / 2);
					return;
				}
			}
		}

		deduct_mana(ch,mana);

		// If casted on a relic puller make the offender PK
		if (is_pulling_relic(victim) && (type == TAR_CHAR_OFFENSIVE || type == TAR_OBJ_CHAR_OFF))
			set_pk_timer(ch, victim, PULSE_VIOLENCE * 4);

		id[0] = token->id[0];
		id[1] = token->id[1];
		if (target == TARGET_CHAR && victim && IS_AFFECTED2(victim, AFF2_SPELL_DEFLECTION)) {
			if (check_spell_deflection_token(ch, victim, token, script)) {
				token->value[3] = ch->tot_level;
				execute_script(script->vnum, script, NULL, NULL, NULL, token, ch, NULL, NULL, victim, NULL,ch->cast_target_name,NULL);
			}
		} else {
			token->value[3] = ch->tot_level;
			execute_script(script->vnum, script, NULL, NULL, NULL, token, ch, (target == TARGET_OBJ)?obj:NULL, NULL, (target == TARGET_CHAR)?victim:NULL, NULL,ch->cast_target_name,NULL);
		}

		// Only bother with the token if it is valid and the SAME token as before the casting
		if(IS_VALID(token) && id[0] == token->id[0] && id[1] == token->id[1]) {
			// If we don't do a skill test before the code is executed,
			//	then successful casting also does no test
			if(!IS_SET(token->pIndexData->flags,TOKEN_NOSKILLTEST))
				token_skill_improve(ch,token,TRUE,1);
		}

	} else {
		chance = 0;
		if (IS_SET(ch->in_room->room2_flags, ROOM_HARD_MAGIC)) chance += 2;
		if (ch->in_room->sector_type == SECT_CURSED_SANCTUM) chance += 2;
		if (!IS_NPC(ch) && chance > 0 && number_range(1,chance) > 1) {
			send_to_char("You can't seem to focus your mana into the spell.\n\r", ch);
			deduct_mana(ch,mana / 3);
			return;
		}
		if (number_percent() > get_skill(ch,sn)) {
			send_to_char("You lost your concentration.\n\r", ch);
			check_improve(ch,sn,FALSE,1);
			deduct_mana(ch,mana / 2);
			return;
		}

		deduct_mana(ch,mana);

		// If casted on a relic puller make the offender PK
		if (is_pulling_relic(victim) && (type == TAR_CHAR_OFFENSIVE || type == TAR_OBJ_CHAR_OFF))
			set_pk_timer(ch, victim, PULSE_VIOLENCE * 4);

		if (target == TARGET_CHAR && victim && IS_AFFECTED2(victim, AFF2_SPELL_DEFLECTION)) {
			if (check_spell_deflection(ch, victim, sn))
				(*skill_table[sn].spell_fun) (sn, ch->tot_level, ch, vo, target);
		} else
			(*skill_table[sn].spell_fun) (sn, ch->tot_level, ch, vo, target);

		check_improve(ch,sn,TRUE,1);

	}

	if ((type == TAR_CHAR_OFFENSIVE || (type == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR)) &&
		victim != ch && victim->master != ch && !IS_DEAD(victim)) {
		CHAR_DATA *vch;
		CHAR_DATA *vch_next;

		for (vch = ch->in_room->people; vch; vch = vch_next) {
			vch_next = vch->next_in_room;
			if (victim == vch && !victim->fighting) {
				multi_hit(victim, ch, TYPE_UNDEFINED);
				break;
			}
		}
	}

	free_string(ch->cast_target_name);
	ch->cast_target_name = NULL;
}


/*
 * Lets a character cast spells at targets using a magical object: scroll, wand, etc.
 * Effect only, no spell delay.
 */
void obj_cast_spell(int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj)
{
    void *vo;
    int target = TARGET_NONE;

    if (sn <= 0)
	return;

    if (sn >= MAX_SKILL || skill_table[sn].spell_fun == 0)
    {
	bug("Obj_cast_spell: bad sn %d.", sn);
	return;
    }

    switch (skill_table[sn].target)
    {
        default:
	    bug("Obj_cast_spell: bad target for sn %d.", sn);
	    return;

	case TAR_IGNORE:
	    vo = NULL;
	    break;

	case TAR_CHAR_OFFENSIVE:
	    if (victim == NULL)
  	        victim = ch->fighting;

	    if (victim == NULL)
	    {
	        send_to_char("You can't do that.\n\r", ch);
		return;
	    }

	    if (is_safe(ch,victim, TRUE) && ch != victim)
	    {
	        send_to_char("Something isn't right...\n\r",ch);
		return;
	    }

	    vo = (void *) victim;
	    target = TARGET_CHAR;
	    break;

	case TAR_CHAR_DEFENSIVE:
	case TAR_CHAR_SELF:
	    if (victim == NULL)
		    victim = ch;

	    vo = (void *) victim;
	    target = TARGET_CHAR;
	    break;

	case TAR_OBJ_INV:
	    if (obj == NULL)
	    {
	        send_to_char("You can't do that.\n\r", ch);
		return;
	    }
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	    break;

	case TAR_OBJ_CHAR_OFF:
	    if (victim == NULL && obj == NULL)
	    {
	        if (ch->fighting != NULL)
	   	    victim = ch->fighting;
  	        else
	        {
		    send_to_char("You can't do that.\n\r",ch);
	   	    return;
		}
	    }

	    if (victim != NULL)
	    {
	        if (is_safe_spell(ch,victim,FALSE) && ch != victim)
	        {
	            send_to_char("Something isn't right...\n\r",ch);
		    return;
		}

		vo = (void *) victim;
		target = TARGET_CHAR;
	    }
	    else
	    {
		    vo = (void *) obj;
		    target = TARGET_OBJ;
	    }
	    break;


	case TAR_OBJ_CHAR_DEF:
	    if (victim == NULL && obj == NULL)
	    {
	        vo = (void *) ch;
		target = TARGET_CHAR;
	    }
	    else if (victim != NULL)
	    {
		vo = (void *) victim;
		target = TARGET_CHAR;
	    }
	    else
	    {
		vo = (void *) obj;
		target = TARGET_OBJ;
	    }

	    break;
    }

    if (target == TARGET_CHAR && victim != NULL)
    {
	if (check_spell_deflection(ch, victim, sn))
	    (*skill_table[sn].spell_fun) (sn, level, ch, vo, target);
    }
    else
        (*skill_table[sn].spell_fun) (sn, level, ch, vo,target);

    if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE
        || (skill_table[sn].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR))
    &&  victim != ch
    &&  victim->master != ch)
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	for (vch = ch->in_room->people; vch; vch = vch_next)
	{
	    vch_next = vch->next_in_room;
	    if (victim == vch && victim->fighting == NULL)
	    {
		multi_hit(victim, ch, TYPE_UNDEFINED);
		break;
	    }
	}
    }
}


// Lets an object cast a spell all by itself.
void obj_cast(int sn, int level, OBJ_DATA *obj, ROOM_INDEX_DATA *room, char *argument)
{
    CHAR_DATA *ch;
    CHAR_DATA *victim = NULL;
    OBJ_DATA *target_obj = NULL;
    OBJ_DATA *reagent;
    void *vo;
    int target = TARGET_NONE;
    char buf[MSL];

    ch = create_mobile(get_mob_index(MOB_VNUM_OBJCASTER));
    char_to_room(ch, room);

    ch->level = obj->level;
    free_string(ch->name);
    ch->name = str_dup(obj->name);
    free_string(ch->short_descr);
    ch->short_descr = str_dup(obj->short_descr);

    // Make sure they have a reagent for the powerful spells
    reagent = create_object(get_obj_index(OBJ_VNUM_SHARD), 1, FALSE);
    obj_to_char(reagent,ch);

    switch (skill_table[sn].target)
    {
        default:
	    bug("obj_cast: bad target for sn %d.", sn);
	    return;

	case TAR_IGNORE:
	    vo = NULL;
	    break;

	case TAR_CHAR_OFFENSIVE:
	    victim = get_char_room(NULL, room, argument);

	    vo = (void *) victim;
	    target = TARGET_CHAR;
	    break;

	case TAR_CHAR_DEFENSIVE:
	case TAR_CHAR_SELF:
	    victim = get_char_room(NULL, room, argument);

	    vo = (void *) victim;
	    target = TARGET_CHAR;
	    break;

	case TAR_OBJ_INV:
	case TAR_OBJ_GROUND:
	    target_obj = get_obj_list(ch, argument, room->contents);

	    vo = (void *) target_obj;
	    target = TARGET_OBJ;
	    break;

	case TAR_OBJ_CHAR_OFF:
	    victim = get_char_room(NULL, room, argument);
	    target_obj = get_obj_list(ch, argument, room->contents);

	    if (victim != NULL)
	    {
		vo = (void *) victim;
		target = TARGET_CHAR;
	    }
	    else
	    {
		vo = (void *) target_obj;
		target = TARGET_OBJ;
	    }
	    break;

	case TAR_OBJ_CHAR_DEF:
	    if ((victim = get_char_room(NULL, room, argument)) != NULL)
	    {
		vo = (void *) victim;
		target = TARGET_CHAR;
	    }
	    else
	    {
		target_obj = get_obj_list(ch, argument, room->contents);
		vo = (void *) target_obj;
		target = TARGET_OBJ;
	    }
	    break;
	case TAR_IGNORE_CHAR_DEF:
	    victim = get_char_world(NULL, argument);
	    vo = (void *) victim;
	    target = TARGET_CHAR;
	    break;
    }

    if (target == TARGET_CHAR && victim != NULL)
    {
	if (is_affected(victim, sn))
	    return;

	if (!check_spell_deflection(ch, victim, sn)) {
	    extract_char(ch, TRUE);
	    return;
	}
    }

    if ((target == TARGET_CHAR && victim != NULL)
    ||   (target == TARGET_OBJ  && target_obj != NULL)
    ||    target == TARGET_ROOM
    ||    target == TARGET_NONE)
	(*skill_table[sn].spell_fun)(sn, obj->level, ch, vo, target);
    else
    {
	sprintf(buf, "obj_cast: %s(%ld) couldn't find its target", obj->short_descr, obj->pIndexData->vnum);
	log_string(buf);
    }

    extract_char(ch, TRUE);
}

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

int cap_spell_damage(int dam)
{
	return UMIN(dam,2500);
}


















































bool can_escape(CHAR_DATA *ch)
{
	//Added area_no_recall check to go with corresponding area flag - Areo 08-10-2006
	if (!ch->in_room->area->open ||
		!str_cmp(ch->in_room->area->name, "Netherworld") ||
		!str_cmp(ch->in_room->area->name, "Eden") ||
		!str_cmp(ch->in_room->area->name, "Ethereal Void") ||
		!str_cmp(ch->in_room->area->name, "Social") ||
		!str_cmp(ch->in_room->area->name, "Arena") ||
		!str_prefix("Maze", ch->in_room->area->name) ||
		!str_cmp(ch->in_room->area->name, "Pyramid of the Abyss") ||
		IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL) ||
		IS_SET(ch->in_room->area->area_flags, AREA_NO_RECALL)) {
		send_to_char("Outside interference stops your transportation.\n\r", ch);
		return FALSE;
	}

	if (ch->pulled_cart) {
		act("You can't take $p with you.", ch, ch->pulled_cart, NULL, TO_CHAR);
		return FALSE;
	}

	return TRUE;
}


bool can_gate(CHAR_DATA *ch, CHAR_DATA *victim)
{
	char buf[MAX_STRING_LENGTH];

	if (IS_NPC(victim))
		return FALSE;

	if (victim == ch) {
		send_to_char("What would be the point?\n\r", ch);
		return FALSE;
	}

	if (!victim || !victim->in_room) {
		send_to_char("They aren't anywhere in the world.\n\r", ch);

		sprintf(buf, "can_gate: %s tried to gate to %s who had null in_room!",
			ch->name, (!victim) ? "nobody???" : victim->name);
		bug(buf, 0);
		return FALSE;
	}

	/* take care of the wilderness case */
	if (!str_cmp(ch->in_room->area->name, "Wilderness")) {
		if (ch->in_room->sector_type == SECT_WATER_NOSWIM) {
			send_to_char("The deep ocean waters cancel out your magic.\n\r", ch);
			return FALSE;
		}

		if (get_region(ch) != get_region(victim)) {
			act("$N is too far away.", ch, NULL, victim, TO_CHAR);
			return FALSE;
		}
	}

	//Added area_no_recall check to go with corresponding area flag - Areo 08-10-2006
	if (!str_cmp(ch->in_room->area->name, "Netherworld") ||
		!str_cmp(ch->in_room->area->name, "Eden") ||
		!str_cmp(victim->in_room->area->name, "Netherworld") ||
		!str_cmp(victim->in_room->area->name, "Eden") ||
		!str_cmp(ch->in_room->area->name, "Arena") ||
		!str_cmp(victim->in_room->area->name, "Arena") ||
		!str_prefix("Church Temples", ch->in_room->area->name) ||
		!str_prefix("Church Temples", victim->in_room->area->name) ||
		IS_SET(ch->in_room->area->place_flags, PLACE_NOWHERE) ||
		IS_SET(victim->in_room->area->place_flags, PLACE_NOWHERE) ||
		IS_SET(ch->in_room->area->place_flags, PLACE_OTHER_PLANE) ||
		IS_SET(victim->in_room->area->place_flags, PLACE_OTHER_PLANE) ||
		IS_SET(victim->in_room->area->area_flags, AREA_NO_RECALL)) {
		send_to_char("Outside interference stops your transportation.\n\r", ch);
		return FALSE;
	}

	if (IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL) ||
		IS_SET(victim->in_room->room_flags, ROOM_NOMAGIC) ||
		IS_SET(victim->in_room->room_flags, ROOM_CPK)) {
		send_to_char("That room is protected from gating magic.\n\r", ch);
		return FALSE;
	}

	if (str_cmp(ch->in_room->area->name, "Wilderness") &&
		!SAME_PLACE(ch->in_room, victim->in_room)) {
		send_to_char("Your destination is too far away.\n\r", ch);
		return FALSE;
	}

	return TRUE;
}

SPELL_FUNC(spell_null)
{
	send_to_char("That's not a spell!\n\r", ch);
	return FALSE;
}






/////









































// Find a warpstone(astral) on a character.
OBJ_DATA *get_warp_stone(CHAR_DATA *ch)
{
	OBJ_DATA *obj;
	OBJ_DATA *objNest;

	for (obj = ch->carrying; obj != NULL; obj = obj->next_content) {
		if (obj->item_type == ITEM_CATALYST && obj->value[0] == CATALYST_ASTRAL) {
			return obj;
		} else if (obj->contains) {
			for (objNest = obj->contains; objNest; objNest = objNest->next_content) {
				if (objNest->item_type == ITEM_CATALYST && obj->value[0] == CATALYST_ASTRAL)
					return objNest;
			}
		}
	}

	return NULL;
}






void do_reverie(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    //char buf[MSL];
    int amount;
    int time;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (is_dead(ch))
	return;

    if (get_skill(ch,gsn_reverie) == 0)
    {
	send_to_char("You do not have this skill.\n\r",ch);
	return;
    }

    if(arg1[0] =='\0')
    {
	send_to_char("Transfer to the body or to the mind?\n\r", ch);
	return;
    }

    if ((str_cmp(arg1, "body")) && (str_cmp(arg1, "mind")))
    {
	send_to_char("Syntax: reverie <body|mind> <amount>\n\r", ch);
	return;
    }

    if(arg2[0] =='\0')
    {
	send_to_char("How much do you want to transfer?\n\r", ch);
	return;
    }

    if (IS_SET(ch->has_done, DONE_REVERIE))
    {
	send_to_char("Your meditative energies are already drained.\n\r", ch);
	return;
    }

    amount = atoi(arg2);

    if (amount <= 0)
    {
    	send_to_char("You can't transfer negative amounts.\n\r", ch);
    	return;
    }

    if(!str_cmp(arg1, "body") && amount * 2 > ch->mana)
    {
	send_to_char("You don't have that much mana to transfer.\n\r", ch);
	return;
    }

    if(!str_cmp(arg1, "mind") && amount * 2 > ch->hit)
    {
	send_to_char("You don't have that much health to transfer.\n\r", ch);
	return;
    }

    if ((!str_cmp(arg1,"mind") && amount > ch->max_hit)
    ||   (!str_cmp(arg1,"body") && amount > ch->max_mana))
    {
	send_to_char("That's more than your body can handle.\n\r", ch);
	return;
    }

    ch->reverie_amount = amount;

    if(!str_cmp(arg1, "body"))
    {
	send_to_char("{YYou close your eyes and begin to meditate, channeling mana into your body...{x\n\r", ch);
	ch->reverie_type = MANA_TO_HIT;
    }
    else
    {
	send_to_char("{YYou relax and tap your physical strength, converting it to mana...{x\n\r", ch);
	ch->reverie_type = HIT_TO_MANA;
    }

    time = 4;
    time += (amount/120) * UMIN((100/get_skill(ch,gsn_reverie)),50);
    //sprintf(buf, "%d", time); gecho(buf);
    REVERIE_STATE(ch,time);
}


void reverie_end(CHAR_DATA *ch, int amount)
{
    char buf[MAX_STRING_LENGTH];

    if (ch->reverie_type == MANA_TO_HIT)
    {
        ch->mana -= ch->reverie_amount * 2;
	ch->hit += ch->reverie_amount;
    }
    else
    {
	ch->hit -= ch->reverie_amount * 2;
	ch->mana += ch->reverie_amount;
    }

    sprintf(buf, "{YYou have finished your reverie.{x");
    act(buf, ch, NULL, NULL, TO_CHAR);
    sprintf(buf, "{Y$n has finished $s reverie.{x");
    act(buf, ch, NULL, NULL, TO_ROOM);
    sprintf(buf, "You have transferred %i %s to your %s.\n\r",
	amount,
	ch->reverie_type == MANA_TO_HIT ? "mana" : "hit points",
	ch->reverie_type == MANA_TO_HIT ? "body" : "mind");
    send_to_char(buf, ch);

    ch->hit = UMAX(1, ch->hit);
    ch->mana = UMAX(1, ch->mana);

    ch->reverie_amount = 0;

    if (ch->fighting != NULL)
	SET_BIT(ch->has_done, DONE_REVERIE);

    check_improve(ch, gsn_reverie, TRUE, 1);
}
































