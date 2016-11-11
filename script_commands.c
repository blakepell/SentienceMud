#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <ctype.h>
#include "merc.h"
#include "db.h"
#include "recycle.h"
#include "tables.h"
#include "scripts.h"

//#define DEBUG_MODULE
#include "debug.h"

// AWARD mobile string(type) number(amount)
// Types: silver, gold, pneuma, deity/dp, practice, train, quest/qp, experience/xp
//
// AWARD church string(type) number(amount)
// Types: gold, pneuma, deity/dp
//
SCRIPT_CMD(scriptcmd_award)
{
	char buf[MSL], *rest;
	char field[MIL];
	char *field_name;
	CHAR_DATA *victim = NULL;
	CHURCH_DATA *church = NULL;
	int amount = 0;
	SCRIPT_PARAM arg;


	if (!(rest = expand_argument(info,argument,&arg)))
		return;

	switch(arg.type) {
	case ENT_CHURCH: church = arg.d.church; break;
	case ENT_STRING: victim = script_get_char_room(info, arg.d.str, TRUE); break;
	case ENT_MOBILE: victim = arg.d.mob; break;
	default: victim = NULL; break;
	}

	if (!victim && !church) return;

	if (!(rest = expand_argument(info,rest,&arg)))
		return;

	if( arg.type != ENT_STRING ) return;
	strncpy(field,arg.d.str,MIL-1);

	if (!(rest = expand_argument(info,rest,&arg)))
		return;

	switch(arg.type) {
	case ENT_STRING: amount = atoi(arg.d.str); break;
	case ENT_NUMBER: amount = arg.d.num; break;
	default: amount = 0; break;
	}

	if(amount < 1) return;

	if( church ) {
		if( !str_prefix(field, "gold") ) {
			church->gold += amount;
			field_name = "gold";

		} else if( !str_prefix(field, "pneuma") ) {
			church->pneuma += amount;
			field_name = "pneuma";

		} else if( !str_prefix(field, "deity") || !str_cmp(field, "dp") ) {
			church->dp += amount;
			field_name = "deity points";

		} else
			return;


		sprintf(buf, "Award logged: Church %s was awarded %d %s", church->name, amount, field_name);
		log_string(buf);

	} else {

		if( !str_prefix(field, "silver") ) {
			victim->silver += amount;
			field_name = "silver";

		} else if( !str_prefix(field, "gold") ) {
			victim->gold += amount;
			field_name = "gold";

		} else if( !str_prefix(field, "pneuma") ) {
			victim->pneuma += amount;
			field_name = "pneuma";

		} else if( !str_prefix(field, "deity") || !str_cmp(field, "dp") ) {
			victim->deitypoints += amount;
			field_name = "deity points";

		} else if( !str_prefix(field, "practice") ) {
			victim->practice += amount;
			field_name = "practices";

		} else if( !str_prefix(field, "train") ) {
			victim->train += amount;
			field_name = "trains";

		} else if( !str_prefix(field, "quest") || !str_cmp(field, "qp") ) {
			victim->questpoints += amount;
			field_name = "quest points";

		} else if( !str_prefix(field, "experience") || !str_cmp(field, "xp") ) {
			gain_exp(victim, amount);
			field_name = "experience";

		} else
			return;


		if(!IS_NPC(victim)) {
			sprintf(buf, "Award logged: %s was awarded %d %s", victim->name, amount, field_name);
			log_string(buf);
		}
	}
}


