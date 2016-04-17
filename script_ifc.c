#include "merc.h"
#include "tables.h"
#include "scripts.h"
#include "wilds.h"
#include <math.h>

extern bool wiznet_script;

//
// All strings will be stored in buffers by the calling routine.
// An ifcheck will not be called unless all of the parameter requirements are met.
// If not, the value will default to the return type default of FALSE or ZERO.
//

#define ARG_TYPE(x,f)	(argv[(x)].d.f)
#define ISARG_TYPE(x,t,f)	((argv[(x)].type == (t)) && ARG_TYPE(x,f))

#define ISARG_NUM(x)	(argv[(x)].type == ENT_NUMBER)
#define ISARG_STR(x)	ISARG_TYPE(x,ENT_STRING,str)
#define ISARG_MOB(x)	ISARG_TYPE(x,ENT_MOBILE,mob)
#define ISARG_OBJ(x)	ISARG_TYPE(x,ENT_OBJECT,obj)
#define ISARG_ROOM(x)	ISARG_TYPE(x,ENT_ROOM,room)
#define ISARG_EXIT(x)	(argv[(x)].type == ENT_EXIT)
#define ISARG_TOK(x)	ISARG_TYPE(x,ENT_TOKEN,token)
#define ISARG_AREA(x)	ISARG_TYPE(x,ENT_AREA,area)
#define ISARG_SKILL(x)	(argv[(x)].type == ENT_SKILL)
#define ISARG_SKINFO(x)	(argv[(x)].type == ENT_SKILLINFO)
#define ISARG_CONN(x)	ISARG_TYPE(x,ENT_CONN,conn)
#define ISARG_WILDS(x)	ISARG_TYPE(x,ENT_WILDS,wilds)
#define ISARG_CHURCH(x)	ISARG_TYPE(x,ENT_CHURCH,church)
#define ISARG_AFF(x)	ISARG_TYPE(x,ENT_AFFECT,aff)
#define ISARG_MUID(x)	(argv[(x)].type == ENT_MOBILE_ID)
#define ISARG_OUID(x)	(argv[(x)].type == ENT_OBJECT_ID)
#define ISARG_TUID(x)	(argv[(x)].type == ENT_TOKEN_ID)
#define ISARG_SKID(x)	(argv[(x)].type == ENT_SKILLINFO_ID)
#define ISARG_AID(x)	(argv[(x)].type == ENT_AREA_ID)
#define ISARG_WID(x)	(argv[(x)].type == ENT_WILDS_ID)
#define ISARG_CHID(x)	(argv[(x)].type == ENT_CHURCH_ID)
#define ISARG_BLIST(x,t)	((argv[(x)].type == (t)) && IS_VALID(argv[(x)].d.blist))
#define ISARG_BLLIST_ROOM(x)	ISARG_BLIST(x,ENT_BLLIST_ROOM)
#define ISARG_BLLIST_MOB(x)	ISARG_BLIST(x,ENT_BLLIST_MOB)
#define ISARG_BLLIST_OBJ(x)	ISARG_BLIST(x,ENT_BLLIST_OBJ)
#define ISARG_BLLIST_TOK(x)	ISARG_BLIST(x,ENT_BLLIST_TOK)
#define ISARG_BLLIST_EXIT(x)	ISARG_BLIST(x,ENT_BLLIST_EXIT)
#define ISARG_BLLIST_SKILL(x)	ISARG_BLIST(x,ENT_BLLIST_SKILL)
#define ISARG_BLLIST_AREA(x)		ISARG_BLIST(x,ENT_BLLIST_AREA)
#define ISARG_BLLIST_WILDS(x)	ISARG_BLIST(x,ENT_BLLIST_WILDS)

#define ISARG_PLLIST_STR(x)	ISARG_BLIST(x,ENT_PLLIST_STR)
#define ISARG_PLLIST_CONN(x)	ISARG_BLIST(x,ENT_PLLIST_CONN)
#define ISARG_PLLIST_ROOM(x)	ISARG_BLIST(x,ENT_PLLIST_ROOM)
#define ISARG_PLLIST_MOB(x)	ISARG_BLIST(x,ENT_PLLIST_MOB)
#define ISARG_PLLIST_OBJ(x)	ISARG_BLIST(x,ENT_PLLIST_OBJ)
#define ISARG_PLLIST_TOK(x)	ISARG_BLIST(x,ENT_PLLIST_TOK)
#define ISARG_PLLIST_CHURCH(x)	ISARG_BLIST(x,ENT_PLLIST_CHURCH)


#define ARG_NUM(x)	ARG_TYPE(x,num)
#define ARG_STR(x)	ARG_TYPE(x,str)
#define ARG_MOB(x)	ARG_TYPE(x,mob)
#define ARG_OBJ(x)	ARG_TYPE(x,obj)
#define ARG_ROOM(x)	ARG_TYPE(x,room)
#define ARG_EXIT(x)	ARG_TYPE(x,door)
#define ARG_TOK(x)	ARG_TYPE(x,token)
#define ARG_AREA(x)	ARG_TYPE(x,area)
#define ARG_SKILL(x)	ARG_TYPE(x,sn)
#define ARG_SKINFO(x)	ARG_TYPE(x,sk)
#define ARG_CONN(x)	ARG_TYPE(x,conn)
#define ARG_WILDS(x)	ARG_TYPE(x,wilds)
#define ARG_CHURCH(x)	ARG_TYPE(x,church)
#define ARG_AFF(x)	ARG_TYPE(x,aff)
#define ARG_BLIST(x)	ARG_TYPE(x,blist)
#define ARG_UID(x)	ARG_TYPE(x,uid)
#define ARG_UID2(x,y)	(argv[(x)].d.uid[(y)])
#define ARG_AID(x)	ARG_TYPE(x,aid)
#define ARG_WID(x)	ARG_TYPE(x,wid)
#define ARG_CHID(x)	ARG_TYPE(x,chid)

#define SHIFT_MOB()	do { if(ISARG_MOB(0)) { mob = ARG_MOB(0); ++argv; --argc; } } while(0)
#define SHIFT_OBJ()	do { if(ISARG_OBJ(0)) { obj = ARG_OBJ(0); ++argv; --argc; } } while(0)
#define SHIFT_ROOM()	do { if(ISARG_ROOM(0)) { room = ARG_ROOM(0); ++argv; --argc; } } while(0)
#define SHIFT_TOK()	do { if(ISARG_TOK(0)) { token = ARG_TOK(0); ++argv; --argc; } } while(0)

#define VALID_STR(x) (ISARG_STR(x) && ARG_STR(x))
#define VALID_NPC(x) (ISARG_MOB(x) && IS_NPC(ARG_MOB(x)))
#define VALID_PLAYER(x) (ISARG_MOB(x) && !IS_NPC(ARG_MOB(x)))

#define ARG_BOOL(x) ( (ISARG_NUM(x) && ARG_NUM(x) != 0) || (ISARG_STR(x) && !str_cmp(ARG_STR(x), "true")) )

// Checks if the "act" field on the mob has the given flags
DECL_IFC_FUN(ifc_act)
{
	*ret = (ISARG_MOB(0) && ISARG_STR(1) &&
		((IS_NPC(ARG_MOB(0)) && IS_SET(ARG_MOB(0)->act, flag_value(act_flags,ARG_STR(1)))) ||
		(!IS_NPC(ARG_MOB(0)) && IS_SET(ARG_MOB(0)->act, flag_value(plr_flags,ARG_STR(1))))));
	return TRUE;
}

// Checks if the "act2" field on the mob has the given flags
DECL_IFC_FUN(ifc_act2)
{
	*ret = (ISARG_MOB(0) && ISARG_STR(1) &&
		((IS_NPC(ARG_MOB(0)) && IS_SET(ARG_MOB(0)->act2, flag_value(act2_flags,ARG_STR(1)))) ||
		(!IS_NPC(ARG_MOB(0)) && IS_SET(ARG_MOB(0)->act2, flag_value(plr2_flags,ARG_STR(1))))));
	return TRUE;
}

// Checks if the "affected_by" field on the mob has the given flags
DECL_IFC_FUN(ifc_affected)
{
	*ret = (ISARG_MOB(0) && ISARG_STR(1) && IS_SET(ARG_MOB(0)->affected_by, flag_value( affect_flags,ARG_STR(1))));
	return TRUE;
}

// Checks if the "affected_by2" field on the mob has the given flags
DECL_IFC_FUN(ifc_affected2)
{
	*ret = (ISARG_MOB(0) && ISARG_STR(1) && IS_SET(ARG_MOB(0)->affected_by2, flag_value( affect2_flags,ARG_STR(1))));
	return TRUE;
}

// Checks if the mobile has an affect with the custom name
DECL_IFC_FUN(ifc_affectedname)
{
	*ret = (ISARG_MOB(0) && ISARG_STR(1) && is_affected_name(ARG_MOB(0), get_affect_cname(ARG_STR(1))));
	return TRUE;
}

// Checks if the mobile is affected by the spell
DECL_IFC_FUN(ifc_affectedspell)
{
	*ret = (ISARG_MOB(0) && ISARG_STR(1) && is_affected(ARG_MOB(0), skill_lookup(ARG_STR(1))));
	return TRUE;
}


// Gets the player's age
DECL_IFC_FUN(ifc_age)
{
	*ret = VALID_PLAYER(0) ? GET_AGE(ARG_MOB(0)) : 0;
	return TRUE;
}

// Gets the mobile's alignment
DECL_IFC_FUN(ifc_align)
{
	*ret = ISARG_MOB(0) ? ARG_MOB(0)->alignment : 0;
	return TRUE;
}

// Gets the player's arena fight count: wins + loss
DECL_IFC_FUN(ifc_arenafights)
{
	*ret = VALID_PLAYER(0) ? (ARG_MOB(0)->arena_deaths+ARG_MOB(0)->arena_kills) : 0;
	return TRUE;
}

// Gets the player's arena losses
DECL_IFC_FUN(ifc_arenaloss)
{
	*ret = VALID_PLAYER(0) ? ARG_MOB(0)->arena_deaths : 0;
	return TRUE;
}

// Gets the player's arena win ratio: 0 = 0.0% to 1000 = 100.0%
DECL_IFC_FUN(ifc_arenaratio)
{
	int val;

	if (VALID_PLAYER(0)) {
		val = (ARG_MOB(0)->arena_deaths+ARG_MOB(0)->arena_kills);
		if(val > 0) val = 1000 * ARG_MOB(0)->arena_kills / val;
		*ret = val;
		return TRUE;
	}

	return FALSE;
}

// Gets the player's arena wins
DECL_IFC_FUN(ifc_arenawins)
{
	*ret = VALID_PLAYER(0) ? ARG_MOB(0)->arena_kills : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_canhunt)
{
	if(argc == 2)
		*ret = (ISARG_MOB(0) && ISARG_MOB(1) && can_hunt(ARG_MOB(0),ARG_MOB(1)));
	else
		*ret = (mob && ISARG_MOB(0) && can_hunt(mob,ARG_MOB(0)));
	return TRUE;
}

DECL_IFC_FUN(ifc_canpractice)
{
	int sn;

	*ret = (VALID_PLAYER(0) && ARG_STR(1) &&
		((sn = skill_lookup(ARG_STR(1))) > 0) &&
		can_practice(ARG_MOB(0),sn));
	return TRUE;
}

DECL_IFC_FUN(ifc_canscare)
{
	*ret = (ISARG_MOB(0) && can_scare(ARG_MOB(0)));
	return TRUE;
}

// Checks to see if the SELF object is worn by the target mobile
DECL_IFC_FUN(ifc_carriedby)
{
	*ret = (obj && ISARG_MOB(0) && obj->carried_by == ARG_MOB(0) &&
		obj->wear_loc == WEAR_NONE);
	return TRUE;
}

DECL_IFC_FUN(ifc_carries)
{
	if(ISARG_MOB(0)) {
		if (ISARG_NUM(1))
			*ret = (int)has_item(ARG_MOB(0), ARG_NUM(1), -1, FALSE);
		else if(ISARG_STR(1)) {
			if (is_number(ARG_STR(1)))
				*ret = (int)has_item(ARG_MOB(0), atol(ARG_STR(1)), -1, FALSE);
			else
				*ret = (int)(get_obj_carry(ARG_MOB(0), ARG_STR(1), ARG_MOB(0)) && 1);
		} else if(ISARG_OBJ(1))
			*ret = (ARG_OBJ(1)->carried_by == ARG_MOB(0)) ||
				(ARG_OBJ(1)->in_obj && ARG_OBJ(1)->in_obj->carried_by == ARG_MOB(0));
		else
			return FALSE;
		return TRUE;
	}
	return FALSE;
}

DECL_IFC_FUN(ifc_carryleft)
{
	if (ISARG_OBJ(0) && (obj = ARG_OBJ(0))) {
		if(obj->item_type == ITEM_CART || obj->item_type == ITEM_CONTAINER)
			*ret = obj->value[3] - get_obj_number_container(obj);
		else if(obj->item_type == ITEM_WEAPON_CONTAINER)
			*ret = obj->value[2] - get_obj_number_container(obj);
		else
			*ret = 0;
	} else if(ISARG_MOB(0) && (mob = ARG_MOB(0)))
		*ret = can_carry_n(mob) - mob->carry_number;
	else
		return FALSE;

	return TRUE;
}

DECL_IFC_FUN(ifc_church)
{
	*ret = FALSE;
	if(VALID_PLAYER(0) && ARG_MOB(0)->church) {
		if(ISARG_STR(1) && !str_cmp(ARG_STR(1), ARG_MOB(0)->church->name))
			*ret = TRUE;
		else if(ISARG_CHURCH(1) && ARG_MOB(0)->church == ARG_CHURCH(1))
			*ret = TRUE;
	}
	return TRUE;
}

DECL_IFC_FUN(ifc_churchonline)
{
	if(VALID_PLAYER(0))
		*ret = get_church_online_count(ARG_MOB(0));
	else if(ISARG_CHURCH(0))
		*ret = list_size(ARG_CHURCH(0)->online_players);
	else
		*ret = 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_churchrank)
{
	if(ISARG_MOB(0)) {
		mob = ARG_MOB(0);
		*ret = (!IS_NPC(mob) && mob->church && mob->church_member) ?
			(mob->church_member->rank+1) : 0;
		return TRUE;
	}

	return FALSE;
}

DECL_IFC_FUN(ifc_clan)
{
	return FALSE;
}

DECL_IFC_FUN(ifc_class)
{
	return FALSE;
}

DECL_IFC_FUN(ifc_clones)
{
	*ret = mob ? count_people_room(mob, NULL, NULL, NULL, 3) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_container)
{
	*ret = ISARG_OBJ(0) && ARG_OBJ(0)->item_type == ITEM_CONTAINER &&
		IS_SET(ARG_OBJ(0)->value[1], flag_value(container_flags,ARG_STR(1)));
	return TRUE;
}

// Gets the player's cpk fight count: wins + loss
DECL_IFC_FUN(ifc_cpkfights)
{
	*ret = VALID_PLAYER(0) ? (ARG_MOB(0)->cpk_deaths+ARG_MOB(0)->cpk_kills) : 0;
	return TRUE;
}

// Gets the player's cpk losses
DECL_IFC_FUN(ifc_cpkloss)
{
	*ret = VALID_PLAYER(0) ? ARG_MOB(0)->cpk_deaths : 0;
	return TRUE;
}

// Gets the player's cpk win ratio: 0 = 0.0% to 1000 = 100.0%
DECL_IFC_FUN(ifc_cpkratio)
{
	int val;

	if (VALID_PLAYER(0)) {
		val = (ARG_MOB(0)->cpk_deaths+ARG_MOB(0)->cpk_kills);
		if(val > 0) val = 1000 * ARG_MOB(0)->cpk_kills / val;
		*ret = val;
		return TRUE;
	}

	return FALSE;
}

// Gets the player's cpk wins
DECL_IFC_FUN(ifc_cpkwins)
{
	*ret = VALID_PLAYER(0) ? ARG_MOB(0)->cpk_kills : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_curhit)
{
	*ret = ISARG_MOB(0) ? ARG_MOB(0)->hit : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_curmana)
{
	*ret = ISARG_MOB(0) ? ARG_MOB(0)->mana : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_curmove)
{
	*ret = ISARG_MOB(0) ? ARG_MOB(0)->move : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_danger)
{
	*ret = VALID_PLAYER(0) ? ARG_MOB(0)->pcdata->danger_range : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_damtype)
{
	if(ARG_MOB(0)) *ret = (ARG_MOB(0)->dam_type == attack_lookup(ARG_STR(1)));
	else if(ARG_OBJ(0)) *ret = (ARG_OBJ(0)->item_type == ITEM_WEAPON &&
			ARG_OBJ(0)->value[3] == attack_lookup(ARG_STR(1)));
	else *ret = FALSE;
	return TRUE;
}

// Returns the current game day
DECL_IFC_FUN(ifc_day)
{
	*ret = time_info.day;
	return TRUE;
}

DECL_IFC_FUN(ifc_deathcount)
{
	*ret = VALID_PLAYER(0) ? ARG_MOB(0)->deaths : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_deity)
{
	*ret = ISARG_MOB(0) ? ARG_MOB(0)->deitypoints : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_dice)
{
	if(ISARG_OBJ(0)) {
		if(ARG_OBJ(0)->item_type == ITEM_WEAPON)
			*ret = dice(ARG_OBJ(0)->value[1], ARG_OBJ(0)->value[2]);
		else
			*ret = 0;
		if(ISARG_NUM(1)) *ret += ARG_NUM(1);
	} else if(ISARG_NUM(0) && ISARG_NUM(1)) {
		*ret = dice(ARG_NUM(0), ARG_NUM(1));
		if(ISARG_NUM(2))
			*ret += ARG_NUM(2);
	} else
		return FALSE;

	return TRUE;
}

DECL_IFC_FUN(ifc_drunk)
{
	*ret = VALID_PLAYER(0) ? ARG_MOB(0)->pcdata->condition[COND_DRUNK] : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_exists)
{
	return FALSE;
}

DECL_IFC_FUN(ifc_exitexists)
{
	int door;
	if(ISARG_STR(0)) {
		if(mob) room = mob->in_room;
		else if(obj) room = obj_room(obj);
		else if(token) room = token_room(token);

		door = get_num_dir(ARG_STR(0));
	} else if(ISARG_STR(1)) {
		if(ISARG_MOB(0)) room = ARG_MOB(0)->in_room;
		else if(ISARG_OBJ(0)) room = obj_room(ARG_OBJ(0));
		else if(ISARG_ROOM(0)) room = ARG_ROOM(0);
		else if(ISARG_TOK(0)) room = token_room(ARG_TOK(0));
		else return FALSE;

		door = get_num_dir(ARG_STR(1));
	} else return FALSE;

	*ret = (room && door >= 0 && room->exit[door] && room->exit[door]->u1.to_room);
	return TRUE;
}

