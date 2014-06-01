#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "merc.h"
#include "interp.h"
#include "tables.h"
#include "wilds.h"

extern void persist_save(void);

// Global variables
int save_number = 0;
int pulse_point;

// Event system for queued events.
EVENT_DATA *events;

// Local functions
int hit_gain	args((CHAR_DATA *ch));
int mana_gain	args((CHAR_DATA *ch));
int move_gain	args((CHAR_DATA *ch));
int toxin_gain(CHAR_DATA *ch);
void mobile_update	args((void));
void char_update	args((void));
void obj_update	args((void));
void aggr_update	args((void));
void ship_update     args((void));
void npc_ship_state_update     args((void));
void who_list	args((void));
void quest_update    args((void));
void remove_port     args((long vnum_boat_dock, int door));
void create_port     args((long vnum_port, long vnum_boat_dock, int door));
void stock_update    args((void));
void update_scuttle	args((void));
void update_hunting	args((void));
void toxin_update args ((CHAR_DATA *ch));
SHIP_DATA *is_being_attacked_by_ship args((SHIP_DATA *pShip));
SHIP_DATA *is_being_chased_by_ship args((SHIP_DATA *pShip));
void scare_update(CHAR_DATA *ch);
void update_has_done(CHAR_DATA *ch);
void reset_waypoint(NPC_SHIP_DATA *npc_ship) ;
void relic_update(void);
void check_relic_vanish(OBJ_DATA *relic);
void update_invasion_quest();

/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */
void update_handler(void)
{
    static int pulse_area;
    static int pulse_mobile;
    static int pulse_violence;
    static int pulse_auction;
    static int pulse_mail;
    static int pulse_aggr;
    static int pulse_event;
    char buf[MSL];
    int i;

    if (merc_down)
    	return;

    if (--pulse_area <= 0)
    {
        pulse_area = PULSE_AREA;
	area_update(FALSE);
	write_permanent_objs();
	persist_save();
	save_npc_ships();
	write_mail();
	save_projects();
	save_immstaff();
    }

    if (--pulse_auction <= 0)
    {
        pulse_auction = PULSE_AUCTION;
        auction_update();
    }

    if (--pulse_mail <= 0)
    {
	pulse_mail = PULSE_MAIL;
	mail_update();
    }

    if (--pulse_mobile <= 0)
    {
	pulse_mobile = PULSE_MOBILE;
	mobile_update();
    }

    if (--pulse_violence <= 0)
    {
	pulse_violence = PULSE_VIOLENCE;
	violence_update();
        update_hunting();
    }

    // Check to see if boosts have run out.
    for (i = 0; boost_table[i].name != NULL; i++) {
	if (i != BOOST_RECKONING && boost_table[i].timer != 0
	&&  current_time > boost_table[i].timer)
	{
	    sprintf(buf, "%s {Dboost has ended.{x\n\r", boost_table[i].color_name);
	    gecho(buf);
	    boost_table[i].timer = 0;
	    boost_table[i].boost = 100;
	}
    }

    // TICK
    if (--pulse_point <= 0)
    {
	wiznet("TICK!", NULL, NULL, WIZ_TICKS, 0, 0);
	pulse_point = PULSE_TICK;

	if (number_percent() < 20)
	    write_churches_new();

	//write_gq();
        //update_ship_exits();
	time_update();
	char_update();
	//update_invasion_quest(); Syn - don't do this. it seems to eat a lot of CPU time.
	obj_update();
	quest_update();
    	pneuma_relic_update();
	relic_update();
	update_area_trade();
	/* 2006-07-27 This is now redundant, and this function seems to loop (Syn).
	   Wilderness exits are cleaned up in char_from_room, which is much easier and more elegant.
	if (top_wilderness_exit > MAX_WILDERNESS_EXITS)
	    remove_wilderness_exits(); */

	//update_weather();

	// An autowar has started
	if (auto_war_timer > 0)
	{
	    auto_war_timer--;
	    if (auto_war_timer == 0)
		start_war();
	    else
	    {
		sprintf(buf, "{RGet ready! {RA {Y%s{R war will begin for levels {Y%d{R to {Y%d{R in {Y%d{R minutes!{x\n\r",
			auto_war_table[ auto_war->war_type ].name,
			auto_war->min,
			auto_war->max,
			auto_war_timer);
		gecho(buf);
		gecho("Type 'war join' to enter!\n\r");
	    }
	}

	// End an autowar in progress
	if (auto_war_battle_timer > 0)
	{
	    auto_war_battle_timer--;
	    if (auto_war_battle_timer == 0)
		auto_war_time_finish();
	}

	// Auto-reboot
	if (reboot_timer > 0)
	{
	    if ((reboot_timer - current_time)/60 <= 0)
	    {
		CHAR_DATA *rebooting;

		if ((rebooting = get_char_world(NULL, reboot_by)) == NULL)
		{
		    free_string(reboot_by);
		    gecho("{WIMPLEMENTOR NOT PRESENT. REBOOT AVERTED.{x\n\r");
		    reboot_timer = 0;
		    down_timer = 0;
		}
		else
		{
		    free_string(reboot_by);
		    sprintf(buf, "{WREBOOTING. DOWNTIME WILL BE APPROXIMATELY %d MINUTES.{x\n\r", down_timer);
		    gecho(buf);

		    do_function(rebooting, &do_shutdown, "");
		}
	    }
	    else
	    {
		sprintf(buf, "{WREBOOT IN %ld MINUTE%s.{x\n\r",
			(long int)((reboot_timer - current_time)/60),
			(reboot_timer - current_time)/60 == 1 ? "" : "S");
		gecho(buf);
	    }
	}
    }

    if (--pulse_aggr <= 0)
    {
        pulse_aggr = PULSE_AGGR;
	aggr_update();
    }

    if ( --pulse_event        <= 0 )
    {
	pulse_event             = PULSE_EVENT;
	event_update();
    }

    tail_chain();
}


// Advance a PC one level.
void advance_level(CHAR_DATA *ch, bool hide)
{
    char buf[MAX_STRING_LENGTH];
    int add_hp;
    int add_mana;
    int add_move;
    int add_prac;

    ch->exp = 0;
    ch->pcdata->last_level =
	(ch->played + (int) (current_time - ch->logon)) / 3600;

    add_hp	= con_app[get_curr_stat(ch,STAT_CON)].hitp + number_range(
		  class_table[get_profession(ch, CLASS_CURRENT)].hp_min,
		  class_table[get_profession(ch, CLASS_CURRENT)].hp_max);

    add_mana 	= get_curr_stat(ch,STAT_INT)/4 +
	          get_curr_stat(ch,STAT_WIS)/4 +
		  number_range(1, 10);

    if (!class_table[get_profession(ch, CLASS_CURRENT)].fMana)
	add_mana /= 2;

    add_move	= get_curr_stat(ch, STAT_STR) / 2 + get_curr_stat(ch, STAT_DEX) / 2 + number_range(1, 3);

    add_prac	= 2 * wis_app[get_curr_stat(ch,STAT_WIS)].practice;

    add_hp = add_hp * 9/10;
    add_mana = add_mana * 9/10;
    add_move = add_move * 9/10;

    add_hp	= UMAX( 2, add_hp  );
    add_mana	= UMAX( 2, add_mana);
    add_move	= UMAX( 6, add_move);

    if (IS_REMORT(ch))
    {
	add_hp = (ch->pcdata->hit_before / 120);
	add_mana = (ch->pcdata->mana_before / 120);
	add_move = (ch->pcdata->move_before / 120);
    }

    if (ch->pcdata->perm_hit + add_hp > pc_race_table[ch->race].max_vital_stats[MAX_HIT])
	add_hp = pc_race_table[ch->race].max_vital_stats[MAX_HIT] - ch->pcdata->perm_hit;
    if (ch->pcdata->perm_mana + add_mana > pc_race_table[ch->race].max_vital_stats[MAX_MANA])
	add_mana = pc_race_table[ch->race].max_vital_stats[MAX_MANA] - ch->pcdata->perm_mana;
    if (ch->pcdata->perm_move + add_move > pc_race_table[ch->race].max_vital_stats[MAX_MOVE])
	add_move = pc_race_table[ch->race].max_vital_stats[MAX_MOVE] - ch->pcdata->perm_move;

    ch->max_hit += add_hp;
    ch->max_mana += add_mana;
    ch->max_move += add_move;
    ch->practice += add_prac;

    if ((ch->tot_level <= 10)
          || (ch->tot_level > 10 && number_percent() < 10))
	ch->train += 1;

    ch->pcdata->perm_hit	+= add_hp;
    ch->pcdata->perm_mana	+= add_mana;
    ch->pcdata->perm_move	+= add_move;

    if (!hide)
    {
	sprintf(buf,
	    "{MYou gain {W%d {Mhit point%s, {W%d {Mmana, {W%d {Mmove, "
	    "and {W%d {Mpractice%s.\n\r{x",
	    add_hp,
	    add_hp == 1 ? "" : "s",
	    add_mana,
	    add_move,
	    add_prac,
	    add_prac == 1 ? "" : "s");
	send_to_char(buf, ch);
    }
}


// Give a character exp
void gain_exp(CHAR_DATA *ch, int gain)
{
	char buf[MAX_STRING_LENGTH];

	if (IS_IMMORTAL(ch)) return;

	if(IS_NPC(ch)) {
		if(!IS_SET(ch->act2,ACT2_CANLEVEL) || ch->maxexp < 1) return;
		ch->exp += gain;

		if(ch->exp >= ch->maxexp) {
			if( !p_percent_trigger(ch, NULL, NULL, NULL, ch, NULL, NULL, NULL, NULL,TRIG_LEVEL, NULL) ) {
				ch->exp = 0;

				ch->level += 1;
				ch->tot_level += 1;
			}
		}
	} else {
		if (ch->tot_level >= 120)
			return;

		/* make sure you never get more than the exp for your level */
		ch->exp = UMIN(exp_per_level(ch,ch->pcdata->points), ch->exp + gain);

		if (ch->tot_level < LEVEL_HERO && ch->level < MAX_CLASS_LEVEL &&
			ch->exp >= exp_per_level(ch,ch->pcdata->points)) {

			send_to_char("{MYou raise a level!!{x\n\r", ch);
			ch->exp = 0;
			ch->level += 1;
			ch->tot_level += 1;
			sprintf(buf,"%s gained level %d",ch->name,ch->level);

			sprintf(buf, "All congratulate %s who is now level %d!!!", ch->name, ch->tot_level);
			crier_announce(buf);

			if (ch->level >= MAX_CLASS_LEVEL) {
				if (ch->tot_level != 120) {
					send_to_char("You are now ready to multiclass."
						"\n\rType 'help multiclass' for more information.\n\r", ch);
				}
			}

			log_string(buf);
			sprintf(buf,"$N has attained level %d!",ch->level);
			wiznet(buf,ch,NULL,WIZ_LEVELS,0,0);
			advance_level(ch,FALSE);

			save_char_obj(ch);

			p_percent_trigger(ch, NULL, NULL, NULL, ch, NULL, NULL, NULL, NULL,TRIG_LEVEL, NULL);
		}
	}
}


int hit_gain(CHAR_DATA *ch)
{
    int gain;
    int number;
    long amount;
    char buf[MAX_STRING_LENGTH];

    if (ch->in_room == NULL) {
		sprintf(buf, "hit_gain: %s had null in_room!", IS_NPC(ch) ? ch->short_descr : ch->name);
		bug(buf, 0);
		return 0;
    }

    if (IS_NPC(ch)) {
		gain =  5 + ch->level;
		if (IS_AFFECTED(ch,AFF_REGENERATION))
		    gain *= 2;

		switch(ch->position) {
	    default: 		gain /= 2;		break;
	    case POS_SLEEPING: 	gain = 3 * gain/2;    	break;
	    case POS_RESTING:  				break;
	    case POS_FIGHTING:	gain /= 3;		break;
		}
    } else {
		gain = UMAX(3,get_curr_stat(ch,STAT_CON) - 3 + ch->tot_level/2);
		gain += class_table[get_profession(ch, CLASS_CURRENT)].hp_max - 10;
		number = number_percent();
		if (number < get_skill(ch,gsn_fast_healing)) {
		    gain += number * gain / 100;
		    if (ch->hit < ch->max_hit)
				check_improve(ch,gsn_fast_healing,TRUE,8);
		}

		switch (ch->position) {
	    default:	   	gain = 3 * gain / 2;	break;
	    case POS_SLEEPING: 	gain = 3 * gain;	break;
	    case POS_RESTING:   gain = gain * 2; 	break;
	    case POS_FIGHTING: 	gain /= 2;		break;
		}

		if (ch->pcdata->condition[COND_HUNGER] == 0)
		    gain /= 2;

		if (ch->pcdata->condition[COND_THIRST] == 0)
		    gain /= 2;
    }

    if (ch->in_room->heal_rate > 0)
		gain = gain * ch->in_room->heal_rate / 100;

    if (ch->on && ch->on->item_type == ITEM_FURNITURE && ch->on->value[3] > 0)
		gain = gain * ch->on->value[3] / 100;

    if (IS_AFFECTED(ch, AFF_POISON))
		gain /= 4;

    if (IS_AFFECTED(ch, AFF_PLAGUE))
		gain /= 8;

    if (IS_AFFECTED(ch,AFF_REGENERATION) || (ch->tot_level < 31 && !IS_REMORT(ch)))
		gain *= 2;

    // Druids get 33% more in nature
    if (get_profession(ch, SUBCLASS_CLERIC) == CLASS_CLERIC_DRUID && is_in_nature(ch))
		gain += gain/3;

	/* If you have the relic you get 25% more */
	if (ch->church && vnum_in_treasure_room(ch->church, OBJ_VNUM_RELIC_HP_REGEN))
		gain += gain / 4;

   amount = UMIN(gain, ch->max_hit - ch->hit);
   return amount;
}


int mana_gain(CHAR_DATA *ch)
{
    int gain;
    int number;
    char buf[MSL];

    if (ch->in_room == NULL)
    {
        sprintf(buf, "mana_gain: %s had null in_room!",
	    IS_NPC(ch) ? ch->short_descr : ch->name);
	    bug(buf, 0);
	return 0;
    }

    if (IS_NPC(ch))
    {
        gain = 5 + ch->level;
	switch (ch->position)
	{
	    default:		gain /= 2;		break;
	    case POS_SLEEPING:	gain = 3 * gain/2;	break;
	    case POS_RESTING:				break;
	    case POS_FIGHTING:	gain /= 3;		break;
	}
    }
    else
    {
	gain = (get_curr_stat(ch,STAT_WIS)
	      + get_curr_stat(ch,STAT_INT) + ch->tot_level) / 2;
	number = number_percent();

	if (number < get_skill(ch,gsn_meditation))
	{
	    gain += number * gain / 100;
	    if (ch->mana < ch->max_mana)
		check_improve(ch,gsn_meditation,TRUE,8);
	}

	if (!class_table[get_profession(ch, CLASS_CURRENT)].fMana)
	    gain /= 2;

	switch (ch->position)
	{
	    default:	   	gain = gain;			break;
	    case POS_SLEEPING: 	gain = 2 * gain;	break;
	    case POS_RESTING:   gain = 3 * gain/2; 			break;
	    case POS_FIGHTING: 	gain /= 2;			break;
	}

	if (ch->pcdata->condition[COND_HUNGER]   == 0)
	    gain /= 2;

	if (ch->pcdata->condition[COND_THIRST] == 0)
	    gain /= 2;

    }

    if (ch->in_room->mana_rate > 0)
	gain = gain * ch->in_room->mana_rate / 100;

    if (ch->on != NULL
    && ch->on->item_type == ITEM_FURNITURE
    && ch->on->value[4] > 0)
	gain = gain * ch->on->value[4] / 100;

    if (IS_AFFECTED(ch, AFF_POISON))
	gain /= 4;

    if (IS_AFFECTED(ch, AFF_PLAGUE))
	gain /= 8;

    if (IS_AFFECTED(ch,AFF_HASTE) || IS_AFFECTED(ch,AFF_SLOW))
	gain /= 2;

    // Druids get 33% more in nature
    if (get_profession(ch, SUBCLASS_CLERIC) == CLASS_CLERIC_DRUID
    &&   is_in_nature(ch))
	gain += gain/3;

	if (ch->church && vnum_in_treasure_room(ch->church, OBJ_VNUM_RELIC_MANA_REGEN))
		gain += gain / 4;

   if (IS_ELF(ch))
       gain *= 2;

   if (!str_cmp(race_table[ch->race].name, "lich"))
       gain = (gain * 5)/2;

   if (ch->tot_level < 31 && !IS_REMORT(ch))
       gain *= 2;

   return UMIN(gain, ch->max_mana - ch->mana);
}


int move_gain(CHAR_DATA *ch)
{
    int gain;
    long amount;
    char buf[MSL];

    if (ch->in_room == NULL)
    {
        sprintf(buf, "move_gain: %s had null in_room!",
	    IS_NPC(ch) ? ch->short_descr : ch->name);
	    bug(buf, 0);
	return 0;
    }

    if (IS_NPC(ch))
	gain = ch->level;
    else
    {
	gain = UMAX(15, ch->level);

	switch (ch->position)
	{
	    case POS_SLEEPING:
	        gain += get_curr_stat(ch,STAT_DEX)*3;
	        break;
	    case POS_RESTING:
	        gain += get_curr_stat(ch,STAT_DEX) / 2 * 3;
	    	break;
	}

	if (ch->pcdata->condition[COND_HUNGER]   == 0)
	    gain /= 2;

	if (ch->pcdata->condition[COND_THIRST] == 0)
	    gain /= 2;
    }

    if (ch->in_room->move_rate > 0)
	gain = gain * ch->in_room->move_rate/100;

    if (ch->on != NULL
    && ch->on->item_type == ITEM_FURNITURE
    && ch->on->value[5] > 0)
	gain = gain * ch->on->value[5] / 100;

    if (IS_AFFECTED(ch, AFF_POISON))
	gain /= 4;

    if (IS_AFFECTED(ch, AFF_PLAGUE))
	gain /= 8;

    if (IS_AFFECTED(ch,AFF_HASTE) || IS_AFFECTED(ch,AFF_SLOW))
	gain /= 2;

    // Druids get 33% more in nature
    if (get_profession(ch, SUBCLASS_CLERIC) == CLASS_CLERIC_DRUID
    &&   is_in_nature(ch))
	gain += gain/3;

    if (ch->tot_level < 31 && !IS_REMORT(ch))
	gain *= 2;

    amount = UMIN(gain, ch->max_move - ch->move);

    return amount;
}


// Regen sith toxins.
int toxin_gain(CHAR_DATA *ch)
{
    int gain;
    char buf[MAX_STRING_LENGTH];

    if (ch->in_room == NULL)
    {
        sprintf(buf, "toxin_gain: %s had null in_room!",
            IS_NPC(ch) ? ch->short_descr : ch->name);
        bug(buf, 0);
	return 0;
    }

    if (IS_NPC(ch))
    {
	gain =  5 + ch->level;
	if (IS_AFFECTED(ch,AFF_REGENERATION))
	    gain *= 2;

	switch(ch->position)
	{
	    default : 		gain /= 2;			break;
	    case POS_SLEEPING: 	gain = 3 * gain/2;		break;
	    case POS_RESTING:  					break;
	    case POS_FIGHTING:	gain /= 3;		 	break;
	}
    }
    else
    {
	gain = UMAX(3,get_curr_stat(ch,STAT_CON) - 3);

	switch (ch->position)
	{
	    default:	   	gain = 3 * gain / 2;			break;
	    case POS_SLEEPING: 	gain = 3 * gain;	break;
	    case POS_RESTING:   gain = gain * 2; 			break;
	    case POS_FIGHTING: 	gain /= 2;			break;
	}

	if (ch->pcdata->condition[COND_HUNGER]   == 0)
	    gain /= 2;

	if (ch->pcdata->condition[COND_THIRST] == 0)
	    gain /= 2;
    }

    if (IS_AFFECTED(ch, AFF_POISON))
	gain /= 4;

    if (IS_AFFECTED(ch, AFF_PLAGUE))
	gain /= 8;

    if (IS_AFFECTED(ch,AFF_REGENERATION))
	    gain *= 2;

    gain += number_range(1,2);

    return (URANGE(1, gain, 15));
}


// Update a condition (hunger, thirst, drunk, etc.)
void gain_condition(CHAR_DATA *ch, int iCond, int value)
{
    int condition;

    // For the life sustaining quest item.
    if (value < 0
    && (iCond == COND_HUNGER || iCond == COND_THIRST)
    && is_sustained(ch))
    {
        ch->pcdata->condition[iCond] = 48;
	return;
    }

    if (value == 0 || IS_NPC(ch) || ch->level >= LEVEL_IMMORTAL)
	return;

    condition = ch->pcdata->condition[iCond];
    if (condition == -1)
	return;

	// When draining hunger/thirst, they have a CON% chance of not losing it.
	if( (value < 0) && (iCond == COND_HUNGER || iCond == COND_THIRST) &&
		(number_percent() < get_curr_stat(ch, STAT_CON)))
		return;

    ch->pcdata->condition[iCond] = URANGE(0, condition + value, 48);

    if (ch->pcdata->condition[iCond] == 0)
    {
	switch (iCond)
	{
	case COND_HUNGER:
	    send_to_char("You are hungry.\n\r",  ch);
	    break;

	case COND_THIRST:
	    send_to_char("You are thirsty.\n\r", ch);
	    break;

	case COND_DRUNK:
	    if (condition != 0)
		send_to_char("You are sober.\n\r", ch);
	    break;
	}
    }
}


/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Merc cpu time.
 */
