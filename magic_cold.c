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

SPELL_FUNC(spell_chill_touch)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	int dam;

	memset(&af,0,sizeof(af));

	dam = level + UMIN(victim->max_hit/15, 100);
	if (!saves_spell(level, victim,DAM_COLD)) {
		act("{C$n turns blue and shivers.{x",victim, NULL, NULL, NULL, NULL,NULL,NULL,TO_ROOM);
		af.where = TO_AFFECTS;
		af.group = AFFGROUP_MAGICAL;
		af.type = sn;
		af.level = level;
		af.duration = 6;
		af.location = APPLY_STR;
		af.modifier = -1;
		af.bitvector = 0;
		af.bitvector2 = 0;
		affect_join(victim, &af);
	} else
		dam /= 2;

	damage(ch, victim, dam, sn, DAM_COLD,TRUE);
	return TRUE;
}

SPELL_FUNC(spell_frost_barrier)
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
	else if (is_affected(victim, sn)) {
		if (victim == ch)
			send_to_char("You are already surrounded by a frost barrier.\n\r",ch);
		else
			act("$N is already surrounded by a frost barrier.", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
		return FALSE;
	}

	af.where = TO_AFFECTS;
	af.group = AFFGROUP_MAGICAL;
	af.type = sn;
	af.level = level;
	af.duration = perm ? -1 : (level / 3);
	af.location = APPLY_NONE;
	af.modifier = 0;
	af.bitvector = 0;
	af.bitvector2 = AFF2_FROST_BARRIER;
	affect_to_char(victim, &af);
	act("{BYou hear a low-pitched humming sound as the temperature around $n drops.{x", victim, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
	send_to_char("{BThe temperature of the air around you drops rapidly, forming a frost barrier.{x\n\r", victim);
	return TRUE;
}


SPELL_FUNC(spell_frost_breath)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	CHAR_DATA *vch, *vch_next;
	int dam;

	act("$n breathes out a freezing cone of frost!",ch,victim, NULL, NULL, NULL, NULL, NULL,TO_NOTVICT);
	act("You breath out a cone of frost.",ch,NULL,NULL,NULL,NULL,NULL,NULL,TO_CHAR);

	if (check_shield_block_projectile(ch, victim, "freezing cone of frost", NULL))
		return FALSE;

	act("$n breathes a freezing cone of frost over you!", ch,victim, NULL, NULL, NULL, NULL, NULL,TO_VICT);

	dam = level * 13;

	if (IS_DRAGON(ch))
		dam += dam/4;

	cold_effect(victim->in_room,level,dam/7,TARGET_ROOM);

	for (vch = victim->in_room->people; vch; vch = vch_next) {
		vch_next = vch->next_in_room;

		if (is_safe_spell(ch,vch,TRUE) || is_same_group(ch, vch) || (IS_NPC(vch) && IS_NPC(ch) &&
			(ch->fighting != vch || vch->fighting != ch)))
			continue;

		vch->set_death_type = DEATHTYPE_BREATH;

		if (vch == victim) {
			if (saves_spell(level,vch,DAM_COLD)) {
				cold_effect(vch,level/2,dam/4,TARGET_CHAR);
				damage(ch,vch,dam/2,sn,DAM_COLD,TRUE);
			} else {
				cold_effect(vch,level,dam,TARGET_CHAR);
				damage(ch,vch,dam,sn,DAM_COLD,TRUE);
			}
		} else {
			if (saves_spell(level - 2,vch,DAM_COLD)) {
				cold_effect(vch,level/4,dam/8,TARGET_CHAR);
				damage(ch,vch,dam/4,sn,DAM_COLD,TRUE);
			} else {
				cold_effect(vch,level/2,dam/4,TARGET_CHAR);
				damage(ch,vch,dam/2,sn,DAM_COLD,TRUE);
			}
		}
	}

	return TRUE;
}