DECL_IFC_FUN(ifc_exitflag)
{
	int door;
	EXIT_DATA *ex;

	if(ISARG_EXIT(0)) {
		if(!ISARG_STR(1)) return FALSE;

		if( !ARG_EXIT(0).r ) return FALSE;

		if( !(ex = ARG_EXIT(0).r->exit[ARG_EXIT(0).door]) ) return FALSE;

		*ret = IS_SET(ex->exit_info, flag_value(exit_flags,ARG_STR(1)));
	} else {


		if(!ISARG_STR(1) || !ISARG_STR(2)) return FALSE;

		if(ISARG_MOB(0)) room = ARG_MOB(0)->in_room;
		else if(ISARG_OBJ(0)) room = obj_room(ARG_OBJ(0));
		else if(ISARG_ROOM(0)) room = ARG_ROOM(0);
		else if(ISARG_TOK(0)) room = token_room(ARG_TOK(0));
		else return FALSE;

		door = get_num_dir(ARG_STR(1));

		*ret = (room && door >= 0 && room->exit[door] && IS_SET(room->exit[door]->exit_info, flag_value(exit_flags,ARG_STR(2))));
	}
	return TRUE;
}

DECL_IFC_FUN(ifc_fullness)
{
	*ret = VALID_PLAYER(0) ? ARG_MOB(0)->pcdata->condition[COND_FULL] : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_furniture)
{
	*ret = ISARG_OBJ(0) && ARG_OBJ(0)->item_type == ITEM_FURNITURE &&
		IS_SET(ARG_OBJ(0)->value[2], flag_value(furniture_flags,ARG_STR(1)));
	return TRUE;
}

DECL_IFC_FUN(ifc_gold)
{
	*ret = ISARG_MOB(0) ? ARG_MOB(0)->gold : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_groundweight)
{
	if(ISARG_MOB(0)) room = ARG_MOB(0)->in_room;
	else if(ISARG_OBJ(0)) room = obj_room(ARG_OBJ(0));
	else if(ISARG_ROOM(0)) room = ARG_ROOM(0);
	else if(ISARG_TOK(0)) room = token_room(ARG_TOK(0));
	else return FALSE;

	*ret = get_room_weight(room,TRUE,TRUE,TRUE);
	return TRUE;
}

DECL_IFC_FUN(ifc_groupcon)
{
	*ret = ISARG_MOB(0) ? get_curr_group_stat(ARG_MOB(0),STAT_CON) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_groupdex)
{
	*ret = ISARG_MOB(0) ? get_curr_group_stat(ARG_MOB(0),STAT_DEX) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_groupint)
{
	*ret = ISARG_MOB(0) ? get_curr_group_stat(ARG_MOB(0),STAT_INT) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_groupstr)
{
	*ret = ISARG_MOB(0) ? get_curr_group_stat(ARG_MOB(0),STAT_STR) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_groupwis)
{
	*ret = ISARG_MOB(0) ? get_curr_group_stat(ARG_MOB(0),STAT_WIS) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_grpsize)
{
	*ret = ISARG_MOB(0) ? count_people_room(ARG_MOB(0), NULL, NULL, NULL, 4) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_has)
{
	*ret = (ISARG_MOB(0) && ISARG_STR(1) && has_item(ARG_MOB(0), -1, item_lookup(ARG_STR(1)), FALSE));
	return TRUE;
}

DECL_IFC_FUN(ifc_hasqueue)
{
	if(ISARG_MOB(0)) *ret = (bool)(int)(ARG_MOB(0)->events && 1);
	else if(ISARG_OBJ(0)) *ret = (bool)(int)(ARG_OBJ(0)->events && 1);
	else if(ISARG_ROOM(0)) *ret = (bool)(int)(ARG_ROOM(0)->events && 1);
	else if(ISARG_TOK(0)) *ret = (bool)(int)(ARG_TOK(0)->events && 1);
	else return FALSE;
	return TRUE;
}

DECL_IFC_FUN(ifc_hasship)
{
	return FALSE;
}

DECL_IFC_FUN(ifc_hastarget)
{
	*ret = (ISARG_MOB(0) && ARG_MOB(0)->progs->target &&
		ARG_MOB(0)->in_room == ARG_MOB(0)->progs->target->in_room);
	return TRUE;
}

DECL_IFC_FUN(ifc_hastoken)
{
	if(ISARG_MOB(0)) *ret = TRUE && get_token_char(ARG_MOB(0), ARG_NUM(1), (ISARG_NUM(2) ? ARG_NUM(2) : 1));
	else if(ISARG_OBJ(0)) *ret = TRUE && get_token_obj(ARG_OBJ(0), ARG_NUM(1), (ISARG_NUM(2) ? ARG_NUM(2) : 1));
	else if(ISARG_ROOM(0)) *ret = TRUE && get_token_room(ARG_ROOM(0), ARG_NUM(1), (ISARG_NUM(2) ? ARG_NUM(2) : 1));
	else return FALSE;

	return TRUE;
}

DECL_IFC_FUN(ifc_healregen)
{
	if(ISARG_MOB(0)) room = ARG_MOB(0)->in_room;
	else if(ISARG_OBJ(0)) room = obj_room(ARG_OBJ(0));
	else if(ISARG_ROOM(0)) room = ARG_ROOM(0);
	else if(ISARG_TOK(0)) room = token_room(ARG_TOK(0));
	else return FALSE;

	*ret = room ? room->heal_rate : 0;
	return TRUE;
}

// Returns the current game hour
DECL_IFC_FUN(ifc_hour)
{
	*ret = time_info.hour;
	return TRUE;
}

DECL_IFC_FUN(ifc_hpcnt)
{
	*ret = ISARG_MOB(0) ? (ARG_MOB(0)->hit * 100 / (UMAX(1,ARG_MOB(0)->max_hit))) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_hunger)
{
	*ret = VALID_PLAYER(0) ? ARG_MOB(0)->pcdata->condition[COND_HUNGER] : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_id)
{
	if(ISARG_MOB(0)) *ret = (int)ARG_MOB(0)->id[0];
	else if(ISARG_OBJ(0)) *ret = (int)ARG_OBJ(0)->id[0];
	else if(ISARG_ROOM(0)) *ret = (int)ARG_ROOM(0)->id[0];
	else if(ISARG_TOK(0)) *ret = (int)ARG_TOK(0)->id[0];
	else if(ISARG_MUID(0)) *ret = (int)ARG_UID2(0,0);
	else if(ISARG_OUID(0)) *ret = (int)ARG_UID2(0,0);
	else if(ISARG_TUID(0)) *ret = (int)ARG_UID2(0,0);
	else if(ISARG_AREA(0)) *ret = ARG_AREA(0)->uid;
	else if(ISARG_AID(0)) *ret = (int)ARG_AID(0);
	else if(ISARG_WILDS(0)) *ret = ARG_WILDS(0)->uid;
	else if(ISARG_WID(0)) *ret = (int)ARG_WID(0);
	else if(ISARG_CHURCH(0)) *ret = ARG_CHURCH(0)->uid;
	else if(ISARG_CHID(0)) *ret = (int)ARG_CHID(0);
	else *ret = 0;

	return TRUE;
}

DECL_IFC_FUN(ifc_id2)
{
	if(ISARG_MOB(0)) *ret = (int)ARG_MOB(0)->id[1];
	else if(ISARG_OBJ(0)) *ret = (int)ARG_OBJ(0)->id[1];
	else if(ISARG_ROOM(0)) *ret = (int)ARG_ROOM(0)->id[1];
	else if(ISARG_TOK(0)) *ret = (int)ARG_TOK(0)->id[1];
	else if(ISARG_MUID(0)) *ret = (int)ARG_UID2(0,1);
	else if(ISARG_OUID(0)) *ret = (int)ARG_UID2(0,1);
	else if(ISARG_TUID(0)) *ret = (int)ARG_UID2(0,1);
	else *ret = 0;

	return TRUE;
}

DECL_IFC_FUN(ifc_identical)
{
	if(argc != 2) *ret = FALSE;
	else if(ISARG_NUM(0) && ISARG_NUM(1)) *ret = ARG_NUM(0) == ARG_NUM(1);
	else if(ISARG_STR(0) && ISARG_STR(1)) *ret = !str_cmp(ARG_STR(0),ARG_STR(1));	// Full string comparison, not isname!
	else if(ISARG_MOB(0) && ISARG_MOB(1)) *ret = ARG_MOB(0) == ARG_MOB(1);
	else if(ISARG_OBJ(0) && ISARG_OBJ(1)) *ret = ARG_OBJ(0) == ARG_OBJ(1);
	else if(ISARG_ROOM(0) && ISARG_ROOM(1)) *ret = ARG_ROOM(0) == ARG_ROOM(1);
	else if(ISARG_TOK(0) && ISARG_TOK(1)) *ret = ARG_TOK(0) == ARG_TOK(1);
	else if(ISARG_EXIT(0) && ISARG_EXIT(1)) *ret = (ARG_EXIT(0).r == ARG_EXIT(1).r) && (ARG_EXIT(0).door == ARG_EXIT(1).door);
	else if(ISARG_AREA(0) && ISARG_AREA(1)) *ret = ARG_AREA(0) == ARG_AREA(1);
	else *ret = FALSE;

	return TRUE;
}

DECL_IFC_FUN(ifc_imm)
{
	*ret = (ISARG_MOB(0) && IS_SET(ARG_MOB(0)->imm_flags, flag_value(imm_flags,ARG_STR(1))));
	return TRUE;
}

DECL_IFC_FUN(ifc_innature)
{
	*ret = (ISARG_MOB(0) && is_in_nature(ARG_MOB(0)));
	return TRUE;
}

DECL_IFC_FUN(ifc_isactive)
{
	*ret = (ISARG_MOB(0) && ARG_MOB(0)->position > POS_SLEEPING);
	return TRUE;
}

DECL_IFC_FUN(ifc_isambushing)
{
	*ret = (ISARG_MOB(0) && ARG_MOB(0)->ambush);
	return TRUE;
}

DECL_IFC_FUN(ifc_isangel)
{
	*ret = (ISARG_MOB(0) && IS_ANGEL(ARG_MOB(0)));
	return TRUE;
}

DECL_IFC_FUN(ifc_isbrewing)
{
	int sn;

	*ret = ISARG_MOB(0) && ARG_MOB(0)->brew_sn > 0 &&
		(!ISARG_STR(1) || !*ARG_STR(1) ||
			((sn = skill_lookup(ARG_STR(1))) > 0 &&
				ARG_MOB(0)->brew_sn == sn));
	return TRUE;
}

DECL_IFC_FUN(ifc_iscasting)
{
	int sn;

	*ret = ISARG_MOB(0) && ARG_MOB(0)->cast > 0 &&
		((ISARG_NUM(1) && (token = get_token_char(ARG_MOB(0), ARG_NUM(1), (ISARG_NUM(2) ? ARG_NUM(2) : 1))) &&
				token->pIndexData->type == TOKEN_SPELL && ARG_MOB(0)->cast_token == token) ||
			(ISARG_TOK(1) && ARG_TOK(0)->pIndexData->type == TOKEN_SPELL &&
				ARG_TOK(1)->player == ARG_MOB(0) && ARG_MOB(0)->cast_token == ARG_TOK(1)) ||
			!ISARG_STR(1) || !*ARG_STR(1) ||
			((sn = skill_lookup(ARG_STR(1))) > 0 &&
				ARG_MOB(0)->cast_sn == sn));

	return TRUE;
}

DECL_IFC_FUN(ifc_ischarm)
{
	*ret = (ISARG_MOB(0) && IS_AFFECTED(ARG_MOB(0), AFF_CHARM));
	return TRUE;
}

DECL_IFC_FUN(ifc_ischurchexcom)
{
	*ret = (VALID_PLAYER(0) && ARG_MOB(0)->church && ARG_MOB(0)->church_member &&
    		IS_SET(ARG_MOB(0)->church_member->flags,CHURCH_PLAYER_EXCOMMUNICATED));
	return TRUE;
}

DECL_IFC_FUN(ifc_iscpkproof)
{
	*ret = ISARG_OBJ(0) && IS_SET(ARG_OBJ(0)->extra_flags,ITEM_NODROP) && IS_SET(ARG_OBJ(0)->extra_flags,ITEM_NOUNCURSE);
	return TRUE;
}

DECL_IFC_FUN(ifc_isdead)
{
	*ret = (ISARG_MOB(0) && IS_DEAD(ARG_MOB(0)));
	return TRUE;
}

DECL_IFC_FUN(ifc_isdelay)
{
	if(ISARG_MOB(0)) *ret = (ARG_MOB(0)->progs->delay > 0);
	else if(ISARG_OBJ(0)) *ret = (ARG_OBJ(0)->progs->delay > 0);
	else if(ISARG_ROOM(0)) *ret = (ARG_ROOM(0)->progs->delay > 0);
	else if(ISARG_TOK(0)) *ret = (ARG_TOK(0)->progs->delay > 0);
	else return FALSE;

	return TRUE;
}

DECL_IFC_FUN(ifc_isdemon)
{
	*ret = (ISARG_MOB(0) && IS_DEMON(ARG_MOB(0)));
	return TRUE;
}

DECL_IFC_FUN(ifc_isevil)
{
	*ret = (ISARG_MOB(0) && IS_EVIL(ARG_MOB(0)));
	return TRUE;
}

DECL_IFC_FUN(ifc_isfading)
{
	*ret = ISARG_MOB(0) && ARG_MOB(0)->fade_dir != -1 &&
		(!ISARG_STR(1) || !*ARG_STR(1) ||
			ARG_MOB(0)->fade_dir == get_num_dir(ARG_STR(1)));
	return TRUE;
}

// isfighting <victim>[ <opponent>]
//
// "isfighting <victim>" is <mobile> fighting?
// "isfighting <victim> <mobile>" is <victim> fighting <mobile>?
// "isfighting <victim> <string>" is <victim>'s opponent have the name <string>?
// "isfighting <victim> <number>" is <victim>'s opponent have the vnum <number>?
DECL_IFC_FUN(ifc_isfighting)
{
	*ret = FALSE;
	if(ISARG_MOB(0)) {
		if(ISARG_MOB(1))
			*ret = (ARG_MOB(0) && ARG_MOB(0)->fighting == ARG_MOB(1));
		else if(ISARG_STR(1)) {
			if(is_number(ARG_STR(1)))
				*ret = (ARG_MOB(0) && ARG_MOB(0)->fighting && IS_NPC(ARG_MOB(0)->fighting) && ARG_MOB(0)->fighting->pIndexData->vnum == atoi(ARG_STR(1)));
			else
				*ret = (ARG_MOB(0) && ARG_MOB(0)->fighting && is_name(ARG_STR(1),ARG_MOB(0)->fighting->name));
		} else if(ISARG_NUM(1))
			*ret = (ARG_MOB(0) && ARG_MOB(0)->fighting && IS_NPC(ARG_MOB(0)->fighting) && ARG_MOB(0)->fighting->pIndexData->vnum == ARG_NUM(1));
		else if(argc == 1)
			*ret = (ARG_MOB(0) && ARG_MOB(0)->fighting);
	}
	return TRUE;
}

DECL_IFC_FUN(ifc_isflying)
{
	*ret = ISARG_MOB(0) && mobile_is_flying(ARG_MOB(0));

	return TRUE;
}

DECL_IFC_FUN(ifc_isfollow)
{
	*ret = (ISARG_MOB(0) && ARG_MOB(0)->master &&
		ARG_MOB(0)->master->in_room == ARG_MOB(0)->in_room);
	return TRUE;
}

DECL_IFC_FUN(ifc_isgood)
{
	*ret = (ISARG_MOB(0) && IS_GOOD(ARG_MOB(0)));
	return TRUE;
}

DECL_IFC_FUN(ifc_ishunting)
{
	*ret = (ISARG_MOB(0) && ARG_MOB(0)->hunting);
	return TRUE;
}

DECL_IFC_FUN(ifc_isimmort)
{
	*ret = (VALID_PLAYER(0) && IS_IMMORTAL(ARG_MOB(0)));
	return TRUE;
}

DECL_IFC_FUN(ifc_iskey)
{
	int door;
	EXIT_DATA *ex;

	if(ISARG_OBJ(0)) {
		if(ISARG_EXIT(1)) {
			ex = ARG_EXIT(1).r ? ARG_EXIT(1).r->exit[ARG_EXIT(1).door] : NULL;
			*ret = ex && ARG_OBJ(0)->item_type == ITEM_KEY &&
				ex->door.key_vnum == ARG_OBJ(0)->pIndexData->vnum;
		} else if(ISARG_STR(1))
			*ret = ARG_OBJ(0)->item_type == ITEM_KEY && (room = obj_room(ARG_OBJ(0))) &&
				(door = get_num_dir(ARG_STR(1))) != -1 && room->exit[door] &&
				room->exit[door]->door.key_vnum == ARG_OBJ(0)->pIndexData->vnum;
		else *ret = FALSE;

		return TRUE;
	} else if(obj) {
		if(ISARG_EXIT(0)) {
			ex = ARG_EXIT(0).r ? ARG_EXIT(0).r->exit[ARG_EXIT(0).door] : NULL;
			*ret = ex && obj->item_type == ITEM_KEY &&
				ex->door.key_vnum == obj->pIndexData->vnum;
		} else if(ISARG_STR(0))
			*ret = obj->item_type == ITEM_KEY && (room = obj_room(obj)) &&
				(door = get_num_dir(ARG_STR(0))) != -1 && room->exit[door] &&
				room->exit[door]->door.key_vnum == obj->pIndexData->vnum;
		else *ret = FALSE;

		return TRUE;
	}

	return FALSE;
}

DECL_IFC_FUN(ifc_ismorphed)
{
	*ret = (ISARG_MOB(0) && IS_MORPHED(ARG_MOB(0)));
	return TRUE;
}

DECL_IFC_FUN(ifc_ismystic)
{
	*ret = (ISARG_MOB(0) && IS_MYSTIC(ARG_MOB(0)));
	return TRUE;
}

DECL_IFC_FUN(ifc_isneutral)
{
	*ret = (ISARG_MOB(0) && IS_NEUTRAL(ARG_MOB(0)));
	return TRUE;
}

DECL_IFC_FUN(ifc_isnpc)
{
	*ret = VALID_NPC(0);
	return TRUE;
}

