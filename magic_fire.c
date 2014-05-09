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

SPELL_FUNC(spell_afterburn)
{
	CHAR_DATA *victim;
	CHAR_DATA *vnext;
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *exit;
	char *arg = (char *) vo;
	int depth;
	int max_depth;
	int door;
	int dam;
	char buf[MAX_STRING_LENGTH];

	if (!arg) return FALSE;

	if (!arg[0]) {
		send_to_char("Cast it in which direction?\n\r", ch);
		return FALSE;
	}

	if ((door = parse_direction(arg)) < 0) {
		send_to_char("That's not a direction.\n\r", ch);
		return FALSE;
	}

	act("{RA scorching fireball erupts from your hands and flies $tward!{x", ch, dir_name[door], NULL, TO_CHAR);
	act("{RA scorching fireball erupts from $n's hands and flies $tward!{x", ch, dir_name[door], NULL, TO_ROOM);

	max_depth = 5;

	dam = dice(level, 5);
	fire_effect((void *) ch->in_room, level, dam, TARGET_ROOM);

	/* hurt people in the room first */
	for (victim = ch->in_room->people; victim ; victim = vnext) {
		vnext = victim->next_in_room;
		if (!is_safe(ch, victim, FALSE) && victim != ch) {
			if (!check_spell_deflection(ch, victim, sn))
				continue;

			damage(ch, victim, dam, sn, DAM_FIRE, TRUE);
			fire_effect((void *)victim, level, dam, TARGET_CHAR);
		}
	}

	to_room = ch->in_room;
	depth = 0;
	for (exit = ch->in_room->exit[door]; exit; exit = to_room->exit[door]) {
		dam = dice(level, 5);
		if (IS_SET(exit->exit_info, EX_CLOSED) ||
			depth >= max_depth ||
			!to_room->exit[door]) {
			sprintf(buf, "{RA scorching fireball explodes in a roaring inferno!{x\n\r");
			room_echo(to_room, buf);
			break;
		}

		to_room = exit->u1.to_room;
		sprintf(buf, "{RA scorching fireball flies in from the %s!{x\n\r", dir_name[ rev_dir[ door ] ]);
		room_echo(to_room, buf);

		fire_effect((void *) to_room, level, dam, TARGET_ROOM);
		for (victim = to_room->people; victim ; victim = vnext) {
			vnext = victim->next_in_room;
			if (!is_safe(ch, victim, FALSE) && victim != ch) {
				if (!check_spell_deflection(ch, victim, sn))
					continue;

				damage(ch, victim, dam,sn, DAM_FIRE, TRUE);
				fire_effect((void *)victim, level, dam, TARGET_CHAR);
			}
		}

		level = UMAX(level - 5, 1); // strength fades as it goes more rooms
		depth++;

		sprintf(buf, "{RA scorching fireball flies %sward!{x\n\r",dir_name[ door ]);
		room_echo(to_room, buf);
	}
	return TRUE;
}


SPELL_FUNC(spell_burning_hands)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(level, 6) + dice(2, level / 10);

	if (saves_spell(level, victim,DAM_FIRE)) dam /= 2;

	dam = UMIN(dam, 2500);

	damage(ch, victim, dam, sn, DAM_FIRE,TRUE);
	return TRUE;
}

SPELL_FUNC(spell_fire_barrier)
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
			send_to_char("You are already surrounded by a fire barrier.\n\r",ch);
		else
			act("$N is already surrounded by a fire barrier.", ch, NULL, victim, TO_CHAR);
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
	af.bitvector2 = AFF2_FIRE_BARRIER;
	affect_to_char(victim, &af);
	act("{RA barrier fire blooms around $n, defending $m from harm.{x", victim, NULL, NULL, TO_ROOM);
	send_to_char("{RA barrier of fire blooms around you.{x\n\r", victim);
	return TRUE;
}


SPELL_FUNC(spell_fire_breath)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	CHAR_DATA *vch, *vch_next;
	int dam;

	act("$n breathes forth a cone of fire.",ch,NULL,victim,TO_NOTVICT);
	act("$n breathes a cone of hot fire over you!",ch,NULL,victim,TO_VICT);
	act("You breathe forth a cone of fire.",ch,NULL,NULL,TO_CHAR);

	if (check_shield_block_projectile(ch, victim, "cone of fire", NULL))
		return FALSE;

	dam = level * 12;

	if (IS_DRAGON(ch))
		dam += dam/4;

	fire_effect(victim->in_room,level,dam/6,TARGET_ROOM);

	for (vch = victim->in_room->people; vch; vch = vch_next) {
		vch_next = vch->next_in_room;

		if (is_safe_spell(ch, vch, TRUE) || is_same_group(ch, vch) || (IS_NPC(vch) && IS_NPC(ch) &&
			(ch->fighting != vch || vch->fighting != ch)))
			continue;

		vch->set_death_type = DEATHTYPE_BREATH;

		if (vch == victim) {
			if (saves_spell(level,vch,DAM_FIRE)) {
				fire_effect(vch,level/2,dam/4,TARGET_CHAR);
				damage(ch,vch,dam/2,sn,DAM_FIRE,TRUE);
			} else	{
				fire_effect(vch,level,dam,TARGET_CHAR);
				damage(ch,vch,dam,sn,DAM_FIRE,TRUE);
			}
		} else {
			if (saves_spell(level - 2,vch,DAM_FIRE)) {
				fire_effect(vch,level/4,dam/8,TARGET_CHAR);
				damage(ch,vch,dam/4,sn,DAM_FIRE,TRUE);
			} else {
				fire_effect(vch,level/2,dam/4,TARGET_CHAR);
				damage(ch,vch,dam/2,sn,DAM_FIRE,TRUE);
			}
		}
	}
	return TRUE;
}