SPELL_FUNC(spell_ice_shards)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	if(both_hands_full(ch)) {
		act("{CThe air before you freezes in a flash.{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
		act("{CThe air before $n freezes in a flash.{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);

		act("{WYou hurl shards of ice at $N!{x", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
		act("{W$n hurls shards of ice at you!{x", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_VICT);
		act("{W$n hurls shards of ice at $N!{x", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_NOTVICT);
	} else {

		act("{CThe air around your hand freezes in a flash.{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
		act("{CThe air around $n's hand freezes in a flash.{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);

		act("{WYou throw out your hand, hurling shards of ice at $N!{x", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
		act("{W$n throw out $s hand, hurling shards of ice at you!{x", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_VICT);
		act("{W$n throw out $s hand, hurling shards of ice at $N!{x", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_NOTVICT);
	}

	if (IS_SET(ch->in_room->room2_flags, ROOM_FIRE) || ch->in_room->sector_type == SECT_LAVA) {
		act("{RThe intense heat in the area melts the shards with a sizzle.{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ALL);
		return FALSE;
	}

	if (check_shield_block_projectile(ch, victim, "ice shards", NULL))
		return FALSE;

	level	= UMAX(0, level);
	dam	= dice(level, level/7);

	if (saves_spell(level, victim, DAM_COLD)) dam /= 2;

	/* CAP */
	dam = UMIN(dam, 2500);

	damage(ch,victim,dam,gsn_ice_shards, DAM_COLD, TRUE);

	return TRUE;
}

void spellassist_room_freeze(ROOM_INDEX_DATA *room, CHAR_DATA *ch, int level, int sides, int sn)
{
	CHAR_DATA *victim;
	CHAR_DATA *vnext;
	int dam;

	cold_effect((void *) room, level, dice(level,sides), TARGET_ROOM);
	for (victim = room->people; victim != NULL; victim = vnext) {
		vnext = victim->next_in_room;
		if (!is_safe(ch, victim, FALSE) && victim != ch) {
			dam = dice(level,sides);

			damage(ch, victim, dam, sn, DAM_COLD, TRUE);
			cold_effect((void *)victim, level, dam, TARGET_CHAR);
		}
	}
}

SPELL_FUNC(spell_glacial_wave)
{
	ROOM_INDEX_DATA *to_room;
	OBJ_DATA *obj;
	EXIT_DATA *ex;
	char *arg = (char *) vo;
	int depth, depth_scale;
	int max_depth;
	int door;
	int catalyst, i;
	int heat;
	bool do_ice, large = FALSE;
	char buf[MAX_STRING_LENGTH];

	if (!arg) return FALSE;

	if (arg[0] == '\0') {
		send_to_char("Cast it in which direction?\n\r", ch);
		return FALSE;
	}

	if ((door = parse_direction(arg)) == -1) {
		send_to_char("That's not a direction.\n\r", ch);
		return FALSE;
	}

	if (!ch->in_room->exit[door] || !ch->in_room->exit[door]->u1.to_room || (IS_SET(ch->in_room->exit[door]->exit_info,EX_HIDDEN) && !IS_SET(ch->in_room->exit[door]->exit_info,EX_FOUND)) || IS_SET(ch->in_room->exit[door]->exit_info,EX_CLOSED)) {
		send_to_char("You need an open direction.\n\r", ch);
		return FALSE;
	}

	if(IS_NPC(ch) && ch->pIndexData->vnum == MOB_VNUM_OBJCASTER) {	// non-mob caster
		max_depth = 5;
		depth_scale = 50;
		do_ice = FALSE;
		catalyst = 0;
	} else {
		catalyst = use_catalyst(ch,NULL,CATALYST_ICE,CATALYST_ROOM|CATALYST_HOLD,10,1,CATALYST_MAXSTRENGTH,TRUE);

		// If they have an ice catalyst...
		if(catalyst > 0) {
			level = level * number_range(20,20+5*catalyst)/10;
			max_depth = 6 + number_range(0,catalyst)/3;
			for(depth_scale = 50, i=0;i<catalyst;i++, depth_scale = (2099 + 80 * depth_scale) / 100);
			do_ice = TRUE;		// Enable ice storms
			large = number_range(0,9) < catalyst;

		// If not...
		} else {
			max_depth = 5;
			depth_scale = 50;
			do_ice = FALSE;
		}

		act("{BA great cold envelops the hands of $n as the air rapidly cools to well past freezing.{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
		send_to_char("{BA great cold envelops your hands as the air rapidly cools to well past freezing.{x\n\r", ch);

		// Affect all eq at these locations
		if((obj = get_eq_char(ch, WEAR_HANDS))) cold_effect(obj,level,level,TARGET_OBJ);
		if((obj = get_eq_char(ch, WEAR_WIELD))) cold_effect(obj,level,level,TARGET_OBJ);
		if((obj = get_eq_char(ch, WEAR_HOLD))) cold_effect(obj,level,level,TARGET_OBJ);
		if((obj = get_eq_char(ch, WEAR_SHIELD))) cold_effect(obj,level,level,TARGET_OBJ);
		if((obj = get_eq_char(ch, WEAR_SECONDARY))) cold_effect(obj,level,level,TARGET_OBJ);

		act("{C$n gathers cold energy until $e hurls it $tward as a glacial wave.{x", ch, NULL, NULL, NULL, NULL, dir_name[door], NULL, TO_ROOM);
		act("{CYou gather cold energy until you hurl it $tward as a glacial wave.{x", ch, NULL, NULL, NULL, NULL, dir_name[door], NULL, TO_CHAR);
	}

	to_room = ch->in_room;
	// Count the sources of heat
	heat = 0;

	if (IS_SET(to_room->room2_flags, ROOM_FIRE)) heat += 25;
	if (to_room->sector_type == SECT_LAVA) heat += 60;

	for (obj = to_room->contents; obj != NULL; obj = obj->next_content) {
		if (obj->item_type == ITEM_ROOM_FLAME) heat += 15;
	}

	heat = URANGE(0,heat,100);
	heat = heat * (100 - catalyst*catalyst) / 100;	// If they use catalysts, reduce the heat quadratically

	if (do_ice && number_range(0,24) < heat) do_ice = FALSE;	// It takes little to prevent ice storms

	if (number_percent() < heat) {
		act("{RThe intense heat in the area absorbs the glacial wave.{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ALL);
		return FALSE;
	}

	spellassist_room_freeze(to_room,ch,level,large?8:5,sn);

	level = level * depth_scale / 100;
	depth = 0;
	for (ex = to_room->exit[door]; ex; ex = to_room->exit[door]) {
		level = level * depth_scale / 100;
		--catalyst;
		if(large) large = number_range(0,9) < catalyst;

		// If at a closed exit, at the depth limit or there is no exit
		if (IS_SET(ex->exit_info, EX_CLOSED) || depth >= max_depth || !to_room->exit[door]->u1.to_room) {
			if(large)
				sprintf(buf, "{CAn enormous glacial wave explodes in a fury of ice!{x\n\r");
			else
				sprintf(buf, "{CA glacial wave explodes in a fury of ice!{x\n\r");
			room_echo(to_room, buf);

			if(large) spellassist_room_freeze(to_room,ch,level,5,sn);
			return TRUE;
		}

		// Get new room
		to_room = to_room->exit[door]->u1.to_room;

		if(large)
			sprintf(buf, "{CAn enormous glacial wave rushes in from the %s!{x\n\r", dir_name[ rev_dir[ door ] ]);
		else
			sprintf(buf, "{CA glacial wave flies in from the %s!{x\n\r", dir_name[ rev_dir[ door ] ]);
		room_echo(to_room, buf);

		// Count the sources of heat
		heat = 0;

		if (IS_SET(to_room->room2_flags, ROOM_FIRE)) heat += 25;
		if (to_room->sector_type == SECT_LAVA) heat += 60;

		for (obj = to_room->contents; obj != NULL; obj = obj->next_content) {
			if (obj->item_type == ITEM_ROOM_FLAME) heat += 15;
		}

		heat = URANGE(0,heat,100);
		heat = heat * (100 - catalyst*catalyst) / 100;	// If they use catalysts, reduce the heat quadratically

		if (large && number_range(0,24) < heat) {
			large = FALSE;	// It takes little to prevent ice storms
			room_echo(to_room,"{RThe intense heat in the area diminishes the glacial wave significantly.{x");

		} else if (number_percent() < heat) {
			room_echo(to_room,"{RThe intense heat in the area absorbs the glacial wave.{x");
			return TRUE;
		}

		spellassist_room_freeze(to_room,ch,level,large?8:5,sn);

		if(large)
			sprintf(buf, "{CAn enormous glacial wave rushes %sward!{x\n\r", dir_name[ door ]);
		else
			sprintf(buf, "{CA glacial wave flies %sward!{x\n\r", dir_name[ door ]);
		room_echo(to_room, buf);

		++depth;
	}
	return TRUE;
}

SPELL_FUNC(spell_ice_storm)
{
	OBJ_DATA *obj;
	CHAR_DATA *vch;
	ROOM_INDEX_DATA *room;


	if (IS_SET(ch->in_room->room2_flags, ROOM_FIRE)) {
		send_to_char("The intense heat in the area melts your ice storm as soon as it appears.\n\r", ch);
		act("$n summons an ice storm, but it melts instantly.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
		return FALSE;
	}

	room = ch->in_room;
	if (room->sector_type == SECT_AIR || room->sector_type == SECT_NETHERWORLD) {
		send_to_char("You can't seem to summon the powers of ice here.\n\r", ch);
		return FALSE;
	}

	obj = create_object(get_obj_index(OBJ_VNUM_ICE_STORM), 0, TRUE);
	act("{BYou summon a huge ice storm!{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
	act("{B$n summons a huge ice storm!{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
	obj_to_room(obj, ch->in_room);
	cold_effect(ch->in_room, level, dice(4,8), TARGET_ROOM);
	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
		if (!is_safe_spell(ch, vch, TRUE))
			cold_effect(vch, level/2, dice(4,8), TARGET_CHAR);
	}

	obj->timer = UMAX(level/30, 1);

	return TRUE;
}