DECL_IFC_FUN(ifc_ison)
{
	if(!ISARG_MOB(0)) return FALSE;

	*ret = FALSE;
	if(ISARG_STR(1)) {
		if(is_number(ARG_STR(1)))
			*ret = (ARG_MOB(0)->on && ARG_MOB(0)->on->pIndexData->vnum == atoi(ARG_STR(1)));
		else
			*ret = (ARG_MOB(0)->on && is_name(ARG_STR(1),ARG_MOB(0)->on->pIndexData->name));
	} else if(ISARG_NUM(1))
		*ret = (ARG_MOB(0)->on && ARG_MOB(0)->on->pIndexData->vnum == ARG_NUM(1));
	else if(ISARG_OBJ(1))
		*ret = (ARG_MOB(0)->on == ARG_OBJ(1));
	else if(argc == 1)
		*ret = (obj && ARG_MOB(0)->on == obj);

	return TRUE;
}

DECL_IFC_FUN(ifc_ispc)
{
	*ret = VALID_PLAYER(0);
	return TRUE;
}

DECL_IFC_FUN(ifc_isprey)
{
	if(ISARG_MOB(0)) {
		if(ISARG_MOB(1))
			*ret = (ARG_MOB(0)->hunting == ARG_MOB(1))?TRUE:FALSE;
		else
			*ret = (mob->hunting == ARG_MOB(0))?TRUE:FALSE;

		return TRUE;
	}
	return FALSE;
}

DECL_IFC_FUN(ifc_ispulling)
{
	*ret = FALSE;
	if(ISARG_MOB(0)) {
		if(ISARG_OBJ(1))
			*ret = (ARG_MOB(0) && ARG_MOB(0)->pulled_cart == ARG_OBJ(1));
		else if(ISARG_STR(1)) {
			if(is_number(ARG_STR(1)))
				*ret = (ARG_MOB(0) && ARG_MOB(0)->pulled_cart && ARG_MOB(0)->pulled_cart->pIndexData->vnum == atoi(ARG_STR(1)));
			else
				*ret = (ARG_MOB(0) && ARG_MOB(0)->pulled_cart && is_name(ARG_STR(1),ARG_MOB(0)->pulled_cart->pIndexData->name));
		} else if(ISARG_NUM(1))
			*ret = (ARG_MOB(0) && ARG_MOB(0)->pulled_cart && ARG_MOB(0)->pulled_cart->pIndexData->vnum == ARG_NUM(1));
		else if(argc == 1)
			*ret = (ARG_MOB(0) && ARG_MOB(0)->pulled_cart);
	}
	return TRUE;
}

DECL_IFC_FUN(ifc_ispullingrelic)
{
	*ret = (ISARG_MOB(0) && is_pulling_relic(ARG_MOB(0)) &&
	    	(!ARG_STR(1) || !*ARG_STR(1) ||
	    	ARG_MOB(0)->pulled_cart->pIndexData->vnum == flag_value( relic_types,ARG_STR(1))));
	return TRUE;
}

DECL_IFC_FUN(ifc_isquesting)
{
	*ret = VALID_PLAYER(0) && ON_QUEST(ARG_MOB(0));
	return TRUE;
}

DECL_IFC_FUN(ifc_isrepairable)
{
	*ret = ISARG_OBJ(0) && (ARG_OBJ(0)->times_fixed < ARG_OBJ(0)->times_allowed_fixed);
	return TRUE;
}

DECL_IFC_FUN(ifc_isridden)
{
	*ret = ISARG_MOB(0) && ARG_MOB(0)->rider;
	return TRUE;
}

DECL_IFC_FUN(ifc_isrider)
{
	*ret = FALSE;
	if(ISARG_MOB(0) && ARG_MOB(0)->riding) {
		if(ISARG_MOB(1))
			*ret = (ARG_MOB(0) && ARG_MOB(0)->rider == ARG_MOB(1));
		else if(ISARG_STR(1)) {
			if(is_number(ARG_STR(1)))
				*ret = (ARG_MOB(0) && ARG_MOB(0)->rider && IS_NPC(ARG_MOB(0)->rider) && ARG_MOB(0)->rider->pIndexData->vnum == atoi(ARG_STR(1)));
			else
				*ret = (ARG_MOB(0) && ARG_MOB(0)->rider && is_name(ARG_STR(1),ARG_MOB(0)->rider->name));
		} else if(ISARG_NUM(1))
			*ret = (ARG_MOB(0) && ARG_MOB(0)->rider && IS_NPC(ARG_MOB(0)->rider) && ARG_MOB(0)->rider->pIndexData->vnum == ARG_NUM(1));
	}
	return TRUE;
}

DECL_IFC_FUN(ifc_isriding)
{
	*ret = FALSE;
	if(ISARG_MOB(0) && ARG_MOB(0)->riding) {
		if(ISARG_MOB(1))
			*ret = (ARG_MOB(0) && ARG_MOB(0)->mount == ARG_MOB(1));
		else if(ISARG_STR(1)) {
			if(is_number(ARG_STR(1)))
				*ret = (ARG_MOB(0) && ARG_MOB(0)->mount && IS_NPC(ARG_MOB(0)->mount) && ARG_MOB(0)->mount->pIndexData->vnum == atoi(ARG_STR(1)));
			else
				*ret = (ARG_MOB(0) && ARG_MOB(0)->mount && is_name(ARG_STR(1),ARG_MOB(0)->mount->name));
		} else if(ISARG_NUM(1))
			*ret = (ARG_MOB(0) && ARG_MOB(0)->mount && IS_NPC(ARG_MOB(0)->mount) && ARG_MOB(0)->mount->pIndexData->vnum == ARG_NUM(1));
		else if(argc == 1)
			*ret = TRUE;
	}
	return TRUE;
}

DECL_IFC_FUN(ifc_isroomdark)
{
	if(ISARG_MOB(0)) room = ARG_MOB(0)->in_room;
	else if(ISARG_OBJ(0)) room = obj_room(ARG_OBJ(0));
	else if(ISARG_ROOM(0)) room = ARG_ROOM(0);
	else if(ISARG_TOK(0)) room = token_room(ARG_TOK(0));
	else return FALSE;

	*ret = room ? room_is_dark(room) : FALSE;
	return TRUE;
}

DECL_IFC_FUN(ifc_isscribing)
{
	int sn;

	*ret = ISARG_MOB(0) && ARG_MOB(0)->scribe_sn > 0 &&
		(!ISARG_STR(1) || !*ARG_STR(1) ||
			((sn = skill_lookup(ARG_STR(1))) > 0 &&
			(	ARG_MOB(0)->scribe_sn == sn ||
				ARG_MOB(0)->scribe_sn2 == sn ||
				ARG_MOB(0)->scribe_sn3 == sn )));
	return TRUE;
}

DECL_IFC_FUN(ifc_isshifted)
{
	*ret = (ISARG_MOB(0) && IS_SHIFTED(ARG_MOB(0)));
	return TRUE;
}

DECL_IFC_FUN(ifc_isshooting)
{
	*ret = ISARG_MOB(0) && ARG_MOB(0)->projectile_dir >= 0 &&
		(!ARG_STR(1) || !*ARG_STR(1) ||
			ARG_MOB(0)->projectile_dir == get_num_dir(ARG_STR(1)));
	return TRUE;
}

DECL_IFC_FUN(ifc_isshopkeeper)
{
	*ret = VALID_NPC(0) && ARG_MOB(0)->pIndexData->pShop;
	return TRUE;
}

DECL_IFC_FUN(ifc_issustained)
{
	*ret = (ISARG_MOB(0) && is_sustained(ARG_MOB(0)));
	return TRUE;
}

DECL_IFC_FUN(ifc_istarget)
{
	if(ISARG_MOB(0)) {
		if(mob) *ret = (mob->progs->target == ARG_MOB(0));
		else if(obj) *ret = (obj->progs->target == ARG_MOB(0));
		else if(room) *ret = (room->progs->target == ARG_MOB(0));
		else if(token) *ret = (token->progs->target == ARG_MOB(0));
		else *ret = FALSE;
	} else *ret = FALSE;
	return TRUE;
}

DECL_IFC_FUN(ifc_istattooing)
{
	int sn;

	*ret = ISARG_MOB(0) && ARG_MOB(0)->ink_sn > 0 &&
		(!ISARG_STR(1) || !*ARG_STR(1) ||
			((sn = skill_lookup(ARG_STR(1))) > 0 &&
			(	ARG_MOB(0)->ink_sn == sn ||
				ARG_MOB(0)->ink_sn2 == sn ||
				ARG_MOB(0)->ink_sn3 == sn )));
	return TRUE;
}


DECL_IFC_FUN(ifc_isvisible)
{
	if(mob) {
		if(ISARG_MOB(0)) *ret = can_see(mob,ARG_MOB(0));
		else if(ISARG_OBJ(0)) *ret = can_see_obj(mob,ARG_OBJ(0));
		else if(ISARG_ROOM(0)) *ret = can_see_room(mob,ARG_ROOM(0));
		else *ret = FALSE;
	} else *ret = TRUE;

	return TRUE;
}

DECL_IFC_FUN(ifc_isvisibleto)
{
	if(ARG_MOB(0)) {
		if(ISARG_MOB(1)) *ret = can_see(ARG_MOB(0),ARG_MOB(1));
		else if(ISARG_OBJ(1)) *ret = can_see_obj(ARG_MOB(0),ARG_OBJ(1));
		else if(ISARG_ROOM(1)) *ret = can_see_room(ARG_MOB(0),ARG_ROOM(1));
		else if(mob) *ret = can_see(ARG_MOB(0),mob);
		else if(obj) *ret = can_see_obj(ARG_MOB(0),obj);
		else if(room) *ret = can_see_room(ARG_MOB(0),room);
		else *ret = FALSE;
	} else *ret = TRUE;
	return TRUE;
}

DECL_IFC_FUN(ifc_isworn)
{
	*ret = (ISARG_OBJ(0) && ARG_OBJ(0)->wear_loc != WEAR_NONE);
	return TRUE;
}

DECL_IFC_FUN(ifc_lastreturn)
{
	if(mob) *ret = mob->progs->lastreturn;
	else if(obj) *ret = obj->progs->lastreturn;
	else if(room) *ret = room->progs->lastreturn;
	else if(token) *ret = token->progs->lastreturn;
	else return FALSE;
	if(*ret < 0) *ret = 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_level)
{
	*ret = ISARG_MOB(0) ? ARG_MOB(0)->tot_level : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_liquid)
{
	*ret = ISARG_OBJ(0) &&
		(ARG_OBJ(0)->item_type == ITEM_DRINK_CON || ARG_OBJ(0)->item_type == ITEM_FOUNTAIN) &&
		(ARG_OBJ(0)->value[2] == liq_lookup(ARG_STR(1)));
	return TRUE;
}

DECL_IFC_FUN(ifc_manaregen)
{
	if(ISARG_MOB(0)) room = ARG_MOB(0)->in_room;
	else if(ISARG_OBJ(0)) room = obj_room(ARG_OBJ(0));
	else if(ISARG_ROOM(0)) room = ARG_ROOM(0);
	else if(ISARG_TOK(0)) room = token_room(ARG_TOK(0));
	else return FALSE;

	*ret = room ? room->mana_rate : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_material)
{
	if(ISARG_MOB(0)) *ret = ARG_MOB(0)->material && !str_cmp(ARG_STR(1), ARG_MOB(0)->material);
	else if(ISARG_OBJ(0)) *ret = ARG_OBJ(0)->material && !str_cmp(ARG_STR(1), ARG_OBJ(0)->material);
	else *ret = FALSE;
	return TRUE;
}

DECL_IFC_FUN(ifc_maxcarry)
{
	if (ISARG_OBJ(0)) {
		obj = ARG_OBJ(0);
		if(obj->item_type == ITEM_CART || obj->item_type == ITEM_CONTAINER)
			*ret = obj->value[3];
		else if(obj->item_type == ITEM_WEAPON_CONTAINER)
			*ret = obj->value[2];
		else
			return FALSE;
	} else if(ISARG_MOB(0))
		*ret = can_carry_n(ARG_MOB(0));
	else
		return TRUE;

	*ret = 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_maxhit)
{
	*ret = ISARG_MOB(0) ? ARG_MOB(0)->max_hit : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_maxmana)
{
	*ret = ISARG_MOB(0) ? ARG_MOB(0)->max_mana : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_maxmove)
{
	*ret = ISARG_MOB(0) ? ARG_MOB(0)->max_move : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_maxweight)
{
	if (ISARG_OBJ(0)) {
		obj = ARG_OBJ(0);
		if(obj->item_type == ITEM_CART ||
			obj->item_type == ITEM_CONTAINER ||
			obj->item_type == ITEM_WEAPON_CONTAINER) {
			*ret = obj->value[0];
			return TRUE;
		}
	} else if(ARG_MOB(0)) {
		*ret = can_carry_w(ARG_MOB(0));
		return TRUE;
	}

	return FALSE;
}

DECL_IFC_FUN(ifc_mobexists)
{
	if(ISARG_NUM(0)) {
		MOB_INDEX_DATA *pMobIndex;

		if (!(pMobIndex = get_mob_index(ARG_NUM(0))))
			*ret = FALSE;
		else
			*ret = (bool)(int)(get_char_world_index(NULL, pMobIndex) && 1);
		return TRUE;
	} else if(ISARG_STR(0)) {
		if (is_number(ARG_STR(0))) {
			MOB_INDEX_DATA *pMobIndex;

			if (!(pMobIndex = get_mob_index(atol(ARG_STR(0)))))
				*ret = FALSE;
			else
				*ret = (bool)(int)(get_char_world_index(NULL, pMobIndex) && 1);
		} else
			*ret = ((bool)(int) (get_char_world(NULL, ARG_STR(0)) && 1));
		return TRUE;
	}
	return TRUE;
}

DECL_IFC_FUN(ifc_mobhere)
{
	if(ISARG_NUM(0))
		*ret = ((bool)(int)(get_mob_vnum_room(mob, obj, room, token, ARG_NUM(0)) && 1));
	else if(ISARG_STR(0)) {
		if(is_number(ARG_STR(0)))
			*ret = ((bool)(int)(get_mob_vnum_room(mob, obj, room, token, atol(ARG_STR(0))) && 1));
		else
			*ret = ((bool)(int)(get_char_room(mob, obj ? obj_room(obj) : (token ? token_room(token) : room), ARG_STR(0)) && 1));
	} else if(ISARG_MOB(0)) {
		if(mob) *ret = mob->in_room && ARG_MOB(0)->in_room == mob->in_room;
		else if(obj) *ret = obj_room(obj) && ARG_MOB(0)->in_room == obj_room(obj);
		else if(room) *ret = ARG_MOB(0)->in_room == room;
		else if(token) *ret = token_room(token) && ARG_MOB(0)->in_room == token_room(token);
		else *ret = FALSE;
	}
	return TRUE;
}

DECL_IFC_FUN(ifc_mobs)
{
	*ret = count_people_room(mob, obj, room, token, 2);
	return TRUE;
}

DECL_IFC_FUN(ifc_money)
{
	*ret = ISARG_MOB(0) ? (ARG_MOB(0)->gold*100 + ARG_MOB(0)->silver) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_monkills)
{
	*ret = VALID_PLAYER(0) ? ARG_MOB(0)->monster_kills : 0;
	return TRUE;
}

// Returns the current game day
DECL_IFC_FUN(ifc_month)
{
	*ret = time_info.month;
	return TRUE;
}

DECL_IFC_FUN(ifc_moveregen)
{
	if(ISARG_MOB(0)) room = ARG_MOB(0)->in_room;
	else if(ISARG_OBJ(0)) room = obj_room(ARG_OBJ(0));
	else if(ISARG_ROOM(0)) room = ARG_ROOM(0);
	else if(ISARG_TOK(0)) room = token_room(ARG_TOK(0));
	else return FALSE;

	*ret = room ? room->move_rate : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_name)
{
	if(!ISARG_STR(1)) return FALSE;
	if(ISARG_MOB(0)) *ret = is_name(ARG_STR(1), ARG_MOB(0)->name);
	else if(ISARG_OBJ(0)) *ret = is_name(ARG_STR(1), ARG_OBJ(0)->name);
	else if(ISARG_TOK(0)) *ret = is_name(ARG_STR(1), ARG_TOK(0)->name);
	else if(ISARG_AREA(0)) *ret = is_name(ARG_STR(1), ARG_AREA(0)->name);
	else *ret = FALSE;
	return TRUE;
}

DECL_IFC_FUN(ifc_objcond)
{
	*ret = ISARG_OBJ(0) ? ARG_OBJ(0)->condition : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_objcost)
{
	*ret = ISARG_OBJ(0) ? ARG_OBJ(0)->cost : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_objexists)
{
	if(!ISARG_STR(0)) return FALSE;
	*ret = ((bool)(int) (get_obj_world(NULL, ARG_STR(0)) && 1));
	return TRUE;
}

DECL_IFC_FUN(ifc_objextra)
{
	*ret = (ISARG_OBJ(0) && ISARG_STR(1) && IS_SET(ARG_OBJ(0)->extra_flags, flag_value(extra_flags,ARG_STR(1))));
	return TRUE;
}

DECL_IFC_FUN(ifc_objextra2)
{
	*ret = (ISARG_OBJ(0) && ISARG_STR(1) && IS_SET(ARG_OBJ(0)->extra2_flags, flag_value(extra2_flags,ARG_STR(1))));
	return TRUE;
}

DECL_IFC_FUN(ifc_objextra3)
{
	*ret = (ISARG_OBJ(0) && ISARG_STR(1) && IS_SET(ARG_OBJ(0)->extra3_flags, flag_value(extra3_flags,ARG_STR(1))));
	return TRUE;
}

DECL_IFC_FUN(ifc_objextra4)
{
	*ret = (ISARG_OBJ(0) && ISARG_STR(1) && IS_SET(ARG_OBJ(0)->extra4_flags, flag_value(extra4_flags,ARG_STR(1))));
	return TRUE;
}

DECL_IFC_FUN(ifc_objhere)
{
	if(ISARG_NUM(0))
		*ret = ((bool)(int)(get_obj_vnum_room(mob, obj, room, token, ARG_NUM(0)) && 1));
	else if(ISARG_STR(0)) {
		if(is_number(ARG_STR(0)))
			*ret = ((bool)(int)(get_obj_vnum_room(mob, obj, room, token, atol(ARG_STR(0))) && 1));
		else
			*ret = ((bool)(int)(get_obj_here(mob, obj ? obj_room(obj) : (token ? token_room(token) : room), ARG_STR(0)) && 1));
	} else if(ISARG_OBJ(0)) {
		if(mob) *ret = mob->in_room && obj_room(ARG_OBJ(0)) == mob->in_room;
		else if(obj) *ret = obj_room(obj) && obj_room(ARG_OBJ(0)) == obj_room(obj);
		else if(room) *ret = obj_room(ARG_OBJ(0)) == room;
		else if(token) *ret = token_room(token) && obj_room(ARG_OBJ(0)) == token_room(token);
		else *ret = FALSE;
	}
	return TRUE;
}