SPELL_FUNC(spell_fire_cloud)
{
	OBJ_INDEX_DATA *inferno;
	OBJ_DATA *fire_cloud;
	OBJ_DATA *obj;
	ROOM_INDEX_DATA *room;
	int dir = 0;
	bool exists = FALSE;

	if (!(inferno = get_obj_index(OBJ_VNUM_INFERNO))) {
		bug("spell_fire_cloud: null obj_index!\n", 0);
		return FALSE;
	}

	act("{RA cloud of burning fire erupts from $n's mouth setting light to everything!{x", ch, NULL, NULL, TO_ROOM);
	act("{RYou breathe forth a massive cloud of fire!{x", ch, NULL, NULL, TO_CHAR);

	for (obj = ch->in_room->contents; obj; obj = obj->next_content) {
		if (obj->item_type == ITEM_ROOM_FLAME) {
			exists = TRUE;
			break;
		}
	}

	if (!exists) {
		fire_cloud = create_object(inferno, 0, TRUE);
		fire_cloud->timer = 4;
		obj_to_room(fire_cloud, ch->in_room);
	}

	echo_around(ch->in_room, "{RA huge fireball flies into the room setting light to everything!{x\n\r");

	exists = FALSE;
	for (dir = 0; dir < MAX_DIR; dir++) {
		if (!ch->in_room->exit[dir]) continue;

		if (IS_SET(ch->in_room->exit[dir]->exit_info, EX_CLOSED)) continue;

		room = ch->in_room->exit[dir]->u1.to_room;

		for (obj = room->contents; obj != NULL; obj = obj->next_content) {
			if (obj->item_type == ITEM_ROOM_FLAME) {
				exists = TRUE;
				break;
			}
		}

		if (!exists) {
			fire_cloud = create_object(inferno, 0, TRUE);
			fire_cloud->timer = 4;
			obj_to_room(fire_cloud, room);
		}
	}

	return TRUE;
}


SPELL_FUNC(spell_fireball)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	act("{RA large fireball shoots from the palms of your hands.{x", ch, NULL, NULL, TO_CHAR);
	act("{RA large fireball flies from $n's hands toward $N!{x", ch, NULL, victim, TO_NOTVICT);
	act("{RA large fireball flies from $n's hands towards you!{x", ch, NULL, victim, TO_VICT);

	if (check_shield_block_projectile(ch, victim, "fireball", NULL))
		return FALSE;

	level = UMAX(0, level);
	dam = dice(level, level/7);

	if (saves_spell(level, victim, DAM_FIRE))
		dam /= 2;

	/* CAP */
	dam = UMIN(dam, 2500);

	damage(ch,victim,dam,skill_lookup("fireball"), DAM_FIRE, TRUE);
	// fire_effect(victim, level, dam, target);
	return TRUE;
}


SPELL_FUNC(spell_fireproof)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	AFFECT_DATA af;
	memset(&af,0,sizeof(af));

	if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF)) {
		act("$p is already protected from burning.",ch,obj,NULL,TO_CHAR);
		return FALSE;
	}

	if (obj->item_type == ITEM_SCROLL || obj->item_type == ITEM_POTION) {
		act("$p is already invested with enough magic. You can't make it fireproof.", ch, obj, NULL, TO_CHAR);
		return FALSE;
	}

	af.where = TO_OBJECT;
	af.group = AFFGROUP_ENCHANT;
	af.type = sn;
	af.level = level;
	af.duration = number_fuzzy(level / 4);
	af.location = APPLY_NONE;
	af.modifier = 0;
	af.bitvector = ITEM_BURN_PROOF;
	af.bitvector2 = 0;

	affect_to_obj(obj,&af);

	act("You protect $p from fire.",ch,obj,NULL,TO_CHAR);
	act("$p is surrounded by a protective aura.",ch,obj,NULL,TO_ROOM);
	return TRUE;
}

SPELL_FUNC(spell_flamestrike)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	act("{RA large stream of fire shoots from your fingertips toward $N.{x",ch, NULL, victim, TO_CHAR);
	act("{RA large stream of fire shoots from $n's fingertips toward $N.{x",ch, NULL, victim, TO_NOTVICT);
	act("{RA large stream of fire shoots from $n's fingertips toward you.{x",ch, NULL, victim, TO_VICT);

	if (check_shield_block_projectile(ch, victim, "flamestrike", NULL))
		return FALSE;

	dam = dice(level/3 + 2, level/3 - 2);

	dam = (dam * get_skill(ch, sn))/100;

	damage(ch, victim, dam, sn, DAM_FIRE ,TRUE);
	return TRUE;
}


SPELL_FUNC(spell_inferno)
{
	OBJ_DATA *inferno;
	OBJ_DATA *obj;

	for (obj = ch->in_room->contents; obj != NULL; obj = obj->next_content)
		if (obj->item_type == ITEM_ROOM_FLAME) {
			send_to_char("The room is already a raging inferno.\n\r", ch);
			return FALSE;
		}

	inferno = create_object(get_obj_index(OBJ_VNUM_INFERNO), 0, TRUE);
	inferno->timer = 4;
	obj_to_room(inferno, ch->in_room);
	act("With a whisper, the room is ablaze with the burning fires of Hell!",   ch, NULL, NULL, TO_ROOM);
	act("With a whisper, the room is ablaze with the burning fires of Hell!",   ch, NULL, NULL, TO_CHAR);
	return TRUE;
}

SPELL_FUNC(spell_hell_forge)
{
#if 0

#endif
	return TRUE;
}

SPELL_FUNC(spell_magma_flow)
{
	return TRUE;
}