void mobile_update(void)
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    EXIT_DATA *pexit;
    int door;
    char buf[MSL];

    for (ch = char_list; ch != NULL; ch = ch_next)
    {
	ch_next = ch->next;

	if (ch->in_room == NULL
	||   IS_AFFECTED(ch,AFF_CHARM))
	    continue;

	// Done to allow for TOKEN random type scripts on players, but only if they have tokens!
	if (!IS_NPC(ch)) {
	    if(ch->tokens) {
		if(p_percent_trigger(ch, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, TRIG_RANDOM, NULL)) continue;

		// Prereckoning
		if (pre_reckoning > 0 && reckoning_timer > 0 && p_percent_trigger(ch, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, TRIG_PRERECKONING, NULL))
		    continue;

		// Reckoning
		if (!pre_reckoning && reckoning_timer > 0 && p_percent_trigger(ch, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, TRIG_RECKONING, NULL))
		    continue;
	    }
	    continue;
	}

	if (ch->in_room->area->empty && !IS_SET(ch->act,ACT_UPDATE_ALWAYS))
	    continue;

	// A dirty hack to remove any Death mobs that have been stranded
	if (!str_cmp(ch->short_descr, "Death"))
	{
            CHAR_DATA *vch;
	    CHAR_DATA *vch_next;
	    for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
            {
	        vch_next = vch->next_in_room;
		if (vch != ch && !IS_NPC(vch))
		{
		    send_to_char("{C'Well then.' says Death. 'Looks like that corpse is adequately dead. Cya folks, im outta here.'{x\n\r", vch);
		    send_to_char("Death waves happily.\n\r", vch);
		    send_to_char("{DDeath sinks into the ground and disappears.{x\n\r", vch);
		}
	    }

	    extract_char(ch, TRUE);
	    continue;
	}

	// Examine call for special procedure
	if (ch->spec_fun != 0)
	{
	    if ((*ch->spec_fun)(ch))
		continue;
	}

	// Give shop owners gold
	if (ch->pIndexData->pShop != NULL)
	{
	    if ((ch->gold * 100 + ch->silver) < ch->pIndexData->wealth)
	    {
		ch->gold += ch->pIndexData->wealth * number_range(1,20)/5000000;
		ch->silver += ch->pIndexData->wealth * number_range(1,20)/50000;
	    }
	}

	// Check mob triggers

	// Delay
	if (ch->progs->delay > 0)
	{
	    if (--ch->progs->delay <= 0)
	    {
		p_percent_trigger(ch, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, TRIG_DELAY, NULL);
		continue;
	    }
	}

        // Random
	if (p_percent_trigger(ch, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, TRIG_RANDOM, NULL)) continue;

	// Prereckoning
	if (pre_reckoning > 0 && reckoning_timer > 0) {
	    if(p_percent_trigger(ch, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, TRIG_PRERECKONING, NULL))
	    continue;
	}

	// Reckoning
	if (pre_reckoning == 0 && reckoning_timer > 0) {
	    if (p_percent_trigger(ch, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, TRIG_RECKONING, NULL))
		continue;
	}

	// get rid of crew when they are past their hired date
	if (ch->belongs_to_ship != NULL
	&&   !IS_NPC_SHIP(ch->belongs_to_ship)
	&&   current_time > ch->hired_to)
	    extract_char(ch, TRUE);

	// That's all for sleeping / busy monster, and empty zones
	if (ch->position != POS_STANDING)
	    continue;

	// Ship Quest masters
	if (IS_SET(ch->act2, ACT2_SHIP_QUESTMASTER) && number_percent() < 5) {
	    AREA_DATA *pArea = NULL;

	    for (pArea = area_first; pArea != NULL; pArea = pArea->next) {

		if (pArea->invasion_quest != NULL && ch->in_room != NULL &&
			ch->in_room->area->place_flags == pArea->place_flags) {
		    sprintf(buf, "We are offering a reward to anyone that can restore order in %s.", pArea->name);
		    do_say(ch, buf);
		}
	    }
	}

	// Scavenge
	if (IS_SET(ch->act, ACT_SCAVENGER)
	&&   ch->in_room->contents != NULL
	&&   number_bits(6) == 0)
	{
	    OBJ_DATA *obj;
	    OBJ_DATA *obj_best;
	    int max;

	    max = 1;
	    obj_best = 0;
	    for (obj = ch->in_room->contents; obj; obj = obj->next_content)
	    {
		if (!can_get_obj(ch, obj, NULL, NULL, TRUE))
		    continue;

		if (CAN_WEAR(obj, ITEM_TAKE)
		&&   obj->cost > max
		&&   obj->cost > 0
		&&   !is_quest_token(obj))
		{
		    obj_best = obj;
		    max = obj->cost;
		}
	    }

	    if (obj_best != NULL)
	    {
		obj_from_room(obj_best);
		obj_to_char(obj_best, ch);
		act("$n gets $p.", ch, NULL, NULL, obj_best, NULL, NULL, NULL, TO_ROOM);
	    }
	}

        if (ch->in_room == NULL)
	{
	    sprintf(buf, "mobile_update: ch %s (%ld) had null in_room!",
	    	IS_NPC(ch) ? ch->short_descr : ch->name,
		IS_NPC(ch) ? ch->pIndexData->vnum : 0);
	    bug(buf, 0);
	    continue;
	}

	/* Wander */

	/* Syn - Only do this for mobs that aren't grouped. Obviously
	   to prevent grouped mobs from wandering off, since wandering
	   can be done with a mprog, while the reverse cannot be done */
	if (ch->leader == NULL // Following AND grouped
	&&  ch->master == NULL // Following only
	&&  !IS_SET(ch->act, ACT_SENTINEL)
	&&  number_bits(3) == 0
	&&  !IS_SET(ch->act, ACT_MOUNT)) {
	    door = number_range(0, MAX_DIR - 1);
	    if ((pexit = ch->in_room->exit[door]) != NULL
	    &&  pexit->u1.to_room != NULL
	    &&  !IS_SET(pexit->exit_info, EX_CLOSED)
	    &&  !IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB)
	    &&  !IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_WANDER)
	    &&  (!IS_SET(ch->act2, ACT_STAY_LOCALE) ||
	    	(pexit->u1.to_room->area == ch->in_room->area &&
	    		(!ch->in_room->locale || !pexit->u1.to_room->locale || ch->in_room->locale == pexit->u1.to_room->locale)))
	    &&  (IS_SET(ch->act2, ACT2_WILDS_WANDERER) || !pexit->u1.to_room->wilds)
	    &&  (!IS_SET(ch->act, ACT_STAY_AREA)
		 || pexit->u1.to_room->area == ch->in_room->area)
	    &&  (!IS_SET(ch->act, ACT_OUTDOORS)
		 || !IS_SET(pexit->u1.to_room->room_flags,ROOM_INDOORS))
	    &&  (!IS_SET(ch->act, ACT_INDOORS)
		 || IS_SET(pexit->u1.to_room->room_flags,ROOM_INDOORS)))
		move_char(ch, door, FALSE);
	}
    }
}


void remove_port(long vnum_boat_dock, int door)
{
#if 0
    ROOM_INDEX_DATA *pWildRoom;
    char buf[MAX_STRING_LENGTH];

    if ((pWildRoom = get_room_index(vnum_boat_dock)) != NULL)
    {
	rp_change_exit(get_room_index(6551), "west delete", DIR_WEST);
	sprintf(buf, "west %ld", vnum_boat_dock-1);
	rp_change_exit(pWildRoom, buf, DIR_WEST);
	sprintf(buf, "north %ld", vnum_boat_dock-348);
	rp_change_exit(pWildRoom, buf, DIR_NORTH);
	sprintf(buf, "south %ld", vnum_boat_dock+348);
	rp_change_exit(pWildRoom, buf, DIR_SOUTH);
	sprintf(buf, "east %ld", vnum_boat_dock+1);
	rp_change_exit(pWildRoom, buf, DIR_EAST);
    }
#endif
}


void create_port(long vnum_port, long vnum_boat_dock, int door)
{
#if 0
    ROOM_INDEX_DATA *pRoom;
    ROOM_INDEX_DATA *pWildRoom;

    if ((pWildRoom = get_room_index(vnum_boat_dock)) != NULL &&
	    (pRoom = get_room_index(vnum_port)) != NULL)
    {
	rp_change_exit(pWildRoom, "west delete", DIR_WEST);
	rp_change_exit(pWildRoom, "east delete", DIR_EAST);
	rp_change_exit(pWildRoom, "south delete", DIR_SOUTH);
	rp_change_exit(pWildRoom, "north delete", DIR_NORTH);
	rp_change_exit(pRoom, "east 6551", DIR_EAST);
    }
#endif
}

// Update the public boat.
void update_public_boat(int time)
{
#if 0
    ROOM_INDEX_DATA *room;
    AREA_DATA *ship_area;

    ship_area = find_area("Ship");
    if (ship_area == NULL)
    {
	bug("update_public_boat: ship area was null!", 0);
	return;
    }

    if (find_area("Wilderness") == NULL)
    {
	log_string("update_public_boat: no wilderness!");
	return;
    }

    // Boat arrives in at Plith
    if (time == 7)
    {
	room = get_room_index(744104);
	rp_change_exit(room, "command delete", DIR_EAST);
	rp_change_exit(room, "command 6551", DIR_EAST);

	room = get_room_index(6551);
	rp_change_exit(room, "command 744104", DIR_WEST);

	room_echo(room, "{YCaptain Pinot yells 'The Endeavor has arrived in Plith!'{x\n\r");

	sector_echo(ship_area, "{YCaptain Pinot yells 'The Endeavor has arrived in Plith!'{x", SECT_INSIDE);

	sector_echo(find_area("Plith"), "{YYou hear the large horns of the Endeavor as it enters Plith harbour!{x", SECT_CITY);
    }
    else
    // Boat leaves from Plith
    if (time == 9)
    {
	room = get_room_index(744104);
	room_echo(room, "{YCaptain Pinot yells 'The Endeavor has left for Achaeus!'{x\n\r");

	rp_change_exit(room, "command delete", DIR_EAST);
	rp_change_exit(room, "command 744105", DIR_EAST);

	room = get_room_index(6551);
	rp_change_exit(room, "command delete", DIR_WEST);

	sector_echo(ship_area, "{YCaptain Pinot yells 'The Endeavor has left for Achaeus!'{x", SECT_INSIDE);

	sector_echo(find_area("Plith"), "{YYou hear the large horns of the Endeavor as it leaves Plith harbour!{x", SECT_CITY);
    }
    else
    // Boat arrives in at Achaeus
    if (time == 16)
    {
	room = get_room_index(781122);

	rp_change_exit(room, "command delete", DIR_EAST);
	rp_change_exit(room, "command 6551", DIR_EAST);

	room = get_room_index(6551);
	rp_change_exit(room, "command 781122", DIR_WEST);

	room_echo(room, "{YCaptain Pinot yells 'The Endeavor has arrived in Achaeus!'{x\n\r");

	sector_echo(ship_area, "{YCaptain Pinot yells 'The Endeavor has arrived in Achaeus!'{x", SECT_INSIDE);

	sector_echo(find_area("Achaeus"), "{YYou hear the large horns of the Endeavor as it enters Achaeus harbour!{x", SECT_CITY);
    }
    else
    // Boat leaves from Achaeus
    if (time == 18)
    {
	room = get_room_index(781122);

	rp_change_exit(room, "command delete", DIR_EAST);
	rp_change_exit(room, "command 781123", DIR_EAST);

	room = get_room_index(6551);
	rp_change_exit(room, "command delete", DIR_WEST);

	room_echo(room, "{YCaptain Pinot yells 'The Endeavor has left for Olaria!'{x\n\r");

	sector_echo(ship_area, "{YCaptain Pinot yells 'The Endeavor has left for Olaria!'{x", SECT_INSIDE);

	sector_echo(find_area("Achaeus"), "{YYou hear the large horns of the Endeavor as it leaves Achaeus harbour!{x", SECT_CITY);
    }
    else
    // Boat arrives in at Olaria
    if (time == 1)
    {
	room = get_room_index(656895);
	rp_change_exit(room, "command delete", DIR_EAST);
	rp_change_exit(room, "command 6551", DIR_EAST);

	room = get_room_index(6551);
	rp_change_exit(room, "command 656895", DIR_WEST);

	room_echo(room, "{YCaptain Pinot yells 'The Endeavor has arrived in Olaria!'{x\n\r");

	sector_echo(ship_area, "{YCaptain Pinot yells 'The Endeavor has arrived in Olaria!'{x", SECT_INSIDE);

	sector_echo(find_area("Olaria"), "{YYou hear the large horns of the Endeavor as it enters Olaria harbour!{x", SECT_CITY);
    }
    else
    // Boat leaves from Olaria
    if (time == 3)
    {
	room = get_room_index(656895);
	rp_change_exit(room, "command delete", DIR_EAST);
	rp_change_exit(room, "command 656896", DIR_EAST);

	room = get_room_index(6551);
	rp_change_exit(room, "command delete", DIR_WEST);

	room_echo(room, "{YCaptain Pinot yells 'The Endeavor has left for Plith!'{x\n\r");

	sector_echo(ship_area, "{YCaptain Pinot yells 'The Endeavor has left for Plith!'{x", SECT_INSIDE);

	sector_echo(find_area("Olaria"), "{YYou hear the large horns of the Endeavor as it leaves Olaria harbour!{x", SECT_CITY);
    }
#endif
}


// Update the time.
void time_update(void)
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int hours;

    buf[0] = '\0';

    time_info.hour++;

    // Update public boat.
    switch (time_info.hour)
    {
	case  1: // Boat arrives in Olaria
	    //update_public_boat(time_info.hour);
	    break;

	case  3: // Boat leaves from Olaria
	    //update_public_boat(time_info.hour);
	    break;

	case  5:
	    weather_info.sunlight = SUN_RISE;
	    strcat(buf, "The day has begun.\n\r");
	    break;

	case  6:
	    weather_info.sunlight = SUN_LIGHT;
	    strcat(buf, "The sun rises in the east.\n\r");
	    break;

	case 7: //Boat arrives in plith
	    //update_public_boat(time_info.hour);
	    break;

	case 9: //Boat leaves in plith
	    //update_public_boat(time_info.hour);
	    break;

	case 16: // Boat arrives in achaeus
	    //update_public_boat(time_info.hour);
	    break;

	case 18: // Boat leaves achaeus
	    //update_public_boat(time_info.hour);
	    break;

	case 19:
	    weather_info.sunlight = SUN_SET;
	    strcat(buf, "The sun slowly disappears in the west.\n\r");
	    break;

	case 20:
	    weather_info.sunlight = SUN_DARK;
	    strcat(buf, "{DThe night has begun.{x\n\r");
	    break;

	case 24:
	    time_info.hour = 0;
	    time_info.day++;
	    break;
    }

#if 0
    /*
     * This is for the PLITH Airship. After two hours it must be made to go back home.
     */
    if ( plith_airship != NULL )
    {
  if ( plith_airship->captain != NULL
  && plith_airship->captain->ship_arrival_time == time_info.hour
  && str_cmp( plith_airship->ship->ship->in_room->area->name, "Plith" ) )
  {
      AREA_DATA *plith = find_area( "Plith" );
//sprintf(buf, "Just added plith waypoint %ld %ld ",  plith->x, plith->y );
//log_string(buf);
      add_move_waypoint( plith_airship->ship, plith->x, plith->y );

//      sprintf(buf, "{YThe %s rises high into the sky then disappears!{x\n\r", plith_airship->ship->ship_name );
//      echo_around(plith_airship->ship->ship->in_room, buf);
 //     room_echo(plith_airship->ship->ship->in_room, buf);
				SHIP_STATE(plith_airship->captain, 8);
//gecho("moo");
      //gecho( "ITS GONE TO fly back home!" );

		//plith_airship->captain->ship_depart_time = 80;
	//	plith_airship->captain->ship_dest_x = plith->x;
//		plith_airship->captain->ship_dest_y = plith->y;
  }
  else
  {
      sprintf( buf, "Airship ship_arrival_time=%d. Area name='%s'. Captain exists=%s", plith_airship->captain->ship_arrival_time, plith_airship->ship->ship->in_room->area->name, plith_airship->captain==NULL ? "FALSE" : "TRUE" );
      log_string( buf );
  }
    }
#endif

    buf[0] = '\0';

    if (time_info.day   >= 35)
    {
	time_info.day = 0;
	time_info.month++;
    }

    if (time_info.month >= 12)
    {
	time_info.month = 0;
	time_info.year++;
    }

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

	if (time_info.moon == MOON_FULL && weather_info.sunlight == SUN_DARK && number_percent() < reckoning_chance) {
		struct tm *reck_time;

		reck_time = (struct tm *) localtime(&current_time);
		reck_time->tm_min += 30;
		boost_table[BOOST_RECKONING].timer = (time_t) mktime(reck_time);
		pre_reckoning = 1;
	}

	// If pre_reckoning is 0 then it is taking place
	if (reckoning_timer > 0 && pre_reckoning > 0) {
		if (pre_reckoning == 5) {
			sprintf(buf, "{MA thick purple hazy mist descends around as you the reckoning takes hold!{x\n\r"
				"{yYou feel a sudden urge to kill the innocent for personal gain!{x\n\r");
			pre_reckoning = 0;
			boost_table[BOOST_RECKONING].timer = reckoning_timer;
			boost_table[BOOST_RECKONING].boost = 200;
		} else {
			switch(pre_reckoning++) {
			case 1: sprintf(buf, "You notice a slight discolouration in the sky.\n\r"); break;
			case 2: sprintf(buf, "{MThe sky dims as dark purple and maroon clouds roll in out of nowhere.{x\n\r"); break;
			case 3: sprintf(buf, "{CA strong gust picks up, howling loudly as it rips across the land.{x\n\r"); break;
			case 4: sprintf(buf, "{BSheets of cold blue lightning gather in the sky as a demonic terror grips the world.{x\n\r"); break;
			}
		}
	}

	if (reckoning_timer > 0 && current_time > reckoning_timer) {
		gecho("{MAs quickly as it appeared, the hazy purple mist dissipates. The reckoning has ended.{x\n\r");
		reckoning_timer = 0;
		reckoning_chance = 25;
		boost_table[BOOST_RECKONING].boost = 100;
	}

    if (buf[0] != '\0')
    {
	for (d = descriptor_list; d != NULL; d = d->next)
	{
	    if (d->connected == CON_PLAYING
		    &&   (d->character->in_room != NULL &&
			d->character->in_room->sector_type != SECT_INSIDE)//IS_OUTSIDE(d->character)
		    && !IN_EDEN(d->character)
		    && !IN_NETHERWORLD(d->character)
		    &&   IS_AWAKE(d->character)) {
		send_to_char(buf, d->character);
	    }
	}
    }

  buf[0] = '\0';
	for (d = descriptor_list; d != NULL; d = d->next)
	{
	    if (pre_reckoning == 0 && reckoning_timer > 0)
	    {
		if (d->connected == CON_PLAYING
		&& d->character->in_room != NULL
		&& d->character->in_room->sector_type != SECT_INSIDE
		&&   IS_AWAKE(d->character))
		{
		    if (number_percent() < 50)
			sprintf(buf, "{YLightning crashes down around you!{x\n\r");
		    else
			sprintf(buf, "{YThe wind howls as it screams around you!{x\n\r");
		    send_to_char(buf, d->character);
		}
	}
    }
}

#if 0
SHIP_DATA *is_being_attacked_by_ship(SHIP_DATA *pShip)
{
#if 0
    SHIP_DATA *ship;
    for (ship = ((AREA_DATA *) get_sailing_boat_area())->ship_list;
         ship != NULL; ship = ship->next) {
	if (ship->ship_attacked == pShip)
	{
	    return ship;
	}
    }
#endif
    return NULL;
}


SHIP_DATA *is_being_chased_by_ship(SHIP_DATA *pShip)
{
#if 0
    SHIP_DATA *ship;
    for (ship = ((AREA_DATA *) get_sailing_boat_area())->ship_list;
         ship != NULL; ship = ship->next) {
	if (ship->ship_chased == pShip)
	{
	    return ship;
	}
    }
#endif
    return NULL;
}


void npc_ship_chase_ship(NPC_SHIP_DATA *npc_ship, SHIP_DATA *target_ship)
{
#if 0
    SHIP_DATA *orig_ship;
    SHIP_DATA *ship;
    SHIP_DATA *attack;
    int x, y;

//gecho("CHASE SHIP HAS BEEEEEEEEEEEEEEEN CALLED!");
        x = get_squares_to_show_x(0);
        y = get_squares_to_show_y(0);

        orig_ship = npc_ship->ship;

        attack = NULL;

		// The only time a chase can't happen is when someone is in a harbour
		if ( npc_ship->pShipData->npc_type != NPC_SHIP_COAST_GUARD && (
			 target_ship->ship->in_room->vnum == ROOM_VNUM_SEA_PLITH_HARBOUR ||
		 	 target_ship->ship->in_room->vnum == ROOM_VNUM_SEA_NORTHERN_HARBOUR ||
		  	 target_ship->ship->in_room->vnum == ROOM_VNUM_SEA_SOUTHERN_HARBOUR))
		{
			return;
		}

            // Check for sailing ship to attack
            for ( ship = ((AREA_DATA *) get_sailing_boat_area())->ship_list; ship != NULL; ship = ship->next)
                if (
                  (ship->ship->in_room->x < orig_ship->ship->in_room->x + x &&
                     ship->ship->in_room->x > orig_ship->ship->in_room->x - x)
                 && (ship->ship->in_room->y < orig_ship->ship->in_room->y + y &&
                     ship->ship->in_room->y > orig_ship->ship->in_room->y - y) &&
		     ship == target_ship)
                 {
                      attack = ship;
                 }

        if (attack == NULL)
        {
            return;
        }

	npc_ship->ship->destination = attack->ship->in_room;
    npc_ship->ship->ship_chased = attack;
	npc_ship->state = NPC_SHIP_STATE_CHASING;

	if (npc_ship->captain->ship_move <= 0)
    {
	    SHIP_STATE(npc_ship->captain, 16);
    }
	npc_ship->ship->speed = SHIP_SPEED_FULL_SPEED;
#endif
}


void npc_ship_attack_ship(NPC_SHIP_DATA *npc_ship, SHIP_DATA *target_ship)
{
    char buf[MAX_STRING_LENGTH];
    SHIP_DATA *orig_ship;
    SHIP_DATA *ship;
    SHIP_DATA *attack;
    int x, y;

        x = get_squares_to_show_x(0);
        y = get_squares_to_show_y(0);

        orig_ship = npc_ship->ship;

        attack = NULL;

            // Check for sailing ship to attack
            for ( ship = ((AREA_DATA *) get_sailing_boat_area())->ship_list; ship != NULL; ship = ship->next)
                if (
                  (ship->ship->in_room->x < orig_ship->ship->in_room->x + x &&
                     ship->ship->in_room->x > orig_ship->ship->in_room->x - x)
                 && (ship->ship->in_room->y < orig_ship->ship->in_room->y + y &&
                     ship->ship->in_room->y > orig_ship->ship->in_room->y - y) &&
		     ship == target_ship)
                 {
                      attack = ship;
                 }

        if (attack == NULL)
        {
            return;
        }

        SHIP_ATTACK_STATE(npc_ship->captain, 8);

        npc_ship->ship->attack_position = SHIP_ATTACK_LOADING;
        npc_ship->ship->ship_attacked = attack;

        sprintf(buf, "%s's sailing boat is turning to aim at you.", npc_ship->ship->owner_name);

        boat_echo(attack, buf);
}


void make_npc_ship_board_ship(NPC_SHIP_DATA *npc_ship, SHIP_DATA *ship)
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *mob;
    CHAR_DATA *captain;
    int num;

    if (IS_NPC_SHIP(ship))
    {
        captain = ship->npc_ship->captain;
    }
    else
    {
        captain = ship->owner;
    }

    // Transfer mobs to boarded ship
    for (mob = npc_ship->ship->crew_list; mob != NULL; mob = mob->next_in_crew)
    {
        if (mob != npc_ship->captain)
        {

	char_from_room(mob);
	char_to_room(mob, get_room_index(ship->first_room));

	num = number_percent();
	if (num < 50)
		sprintf(buf, "%s swings in from a rope landing on the main deck.", capitalize(mob->short_descr));
	else
	if (num < 60)
		sprintf(buf, "%s climbs up the side of vessel.", capitalize(mob->short_descr));
	else
	if (num < 80)
		sprintf(buf, "%s flies in off the enemy ship's mainsail.", capitalize(mob->short_descr));
	else
	if (num < 90)
		sprintf(buf, "%s backflips onto the deck from the other vessel.", capitalize(mob->short_descr));
	else
	if (num <= 100)
		sprintf(buf, "%s jumps across onto the main deck.", capitalize(mob->short_descr));
	boat_echo(ship, buf);

        mob->boarded_ship = ship;

            p_percent_trigger( mob,NULL, NULL, NULL, captain, NULL, NULL, NULL, NULL, TRIG_BOARD , NULL);
	}
    }

    // Bring across captain
    char_from_room(npc_ship->captain);
    char_to_room(npc_ship->captain, get_room_index(ship->first_room));
    npc_ship->captain->boarded_ship = ship;
    ship->boarded_by = npc_ship->ship;

    sprintf(buf, "\n\r{YThrough the smoke and yelling you see %s board the vessel!{x", npc_ship->captain->short_descr);
    boat_echo(ship, buf);
        p_percent_trigger( npc_ship->captain,NULL, NULL, NULL, captain, NULL, NULL, NULL, NULL, TRIG_BOARD , NULL);

    // All mobs on attacked ship must know they are being attacked
    for (mob = ship->crew_list; mob != NULL; mob = mob->next_in_crew)
    {
	// if crew not at first room then come running in
        if (mob->in_room != get_room_index(ship->first_room))
        {
	    char_from_room(mob);
	    char_to_room(mob, get_room_index(ship->first_room));
        }
        mob->boarded_ship = ship;
    }

    // The captain of the attacked ship should join the fight.
    if ( IS_NPC_SHIP(ship) )
	{
		if ( ship->npc_ship->captain != NULL )
		{
			char_from_room(ship->npc_ship->captain);
			char_to_room(ship->npc_ship->captain, get_room_index(ship->first_room));
		}
	}
	else
	{
    // Make sure the player captain is actually on his/her boat before transferring them into the fray
		if (ship->owner != NULL && ON_SHIP(ship->owner) && ship->owner->in_room->ship == ship)
		{
			send_to_char("You meet your aggressors in combat!\n\r", ship->owner);
			char_from_room( ship->owner );
			char_to_room( ship->owner, get_room_index(ship->first_room));
		}
	}

    if ( captain != NULL )
    {
	    captain->boarded_ship = ship;
    }
}