DECL_IFC_FUN(ifc_objmaxweight)
{
	int val;
	if (ISARG_OBJ(0)) {
		obj = ARG_OBJ(0);
		if(obj->item_type == ITEM_CART || obj->item_type == ITEM_CONTAINER)
			val = obj->value[4];
		else if(obj->item_type == ITEM_WEAPON_CONTAINER)
			val = obj->value[3];
		else
			return FALSE;
		if(val < 1) val = 100;
		*ret = obj->value[0] * 100 / val;
		return TRUE;
	}
	return TRUE;
}

DECL_IFC_FUN(ifc_objtimer)
{
	*ret = ISARG_OBJ(0) ? ARG_OBJ(0)->timer : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_objtype)
{
	*ret = ISARG_OBJ(0) && ARG_OBJ(0)->item_type == item_lookup(ARG_STR(1));
	return TRUE;
}

DECL_IFC_FUN(ifc_objval0)
{
	*ret = ISARG_OBJ(0) ? ARG_OBJ(0)->value[0] : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_objval1)
{
	*ret = ISARG_OBJ(0) ? ARG_OBJ(0)->value[1] : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_objval2)
{
	*ret = ISARG_OBJ(0) ? ARG_OBJ(0)->value[2] : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_objval3)
{
	*ret = ISARG_OBJ(0) ? ARG_OBJ(0)->value[3] : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_objval4)
{
	*ret = ISARG_OBJ(0) ? ARG_OBJ(0)->value[4] : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_objval5)
{
	*ret = ISARG_OBJ(0) ? ARG_OBJ(0)->value[5] : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_objval6)
{
	*ret = ISARG_OBJ(0) ? ARG_OBJ(0)->value[6] : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_objval7)
{
	*ret = ISARG_OBJ(0) ? ARG_OBJ(0)->value[7] : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_objwear)
{
	*ret = (ISARG_OBJ(0) && ISARG_STR(1) && IS_SET(ARG_OBJ(0)->wear_flags, flag_value(wear_flags,ARG_STR(1))));
	return TRUE;
}

DECL_IFC_FUN(ifc_objwearloc)
{
	*ret = (ISARG_OBJ(0) && ISARG_STR(1) && ARG_OBJ(0)->wear_loc == flag_value(wear_loc_flags,ARG_STR(1)));
	return TRUE;
}

