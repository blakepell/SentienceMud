/***************************************************************************
 *                                                                         *
 *    Scripting engine rebuilt by Michael Kurtz (Nibelung)                 *
 *    Used with permission.                                                *
 *                                                                         *
 **************************************************************************/

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


SPELL_FUNC(spell_calm)
{
	CHAR_DATA *vch;
	AFFECT_DATA af;
	bool found = FALSE;

	memset(&af,0,sizeof(af));

	for (vch = ch->in_room->people; vch; vch = vch->next_in_room) {
		if ((IS_NPC(vch) && (IS_SET(vch->imm_flags,IMM_MAGIC) || IS_SET(vch->act,ACT_UNDEAD))) || IS_IMMORTAL(vch))
			continue;

		if (!check_spell_deflection(ch, vch, sn))
			continue;

		if (IS_AFFECTED(vch,AFF_CALM) || IS_AFFECTED(vch,AFF_BERSERK) || IS_AFFECTED(vch,AFF_FRENZY) || (!is_same_group(vch, ch) && !is_same_group(vch, ch->fighting)))
			continue;

		found = TRUE;
		send_to_char("A wave of calm passes over you.\n\r",vch);

		if (vch->fighting || vch->position == POS_FIGHTING)
			stop_fighting(vch,FALSE);

		af.slot	= WEAR_NONE;
		af.where = TO_AFFECTS;
		af.group = AFFGROUP_MENTAL;
		af.type = sn;
		af.level = level;
		af.duration = level/4;
		af.location = APPLY_HITROLL;
		af.modifier = IS_NPC(vch) ? -2 : -5;
		af.bitvector = AFF_CALM;
		af.bitvector2 = 0;
		affect_to_char(vch,&af);

		af.location = APPLY_DAMROLL;
		affect_to_char(vch,&af);
	}
	return found;
}