void make_flee(NPC_SHIP_DATA *npc_ship, SHIP_DATA *ship)
{
    int dx, dy;
    int revdir;
    long index = 0;
    AREA_DATA *pArea;
    ROOM_INDEX_DATA *pRoom;
    int a;

    a = 15;
    // Find direction away from ship
    revdir = number_range(0, MAX_DIR-1);//rev_dir[ship->dir]];
    dx = npc_ship->ship->ship->in_room->x;
    dy = npc_ship->ship->ship->in_room->y;
    pArea = npc_ship->ship->ship->in_room->area;


    if (revdir == DIR_EAST)
    {
	dx += a;
	if (dx < pArea->map_size_x)
	{
            index = (long)((long)dy * pArea->map_size_x + (long)dx + pArea->min_vnum + WILDERNESS_VNUM_OFFSET);
	}
    }
    else
    if (revdir == DIR_WEST)
    {
	dx -= a;
	if (dx > 0)
	{
            index = (long)((long)dy * pArea->map_size_x + (long)dx + pArea->min_vnum + WILDERNESS_VNUM_OFFSET);
	}
    }
    else
    if (revdir == DIR_SOUTH)
    {
	dy += a;
	if (dy < pArea->map_size_y)
	{
            index = (long)((long)dy * pArea->map_size_x + (long)dx + pArea->min_vnum + WILDERNESS_VNUM_OFFSET);
	}
    }
    else
    if (revdir == DIR_NORTH)
    {
	dy -= a;
	if (dy > 0)
	{
            index = (long)((long)dy * pArea->map_size_x + (long)dx + pArea->min_vnum + WILDERNESS_VNUM_OFFSET);
	}
    }
    else
    if (revdir == DIR_NORTHEAST)
    {
	dy -= a;
	dx += a;
	if (dy > 0 && dx < pArea->map_size_x)
	{
            index = (long)((long)dy * pArea->map_size_x + (long)dx + pArea->min_vnum + WILDERNESS_VNUM_OFFSET);
	}
    }
    else
    if (revdir == DIR_NORTHWEST)
    {
	dy -= a;
	dx -= a;
	if (dy > 0 && dx > 0)
	{
            index = (long)((long)dy * pArea->map_size_x + (long)dx + pArea->min_vnum + WILDERNESS_VNUM_OFFSET);
	}
    }
    else
    if (revdir == DIR_SOUTHEAST)
    {
	dy += a;
 	dx += a;
	if (dy < pArea->map_size_y && dx < pArea->map_size_x)
	{
            index = (long)((long)dy * pArea->map_size_x + (long)dx + pArea->min_vnum + WILDERNESS_VNUM_OFFSET);
	}
    }
    else
    if (revdir == DIR_NORTHEAST)
    {
	dy += a;
	dx += a;
	if (dy < pArea->map_size_y && dx < pArea->map_size_x)
	{
            index = (long)((long)dy * pArea->map_size_x + (long)dx + pArea->min_vnum + WILDERNESS_VNUM_OFFSET);
	}
    }
    else
    if (revdir == DIR_NORTHWEST)
    {
	dy += a;
	dx -= a;
	if (dy < pArea->map_size_y && dx > 0)
	{
            index = (long)((long)dy * pArea->map_size_x + (long)dx + pArea->min_vnum + WILDERNESS_VNUM_OFFSET);
	}
    }

    if ((pRoom = get_room_index(index)) != NULL)
    {
	npc_ship->ship->destination = pRoom;
	npc_ship->state = NPC_SHIP_STATE_FLEEING;
	SHIP_STATE(npc_ship->captain, 16);
	npc_ship->ship->speed = SHIP_SPEED_FULL_SPEED;
	npc_ship_attack_ship(npc_ship, ship);
    }
}



/*
 * Send ships toward their destination.
 */
void ship_update( void )
{
		/*
    char buf[256];
    SHIP_DATA *ship = NULL;
    SHIP_DATA *pShip = NULL;
    CHAR_DATA *captain = NULL;
    int sx, sy, dx, dy;

    for (ship = ((AREA_DATA*) get_sailing_boat_area())->ship_list; ship != NULL; ship = ship->next)
    {
        if (IS_NPC_SHIP(ship))
        {
	    	captain = ship->npc_ship->captain;

			if ( ship->npc_ship == plith_airship && plith_airship != NULL )
			{

				if ( plith_airship->captain->ship_depart_time > 0  )
				{
					plith_airship->captain->ship_depart_time--;

					if ( plith_airship->captain->ship_depart_time == 70 )
					{
						sprintf( buf, "{DThere is a loud hiss as bursts of steam erupt from the rusty pipes.{x" );
						boat_echo( plith_airship->ship, buf );

						sprintf( buf, "The propellers on the back of the %s begin to turn!\n\r", plith_airship->ship->ship_name );
						echo_around( plith_airship->ship->ship->in_room, buf);
						room_echo( plith_airship->ship->ship->in_room, buf);
					}


					if ( plith_airship->captain->ship_depart_time == 50 )
					{
						//sprintf( buf, "{C%s says 'This is goblin eagle roger roger beetle beetle, reporting to Goblin Tower, commencing take off from %s!'{x\n\r", plith_airship->captain->short_descr, plith_airship->ship->ship->in_room->area->name );
						sprintf( buf, "{C%s says 'This is goblin eagle, reporting to Goblin Tower, commencing take off from %s!'{x\n\r", plith_airship->captain->short_descr, plith_airship->ship->ship->in_room->area->name );
						echo_around( plith_airship->ship->ship->in_room, buf);
						room_echo( plith_airship->ship->ship->in_room, buf);
						//sprintf( buf, "{C%s says 'This is goblin eagle roger roger beetle beetle, reporting to Goblin Tower, commencing take off from %s!'{x", plith_airship->captain->short_descr, plith_airship->ship->ship->in_room->area->name );
						sprintf( buf, "{C%s says 'This is goblin eagle, reporting to Goblin Tower, commencing take off from %s!'{x", plith_airship->captain->short_descr, plith_airship->ship->ship->in_room->area->name );
						boat_echo( plith_airship->ship, buf);
					}

					if ( plith_airship->captain->ship_depart_time == 20 )
					{
						sprintf( buf, "You hear a squelching noise come from the %s.\n\r", plith_airship->ship->ship_name );
						echo_around( plith_airship->ship->ship->in_room, buf);
						room_echo( plith_airship->ship->ship->in_room, buf);
						sprintf( buf, "You hear a squelching noise come from the %s.", plith_airship->ship->ship_name );
						boat_echo( plith_airship->ship, buf);
					}

					if ( plith_airship->captain->ship_depart_time == 15 )
					{
						//sprintf( buf, "{CSomeone says, '...This is Goblin Tower confirming take off. Good luck goblin eagle roger roger beetle beetle.'{x\n\r" );
						sprintf( buf, "{CSomeone says, '...This is Goblin Tower confirming take off. Good luck goblin eagle.'{x\n\r" );

						echo_around( plith_airship->ship->ship->in_room, buf);
						room_echo( plith_airship->ship->ship->in_room, buf);

						//sprintf( buf, "{CSomeone says, '...This is Goblin Tower confirming take off. Good luck goblin eagle roger roger beetle beetle.'{x" );
						sprintf( buf, "{CSomeone says, '...This is Goblin Tower confirming take off. Good luck goblin eagle.'{x" );
						boat_echo( plith_airship->ship, buf);
					}

					if ( plith_airship->captain->ship_depart_time <= 0 )
					{
						sprintf( buf, "Sand bags drop from the %s.\n\r", plith_airship->ship->ship_name );
						echo_around( plith_airship->ship->ship->in_room, buf);
						room_echo( plith_airship->ship->ship->in_room, buf);
						sprintf( buf, "Sand bags drop from the %s.", plith_airship->ship->ship_name );
						boat_echo( plith_airship->ship, buf);

						add_move_waypoint( plith_airship->ship, plith_airship->captain->ship_dest_x, plith_airship->captain->ship_dest_y );
					}
				}
			}

				if ( plith_airship != NULL && plith_airship->captain->ship_crash_time > 0  )
				{
					plith_airship->captain->ship_crash_time--;

					if ( plith_airship->captain->ship_crash_time <= 0 )
					{
						CHAR_DATA *ch = NULL;
						CHAR_DATA *ch_next = NULL;

						plith_airship->captain->ship_crash_time = -1;

						sprintf( buf, " {D ! ! ! {rK     {RA     {YB    {W    O    O    O    O    O    O    O    O    O    {YO    {RO    {rM    {D! ! !" );
						boat_echo( plith_airship->ship, buf );

						sprintf( buf, "{rYour body is squished into an unrecognizeable lump as the vessel crashes into the ground!{x" );
						boat_echo( plith_airship->ship, buf );

						sprintf( buf, "{R%s explodes as it crashes into the ground!!!{x\n\r", plith_airship->ship->ship_name );
						echo_around( plith_airship->ship->ship->in_room, buf);
						room_echo( plith_airship->ship->ship->in_room, buf);

						for ( ch = char_list; ch != NULL; ch = ch_next )
						{
							ch_next = ch->next;

							if ( !IS_NPC( ch ) && ch->in_room != NULL && ch->in_room->ship == plith_airship->ship )
							{
								raw_kill( ch, TRUE, FALSE, RAWKILL_NOCORPSE );
							}
						}

						extract_npc_boat( plith_airship );

						plith_airship = NULL;
					}
				}
		}
		else
		{
			captain = ship->owner;
		}
		// What if someone just boarded the ship and beat down the captain?
		if (IS_NPC_SHIP(ship) && ship->boarded_by == NULL)
		{
            if (captain == NULL && ship->scuttle_time <= 0)
            {
	    		sprintf(buf, "{YThe remaining crew scuttle the boat!{x");
				boat_echo(ship, buf);

				ship->scuttle_time = 5;

				ship->ship_attacked = NULL;
                ship->ship_chased = NULL;
                ship->destination = NULL;

				// RESET STATS OF NPC_BOAT
				ship->npc_ship->pShipData->ships_destroyed = 0;
				free_string(ship->npc_ship->pShipData->current_name);
				ship->npc_ship->pShipData->current_name = pirate_name_generator( );
			}
		}

        // If the npc ship is being boarded, is the captain still alive?
        if (IS_NPC_SHIP(ship) && ship->boarded_by != NULL)
        {
            if (captain == NULL)
            {
               // gecho("NPC captain has been defeated!");
			    sprintf(buf, "{WThe attacking vessel has been defeated!{x");
				boat_echo(ship, buf);

				if ( IS_NPC_SHIP(ship->boarded_by) )
				{
					sprintf(buf, "(WIZNET) NPC Boat vnum %ld has just defeated NPC Boat vnum '%ld'", ship->boarded_by->npc_ship->pShipData->vnum, ship->npc_ship->pShipData->vnum );
					log_string( buf );
	        		wiznet(buf,NULL,NULL,WIZ_DEATHS,0,0);
				}
				else
				{
					sprintf(buf, "(WIZNET) Player Boat '%s' has just defeated NPC Boat vnum '%ld'", ship->boarded_by->owner_name, ship->npc_ship->pShipData->vnum );
					log_string( buf );
	        		wiznet(buf,NULL,NULL,WIZ_DEATHS,0,0);
				}

				ship->scuttle_time = 5;

	        	ship->boarded_by->ship_attacked = NULL;
				ship->ship_attacked = NULL;
                ship->boarded_by->ship_chased = NULL;
                ship->ship_chased = NULL;
                ship->destination = NULL;
                ship->boarded_by->destination = NULL;

				// RESET STATS OF NPC_BOAT
				ship->npc_ship->pShipData->ships_destroyed = 0;
				free_string(ship->npc_ship->pShipData->current_name);
				ship->npc_ship->pShipData->current_name = pirate_name_generator( );

		stop_boarding(ship);
            }
        }

		// If the pc ship is being boarded, is the captain still alive?
		if (!IS_NPC_SHIP(ship) && ship->boarded_by != NULL)
		{
			if (captain == NULL || IS_DEAD(captain))
			{
				sprintf(buf, "{WThe boarded vessel has been defeated!{x");
				boat_echo(ship, buf);

				if ( IS_NPC_SHIP(ship->boarded_by) )
				{
					sprintf(buf, "NPC Boat vnum %ld has just defeated Player '%s's boat!", ship->boarded_by->npc_ship->pShipData->vnum, ship->owner_name );
					log_string( buf );
	        		wiznet(buf,NULL,NULL,WIZ_DEATHS,0,0);
					transfer_cargo( ship, ship->boarded_by );
				}
				else
				{
					if (ship->boarded_by != NULL &&
						ship->boarded_by->owner != NULL)
					{
						send_to_char("{YYou and your remaining crew return to your vessel.{x\n\r", ship->boarded_by->owner);
					}
					sprintf(buf, "Player '%s' has just defeated Player '%s's boat!", ship->boarded_by->owner_name, ship->owner_name );
					log_string( buf );
	        		wiznet(buf,NULL,NULL,WIZ_DEATHS,0,0);
				}

				//gecho("The player captain has been defeated!");
				//gecho("Mob has scuttled boat");
				ship->scuttle_time = 5;

				ship->boarded_by->ship_attacked = NULL;
				ship->ship_attacked = NULL;
				ship->boarded_by->ship_chased = NULL;
				ship->ship_chased = NULL;
				ship->destination = NULL;
				ship->boarded_by->destination = NULL;

				if ( IS_NPC_SHIP(ship->boarded_by) )
				{
					// INCREASE STATS OF NPC_BOAT
					ship->boarded_by->npc_ship->pShipData->ships_destroyed++;
          if ( ship->boarded_by->npc_ship->captain != NULL) {
						 ship->boarded_by->npc_ship->captain->ships_destroyed = ship->boarded_by->npc_ship->pShipData->ships_destroyed;
          }
					reset_waypoint( ship->boarded_by->npc_ship );
				}

				stop_boarding(ship);
			}
		}

		if (!IS_NPC_SHIP(ship) && ship->boarded_by != NULL)
		{
			   if (IS_NPC_SHIP(ship->boarded_by) && ship->boarded_by->npc_ship->captain != NULL)
			   {
			   gecho( ship->boarded_by->npc_ship->captain->short_descr);
			   sprintf(buf, "%ld\n\r", (ship->boarded_by->npc_ship->captain->in_room) == NULL ? -1 : ship->boarded_by->npc_ship->captain->in_room->vnum);
			   gecho(buf);
			   }
			if (IS_NPC_SHIP(ship->boarded_by) && ship->boarded_by->npc_ship->captain == NULL)
			{
				sprintf(buf, "{WThe attacking vessel has been defeated!{x");
				boat_echo(ship, buf);
			    sprintf(buf, "The remaining enemy crew return shamefully to their vessel.");
				boat_echo(ship, buf);

				ship->boarded_by->scuttle_time = 5;
				ship->boarded_by->ship_attacked = NULL;
				ship->ship_attacked = NULL;
				ship->boarded_by->ship_chased = NULL;
				ship->ship_chased = NULL;
				ship->destination = NULL;
				ship->boarded_by->destination = NULL;

				stop_boarding(ship);
			}
			else
				if (IS_NPC_SHIP(ship->boarded_by) && (ship->owner == NULL || IS_DEAD(ship->owner) || !is_on_ship(ship->owner, ship)))
				{
					boat_echo(ship, "{WWith the captain gone, the boarding party takes control of the vessel and scuttles it!{x");
					sprintf(buf, "The remaining enemy crew return shamefully to their vessel.");
					boat_echo(ship, buf);

					transfer_cargo( ship, ship->boarded_by );
					ship->scuttle_time = 5;

					//gecho("Ship has been scuttled!");
					//gecho("The player captain has been defeated!");
					//gecho("Mob has scuttled boat");
					ship->boarded_by->ship_attacked = NULL;
					ship->ship_attacked = NULL;
					ship->boarded_by->ship_chased = NULL;
					ship->ship_chased = NULL;
					ship->destination = NULL;
					ship->boarded_by->destination = NULL;

					// INCREASE STATS OF NPC_BOAT
					ship->boarded_by->npc_ship->pShipData->ships_destroyed++;
  if ( ship->boarded_by->npc_ship->captain != NULL) {
             ship->boarded_by->npc_ship->captain->ships_destroyed = ship->boarded_by->npc_ship->pShipData->ships_destroyed;
          }

					reset_waypoint( ship->boarded_by->npc_ship );

					stop_boarding(ship);

				}
				else
					if (!IS_NPC_SHIP(ship->boarded_by) && (ship->owner == NULL || IS_DEAD(ship->owner) || !is_on_ship(ship->owner, ship)))
					{
						//	gecho("Attacking pc captain has won!");
						sprintf(buf, "{WYou have successfully taken the vessel!{x");
						boat_echo(ship, buf);
						sprintf(buf, "{RThe vessel bursts into flame!{x");
						boat_echo(ship, buf);

						ship->scuttle_time = 5;

						ship->boarded_by->ship_attacked = NULL;
						ship->ship_attacked = NULL;
						ship->boarded_by->ship_chased = NULL;
						ship->ship_chased = NULL;
						ship->destination = NULL;
						ship->boarded_by->destination = NULL;

						stop_boarding(ship);
					}
					else
						if (!IS_NPC_SHIP(ship->boarded_by) && ship->boarded_by->owner != NULL && (ship->boarded_by->owner == NULL || IS_DEAD(ship->boarded_by->owner) || !is_on_ship(ship->boarded_by->owner, ship)))
						{
							//gecho("Attacking pc captain defeated");
							sprintf(buf, "{WYou have successfully fought off the attacking vessel!{x");
							boat_echo(ship, buf);
							sprintf(buf, "{RThe enemy vessel bursts into flame!{x");
							boat_echo(ship, buf);
							ship->boarded_by->scuttle_time = 5;

							ship->ship_attacked = NULL;
							ship->boarded_by->ship_attacked = NULL;
							ship->ship_chased = NULL;
							ship->boarded_by->ship_chased = NULL;
							ship->destination = NULL;
							ship->boarded_by->destination = NULL;

							stop_boarding(ship);
						}
		}

		if (captain == NULL)
		{
			continue;
		}

		if ( captain->ship_attack > 0 && (ship->char_attacked != NULL || ship->ship_attacked != NULL))
		{
			--captain->ship_attack;
			if (captain->ship_attack <= 0)
			{
				boat_attack(captain);
			}
		}

		if (ship->destination == NULL
				&& captain->ship_move > 0
				&& !IS_NPC_SHIP(ship))
		{
			--captain->ship_move;
			if (captain->ship_move <= 0)
			{
				boat_move(captain);
			}

			if (!is_on_ship(captain, ship))
			{
				ship->speed = SHIP_SPEED_STOPPED;
			}
		}

		if ( IS_NPC_SHIP(ship) && ship->ship_chased == NULL && ship->ship_attacked == NULL && ship->destination == NULL && (ship->npc_ship->pShipData->npc_type == NPC_SHIP_PIRATE || ship->npc_ship->pShipData->npc_type == NPC_SHIP_COAST_GUARD || ship->npc_ship->pShipData->npc_type == NPC_SHIP_BOUNTY_HUNTER ))
		{
			SHIP_DATA *ship2 = NULL;
			sx = ship->ship->in_room->x;
			sy = ship->ship->in_room->y;

			//log_string(ship->ship_name);
			for (ship2 = ((AREA_DATA *) get_sailing_boat_area())->ship_list; ship2 != NULL; ship2 = ship2->next)
			{
				if ( !IS_NPC_SHIP(ship2) && ship2->scuttle_time <= 0)
				{
					dx = ship2->ship->in_room->x;
					dy = ship2->ship->in_room->y;


				//sprintf( buf, "scuttle time is %d\n\r", ship2->scuttle_time );
				//gecho(buf);
				//sprintf(buf, "Ship %s is sx=%ld sy=%ld dx=%ld dy=%ld deltax=%ld deltay=%ld", ship2->ship_name, sx, sy, dx, dy, abs(dx-sx), abs(dy-sy));
				//log_string(buf);

	            if (abs(dx-sx) < 15 && abs(dy-sy) < 15)
                {
					switch( ship->npc_ship->pShipData->npc_type )
					{
						case NPC_SHIP_BOUNTY_HUNTER:

							if (ship->npc_ship->pShipData->npc_type == NPC_SHIP_BOUNTY_HUNTER &&
									!IS_NPC_SHIP(ship2) &&
									ship2->owner != NULL &&
									IS_PIRATE(ship2->owner))
							{
								npc_ship_chase_ship(ship->npc_ship, ship2);
								// Only show comment if ship was in range
								if (ship->ship_chased != NULL)
								{
									sprintf(buf, "{WA %s has turned and appears to be chasing us captain! It appears to be a bounty hunter!\n\r", boat_table[ship->ship_type].name);
									boat_echo(pShip, buf);
									npc_ship_attack_ship(ship->npc_ship, ship2);
								}
							}
							break;
						case NPC_SHIP_COAST_GUARD:

							if ( ship->npc_ship->pShipData->npc_type == NPC_SHIP_COAST_GUARD &&
									!IS_NPC_SHIP(ship2) &&
									ship2->owner != NULL &&
									((ship->npc_ship->pShipData->npc_sub_type == NPC_SHIP_SUB_TYPE_COAST_GUARD_SERALIA &&
									  ship2->owner->pcdata->rank[CONT_SERALIA] == NPC_SHIP_RANK_PIRATE) ||
									 (ship->npc_ship->pShipData->npc_sub_type == NPC_SHIP_SUB_TYPE_COAST_GUARD_ATHEMIA &&
									  ship2->owner->pcdata->rank[CONT_ATHEMIA] == NPC_SHIP_RANK_PIRATE))
							   )
							{
								npc_ship_chase_ship(ship->npc_ship, ship2);
								// Only show comment if ship was in range
								if (ship->ship_chased != NULL)
								{
									sprintf(buf, "{WA %s has turned and appears to be chasing us captain! It appears to be a military vessel!\n\r", boat_table[ship->ship_type].name);
									boat_echo(pShip, buf);
									npc_ship_attack_ship(ship->npc_ship, ship2);
								}
							}
							break;
						case NPC_SHIP_PIRATE:
							npc_ship_chase_ship(ship->npc_ship, ship2);
							// Only show comment if ship was in range
							if (ship->ship_chased != NULL)
							{
							   sprintf(buf, "{YIts the %s pirate %s! %s %s appears to be chasing us!{x\n\r",
							 	   rating_table[ get_rating( ship->npc_ship->pShipData->ships_destroyed ) ].name,
								   ship->owner_name, ship->npc_ship->captain->sex == SEX_MALE ? "His" : (ship->npc_ship->captain->sex == SEX_FEMALE ? "Her" : "Its"), boat_table[ship->ship_type].name);
								   boat_echo(ship2, buf);
							   npc_ship_attack_ship(ship->npc_ship, ship2);
							}
							break;
					}
				}
			}
		}
	}

			if ( npc_ship->pShipData->npc_type = NPC_SHIP_AIR_SHIP )
	            {
			    // Find distance between src and destination, divide by move factor
			    npc_ship->captain->ship_delta_time = (int) sqrt( 					\
				    ( npc_ship->ship->current_waypoint->x - npc_ship->captain->ship_src_x ) *	\
				    ( npc_ship->ship->current_waypoint->x - npc_ship->captain->ship_src_x ) +	\
				    ( npc_ship->ship->current_waypoint->y - npc_ship->captain->ship_src_y ) *	\
				    ( npc_ship->ship->current_waypoint->y - npc_ship->captain->ship_src_y ) ) / 5 ;

			    // this will be the delta to move the src by
			    npc_ship->ship->ship_delta_x = (npc_ship->ship->current_waypoint->x -		\
					    			npc_ship->captain->ship_src_x )	/ 		\
				    				npc_ship->captain->ship_delta_time 		\

			    // Keep a record of our original coords to add delta to
			    npc_ship->ship->ship_src_x = npc_ship->ship->in_room->x;
			    npc_ship->ship->ship_src_y = npc_ship->ship->in_room->y;

			    sprintf(buf, "{Y%s rises high into the sky then disappears!{x", npc_ship->ship->ship_name );
			    echo_around(npc_ship->ship->ship->in_room, buf);
			    room_echo(npc_ship->ship->ship->in_room, buf);
			    sprintf(buf, "{YYou feel yourself rising high into the sky!{x" );
			    boat_echo( npc_ship->ship, buf );

		    }
if ( IS_NPC_SHIP(ship) && ship->npc_ship->pShipData->npc_type == NPC_SHIP_AIR_SHIP ) {
log_string("Airship");
if (captain->ship_move == 0) {
log_string("Ship move is 0");
}
if (ship->destination == NULL) {
log_string("Destination is NULL");
}
}
	if ( IS_NPC_SHIP( ship ) && ship->npc_ship->pShipData->npc_type == NPC_SHIP_AIR_SHIP && captain->ship_move > 0 &&
			ship->destination != NULL)
	{
		long index;

            	--captain->ship_move;
		if ( captain->ship_move <= 0 )
		{
			AREA_DATA *pArea;
			AREA_DATA *pLandArea;
			ROOM_INDEX_DATA *pRoom;
			NPC_SHIP_DATA *npc_ship;
			static AREA_DATA *plith = NULL;

			if ( plith == NULL )
			{
				plith = find_area( "Plith" );
			}

			npc_ship = ship->npc_ship;

        //sprintf(buf, "Ship in room %ld %ld needs to get to %ld %ld", npc_ship->ship->ship->in_room->x, npc_ship->ship->ship->in_room->y,  npc_ship->ship->current_waypoint->x,  npc_ship->ship->current_waypoint->y );
        //log_string(buf);

			if ( npc_ship->ship->ship->in_room->x == npc_ship->ship->current_waypoint->x &&
			     npc_ship->ship->ship->in_room->y == npc_ship->ship->current_waypoint->y )
			{
				pArea = find_area_at_coords( npc_ship->ship->current_waypoint->x,
								npc_ship->ship->current_waypoint->y );

				if ( pArea == NULL )
				{
					pArea = find_area_at_land_coords( npc_ship->ship->current_waypoint->x,
								npc_ship->ship->current_waypoint->y );

					if ( pArea == NULL )
					{
						//bug( "We have a problem - airship couldn't find area at x,y", 0);
						//continue;
						sprintf( buf, "{WThe %s descends and lands.{x\n\r", ship->ship_name );
						boat_echo( npc_ship->ship, buf );
						room_echo(npc_ship->ship->ship->in_room, buf);
						echo_around(npc_ship->ship->ship->in_room, buf);
				    }
					else
					{
						sprintf( buf, "{WThe %s descends near %s and lands.{x\n\r", ship->ship_name, pArea->name );
						boat_echo( npc_ship->ship, buf );
						room_echo(npc_ship->ship->ship->in_room, buf);
						echo_around(npc_ship->ship->ship->in_room, buf);
						sector_echo( pArea, buf, SECT_CITY );
					}
				}
				else
				{
					// Send ship to another room if in Plith
					if ( pArea == plith )
					{
						if ( ( pRoom = get_room_index( 11068 ) ) == NULL )
						{
							bug( "Couldn't land airship in Plith, no recall for area!", 0 );
							continue;
						}
					}
					else
					if ( ( pRoom = get_room_index( pArea->recall ) ) == NULL )
					{
						bug( "Couldn't land airship, no recall for area!", 0 );
						continue;
					}

					obj_from_room( npc_ship->ship->ship );
					obj_to_room( npc_ship->ship->ship, pRoom );

					sprintf( buf, "{WThe %s descends into %s and lands.{x", ship->ship_name, ship->ship->in_room->area->name );
					boat_echo( npc_ship->ship, buf );

					sprintf( buf, "{YYou hear a loud roar from above as the %s descends into %s.{x", ship->ship_name, pArea->name );
					sector_echo( pArea, buf, SECT_CITY );

				}

				clear_waypoints( ship );

				//gecho(buf);

        sprintf(buf, "Looking for an area at land coords %ld %ld", npc_ship->ship->ship->in_room->x, npc_ship->ship->ship->in_room->y );
        log_string(buf);
				pLandArea = find_area_at_land_coords( npc_ship->ship->ship->in_room->x, npc_ship->ship->ship->in_room->y );

				// Hopefully not null but better check anyway
				if ( pLandArea == NULL )
				{
					//pLandArea = plith;
					bug("pLandArea was NULL. Might have thought it was Plith???", 0);
				}

				// If the x,y coords are both 0 then assume Plith
				if ( npc_ship->ship->ship->in_room->x == 0 &&
				     npc_ship->ship->ship->in_room->y == 0 )
				{
					pLandArea = plith;
				}

				if (pLandArea != NULL)
				{
					sprintf( buf, "{YYou hear a loud roar from above as the %s descends into %s.{x\n\r", ship->ship_name, pLandArea->name );
					echo_around(npc_ship->ship->ship->in_room, buf);
					room_echo(npc_ship->ship->ship->in_room, buf);

					boat_echo( npc_ship->ship, "You hear a loud buzz from the airship intercom." );
					sprintf( buf, "{C%s says, 'We have successfully landed in %s.'{x", npc_ship->captain->short_descr, pLandArea->name );
					boat_echo( npc_ship->ship, buf );
				}
				else
				{
					sprintf( buf, "{YYou hear a loud roar from above as the %s descends.{x\n\r", ship->ship_name );
					echo_around(npc_ship->ship->ship->in_room, buf);
					room_echo(npc_ship->ship->ship->in_room, buf);

					boat_echo( npc_ship->ship, "You hear a loud buzz from the airship intercom." );
					sprintf( buf, "{C%s says, 'We have successfully landed.'{x", npc_ship->captain->short_descr );
					boat_echo( npc_ship->ship, buf );
				}

				sprintf( buf, "{C%s says, 'We at Goblin Airships hope you have enjoyed your flight and hope to see you again soon.'{x", npc_ship->captain->short_descr);
				boat_echo( npc_ship->ship, buf );

				// Only set the go-back-to-plith timer if we aren't in Plith.
				if ( pLandArea != plith )
				{
					npc_ship->captain->ship_arrival_time = (time_info.hour + 2) % 24;
					sprintf( buf, "Just set the Plith Airship go-back-to-plith timer to %d",
							npc_ship->captain->ship_arrival_time );
					log_string( buf );

				}

				ship->destination = NULL;
				ship->current_waypoint = NULL;
				npc_ship->state = NPC_SHIP_STATE_STOPPED;
				ship->speed = SHIP_SPEED_STOPPED;
				//gecho("SETTOSTOP");
			}
			else
			{
				int x, y;
				CHAR_DATA *captain;
	        		DESCRIPTOR_DATA *d;

				pArea = npc_ship->ship->ship->in_room->area;
				captain = npc_ship->captain;

log_string("It hasn't  reached the waypoint");
				x = (int)(captain->ship_src_x + captain->ship_delta_x * (float)captain->ship_time);
				y = (int)(captain->ship_src_y + captain->ship_delta_y * (float)captain->ship_time);
				if ( captain->ship_time == (int)captain->ship_delta_time )
				{
					x = npc_ship->ship->current_waypoint->x;
					y = npc_ship->ship->current_waypoint->y;
				}
				else
				{
					captain->ship_time++;
				}

//			    sprintf( buf, "%f %f %f \n\r", npc_ship->captain->ship_delta_time, npc_ship->captain->ship_delta_x, npc_ship->captain->ship_delta_y );
//			    gecho( buf );
				x = npc_ship->ship->ship->in_room->x + captain->ship_delta_x;
				y = npc_ship->ship->ship->in_room->y + captain->ship_delta_y;

    		   	index = (long)((long)y * pArea->map_size_x + (long)x + pArea->min_vnum + WILDERNESS_VNUM_OFFSET);

//				sprintf( buf, "%ld %ld %ld\n\r", x, y, index );
//				gecho( buf );

				if ( ( pRoom = get_room_index( index ) ) == NULL )
				{
					bug( "Couldn't find room to move airship to.", 0 );
					continue;
				}

				obj_from_room( npc_ship->ship->ship );
				obj_to_room( npc_ship->ship->ship, pRoom );
//				gecho("move");

				for ( d = descriptor_list; d != NULL; d = d->next )
				{
				   CHAR_DATA *victim;

			     	   victim = d->original ? d->original : d->character;

				   if ( d->connected == CON_PLAYING && victim->in_room->ship == npc_ship->ship )
				   {
        				do_function(victim, &do_survey, "auto" );
				   }
				}

				SHIP_STATE(captain, 8);
			}
		}
	}
	else
	if (ship->destination != NULL && captain->ship_move > 0)
        {
            --captain->ship_move;

	    // Is destination still valid?
	    if (!IS_NPC_SHIP(ship) && ship->ship_chased != NULL && ship->destination != ship->ship_chased->ship->in_room)
            {
		ship->destination = ship->ship_chased->ship->in_room;
            }

            if (captain->ship_move <= 0)
	    {
 	    int old_dir;
            old_dir = ship->dir;
	    sx = ship->ship->in_room->x;
            sy = ship->ship->in_room->y;
	    dx = ship->destination->x;
            dy = ship->destination->y;

//             sprintf(buf, "Destination is %ld %ld currently at %ld %ld\n\r", dx,dy,sx,sy);
//            gecho(buf);

	    if (abs(dy-sx) < 5 && abs(dy-sx) < 5)
            {
		ship->speed = SHIP_SPEED_HALF_SPEED;
	    }

            if (dx < sx && dy < sy)
            {
		ship->dir = DIR_NORTHWEST;
	    }
	    else
            if (dx < sx && dy > sy)
            {
		ship->dir = DIR_SOUTHWEST;
	    }
	    else
            if (dx > sx && dy < sy)
            {
		ship->dir = DIR_NORTHEAST;
	    }
	    else
            if (dx > sx && dy > sy)
            {
		ship->dir = DIR_SOUTHEAST;
	    }
	    else
            if (dx < sx)
            {
		ship->dir = DIR_WEST;
	    }
	    else
	    if (dx > sx)
	    {
		ship->dir = DIR_EAST;
	    }
	    else
	    if (dy < sy)
	    {
		ship->dir = DIR_NORTH;
	    }
	    else
	    if (dy > sy)
	    {
		ship->dir = DIR_SOUTH;
	    }
	//gecho(dir_name[ship->dir]);
	    if (old_dir != ship->dir)
            {
	        act("{WThe vessel is now steered to the $T.{x", captain, NULL, dir_name[ship->dir], TO_ROOM);
	    }

	    ship->speed = SHIP_SPEED_FULL_SPEED;
            boat_move( captain );
	//sprintf(buf, "boat moved to room %ld\n\r", ship->ship->in_room->vnum);
	//gecho(buf);
	    sx = ship->ship->in_room->x;
            sy = ship->ship->in_room->y;
	    dx = ship->destination->x;
            dy = ship->destination->y;

	    // THIS IS ONLY FOR PLAYER SHIPS. PLAYER SHIPS DONT HAVE A STOP AND WAIT WAYPOINT LIKE NPCS
			if ( sx == dx && sy == dy && !IS_NPC_SHIP(ship) && ship->current_waypoint != NULL)
			{
				long index;
				boat_echo(ship, "{YReached Waypoint!{x");
				boat_echo(ship, "{WNow progressing to next waypoint!{x");
				ship->current_waypoint = ship->current_waypoint->next;
				if (ship->current_waypoint == NULL)
				{
					ship->current_waypoint = ship->waypoint_list;
				}

				if ( ship->current_waypoint != NULL )
				{
					index = (long)((long)ship->current_waypoint->y * ship->ship->in_room->area->map_size_x + (long)ship->current_waypoint->x + ship->ship->in_room->area->min_vnum + WILDERNESS_VNUM_OFFSET);
					ship->destination = get_room_index(index);
				}
			}

			if ( sx == dx && sy == dy && IS_NPC_SHIP(ship))
			{
				if (ship->npc_ship->state == NPC_SHIP_STATE_CHASING)
				{
					if (ship->ship_chased->ship->in_room == ship->destination)
					{
						sprintf(buf, "{WThe %s crashes violently up against the vessel!{x", ship->ship_name);
						boat_echo(ship->ship_chased, buf);
						ship->speed = SHIP_SPEED_STOPPED;
						ship->ship_chased->speed = SHIP_SPEED_STOPPED;
						ship->ship_chased->destination = NULL;
						ship->npc_ship->state = NPC_SHIP_STATE_BOARDING;
						ship->attack_position = SHIP_ATTACK_STOPPED;
						ship->ship_chased->attack_position = SHIP_ATTACK_STOPPED;
						make_npc_ship_board_ship(ship->npc_ship, ship->ship_chased);
						ship->ship_chased->destination = NULL;
						ship->ship_chased->ship_chased = NULL;
						ship->ship_chased = NULL;
						ship->destination = NULL;
					}
				}

				if (ship->npc_ship->state == NPC_SHIP_STATE_FLEEING)
				{
					if ((pShip = is_being_attacked_by_ship(ship)) != NULL)
					{
						//gecho("Yes its STILL being attacked");
						make_flee(ship->npc_ship, pShip);
						continue;
					}
				}

				ship->speed = SHIP_SPEED_STOPPED;
				ship->destination = NULL;

				// Dont find a new current waypoint if we are chasing.
				if (ship->npc_ship->state != NPC_SHIP_STATE_CHASING &&
						ship->npc_ship->state != NPC_SHIP_STATE_BOARDING)
				{

					ship->npc_ship->state = NPC_SHIP_STATE_STOPPED;
					//gecho("SETTOSTOP2");
					ship->current_waypoint = ship->current_waypoint->next;
					if (ship->current_waypoint == NULL)
					{
						ship->current_waypoint = ship->waypoint_list;
					}
				}

				// Remove ship trail by nullifying the last room it was in
				ship->last_room[0] = NULL;
				ship->last_room[1] = NULL;
				ship->last_room[2] = NULL;

				//gecho("Stopped...\n\r");

			}

			// If normal ship then stop it
			if ( sx == dx && sy == dy && !IS_NPC_SHIP(ship))
			{
				ship->speed = SHIP_SPEED_STOPPED;
				ship->destination = NULL;
				ship->last_room[0] = NULL;
				ship->last_room[1] = NULL;
				ship->last_room[2] = NULL;
				sprintf(buf, "{WThe %s crashes violently up against the persued vessel!{x", boat_table[ship->ship_type].name);
				boat_echo(ship, buf);
			}
		}
		}
	}
*/
}