// DEDUCT mobile string(type) number(amount)
// Types: silver, gold, pneuma, deity/dp, practice, train, quest/qp
// Returns actual amount deducted
//
// DEDUCT church string(type) number(amount)
// Types: gold, pneuma, deity/dp
// Returns actual amount deducted
//
SCRIPT_CMD(scriptcmd_deduct)
{
	char buf[MSL], *rest;
	char field[MIL];
	char *field_name;
	CHAR_DATA *victim = NULL;
	CHURCH_DATA *church = NULL;
	int amount = 0;
	SCRIPT_PARAM arg;

	info->progs->lastreturn = 0;

	if(!(rest = expand_argument(info,argument,&arg)))
		return;

	switch(arg.type) {
	case ENT_CHURCH: church = arg.d.church; break;
	case ENT_STRING: victim = script_get_char_room(info, arg.d.str, TRUE); break;
	case ENT_MOBILE: victim = arg.d.mob; break;
	default: victim = NULL; break;
	}

	if (!victim && !church) return;

	if(!(rest = expand_argument(info,rest,&arg)))
		return;

	if( arg.type != ENT_STRING ) return;
	strncpy(field,arg.d.str,MIL-1);

	if(!(rest = expand_argument(info,rest,&arg)))
		return;

	switch(arg.type) {
	case ENT_STRING: amount = atoi(arg.d.str); break;
	case ENT_NUMBER: amount = arg.d.num; break;
	default: amount = 0; break;
	}

	if(amount < 1) return;

	if( church ) {
		if( !str_prefix(field, "gold") ) {
			info->progs->lastreturn = UMIN(church->gold, amount);
			church->gold -= info->progs->lastreturn;
			field_name = "gold";

		} else if( !str_prefix(field, "pneuma") ) {
			info->progs->lastreturn = UMIN(church->pneuma, amount);
			church->pneuma -= info->progs->lastreturn;
			field_name = "pneuma";

		} else if( !str_prefix(field, "deity") || !str_cmp(field, "dp") ) {
			info->progs->lastreturn = UMIN(church->dp, amount);
			church->dp -= info->progs->lastreturn;
			field_name = "deity points";

		} else
			return;

		sprintf(buf, "Deduct logged: Church %s was deducted %d %s", church->name, amount, field_name);
		log_string(buf);
	} else {
		if( !str_prefix(field, "silver") ) {
			info->progs->lastreturn = UMIN(victim->silver, amount);
			victim->silver -= info->progs->lastreturn;
			field_name = "silver";

		} else if( !str_prefix(field, "gold") ) {
			info->progs->lastreturn = UMIN(victim->gold, amount);
			victim->gold -= info->progs->lastreturn;
			field_name = "gold";

		} else if( !str_prefix(field, "pneuma") ) {
			info->progs->lastreturn = UMIN(victim->pneuma, amount);
			victim->pneuma -= info->progs->lastreturn;
			field_name = "pneuma";

		} else if( !str_prefix(field, "deity") || !str_cmp(field, "dp") ) {
			info->progs->lastreturn = UMIN(victim->deitypoints, amount);
			victim->deitypoints -= info->progs->lastreturn;
			field_name = "deity points";

		} else if( !str_prefix(field, "practice") ) {
			info->progs->lastreturn = UMIN(victim->practice, amount);
			victim->practice -= info->progs->lastreturn;
			field_name = "practices";

		} else if( !str_prefix(field, "train") ) {
			info->progs->lastreturn = UMIN(victim->train, amount);
			victim->train -= info->progs->lastreturn;
			field_name = "trains";

		} else if( !str_prefix(field, "quest") || !str_cmp(field, "qp") ) {
			info->progs->lastreturn = UMIN(victim->questpoints, amount);
			victim->questpoints -= info->progs->lastreturn;
			field_name = "quest points";

		} else
			return;


		if(!IS_NPC(victim)) {
			sprintf(buf, "Deduct logged: %s was deducted %d %s", victim->name, amount, field_name);
			log_string(buf);
		}
	}

}