SPELL_FUNC(spell_charm_person)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	memset(&af,0,sizeof(af));

	if (is_safe(ch,victim, TRUE))
		return FALSE;

	if (victim == ch) {
		send_to_char("You like yourself even better!\n\r", ch);
		return FALSE;
	}

	if (IS_SET(victim->imm_flags, IMM_CHARM)) {
		act("No matter how hard you try, you can't bend $N to your will.", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
		return FALSE;
	}

	if (IS_AFFECTED(victim, AFF_CHARM) || IS_AFFECTED(ch, AFF_CHARM) ||
		level < victim->tot_level || IS_SET(victim->imm_flags,IMM_CHARM) ||
		saves_spell(level, victim,DAM_CHARM))
		return FALSE;

	if (!IS_NPC(victim) && !IS_SET(victim->in_room->room_flags, ROOM_CPK)) {
		send_to_char("You can only charm players in a Chaotic Player Killing room.\n\r", ch);
		return FALSE;
	}

	if (IS_NPC(victim) && victim->tot_level >= (ch->tot_level+15)) {
		act("$N seems unaffected by your attempted charm.", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
		return FALSE;
	}

	if (RIDDEN(victim)) {
		act("$N is completely under $S master's control.", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
		return FALSE;
	}

	if (victim->master) stop_follower(victim,TRUE);

	add_follower(victim, ch, TRUE);
	if (!add_grouped(victim, ch, TRUE))
		return FALSE;

	af.slot	= WEAR_NONE;
	af.where = TO_AFFECTS;
	af.group = AFFGROUP_MENTAL;
	af.type = sn;
	af.level = level;
	af.duration = number_fuzzy(level / 4);
	af.location = 0;
	af.modifier = 0;
	af.bitvector = AFF_CHARM;
	af.bitvector2 = 0;
	affect_to_char(victim, &af);

	if (IS_NPC(victim) && IS_SET(victim->act, ACT_AGGRESSIVE))
		REMOVE_BIT(victim->act, ACT_AGGRESSIVE);

	act("Isn't $n just so nice?", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_VICT);
	if (ch != victim)
		act("$N looks at you with adoring eyes.",ch,victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
	return TRUE;
}


SPELL_FUNC(spell_detect_hidden)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	bool perm = FALSE;
	memset(&af,0,sizeof(af));

	if (level > MAGIC_WEAR_SPELL) {
		level -= MAGIC_WEAR_SPELL;
		perm = TRUE;
	}

	if (perm && is_affected(victim, sn)) {
		affect_strip(victim, sn);
	} else if (IS_AFFECTED(victim, AFF_DETECT_HIDDEN)) {
		if (victim == ch)
			send_to_char("You are already as alert as you can be. \n\r",ch);
		else
			act("$N can already sense hidden lifeforms.",ch,victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
		return FALSE;
	}

	af.slot = obj_wear_loc;
	af.where = TO_AFFECTS;
	af.group = AFFGROUP_MAGICAL;
	af.type = sn;
	af.level = level;
	af.duration = perm ? -1 : level;
	af.location = APPLY_NONE;
	af.modifier = 0;
	af.bitvector = AFF_DETECT_HIDDEN;
	af.bitvector2 = 0;
	affect_to_char(victim, &af);
	send_to_char("Your awareness improves.\n\r", victim);
	if (ch != victim)
		act("$N blinks as $S awareness improves.", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
	return TRUE;
}


SPELL_FUNC(spell_detect_invis)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	bool perm = FALSE;
	memset(&af,0,sizeof(af));

	if (level > MAGIC_WEAR_SPELL) {
		level -= MAGIC_WEAR_SPELL;
		perm = TRUE;
	}

	if (perm && is_affected(victim, sn)) {
		affect_strip(victim, sn);
	} else if (IS_AFFECTED(victim, AFF_DETECT_INVIS)) {
		if (victim == ch)
			send_to_char("You can already see invisible.\n\r",ch);
		else
			act("$N can already see invisible things.",ch,victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
		return FALSE;
	}

	af.slot = obj_wear_loc;
	af.where = TO_AFFECTS;
	af.group = AFFGROUP_MAGICAL;
	af.type = sn;
	af.level = level;
	af.duration = perm ? -1 : level;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_DETECT_INVIS;
	af.bitvector2 = 0;
	affect_to_char(victim, &af);
	send_to_char("Your eyes tingle.\n\r", victim);
	if (ch != victim)
		act("$N blinks as $S eyes tingle.", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
	return TRUE;
}


SPELL_FUNC(spell_detect_magic)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	bool perm = FALSE;
	memset(&af,0,sizeof(af));

	if (level > MAGIC_WEAR_SPELL) {
		level -= MAGIC_WEAR_SPELL;
		perm = TRUE;
	}

	if (perm && is_affected(victim, sn))
		affect_strip(victim, sn);
	else if (IS_AFFECTED(victim, AFF_DETECT_MAGIC)) {
		if (victim == ch)
			send_to_char("You can already sense magical auras.\n\r",ch);
		else
			act("$N can already detect magic.",ch,victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
		return FALSE;
	}

	af.slot = obj_wear_loc;
	af.where = TO_AFFECTS;
	af.group = AFFGROUP_MAGICAL;
	af.type = sn;
	af.level = level;
	af.duration = perm ? -1 : level;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_DETECT_MAGIC;
	af.bitvector2 = 0;
	affect_to_char(victim, &af);
	send_to_char("Your eyes tingle.\n\r", victim);
	if (ch != victim)
		act("$N blinks as $S eyes become sensitive to magic.", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
	return TRUE;
}

SPELL_FUNC(spell_frenzy)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	bool perm = FALSE;
	memset(&af,0,sizeof(af));

	if (level > MAGIC_WEAR_SPELL) {
		level -= MAGIC_WEAR_SPELL;
		perm = TRUE;
	}

	if (perm && is_affected(victim, sn)) {
		affect_strip(victim, sn);
	} else if (is_affected(victim,sn) || IS_AFFECTED(victim,AFF_BERSERK)) {
		if (victim == ch)
			send_to_char("You are already in a frenzy.\n\r",ch);
		else
			act("$N is already in a frenzy.",ch,victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
		return FALSE;
	}

	if (is_affected(victim,gsn_calm)) {
		if (victim == ch)
			send_to_char("Why don't you just relax for a while?\n\r",ch);
		else
			act("$N doesn't look like $e wants to fight anymore.", ch,victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
		return FALSE;
	}

	if ((IS_GOOD(ch) && !IS_GOOD(victim)) ||
		(IS_NEUTRAL(ch) && !IS_NEUTRAL(victim)) ||
		(IS_EVIL(ch) && !IS_EVIL(victim))) {
		act("Your god doesn't seem to like $N.",ch,victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
		return FALSE;
	}

	af.slot = obj_wear_loc;
	af.where = TO_AFFECTS;
	af.group = AFFGROUP_DIVINE;
	af.type = sn;
	af.level = level;
	af.duration = perm ? -1 : (level / 3);
	af.modifier  = level / 6;
	af.bitvector = AFF_FRENZY;
	af.bitvector2 = 0;

	af.location  = APPLY_HITROLL;
	affect_to_char(victim,&af);

	af.location  = APPLY_DAMROLL;
	affect_to_char(victim,&af);

	af.modifier  = 10 * (level / 12);
	af.location  = APPLY_AC;
	affect_to_char(victim,&af);

	send_to_char("You are filled with holy wrath!\n\r",victim);
	act("$n gets a wild look in $s eyes!",victim,NULL,NULL, NULL, NULL, NULL, NULL,TO_ROOM);
	return TRUE;
}

SPELL_FUNC(spell_morphlock)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	int lvl, catalyst;
	bool perm = FALSE;

	memset(&af,0,sizeof(af));

	if (level > MAGIC_WEAR_SPELL) {
		level -= MAGIC_WEAR_SPELL;
		perm = TRUE;
	}

	if (perm && is_affected(victim, sn)) {
		affect_strip(victim, sn);
	} else if (is_affected(victim,sn) || IS_AFFECTED2(victim, AFF2_MORPHLOCK)) {
		if (victim == ch)
			send_to_char("You are already confined to your shape.\n\r",ch);
		else
			act("$N is already confined to $S shape.", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
		return FALSE;

		lvl = victim->tot_level - ch->tot_level;
		if(IS_REMORT(victim)) lvl += LEVEL_HERO;	// If the victim is remort, it will require MORE catalyst
		if(IS_REMORT(ch)) lvl -= LEVEL_HERO;		// If the caster is remort, it will require LESS catalyst
		lvl = (lvl > 19) ? (lvl / 10) : 1;

		catalyst = has_catalyst(ch,NULL,CATALYST_MIND,CATALYST_INVENTORY,1,CATALYST_MAXSTRENGTH);
		if(catalyst >= 0 && catalyst < lvl) {
			send_to_char("You appear to be missing a required mental catalyst.\n\r", ch);
			return FALSE;
		}

		use_catalyst(ch,NULL,CATALYST_MIND,CATALYST_INVENTORY,lvl,1,CATALYST_MAXSTRENGTH,TRUE);

		if (saves_spell(level,victim,DAM_MENTAL)) {
			send_to_char("Nothing happens.\n\r", ch);
			return FALSE;
		}

	}

	af.slot = obj_wear_loc;
	af.where = TO_AFFECTS;
	af.group = AFFGROUP_MENTAL;
	af.type = sn;
	af.location = APPLY_NONE;
	af.modifier = 0;
	af.level = level + 1;
	af.duration = perm ? -1 : (2 + (int) ch->tot_level/10);
	af.bitvector = 0;
	af.bitvector2 = AFF2_MORPHLOCK;
	affect_to_char(victim, &af);
	act("{WA wave of mental energy constrains your being.{x", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_VICT);
	return TRUE;
}

SPELL_FUNC(spell_sleep)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	memset(&af,0,sizeof(af));

	if (IS_AFFECTED(victim, AFF_SLEEP) || (IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD)) ||
		(level + 2) < victim->tot_level || saves_spell(level-4, victim,DAM_CHARM))
		return FALSE;

	af.slot	= WEAR_NONE;
	af.where = TO_AFFECTS;
	af.group = AFFGROUP_MAGICAL;
	af.type = sn;
	af.level = level;
	af.duration = 1;
	af.location = APPLY_NONE;
	af.modifier = 0;
	af.bitvector = AFF_SLEEP;
	af.bitvector2 = 0;
	affect_join(victim, &af);

	if (IS_AWAKE(victim)) {
		send_to_char("You feel very sleepy ..... zzzzzz.\n\r", victim);
		act("$n drops like a rock into a deep slumber.", victim, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
		victim->position = POS_SLEEPING;
	}
	return TRUE;
}


SPELL_FUNC(spell_third_eye)
{
	CHAR_DATA *victim;
	OBJ_DATA *skull;
	AFFECT_DATA af;
	AFFECT_DATA *af_old;
	memset(&af,0,sizeof(af));

	skull = (OBJ_DATA *) vo;
	if (skull->pIndexData->vnum != OBJ_VNUM_SKULL && skull->pIndexData->vnum != OBJ_VNUM_GOLD_SKULL) {
		send_to_char("This spell must be cast on a skull.\n\r", ch);
		return FALSE;
	}

	if (IS_SET(skull->extra2_flags, ITEM_THIRD_EYE)) {
		act("$p has already been enchanted with third eye.", ch, NULL, NULL, skull, NULL, NULL, NULL, TO_CHAR);
		return FALSE;
	}

	if (!(victim = get_char_world(NULL, skull->owner))) {
		act("You cannot locate the soul of $T.", ch, NULL, NULL, NULL, NULL, NULL, skull->owner, TO_CHAR);
		return FALSE;
	}

	for (af_old = skull->affected; af_old; af_old = af_old->next) {
		if (af_old->type == sn) {
			act("The soul of $T is already bound to $p.", ch, NULL, NULL, skull, NULL, skull->owner, NULL, TO_CHAR);
			return FALSE;
		}
	}

	act("{YYou send a flow of dark power into $p, binding it to $S soul.{x", ch, victim, NULL, skull, NULL, NULL, NULL, TO_CHAR);
	act("{Y$n grasps $p and chants words of dark power.{x", ch, NULL, NULL, skull, NULL, NULL, NULL, TO_ROOM);
	send_to_char("{WYou get a momentary shiver up your spine.{x\n\r", victim);

	af.slot	= WEAR_NONE;
	af.where = TO_OBJECT;
	af.group = AFFGROUP_ENCHANT;
	af.type	= sn;
	af.level = level;
	af.duration = (level * level / 900);	// 1 to 16
	af.duration = UMAX(1,af.duration);
	af.location = APPLY_NONE;
	af.modifier = 0;
	af.bitvector = 0;
	af.bitvector2 = ITEM_THIRD_EYE;
	affect_to_obj(skull, &af);
	return TRUE;
}