void ship_hotspot_update()
{
    int counter;
    float length;
    float dx, dy;
    SHIP_DATA *ship;
    int x, y, type;

    counter = 0;
    while ( npc_ship_hotspot_table[counter].x != 0 )
    {
		//ship = ((AREA_DATA *) get_sailing_boat_area())->ship_list;

		for (ship = ((AREA_DATA *) get_sailing_boat_area())->ship_list; ship != NULL; ship = ship->next)
		{
		   x = npc_ship_hotspot_table[counter].x;
		   y = npc_ship_hotspot_table[counter].y;
		   type = npc_ship_hotspot_table[counter].type;

		   if ( ship->ship->in_room == NULL )
		   {
			   continue;
		   }
		   dx = (ship->ship->in_room->x - x)*(ship->ship->in_room->x - x);
		   dy = (ship->ship->in_room->y - y)*(ship->ship->in_room->y - y);
		   length = (float) sqrt( (double)( dy + dx ) );

			   //sprintf(buf, "%d,%d  %d, %d %f %f %f", x,y, ship->ship->in_room->x, ship->ship->in_room->y, (float)dx, (float)dy, (float)length);
		   //gecho(buf);
		   //send_to_char(buf, ch);
		}
		counter++;
    }
}


/*
 * Update the scuttle status of a ship.
 */