// GRANTSKILL player name[ int(rating=1)[ bool(permanent=false)[ string(flags)]]]
// GRANTSKILL player vnum[ int(rating=1)[ bool(permanent=false)[ string(flags)]]]
SCRIPT_CMD(scriptcmd_grantskill)
{
	char buf[MSL];
	char *rest;
	SCRIPT_PARAM arg;
	CHAR_DATA *mob;
	TOKEN_DATA *token = NULL;
	TOKEN_INDEX_DATA *token_index = NULL;
	int rating = 1;
	int sn = -1;
	long flags = SKILL_AUTOMATIC;
	char source = SKILLSRC_SCRIPT;

	info->progs->lastreturn = 0;

	if(!(rest = expand_argument(info,argument,&arg)))
		return;

	if(arg.type != ENT_MOBILE || !arg.d.mob || IS_NPC(arg.d.mob)) return;

	mob = arg.d.mob;

	if(!(rest = expand_argument(info,rest,&arg)))
		return;

	if( arg.type == ENT_STRING ) {
		sn = skill_lookup(arg.d.str);
		if( sn <= 0 ) return;
	} else if( arg.type == ENT_NUMBER ) {
		token_index = get_token_index(arg.d.num);

		if( !token_index ) return;
	}
	else
		return;


	if( *rest ) {

		if(!(rest = expand_argument(info,rest,&arg)))
			return;

		if( arg.type != ENT_NUMBER ) return;
		rating = URANGE(1,arg.d.num,100);

		if( *rest ) {
			bool fPerm = FALSE;

			if(!(rest = expand_argument(info,rest,&arg)))
				return;

			if( arg.type == ENT_BOOLEAN )
				fPerm = arg.d.boolean;
			else if( arg.type == ENT_NUMBER )
				fPerm = (arg.d.num != 0);
			else if( arg.type == ENT_STRING )
				fPerm = !str_cmp(arg.d.str, "true") || !str_cmp(arg.d.str, "yes") || !str_cmp(arg.d.str, "perm");
			else
				return;

			source = fPerm ? SKILLSRC_SCRIPT_PERM : SKILLSRC_SCRIPT;

			if( *rest ) {
				expand_string(info,rest,buf);

				if(!buf[0]) return;

				if (!str_cmp(buf, "none"))
					flags = 0;
				else if ((flags = flag_value(skill_flags, buf)) == NO_FLAG)
					flags = SKILL_AUTOMATIC;
			}
		}
	}

	if( token_index )
	{
		if( skill_entry_findtokenindex(mob->sorted_skills, token_index) )
			return;


		token = create_token(token_index);

		if(!token) return;


		if( token_index->value[TOKVAL_SPELL_RATING] > 0 )
			token->value[TOKVAL_SPELL_RATING] = token_index->value[TOKVAL_SPELL_RATING] * rating;
		else
			token->value[TOKVAL_SPELL_RATING] = rating;

		token_to_char_ex(token, mob, source, flags);
	}
	else if(sn > 0 && sn < MAX_SKILL )
	{
		if( skill_entry_findsn(mob->sorted_skills, sn) )
			return;

		mob->pcdata->learned[sn] = rating;
		if( skill_table[sn].spell_fun == spell_null ) {
			skill_entry_addskill(mob, sn, NULL, source, flags);
		} else {
			skill_entry_addspell(mob, sn, NULL, source, flags);
		}
	}
	else
		return;

	info->progs->lastreturn = 1;
}


// REVOKESKILL player name
// REVOKESKILL player vnum
SCRIPT_CMD(scriptcmd_revokeskill)
{
	char *rest;
	SKILL_ENTRY *entry;
	SCRIPT_PARAM arg;
	CHAR_DATA *mob;
	TOKEN_INDEX_DATA *token_index = NULL;
	int sn = -1;

	info->progs->lastreturn = 0;

	if(!(rest = expand_argument(info,argument,&arg)))
		return;

	if(arg.type != ENT_MOBILE || !arg.d.mob || IS_NPC(arg.d.mob)) return;

	mob = arg.d.mob;

	if(!(rest = expand_argument(info,rest,&arg)))
		return;

	if( arg.type == ENT_STRING ) {
		sn = skill_lookup(arg.d.str);
		if( sn <= 0 ) return;

		entry = skill_entry_findsn(mob->sorted_skills, sn);

	} else if( arg.type == ENT_NUMBER ) {
		token_index = get_token_index(arg.d.num);

		if( !token_index ) return;

		entry = skill_entry_findtokenindex(mob->sorted_skills, token_index);

	}
	else
		return;

	if( !entry ) return;

	skill_entry_removeentry(&mob->sorted_skills, entry);
	info->progs->lastreturn = 1;
}