DECL_IFC_FUN(ifc_objweight)
{
	*ret = ISARG_OBJ(0) ? get_obj_weight(ARG_OBJ(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_objweightleft)
{
	int val;
	if (ISARG_OBJ(0)) {
		obj = ARG_OBJ(0);
		if(obj->item_type == ITEM_CART || obj->item_type == ITEM_CONTAINER)
			val = obj->value[4];
		else if(obj->item_type == ITEM_WEAPON_CONTAINER)
			val = obj->value[3];
		else
			return FALSE;
		if(val < 1) val = 100;
		*ret = (obj->value[0] * 100 / val) - get_obj_weight_container(obj);
		return TRUE;
	}
	return FALSE;
}

DECL_IFC_FUN(ifc_off)
{
	*ret = (ISARG_MOB(0) && IS_SET(ARG_MOB(0)->off_flags, flag_value( off_flags,ARG_STR(1))));
	return TRUE;
}

DECL_IFC_FUN(ifc_order)
{
	*ret = (mob || obj) ? get_order(mob, obj) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_parts)
{
	*ret = (ISARG_MOB(0) && ISARG_STR(1) && IS_SET(ARG_MOB(0)->parts, flag_value(part_flags,ARG_STR(1))));
	return TRUE;
}

DECL_IFC_FUN(ifc_people)
{
	*ret = count_people_room(mob, obj, room, token, 0);
	return TRUE;
}

DECL_IFC_FUN(ifc_permcon)
{
	*ret = ISARG_MOB(0) ? ARG_MOB(0)->perm_stat[STAT_CON] : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_permdex)
{
	*ret = ISARG_MOB(0) ? ARG_MOB(0)->perm_stat[STAT_DEX] : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_permint)
{
	*ret = ISARG_MOB(0) ? ARG_MOB(0)->perm_stat[STAT_INT] : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_permstr)
{
	*ret = ISARG_MOB(0) ? ARG_MOB(0)->perm_stat[STAT_STR] : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_permwis)
{
	*ret = ISARG_MOB(0) ? ARG_MOB(0)->perm_stat[STAT_WIS] : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_pgroupcon)
{
	*ret = ISARG_MOB(0) ? get_perm_group_stat(ARG_MOB(0),STAT_CON) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_pgroupdex)
{
	*ret = ISARG_MOB(0) ? get_perm_group_stat(ARG_MOB(0),STAT_DEX) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_pgroupint)
{
	*ret = ISARG_MOB(0) ? get_perm_group_stat(ARG_MOB(0),STAT_INT) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_pgroupstr)
{
	*ret = ISARG_MOB(0) ? get_perm_group_stat(ARG_MOB(0),STAT_STR) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_pgroupwis)
{
	*ret = ISARG_MOB(0) ? get_perm_group_stat(ARG_MOB(0),STAT_WIS) : 0;
	return TRUE;
}

// Gets the player's pk fight count: wins + loss
DECL_IFC_FUN(ifc_pkfights)
{
	*ret = VALID_PLAYER(0) ? (ARG_MOB(0)->player_deaths+ARG_MOB(0)->player_kills) : 0;
	return TRUE;
}

// Gets the player's pk losses
DECL_IFC_FUN(ifc_pkloss)
{
	*ret = VALID_PLAYER(0) ? ARG_MOB(0)->player_deaths : 0;
	return TRUE;
}

// Gets the player's pk win ratio: 0 = 0.0% to 1000 = 100.0%
DECL_IFC_FUN(ifc_pkratio)
{
	int val;

	if (VALID_PLAYER(0)) {
		val = (ARG_MOB(0)->player_deaths+ARG_MOB(0)->player_kills);
		if(val > 0) val = 1000 * ARG_MOB(0)->player_kills / val;
		*ret = val;
		return TRUE;
	}

	return FALSE;
}

// Gets the player's pk wins
DECL_IFC_FUN(ifc_pkwins)
{
	*ret = VALID_PLAYER(0) ? ARG_MOB(0)->player_kills : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_players)
{
	*ret = count_people_room(mob, obj, room, token, 1);
	return TRUE;
}

DECL_IFC_FUN(ifc_pneuma)
{
	*ret = VALID_PLAYER(0) ? ARG_MOB(0)->pneuma : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_portal)
{
	*ret = ISARG_OBJ(0) && ARG_OBJ(0)->item_type == ITEM_PORTAL &&
		IS_SET(ARG_OBJ(0)->value[2], flag_value(portal_flags,ARG_STR(1)));
	return TRUE;
}

DECL_IFC_FUN(ifc_portalexit)
{
	*ret = ISARG_OBJ(0) && ARG_OBJ(0)->item_type == ITEM_PORTAL &&
		IS_SET(ARG_OBJ(0)->value[1], flag_value(exit_flags,ARG_STR(1)));
	return TRUE;
}

DECL_IFC_FUN(ifc_pos)
{
//	char buf[MIL];
//	sprintf(buf,"ifc_pos: %s(%d), %s(%d), %s(%08X)",
//		(mob ? HANDLE(mob) : "(MOB)"),
//		(mob ? VNUM(mob) : -1),
//		(ARG_MOB(0) ? HANDLE(ARG_MOB(0)) : "(MOB)"),
//		(ARG_MOB(0) ? VNUM(ARG_MOB(0)) : -1),
//		(ARG_STR(1) ? ARG_STR(1) : "(null)"),
//		ARG_STR(1));
//	log_string(buf);
	*ret = ISARG_MOB(0) && VALID_STR(1) && ARG_MOB(0)->position == position_lookup(ARG_STR(1));
	return TRUE;
}

DECL_IFC_FUN(ifc_practices)
{
	*ret = VALID_PLAYER(0) ? ARG_MOB(0)->practice : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_quest)
{
	*ret = ISARG_MOB(0) ? ARG_MOB(0)->questpoints : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_race)
{
	*ret = ISARG_MOB(0) && VALID_STR(1) && ARG_MOB(0)->race == race_lookup(ARG_STR(1));
	return TRUE;
}

// Random number check
// Forms:
//	rand <num>		TRUE: [0:99] < num
//	rand <num1> <num2>	TRUE: [0:num2] < num1
DECL_IFC_FUN(ifc_rand)
{
	if(ISARG_NUM(1))
		*ret = (number_range(0,ARG_NUM(1)-1) < ARG_NUM(0));
	else
		*ret = (number_percent() < ARG_NUM(0));
	return TRUE;
}

DECL_IFC_FUN(ifc_res)
{
	*ret = (ISARG_MOB(0) && VALID_STR(1) && IS_SET(ARG_MOB(0)->res_flags, flag_value(res_flags,ARG_STR(1))));
	return TRUE;
}

DECL_IFC_FUN(ifc_room)
{
	if(ISARG_MOB(0)) room = ARG_MOB(0)->in_room;
	else if(ISARG_OBJ(0)) room = obj_room(ARG_OBJ(0));
	else if(ISARG_ROOM(0)) room = ARG_ROOM(0);
	else if(ISARG_TOK(0)) room = token_room(ARG_TOK(0));
	else return FALSE;

	*ret = room ? room->vnum : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_roomflag)
{
	// Wilderness format
	if(ISARG_NUM(0) && ISARG_NUM(1) && ISARG_NUM(2)) {
		WILDS_DATA *pWilds;
		WILDS_TERRAIN *pTerrain;

		if(!ISARG_STR(3)) return FALSE;

		if(!(pWilds = get_wilds_from_uid(NULL,ARG_NUM(0))))
			*ret = FALSE;
		else if((room = get_wilds_vroom(pWilds, ARG_NUM(1), ARG_NUM(2))))
			*ret = (IS_SET(room->room_flags, flag_value(room_flags,ARG_STR(3))));
		else if(!(pTerrain = get_terrain_by_coors (pWilds, ARG_NUM(1), ARG_NUM(2))))
			*ret = FALSE;
		else
			*ret = (IS_SET(pTerrain->template->room_flags, flag_value(room_flags,ARG_STR(3))));

	} else {
		if(!ISARG_STR(1)) return FALSE;
		if(ISARG_MOB(0)) room = ARG_MOB(0)->in_room;
		else if(ISARG_OBJ(0)) room = obj_room(ARG_OBJ(0));
		else if(ISARG_ROOM(0)) room = ARG_ROOM(0);
		else if(ISARG_TOK(0)) room = token_room(ARG_TOK(0));
		else return FALSE;
		*ret = (room && IS_SET(room->room_flags, flag_value(room_flags,ARG_STR(1))));
	}

	return TRUE;
}

DECL_IFC_FUN(ifc_roomflag2)
{
	// Wilderness format
	if(ISARG_NUM(0) && ISARG_NUM(1) && ISARG_NUM(2)) {
		WILDS_DATA *pWilds;
		WILDS_TERRAIN *pTerrain;

		if(!ISARG_STR(3)) return FALSE;

		if(!(pWilds = get_wilds_from_uid(NULL,ARG_NUM(0))))
			*ret = FALSE;
		else if((room = get_wilds_vroom(pWilds, ARG_NUM(1), ARG_NUM(2))))
			*ret = (IS_SET(room->room2_flags, flag_value(room2_flags,ARG_STR(3))));
		else if(!(pTerrain = get_terrain_by_coors (pWilds, ARG_NUM(1), ARG_NUM(2))))
			*ret = FALSE;
		else
			*ret = (IS_SET(pTerrain->template->room2_flags, flag_value(room2_flags,ARG_STR(3))));

	} else {
		if(!ISARG_STR(1)) return FALSE;
		if(ISARG_MOB(0)) room = ARG_MOB(0)->in_room;
		else if(ISARG_OBJ(0)) room = obj_room(ARG_OBJ(0));
		else if(ISARG_ROOM(0)) room = ARG_ROOM(0);
		else if(ISARG_TOK(0)) room = token_room(ARG_TOK(0));
		else return FALSE;
		*ret = (room && IS_SET(room->room2_flags, flag_value(room2_flags,ARG_STR(1))));
	}

	return TRUE;
}

DECL_IFC_FUN(ifc_roomweight)
{
	if(ISARG_MOB(0)) room = ARG_MOB(0)->in_room;
	else if(ISARG_OBJ(0)) room = obj_room(ARG_OBJ(0));
	else if(ISARG_ROOM(0)) room = ARG_ROOM(0);
	else if(ISARG_TOK(0)) room = token_room(ARG_TOK(0));
	else return FALSE;

	*ret = get_room_weight(room,TRUE,TRUE,FALSE);
	return TRUE;
}

DECL_IFC_FUN(ifc_sector)
{
	// Wilderness format
	if(ISARG_NUM(0) && ISARG_NUM(1) && ISARG_NUM(2)) {
		WILDS_DATA *pWilds;
		WILDS_TERRAIN *pTerrain;

		if(!ISARG_STR(3)) return FALSE;

		if(!(pWilds = get_wilds_from_uid(NULL,ARG_NUM(0))))
			*ret = FALSE;
		else if((room = get_wilds_vroom(pWilds, ARG_NUM(1), ARG_NUM(2))))
			*ret = (room->sector_type == flag_value( sector_flags,ARG_STR(3)));
		else if(!(pTerrain = get_terrain_by_coors (pWilds, ARG_NUM(1), ARG_NUM(2))))
			*ret = FALSE;
		else
			*ret = (pTerrain->template->sector_type == flag_value( sector_flags,ARG_STR(3)));

	} else {
		if(!ISARG_STR(1)) return FALSE;
		if(ISARG_MOB(0)) room = ARG_MOB(0)->in_room;
		else if(ISARG_OBJ(0)) room = obj_room(ARG_OBJ(0));
		else if(ISARG_ROOM(0)) room = ARG_ROOM(0);
		else if(ISARG_TOK(0)) room = token_room(ARG_TOK(0));
		else return FALSE;

		*ret = (room && room->sector_type == flag_value( sector_flags,ARG_STR(1)));
	}
	return TRUE;
}

DECL_IFC_FUN(ifc_sex)
{
	*ret = ISARG_MOB(0) ? ARG_MOB(0)->sex : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_silver)
{
	*ret = ISARG_MOB(0) ? ARG_MOB(0)->silver : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_skeyword)
{
	if(!ISARG_STR(1)) return FALSE;
	if(ISARG_MOB(0)) *ret = IS_NPC(ARG_MOB(0)) && is_name(ARG_STR(1), ARG_MOB(0)->pIndexData->skeywds);
	else if(ISARG_OBJ(0)) *ret = is_name(ARG_STR(1), ARG_OBJ(0)->pIndexData->skeywds);
	else return FALSE;
	return TRUE;
}

DECL_IFC_FUN(ifc_skill)
{
	int sn;

	*ret = (VALID_PLAYER(0) && ISARG_STR(1) && ((sn = skill_lookup(ARG_STR(1))) > 0)) ?
			get_skill(ARG_MOB(0), sn) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_statcon)
{
	*ret = ISARG_MOB(0) ? get_curr_stat(ARG_MOB(0),STAT_CON) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_statdex)
{
	*ret = ISARG_MOB(0) ? get_curr_stat(ARG_MOB(0),STAT_DEX) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_statint)
{
	*ret = ISARG_MOB(0) ? get_curr_stat(ARG_MOB(0),STAT_INT) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_statstr)
{
	*ret = ISARG_MOB(0) ? get_curr_stat(ARG_MOB(0),STAT_STR) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_statwis)
{
	*ret = ISARG_MOB(0) ? get_curr_stat(ARG_MOB(0),STAT_WIS) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_testskill)
{
	int sn;
	*ret = (VALID_PLAYER(0) && ISARG_STR(1) && ((sn = skill_lookup(ARG_STR(1))) > 0) &&
			number_percent() < get_skill(ARG_MOB(0), sn));
	return TRUE;
}

DECL_IFC_FUN(ifc_thirst)
{
	*ret = VALID_PLAYER(0) ? ARG_MOB(0)->pcdata->condition[COND_THIRST] : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_stoned)
{
	*ret = VALID_PLAYER(0) ? ARG_MOB(0)->pcdata->condition[COND_STONED] : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_tokencount)
{
	TOKEN_INDEX_DATA *ti = NULL;
	TOKEN_DATA *tok;
	int i;

	if((ISARG_NUM(1) && !(ti = get_token_index(ARG_NUM(1)))))
		return FALSE;

	if(ISARG_MOB(0)) tok = ARG_MOB(0)->tokens;
	else if(ISARG_OBJ(0)) tok = ARG_OBJ(0)->tokens;
	else if(ISARG_ROOM(0)) tok = ARG_ROOM(0)->tokens;
	else return FALSE;

	for(i = 0;tok;tok = tok->next) if(!ti || (tok->pIndexData == ti)) ++i;

	*ret = i;
	return TRUE;
}

DECL_IFC_FUN(ifc_tokenexists)
{
	TOKEN_INDEX_DATA *ti;
	*ret = (ISARG_NUM(0) && (ti = get_token_index(ARG_NUM(0))) && ti->loaded > 0);
	return TRUE;
}

DECL_IFC_FUN(ifc_tokentimer)
{
	TOKEN_DATA *tok;

	if(ISARG_MOB(0) && ISARG_NUM(1) && ISARG_NUM(2)) {
		tok = get_token_char(ARG_MOB(0), ARG_NUM(1), ARG_NUM(2));

	} else if(ISARG_OBJ(0) && ISARG_NUM(1) && ISARG_NUM(2)) {
		tok = get_token_obj(ARG_OBJ(0), ARG_NUM(1), ARG_NUM(2));

	} else if(ISARG_ROOM(0) && ISARG_NUM(1) && ISARG_NUM(2)) {
		tok = get_token_room(ARG_ROOM(0), ARG_NUM(1), ARG_NUM(2));

	} else if(ISARG_MOB(0) && ISARG_NUM(1)) {
		tok = get_token_char(ARG_MOB(0), ARG_NUM(1), 1);

	} else if(ISARG_OBJ(0) && ISARG_NUM(1)) {
		tok = get_token_obj(ARG_OBJ(0), ARG_NUM(1), 1);

	} else if(ISARG_ROOM(0) && ISARG_NUM(1)) {
		tok = get_token_room(ARG_ROOM(0), ARG_NUM(1), 1);

	} else if(ISARG_TOK(0)) {
		tok = ARG_TOK(0);
	} else
		return FALSE;

	// If the token doesn't exist on the entity, any checks on its values are always false.
	if(!tok) return FALSE;

	*ret = tok->timer;
	return TRUE;
}

DECL_IFC_FUN(ifc_tokentype)
{
	return FALSE;
}

DECL_IFC_FUN(ifc_tokenvalue)
{
	TOKEN_DATA *tok;
	int val;

	if(ISARG_MOB(0) && ISARG_NUM(1) && ISARG_NUM(2) && ISARG_NUM(3)) {
		tok = get_token_char(ARG_MOB(0), ARG_NUM(1), ARG_NUM(2));
		val = ARG_NUM(3);

	} else if(ISARG_OBJ(0) && ISARG_NUM(1) && ISARG_NUM(2) && ISARG_NUM(3)) {
		tok = get_token_obj(ARG_OBJ(0), ARG_NUM(1), ARG_NUM(2));
		val = ARG_NUM(3);

	} else if(ISARG_ROOM(0) && ISARG_NUM(1) && ISARG_NUM(2) && ISARG_NUM(3)) {
		tok = get_token_room(ARG_ROOM(0), ARG_NUM(1), ARG_NUM(2));
		val = ARG_NUM(3);

	} else if(ISARG_MOB(0) && ISARG_NUM(1) && ISARG_NUM(2)) {
		tok = get_token_char(ARG_MOB(0), ARG_NUM(1), 1);
		val = ARG_NUM(2);

	} else if(ISARG_OBJ(0) && ISARG_NUM(1) && ISARG_NUM(2)) {
		tok = get_token_obj(ARG_OBJ(0), ARG_NUM(1), 1);
		val = ARG_NUM(2);

	} else if(ISARG_ROOM(0) && ISARG_NUM(1) && ISARG_NUM(2)) {
		tok = get_token_room(ARG_ROOM(0), ARG_NUM(1), 1);
		val = ARG_NUM(2);

	} else if(ISARG_TOK(0) && ISARG_NUM(1)) {
		tok = ARG_TOK(0);
		val = ARG_NUM(1);
	} else
		return FALSE;

	// If the token doesn't exist on the entity, any checks on its values are always false.
	if(!tok) return FALSE;

	if (val < 0 || val >= MAX_TOKEN_VALUES)
		return FALSE;

	*ret = tok->value[val];
	return TRUE;
}

DECL_IFC_FUN(ifc_totalfights)
{
	*ret = VALID_PLAYER(0) ? (ARG_MOB(0)->cpk_deaths+ARG_MOB(0)->cpk_kills+
			ARG_MOB(0)->arena_deaths+ARG_MOB(0)->arena_kills+
			ARG_MOB(0)->player_deaths+ARG_MOB(0)->player_kills) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_totalloss)
{
	*ret = VALID_PLAYER(0) ? (ARG_MOB(0)->cpk_deaths+ARG_MOB(0)->arena_deaths+ARG_MOB(0)->player_deaths) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_totalpkfights)
{
	*ret = VALID_PLAYER(0) ? (ARG_MOB(0)->cpk_deaths+ARG_MOB(0)->cpk_kills+
	    	ARG_MOB(0)->player_deaths+ARG_MOB(0)->player_kills) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_totalpkloss)
{
	*ret = VALID_PLAYER(0) ? (ARG_MOB(0)->cpk_deaths+ARG_MOB(0)->player_deaths) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_totalpkratio)
{
	int val;
	if(VALID_PLAYER(0)) {
		val = ARG_MOB(0)->cpk_deaths+ARG_MOB(0)->cpk_kills+
			ARG_MOB(0)->player_deaths+ARG_MOB(0)->player_kills;
		if(val > 0) val = 1000 * (ARG_MOB(0)->cpk_kills+ARG_MOB(0)->player_kills) / val;
		*ret = val;
		return TRUE;
	}
	return FALSE;
}

DECL_IFC_FUN(ifc_totalpkwins)
{
	*ret = VALID_PLAYER(0) ? (ARG_MOB(0)->cpk_kills+ARG_MOB(0)->player_kills) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_totalquests)
{
	*ret = VALID_PLAYER(0) ? ARG_MOB(0)->pcdata->quests_completed : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_totalratio)
{
	int val;
	if(VALID_PLAYER(0)) {
		val = ARG_MOB(0)->cpk_deaths+ARG_MOB(0)->cpk_kills+
			ARG_MOB(0)->arena_deaths+ARG_MOB(0)->arena_kills+
			ARG_MOB(0)->player_deaths+ARG_MOB(0)->player_kills;
		if(val > 0) val = 1000 * (ARG_MOB(0)->cpk_kills+ARG_MOB(0)->arena_kills+ARG_MOB(0)->player_kills) / val;
		*ret = val;
		return TRUE;
	}
	return FALSE;
}

DECL_IFC_FUN(ifc_totalwins)
{
	*ret = VALID_PLAYER(0) ? (ARG_MOB(0)->cpk_kills+ARG_MOB(0)->arena_kills+ARG_MOB(0)->player_kills) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_toxin)
{
	int tox;
	if(ISARG_MOB(0) && ISARG_STR(1)) {
		*ret = ((tox = toxin_lookup(ARG_STR(1))) >= 0 && tox < MAX_TOXIN) ?
			ARG_MOB(0)->toxin[tox] : 0;
		return TRUE;
	}

	return FALSE;
}

DECL_IFC_FUN(ifc_trains)
{
	*ret = VALID_PLAYER(0) ? ARG_MOB(0)->train : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_uses)
{
	*ret = (ISARG_MOB(0) && ISARG_STR(1) && has_item(ARG_MOB(0), -1, item_lookup(ARG_STR(1)), TRUE));
	return TRUE;
}

DECL_IFC_FUN(ifc_varexit)
{
	pVARIABLE var, vars = NULL;
	if(ISARG_MOB(0)) { vars = ARG_MOB(0)->progs->vars; ++argv; }
	else if(ISARG_OBJ(0)) { vars = ARG_OBJ(0)->progs->vars; ++argv; }
	else if(ISARG_ROOM(0)) { vars = ARG_ROOM(0)->progs->vars; ++argv; }
	else if(ISARG_TOK(0)) { vars = ARG_TOK(0)->progs->vars; ++argv; }
	else if(mob) vars = mob->progs->vars;
	else if(obj) vars = obj->progs->vars;
	else if(room) vars = room->progs->vars;
	else if(token) vars = token->progs->vars;

	if(ISARG_STR(0) && ISARG_STR(1)) {
		int door = get_num_dir(ARG_STR(1));

		var = variable_get(vars,ARG_STR(0));

		if(var && var->type == VAR_EXIT) {
			*ret = var->_.door.r && var->_.door.door == door;
			return TRUE;
		}
	}
	return FALSE;
}

DECL_IFC_FUN(ifc_varnumber)
{
	pVARIABLE var, vars = NULL;
	if(ISARG_MOB(0)) { vars = ARG_MOB(0)->progs->vars; ++argv; }
	else if(ISARG_OBJ(0)) { vars = ARG_OBJ(0)->progs->vars; ++argv; }
	else if(ISARG_ROOM(0)) { vars = ARG_ROOM(0)->progs->vars; ++argv; }
	else if(ISARG_TOK(0)) { vars = ARG_TOK(0)->progs->vars; ++argv; }
	else if(mob) vars = mob->progs->vars;
	else if(obj) vars = obj->progs->vars;
	else if(room) vars = room->progs->vars;
	else if(token) vars = token->progs->vars;

	if(ISARG_STR(0)) {
		var = variable_get(vars,ARG_STR(0));

		if(var && var->type == VAR_INTEGER) {
			*ret = var->_.i;
			return TRUE;
		}
	}
	return FALSE;
}

DECL_IFC_FUN(ifc_vardefined)
{
	char buf[MIL];
	PROG_DATA * progs = NULL;
	pVARIABLE var;
	if(ISARG_MOB(0)) { progs = ARG_MOB(0)->progs; ++argv; }
	else if(ISARG_OBJ(0)) { progs = ARG_OBJ(0)->progs; ++argv; }
	else if(ISARG_ROOM(0)) { progs = ARG_ROOM(0)->progs; ++argv; }
	else if(ISARG_TOK(0)) { progs = ARG_TOK(0)->progs; ++argv; }
	else if(mob) progs = mob->progs;
	else if(obj) progs = obj->progs;
	else if(room) progs = room->progs;
	else if(token) progs = token->progs;

//	if(wiznet_script) {
//		sprintf(buf, "vardefined searching for '%s'", ARG_STR(0));
//		wiznet(buf,NULL,NULL,WIZ_SCRIPTS,0,0);
//	}

	if(progs && progs->vars && ISARG_STR(0)) {
		var = variable_get(progs->vars,ARG_STR(0));

//		if( var && wiznet_script ) {
//			sprintf(buf, "vardefined found variable: '%s'", var->name);
//			wiznet(buf,NULL,NULL,WIZ_SCRIPTS,0,0);
//		}

		*ret = var ? TRUE : FALSE;
		return TRUE;
	}
	return FALSE;
}

DECL_IFC_FUN(ifc_varstring)
{
	pVARIABLE var, vars = NULL;
	if(ISARG_MOB(0)) { vars = ARG_MOB(0)->progs->vars; ++argv; }
	else if(ISARG_OBJ(0)) { vars = ARG_OBJ(0)->progs->vars; ++argv; }
	else if(ISARG_ROOM(0)) { vars = ARG_ROOM(0)->progs->vars; ++argv; }
	else if(ISARG_TOK(0)) { vars = ARG_TOK(0)->progs->vars; ++argv; }
	else if(mob) vars = mob->progs->vars;
	else if(obj) vars = obj->progs->vars;
	else if(room) vars = room->progs->vars;
	else if(token) vars = token->progs->vars;

	if(ISARG_STR(0) && ISARG_STR(1)) {
		var = variable_get(vars,ARG_STR(0));

		if(var && (var->type == VAR_STRING || var->type == VAR_STRING_S)) {
			*ret = is_name(ARG_STR(1),var->_.s);
			return TRUE;
		}
	}
	return FALSE;
}

DECL_IFC_FUN(ifc_vnum)
{
	if(ISARG_MOB(0)) *ret = IS_NPC(ARG_MOB(0)) ? ARG_MOB(0)->pIndexData->vnum : 0;
	else if(ISARG_OBJ(0)) *ret = ARG_OBJ(0)->pIndexData->vnum;
	else if(ISARG_ROOM(0)) *ret = ARG_ROOM(0)->vnum;
	else if(ISARG_TOK(0)) *ret = ARG_TOK(0)->pIndexData->vnum;
	else *ret = 0;

	return TRUE;
}

DECL_IFC_FUN(ifc_vuln)
{
	*ret = (ISARG_MOB(0) && ISARG_STR(1) && IS_SET(ARG_MOB(0)->vuln_flags, flag_value(vuln_flags,ARG_STR(1))));
	return TRUE;
}

DECL_IFC_FUN(ifc_weapon)
{
	*ret = ISARG_OBJ(0) && ARG_OBJ(0)->item_type == ITEM_WEAPON &&
		IS_SET(ARG_OBJ(0)->value[4], flag_value(weapon_type2,ARG_STR(1)));
	return TRUE;
}

DECL_IFC_FUN(ifc_weapontype)
{
	*ret = ISARG_OBJ(0) && ARG_OBJ(0)->item_type == ITEM_WEAPON &&
		ARG_OBJ(0)->value[0] == weapon_type(ARG_STR(1));
	return TRUE;
}

DECL_IFC_FUN(ifc_weaponskill)
{
	int sn;

	*ret = (VALID_PLAYER(0) && ISARG_OBJ(1) && ((sn = get_objweapon_sn(ARG_OBJ(1))) > 0)) ?
			get_skill(ARG_MOB(0), sn) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_wears)
{
	if(ISARG_MOB(0)) {
		if(ISARG_NUM(1)) {
			*ret = (int)has_item(ARG_MOB(0), ARG_NUM(1), -1, TRUE);
		} else if(ISARG_STR(1)) {
			if (is_number(ARG_STR(1)))
				*ret = (int)has_item(ARG_MOB(0), atol(ARG_STR(1)), -1, TRUE);
			else
				*ret = (int)(get_obj_wear(ARG_MOB(0), ARG_STR(1), TRUE) && 1);
		} else if(ISARG_OBJ(1)) {
			*ret = (ARG_OBJ(1)->carried_by == ARG_MOB(0) && ARG_OBJ(1)->wear_loc != WEAR_NONE) ||
				(ARG_OBJ(1)->in_obj && ARG_OBJ(1)->in_obj->carried_by == ARG_MOB(0) && ARG_OBJ(1)->in_obj->wear_loc != WEAR_NONE);
		} else
			return FALSE;
		return TRUE;
	}
	return FALSE;

}

DECL_IFC_FUN(ifc_wearused)
{
	int wl;
	*ret = ISARG_MOB(0) && ((wl = flag_value(wear_loc_flags,ARG_STR(1))) != WEAR_NONE)
		&& get_eq_char(ARG_MOB(0), wl);
	return TRUE;
}

DECL_IFC_FUN(ifc_weight)
{
	if(ISARG_MOB(0)) *ret = get_carry_weight(ARG_MOB(0));
	else if(ISARG_OBJ(0)) *ret = get_obj_weight(ARG_OBJ(0));
	else return FALSE;

	return TRUE;
}

DECL_IFC_FUN(ifc_weightleft)
{
	if (ISARG_OBJ(0)) {
		obj = ARG_OBJ(0);
		if(obj->item_type == ITEM_CART ||
			obj->item_type == ITEM_CONTAINER ||
			obj->item_type == ITEM_WEAPON_CONTAINER) {
			*ret = obj->value[0] - get_obj_weight_container(obj);
		} else
			*ret = 0;
		return TRUE;
	} else if(ISARG_MOB(0)) {
		*ret = can_carry_w(ARG_MOB(0)) - get_carry_weight(ARG_MOB(0));
		return TRUE;
	}

	return FALSE;
}

DECL_IFC_FUN(ifc_wimpy)
{
	*ret = VALID_PLAYER(0) ? ARG_MOB(0)->wimpy : 0;
	return TRUE;
}

// Checks to see if the SELF object is worn by the target mobile
DECL_IFC_FUN(ifc_wornby)
{
	*ret = (obj && ISARG_MOB(0) && obj->carried_by == ARG_MOB(0) &&
		obj->wear_loc != WEAR_NONE);
	return TRUE;
}

// Returns the current game year
DECL_IFC_FUN(ifc_year)
{
	*ret = time_info.year;
	return TRUE;
}


DECL_IFC_FUN(ifc_bit)
{
	*ret = 0;

	while(argc > 0) {
		if(ISARG_NUM(0) && ARG_NUM(0) >= 0 && ARG_NUM(0) < (sizeof(int)*8))
			*ret |= (1<<ARG_NUM(0));
		argc--;
		argv++;
	}

	return TRUE;
}

DECL_IFC_FUN(ifc_flag_act)
{
	*ret = ISARG_STR(0) ? flag_value(act_flags,ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_flag_act2)
{
	*ret = ISARG_STR(0) ? flag_value(act2_flags,ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_flag_affect)
{
	*ret = ISARG_STR(0) ? flag_value(affect_flags,ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_flag_affect2)
{
	*ret = ISARG_STR(0) ? flag_value(affect2_flags,ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_flag_container)
{
	*ret = ISARG_STR(0) ? flag_value(container_flags,ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_flag_exit)
{
	*ret = ISARG_STR(0) ? flag_value(exit_flags,ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_flag_extra)
{
	*ret = ISARG_STR(0) ? flag_value(extra_flags,ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_flag_extra2)
{
	*ret = ISARG_STR(0) ? flag_value(extra2_flags,ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_flag_extra3)
{
	*ret = ISARG_STR(0) ? flag_value(extra3_flags,ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_flag_extra4)
{
	*ret = ISARG_STR(0) ? flag_value(extra4_flags,ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_flag_form)
{
	*ret = ISARG_STR(0) ? flag_value(form_flags,ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_flag_furniture)
{
	*ret = ISARG_STR(0) ? flag_value(furniture_flags,ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_flag_imm)
{
	*ret = ISARG_STR(0) ? flag_value(imm_flags,ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_flag_interrupt)
{
	*ret = ISARG_STR(0) ? flag_value(interrupt_action_types,ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_flag_off)
{
	*ret = ISARG_STR(0) ? flag_value(off_flags,ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_flag_part)
{
	*ret = ISARG_STR(0) ? flag_value(part_flags,ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_flag_portal)
{
	*ret = ISARG_STR(0) ? flag_value(portal_flags,ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_flag_res)
{
	*ret = ISARG_STR(0) ? flag_value(res_flags,ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_flag_room)
{
	*ret = ISARG_STR(0) ? flag_value(room_flags,ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_flag_room2)
{
	*ret = ISARG_STR(0) ? flag_value(room2_flags,ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_flag_vuln)
{
	*ret = ISARG_STR(0) ? flag_value(vuln_flags,ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_flag_weapon)
{
	*ret = ISARG_STR(0) ? flag_value(weapon_type2,ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_flag_wear)
{
	*ret = ISARG_STR(0) ? flag_value(wear_flags,ARG_STR(0)) : 0;
	return TRUE;
}


DECL_IFC_FUN(ifc_value_ac)
{
	*ret = ISARG_STR(0) ? flag_value(ac_type,ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_value_damage)
{
	*ret = ISARG_STR(0) ? attack_lookup(ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_value_position)
{
	*ret = ISARG_STR(0) ? flag_value(position_flags,ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_value_ranged)
{
	*ret = ISARG_STR(0) ? flag_value(ranged_weapon_class,ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_value_relic)
{
	*ret = ISARG_STR(0) ? flag_value(relic_types,ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_value_sector)
{
	*ret = ISARG_STR(0) ? flag_value(sector_flags,ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_value_size)
{
	*ret = ISARG_STR(0) ? flag_value(size_flags,ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_value_toxin)
{
	*ret = ISARG_STR(0) ? toxin_lookup(ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_value_type)
{
	*ret = ISARG_STR(0) ? flag_value(type_flags,ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_value_weapon)
{
	*ret = ISARG_STR(0) ? flag_value(weapon_class,ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_value_wear)
{
	*ret = ISARG_STR(0) ? flag_value(wear_loc_flags,ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_value_moon)
{
	*ret = ISARG_STR(0) ? flag_value(moon_phases,ARG_STR(0)) : -1;
	return TRUE;
}

DECL_IFC_FUN(ifc_value_acstr)
{
	*ret = ISARG_STR(0) ? flag_value(armor_strength_table,ARG_STR(0)) : -1;
	return TRUE;
}

DECL_IFC_FUN(ifc_timer)
{
	int i;
	*ret = 0;
	if(ISARG_OBJ(0)) *ret = ARG_OBJ(0)->timer;
	else if(ISARG_MOB(0)) {
		if(!ISARG_STR(1)) {
			if(argc == 1) *ret = ARG_MOB(0)->wait;
			else return FALSE;
		} else if(!str_prefix(ARG_STR(1),"banish")) *ret = ARG_MOB(0)->maze_time_left;
		else if(!str_prefix(ARG_STR(1),"bashed")) *ret = ARG_MOB(0)->bashed;
		else if(!str_prefix(ARG_STR(1),"bind")) *ret = ARG_MOB(0)->bind;
		else if(!str_prefix(ARG_STR(1),"bomb")) *ret = ARG_MOB(0)->bomb;
		else if(!str_prefix(ARG_STR(1),"brew")) *ret = ARG_MOB(0)->brew;
		else if(!str_prefix(ARG_STR(1),"cast")) *ret = ARG_MOB(0)->cast;
		else if(!str_prefix(ARG_STR(1),"daze")) *ret = ARG_MOB(0)->daze;
		else if(!str_prefix(ARG_STR(1),"death")) *ret = ARG_MOB(0)->time_left_death;
		else if(!str_prefix(ARG_STR(1),"fade")) *ret = ARG_MOB(0)->fade;
		else if(!str_prefix(ARG_STR(1),"hide")) *ret = ARG_MOB(0)->hide;
		else if(!str_prefix(ARG_STR(1),"music")) *ret = ARG_MOB(0)->music;
		else if(!str_prefix(ARG_STR(1),"norecall")) *ret = ARG_MOB(0)->no_recall;
		else if(!str_prefix(ARG_STR(1),"panic")) *ret = ARG_MOB(0)->panic;
		else if(!str_prefix(ARG_STR(1),"paralyzed")) *ret = ARG_MOB(0)->paralyzed;
		else if(!str_prefix(ARG_STR(1),"paroxysm")) *ret = ARG_MOB(0)->paroxysm;
		else if(!str_prefix(ARG_STR(1),"pk")) *ret = ARG_MOB(0)->pk_timer;
		else if(!str_prefix(ARG_STR(1),"ranged")) *ret = ARG_MOB(0)->ranged;
		else if(!str_prefix(ARG_STR(1),"recite")) *ret = ARG_MOB(0)->recite;
		else if(!str_prefix(ARG_STR(1),"resurrect")) *ret = ARG_MOB(0)->resurrect;
		else if(!str_prefix(ARG_STR(1),"reverie")) *ret = ARG_MOB(0)->reverie;
		else if(!str_prefix(ARG_STR(1),"scribe")) *ret = ARG_MOB(0)->scribe;
		else if(!str_prefix(ARG_STR(1),"timer")) *ret = ARG_MOB(0)->timer;
		else if(!str_prefix(ARG_STR(1),"trance")) *ret = ARG_MOB(0)->trance;
		else if(!str_prefix(ARG_STR(1),"wait")) *ret = ARG_MOB(0)->wait;
		else return FALSE;
	} else if(ISARG_STR(0)) {
		if(!str_prefix(ARG_STR(0),"reckoning"))
			*ret = UMAX(0,reckoning_timer);
		else if(!str_prefix(ARG_STR(0),"prereckoning"))
			*ret = UMAX(0,pre_reckoning);
		else {
			for(i=0;boost_table[i].name;i++)
				if (!str_prefix(ARG_STR(0), boost_table[i].name))
					break;
			if(boost_table[i].name)
				*ret = UMAX(0,boost_table[i].timer);
			else
				return FALSE;
		}
	} else if(ISARG_TOK(0)) {
		*ret = ARG_TOK(0)->timer;
	} else
		return FALSE;

	return TRUE;
}


DECL_IFC_FUN(ifc_isrestrung)
{
	*ret = ISARG_OBJ(0) && (ARG_OBJ(0)->old_short_descr || ARG_OBJ(0)->old_description);
	return TRUE;
}

DECL_IFC_FUN(ifc_timeofday)
{
	int val;

	*ret = FALSE;

	if(ISARG_STR(0)) {

		val = flag_value(time_of_day_flags,ARG_STR(0));

		if(IS_SET(val, TOD_AFTERMIDNIGHT)) {
			if(time_info.hour > 0 && time_info.hour < 5) { *ret = TRUE; return TRUE; }
		}

		if(IS_SET(val, TOD_AFTERNOON)) {
			if(time_info.hour > 12 && time_info.hour < 19) { *ret = TRUE; return TRUE; }
		}

		if(IS_SET(val, TOD_DAWN)) {
			if(time_info.hour == 5) { *ret = TRUE; return TRUE; }
		}

		if(IS_SET(val, TOD_DAY)) {
			if(time_info.hour > 5 && time_info.hour < 20) { *ret = TRUE; return TRUE; }
		}

		if(IS_SET(val, TOD_DUSK)) {
			if(time_info.hour == 19) { *ret = TRUE; return TRUE; }
		}

		if(IS_SET(val, TOD_EVENING)) {
			if(time_info.hour > 19 && time_info.hour < 24) { *ret = TRUE; return TRUE; }
		}

		if(IS_SET(val, TOD_MIDNIGHT)) {
			if(!time_info.hour) { *ret = TRUE; return TRUE; }
		}

		if(IS_SET(val, TOD_MORNING)) {
			if(time_info.hour > 5 && time_info.hour < 12) { *ret = TRUE; return TRUE; }
		}

		if(IS_SET(val, TOD_NIGHT)) {
			if(time_info.hour < 6 || time_info.hour > 19) { *ret = TRUE; return TRUE; }
		}

		if(IS_SET(val, TOD_NOON)) {
			if(time_info.hour == 12) { *ret = TRUE; return TRUE; }
		}
	}

	return TRUE;
}

DECL_IFC_FUN(ifc_ismoonup)
{
	// Fix to take in location!
	*ret = TRUE;
	return TRUE;
}

DECL_IFC_FUN(ifc_moonphase)
{
	*ret = time_info.moon;
	return TRUE;
}

DECL_IFC_FUN(ifc_reckoning)
{
	*ret = (reckoning_timer > 0) ? ((pre_reckoning > 0) ? 1 : 2) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_reckoningchance)
{
	*ret = reckoning_chance;
	return TRUE;
}

DECL_IFC_FUN(ifc_death)
{
	*ret = (ISARG_MOB(0) && ISARG_STR(1) && (ARG_MOB(0)->death_type == flag_value(death_types,ARG_STR(1))));
	return TRUE;
}

// word STRING NUMBER STRING
DECL_IFC_FUN(ifc_word)
{
	int i,c;
	char *p, buf[MIL];

	*ret = FALSE;
	if(ISARG_STR(0) && ISARG_NUM(1) && ISARG_STR(2)) {
		c = ARG_NUM(1);

		if(c > 0) {
			p = ARG_STR(0);
			for(i=0;i<c;i++) p = one_argument(p,buf);
			*ret = !str_cmp(buf,ARG_STR(2));
		} else
			*ret = !str_cmp(ARG_STR(0),ARG_STR(2));
	}

	return TRUE;
}

DECL_IFC_FUN(ifc_inputwait)
{
	*ret = VALID_PLAYER(0) && ARG_MOB(0)->desc && (ARG_MOB(0)->desc->input ||
		ARG_MOB(0)->remove_question ||
		ARG_MOB(0)->pcdata->inquiry_subject ||
		ARG_MOB(0)->pk_question ||
		ARG_MOB(0)->personal_pk_question ||
		ARG_MOB(0)->cross_zone_question ||
		ARG_MOB(0)->pcdata->convert_church != -1 ||
		ARG_MOB(0)->challenged);
	return TRUE;
}

DECL_IFC_FUN(ifc_cos)
{
	*ret = 0;
	if(ISARG_NUM(0)) {
		double scale = (double)((ISARG_NUM(1) && ARG_NUM(1) > 1) ? ARG_NUM(1) : 1);
		*ret = (int)(10000 * cos(ARG_NUM(0) * 3.1415926 / 180.0 / scale) + 0.5);
	}
	return TRUE;
}

DECL_IFC_FUN(ifc_sin)
{
	*ret = 0;
	if(ISARG_NUM(0)) {
		double scale = (double)((ISARG_NUM(1) && ARG_NUM(1) > 1) ? ARG_NUM(1) : 1);
		*ret = (int)(10000 * sin(ARG_NUM(0) * 3.1415926 / 180.0 / scale) + 0.5);
	}
	return TRUE;
}

DECL_IFC_FUN(ifc_angle)
{
	*ret = -1;
	if(ISARG_NUM(0) && ISARG_NUM(1)) {
		int scale = (ISARG_NUM(2) && ARG_NUM(2) > 1) ? ARG_NUM(2) : 1;
		*ret = (int)(scale * atan2(ARG_NUM(1),ARG_NUM(0)) * 180.0 / 3.1415926);
		if(*ret < 0) *ret += 360 * scale;
	}
	return TRUE;
}

DECL_IFC_FUN(ifc_sign)
{
	*ret = 0;
	if(ISARG_NUM(0))
		*ret = ((ARG_NUM(0) < 0)?-1:((ARG_NUM(0) > 0)?1:0));
	return TRUE;
}

DECL_IFC_FUN(ifc_abs)
{
	*ret = 0;
	if(ISARG_NUM(0))
		*ret = abs(ARG_NUM(0));
	return TRUE;
}

DECL_IFC_FUN(ifc_inwilds)
{
	if(ISARG_MOB(0)) room = ARG_MOB(0)->in_room;
	else if(ISARG_OBJ(0)) room = obj_room(ARG_OBJ(0));
	else if(ISARG_ROOM(0)) room = ARG_ROOM(0);
	else if(ISARG_TOK(0)) room = token_room(ARG_TOK(0));
	else return FALSE;

	*ret = (room->wilds != NULL);
	return TRUE;
}


DECL_IFC_FUN(ifc_areaid)
{
	if(ISARG_MOB(0)) room = ARG_MOB(0)->in_room;
	else if(ISARG_OBJ(0)) room = obj_room(ARG_OBJ(0));
	else if(ISARG_ROOM(0)) room = ARG_ROOM(0);
	else if(ISARG_TOK(0)) room = token_room(ARG_TOK(0));
	else return FALSE;

	*ret = room ? room->area->uid : -1;
	return TRUE;
}


DECL_IFC_FUN(ifc_mapid)
{
	if(ISARG_MOB(0)) room = ARG_MOB(0)->in_room;
	else if(ISARG_OBJ(0)) room = obj_room(ARG_OBJ(0));
	else if(ISARG_ROOM(0)) room = ARG_ROOM(0);
	else if(ISARG_TOK(0)) room = token_room(ARG_TOK(0));
	else return FALSE;

	*ret = (room && room->wilds)?room->wilds->uid:-1;
	return TRUE;
}


DECL_IFC_FUN(ifc_mapx)
{
	if(ISARG_MOB(0)) room = ARG_MOB(0)->in_room;
	else if(ISARG_OBJ(0)) room = obj_room(ARG_OBJ(0));
	else if(ISARG_ROOM(0)) room = ARG_ROOM(0);
	else if(ISARG_TOK(0)) room = token_room(ARG_TOK(0));
	else return FALSE;

	*ret = (room && room->wilds)?room->x:-1;
	return TRUE;
}


DECL_IFC_FUN(ifc_mapy)
{
	if(ISARG_MOB(0)) room = ARG_MOB(0)->in_room;
	else if(ISARG_OBJ(0)) room = obj_room(ARG_OBJ(0));
	else if(ISARG_ROOM(0)) room = ARG_ROOM(0);
	else if(ISARG_TOK(0)) room = token_room(ARG_TOK(0));
	else return FALSE;

	*ret = (room && room->wilds)?room->y:-1;
	return TRUE;
}


DECL_IFC_FUN(ifc_areahasland)
{
	if(ISARG_MOB(0)) room = ARG_MOB(0)->in_room;
	else if(ISARG_OBJ(0)) room = obj_room(ARG_OBJ(0));
	else if(ISARG_ROOM(0)) room = ARG_ROOM(0);
	else if(ISARG_TOK(0)) room = token_room(ARG_TOK(0));
	else return FALSE;

	*ret = room && room->area->land_x >= 0 && room->area->land_y >= 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_areahasxy)
{
	if(ISARG_MOB(0)) room = ARG_MOB(0)->in_room;
	else if(ISARG_OBJ(0)) room = obj_room(ARG_OBJ(0));
	else if(ISARG_ROOM(0)) room = ARG_ROOM(0);
	else if(ISARG_TOK(0)) room = token_room(ARG_TOK(0));
	else return FALSE;

	*ret = room && room->area->x >= 0 && room->area->y >= 0;
	return TRUE;
}


DECL_IFC_FUN(ifc_arealandx)
{
	if(ISARG_MOB(0)) room = ARG_MOB(0)->in_room;
	else if(ISARG_OBJ(0)) room = obj_room(ARG_OBJ(0));
	else if(ISARG_ROOM(0)) room = ARG_ROOM(0);
	else if(ISARG_TOK(0)) room = token_room(ARG_TOK(0));
	else return FALSE;

	*ret = room ? room->area->land_x : -1;
	return TRUE;
}


DECL_IFC_FUN(ifc_arealandy)
{
	if(ISARG_MOB(0)) room = ARG_MOB(0)->in_room;
	else if(ISARG_OBJ(0)) room = obj_room(ARG_OBJ(0));
	else if(ISARG_ROOM(0)) room = ARG_ROOM(0);
	else if(ISARG_TOK(0)) room = token_room(ARG_TOK(0));
	else return FALSE;

	*ret = room ? room->area->land_y : -1;
	return TRUE;
}

DECL_IFC_FUN(ifc_areax)
{
	if(ISARG_MOB(0)) room = ARG_MOB(0)->in_room;
	else if(ISARG_OBJ(0)) room = obj_room(ARG_OBJ(0));
	else if(ISARG_ROOM(0)) room = ARG_ROOM(0);
	else if(ISARG_TOK(0)) room = token_room(ARG_TOK(0));
	else return FALSE;

	*ret = room ? room->area->x : -1;
	return TRUE;
}


DECL_IFC_FUN(ifc_areay)
{
	if(ISARG_MOB(0)) room = ARG_MOB(0)->in_room;
	else if(ISARG_OBJ(0)) room = obj_room(ARG_OBJ(0));
	else if(ISARG_ROOM(0)) room = ARG_ROOM(0);
	else if(ISARG_TOK(0)) room = token_room(ARG_TOK(0));
	else return FALSE;

	*ret = room ? room->area->y : -1;
	return TRUE;
}


DECL_IFC_FUN(ifc_mapvalid)
{
	// Wilderness format
	if(ISARG_NUM(0) && ISARG_NUM(1) && ISARG_NUM(2)) {
		WILDS_DATA *pWilds;

		if(!(pWilds = get_wilds_from_uid(NULL,ARG_NUM(0))))
			*ret = FALSE;
		else if (ARG_NUM(1) > (pWilds->map_size_x - 1) || ARG_NUM(2) > (pWilds->map_size_y - 1))
			*ret = FALSE;
		else
			*ret = check_for_bad_room(pWilds, ARG_NUM(1), ARG_NUM(2));

	}
	return TRUE;
}


DECL_IFC_FUN(ifc_mapwidth)
{
	// Wilderness format
	if(ISARG_NUM(0)) {
		WILDS_DATA *pWilds;

		if(!(pWilds = get_wilds_from_uid(NULL,ARG_NUM(0))))
			*ret = -1;
		else
			*ret = pWilds->map_size_x;
	} else
		*ret = 0;
	return TRUE;
}


DECL_IFC_FUN(ifc_mapheight)
{
	// Wilderness format
	if(ISARG_NUM(0)) {
		WILDS_DATA *pWilds;

		if(!(pWilds = get_wilds_from_uid(NULL,ARG_NUM(0))))
			*ret = -1;
		else
			*ret = pWilds->map_size_y;
	} else
		*ret = 0;
	return TRUE;
}


DECL_IFC_FUN(ifc_maparea)
{
	// Wilderness format
	if(ISARG_NUM(0) && ISARG_NUM(1) && ISARG_NUM(2)) {
		WILDS_DATA *pWilds;

		if(!(pWilds = get_wilds_from_uid(NULL,ARG_NUM(0))))
			*ret = -1;
		else
			*ret = pWilds->pArea->uid;
	}
	return TRUE;
}


DECL_IFC_FUN(ifc_hasvlink)
{
	*ret = FALSE;
	return TRUE;
}


// hascatalyst $mobile string string number
DECL_IFC_FUN(ifc_hascatalyst)
{
	if(ISARG_MOB(0)) { mob = ARG_MOB(0); room = NULL; }
	else if(ISARG_ROOM(0)) { mob = NULL; room = ARG_ROOM(0); }
	else return FALSE;
	*ret = ((mob || room) && ISARG_STR(1) && ISARG_STR(2) && ISARG_NUM(3)) ? has_catalyst(mob,room,flag_value(catalyst_types,ARG_STR(1)),flag_value(catalyst_method_types,ARG_STR(2)),ARG_NUM(3),ISARG_NUM(4)?ARG_NUM(4):CATALYST_MAXSTRENGTH) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_hitdamage)
{
	*ret = ISARG_MOB(0)?ARG_MOB(0)->hit_damage:0;
	return TRUE;
}

DECL_IFC_FUN(ifc_hitdamtype)
{
	*ret = ISARG_MOB(0)?(ARG_MOB(0)->hit_type == (attack_lookup(ARG_STR(1))+TYPE_HIT)):FALSE;
	return TRUE;
}

DECL_IFC_FUN(ifc_hitskilltype)
{
	*ret = ISARG_MOB(0)?(ARG_MOB(0)->hit_type == skill_lookup(ARG_STR(1))):FALSE;
	return TRUE;
}

DECL_IFC_FUN(ifc_hitdamclass)
{
	*ret = ISARG_MOB(0)?(ARG_MOB(0)->hit_class == damage_class_lookup(ARG_STR(1))):FALSE;
	return TRUE;
}

DECL_IFC_FUN(ifc_systemtime)
{
	*ret = time(NULL) + (ISARG_NUM(0)?ARG_NUM(0):0);
	return TRUE;
}

DECL_IFC_FUN(ifc_hassubclass)
{
	int sub;

	if(ISARG_MOB(0) && ISARG_STR(1) && !IS_NPC(ARG_MOB(0))) {
		sub = sub_class_search(ARG_STR(1));
		if(sub < 0)
			*ret = FALSE;
		else {
			if(sub_class_table[sub].remort)
				*ret = ((get_profession(ARG_MOB(0),sub_class_table[sub].class + SECOND_CLASS_MAGE)) == sub) ? TRUE : FALSE;
			else
				*ret = ((get_profession(ARG_MOB(0),sub_class_table[sub].class + SUBCLASS_MAGE)) == sub) ? TRUE : FALSE;
		}

	} else
		*ret = FALSE;

	return TRUE;
}

DECL_IFC_FUN(ifc_issubclass)
{
	int sub;

	if(ISARG_MOB(0) && ISARG_STR(1) && !IS_NPC(ARG_MOB(0))) {
		sub = sub_class_search(ARG_STR(1));
		if(sub < 0)
			*ret = FALSE;
		else
			*ret = ((get_profession(ARG_MOB(0),SUBCLASS_CURRENT)) == sub) ? TRUE : FALSE;

	} else
		*ret = FALSE;

	return TRUE;
}

DECL_IFC_FUN(ifc_objfrag)
{
	if(ISARG_OBJ(0) && ISARG_STR(1)) {
		if(!str_prefix(ARG_STR(1),"solid")) *ret = (ARG_OBJ(0)->fragility == OBJ_FRAGILE_SOLID) ? TRUE : FALSE;
		else if(!str_prefix(ARG_STR(1),"strong")) *ret = (ARG_OBJ(0)->fragility == OBJ_FRAGILE_STRONG) ? TRUE : FALSE;
		else if(!str_prefix(ARG_STR(1),"normal")) *ret = (ARG_OBJ(0)->fragility == OBJ_FRAGILE_NORMAL) ? TRUE : FALSE;
		else if(!str_prefix(ARG_STR(1),"weak")) *ret = (ARG_OBJ(0)->fragility == OBJ_FRAGILE_WEAK) ? TRUE : FALSE;
		else *ret = FALSE;
	} else
		*ret = FALSE;
	return TRUE;
}

DECL_IFC_FUN(ifc_tempstore1)
{
	*ret = ISARG_MOB(0) ? ARG_MOB(0)->tempstore[0] : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_tempstore2)
{
	*ret = ISARG_MOB(0) ? ARG_MOB(0)->tempstore[1] : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_tempstore3)
{
	*ret = ISARG_MOB(0) ? ARG_MOB(0)->tempstore[2] : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_tempstore4)
{
	*ret = ISARG_MOB(0) ? ARG_MOB(0)->tempstore[3] : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_strlen)
{
	*ret = ISARG_STR(0) ? strlen(ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_lostparts)
{
	*ret = (ISARG_MOB(0) && ISARG_STR(1) && IS_SET(ARG_MOB(0)->lostparts, flag_value(part_flags,ARG_STR(1))));
	return TRUE;
}

DECL_IFC_FUN(ifc_hasprompt)
{
	*ret = (ISARG_MOB(0) && !IS_NPC(ARG_MOB(0)) && ISARG_STR(1) && string_vector_find(ARG_MOB(0)->pcdata->script_prompts,ARG_STR(1))) ? TRUE : FALSE ;
	return TRUE;
}

DECL_IFC_FUN(ifc_numenchants)
{
	*ret = ISARG_OBJ(0) ? ARG_OBJ(0)->num_enchanted : -1;
	return TRUE;
}

DECL_IFC_FUN(ifc_randpoint)
{
#define MAX_RAND_PT	5
	int x[MAX_RAND_PT];
	int y[MAX_RAND_PT];
	int r,n,i;

	if(!(argc&1)) return FALSE;

	n = argc/2;

	if(n < 4 || n > MAX_RAND_PT) return FALSE;

	for(r=0;r < argc; r++) if(!ISARG_NUM(r)) return FALSE;

	for(r=0;r < n; r++) {
		x[r] = ARG_NUM(2*r);
		y[r] = ARG_NUM(2*r+1);
	}

	for(r=0;r < (n-1);r++)
		for(i=r+1;i < n;i++)
			if(x[r] >= x[i]) return FALSE;

	r = number_range(x[0],x[n-1]);
	for(i = n-1; i-- > 0;) {
		if(r > x[i]) {
			*ret = (y[i+1] - y[i]) * (r - x[i]) / (x[i+1] - x[i]) + y[i];
			return TRUE;
		}
	}

	*ret = y[0];
	return TRUE;
}

DECL_IFC_FUN(ifc_issafe)
{
	if(ISARG_MOB(0)) {
		if(ISARG_MOB(1)) {
			*ret = is_safe(ARG_MOB(0),ARG_MOB(1),FALSE) ? TRUE : FALSE;
			return TRUE;
		}

		if(mob) {
			*ret = is_safe(mob,ARG_MOB(0),FALSE) ? TRUE : FALSE;
			return TRUE;
		}
	}
	return FALSE;
}

DECL_IFC_FUN(ifc_roomx)
{
	if(ISARG_MOB(0)) room = ARG_MOB(0)->in_room;
	else if(ISARG_OBJ(0)) room = obj_room(ARG_OBJ(0));
	else if(ISARG_ROOM(0)) room = ARG_ROOM(0);
	else if(ISARG_TOK(0)) room = token_room(ARG_TOK(0));
	else return FALSE;

	if(!room) return FALSE;

	*ret = room->x;
	return TRUE;
}

DECL_IFC_FUN(ifc_roomy)
{
	if(ISARG_MOB(0)) room = ARG_MOB(0)->in_room;
	else if(ISARG_OBJ(0)) room = obj_room(ARG_OBJ(0));
	else if(ISARG_ROOM(0)) room = ARG_ROOM(0);
	else if(ISARG_TOK(0)) room = token_room(ARG_TOK(0));
	else return FALSE;

	if(!room) return FALSE;

	*ret = room->y;
	return TRUE;
}

DECL_IFC_FUN(ifc_roomz)
{
	if(ISARG_MOB(0)) room = ARG_MOB(0)->in_room;
	else if(ISARG_OBJ(0)) room = obj_room(ARG_OBJ(0));
	else if(ISARG_ROOM(0)) room = ARG_ROOM(0);
	else if(ISARG_TOK(0)) room = token_room(ARG_TOK(0));
	else return FALSE;

	if(!room) return FALSE;

	*ret = room->z;
	return TRUE;
}

DECL_IFC_FUN(ifc_roomwilds)
{
	if(ISARG_MOB(0)) room = ARG_MOB(0)->in_room;
	else if(ISARG_OBJ(0)) room = obj_room(ARG_OBJ(0));
	else if(ISARG_ROOM(0)) room = ARG_ROOM(0);
	else if(ISARG_TOK(0)) room = token_room(ARG_TOK(0));
	else return FALSE;

	if(!room) return FALSE;

	*ret = room->wilds ? room->wilds->uid : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_roomviewwilds)
{
	if(ISARG_MOB(0)) room = ARG_MOB(0)->in_room;
	else if(ISARG_OBJ(0)) room = obj_room(ARG_OBJ(0));
	else if(ISARG_ROOM(0)) room = ARG_ROOM(0);
	else if(ISARG_TOK(0)) room = token_room(ARG_TOK(0));
	else return FALSE;

	if(!room) return FALSE;

	*ret = room->viewwilds ? room->viewwilds->uid : 0;
	return TRUE;
}

// isleader			if $i is its leader
// isleader <mobile>		if <mobile> is $i's leader
// isleader <mobile> <leader>	if <leader> is <mobile>'s leader
DECL_IFC_FUN(ifc_isleader)
{
	if(ISARG_MOB(0)) {
		if(ISARG_MOB(1))
			*ret = (ARG_MOB(0)->leader && (ARG_MOB(1) == ARG_MOB(0)->leader)) ? TRUE : FALSE;
		else
			*ret = (mob->leader && (ARG_MOB(0) == mob->leader)) ? TRUE : FALSE;
	} else if(mob) {
		*ret = (mob == mob->leader) ? TRUE : FALSE;
	} else
		return FALSE;


	return TRUE;
}

DECL_IFC_FUN(ifc_samegroup)
{
	if(ISARG_MOB(0)) {
		if(ISARG_MOB(1))
			*ret = is_same_group(ARG_MOB(0),ARG_MOB(1)) ? TRUE : FALSE;
		else
			*ret = is_same_group(ARG_MOB(0),mob) ? TRUE : FALSE;
	} else
		return FALSE;

	return TRUE;
}

DECL_IFC_FUN(ifc_testhardmagic)
{
	int chance;

	if(ISARG_MOB(0)) { mob = ARG_MOB(0); ++argv; --argc; }

	if(!mob || !mob->in_room) return FALSE;

	chance = 0;
	if (IS_SET(mob->in_room->room2_flags, ROOM_HARD_MAGIC)) chance += 2;
	if (mob->in_room->sector_type == SECT_CURSED_SANCTUM) chance += 2;
	if(!IS_NPC(mob) && chance > 0 && number_range(1,chance) > 1) {
		*ret = TRUE;
	} else
		*ret = FALSE;

	return TRUE;
}

DECL_IFC_FUN(ifc_testslowmagic)
{
	if(ISARG_MOB(0)) { mob = ARG_MOB(0); ++argv; --argc; }

	if(!mob || !mob->in_room) return FALSE;

	*ret = IS_SET(mob->in_room->room2_flags,ROOM_SLOW_MAGIC) || (mob->in_room->sector_type == SECT_CURSED_SANCTUM);

	return TRUE;
}

// This mimics the built in code sans the messages and skill improves.
//   That will be up to the script to determine that
DECL_IFC_FUN(ifc_testtokenspell)
{
	SHIFT_MOB();

	if(ISARG_TOK(0)) {
		if(!ARG_TOK(0)->pIndexData->value[TOKVAL_SPELL_RATING])
			*ret = (number_percent() < ARG_TOK(0)->value[TOKVAL_SPELL_RATING]);
		else
			*ret = (number_range(0,ARG_TOK(0)->pIndexData->value[TOKVAL_SPELL_RATING]-1) < ARG_TOK(0)->value[TOKVAL_SPELL_RATING]);
		return TRUE;
	}

	return FALSE;
}


DECL_IFC_FUN(ifc_isspell)
{
	if(ISARG_MOB(0) && ISARG_NUM(1)) {
		token = get_token_char(ARG_MOB(0), ARG_NUM(1), (ISARG_NUM(2) ? ARG_NUM(2) : 1));
		*ret = token ? (token->pIndexData->type == TOKEN_SPELL) : FALSE;
	} else if(ISARG_TOK(0)) {
		*ret = (ARG_TOK(0)->pIndexData->type == TOKEN_SPELL);
	} else {
		int sn;

		if(ISARG_STR(0))
			sn = skill_lookup(ARG_STR(0));
		else if(ISARG_SKILL(0))
			sn = ARG_SKILL(0);
		else if(ISARG_SKINFO(0))
			sn = ARG_SKINFO(0).m ? ARG_SKINFO(0).sn : -1;
		else
			return FALSE;

		*ret = sn >= 0 && sn < MAX_SKILL && skill_table[sn].spell_fun && skill_table[sn].spell_fun != spell_null;
	}

	return TRUE;
}

DECL_IFC_FUN(ifc_hasenvironment)
{
	if(ISARG_MOB(0)) room = ARG_MOB(0)->in_room;
	else if(ISARG_OBJ(0)) room = obj_room(ARG_OBJ(0));
	else if(ISARG_ROOM(0)) room = ARG_ROOM(0);
	else if(ISARG_TOK(0)) room = token_room(ARG_TOK(0));
	else if(mob) room = mob->in_room;
	else if(obj) room = obj_room(obj);
	// if room is set because this is from an rprog, duh!
	else if(token) room = token_room(token);

	if(!room) return FALSE;

	*ret = IS_SET(room->room2_flags,ROOM_VIRTUAL_ROOM) && (room->environ_type != ENVIRON_NONE);
	return TRUE;
}

DECL_IFC_FUN(ifc_iscloneroom)
{
	if(ISARG_MOB(0)) room = ARG_MOB(0)->in_room;
	else if(ISARG_OBJ(0)) room = obj_room(ARG_OBJ(0));
	else if(ISARG_ROOM(0)) room = ARG_ROOM(0);
	else if(ISARG_TOK(0)) room = token_room(ARG_TOK(0));
	else if(mob) room = mob->in_room;
	else if(obj) room = obj_room(obj);
	// if room is set because this is from an rprog, duh!
	else if(token) room = token_room(token);

	if(!room) return FALSE;

	*ret = room_is_clone(room);
	return TRUE;
}

DECL_IFC_FUN(ifc_scriptsecurity)
{
	*ret = script_security;
	return TRUE;
}

DECL_IFC_FUN(ifc_bankbalance)
{
	*ret = VALID_PLAYER(0) ? ARG_MOB(0)->pcdata->bankbalance : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_grouphit)
{
	CHAR_DATA *rch;
	int sum = 0;

	SHIFT_MOB();
	*ret = 0;

	if(!mob || !mob->in_room) return FALSE;	// Since all stats have a minimum, this would be an ERROR

	for(rch = mob->in_room->people;rch; rch = rch->next_in_room)
		if(mob == rch || is_same_group(mob,rch))
			sum += rch->hit;

	*ret = sum;
	return TRUE;
}

DECL_IFC_FUN(ifc_groupmana)
{
	CHAR_DATA *rch;
	int sum = 0;

	SHIFT_MOB();
	*ret = 0;

	if(!mob || !mob->in_room) return FALSE;

	for(rch = mob->in_room->people;rch; rch = rch->next_in_room)
		if(mob == rch || is_same_group(mob,rch))
			sum += rch->mana;

	*ret = sum;
	return TRUE;
}

DECL_IFC_FUN(ifc_groupmove)
{
	CHAR_DATA *rch;
	int sum = 0;

	SHIFT_MOB();
	*ret = 0;

	if(!mob || !mob->in_room) return FALSE;

	for(rch = mob->in_room->people;rch; rch = rch->next_in_room)
		if(mob == rch || is_same_group(mob,rch))
			sum += rch->move;

	*ret = sum;
	return TRUE;
}

DECL_IFC_FUN(ifc_groupmaxhit)
{
	CHAR_DATA *rch;
	int sum = 0;

	SHIFT_MOB();
	*ret = 0;

	if(!mob || !mob->in_room) return FALSE;	// Since all stats have a minimum, this would be an ERROR

	for(rch = mob->in_room->people;rch; rch = rch->next_in_room)
		if(mob == rch || is_same_group(mob,rch))
			sum += rch->max_hit;

	*ret = sum;
	return TRUE;
}

DECL_IFC_FUN(ifc_groupmaxmana)
{
	CHAR_DATA *rch;
	int sum = 0;

	SHIFT_MOB();
	*ret = 0;

	if(!mob || !mob->in_room) return FALSE;

	for(rch = mob->in_room->people;rch; rch = rch->next_in_room)
		if(mob == rch || is_same_group(mob,rch))
			sum += rch->max_mana;

	*ret = sum;
	return TRUE;
}

DECL_IFC_FUN(ifc_groupmaxmove)
{
	CHAR_DATA *rch;
	int sum = 0;

	SHIFT_MOB();
	*ret = 0;

	if(!mob || !mob->in_room) return FALSE;

	for(rch = mob->in_room->people;rch; rch = rch->next_in_room)
		if(mob == rch || is_same_group(mob,rch))
			sum += rch->max_move;

	*ret = sum;
	return TRUE;
}

DECL_IFC_FUN(ifc_manastore)
{
	*ret = ISARG_MOB(0) ? ARG_MOB(0)->manastore : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_arearegion)
{
	*ret = 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_areaflag)
{
	*ret = FALSE;
	return TRUE;
}


DECL_IFC_FUN(ifc_areawho)
{
	*ret = ISARG_AREA(0) && ISARG_STR(1) && (ARG_AREA(0)->area_who == flag_value(area_who_titles,ARG_STR(1)));
	return TRUE;
}

DECL_IFC_FUN(ifc_areaplace)
{
	*ret = ISARG_AREA(0) && ISARG_STR(1) && (ARG_AREA(0)->area_who == flag_value(area_who_titles,ARG_STR(1)));
	return TRUE;
}

DECL_IFC_FUN(ifc_isaffectcustom)
{
	*ret = ARG_AFF(0) && ARG_AFF(0)->custom_name != NULL;
	return TRUE;
}

DECL_IFC_FUN(ifc_affectskill)
{
	*ret = ISARG_AFF(0) ? ARG_AFF(0)->type : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_isaffectskill)
{
	*ret = ISARG_AFF(0) && ISARG_STR(1) && (ARG_AFF(0)->custom_name == NULL) && (ARG_AFF(0)->type == skill_lookup(ARG_STR(1)));
	return TRUE;
}

DECL_IFC_FUN(ifc_affectlocation)
{
	*ret = ISARG_AFF(0) ? ARG_AFF(0)->location : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_affectmodifier)
{
	*ret = ISARG_AFF(0) ? ARG_AFF(0)->modifier : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_affecttimer)
{
	*ret = ISARG_AFF(0) ? ARG_AFF(0)->duration : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_affectgroup)
{
	*ret = ISARG_AFF(0) ? ARG_AFF(0)->group : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_isaffectgroup)
{
	*ret = ISARG_AFF(0) && ISARG_STR(1) && (ARG_AFF(0)->group == flag_value(apply_types,ARG_STR(1)));
	return TRUE;
}

DECL_IFC_FUN(ifc_affectbit)
{
	*ret = ISARG_AFF(0) && ISARG_STR(1) && IS_SET(ARG_AFF(0)->bitvector, flag_value(affect_flags,ARG_STR(1)));
	return TRUE;
}

DECL_IFC_FUN(ifc_affectbit2)
{
	*ret = ISARG_AFF(0) && ISARG_STR(1) && IS_SET(ARG_AFF(0)->bitvector, flag_value(affect2_flags,ARG_STR(1)));
	return TRUE;
}

DECL_IFC_FUN(ifc_skilllookup)
{
	*ret = ISARG_STR(0) ? skill_lookup(ARG_STR(0)) : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_xp)
{
	*ret = ISARG_MOB(0) ? ARG_MOB(0)->exp : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_maxxp)
{
	*ret = ISARG_MOB(0) ? ARG_MOB(0)->maxexp : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_sublevel)
{
	*ret = ISARG_MOB(0) ? ARG_MOB(0)->level : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_mobsize)
{
	*ret = ISARG_MOB(0) ? ARG_MOB(0)->size : 0;
	return TRUE;
}

DECL_IFC_FUN(ifc_handsfull)
{
	*ret = ISARG_MOB(0) ? both_hands_full(ARG_MOB(0)) : FALSE;
	return TRUE;
}

DECL_IFC_FUN(ifc_isremort)
{
	*ret = VALID_PLAYER(0) && IS_REMORT(ARG_MOB(0));
	return TRUE;
}


DECL_IFC_FUN(ifc_findpath)
{
	long start = 0, end = 0;
	int depth = 10, in_zone = 1, thru_doors = 0;

	if( argc < 2 ) {
		*ret = -1;
		return FALSE;
	}

	if(ISARG_NUM(0)) start = ARG_NUM(0);
	else if(ISARG_MOB(0)) start = ARG_MOB(0)->in_room ? ARG_MOB(0)->in_room->vnum : 0;
	else if(ISARG_OBJ(0)) { room = obj_room(ARG_OBJ(0)); start = room ? room->vnum : 0; }
	else if(ISARG_ROOM(0)) start = ARG_ROOM(0)->vnum;
	else if(ISARG_TOK(0)) { room = token_room(ARG_TOK(0)); start = room ? room->vnum : 0; }

	if(ISARG_NUM(1)) end = ARG_NUM(1);
	else if(ISARG_MOB(1)) end = ARG_MOB(1)->in_room ? ARG_MOB(1)->in_room->vnum : 1;
	else if(ISARG_OBJ(1)) { room = obj_room(ARG_OBJ(1)); end = room ? room->vnum : 1; }
	else if(ISARG_ROOM(1)) end = ARG_ROOM(1)->vnum;
	else if(ISARG_TOK(1)) { room = token_room(ARG_TOK(1)); end = room ? room->vnum : 1; }

	if(argc > 2 && ISARG_NUM(2)) depth = ARG_NUM(2);

	if(argc > 3 && ISARG_NUM(3)) in_zone = ARG_NUM(3) && 1;

	if(argc > 4 && ISARG_NUM(4)) thru_doors = ARG_NUM(4) && 1;

	*ret = find_path( start, end, NULL, (thru_doors ? -depth : depth), in_zone);
	return TRUE;
}

// Determines the level of sunlight from 0 to 1000
// Sunlight is 0 inside or in the netherworld
DECL_IFC_FUN(ifc_sunlight)
{
	if(argc > 0) {
		if(ISARG_MOB(0)) room = ARG_MOB(0)->in_room;
		else if(ISARG_OBJ(0)) room = obj_room(ARG_OBJ(0));
		else if(ISARG_ROOM(0)) room = ARG_ROOM(0);
		else if(ISARG_TOK(0)) room = token_room(ARG_TOK(0));
	} else {
		if(mob) room = mob->in_room;
		else if(obj) room = obj_room(obj);
		else if(token) room = token_room(token);
	}

	if (room && (room->wilds || (room->sector_type != SECT_INSIDE && room->sector_type != SECT_NETHERWORLD && !IS_SET(room->room_flags, ROOM_INDOORS)))) {
		*ret = (int)(-1000 * cos(3.1415926 * time_info.hour / 12));
		if(*ret < 0) *ret = 0;
	} else
		*ret = 0;
	return TRUE;
}

// if istreasureroom[ $<mobile|church|name>] $<room|vnum>
DECL_IFC_FUN(ifc_istreasureroom)
{
	CHURCH_DATA *church = NULL;
	ROOM_INDEX_DATA *here = NULL;
	ITERATOR it;

	*ret = FALSE;
	if(ISARG_MOB(0)) church = ARG_MOB(0)->church;
	else if(ISARG_CHURCH(0)) church = ARG_CHURCH(0);
	else if(ISARG_STR(0)) church = find_church_name(ARG_STR(0));
	else {
		if(ISARG_ROOM(0)) here = ARG_ROOM(0);
		else if(ISARG_NUM(0)) here = get_room_index(ARG_NUM(0));

		if(here) {
			iterator_start(&it, list_churches);
			while( (church = (CHURCH_DATA *)iterator_nextdata(&it)) ) {
				if( is_treasure_room(church, here) ) {
					*ret = TRUE;
					break;
				}
			}
			iterator_stop(&it);
		}

		return TRUE;
	}

	if(church) {
		if(ISARG_ROOM(1)) here = ARG_ROOM(1);
		else if(ISARG_NUM(1)) here = get_room_index(ARG_NUM(1));

		*ret = here ? is_treasure_room(church, here) : FALSE;
	}

	return TRUE;
}

// if churchhasrelic $<mobile|church|name> $<string=relic>
DECL_IFC_FUN(ifc_churchhasrelic)
{
	CHURCH_DATA *church = NULL;

	*ret = FALSE;
	if(ISARG_MOB(0)) church = ARG_MOB(0)->church;
	else if(ISARG_CHURCH(0)) church = ARG_CHURCH(0);
	else if(ISARG_STR(0)) church = find_church_name(ARG_STR(0));

	if(church) {
		OBJ_DATA *relic = NULL;
		if(ISARG_STR(1)) {
			if(!str_cmp(ARG_STR(1),"xp")) relic = xp_relic;
			else if(!str_cmp(ARG_STR(1),"damage")) relic = damage_relic;
			else if(!str_cmp(ARG_STR(1),"pneuma")) relic = pneuma_relic;
			else if(!str_cmp(ARG_STR(1),"health")) relic = hp_regen_relic;
			else if(!str_cmp(ARG_STR(1),"mana")) relic = mana_regen_relic;
		}

		if( relic && relic->in_room ) {
			ITERATOR it;
			ROOM_INDEX_DATA *troom;

			iterator_start(&it,church->treasure_rooms);
			while( (troom = (ROOM_INDEX_DATA *)iterator_nextdata(&it)) ) {
				if( relic->in_room == troom ) {
					*ret = TRUE;
					break;
				}
			}
			iterator_stop(&it);
		}
	}

	return TRUE;
}

// if ischurchpk $<mobile|church|name>
DECL_IFC_FUN(ifc_ischurchpk)
{
	CHURCH_DATA *church = NULL;

	if(ISARG_MOB(0)) church = ARG_MOB(0)->church;
	else if(ISARG_CHURCH(0)) church = ARG_CHURCH(0);
	else if(ISARG_STR(0)) church = find_church_name(ARG_STR(0));

	*ret = church && church->pk;

	return TRUE;
}

// if iscrosszone $<mobile|church|name>
DECL_IFC_FUN(ifc_iscrosszone)
{
	CHURCH_DATA *church = NULL;

	if(ISARG_MOB(0)) church = ARG_MOB(0)->church;
	else if(ISARG_CHURCH(0)) church = ARG_CHURCH(0);
	else if(ISARG_STR(0)) church = find_church_name(ARG_STR(0));

	if(church) *ret = (int)IS_SET(church->settings,CHURCH_ALLOW_CROSSZONES);
	else *ret = FALSE;
	return TRUE;
}

// if isinchurch $<mobile|church|name>[ $<mobile|name>]
DECL_IFC_FUN(ifc_inchurch)
{
	CHURCH_DATA *church = NULL;

	*ret = FALSE;

	if(ISARG_MOB(0)) church = ARG_MOB(0)->church;
	else if(ISARG_CHURCH(0)) church = ARG_CHURCH(0);
	else if(ISARG_STR(0)) church = find_church_name(ARG_STR(0));

	if(church) {
		if(ISARG_MOB(1)) mob = ARG_MOB(1);
		else if(ISARG_STR(1)) mob = get_player(ARG_STR(1));

		if(mob && mob->church == church)
			*ret = TRUE;
	}

	return TRUE;
}

// if ispersist $<mobile|object|room>
DECL_IFC_FUN(ifc_ispersist)
{
	if(ISARG_MOB(0)) *ret = IS_NPC(ARG_MOB(0)) ? ARG_MOB(0)->persist : TRUE;	// Players are persistant by nature
	else if(ISARG_OBJ(0)) *ret = ARG_OBJ(0)->persist;
	else if(ISARG_ROOM(0)) *ret = ARG_ROOM(0)->persist;
	else *ret = FALSE;

	return TRUE;
}

// if listcontains $<list> $<element>
DECL_IFC_FUN(ifc_listcontains)
{
	ITERATOR it;
	LLIST_UID_DATA *luid;
	LLIST_ROOM_DATA *lroom;
	ROOM_INDEX_DATA *proom;
	CHAR_DATA *pmob;
	OBJ_DATA *pobj;
	TOKEN_DATA *ptoken;

	*ret = FALSE;
	if(ISARG_BLLIST_ROOM(0) && ISARG_ROOM(1)) {
		iterator_start(&it,ARG_BLIST(0));
		while( (lroom = (LLIST_ROOM_DATA *)iterator_nextdata(&it)) ) {
			if( lroom->room == ARG_ROOM(1) ) {
				*ret = TRUE;
				break;
			}
		}
		iterator_stop(&it);
	} else if(ISARG_PLLIST_ROOM(0) && ISARG_ROOM(1)) {
		iterator_start(&it,ARG_BLIST(0));
		while( (proom = (ROOM_INDEX_DATA *)iterator_nextdata(&it)) ) {
			if( proom == ARG_ROOM(1) ) {
				*ret = TRUE;
				break;
			}
		}
		iterator_stop(&it);
	} else if(ISARG_BLLIST_MOB(0) && ISARG_MOB(1)) {
		iterator_start(&it,ARG_BLIST(0));
		while( (luid = (LLIST_UID_DATA *)iterator_nextdata(&it)) ) {
			if( (CHAR_DATA*)(luid->ptr) == ARG_MOB(1) ) {
				*ret = TRUE;
				break;
			}
		}
		iterator_stop(&it);
	} else if(ISARG_PLLIST_MOB(0) && ISARG_MOB(1)) {
		iterator_start(&it,ARG_BLIST(0));
		while( (pmob = (CHAR_DATA *)iterator_nextdata(&it)) ) {
			if( pmob == ARG_MOB(1) ) {
				*ret = TRUE;
				break;
			}
		}
		iterator_stop(&it);
	} else if(ISARG_BLLIST_OBJ(0) && ISARG_OBJ(1)) {
		iterator_start(&it,ARG_BLIST(0));
		while( (luid = (LLIST_UID_DATA *)iterator_nextdata(&it)) ) {
			if( (OBJ_DATA*)(luid->ptr) == ARG_OBJ(1) ) {
				*ret = TRUE;
				break;
			}
		}
		iterator_stop(&it);
	} else if(ISARG_PLLIST_OBJ(0) && ISARG_OBJ(1)) {
		iterator_start(&it,ARG_BLIST(0));
		while( (pobj = (OBJ_DATA *)iterator_nextdata(&it)) ) {
			if( pobj == ARG_OBJ(1) ) {
				*ret = TRUE;
				break;
			}
		}
		iterator_stop(&it);
	} else if(ISARG_BLLIST_TOK(0) && ISARG_TOK(1)) {
		iterator_start(&it,ARG_BLIST(0));
		while( (luid = (LLIST_UID_DATA *)iterator_nextdata(&it)) ) {
			if( (TOKEN_DATA*)(luid->ptr) == ARG_TOK(1) ) {
				*ret = TRUE;
				break;
			}
		}
		iterator_stop(&it);
	} else if(ISARG_PLLIST_TOK(0) && ISARG_TOK(1)) {
		iterator_start(&it,ARG_BLIST(0));
		while( (ptoken = (TOKEN_DATA *)iterator_nextdata(&it)) ) {
			if( ptoken == ARG_TOK(1) ) {
				*ret = TRUE;
				break;
			}
		}
		iterator_stop(&it);
	}

	return TRUE;
}

// if ispk $<player>
// if ispk $<room>[ <boolean=arena>]
DECL_IFC_FUN(ifc_ispk)
{
	if(VALID_PLAYER(0)) *ret = is_pk(ARG_MOB(0));
	else if(ISARG_ROOM(0)) *ret = is_room_pk(ARG_ROOM(0),ARG_BOOL(1));
	else *ret = FALSE;

	return TRUE;
}

// if register <number>
DECL_IFC_FUN(ifc_register)
{

	if(ISARG_NUM(0)) {
		int r = ARG_NUM(0);

		if(r >= 0 && r < 5)
		{
			*ret = info->registers[r];
			return TRUE;
		}
	}

	*ret = 0;
	return FALSE;
}

// if comm $<player> <flags>
DECL_IFC_FUN(ifc_comm)
{
	*ret = VALID_PLAYER(0) && ISARG_STR(1) && IS_SET(ARG_MOB(0)->comm, flag_value(comm_flags, ARG_STR(1)));
	return TRUE;
}

// if flagcomm <flags> == <value>
// Usually used for $[[flagcomm <flags>]]
DECL_IFC_FUN(ifc_flag_comm)
{
	*ret = ISARG_STR(0) ? flag_value(comm_flags,ARG_STR(0)) : 0;
	return TRUE;
}

// if min <number> <number>
// returns the minimum of the two numbers
DECL_IFC_FUN(ifc_min)
{
	if(ISARG_NUM(0))
	{
		if( ISARG_NUM(1) && (ARG_NUM(1) < ARG_NUM(0)) )
			*ret = ARG_NUM(1);
		else
			*ret = ARG_NUM(0);
		return TRUE;
	}

	return FALSE;
}

// if max <number> <number>
// returns the maximum of the two numbers
DECL_IFC_FUN(ifc_max)
{
	if(ISARG_NUM(0))
	{
		if( ISARG_NUM(1) && (ARG_NUM(1) > ARG_NUM(0)) )
			*ret = ARG_NUM(1);
		else
			*ret = ARG_NUM(0);
		return TRUE;
	}

	return FALSE;
}

// if candrop $mobile $object[ boolean[ boolean]]
// boolean 1: check keep
DECL_IFC_FUN(ifc_candrop)
{
	OBJ_DATA *o;
	bool keep = FALSE;
	bool silent = FALSE;
	if(!ISARG_MOB(0)) return FALSE;
	if(!ISARG_OBJ(1)) return FALSE;

	if(ISARG_NUM(2) || ISARG_STR(2))
		keep = ARG_BOOL(2);

	if(ISARG_NUM(3) || ISARG_STR(3))
		silent = ARG_BOOL(3);

	o = ARG_OBJ(1);

	*ret = can_drop_obj(ARG_MOB(0), o, silent) && !(keep && IS_SET(o->extra2_flags, ITEM_KEPT));
	return TRUE;
}

// if canget $mobile $object[ $container] [boolean]
DECL_IFC_FUN(ifc_canget)
{
	CHAR_DATA *ch;
	OBJ_DATA *o, *c = NULL;
	bool silent;

	if(!ISARG_MOB(0)) return FALSE;
	if(!ISARG_OBJ(1)) return FALSE;

	ch = ARG_MOB(0);
	o = ARG_OBJ(1);

	if(ISARG_OBJ(2)) {
		c = ARG_OBJ(2);
		++argv;
		--argc;
	}

	silent = ARG_BOOL(2);

	*ret = can_get_obj(ch, o, c, NULL, silent);
	return TRUE;
}

// if canput $mobile $object $container [boolean]
DECL_IFC_FUN(ifc_canput)
{
	bool silent;

	if(!ISARG_MOB(0)) return FALSE;
	if(!ISARG_OBJ(1)) return FALSE;
	if(!ISARG_OBJ(2)) return FALSE;

	*ret = can_put_obj(ARG_MOB(0), ARG_OBJ(1), ARG_OBJ(2), NULL, ARG_BOOL(3));
	return TRUE;
}

// if objcorpse $OBJECT STRING
// where OBJECT is a corpse
DECL_IFC_FUN(ifc_objcorpse)
{
	*ret = ISARG_OBJ(0) && ISARG_STR(1) &&
		(ARG_OBJ(0)->item_type == ITEM_CORPSE_NPC || ARG_OBJ(0)->item_type == ITEM_CORPSE_PC) &&
		IS_SET(CORPSE_FLAGS(ARG_OBJ(0)), flag_value(corpse_object_flags,ARG_STR(1)));
	return TRUE;
}

DECL_IFC_FUN(ifc_flag_corpse)
{
	*ret = ISARG_STR(0) ? flag_value(corpse_object_flags,ARG_STR(0)) : 0;

	return TRUE;
}

// if objweapon $WEAPON TYPE (eg 'sword')
// if objweapon $WEAPONCONTAINER TYPE (eg 'sword')
DECL_IFC_FUN(ifc_objweapon)
{
	*ret = FALSE;
	if(ISARG_OBJ(0) && ISARG_STR(1)) {
		if(ARG_OBJ(0)->item_type == ITEM_WEAPON)
			*ret = ARG_OBJ(0)->value[0] == flag_value(weapon_class,ARG_STR(1));
		else if(ARG_OBJ(0)->item_type == ITEM_WEAPON_CONTAINER)
			*ret = ARG_OBJ(0)->value[1] == flag_value(weapon_class,ARG_STR(1));
	}

	return TRUE;
}

DECL_IFC_FUN(ifc_objweaponstat)
{
	*ret = ISARG_OBJ(0) && ISARG_STR(1) &&
		(ARG_OBJ(0)->item_type == ITEM_WEAPON) &&
		IS_SET(ARG_OBJ(0)->value[4],flag_value(weapon_type2,ARG_STR(1)));
	return TRUE;
}

DECL_IFC_FUN(ifc_objranged)
{
	*ret = ISARG_OBJ(0) && ISARG_STR(1) &&
		(ARG_OBJ(0)->item_type == ITEM_RANGED_WEAPON) &&
		(ARG_OBJ(0)->value[0] == flag_value(ranged_weapon_class,ARG_STR(1)));
	return TRUE;
}

DECL_IFC_FUN(ifc_isbusy)
{
	*ret = ISARG_MOB(0) && is_char_busy(ARG_MOB(0));
	return TRUE;
}

DECL_IFC_FUN(ifc_iscastsuccess)
{
	*ret = ISARG_MOB(0) && (ARG_MOB(0)->cast_successful == MAGICCAST_SUCCESS);
	return TRUE;
}

DECL_IFC_FUN(ifc_iscastfailure)
{
	*ret = ISARG_MOB(0) && (ARG_MOB(0)->cast_successful == MAGICCAST_FAILURE);
	return TRUE;
}

DECL_IFC_FUN(ifc_iscastroomblocked)
{
	*ret = ISARG_MOB(0) && (ARG_MOB(0)->cast_successful == MAGICCAST_ROOMBLOCK);
	return TRUE;
}

DECL_IFC_FUN(ifc_iscastrecovered)
{
	*ret = ISARG_MOB(0) && ARG_MOB(0)->casting_recovered;
	return TRUE;
}