void update_scuttle()
{
    SHIP_DATA *ship;
    SHIP_DATA *nship;
    ROOM_INDEX_DATA *pRoom;
    OBJ_DATA *room_flame;
    OBJ_DATA *darkness;
    CHAR_DATA *ch;
    int i;
    log_string("Update scuttle on ships...");

    for (ship = ((AREA_DATA *) get_sailing_boat_area())->ship_list;
         ship != NULL;
	 ship = nship)
    {
	nship = ship->next;
        if (ship->scuttle_time > 0)
	{
	if (number_percent() > 80)
        {
            boat_echo(ship, "{RThe heat from the fire ignites a store of gunpowder!{x");
	    for (i = 0; i < MAX_SHIP_ROOMS; i++)
            {
                pRoom = ship->ship_rooms[i];
                if (pRoom == NULL)
                {
	            continue;
                }

	        for (ch = ship->ship_rooms[i]->people; ch != NULL; ch = ch->next_in_room)
                {
		    if (!IS_NPC(ch))
                    {
			act("You hear a whistle as the air around you ignites tearing your body apart!", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
                    }
			     raw_kill( ch, FALSE, TRUE, RAWKILL_EXPLODE );
                 }
             }
        }

            switch( ship->scuttle_time )
            {
		 /* First step */
                 case 4:
		 boat_echo(ship, "{RFlames rip up through the main sail, and burning ash falls to the deck below.{x");
	         for (i = 0; i < MAX_SHIP_ROOMS; i++)
                 {
                     pRoom = ship->ship_rooms[i];
                     if (pRoom == NULL)
                     {
			continue;
                     }

                     room_flame = create_object( get_obj_index( OBJ_VNUM_INFERNO ), 0, TRUE );
                     obj_to_room(room_flame, pRoom);
                 }
		 break;
  	         case 3:
		 boat_echo(ship, "{DDark thick smoke bellows in from all directions!{x");
	         for (i = 0; i < MAX_SHIP_ROOMS; i++)
                 {
                     pRoom = ship->ship_rooms[i];
                     if (pRoom == NULL)
                     {
			continue;
                     }
		     darkness = create_object( get_obj_index( OBJ_VNUM_ROOM_DARKNESS ), 0, TRUE );
                     obj_to_room(darkness, pRoom);
                  }
                 break;
		 case 2:
		 boat_echo(ship, "{CThe vessel shudders and shakes as it begins to take on water and sink!{x");
                 break;
                 case 1:
                 boat_echo(ship, "{BThe vessel sinks beneath the ocean!{x");
                 nship = ship->next;
		 if ( IS_NPC_SHIP(ship) )
                 {
		     extract_npc_boat(ship->npc_ship);
                 }
		 else
		 {
		     extract_boat(ship);
		 }
		 break;
	    }
	}
        ship->scuttle_time--;
    }
}


void npc_ship_state_update()
{
    NPC_SHIP_DATA *npc_ship;
    char buf[MAX_STRING_LENGTH];

    for (npc_ship = npc_ship_list; npc_ship != NULL; npc_ship = npc_ship->next)
    {
	AREA_DATA *pArea;
	ROOM_INDEX_DATA *pRoom;

  // If the ship has no captain don't bother
  if (npc_ship->captain == NULL) {
	  continue;
  }
	//if (npc_ship->state == NPC_SHIP_STATE_SAILING)

	// Chasing state
	if (npc_ship->state == NPC_SHIP_STATE_CHASING)
	{
	    // Is destination still valid?
	    if (npc_ship->ship->destination != npc_ship->ship->ship_chased->ship->in_room)
	    {
		npc_ship_chase_ship(npc_ship, npc_ship->ship->ship_chased);
	    }
	}

	if (npc_ship->state == NPC_SHIP_STATE_STOPPED ||
		npc_ship->state == NPC_SHIP_STATE_SAILING)
	{
	    SHIP_DATA *pShip;

	    // Is the ship being attacked?
	    if ((pShip = is_being_attacked_by_ship(npc_ship->ship)) != NULL)
	    {
		// If we are not a pirate then...
		if (npc_ship->pShipData->npc_type != NPC_SHIP_PIRATE)
		{
		    if ( npc_ship->pShipData->npc_type == NPC_SHIP_COAST_GUARD)
		    {
			npc_ship_chase_ship(npc_ship, pShip);

			// Only show comment if ship was in range
			if (npc_ship->ship->ship_chased != NULL)
			{
			    sprintf(buf, "{WA %s has turned and appears to be chasing us captain!\n\r", boat_table[npc_ship->ship->ship_type].name);
			    boat_echo(pShip, buf);
			    npc_ship_attack_ship(npc_ship, pShip);
			}
		    }
		    else
		    {
			// Traders dont try and board
			if ( (npc_ship->pShipData->npc_type == NPC_SHIP_ADVENTURER  ||
				    npc_ship->pShipData->npc_type == NPC_SHIP_BOUNTY_HUNTER)
				&& npc_ship->ship->cannons < pShip->cannons)
			{
			    npc_ship_chase_ship(npc_ship, pShip);
			    // Only show comment if ship was in range
			    if (npc_ship->ship->ship_chased != NULL)
			    {
				sprintf(buf, "{WA %s appears to be following us sir! It is flying the flag '%s'{x\n\r", boat_table[npc_ship->ship->ship_type].name, npc_ship->ship->flag);
				boat_echo(pShip, buf);
				npc_ship_attack_ship(npc_ship, pShip);
			    }
			}
			else
			{
			    make_flee(npc_ship, pShip);
			}
		    }
		}
		else
		{
		    npc_ship_chase_ship(npc_ship, pShip);
		    // Only show comment if ship was in range
		    if (npc_ship->ship->ship_chased != NULL)
		    {
			sprintf(buf, "{YIts the %s pirate %s! %s %s appears to be chasing us!{x\n\r",
				rating_table[ get_rating( npc_ship->pShipData->ships_destroyed ) ].name,
				npc_ship->ship->owner_name, npc_ship->captain->sex == SEX_MALE ? "His" : (npc_ship->captain->sex == SEX_FEMALE ? "Her" : "Its"), boat_table[npc_ship->ship->ship_type].name);
			boat_echo(pShip, buf);
			npc_ship_attack_ship(npc_ship, pShip);
		    }
		}
	    }

	    if ((pShip = is_being_chased_by_ship(npc_ship->ship)) != NULL && npc_ship->ship->ship_chased == NULL && pShip->scuttle_time > 0)
	    {
		// If we are not a pirate then...
		if (npc_ship->pShipData->npc_type != NPC_SHIP_PIRATE)
		{
		    if ( npc_ship->pShipData->npc_type == NPC_SHIP_COAST_GUARD)
		    {
			npc_ship_chase_ship(npc_ship, pShip);
			// Only show comment if ship was in range
			if (npc_ship->ship->ship_chased != NULL)
			{
			    sprintf(buf, "{WA %s has turned and appears to be chasing us captain! It appears to be a military vessel!\n\r", boat_table[npc_ship->ship->ship_type].name);
			    boat_echo(pShip, buf);
			    npc_ship_attack_ship(npc_ship, pShip);
			}
		    }
		    else
		    {
			// Traders dont try and board
			if ( (npc_ship->pShipData->npc_type == NPC_SHIP_ADVENTURER  ||
				    npc_ship->pShipData->npc_type == NPC_SHIP_BOUNTY_HUNTER)
				&& npc_ship->ship->cannons < pShip->cannons)
			{
			    npc_ship_chase_ship(npc_ship, pShip);
			    // Only show comment if ship was in range
			    if (npc_ship->ship->ship_chased != NULL)
			    {
				sprintf(buf, "{WA %s appears to be following us sir! It is flying the flag '%s'{x\n\r", boat_table[npc_ship->ship->ship_type].name, npc_ship->ship->flag);
				boat_echo(pShip, buf);
				npc_ship_attack_ship(npc_ship, pShip);
			    }
			}
			else
			{
			    make_flee(npc_ship, pShip);
			}
		    }
		}
		else
		{
		    npc_ship_chase_ship(npc_ship, pShip);
		    // Only show comment if ship was in range
		    if (npc_ship->ship->ship_chased != NULL)
		    {
			sprintf(buf, "{YIts the %s pirate %s! %s %s appears to be chasing us!{x\n\r",
				rating_table[ get_rating( npc_ship->pShipData->ships_destroyed ) ].name,
				npc_ship->ship->owner_name, npc_ship->captain->sex == SEX_MALE ? "His" : (npc_ship->captain->sex == SEX_FEMALE ? "Her" : "Its"), boat_table[npc_ship->ship->ship_type].name);
			boat_echo(pShip, buf);
			npc_ship_attack_ship(npc_ship, pShip);
		    }
		}
	    }
	}

	// Stopped state
	if (npc_ship->state == NPC_SHIP_STATE_STOPPED)
	{
	    WAYPOINT_DATA *temp;


	    // Stopped state
	    // Is the ship being attacked?
	    //if ((pShip = is_being_attacked_by_ship(npc_ship->ship)) != NULL)
	    // If stopped and no current waypoint then start destination
	    if (npc_ship->ship->current_waypoint == NULL)
	    {
		temp = npc_ship->ship->waypoint_list;
		if (temp == NULL)
		{
		    //bug("NPCShip has no way waypoints.", npc_ship->pShipData->vnum);
		    //bug("no waypoints exist", 0);
		    //return;
		    continue;
		}
		npc_ship->ship->current_waypoint = temp;
	    }

	    // Stopped and waypoint exists and we have no destination?
	    if (npc_ship->ship->current_waypoint != NULL/* && npc_ship->ship->destination == NULL*/)
	    {

		if (npc_ship->ship->current_waypoint->x == 0 &&
			npc_ship->ship->current_waypoint->y == 0)
		{
		    bool start_again = FALSE;
		    int door = 0;
		    EXIT_DATA *pexit;
		    ROOM_INDEX_DATA *pRoom;

		    // If we are stopped and there is a dock next to us
		    // then echo things.
		    for (door = 0; door < MAX_DIR; door++)
		    {
			if ( npc_ship->ship->ship->in_room == NULL )
			{
			    bug( "NPC Ship was in NULL room, killed.", 0 );
			    extract_npc_boat( npc_ship );
			    continue;
			}

			if ((pexit = npc_ship->ship->ship->in_room->exit[door]) == NULL)
			{
			    continue;
			}
			if ((pRoom = pexit->u1.to_room) == NULL)
			{
			    continue;
			}
			if (pRoom->sector_type == SECT_DOCK && number_percent() < 5)
			{
			    if (npc_ship->pShipData->npc_type == NPC_SHIP_BOUNTY_HUNTER)
			    {
				int number = number_percent();
				if (number < 20)
				{
				    room_echo(pRoom, "Some mercenaries arrive carrying swords and board their vessel.\n\r");
				}
				else if (number < 40)
				{
				    room_echo(pRoom, "A band of men carry a heavy looking cannon onto a nearby ship.\n\r");
				}
				else if (number < 60)
				{
				    room_echo(pRoom, "Some sailors carrying sharp expensive looking swords disembark from their vessel.\n\r");
				}
				else if (number < 80)
				{
				    room_echo(pRoom, "A band of mercenaries disembark from their vessel carrying coffins.\n\r");
				}
				else if (number < 80)
				{
				    room_echo(pRoom, "A group of sailors carrying rashons and supplies board a nearby boat singing joyfully.\n\r");
				}
			    }
			    if (npc_ship->pShipData->npc_type == NPC_SHIP_ADVENTURER)
			    {
				int number = number_percent();
				if (number < 20)
				{
				    room_echo(pRoom, "Some crew holding supplies board a nearby ship.\n\r");
				}
				else if (number < 40)
				{
				    room_echo(pRoom, "Some adventurers board a nearby ship.\n\r");
				}
				else if (number < 60)
				{
				    room_echo(pRoom, "Some sailors carry rations and tools aboard a nearby ship.\n\r");
				}
				else if (number < 80)
				{
				    room_echo(pRoom, "You here the knocking of a hammer on a nearby ship.\n\r");
				}
				else if (number < 100)
				{
				    room_echo(pRoom, "A sailor hangs off a nearby ship, repairing a small hole.\n\r");
				}
			    }
			    if (npc_ship->pShipData->npc_type == NPC_SHIP_TRADER)
			    {
				int number = number_percent();
				if (number < 20)
				{
				    room_echo(pRoom, "Some dirty looking sailors carry some goods on board a nearby vessel.\n\r");
				}
				else if (number < 40)
				{
				    room_echo(pRoom, "Some sailors disembark from a nearby vessel carrying goods from another port.\n\r");
				}
				else if (number < 60)
				{
				    room_echo(pRoom, "You hear mad clucking of chickens as a trader desperately tries to round them into his vessel.\n\r");
				}
				else if (number < 80)
				{
				    room_echo(pRoom, "You hear the banging of a hammer from a nearby ship.\n\r");
				}
				else if (number < 100)
				{
				    room_echo(pRoom, "A dirty old sailor carries supplies onto his vessel.\n\r");
				}
			    }
			    if (npc_ship->pShipData->npc_type == NPC_SHIP_PIRATE)
			    {
				int number = number_percent();
				if (number < 20)
				{
				    room_echo(pRoom, "Some shady looking people wearing eye patches disembark from their ship.\n\r");
				}
				else if (number < 40)
				{
				    room_echo(pRoom, "You hear a brief, 'Arrrrr yyeeeee Arrrrrrrr treasure' from a nearby ship.\n\r");
				}
				else if (number < 60)
				{
				    room_echo(pRoom, "You see some shady looking people sneak back onto a nearby ship.\n\r");
				}
				else if (number < 80)
				{
				    room_echo(pRoom, "You hear a strange thomp thomp thomp, like someone with a peg leg not far away.\n\r");
				}
				else if (number < 100)
				{
				    room_echo(pRoom, "A person disembarks from a nearby ship wearing a hook and a peg leg.\n\r");
				}
			    }
			    if (npc_ship->pShipData->npc_type == NPC_SHIP_COAST_GUARD)
			    {
				int number = number_percent();
				if (number < 20)
				{
				    room_echo(pRoom, "Men dressed in military uniform load cannon balls onto a ship.\n\r");
				}
				else if (number < 40)
				{
				    room_echo(pRoom, "You hear the sound of orderly marching coming from a nearby ship.\n\r");
				}
				else if (number < 60)
				{
				    room_echo(pRoom, "Above you see a military officer in the crows nest of a boat.\n\r");
				}
				else if (number < 80)
				{
				    room_echo(pRoom, "A group of well trained troops board a nearby ship.\n\r");
				}
				else if (number < 100)
				{
				    room_echo(pRoom, "A small group of troops disembark from their ship and move off.\n\r");
				}
			    }
			}
		    }

		    // If no coords then we are waiting.
		    if (npc_ship->ship->current_waypoint->hour != 0)
		    {
			if (npc_ship->ship->current_waypoint->hour ==
				time_info.hour+1)
			{
			    start_again = TRUE;
			}
			else
			{
			    start_again = FALSE;
			}
		    }

		    if (npc_ship->ship->current_waypoint->day != 0)
		    {
			if (npc_ship->ship->current_waypoint->day ==
				time_info.day+1)
			{
			    start_again = TRUE;
			}
			else
			{
			    start_again = FALSE;
			}
		    }

		    if (npc_ship->ship->current_waypoint->month != 0)
		    {
			if (npc_ship->ship->current_waypoint->month ==
				time_info.month)
			{
			    start_again = TRUE;
			}
			else
			{
			    start_again = FALSE;
			}
		    }

		    // Move to next waypoint
		    if (start_again)
		    {
			// Echo around vessel
			if ( npc_ship->pShipData->npc_type != NPC_SHIP_AIR_SHIP )
			{
			    sprintf(buf, "{YYou hear orders being given, and the %s begins to set sail!{x\n\r", npc_ship->ship->ship_name);
			    boat_echo( npc_ship->ship, buf );
			    echo_around(npc_ship->ship->ship->in_room, buf);
			    room_echo(npc_ship->ship->ship->in_room, buf);
			}

			temp = npc_ship->ship->current_waypoint;
			if (temp->next == NULL)
			{
			    npc_ship->ship->current_waypoint = npc_ship->ship->waypoint_list;
			}
			else
			{
			    npc_ship->ship->current_waypoint = npc_ship->ship->current_waypoint->next;
			}
		    }
		}
		else
		    //if ( npc_ship->state != NPC_SHIP_STATE_SAILING )
		{
		    long index;
		    AREA_DATA *pWilds_area;

		    temp = npc_ship->ship->current_waypoint;
		    pWilds_area = find_area( "Wilderness" );

		    if ( npc_ship->ship->ship->in_room == NULL )
		    {
			bug( "npc_ship->ship->in_room was null!????", 0 );
			continue;
		    }

		    pArea = npc_ship->ship->ship->in_room->area;

		    if (npc_ship->captain == NULL)
		    {
			// Captain probably died and it shouldn't have gone this far.
			continue;
		    }

		    if ( npc_ship->pShipData->npc_type == NPC_SHIP_AIR_SHIP )
		    {
			index = (long)((long)pArea->y * pWilds_area->map_size_x + (long)pArea->x + pWilds_area->min_vnum + WILDERNESS_VNUM_OFFSET);

// This bit is a bit dodgy. The index can stuff it up
			if ( get_room_index( index ) == NULL )
			{
			index = (long)((long)temp->y * pArea->map_size_x + (long)temp->x + pArea->min_vnum + WILDERNESS_VNUM_OFFSET);
//sprintf(buf, "Index is %ld %ld %ld %ld %ld %ld", index, pArea->y, pWilds_area->map_size_x, pArea->x, pWilds_area->min_vnum);
//log_string(buf);
//log_string("Room index is null...");
			//    continue;
			}

			npc_ship->captain->ship_time = 0;

			// Keep a record of our original coords to add delta to
			if ( !str_cmp( npc_ship->ship->ship->in_room->area->name, "Wilderness" ) )
			{
			    npc_ship->captain->ship_src_x = npc_ship->ship->ship->in_room->x;
			    npc_ship->captain->ship_src_y = npc_ship->ship->ship->in_room->y;
			}
			else
			    if ( npc_ship->ship->ship->in_room->area->land_x == 0 )
			    {
				npc_ship->captain->ship_src_x = npc_ship->ship->ship->in_room->area->x;
				npc_ship->captain->ship_src_y = npc_ship->ship->ship->in_room->area->y;
			    }
			    else
			    {
				npc_ship->captain->ship_src_x = npc_ship->ship->ship->in_room->area->land_x;
				npc_ship->captain->ship_src_y = npc_ship->ship->ship->in_room->area->land_y;
			    }

			/* We don't really need to store delta time. Might just in case i need it later */
			npc_ship->captain->ship_delta_time =  (sqrt( 					\
				    ( npc_ship->ship->current_waypoint->x - npc_ship->captain->ship_src_x ) *	\
				    ( npc_ship->ship->current_waypoint->x - npc_ship->captain->ship_src_x ) +	\
				    ( npc_ship->ship->current_waypoint->y - npc_ship->captain->ship_src_y ) *	\
				    ( npc_ship->ship->current_waypoint->y - npc_ship->captain->ship_src_y ) ) / 5 ) ;

			// this will be the delta to move the src by
			npc_ship->captain->ship_delta_x = (float)(npc_ship->ship->current_waypoint->x -		\
				npc_ship->captain->ship_src_x )	/ 		\
			    npc_ship->captain->ship_delta_time;
			npc_ship->captain->ship_delta_y = (float)(npc_ship->ship->current_waypoint->y -		\
				npc_ship->captain->ship_src_y )	/ 		\
			    npc_ship->captain->ship_delta_time;

			//sprintf( buf, "%f %f %f \n\r", npc_ship->captain->ship_delta_time, npc_ship->captain->ship_delta_x, npc_ship->captain->ship_delta_y );
			//gecho( buf );

			/*
			   sprintf(buf, "{C%s says 'This is roger roger alpha beta, reporting to Goblin Tower, confirming lift off from %s!{x", npc_ship->ship->ship_name, npc_ship->ship->area->name );
			   echo_around(npc_ship->ship->ship->in_room, buf);
			   room_echo(npc_ship->ship->ship->in_room, buf);
			   boat_echo( npc_ship->ship, buf );
			 */

			sprintf(buf, "{YUp above the roof tops, you catch a glimpse of the %s take off out of %s.{x", npc_ship->ship->ship_name, npc_ship->ship->ship->in_room->area->name );
			sector_echo(npc_ship->ship->ship->in_room->area, buf, SECT_CITY );

			sprintf(buf, "{YThe %s rises high into the sky then disappears!{x\n\r", npc_ship->ship->ship_name );
			echo_around(npc_ship->ship->ship->in_room, buf);
			room_echo(npc_ship->ship->ship->in_room, buf);

			sprintf(buf, "{YThe %s begins to move and you feel yourself rising high into the sky!{x", npc_ship->ship->ship_name );
			boat_echo( npc_ship->ship, buf );

			obj_from_room( npc_ship->ship->ship );
			obj_to_room( npc_ship->ship->ship, get_room_index( index ) );
		    }
		    else
		    {
			index = (long)((long)temp->y * pArea->map_size_x + (long)temp->x + pArea->min_vnum + WILDERNESS_VNUM_OFFSET);
		    }

		    if ((pRoom = get_room_index(index)) == NULL)
		    {
			bug("Couldn't find room: ", index);
			//sprintf( buf, "%ld %ld %ld %ld %ld\n\r", index, temp->y, temp->x, pArea->map_size_x, pArea->min_vnum );
			//gecho( buf );
			return ;
		    }

		    npc_ship->ship->destination = pRoom;
		    if (npc_ship->captain != NULL)
		    {
			SHIP_STATE(npc_ship->captain, 16);
		    }

		    if (npc_ship->state == NPC_SHIP_STATE_STOPPED)
		    {
		    }

		    npc_ship->ship->speed = SHIP_SPEED_FULL_SPEED;
		    npc_ship->state = NPC_SHIP_STATE_SAILING;
		    npc_ship->captain->ship_arrival_time = -1;

		    //sprintf(buf, "Vnum %ld: Waypoint going to %d %d\n\r", npc_ship->pShipData->vnum, npc_ship->ship->current_waypoint->x, npc_ship->ship->current_waypoint->y);
		    //gecho(buf);
		}
	    }
	}
    }
}
#endif

/*
 * After an npc ship has killed a ship it needs to continue with its waypoints.
 */
void reset_waypoint( NPC_SHIP_DATA *npc_ship )
{
#if 0
    long index = 0;
    SHIP_DATA *ship = npc_ship->ship;

    if ( ship->current_waypoint != NULL && !(ship->current_waypoint->x==0 && ship->current_waypoint->y==0))
    {
	index = (long)((long)ship->current_waypoint->y * ship->ship->in_room->area->map_size_x + (long)ship->current_waypoint->x + ship->ship->in_room->area->min_vnum + WILDERNESS_VNUM_OFFSET);
	ship->destination = get_room_index(index);
    }
#endif
}


/*
 * Update trade in areas.
 */
void update_area_trade( void )
{
    AREA_DATA *pArea = NULL;
    TRADE_ITEM *pItem = NULL;
    char buf[MAX_STRING_LENGTH];

    log_string( "Updating area trade data..." );
    for ( pArea = area_first; pArea != NULL; pArea = pArea->next )
    {
	if ( pArea->trade_list == NULL )
	{
	    continue;
	}

	for ( pItem = pArea->trade_list; pItem != NULL; pItem = pItem->next )
	{
	    int time = ++pItem->replenish_current_time;

	    if ( time >= pItem->replenish_time )
	    {
		pItem->replenish_current_time = 0;
		pItem->qty += pItem->replenish_amount;
		if ( pItem->qty < 0 )
		{
		    pItem->qty = 0;
		}

		if ( pItem->qty > pItem->max_qty )
		{
		    pItem->qty = pItem->max_qty;
		}

		/* Is the area a supplier of the trade good */
		if ( pItem->replenish_amount > 0 )
		{
		    /* As the quantity increases, the buy price drops */
		    pItem->buy_price = UMAX( (long) (pItem->max_price *
				((float)(pItem->max_qty - pItem->qty)/(float)pItem->max_qty)), pItem->min_price);

		    /* Sell price is 80% of buy price */
		    pItem->sell_price = (long) (pItem->buy_price * 0.8);

		    sprintf( buf, "Updating area %s, supplier trade item %s, to buy price %ld and sell price %ld, min price %ld max price %ld",
			    pArea->name, trade_table[ pItem->trade_type ].name, pItem->buy_price, pItem->sell_price, pItem->min_price, pItem->max_price );
		    log_string( buf );
		}
		else
		{
		    /* Otherwise we are a cosumer */
		    TRADE_ITEM *pTempTrade = NULL;
		    TRADE_ITEM *pMaxSupplier = NULL;
		    long max_qty = 0;
		    AREA_DATA *pTempArea = NULL;

		    // Look for largest max qty supplier in other areas and base sell price off that
		    for ( pTempArea = area_first; pTempArea != NULL; pTempArea = pTempArea->next )
		    {
			if ( pTempArea == pArea || abs(get_coord_distance(pArea->x, pArea->y, pTempArea->x, pTempArea->y)) > 400)
			{
			    continue;
			}

			for ( pTempTrade = pTempArea->trade_list; pTempTrade != NULL; pTempTrade = pTempTrade->next )
			{
			    if ( pTempTrade->trade_type == pItem->trade_type && pTempTrade->replenish_amount > 0 )
			    {
				if ( pTempTrade->qty > max_qty )
				{
				    max_qty = pTempTrade->qty;
				    pMaxSupplier = pTempTrade;
				}
			    }
			}
		    }

		    // What happens if there is not supplier???
		    if ( pTempTrade == NULL )
		    {
			pTempTrade = pItem;
		    }

		    // If there its lots of stock at the supplier then lower the sell price (not as much demand).
		    pItem->sell_price = UMAX( (long) (pItem->max_price * ( (float) ( pTempTrade->max_qty - pTempTrade->qty ) / (float) pTempTrade->max_qty ) ), pItem->min_price );

		    // Buying is 20% more than sell price
		    pItem->buy_price =(long) (pItem->sell_price * 1.2);
		}

	    }
	}
    }
}

// Update all chars, including mobs
void char_update(void)
{
    char buf[MSL];
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *ch_quit;
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;
    TOKEN_DATA *token;
    TOKEN_DATA *token_next;
    ch_quit	= NULL;

    // Update save counter
    save_number++;
    if (save_number > 29)
	save_number = 0;

    for (ch = char_list; ch != NULL; ch = ch_next)
    {
        if (!IS_VALID(ch))
	{
            bug("update_char: Trying to work with an invalidated character.\n", 0);
	    break;
        }

	ch_next = ch->next;

	/* check if in_room is null for logging purposes */
	if (ch->in_room == NULL && IS_NPC(ch))
	    sprintf(buf, "char_update: null in_room on ch %s (%ld)", ch->short_descr, ch->pIndexData->vnum);


        // Characters in social aren't updated
	if (IS_SOCIAL(ch))
	    continue;

	// Update tokens on a character. Remove the one for which the timer has run out.
	for (token = ch->tokens; token != NULL; token = token_next) {
	    token_next = token->next;

	    if (IS_SET(token->flags, TOKEN_REVERSETIMER)) {
		++token->timer;
	    } else if (token->timer > 0) {
		--token->timer;
		if (token->timer <= 0) {
		    sprintf(buf, "char update: token %s(%ld) char %s(%ld) was extracted because of timer",
			    token->name, token->pIndexData->vnum, HANDLE(ch), IS_NPC(ch) ? ch->pIndexData->vnum :
			    0);
		    log_string(buf);
		    p_percent_trigger(NULL, NULL, NULL, token, NULL, NULL, NULL, NULL, NULL, TRIG_EXPIRE, NULL);
		    token_from_char(token);
		    free_token(token);
		}
	    }
	}


        // Kick out people after they idle long enough
	if (ch->timer > 30)
            ch_quit = ch;

	if (ch->position >= POS_STUNNED)
	{
            // Stranded mobs are extracted after a while
            if (0
	    && IS_NPC(ch)
            &&  ch->desc == NULL
	    &&  ch->fighting == NULL
	    &&  !IS_AFFECTED(ch,AFF_CHARM)
            &&  ch->leader == NULL
	    &&  ch->master == NULL
	    &&  ch->in_room != ch->home_room
	    &&  !IS_SET(ch->act,ACT_SENTINEL)
	    &&  number_percent() < 1)
            {
		act("$n wanders on home.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
		if(ch->home_room == NULL) {
		    extract_char(ch, TRUE);
		    continue;
		} else {
		    char_from_room(ch);
		    char_to_room(ch, ch->home_room);
		}
            }

	    // Regen hit, mana and move.
	    if (ch->hit < ch->max_hit)
		ch->hit += hit_gain(ch);
	    else
		ch->hit = ch->max_hit;

	    if (ch->mana < ch->max_mana)
		ch->mana += mana_gain(ch);
	    else
		ch->mana = ch->max_mana;

	    if (ch->move < ch->max_move)
		ch->move += move_gain(ch);
	    else
		ch->move = ch->max_move;
	}

	// Random triggers in the wilderness
	#if 0
	if (!IS_NPC(ch))
	{
	    if (ch->in_room->parent != -1)
		p_percent_trigger(NULL, NULL, ch->in_room, NULL, NULL, NULL, NULL, NULL, NULL, TRIG_RANDOM, NULL);
	}
	#endif

	if (ch->position == POS_STUNNED)
	    update_pos(ch);

        // PCs drown in the water
	if (!IS_NPC(ch)
	&&  !IS_AFFECTED(ch, AFF_SWIM)
	&&  IS_SET(ch->in_room->room_flags, ROOM_UNDERWATER)
	&&  !IS_IMMORTAL(ch))
	{
	    send_to_char("You choke and gag as your lungs fill with water!\n\r", ch);
	    act("$n thrashes about in the water gasping for air!", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
	    damage(ch, ch, ch->hit/2, TYPE_UNDEFINED, DAM_DROWNING,FALSE);
	}

	// Toxin regeneration for siths
	if (IS_SITH(ch))
	{
	    int i;

	    for (i = 0; i < MAX_TOXIN; i++)
		ch->toxin[i] += UMIN(toxin_gain(ch), 100 - ch->toxin[i]);
	}

        // Decrease challenge delay for people.
	if (!IS_NPC(ch) && ch->pcdata->challenge_delay > 0)
	    ch->pcdata->challenge_delay--;

	// Return people from the maze after a while.
	if (ch->maze_time_left > 0)
	{
   	    ch->maze_time_left--;
	    if (ch->maze_time_left <= 0)
	    {
	        send_to_char("{WThe gods have returned you to the mortal realm.{x\n\r", ch);
		return_from_maze(ch);
            }
	}

	// Return from dead
	if (ch->time_left_death > 0)
	{
	    ch->time_left_death--;

	    if (ch->time_left_death <= 0)
	    {
		send_to_char("{WThe gods take pity on you and return you to your body.\n\r", ch);
		resurrect_pc(ch);
	    }
	}

	/*
	if (ch->in_room != NULL
	&&   ch->in_room->vnum == ROOM_VNUM_AUTO_WAR
	&&   auto_war_timer > 0)
	{
	    act("{D$n disappears in a puff of smoke.{x", ch, NULL, NULL, TO_ROOM);
	    act("You have been transported back to Plith.", ch, NULL, NULL, TO_CHAR);
	    char_from_room(ch);
	    char_to_room(ch, get_room_index(ROOM_VNUM_TEMPLE));
	}
	*/

	// Updates for NON-IMM players who aren't dead.
	if (!IS_NPC(ch) && ch->tot_level < LEVEL_IMMORTAL && !IS_DEAD(ch))
	{
	    OBJ_DATA *obj;

	    // Decrease light
	    if ((obj = get_eq_char(ch, WEAR_LIGHT)) != NULL
	    &&  obj->item_type == ITEM_LIGHT
	    &&  obj->value[2] > 0)
	    {
		if (--obj->value[2] <= 0 && ch->in_room != NULL)
		{
		    --ch->in_room->light;
		    act("$p goes out.", ch, NULL, NULL, obj, NULL, NULL, NULL, TO_ROOM);
		    act("$p flickers and goes out.", ch, NULL, NULL, obj, NULL, NULL, NULL, TO_CHAR);
		    log_string("it went out");
		    extract_obj(obj);
		}
	 	else if (obj->value[2] <= 5 && ch->in_room != NULL)
		    act("$p flickers.",ch, NULL, NULL,obj, NULL, NULL,NULL,TO_CHAR);
	    }

	    // Limbo timer (doesn't apply to imms)
	    if (IS_IMMORTAL(ch))
		ch->timer = 0;

	    if (++ch->timer >= 12)
	    {
		/* remove any PURGE_IDLE tokens on the character */
		for (token = ch->tokens; token != NULL; token = token_next) {
		    token_next = token->next;
		    if (IS_SET(token->flags, TOKEN_PURGE_IDLE)) {
			sprintf(buf, "char update: token %s(%ld) char %s(%ld) was purged on idle",
				token->name, token->pIndexData->vnum, HANDLE(ch), IS_NPC(ch) ? ch->pIndexData->vnum :
				0);
			log_string(buf);
			token_from_char(token);
			free_token(token);
		    }
		}

		if (ch->was_in_room == NULL && ch->in_room != NULL)
		{
			ch->was_in_room = ch->in_room;

			if(ch->in_wilds) {
				ch->was_in_wilds = ch->in_wilds;
				ch->was_at_wilds_x = ch->at_wilds_x;
				ch->was_at_wilds_y = ch->at_wilds_y;
			}
			else if(ch->in_room->source)
			{
				ch->was_in_room_id[0] = ch->in_room->id[0];
				ch->was_in_room_id[1] = ch->in_room->id[1];
			}


		    if (ch->fighting != NULL)
			stop_fighting(ch, TRUE);

		    act("{D$n disappears into the void.{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
		    send_to_char("{DYou disappear into the void.\n\r{x", ch);

		    if (ch->level > 1)
		        save_char_obj(ch);

		    char_from_room(ch);
		    char_to_room(ch, get_room_index(ROOM_VNUM_LIMBO));
		}
	    }

	    // Reckoning effects
	    if (pre_reckoning == 0 && reckoning_timer > 0)
	    {
		int num = number_range(0,5);
		int sn = skill_lookup("lightning");
		//int attack_rand = number_percent();

		if (ch->in_room != NULL
		&&   ch->in_room->sector_type != SECT_INSIDE
		&&   !IS_SET(ch->in_room->room_flags, ROOM_INDOORS))
		{
		    switch(num)
		    {
			case 0:
			    act("{YLightning forks down into the earth from above.{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
			    break;
			case 1:
			    act("{MThe wind howls loudly then knocks you to your knees!{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
			    ch->position = POS_RESTING;
			    break;
			case 2:
			    act("{MThe sky groans loudly as the clouds above swirl chaotically.{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
			    break;
			case 3:
			    act("{YLightning crashes to the ground next to you!{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
			    break;
			case 4:
			    if (number_percent() < 5 && !IS_SET(ch->in_room->room_flags, ROOM_SAFE) && ch->fighting == NULL)
			    {
				act("{YZAAAAAAAAAAAAAAP! You are struck by a bolt from the sky...{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
				damage(ch, ch, number_range(500,30000), sn, DAM_LIGHTNING, FALSE);
			    }
			    break;
			case 5:
			    act("{YThe wind screams around you, threatening to blow you over.{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
			    break;
		    }
		}

		/*
		if (attack_rand < 15 && ch->fighting == NULL)
		{
		    CHAR_DATA *player;

		    // Find someone to SLAUGHTER
		    for (player = ch->in_room->people; player != NULL;
			 player=player->next_in_room)
		    {
			if (!IS_NPC(player)
				&& player->tot_level > 30
				&& player->fighting == NULL)
			{
			    break;
			}
		    }

		    if (player != NULL && ch != player)
		    {
			send_to_char("{RYou are overcome with blood lust!{x\n\r", ch);
			act("{Y$n screams with rage and violently attacks $N!{x", ch, NULL, player, TO_ROOM);
			set_fighting(ch, player);
		    }
		}
		*/
	    }

	    /*
	    // Challengers for POA level - under construction
	    if (1 == 0 && !str_prefix(ch->in_room->area->name, "Maze-Level") && ch->challenger == NULL)
	    {
		char buf[MAX_STRING_LENGTH];

		CHAR_DATA *challenger;
		OBJ_DATA *wield;
		OBJ_DATA *new;
		OBJ_DATA *obj;
		OBJ_DATA *obj_next;
		int i;

		challenger = create_mobile(get_mob_index(MOB_VNUM_DARK_WRAITH));

		// Now to make challenger look like a version of the player.
		sprintf(buf, challenger->short_descr, ch->name);
		free_string(challenger->short_descr);
		challenger->short_descr = str_dup(buf);

		// Long Descr
		sprintf(buf, challenger->long_descr, ch->name);
		free_string(challenger->long_descr);
		challenger->long_descr = str_dup(buf);

		// Description
		sprintf(buf, ch->description);
		free_string(challenger->description);
		challenger->description = str_dup(buf);

		// Race
		challenger->race = ch->race;
		challenger->affected_by = ch->affected_by|race_table[ch->race].aff;
		challenger->imm_flags	= ch->imm_flags | race_table[ch->race].imm;
		challenger->res_flags	= ch->res_flags | race_table[ch->race].res;
		challenger->vuln_flags	= ch->vuln_flags | race_table[ch->race].vuln;
		challenger->form	= race_table[ch->race].form;
		challenger->parts	= race_table[ch->race].parts;


		challenger->tot_level		= ch->tot_level;
		challenger->hitroll		= ch->hitroll;
		challenger->damroll		= ch->damroll;
		challenger->max_hit		= ch->max_hit;
		challenger->hit			= ch->max_hit;
		challenger->max_mana	= ch->max_hit;
		challenger->mana		= ch->max_hit;

		for (i = 0; i < 3; i++)
		    challenger->armor[i]	= ch->armor[i];

		wield = get_eq_char(ch, WEAR_WIELD);
		challenger->dam_type = wield->value[3];

		// Clone eq into trash
		for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
		{
		    if (obj->wear_loc != WEAR_NONE)
		    {
			new = create_object(get_obj_index(OBJ_VNUM_DARK_WRAITH_EQ),
				obj->level, TRUE);

			// Short descr
			sprintf(buf, new->short_descr, obj->short_descr);
			free_string(new->short_descr);
			new->short_descr = str_dup(buf);

			// Description
			sprintf(buf, new->description, obj->description);
			free_string(new->description);
			new->description = str_dup(buf);

			new->wear_loc = obj->wear_loc;
			new->wear_flags = obj->wear_flags;
			new->condition = obj->condition;
		    }
		}

		// Wear all eq just made
		for (obj = ch->carrying; obj != NULL; obj = obj_next)
		{
		    obj_next = obj->next_content;
		    wear_obj(ch, obj, FALSE);
		}

		ch->challenger = challenger;

		// Set the npc's challenger to be the player
		challenger->challenger = ch;

		char_to_room(challenger, ch->in_room);
	    }
	    */

     /* 05/29/2006 - Syn - disabling this for now JUST IN CASE it also causes segfaults,
        until me or one of the other coders can spot check it.
     // In bars you hear gossip
     {
     		if (number_percent() < 100) {
        	CHAR_DATA *mob = NULL;

          for (mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room)
          {
            if (IS_NPC(mob) && mob->fighting == NULL) {
            	break;
 						}
          }

         if (mob != NULL) {
					NPC_SHIP_DATA *npc_ship;
					char buf[MAX_STRING_LENGTH];

					for (npc_ship = npc_ship_list; npc_ship != NULL; npc_ship = npc_ship->next)
					{
		          if (npc_ship->pShipData->npc_type == NPC_SHIP_PIRATE) {
                sprintf(buf, "{C$N whispers, 'I heard recently that the %s pirate %s was seen around coordinates %ld latitude, %ld longitude.",
							 	   rating_table[ get_rating( npc_ship->pShipData->ships_destroyed ) ].name,
								   npc_ship->ship->owner_name,
                   npc_ship->ship->ship->in_room->x,
                   npc_ship->ship->ship->in_room->y);

                 act(buf, ch, NULL, mob, TO_CHAR);
                break;
              }
          }
				 }
       }
     } */

     // if not in safe, and not in chat then check if bounty hunter is onto them
    /* 05/29/2006 - Syn - Disabling for now as it's casuing segfaults
     if (!IN_CHAT(ch) &&
          IS_PIRATE(ch) &&
          !IS_SAFE(ch) &&
          number_percent() < 5) {
        CHAR_DATA *hunter = NULL;

        // create pirate hunter
        if (ch->tot_level < 60) {
   	     hunter = create_player_hunter(MOB_VNUM_PIRATE_HUNTER_1, ch);
        }
        else
        if (ch->tot_level < 90) {
   	     hunter = create_player_hunter(MOB_VNUM_PIRATE_HUNTER_2, ch);
        }
        else
        if (ch->tot_level < 120) {
   	     hunter = create_player_hunter(MOB_VNUM_PIRATE_HUNTER_3, ch);
        }

        char_to_room(hunter, ch->in_room);
        act("{W$n has arrived.", hunter, NULL, NULL, TO_ROOM);
        act("{C$n says 'You have lived long enough $N! Your time is over pirate!'{x", hunter, NULL, ch, TO_ROOM);
     }*/

	    // Vampires take sun damage
	    if (IS_OUTSIDE(ch))
		hurt_vampires(ch);

	    // Shifted slayer effects
	    if (IS_SHIFTED_SLAYER(ch) && number_percent() < 10)
	    {
		switch(number_range(0,4))
		{
		    case 0:
			act("$n snorts and shakes some of the rancid mucus from $s body.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
			break;
		    case 1:
			act("$n's lets out a deep chilling growl.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
			break;
		    case 2:
			act("$n nibbles on $s long sharp claws.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
			break;
		    case 3:
			act("$n growls at you intimidatingly.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
			break;
		}
	    }

	    // Slayers have a bad habit of attacking things.
	    if (IS_SHIFTED_SLAYER(ch) && number_percent() < 25 && ch->fighting == NULL)
	    {
		CHAR_DATA *player;

		// Find someone to SLAUGHTER
		for (player = ch->in_room->people; player != NULL; player = player->next_in_room)
		{
		    if (player->fighting == NULL && !is_safe(ch, player,FALSE))
			break;
		}

		if (player != NULL && ch != player)
		{
		    act("$n snorts loudly then viciously attacks $N!", ch, player, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
		    act("You lash out at $N uncontrollably!", ch, player, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
		    set_fighting(ch, player);
		}
	    }

	    // Anti-evil/anti-good items scorch and get dropped.
	    if (!IS_NPC(ch))
	    for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
	    {
		char buf[MAX_STRING_LENGTH];

		if ((ch->alignment < 0 && IS_OBJ_STAT(obj, ITEM_ANTI_EVIL))
		|| (ch->alignment > 0 && IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)))
		{
		    sprintf(buf, "{R$n is scorched by %s!{x", obj->short_descr);
		    act(buf, ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
		    sprintf(buf, "{RYou are scorched by %s!{x\n\r", obj->short_descr);
		    send_to_char(buf, ch);

		    if (obj->wear_loc != WEAR_NONE)
			remove_obj(ch, obj->wear_loc, TRUE);

		    do_function(ch, &do_drop, obj->name);
		    damage(ch, ch, obj->level, TYPE_UNDEFINED, DAM_NONE,FALSE);
		}
	    }

            // No magical flying over the ocean.  Physical flight is ok
	    if (ch->in_room->sector_type == SECT_WATER_NOSWIM
	    &&   ch->in_room->vnum != ROOM_VNUM_SEA_PLITH_HARBOUR
	    &&   ch->in_room->vnum != ROOM_VNUM_SEA_NORTHERN_HARBOUR
	    &&   ch->in_room->vnum != ROOM_VNUM_SEA_SOUTHERN_HARBOUR
	    &&   !IS_NPC(ch)
	    &&   is_affected(ch, gsn_fly))
	    {
		act("{MThe air sparks as the ocean's magical shield dispels your ability to fly.{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
		act("You plummet into the ocean.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
		act("{MThe air around $n sparks, $n plummets into the ocean.{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
		affect_strip(ch, gsn_fly);
	    }

            // If they are physically flying... drain movement slowly
	    if (!IS_NPC(ch) && is_affected(ch, gsn_flight)) {
			bool fall = FALSE;
			int amount, weight;
			char *reason = "Feeling exhausted";

			// Lacking WINGS?
			if(!IS_SET(ch->parts,PART_WINGS)) {
				reason = "Lacking the ability to stay airborne";
				fall = TRUE;
			} else {
				amount = get_curr_stat(ch,STAT_CON);
				amount = URANGE(3,amount,50);
				amount = number_range(10,500/amount);	// con(3)=[1,16.7], con(50)=[1,1]
				amount = UMAX(1,amount);

				weight = get_carry_weight(ch);
				if(RIDDEN(ch)) weight += get_carry_weight(RIDDEN(ch)) + size_weight[RIDDEN(ch)->size]; // plus weight of rider

				// if the weight is too high, it uses more...
				if(!IS_IMMORTAL(ch) && (number_range(0,can_carry_w(ch))) < weight)
					amount = 3 * amount / 2;

				// athletics

				// other things?

				ch->move -= amount/10;
				ch->move = UMAX(0,ch->move);

				if(number_range(0,ch->max_move/get_curr_stat(ch,STAT_CON)) > ch->move) {
					reason = "Exhausted from flying";
					fall = TRUE;
				}
			}

			if(fall) {
				affect_strip(ch,gsn_flight);
				if(	ch->in_room->sector_type == SECT_WATER_NOSWIM ||
					ch->in_room->sector_type == SECT_WATER_SWIM ||
					ch->in_room->sector_type == SECT_UNDERWATER ||
					ch->in_room->sector_type == SECT_DEEP_UNDERWATER) {
					act("$t, you plummet into the water below.", ch, NULL, NULL, NULL, NULL, reason, NULL, TO_CHAR);
					act("$t, $n plummets into the water below.", ch, NULL, NULL, NULL, NULL, reason, NULL, TO_ROOM);
					damage(ch, ch, number_range(10,100), TYPE_UNDEFINED, IS_AFFECTED(ch,AFF_SWIM)?DAM_WATER:DAM_DROWNING, FALSE);
					if(RIDDEN(ch)) damage(RIDDEN(ch), RIDDEN(ch), number_range(10,100), TYPE_UNDEFINED, IS_AFFECTED(RIDDEN(ch),AFF_SWIM)?DAM_WATER:DAM_DROWNING, FALSE);
				} else {
					act("$t, you plummet to the ground below.", ch, NULL, NULL, NULL, NULL, reason, NULL, TO_CHAR);
					act("$t, $n plummets to the ground below.", ch, NULL, NULL, NULL, NULL, reason, NULL, TO_ROOM);
					damage(ch, ch, number_range(20,250), TYPE_UNDEFINED, DAM_BASH, FALSE);
					if(RIDDEN(ch)) damage(RIDDEN(ch), RIDDEN(ch), number_range(20,250), TYPE_UNDEFINED, DAM_BASH, FALSE);
				}
			}
	    }


            // Drown them!
	    if (ch->in_room->sector_type == SECT_WATER_NOSWIM
	    &&   !IS_NPC(ch)
	    &&   ch->move <= 50
	    &&   !IS_AFFECTED(ch, AFF_FLYING))
	    {
		act("Completely exhausted, you find little energy to keep swimming.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
		act("Completely exhausted, $n stops swimming from lack of energy.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);

		if (!IS_AFFECTED(ch, AFF_SWIM))
		{
		    act("You cough and splutter as you breath in a lung full of water.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
		    act("$n coughs and splutters as $s breaths in a lung full of water.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);

		    damage(ch, ch, 30000, TYPE_UNDEFINED, DAM_DROWNING, FALSE);
		}
	    }

	    // Fire off deathtraps.
	    if (IS_SET(ch->in_room->room_flags, ROOM_DEATH_TRAP)
	    && !IS_SET(ch->in_room->room_flags, ROOM_CPK)) {// no cpk-deathtraps
		raw_kill(ch, TRUE, FALSE, RAWKILL_NORMAL);
	    }

	    // The enchanted forest saps hit,mana, and move.
	    if (ch->in_room->sector_type == SECT_ENCHANTED_FOREST
	    &&  ch->position == POS_SLEEPING)
	    {
		ch->hit = ch->hit - ch->max_hit/3;

		update_pos(ch);

		if (ch->hit <= 0)
		{
		    send_to_char("You feel yourself disintegrate into dust.\n\r", ch);
		    act("$n disintegrates into dust.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
		    raw_kill(ch, FALSE, TRUE, RAWKILL_INCINERATE);
		}

		ch->mana = ch->mana - ch->max_mana/4;
		ch->move = ch->move - ch->max_move/4;

		ch->mana = UMAX(ch->mana, 0);
		ch->move = UMAX(ch->move, 0);
	    }

	    // non-demons without a light in the demon area get fucked
	    if (IS_SET(ch->in_room->room_flags, ROOM_ATTACK_IF_DARK) && !IS_DEMON(ch))
	    {
		if (!IS_SET(ch->act, PLR_HOLYLIGHT))
		{
		    // No light = death
		    if (room_is_dark(ch->in_room))
		    {
			send_to_char("{RSomething bites you on the ass really hard.{x\n\r", ch);
			damage(ch, ch, ch->max_hit, TYPE_UNDEFINED, DAM_NONE,FALSE);
		    }
		    else // Creepy messages
		    {
			int num = number_percent();
			if (num < 10)
			    send_to_char("{YThere is a sudden flapping as something takes off out of sight.{x\n\r", ch);
			else if (num < 20)
			    send_to_char("{DYour heart pounds rapidly as you hear sounds from the darkness...{x\n\r", ch);
			else if (num < 30)
			    send_to_char("{YA sudden chill runs up your spine.\n\r{x", ch);
			else if (num < 40)
			    send_to_char("{CSomeone whispers 'Tuuuurn offff yoour liiight mooortal.'{x\n\r", ch);
			else if (num < 50)
			    send_to_char("{RProwling, inhuman eyes materialize from the shadows, then flit away at the sight of your light.{X\n\r", ch);
			else if (num < 60)
			    send_to_char("{YYou feel as if someone or something is following you.{x\n\r", ch);
			else if (num < 70)
			    send_to_char("You feel something brush past your shoulder.\n\r", ch);
			else
			    send_to_char("{YYour light flickers momentarily.{x\n\r", ch);
		    }
		}
	    }

 	    // Hints for newbs
	    if (!IS_SET(ch->comm, COMM_NOHINTS))
	    {
		char buf[MAX_STRING_LENGTH];

		send_to_char("{MHint: ", ch);
	 	sprintf(buf, "%s", hintsTable[number_percent() % 15].hint);
		send_to_char(buf, ch);
		send_to_char("{x", ch);
	    }

	    // Update conditions
	    gain_condition(ch, COND_DRUNK, -1);
	    gain_condition(ch, COND_FULL, -1);
	    gain_condition(ch, COND_STONED, -1);

		gain_condition(ch, COND_THIRST, -1);
		gain_condition(ch, COND_HUNGER, -1);
	}

	// Update affects on the character
	for (paf = ch->affected; paf != NULL; paf = paf_next)
	{
	    paf_next = paf->next;

	    if (paf->duration > 0) // spells with a finite duration
	    {
		paf->duration--;
		if (number_range(0,4) == 0 && paf->level > 0)
		  paf->level--;  // spell strength fades with time
            }
	    else if (paf->duration < 0) // infinite spells, like on eq
	    {
		;
	    }
	    else // remove worn-out spells
	    {
		if (paf_next == NULL
		||   paf_next->type != paf->type
		||   paf_next->duration > 0)
		{
		    if (paf->type > 0 && skill_table[paf->type].msg_off)
		    {
			send_to_char(skill_table[paf->type].msg_off, ch);
			send_to_char("\n\r", ch);
		    }
		}

		affect_remove(ch, paf);
	    }
	}

	if (ch->fighting == NULL)
	    update_has_done(ch);

	// Scary people can make others flee.
	if (can_scare(ch))
	    scare_update(ch);

	/* Toggle off builder flag for people who haven't built in 30 minutes. */
	if (!IS_NPC(ch) && IS_IMMORTAL(ch) && IS_SET(ch->act, PLR_BUILDING)) {
	    if ((current_time - ch->pcdata->immortal->last_olc_command)/60 >= MAX_BUILDER_IDLE_MINUTES) {
		sprintf(buf, "%d minutes have passed for %s without any OLC commands; toggling off builder flag.\n\r",
			MAX_BUILDER_IDLE_MINUTES, ch->name);
		wiznet(buf, NULL, NULL, WIZ_BUILDING, 0, 0);
		REMOVE_BIT(ch->act, PLR_BUILDING);
	    } else  // Increment #minutes built by 1
		ch->pcdata->immortal->builder->minutes++;
	}

	// Effects of poison.
	if (IS_AFFECTED(ch, AFF_POISON)
	&&  !IS_AFFECTED(ch, AFF_SLOW))
	{
	    AFFECT_DATA *poison;

	    poison = affect_find(ch->affected,gsn_poison);

	    if (poison != NULL)
	    {
		act("$n shivers and suffers.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
		send_to_char("You shiver and suffer.\n\r", ch);
		ch->set_death_type = DEATHTYPE_TOXIN;
		damage(ch, ch, poison->level/10 + 1, gsn_poison, DAM_POISON, FALSE);
	    }
	}
	// Folks on the verge of death eventually go the whole way w/o help.
	else if (ch->position == POS_INCAP && number_range(0,1) == 0)
	    damage(ch, ch, 1, TYPE_UNDEFINED, DAM_NONE,FALSE);
	else if (ch->position == POS_MORTAL)
	    damage(ch, ch, 1, TYPE_UNDEFINED, DAM_NONE,FALSE);
    }

    // Autosave and autoquit. Check that these chars still exist.
    for (ch = char_list; ch != NULL; ch = ch_next)
    {
        ch_next = ch->next;

	if (ch->desc != NULL && ch->desc->descriptor % 15 == save_number)
	    save_char_obj(ch);

        if (ch == ch_quit)
            do_function(ch, &do_quit, NULL);
    }
}


// Update all objs (performance-sensitive)
void obj_update(void)
{
	ITERATOR tit;
	TOKEN_DATA *token;
	OBJ_DATA *obj, *obj_next;
	AFFECT_DATA *paf, *paf_next;
	CHAR_DATA *rch, *rch_next;
	char *message;
	char buf[MAX_STRING_LENGTH];
	bool nuke_obj;
	int spill_contents;
	long uid[2];

	log_string("Update objects...");

	for (obj = object_list; obj != NULL; obj = obj_next) {
		obj_next = obj->next;

		// Adjust obj affects - except for people in social
		if (obj->carried_by == NULL || !IS_SOCIAL(obj->carried_by)) {
			for (paf = obj->affected; paf != NULL; paf = paf_next) {
				paf_next = paf->next;

				if (paf->duration > 0) {
					paf->duration--;

					// Affect strength fades with time
					if (number_range(0,4) == 0 && paf->level > 0)
					paf->level--;

				// Affect wears off, send message if applicable
				} else if (!paf->duration) {
					if (!paf_next || paf_next->type != paf->type || paf_next->duration > 0) {
						if (paf->type > 0 && skill_table[paf->type].msg_obj) {
							if (obj->carried_by != NULL) {
								rch = obj->carried_by;
								act(skill_table[paf->type].msg_obj, rch, NULL, NULL, obj, NULL, NULL, NULL, TO_CHAR);
							} else if (obj->in_room && obj->in_room->people) {
								rch = obj->in_room->people;
								act(skill_table[paf->type].msg_obj, rch, NULL, NULL, obj, NULL, NULL, NULL, TO_ALL);
							}
						}
					}
					affect_remove_obj(obj, paf);
				}
			}
		}

		uid[0] = obj->id[0];
		uid[1] = obj->id[1];
		// Oprog triggers - need a room to function in
		if (obj_room(obj) != NULL) {
			if (obj->progs->delay > 0) {
				if (--obj->progs->delay <= 0)
					p_percent_trigger(NULL, obj, NULL, NULL, NULL, NULL, NULL, NULL, NULL, TRIG_DELAY, NULL);

				// Make sure the object is still there before proceeding
				if(!obj->valid || obj->id[0] != uid[0] || obj->id[1] != uid[1])
					continue;
			}

			if (!obj->locker)
				p_percent_trigger(NULL, obj, NULL, NULL, NULL, NULL, NULL, NULL, NULL, TRIG_RANDOM, NULL);
		}

		// Make sure the object is still there before proceeding
		if(!obj->valid || obj->id[0] != uid[0] || obj->id[1] != uid[1])
			continue;

		// Update tokens on object. Remove the one for which the timer has run out.
		iterator_start(&tit, obj->ltokens);
		while((token = (TOKEN_DATA *)iterator_nextdata(&tit))) {
			if (IS_SET(token->flags, TOKEN_REVERSETIMER)) {
				++token->timer;
			} else if (token->timer > 0) {
				--token->timer;
				if (token->timer <= 0) {
					sprintf(buf, "obj update: token %s(%ld) obj %s(%ld) was extracted because of timer",
							token->name, token->pIndexData->vnum, obj->short_descr, obj->pIndexData->vnum);
					log_string(buf);
					p_percent_trigger(NULL, NULL, NULL, token, NULL, NULL, NULL, NULL, NULL, TRIG_EXPIRE, NULL);
					token_from_obj(token);
					free_token(token);
				}
			}
		}
		iterator_stop(&tit);

		// Make seeds grow.
		if (obj->item_type == ITEM_SEED && obj->in_room != NULL && IS_OBJ_STAT(obj, ITEM_PLANTED)) {
			obj->value[0]--;
			if (obj->value[0] <= 0) {

				// Force this object to prevent its own destruction as it will be destroyed by default
				SET_BIT(obj->progs->entity_flags,PROG_NODESTRUCT);

				if( !p_percent_trigger(NULL, obj, NULL, NULL, NULL, NULL, NULL, NULL, NULL, TRIG_GROW, NULL) )
				{
					if (obj->value[1] == 0)
						bug("Seed has 0 vnum.", obj->pIndexData->vnum);
					else {
						OBJ_DATA *new_obj;
						if (get_obj_index(obj->value[1]) == NULL) {
							bug("Seed is buggered. Value 1 doesn't match anything:", obj->pIndexData->vnum);
							continue;
						}

						new_obj = create_object(get_obj_index(obj->value[1]), obj->level, TRUE);
						obj_to_room(new_obj, obj->in_room);

						p_percent_trigger(NULL, new_obj, NULL, NULL, NULL, NULL, NULL, NULL, NULL, TRIG_REPOP, NULL);
					}
				}
				extract_obj(obj);
			}
		}

		// Ice storms - work in PK rooms
		if (obj->in_room != NULL && obj->item_type == ITEM_ICE_STORM && is_room_pk(obj->in_room, TRUE)) {
			for (rch = obj->in_room->people; rch != NULL; rch = rch_next) {
				rch_next = rch->next_in_room;

				switch (check_immune(rch, DAM_COLD)) {
				case IS_IMMUNE:
					break;

				case IS_RESISTANT:
					act("You shiver a bit, but are able to withstand the cold.", rch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
					act("$n shivers a bit, but is able to withstand the cold.",  rch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
					break;

				default:
					act("You shiver from the intense ice storm.", rch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
					act("$n shivers from the intense ice storm.", rch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
					break;
				}
			}

			cold_effect(obj->in_room, 1, dice(4,8), TARGET_ROOM);
		}

		// Handle timers for decaying objs, etc
		if ((obj->timer <= 0 || --obj->timer > 0))
			continue;

		nuke_obj = TRUE;
		spill_contents = 100;

		switch (obj->item_type)
		{
		// Simple messages
		default:					message = "$p crumbles into dust."; break;
		case ITEM_FOUNTAIN:			message = "$p dries up."; break;
		case ITEM_ROOM_FLAME:		message = "{DThe flames die down and disappear.{x"; break;
		case ITEM_ROOM_DARKNESS:	message = "{YThe light returns.{x"; break;
		case ITEM_ROOM_ROOMSHIELD:	message = "{YThe energy field shielding the room fades away.{X"; break;
		case ITEM_STINKING_CLOUD:	message = "{YThe poisonous haze disappears.{x"; break;
		case ITEM_WITHERING_CLOUD:	message = "{YThe poisonous haze disappears.{x"; break;
		case ITEM_FOOD:				message = "$p decomposes."; break;
		case ITEM_ICE_STORM:		message = "{W$p dies down and melts.{x"; break;
		case ITEM_POTION:			message = "$p has evaporated from disuse.";break;
		case ITEM_TATTOO:			message = "$p fades away as the ink dries.";break;
		case ITEM_PORTAL:			message = "$p fades out of existence."; break;

		// Corpse decaying
		case ITEM_CORPSE_NPC:
		case ITEM_CORPSE_PC:
			message = corpse_info_table[CORPSE_TYPE(obj)].decay_message;

			if (obj->carried_by)
				act(message, obj->carried_by, NULL, NULL, obj, NULL, NULL, NULL, TO_CHAR);
			else if (obj->in_room && obj->in_room->people)
				act(message, obj->in_room->people, NULL, NULL, obj, NULL, NULL, NULL, TO_ALL);
			message = NULL;

			if(corpse_info_table[CORPSE_TYPE(obj)].decay_type != RAWKILL_NOCORPSE) {
				spill_contents = corpse_info_table[CORPSE_TYPE(obj)].decay_spill_chance;
				set_corpse_data(obj,corpse_info_table[CORPSE_TYPE(obj)].decay_type);
				spill_contents += corpse_info_table[CORPSE_TYPE(obj)].decay_spill_chance;
				spill_contents /= 2;	// Split the difference
				nuke_obj = FALSE;
			}
			break;

		case ITEM_CONTAINER:
			if (CAN_WEAR(obj,ITEM_WEAR_FLOAT)) {
				if (obj->contains)
					message = "$p flickers and vanishes, spilling its contents on the floor.";
				else
					message = "$p flickers and vanishes.";
			} else
				message = "$p crumbles into dust.";
			break;
		}

		// Do we have any message to process?
		if(!IS_NULLSTR(message)) {
			if (obj->carried_by)
				act(message, obj->carried_by, NULL, NULL, obj, NULL, NULL, NULL, TO_CHAR);
			else if (obj->in_room && obj->in_room->people)
				act(message, obj->in_room->people, NULL, NULL, obj, NULL, NULL, NULL, TO_ALL);
		}

		// Send the contents of decaying corpses somewhere, depending on where the corpse is.
		if (spill_contents > 0 && (obj->item_type == ITEM_CORPSE_PC || obj->item_type == ITEM_CORPSE_NPC) && obj->contains) {
			OBJ_DATA *t_obj, *next_obj;

			for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj) {
				next_obj = t_obj->next_content;

				if(spill_contents >= 100 || number_percent() < spill_contents) {
					obj_from_obj(t_obj);

					if (obj->in_obj) // in another object
						obj_to_obj(t_obj,obj->in_obj);
					else if (obj->carried_by)
						obj_to_char(t_obj,obj->carried_by);
					else if (obj->in_room != NULL) // to the room
						obj_to_room(t_obj,obj->in_room);
					else { // junk it
						bug("obj_update: decaying corpse room was null!@!# extracted", 0);
						extract_obj(t_obj);
					}
				}
			}
		}

		if (nuke_obj && obj) extract_obj(obj);
	}
}


/*
 * Aggress.
 *
 * for each mortal PC
 *     for each mob in room
 *         aggress on some random PC
 *
 */
void aggr_update(void)
{
    CHAR_DATA *wch;
    CHAR_DATA *wch_next;
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA *paf, *tox, af;
    char buf[MAX_STRING_LENGTH];

    memset(&af,0,sizeof(af));

    for (wch = char_list; wch != NULL; wch = wch_next)
    {
	wch_next = wch->next;

	// if NPC then this is a good place to update casting as aggr_update runs frequently
	if (IS_NPC(wch))
	{
	    if (wch->cast > 0)
	    {
		wch->cast--;
		if (wch->cast <= 0)
		{
		    wch->cast = 0;
		    cast_end(wch);
		}
	    }

	    if (wch->bashed > 0)
	    {
		--wch->bashed;
		if (wch->bashed <= 0)
		{
		    send_to_char("You scramble to your feet!\n\r", wch);
		    act("$n scrambles to $s feet.", wch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);

		    wch->on = NULL;

		    if (wch->fighting != NULL)
			wch->position = POS_FIGHTING;
		    else
			wch->position = POS_STANDING;
		}
	    }

	    if (wch != NULL && wch->ranged > 0)
	    {
		--wch->ranged;
		if (wch->fighting != NULL && number_percent() > get_ranged_skill(wch) + get_curr_stat(wch, STAT_DEX))
		{
		    send_to_char("You lose your aim and lower your weapon.\n\r", wch);
		    wch->ranged = 0;
		}
		else
		    if (wch->ranged <= 0)
			ranged_end(wch);
	    }
	}

	if (IS_NPC(wch) && IS_SET(wch->act2, ACT2_TAKES_SKULLS)
	&&  wch->in_room->contents != NULL)
	{
	    int i;

            i = 0;
	    for (obj = wch->in_room->contents; obj != NULL; obj = obj->next_content)
	    {
		if (is_name("corpse", obj->name))
		    i++;

		if (number_percent() < 95)
		    continue;

		if (obj->item_type == ITEM_CORPSE_PC && !IS_SET(obj->extra_flags, ITEM_NOSKULL))
		{
		    sprintf(buf, "%d.corpse", i);
		    do_function(wch, &do_skull, buf);
		}
	    }
	}

	if (wch->bitten > 0 && number_percent() >
	    (get_curr_stat(wch, STAT_CON) - (wch->bitten_level - wch->tot_level)/3))
	    bitten_update(wch);

	// @@@NIB : 20070126 --------
	if(wch->in_room) {
		int chance = 0;
 		if(!IS_IMMORTAL(wch)) {
			if((tox = affect_find(wch->affected,gsn_toxic_fumes))) {
				int cough = FALSE;
				// is the mobile in a Toxic Bog?
				if(wch->in_room &&
					(IS_SET(wch->in_room->room2_flags, ROOM_TOXIC_BOG) ||
					(wch->in_room->sector_type == SECT_TOXIC_BOG))) {
					bool dec;

					if(IS_SET(wch->in_room->room2_flags, ROOM_TOXIC_BOG)) chance += 10;
					if(wch->in_room->sector_type == SECT_TOXIC_BOG) chance += 10;

					dec = (number_percent() < chance);
					for(paf = tox;paf;paf = paf->next)
						if (paf->type == gsn_toxic_fumes) {
							if(paf->duration > 0)	// Switch all non-permanent affects to permanent
								paf->duration = -paf->duration;
							else if(!paf->duration)
								paf->duration = -1;
							else if(dec && paf->duration > -100)	// Make permanent affects "longer"
								--paf->duration;
							if(paf->level < 120)
								paf->level = 120;
						}

					// Coughing messages
					if(number_range(0,199) < chance) {
						cough = TRUE;
						if(number_percent() < 50)
							send_to_char("{xYou cough uncontrollably from the toxic fumes.\n\r", wch);
						else
							send_to_char("{xYou inhale the toxic fumes, coughing uncontrollably.\n\r", wch);
						act("{x$n coughs uncontrollably from toxic fumes.", wch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
					}
				} else {
					// Change all affects to non-permanent
					for(paf = tox;paf;paf = paf->next)
						if (paf->type == gsn_toxic_fumes && paf->duration < 0)
							paf->duration = -paf->duration;

					// Coughing messages
					if(number_percent() < 4) {
						cough = TRUE;
						send_to_char("{xYou cough uncontrollably.\n\r", wch);
						act("{x$n coughs uncontrollably.", wch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
					}
				}

				// Do cough lag and damage
				if(cough) {
					cough = number_range(15,25);
					DAZE_STATE(wch,cough);
					cough = number_range(10,cough-1);
					WAIT_STATE(wch,cough);

					if(number_percent() < 50)
						damage(wch, wch, number_range(5,10), gsn_toxic_fumes, DAM_NONE, FALSE);
				}
			} else {
				if(IS_SET(wch->in_room->room2_flags, ROOM_TOXIC_BOG)) chance += 50;
				if(wch->in_room->sector_type == SECT_TOXIC_BOG) chance += 50;

				if(chance > 0 && number_percent() < chance)
					toxic_fumes_effect(wch,NULL);
			}
		}

		if(IS_SET(wch->in_room->room2_flags, ROOM_DRAIN_MANA)) {
			wch->mana -= number_range(5,15);
			if(wch->in_room->sector_type == SECT_CURSED_SANCTUM)
				wch->mana -= number_range(5,15);
			if(wch->mana < 0) wch->mana = 0;
			if(!number_percent())
				send_to_char("You feel your magical essense slipping away from you.\n\r", wch);
		}

		chance = 0;
		if(IS_SET(wch->in_room->room2_flags, ROOM_BRIARS)) chance += 5;
		if(wch->in_room->sector_type == SECT_BRAMBLE) chance += 5;

		if(chance > 0 && number_percent() < chance) {
			if(number_percent() < 2)
				send_to_char("{gThe sharp thorns scratch at your skin.{x\n\r", wch);
			damage(wch, wch, number_range(chance/2,chance), TYPE_UNDEFINED, DAM_NONE, FALSE);
		}
	}
	// @@@NIB : 20070126 --------


	if (wch->paralyzed > 0)
	{
	    --wch->paralyzed;
	    if (wch->paralyzed <= 0)
	    {
		send_to_char("You feel the power of movement coming back to your muscles.\n\r", wch);
		act("$n feels the power of movement coming back to $s muscles.",
			wch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
		wch->paralyzed = 0;
	    }
	}

	// This is for inferno, withering cloud, etc.
	if (wch->in_room != NULL
	&&  number_percent() < 10
	&&  !is_safe(wch, wch, FALSE)
	&&  ((IS_NPC(wch) && wch->pIndexData->pShop == NULL) ||
	    (IS_SET(wch->in_room->room_flags, ROOM_PK)
	     || IS_SET(wch->in_room->room_flags, ROOM_CPK))
   	     || is_pk(wch)))
	{
	    for (obj = wch->in_room->contents; obj != NULL; obj = obj->next_content)
	    {
		// Room flames (inferno)
		if (obj->item_type == ITEM_ROOM_FLAME && !IS_SET(wch->in_room->room_flags, ROOM_SAFE))
		{
		    if (number_percent() <= 2)
		    {
			act("{RYou are scorched by flames!{x",
				wch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
			act("{R$n is scorched by flames!{x",
				wch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
			damage(wch, wch, number_range(50, 500),
				TYPE_UNDEFINED, DAM_FIRE, FALSE);
		    }
		    else
		    if (number_percent() <= 2)
		    {
			/* Don't apply the blind affect twice */
			if (!IS_SET(wch->affected_by, AFF_BLIND)) {

			    act("{DYou are blinded by smoke!{x",
				    wch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
			    act("{D$n is blinded by smoke!{x",
				    wch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);

			    af.where     = TO_AFFECTS;
			    af.group     = AFFGROUP_PHYSICAL;
			    af.type      = gsn_blindness;
			    af.level     = obj->level;
			    af.location  = APPLY_HITROLL;
			    af.modifier  = -4;
			    af.duration  = 2;
			    af.bitvector = AFF_BLIND;
			    af.bitvector2 = 0;
			    affect_to_char(wch, &af);
			}
		    }
		    else
		    if (number_percent() <= 2)
		    {
			act("{RYou are scorched by flames!{x",
				wch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
			act("{R$n is scorched by flames!{x",
				wch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
			fire_effect((void *) wch,obj->level,	number_range(0, wch->tot_level * 10),TARGET_CHAR);
		    }
		}
		else
		// Withering clouds (wither spell)
		if (obj->item_type == ITEM_WITHERING_CLOUD)
		{
		    if (number_percent() <= 2)
		    {
			act("You splutter and gag!", wch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
			act("$n splutters and gags!", wch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
		    }
		    else
		    if (number_percent() <= 2)
		    {
			act("You cough and splutter!", wch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
			act("$n coughs and splutters violently!", wch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
		    }
		    if (number_percent() <= 2 && wch->fighting == NULL
		    && IS_AWAKE(wch) && wch->position == POS_STANDING
		    &&  !(IS_NPC(wch) && (IS_SET(wch->act,ACT_PROTECTED) || wch->pIndexData->pShop != NULL))
		    &&  !(!IS_NPC(wch) && IS_IMMORTAL(wch)))
		    {
			act("$n stumbles about choking and gagging!",
				wch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
			act("You stumble about choking and gagging!",
				wch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
			do_function(wch, &do_flee, "anyway");
		    }
		    else
		    if (number_percent() <= 2)
		    {
			act("$n is blinded by the toxic haze!",
				wch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
			act("You are blinded by the toxic haze around you!",
				wch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
			af.where     = TO_AFFECTS;
			af.type      = gsn_blindness;
			af.level     = obj->level;
			af.location  = APPLY_HITROLL;
			af.modifier  = -4;
			af.duration  = 2; //1+level > 3 ? 3 : 1+level;
			af.bitvector = AFF_BLIND;
			af.bitvector2 = 0;
			affect_to_char(wch, &af);
		    }
		    else
		    if (number_percent() <= 2
		    && check_immune(wch, DAM_POISON) != IS_IMMUNE)
		    {
			act("$n is poisoned by the toxic haze!",
				wch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
			act("You are poisoned by the toxic haze around you!",
				wch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
			af.where     = TO_AFFECTS;
			af.type      = gsn_poison;
			af.level     = obj->level * 3/4;
			af.duration  = URANGE(1,obj->level / 2, 5);
			af.location  = APPLY_STR;
			af.modifier  = -1;
			af.bitvector = AFF_POISON;
			af.bitvector2 = 0;
			affect_to_char(wch, &af);
		    }
		    else
		    if (number_percent() <= 2)
			acid_effect((void *)wch,obj->level,number_range(0, wch->tot_level * 10),TARGET_CHAR);
		}
		else
		// Stinking clouds (smoke bombs)
		if (obj->item_type == ITEM_STINKING_CLOUD)
		{
		    if (number_percent() <= 3)
		    {
			CHAR_DATA *victim, *vnext;

			for (victim = wch->in_room->people; victim != NULL; victim = vnext)
			{
			    vnext = victim->next_in_room;

			    if (IS_NPC(victim))
			    {
				if (victim->pIndexData->pShop != NULL
				||   IS_SET(victim->act, ACT_PROTECTED	)
				||   IS_SET(victim->act, ACT_SENTINEL	))
				    continue;
			    }

			    if (victim->tot_level <= obj->level
			    &&  (victim->tot_level > 30 || IS_REMORT(victim))// no newbs!
			    &&  victim->fighting == NULL
			    &&  victim->position == POS_STANDING
			    &&  number_percent() < 20)
			    {
				act("{GYou choke on the acrid fumes from $p!{x", victim, NULL, NULL, obj, NULL, NULL, NULL, TO_CHAR);
				act("{G$n chokes on the acrid fumes from $p!{x", victim, NULL, NULL, obj, NULL, NULL, NULL, TO_ROOM);
				do_flee(victim, "anyway");
			    }
			}
		    }
		}
	    }
	}

	/*
	if (IS_NPC(wch) && wch->boarded_ship != NULL)
	{
	    CHAR_DATA *captain;
	    if (IS_NPC_SHIP(wch->boarded_ship))
	    {
		captain = wch->boarded_ship->npc_ship->captain;
	    }
	    else
	    {
		captain = wch->boarded_ship->owner;
	    }

	    // If captain null then ship is scuttled
	    if (wch->belongs_to_ship != wch->boarded_ship && captain == NULL)
	    {
		//    gecho("Ship scuttled");
	    }

	    // It could be the mobs ship which is scuttled!
	    if (wch->belongs_to_ship == wch->boarded_ship && captain == NULL)
	    {
		//  gecho("Mobs ship scuttled");
	    }
	}

	// If mob isn't fighting and has boarded a ship then find enemy
	if (IS_NPC(wch)
	&&  wch->fighting == NULL
	&&  wch->hunting == NULL
	&&  wch->boarded_ship != NULL)
	{
	    int i;
	    bool no_enemy_left = TRUE;

	    for (i = 0; i < MAX_SHIP_ROOMS; i++)
	    {
		ROOM_INDEX_DATA *pRoom;
		if ((pRoom = wch->boarded_ship->ship_rooms[i]) != NULL)
		{
		    //gecho("look");
		    for (vch = pRoom->people; vch != NULL; vch = vch_next)
		    {
			//	gecho(vch->short_descr);
			//	gecho("\n\r");
			//	if (vch->boarded_ship != NULL)
			//	  gecho("boarded");
			vch_next = vch->next_in_room;

			sprintf(buf,
				"Currently looking at the %s who has boarded ship = %s,"
				" the other is %s who has boarded ship = %s.. "
				"they belong to %s and %s\n\r",
				vch->short_descr,
				vch->boarded_ship ? "yes" : "no",
				wch->short_descr,
				wch->boarded_ship ? "yes" : "no",
				(vch->belongs_to_ship == NULL) ? "no" :
				vch->belongs_to_ship->ship_name,
				wch->belongs_to_ship->ship_name);

			//	gecho(buf);

			if (vch != wch &&
				vch->boarded_ship == wch->boarded_ship &&
				vch->belongs_to_ship != wch->belongs_to_ship)
			{
			    no_enemy_left = FALSE;
			    //	    gecho("FOUND!@!##@!!");
			    if (wch->in_room == vch->in_room)
			    {
				set_fighting(wch, vch);
			    }
			    hunt_char(wch, vch);
			}
		    }
		}
	    }
	    if (no_enemy_left)
	    {
		//gecho("\n\r{RAll enemy have been killed!{x");
		for (vch = wch->belongs_to_ship->crew_list;
			vch != NULL;
			vch = vch_next)
		{
		    vch_next = vch->next_in_crew;
		    vch->boarded_ship = NULL;
		    char_from_room(vch);
		    char_to_room(vch,
			    get_room_index(vch->belongs_to_ship->first_room));
		}
	    }
	}

  // NPC Player hunters should be hunting players
  if (IS_NPC(wch) &&
      IS_SET(wch->act2, ACT2_PLAYER_HUNTER) &&
      wch->target_name != NULL &&
      wch->fighting == NULL) {
     CHAR_DATA *target = get_player(wch->target_name);
     bool consider_going = FALSE;

     if (target != NULL) {
     	if (wch->in_room == target->in_room) {
				set_fighting(wch, target);
      }
      else {
        if (wch->in_room->area != target->in_room->area) {
          consider_going = TRUE;
        }
        else {
          if (number_percent() < 10) {
            act("{W$n sprints off into the distance.{x", wch, NULL, NULL, TO_ROOM);
            char_from_room(wch);
            char_to_room(wch, target->in_room);
            act("{W$n has arrived.{x", wch, NULL, NULL, TO_ROOM);
          }
			    hunt_char(wch, target);
        }
     }
     }
     else {
       consider_going = TRUE;
     }

     if (number_percent() < 2 && number_percent() < 50) {
       act("$n gives up with the hunt and leaves.", wch, NULL, NULL, TO_ROOM);
       extract_char(wch, FALSE);
       continue;
     }
  }

    */
	// Stop there for NPCs; for mortal PCs, aggress
	if (IS_NPC(wch)
	||  wch->level >= LEVEL_IMMORTAL
	||  wch->in_room == NULL
	||  wch->in_room->area->empty)
	    continue;

	if (wch->boarded_ship == NULL)
	{
	    for (ch = wch->in_room->people; ch != NULL; ch = ch_next)
	    {
		int count;

		ch_next	= ch->next_in_room;

		if (!IS_NPC(ch)
		||  ch->in_room == NULL
		||  (!IS_SET(ch->act, ACT_AGGRESSIVE) && ch->boarded_ship == NULL)
		||  IS_SET(ch->in_room->room_flags, ROOM_SAFE)
		||  IS_AFFECTED(ch, AFF_CALM)
		||  ch->fighting != NULL
		||  IS_AFFECTED(ch, AFF_CHARM)
		||  !IS_AWAKE(ch)
		||  IS_SET(ch->act, ACT_WIMPY)
		||  !can_see(ch, wch)
		||  number_bits(1) == 0)
		    continue;

		// Evasion lets you get away from aggro mobs.
		if (check_evasion(wch) == TRUE)
		{
		    check_improve(wch, gsn_evasion, TRUE, 8);
		    continue;
		}
		else
		    check_improve(wch, gsn_evasion, FALSE, 8);

		// Make the NPC agressor (ch) attack a RANDOM person in the room.
		count = 0;
		victim = NULL;
		for (vch = wch->in_room->people; vch != NULL; vch = vch_next)
		{
		    vch_next = vch->next_in_room;

		    // If mob is boarding a ship then may attack anyone
		    if (ch->boarded_ship != NULL
		    &&   ch->belongs_to_ship != vch->belongs_to_ship
		    &&   can_see(ch, vch))
		    {
			if (number_range(0, count) == 0)
			    victim = vch;

			count++;
		    }
		    else
		    if (!IS_NPC(vch)
		    &&  vch->level < LEVEL_IMMORTAL
		    &&  ch->level >= vch->level - 5
		    &&  (!IS_SET(ch->act, ACT_WIMPY) || !IS_AWAKE(vch))
		    &&  can_see(ch, vch))
		    {
			if (number_range(0, count) == 0)
			    victim = vch;

			count++;
		    }
		}

		if (victim == NULL)
		    continue;

		multi_hit(ch, victim, TYPE_UNDEFINED);
	    }
	}
    }
}


// Update NPC hunting
void update_hunting(void)
{
    CHAR_DATA *mob;
    int direction = 0;

    for (mob = hunt_last; mob != NULL; mob = mob->next_in_hunting)
    {
	if (!IS_NPC(mob)
	||  number_percent() < 20
	||  mob->fighting != NULL
	||  !IS_AWAKE(mob))
	    continue;

	if (mob->hunting == NULL
	||  IS_DEAD(mob->hunting)
	||  IS_SET(mob->hunting->affected_by, AFF_HIDE)
	||  mob->hunting->in_room == NULL
	||  mob->hunting->in_room->area != mob->in_room->area
	||  IS_SET(mob->hunting->in_room->room_flags, ROOM_SAFE)
	||  !can_see(mob, mob->hunting)
	||  number_percent() < get_skill(mob->hunting, gsn_trackless_step)/2
	||  (check_evasion(mob->hunting) == TRUE && number_percent() < 33))
	    stop_hunt(mob, FALSE);
	else
	{
	    int result;

	    if (mob->in_room == NULL)
	    {
		stop_hunt(mob, TRUE);
		continue;
	    }

            // Mob has found player
  	    if (mob->in_room == mob->hunting->in_room)
	    {
		if (IS_SET(mob->in_room->room_flags,ROOM_SAFE)
		||  IS_AFFECTED(mob, AFF_CALM)
		||  IS_AFFECTED(mob, AFF_CHARM)
		||  !IS_AWAKE(mob)
		||  !can_see(mob, mob->hunting)
		||  number_bits(1) == 0
		||  check_evasion(mob->hunting) == TRUE)
		    continue;

		multi_hit(mob, mob->hunting, TYPE_UNDEFINED);
		continue;
	    }

	    if (mob->in_room == NULL || mob->hunting == NULL || mob->hunting->in_room == NULL)
	    {
		direction = -1;
		continue;
	    }
	    else
		direction = find_path(mob->in_room->vnum, mob->hunting->in_room->vnum, mob, -600, FALSE);

	    if (direction == -1)
	    {
		stop_hunt(mob, FALSE);
		continue;
	    }

	    move_char(mob, direction, FALSE);

	    result = number_range(0, 2);

            if (number_percent() < 5)
	    switch(result)
	    {
		case 0:
		    act("{DYou get the feeling something is following you...{x", mob->hunting, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
	   	    break;
		case 1:
		    act("{DYou hear footsteps behind you...{x", mob->hunting, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
	   	    break;
		case 2:
		    act("{DYou hear noises as if something is looking for you...{x", mob->hunting, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
	   	    break;
	    }
        }
    }
}


// Auto-hunt, PC version.
void update_hunting_pc(CHAR_DATA *ch)
{
    CHAR_DATA *victim = ch->hunting;
    CHAR_DATA *rch;
    int direction = 0;
    int chance;

    if (ch == NULL)
    {
	bug("update_hunting_pc: null ch!", 0);
	return;
    }

    if (victim == NULL)
    {
	bug("update_hunting_pc: null victim = ch->hunting", 0);
	return;
    }

    if (victim->in_room == NULL)
    {
	bug("update_hunting_pc: victim in_room null!", 0);
	ch->hunting = NULL;
	return;
    }

    if (ch->position != POS_STANDING)
    {
	ch->hunting = NULL;
	return;
    }

    // Chance of failing
    chance = get_skill(ch, gsn_hunt) * 3/4
             + (get_curr_stat(ch, STAT_INT)
	     +   get_curr_stat(ch, STAT_WIS)
	     +   get_curr_stat(ch, STAT_DEX)) / 5;

    if (number_percent() > chance && number_percent() < 25)
    {
	send_to_char("You lost the trail.\n\r", ch);
	act("$n has lost the trail.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
	ch->hunting = NULL;
	return;
    }

    direction = find_path(ch->in_room->vnum, victim->in_room->vnum, ch, -500, FALSE);

    if (direction == -1)
    {
	send_to_char("You lost the trail.\n\r", ch);
	ch->hunting = NULL;
	return;
    }
    else
    {
	if (number_percent() < 20)
	{
	    if (number_percent() < 20)
	    {
		send_to_char("You stop and sniff the air.\n\r", ch);
		act("$n stops and sniffs the air.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
	    }
	    else
	    if (number_percent() < 40)
	    {
		send_to_char("You analyze some tracks.\n\r", ch);
		act("$n analyzes some tracks.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
	    }
	    else
	    if (number_percent() < 60)
	    {
		send_to_char("You look around warily.\n\r", ch);
		act("$n looks around warily.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
	    }
	    else
	    if (number_percent() < 80)
	    {
		act("You scan the horizons for $N.", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
		act("$n scans the horizons for $N.", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
	    }
	    else
	    {
		act("You move towards $N.", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
		act("$n moves towards $N.", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
	    }
	}

	if (number_percent() < 5)
	    send_to_char("You get the feeling that someone is following you.\n\r", victim);

	deduct_move(ch, 5);

	move_char(ch, direction, TRUE);

	// Found them
	if (ch->in_room == victim->in_room)
	    ch->hunting = NULL;

	// For NPCs, if you are hunting one mob and come across another
	// of the same mob it stops.
	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
	{
	    if (IS_NPC(rch))
	    {
		if (rch->pIndexData == victim->pIndexData)
		{
		    ch->hunting = NULL;
		    break;
		}
	    }
	}
    }
}


// Update the pneuma relic. Add another pneuma to its stash.
void pneuma_relic_update(void)
{
    OBJ_DATA *pneuma;
    CHAR_DATA *people;
    int chance;
    char buf[MAX_STRING_LENGTH];
    log_string("Update pneuma relic...");

    if (pneuma_relic != NULL && pneuma_relic->in_room != NULL)
    {
	chance = number_percent();

        if (chance > 80)
	{
	    pneuma = create_object(get_obj_index(OBJ_VNUM_BOTTLED_SOUL), 0, TRUE);
	    obj_to_room(pneuma, pneuma_relic->in_room);

            for (people = pneuma_relic->in_room->people; people != NULL; people = people->next_in_room)
	    {
	        if (!IS_NPC(people))
	        {
		    sprintf(buf,
		    "%s glows gently then neatly drops a bottled soul.\n\r",
		        can_see_obj(people, pneuma_relic) ?
			    pneuma_relic->short_descr :
			    "Something");
		    buf[0] = UPPER(buf[0]);
		    send_to_char(buf, people);
		}
	    }
	}
    }
}


// Update relics, find out if they need to vanish.
void relic_update(void)
{
	log_string("Update all relics...");
    if (pneuma_relic != NULL)
	check_relic_vanish(pneuma_relic);

    if (damage_relic != NULL)
	check_relic_vanish(damage_relic);

    if (xp_relic != NULL)
	check_relic_vanish(xp_relic);

    if (hp_regen_relic != NULL)
	check_relic_vanish(hp_regen_relic);

    if (mana_regen_relic != NULL)
	check_relic_vanish(mana_regen_relic);
}


// Check if a relic needs to vanish from a treasure room.
void check_relic_vanish(OBJ_DATA *relic)
{
	ITERATOR cit, rit, oit;
    ROOM_INDEX_DATA *to_room;
    CHURCH_DATA *church;
    ROOM_INDEX_DATA *treasure_room;
    OBJ_DATA *obj;
    char buf[MSL];
    int chance1 = 5;
    int chance2 = 3;

    if (number_percent() < chance1 && number_percent() < chance2) {
		to_room = get_random_room(NULL, 0);

		sprintf(buf, "{M%s vanishes in a swirl of purple mist.{x\n\r", relic->short_descr);
		buf[2] = UPPER(buf[2]);
		room_echo(relic->in_room, buf);

		// Inform church the relic has vanished
		iterator_start(&cit, list_churches);
		while((church = (CHURCH_DATA *)iterator_nextdata(&cit))) {
			iterator_start(&rit, church->treasure_rooms);
			while(( treasure_room = (ROOM_INDEX_DATA *)iterator_nextdata(&rit))) {
				iterator_start(&oit, treasure_room->lcontents);
				while(( obj = (OBJ_DATA *)iterator_nextdata(&oit))) {
					if (obj == relic) {
						sprintf(buf,
							"{Y[You feel an ancient power depart your church as %s vanishes from your treasure room.]{x\n\r",
								obj->short_descr);
						church_echo(church, buf);
						break;
					}
				}
				iterator_stop(&oit);
			}
			iterator_stop(&rit);
		}
		iterator_stop(&cit);

		obj_from_room(relic);
		obj_to_room(relic, to_room);

		sprintf(buf, "relic_update: %s has vanished to %s (%ld)",
			relic->short_descr,
			to_room->name,
			to_room->vnum);
		log_string(buf);
    }
}


// Update people bitten by a sith.
void bitten_update(CHAR_DATA *ch)
{
    char buf[MAX_STRING_LENGTH];
    int percent = number_percent();

    if (number_percent() < 10)
    {
 	if (percent > 90)
	{
	    send_to_char("{RYou feel slightly uncomfortable.{x\n\r",
		    ch);
	    act("{R$n begins to look uncomfortable.{x", ch,
		    NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
	}
	else if (percent > 80)
	{
	    send_to_char("{RYou feel feverish.{x\n\r", ch);
	    act("{R$n sneezes, looking feverish.{x", ch,
		    NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
	}
	else if (percent > 70)
	{
	    sprintf(buf, "{RYou pale as the toxins race through your veins.{x\n\r");
	    act("{R$n pales as venomous toxins race through $s body.{x", ch,
		    NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
	    send_to_char(buf , ch);
	}
	else if (percent > 60)
	{
	    send_to_char("{RDizzy, you swoon back and forth.{x\n\r", ch);
	    act("{R$n swoons back and forth, dizzy.{x", ch,
		    NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
	}
	else if (percent > 50)
	{
	    send_to_char("{RYour glands swell up as poison races through them.{x\n\r", ch);
	}
	else if (percent > 40)
	{
	    send_to_char("{RYou twitch nervously as you feel an unfamiliar venom in your body.{x\n\r", ch);
	    act("{R$n twitches nervously.{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
	}
	else if (percent > 30)
	{
	    send_to_char("{RYou lose your hearing for a moment.{x\n\r", ch);
	}
	else if (percent > 20)
	{
	    send_to_char("{RYour vision goes black for a second, then returns.{x\n\r", ch);
	}
	else if (percent > 10)
	{
	    send_to_char("{RYour eyes roll back in your head.{x\n\r", ch);
	    act("{R$n's eyes roll back in $s head.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
	}
    }

    damage(ch, ch, dice(UMAX(ch->bitten_level/8, 4),1), gsn_toxins, DAM_POISON, FALSE);

    --ch->bitten;
    if (ch->bitten <= 0)
    {
        ch->bitten = 0;
	bitten_end(ch);
    }
}


// Regen sith toxins. Only happens when they're asleep.
void toxin_update(CHAR_DATA *ch)
{
    int i;

    if (IS_AWAKE(ch))
        return;

    for (i = 0; i < MAX_TOXIN; i++)
    {
	ch->toxin[i] = URANGE(0, ch->toxin[i], 100);
	ch->toxin[i] = UMIN(100, ch->toxin[i] + toxin_gain(ch));
    }
}


// Update the healing locket/ring of argyle evenhand.
void locket_update(OBJ_DATA *obj)
{
#if 0
    CHAR_DATA *ch;
    int sn;

    ch = obj->carried_by;
    if (ch == NULL)
	return;

    if (IS_AFFECTED(ch, AFF_POISON))
    {
	sn = skill_lookup("cure poison");

	act("$p shimmers softly.",
		ch, NULL, NULL, obj, NULL, NULL, NULL, TO_CHAR);
	act("$n's $p shimmers softly.",
		ch, NULL, NULL, obj, NULL, NULL, NULL, TO_ROOM);

	obj_cast_spell(sn , ch->level * 2, ch,
		ch, obj);
	return;
    }

    if (IS_AFFECTED(ch, AFF_PLAGUE))
    {
	sn = skill_lookup("cure disease");

	act("$p hums quietly for a second.",
		ch, obj, NULL, TO_CHAR);
	act("$n's $p hums quietly.",
		ch, obj, NULL, TO_ROOM);

	obj_cast_spell(sn , ch->level * 2, ch,
		ch, obj);
	return;
    }

    if (IS_AFFECTED(ch, AFF_CURSE))
    {
	sn = skill_lookup("remove curse");

	act("$p vibrates for a second, then quiets down.",
		ch, obj, NULL, TO_CHAR);
	act("$n's $p vibrates for a second.",
		ch, obj, NULL, TO_ROOM);

	obj_cast_spell(sn , ch->level * 2, ch,
		ch, obj);
	return;
    }

    if (IS_AFFECTED(ch, AFF_BLIND))
    {
	sn = skill_lookup("cure blindness");

	act("$p glimmers briefly.",
		ch, obj, NULL, TO_CHAR);
	act("$n's $p glimmers briefly.",
		ch, obj, NULL, TO_ROOM);

	obj_cast_spell(sn , ch->level * 2, ch,
		ch, obj);
	return;
    }

    if (number_percent() < 20)
    {
        // Healing
        if (ch->hit < ch->max_hit)
	{
	    act("$p glows with a vibrant blue aura.",
		    ch, obj, NULL, TO_CHAR);
	    act("$n's $p glows with a vibrant blue aura.",
		    ch, obj, NULL, TO_ROOM);

	    if (number_percent() > 60)
		sn = skill_lookup("cure critical");
	    else
		sn = skill_lookup("heal");

	    obj_cast_spell(sn, ch->level, ch, ch, obj);

	    return;
	}

	// Mana
	if (ch->mana < ch->max_mana)
	{
	    int mana;

	    act("$p glows with a vibrant purple aura.",
		    ch, obj, NULL, TO_CHAR);
	    send_to_char("You feel energized!\n\r", ch);
	    act("$n's $p glows with a vibrant purple aura.",
		    ch, obj, NULL, TO_ROOM);

	    mana = ch->max_mana/number_range(10, 20);
	    mana = UMIN(mana, ch->max_mana - mana);

	    ch->mana += mana;

	    return;
	}

	// Move
	if (ch->move < ch->max_move)
	{
	    sn = skill_lookup( "refresh");

	    act("$p glows with a vibrant green aura.",
		    ch, obj, NULL, TO_CHAR);
	    act("$n's $p glows with a vibrant green aura.",
		    ch, obj, NULL, TO_ROOM);

	    obj_cast_spell(sn, ch->level, ch, ch, obj);

	    return;
	}
    }

    if (number_percent() < 15)
    {
	switch(number_range(1, 6))
	{
	    case 1:
		if (!IS_AFFECTED(ch, AFF_SANCTUARY))
		{
		    sn = skill_lookup( "sanctuary");

		    act("$p glows with a magical aura.",
			    ch, obj, NULL, TO_CHAR);
		    act("$n's $p glows with a magical aura.",
			    ch, obj, NULL, TO_ROOM);

		    obj_cast_spell(sn, ch->level, ch, ch, obj);

		    return;
		    break;
		}
	    case 2:
		if (!IS_AFFECTED(ch, AFF_DETECT_INVIS))
		{
		    sn = skill_lookup( "detect invis");

		    act("$p glows with a magical aura.",
			    ch, obj, NULL, TO_CHAR);
		    act("$n's $p glows with a magical aura.",
			    ch, obj, NULL, TO_ROOM);

		    obj_cast_spell(sn, ch->level, ch, ch, obj);

		    return;
		    break;
		}
	    case 3:
		if (!IS_AFFECTED(ch, AFF_DETECT_HIDDEN))
		{
		    sn = skill_lookup( "detect hidden");

		    act("$p glows with a magical aura.",
			    ch, obj, NULL, TO_CHAR);
		    act("$n's $p glows with a magical aura.",
			    ch, obj, NULL, TO_ROOM);

		    obj_cast_spell(sn, ch->level, ch, ch, obj);

		    return;
		    break;
		}
	    case 4:
		if (IS_AFFECTED2(ch, AFF2_IMPROVED_INVIS))
		{
		    sn = skill_lookup( "improved invisibility");

		    act("$p glows with a magical aura.",
			    ch, obj, NULL, TO_CHAR);
		    act("$n's $p glows with a magical aura.",
			    ch, obj, NULL, TO_ROOM);

		    obj_cast_spell(sn, ch->level, ch, ch, obj);

		    return;
		    break;
		}
	    case 5:
		if (!is_affected(ch, skill_lookup("shield")))
		{
		    sn = skill_lookup( "shield");

		    act("$p glows with a magical aura.",
			    ch, obj, NULL, TO_CHAR);
		    act("$n's $p glows with a magical aura.",
			    ch, obj, NULL, TO_ROOM);

		    obj_cast_spell(sn, ch->level, ch, ch, obj);

		    return;
		    break;
		}
	    case 6:
		if (!is_affected(ch, skill_lookup("stone skin")))
		{
		    sn = skill_lookup( "stone skin");

		    act("$p glows with a magical aura.",
			    ch, obj, NULL, TO_CHAR);
		    act("$n's $p glows with a magical aura.",
			    ch, obj, NULL, TO_ROOM);

		    obj_cast_spell(sn, ch->level, ch, ch, obj);

		    return;
		    break;
		}
	    case 7:
		if (!IS_AFFECTED2(ch, AFF2_ELECTRICAL_BARRIER))
		{
		    sn = skill_lookup( "electrical barrier");

		    act("$p glows with a magical aura.",
			    ch, obj, NULL, TO_CHAR);
		    act("$n's $p glows with a magical aura.",
			    ch, obj, NULL, TO_ROOM);

		    obj_cast_spell(sn, ch->level, ch, ch, obj);

		    return;
		    break;
		}
	}
    }
#endif
}


void scare_update(CHAR_DATA *ch)
{
    CHAR_DATA *victim, *vnext;

    if (ch == NULL) {
	bug("scare_update: NULL ch", 0);
	return;
    }

    if (ch->in_room == NULL) {
	bug("scare_update: NULL ch->in_room", 0);
	return;
    }

    for (victim = ch->in_room->people; victim != NULL; victim = vnext)
    {
	vnext = victim->next_in_room;

        // Certain NPCs are protected
	if (IS_NPC(victim))
	{
	    if (victim->pIndexData->pShop != NULL
	    ||  IS_SET(victim->act, ACT_PROTECTED)
	    ||  IS_SET(victim->act, ACT_SENTINEL))
		continue;
	}

	if (victim != ch
	&&  victim->tot_level <= ch->tot_level
	&&  victim->fighting == NULL
	&&  victim->position == POS_STANDING
	&&  ch->invis_level < LEVEL_IMMORTAL
	&&  !is_same_group(victim, ch)
        &&  victim != MOUNTED(ch))
	{
	    if (number_percent() < 25)
	    {
		if (can_see(victim, ch))
		{
		    act("You balk with fear at the sight of $n!",  ch, victim, NULL, NULL, NULL, NULL, NULL, TO_VICT);
		    act("$N balks with fear at the sight of you!", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
		    act("$N balks with fear at the sight of $n!",  ch, victim, NULL, NULL, NULL, NULL, NULL, TO_NOTVICT);
		}
		else
		{
		    act("You balk with terror at a terrifying ominous presence in the room!", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_VICT);
		    act("$n balks with terror at a terrifying ominous presence in the room!", victim, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
		}

		do_flee(victim, "anyway");
	    }
	}
    }
}


// Update things a person has done in a combat round -- currently just reverie
void update_has_done(CHAR_DATA *ch)
{
    if (IS_SET(ch->has_done, DONE_REVERIE))
	REMOVE_BIT(ch->has_done, DONE_REVERIE);
}

void update_invasion_quest()
{
    AREA_DATA *pArea = NULL;
    INVASION_QUEST *quest = NULL;
    char buf[MSL];
    int max_quests = 1;
    int current_quests = 0;
    log_string("Update invasion quest...");

    for (pArea = area_first; pArea != NULL; pArea = pArea->next) {

	if ( pArea->invasion_quest != NULL) {
	    if (current_time > pArea->invasion_quest->expires) {

		sprintf(buf, "The invasion at %s has ended.", pArea->name);
		crier_announce(buf);
		log_string(buf);

		extract_invasion_quest(pArea->invasion_quest);
		pArea->invasion_quest = NULL;
	    }
	    else {
		current_quests++;
	    }
	}
    }

    // dont want too many quests at once
    if (current_quests < max_quests)
    {
	for (pArea = area_first; pArea != NULL; pArea = pArea->next) {

	    // Only towns should be invaded
	    if (pArea->area_who != AREA_TOWNE) continue;

	    // Newbies are in Plith so better not invade Plith
	    if (!str_cmp(pArea->name, "Plith")) {
		continue;
	    }

	    // only seralia or athemia continents should be invaded
	    if ( IS_SET(pArea->place_flags, PLACE_FIRST_CONTINENT) ||
		    IS_SET(pArea->place_flags, PLACE_SECOND_CONTINENT)) {

		if (number_percent() < 10) {
		    int level = number_range(0, 3);
		    switch(level) {
			case 0:
			    level = 30;
			    break;
			case 1:
			    level = 60;
			    break;
			case 2:
			    level = 90;
			    break;
			case 3:
			    level = 120;
			    break;
		    }
//		    quest = create_invasion_quest(pArea, level, 0, 0);

		    pArea->invasion_quest = quest;
		    break;
		}
	    }
	}
    }
}


/*
* Handle events.
*/
void event_update(void)
{
	extern EVENT_DATA *next_event;
	EVENT_DATA tmp;
	EVENT_DATA *ev;
	int depth, sec;

	if (!events) return;

	next_event = NULL;

	for (ev = events; ev; ev = next_event) {
		next_event = ev->next;

		// Delay has expired - perform action and remove event from lists
		if (ev->delay-- <= 0) {
			tmp = *ev;
			ev->args = NULL;
			ev->info = NULL;
			extract_event(ev);

			depth = script_call_depth;
			sec = script_security;
			script_call_depth = tmp.depth;
			script_security = tmp.security;

			switch (tmp.event_type) {

			// These are used by QUEUES
			case EVENT_MOBQUEUE:
			case EVENT_OBJQUEUE:
			case EVENT_ROOMQUEUE:
			case EVENT_TOKENQUEUE:
				do_function((void *)(tmp.info), tmp.function, tmp.args);
				break;
			case EVENT_ECHO:
				room_echo((ROOM_INDEX_DATA *)tmp.entity,tmp.args);
				break;
			}

			script_call_depth = depth;
			script_security = sec;

			free_string(tmp.args);
			if(tmp.info) free(tmp.info);
		}
	}
}
