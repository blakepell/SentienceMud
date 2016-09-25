/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik Strfeldt, Tom Madsen, and Katja Nyboe.    *
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

/***************************************************************************
 *                                                                         *
 *    Scripting engine rebuilt by Michael Kurtz (Nibelung)                 *
 *    Used with permission.                                                *
 *                                                                         *
 **************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "scripts.h"


// Command logging types
#define LOG_NORMAL	0
#define LOG_ALWAYS	1
#define LOG_NEVER	2


// Log-all switch
bool				logAll		= FALSE;


/*
 * Command table.
 */
const	struct	cmd_type	cmd_table	[] =
{
    // Common movement commands
    { "north",		do_north,	POS_STANDING,    0,  LOG_NEVER, 0 },
    { "east",		do_east,	POS_STANDING,	 0,  LOG_NEVER, 0 },
    { "south",		do_south,	POS_STANDING,	 0,  LOG_NEVER, 0 },
    { "west",		do_west,	POS_STANDING,	 0,  LOG_NEVER, 0 },
    { "up",		do_up,		POS_STANDING,	 0,  LOG_NEVER, 0 },
    { "down",		do_down,	POS_STANDING,	 0,  LOG_NEVER, 0 },
    { "northeast",	do_northeast,	POS_STANDING,	 0,  LOG_NEVER, 0 },
    { "northwest",	do_northwest,	POS_STANDING,	 0,  LOG_NEVER, 0 },
    { "southeast",	do_southeast,	POS_STANDING,	 0,  LOG_NEVER, 0 },
    { "southwest",	do_southwest,	POS_STANDING,	 0,  LOG_NEVER, 0 },
    { "ne",		do_northeast,	POS_STANDING,	 0,  LOG_NEVER, 0 },
    { "nw",		do_northwest,	POS_STANDING,	 0,  LOG_NEVER, 0 },
    { "se",		do_southeast,	POS_STANDING,	 0,  LOG_NEVER, 0 },
    { "sw",		do_southwest,	POS_STANDING,	 0,  LOG_NEVER, 0 },

    // Other common commands, placed here so one and two letter abbreviations work.
    { "?",		do_help,	POS_DEAD,	 0,  LOG_NEVER,  1 },
    { "at",             do_at,          POS_DEAD,        0,  LOG_ALWAYS, 0 },
    { "auction",        do_auction,     POS_SLEEPING,    0,  LOG_ALWAYS, 1 },
    { "bar",		do_bar,		POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "bomb",		do_bomb,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "buy",		do_buy,		POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "cast",		do_cast,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "channels",       do_channels,    POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "danger",		do_danger,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "donate",		do_donate,	POS_RESTING,	 0,  LOG_ALWAYS, 1 },
    { "evasion",	do_evasion,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "exits",		do_exits,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "fade",		do_fade,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "get",		do_get,		POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "goto",           do_goto,        POS_DEAD,       L5,  LOG_ALWAYS, 1 },
    { "goxy",		do_goxy,	POS_DEAD,	L5,  LOG_ALWAYS, 1 },
// VIZZWILDS - disabled old command - now replaced by goxy
//    { "mapgoto",        do_mapgoto,     POS_DEAD,       L5,  LOG_ALWAYS, 1 },
    { "group",          do_group,       POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "hit",		do_kill,	POS_FIGHTING,	 0,  LOG_NORMAL, 0 },
    { "hold",		do_wear,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "inventory",	do_inventory,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "judge",		do_judge,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "kill",		do_kill,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "look",		do_look,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "music",          do_music,   	POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "order",		do_order,	POS_RESTING,	 0,  LOG_ALWAYS, 1 },
    { "practice",       do_practice,	POS_FIGHTING,    0,  LOG_NORMAL, 1 },
    { "pull",           do_pull,	POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "push",           do_push,	POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "quest",          do_quest,       POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "rehearse",       do_rehearse,	POS_FIGHTING,    0,  LOG_NORMAL, 1 },
    { "reply",		do_reply,	POS_SLEEPING,	 0,  LOG_NEVER,  1 },
    { "rest",		do_rest,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "resurrect",	do_resurrect,	POS_SLEEPING,	 0,  LOG_ALWAYS, 1 },
    { "sit",		do_sit,		POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "stand",		do_stand,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "stat",		do_stat,	POS_DEAD,	L5,  LOG_NORMAL, 1 },
    { "tell",		do_tell,	POS_SLEEPING,    0,  LOG_NEVER,  1 },
    { "tells",		do_tells,	POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "turn",           do_turn,	POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "unlock",         do_unlock,      POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "ungroup",	do_ungroup,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "use",		do_use,		POS_SLEEPING,    0,  LOG_NORMAL, 1 },
//    { "waypoint",	do_waypoint,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "wield",		do_wear,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "conceal",	do_conceal,	POS_RESTING,	 0,  LOG_NORMAL, 1 },

    // Informational commands
    { "affects",	do_affects,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "challenge",	do_challenge,	POS_SLEEPING,	 0,  LOG_ALWAYS, 1 },
    { "changes",	do_changes,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "commands",	do_commands,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "consider",	do_consider,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "count",		do_count,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "credits",	do_credits,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "dice",		do_dice,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "equipment",	do_equipment,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "examine",	do_examine,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "flag",		do_flag,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "gold",		do_worth,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "help",		do_help,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "house",	 	do_house,	POS_RESTING,	 0,  LOG_ALWAYS, 1 },
    { "intimidate",	do_intimidate,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "motd",		do_motd,	POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "mccp",	 	do_compress,	POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "news",		do_news,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "read",		do_look,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "repair",		do_repair,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "report",		do_report,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "rules",		do_rules,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "score",		do_score,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "scry",		do_scry,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "skills",		do_skills,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "socials",	do_socials,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "spells",		do_spells,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "stats",		do_stats,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "time",		do_time,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "version",	do_showversion,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "war",		do_war,		POS_STANDING,	 0,  LOG_NORMAL, 1 },
    { "weather",	do_weather,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "who",		do_who_new,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "whois",		do_whois,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "wizlist",	do_wizlist,	POS_DEAD,        0,  LOG_NORMAL, 1 },

    // Configuration commands
    { "alias",		do_alias,	POS_DEAD,	 0,  LOG_NORMAL, 1 },

    { "autolist",	do_toggle,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "colour",		do_colour,	POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "color",		do_colour,      POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "config",	        do_toggle,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "description",	do_description,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "delet",		do_delet,	POS_DEAD,	 0,  LOG_ALWAYS, 0 },
    { "delete",		do_delete,	POS_STANDING,	 0,  LOG_ALWAYS, 1 },
    { "email",		do_email,	POS_DEAD,	 0,  LOG_ALWAYS, 1 },
    { "password",	do_password,	POS_DEAD,	 0,  LOG_ALWAYS, 1 },
    { "prompt",		do_prompt,	POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "showdamage",	do_showdamage,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "scroll",		do_scroll,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "title",		do_title,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "toggle",		do_toggle,	POS_DEAD,	 0,  LOG_NORMAL, 0 },
    { "unalias",	do_unalias,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "wimpy",		do_wimpy,	POS_DEAD,	 0,  LOG_NORMAL, 1 },

    // Communication commands
    { "afk",		do_afk,		POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "announcements",  do_announcements, POS_SLEEPING,  0,  LOG_NORMAL, 1 },
    { "catchup",	do_catchup,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "chat",		do_chat,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "/",		do_chat,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { ",",		do_emote,	POS_RESTING,	 0,  LOG_NEVER,  0 },
    { ".",		do_gossip,	POS_SLEEPING,	 0,  LOG_NORMAL, 0 },
    { ";",		do_gtell,	POS_DEAD,	 0,  LOG_NEVER,  0 },
    { "'",		do_say,		POS_RESTING,	 0,  LOG_NEVER,  0 },
    { "emote",		do_emote,	POS_RESTING,	 0,  LOG_NEVER,  1 },
    { "flame",		do_flame,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "formstate",	do_formstate,   POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "gossip",		do_gossip,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "gtell",		do_gtell,	POS_DEAD,	 0,  LOG_NEVER,  1 },
    { "helper",		do_helper, 	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "hints",		do_hints,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "ignore",		do_ignore,	POS_RESTING,	 0,  LOG_ALWAYS, 1 },
    { "intone",		do_intone,	POS_RESTING,	 0,  LOG_ALWAYS, 1 },
    { "note",		do_note,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "notify",		do_notify,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "ooc",		do_ooc,		POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "pursuit",	do_pursuit,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "qlist",		do_qlist,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "quiet",		do_quiet,	POS_SLEEPING, 	 0,  LOG_NORMAL, 1 },
    { "quote",		do_quote,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "replay",		do_replay,	POS_SLEEPING,	 0,  LOG_NEVER,  1 },
    { "say",		do_say,		POS_RESTING,	 0,  LOG_NEVER,  1 },
    { "sayto",		do_sayto,	POS_RESTING,	 0,  LOG_NEVER,  1 },	// NIB : 20070121
    { "unread",		do_unread,	POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "whisper",	do_whisper,	POS_RESTING,	 0,  LOG_NEVER,  1 },
    { "yell",		do_yell,	POS_RESTING,	 0,  LOG_NORMAL, 1 },

    // Object manipulation commands
    { "bind",		do_bind,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "blow",		do_blow,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "brandish",	do_brandish,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "brew",		do_brew,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "close",		do_close,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "combine",	do_combine,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "deposit",	do_deposit,	POS_RESTING,	 0,  LOG_ALWAYS, 1 },
    { "drink",		do_drink,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "drop",		do_drop,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "eat",		do_eat,		POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "envenom",	do_envenom,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "fill",		do_fill,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "give",		do_give,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "hands",		do_hands,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "heal",		do_heal,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "hold",		do_wear,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "infuse",		do_infuse,	POS_DEAD,	 0,  LOG_ALWAYS, 1 },
    { "inspect", 	do_inspect,     POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "keep",		do_keep,	POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "knock",		do_knock,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "list",		do_list,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "lock",		do_lock,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "locker",		do_locker,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "lore",		do_lore,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "mail",		do_mail,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "open",		do_open,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "pick",		do_pick,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "plant",		do_plant,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "pour",		do_pour,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "put",		do_put,		POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "quaff",		do_quaff,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "recite",		do_recite,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "remove",		do_remove,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "restring",       do_restring,    POS_RESTING, 	 0,  LOG_ALWAYS, 1 },
    { "sacrifice",	do_sacrifice,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "scribe",		do_scribe,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "sell",		do_sell,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "skull",		do_skull,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "strike",		do_strike,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "take",		do_get,		POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "unrestring",	do_unrestring,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "value",		do_value,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "wear",		do_wear,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "zap",		do_zap,		POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "affix",		do_affix,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "ink",		do_ink,		POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "ruboff",		do_ruboff,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "touch",		do_touch,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "takeoff",	do_takeoff,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "land",		do_land,	POS_RESTING,	 0,  LOG_NORMAL, 1 },

    // Race specific commands.
    { "bite",		do_bite, 	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "consume", 	do_consume, 	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "feed",		do_bite, 	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "shape",  	do_shape, 	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "shift",		do_shift, 	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "toxins",		do_toxins,	POS_DEAD,	 0,  LOG_NORMAL, 1 },

    // Combat commands.
    { "ambush",		do_ambush,	POS_STANDING,	 0,  LOG_NORMAL, 1 },
    { "backstab",	do_backstab,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "bash",		do_bash,	POS_FIGHTING,    0,  LOG_NORMAL, 1 },
    { "behead",		do_behead,	POS_FIGHTING,    0,  LOG_NORMAL, 1 },
    { "berserk",	do_berserk,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "blackjack",	do_blackjack,	POS_FIGHTING,	 0,  LOG_NORMAL, 0 },
    { "breathe",	do_breathe,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "bs",		do_backstab,	POS_FIGHTING,	 0,  LOG_NORMAL, 0 },
    { "charge",		do_charge,	POS_FIGHTING,    0,  LOG_NORMAL, 1 },
    { "circle",		do_circle,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "dirt",		do_dirt,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "disarm",		do_disarm,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "feign",		do_feign,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "flee",		do_flee,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "holdup",	        do_holdup,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "kick",		do_kick,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "murder",		do_kill,	POS_FIGHTING,	 5,  LOG_ALWAYS, 1 },
    { "pk",		do_pk,		POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "rack",		do_rack,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "rescue",		do_rescue,	POS_FIGHTING,	 0,  LOG_NORMAL, 0 },
    { "shoot",		do_shoot,	POS_FIGHTING,	 0,  LOG_NORMAL, ML },
    { "slit",		do_slit,	POS_FIGHTING,	 0,  LOG_NORMAL, 0 },
    { "smite",		do_smite,	POS_FIGHTING,	 0,  LOG_NORMAL, 0 },
    { "stake",		do_stake,	POS_STANDING,	 0,  LOG_NORMAL, 0 },
    { "tailkick",	do_tail_kick,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "throw",		do_throw,	POS_FIGHTING,	 0,  LOG_NORMAL, 0 },
    { "trample", 	do_trample,	POS_STANDING,	 0,  LOG_NORMAL, 0 },
    { "warcry",		do_warcry,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "weave",		do_weave,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },

    // Mob command interpreter (placed here for faster scan...)
    { "mob",		do_mob,		POS_DEAD,	 0,  LOG_NEVER,  0 },

    //  Ship commands. Declared here for ease of use. Called using do_function from do_ship.
    //  High imm level so people dont see them on commands list.
//    { "ship",		do_ship,	POS_RESTING,	 0,  LOG_NORMAL,  1 },
  /*{ "sail", 		do_steer, 	POS_STANDING,	L1,  LOG_NORMAL, 1 },
    { "scuttle", 	do_scuttle, 	POS_STANDING,	L1,  LOG_NORMAL, 1 },
    { "speed", 		do_speed, 	POS_STANDING,	L1,  LOG_NORMAL, 1 },
    { "steer", 		do_steer, 	POS_STANDING,	L1,  LOG_NORMAL, 1 },
    { "aim",		do_aim, 	POS_STANDING,	L1,  LOG_NORMAL, 1 },
    { "chase",		do_boat_chase,	POS_FIGHTING,   L1,  LOG_ALWAYS, 1 },
    { "crew",           do_crew,   	POS_SLEEPING,    0,  LOG_ALWAYS, 1 }, */
    //{ "pardon",   do_pardon, 	POS_SLEEPING,    0, LOG_NORMAL, 1 },
//    { "cargo", 		do_cargo, 	POS_STANDING,	L1,  LOG_NORMAL, 1 },
//    { "damage",		do_damage, 	POS_STANDING,	L1,  LOG_NORMAL, 1 },
//    { "crew",     do_crew,   	POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "dig",      do_dig,   	POS_STANDING,    0,  LOG_NORMAL, 1 },

    // Misc commands
    { "bank",           do_bank,     	POS_RESTING, 	0,  LOG_NORMAL, 1 },
//    { "board", 		do_board, 	POS_STANDING,	0,  LOG_NORMAL, 1 },
    { "church",		do_church,	POS_DEAD,	0,  LOG_NORMAL, 1 },
    { "clear",		do_clear,	POS_DEAD,	0,  LOG_NORMAL, 1 },
    { "convert",	do_convert,	POS_STANDING,	0,  LOG_NORMAL, 1 },
    { "ct",		do_chtalk,	POS_DEAD,	0,  LOG_NEVER, 1 },
    { "disembark",	do_disembark, 	POS_STANDING,	0,  LOG_NORMAL, 1 },
    { "dismount",	do_dismount,	POS_FIGHTING,	0,  LOG_NORMAL, 1 },
    { "enter", 		do_enter, 	POS_STANDING,	0,  LOG_NORMAL, 1 },
    { "follow",		do_follow,	POS_RESTING,	0,  LOG_NORMAL, 1 },
    { "go",		do_enter,	POS_STANDING,	0,  LOG_NORMAL, 0 },
    { "gohome",		do_gohome,	POS_RESTING,	0,  LOG_NORMAL, 1 },
    { "hide",		do_hide,	POS_RESTING,	0,  LOG_NORMAL, 1 },
    { "hunt",		do_hunt,	POS_STANDING,	0,  LOG_NORMAL, 1 },
//  { "mana",		do_mana,	POS_RESTING,	0,  LOG_NEVER,  1 },
    { "mount",		do_mount,	POS_FIGHTING,	0,  LOG_NORMAL, 1 },
    { "multiclass",	do_multi,	POS_RESTING,	0,  LOG_NORMAL, 1 },
    { "play",		do_play,	POS_RESTING,	0,  LOG_NORMAL, 1 },
    { "quit",		do_quit,	POS_DEAD,	0,  LOG_NORMAL, 1 },
    { "recall",		do_recall,	POS_FIGHTING,	0,  LOG_NORMAL, 1 },
    { "return",         do_return,      POS_DEAD,       0,  LOG_NORMAL, 1 },
    { "reverie",	do_reverie,	POS_RESTING,	0,  LOG_NORMAL, 1 },
    { "save",		do_save,	POS_DEAD,	0,  LOG_NORMAL, 1 },
    { "scan",		do_scan,	POS_RESTING,	0,  LOG_NORMAL, 1 },
    { "search",		do_search,	POS_RESTING,	0,  LOG_NORMAL, 1 },
    { "secondary",	do_secondary,	POS_RESTING,	0,  LOG_NORMAL, 1 },
    { "sleep",		do_sleep,	POS_SLEEPING,	0,  LOG_NORMAL, 1 },
    { "sneak",		do_sneak,	POS_STANDING,	0,  LOG_NORMAL, 1 },
    { "split",		do_split,	POS_RESTING,	0,  LOG_NORMAL, 1 },
    { "steal",		do_steal,	POS_STANDING,	0,  LOG_NORMAL, 1 },
    { "survey", 	do_survey, 	POS_STANDING,	0,  LOG_NORMAL, 1 },
    { "train",		do_train,	POS_RESTING,	0,  LOG_NORMAL, 1 },
    { "visible",	do_visible,	POS_SLEEPING,	0,  LOG_NORMAL, 1 },
    { "wake",		do_wake,	POS_SLEEPING,	0,  LOG_NORMAL, 1 },
    { "warp",		do_warp,	POS_RESTING,	0,  LOG_ALWAYS, 1 },
    { "where",		do_where,	POS_RESTING,	0,  LOG_NORMAL, 1 },
    { "whistle",	do_whistle,	POS_STANDING,	0,  LOG_NORMAL,	1 },

    // Immortal commands.
    { "addcommand",	do_addcommand,	POS_DEAD,	ML,  LOG_ALWAYS, 1 },
    { "advance",	do_advance,	POS_DEAD,	ML,  LOG_ALWAYS, 1 },
    { "alevel",		do_alevel,	POS_DEAD,	ML,  LOG_ALWAYS, 1 },
    { "allow",		do_allow,	POS_DEAD,	L2,  LOG_ALWAYS, 1 },
    { "aload",		do_aload,	POS_DEAD,	ML,  LOG_ALWAYS, 1 },
    { "arealinks",	do_arealinks,   POS_DEAD,       L5,  LOG_NORMAL, 1 },
    { "areset",		do_areset,	POS_DEAD,	ML,  LOG_ALWAYS, 1 },
    { "assignhelper",	do_assignhelper, POS_RESTING,	L2,  LOG_ALWAYS, 1 },
    { "autosetname",	do_autosetname,	POS_DEAD,	IM,  LOG_NORMAL, 1 },
    { "autowar",	do_autowar,	POS_DEAD,	L2,  LOG_ALWAYS, 1 },
    { "ban",		do_ban,		POS_DEAD,	L2,  LOG_ALWAYS, 1 },
    { "besteq",		do_besteq,	POS_DEAD,	ML,  LOG_NORMAL, 1 },
    { "boost",		do_boost,	POS_DEAD,	L1,  LOG_ALWAYS, 1 },
    { "botter",		do_botter,	POS_RESTING,	L4,  LOG_ALWAYS, 1 },
    { "build",		do_build,	POS_RESTING,	IM,  LOG_ALWAYS, 1 },
    { "clone",		do_clone,	POS_DEAD,	L5,  LOG_ALWAYS, 1 },
    { "deny",		do_deny,	POS_DEAD,	L1,  LOG_ALWAYS, 1 },
    { "disconnect",	do_disconnect,	POS_DEAD,	ML,  LOG_ALWAYS, 1 },
    { "dump",		do_dump,	POS_DEAD,	L2,  LOG_ALWAYS, 1 },
    { ":",		do_immtalk,	POS_DEAD,	L5,  LOG_NORMAL, 0 },
    { "evict",	 	do_evict,	POS_RESTING,	L5,  LOG_ALWAYS, 1 },
    { "force",		do_force,	POS_DEAD,	L2,  LOG_ALWAYS, 1 },
    { "freeze",		do_freeze,	POS_DEAD,	L2,  LOG_ALWAYS, 1 },
    { "gecho",		do_echo,	POS_DEAD,	L3 ,  LOG_ALWAYS, 1 },
    { "gq",		do_gq,		POS_DEAD,	L4,  LOG_ALWAYS, 1 },
    { "holyaura",	do_holyaura,	POS_DEAD,	L5,  LOG_NORMAL, 1 },
    { "holylight",	do_holylight,	POS_DEAD,	L5,  LOG_NORMAL, 1 },
    { "housemove",	do_housemove,   POS_DEAD,	L5,  LOG_ALWAYS, 1 },
    { "immflag",	do_immflag,	POS_DEAD,	L5,  LOG_ALWAYS, 1 },
    { "immtalk",	do_immtalk,	POS_DEAD,	L5,  LOG_NORMAL, 1 },
    { "immortalise",    do_immortalise, POS_DEAD,    	L1,  LOG_ALWAYS, 1 },
    { "memory",    do_memory, POS_DEAD,    	L1,  LOG_ALWAYS, 1 },
    { "imotd",          do_imotd,       POS_DEAD,       L5,  LOG_NORMAL, 1 },
    { "incognito",	do_incognito,	POS_DEAD,	L5,  LOG_NORMAL, 1 },
    { "invis",		do_invis,	POS_DEAD,	L5,  LOG_NORMAL, 0 },
    { "junk",		do_junk,	POS_DEAD,	L3,  LOG_ALWAYS, 1 },
    { "load",		do_load,	POS_DEAD,	L5,  LOG_ALWAYS, 1 },
    { "log",		do_log,		POS_DEAD,	L1,  LOG_ALWAYS, 0 },
    { "mlevel",		do_mlevel,	POS_DEAD,	L5,  LOG_NORMAL, 1 },
    { "mpdump",		do_mpdump,	POS_DEAD,	L5,  LOG_NEVER,  1 },
    { "mpstat",		do_mpstat,	POS_DEAD,	L5,  LOG_NEVER,  1 },
    { "mwhere",		do_mwhere,	POS_DEAD,	L5,  LOG_NORMAL, 1 },
    { "newlock",	do_newlock,	POS_DEAD,	L3,  LOG_ALWAYS, 1 },
    { "nochannels",	do_nochannels,	POS_DEAD,	L5,  LOG_ALWAYS, 1 },
    { "notell",		do_notell,	POS_DEAD,	L5,  LOG_ALWAYS, 1 },
    { "olevel",		do_olevel,	POS_DEAD,	L5,  LOG_NORMAL, 1 },
    { "otransfer",	do_otransfer,	POS_DEAD,	L5,  LOG_ALWAYS, 1 },
    { "owhere",		do_owhere,	POS_DEAD,	L5,  LOG_NORMAL, 1 },
    { "peace",		do_peace,	POS_DEAD,	L5,  LOG_NORMAL, 1 },
    { "pecho",		do_pecho,	POS_DEAD,	L4,  LOG_ALWAYS, 1 },
    { "permban",	do_permban,	POS_DEAD,	L1,  LOG_ALWAYS, 1 },
    { "poofin",		do_bamfin,	POS_DEAD,	L5,  LOG_NORMAL, 1 },
    { "poofout",	do_bamfout,	POS_DEAD,	L5,  LOG_NORMAL, 1 },
    { "purge",		do_purge,	POS_DEAD,	L5,  LOG_ALWAYS, 1 },
    { "reboo",		do_reboo,	POS_DEAD,	ML,  LOG_NORMAL, 0 },
    { "reboot",		do_reboot,	POS_DEAD,	ML,  LOG_ALWAYS, 0 },
    { "reckonin",       do_reckonin,    POS_DEAD,       L2,  LOG_ALWAYS, 0 },
    { "reckoning",      do_reckoning,   POS_DEAD,       L2,  LOG_ALWAYS, 1 },
    { "remcommand",	do_remcommand,  POS_DEAD,	ML,  LOG_ALWAYS, 1 },
    { "restore",	do_restore,	POS_DEAD,	L5,  LOG_ALWAYS, 1 },
    { "rwhere",		do_rwhere,	POS_DEAD,	L5,  LOG_ALWAYS, 1 },
    { "set",		do_set,		POS_DEAD,	L1,  LOG_ALWAYS, 1 },
    { "shutdow",	do_shutdow,	POS_DEAD,	ML,  LOG_NORMAL, 0 },
    { "shutdown",	do_shutdown,	POS_DEAD,	ML,  LOG_ALWAYS, 0 },
    { "slay",		do_slay,	POS_DEAD,	L3,  LOG_ALWAYS, 1 },
//    { "snoop",		do_snoop,	POS_DEAD,	L1,  LOG_ALWAYS, 1 },
    { "sockets",        do_sockets,	POS_DEAD,       L2,  LOG_ALWAYS, 1 },
    { "staff",		do_staff,	POS_DEAD,	ML,  LOG_ALWAYS, 1 },
    { "startinvasion",     do_startinvasion,  POS_DEAD,       L2,  LOG_ALWAYS, 1 },
    { "string",		do_string,	POS_DEAD,	L5,  LOG_ALWAYS, 1 },
    { "switch",		do_switch,	POS_DEAD,	L4,  LOG_ALWAYS, 1 },
    { "testport",	do_testport,POS_DEAD,	ML,  LOG_ALWAYS, 1 },	// 20140521 Nibs

    { "tlist",		do_tlist,	POS_DEAD,	L5,  LOG_NORMAL, 1 },
    { "token",		do_token,	POS_DEAD,	L5,  LOG_NORMAL, 1 },
    { "tshow",		do_tshow,	POS_DEAD,	L5,  LOG_NORMAL, 1 },
    { "trance",		do_trance,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "transfer",	do_transfer,	POS_DEAD,	L5,  LOG_ALWAYS, 1 },
    { "uninvis",	do_uninvis,	POS_DEAD,	L5,  LOG_NORMAL, 1 },
    { "^",		do_uninvis,	POS_DEAD,	L5,  LOG_NORMAL, 1 },
    { "vislist",	do_vislist,	POS_DEAD,	L5,  LOG_ALWAYS, 1 },
    { "vlinks",		do_vlinks,	POS_DEAD,	L5,  LOG_ALWAYS, 1 },
    { "vnum",		do_vnum,	POS_DEAD,	L5,  LOG_NORMAL, 1 },
    { "wizhelp",	do_wizhelp,	POS_DEAD,	L5,  LOG_NORMAL, 1 },
    { "wizinvis",	do_invis,	POS_DEAD,	L5,  LOG_NORMAL, 1 },
    { "wizlock",	do_wizlock,	POS_DEAD,	L1,  LOG_ALWAYS, 1 },
    { "wiznet",		do_wiznet,	POS_DEAD,	L5,  LOG_NORMAL, 1 },
    { "zecho",		do_zecho,	POS_DEAD,	L4,  LOG_ALWAYS, 1 },
    { "zot", 		do_zot,		POS_DEAD,       L5,  LOG_ALWAYS, 1 },

    // OLC commands
    { "aedit",		do_aedit,	POS_DEAD,    L5,  LOG_NORMAL, 1 },
    { "alist",		do_alist,	POS_DEAD,    L5,  LOG_NORMAL, 1 },
    { "asave",          do_asave_new,	POS_DEAD,    L5,  LOG_NORMAL, 1 },
    { "asearch",	do_asearch,	POS_DEAD,    L5,  LOG_NORMAL, 1 },
    { "dislink",	do_dislink,	POS_DEAD,    L5,  LOG_ALWAYS, 1 },
    { "edit",		do_olc,		POS_DEAD,    L5,  LOG_NORMAL, 1 },
    { "hedit",		do_hedit,	POS_DEAD,    L4,  LOG_ALWAYS, 1 },
    { "ifchecks",       do_ifchecks,    POS_DEAD,    L5,  LOG_NORMAL, 1 },
    { "mcopy",		do_mcopy,	POS_DEAD,    L5,  LOG_ALWAYS, 1 },
    { "medit",		do_medit,	POS_DEAD,    L5,  LOG_NORMAL, 1 },
    { "mlist",		do_mlist,	POS_DEAD,    L5,  LOG_NORMAL, 1 },
    { "mpcopy",		do_mpcopy,	POS_DEAD,    L5,  LOG_ALWAYS, 1 },
    { "mpdelete",	do_mpdelete,	POS_DEAD,    L2,  LOG_ALWAYS, 1 },
    { "mpedit",		do_mpedit,	POS_DEAD,    L5,  LOG_NORMAL, 1 },
    { "mplist",		do_mplist,	POS_DEAD,    L5,  LOG_NORMAL, 1 },
    { "mshow",		do_mshow,	POS_DEAD,    L5,  LOG_NORMAL, 1 },
    { "ocopy",		do_ocopy,	POS_DEAD,    L5,  LOG_ALWAYS, 1 },
    { "oedit",		do_oedit,	POS_DEAD,    L5,  LOG_NORMAL, 1 },
    { "olist",		do_olist,	POS_DEAD,    L5,  LOG_NORMAL, 1 },
    { "opcopy",		do_opcopy,	POS_DEAD,    L5,  LOG_ALWAYS, 1 },
    { "opdelete",	do_opdelete,	POS_DEAD,    L2,  LOG_ALWAYS, 1 },
    { "opdump",		do_opdump,	POS_DEAD,    L5,  LOG_NEVER,  1 },
    { "opedit",         do_opedit,      POS_DEAD,    L5,  LOG_ALWAYS, 1 },
    { "oplist",		do_oplist,	POS_DEAD,    L5,  LOG_NORMAL, 1 },
    { "opstat",		do_opstat,	POS_DEAD,    L5,  LOG_NEVER,  1 },
    { "oshow",		do_oshow,	POS_DEAD,    L5,  LOG_NORMAL, 1 },
    { "pedit",		do_pedit,	POS_DEAD,    ML,  LOG_ALWAYS, 1 },
    { "pinquiry",	do_pinquiry,	POS_DEAD,    IM,  LOG_ALWAYS, 1 },
    { "plist",		do_plist,	POS_DEAD,    IM,  LOG_ALWAYS, 1 },
    { "pdelete",	do_pdelete,	POS_DEAD,    L1,  LOG_ALWAYS, 1 },
    { "project",	do_project,	POS_DEAD,    IM,  LOG_NORMAL, 1 },
    { "pshow",		do_pshow,	POS_DEAD,    IM,  LOG_ALWAYS, 1 },
    { "rcopy",		do_rcopy,	POS_DEAD,    L5,  LOG_ALWAYS, 1 },
    { "redit",		do_redit,	POS_DEAD,    L5,  LOG_NORMAL, 1 },
    { "resets",		do_resets,	POS_DEAD,    L5,  LOG_NORMAL, 1 },
    { "rjunk",		do_rjunk,	POS_DEAD,    L4,  LOG_ALWAYS, 1 },
    { "rlist",		do_rlist,	POS_DEAD,    L5,  LOG_NORMAL, 1 },
    { "rpcopy",		do_rpcopy,	POS_DEAD,    L5,  LOG_ALWAYS, 1 },
    { "rpdelete",	do_rpdelete,	POS_DEAD,    L2,  LOG_ALWAYS, 1 },
    { "rpdump",		do_rpdump,	POS_DEAD,    L5,  LOG_NEVER,  1 },
    { "rpedit",         do_rpedit,      POS_DEAD,    L5,  LOG_ALWAYS, 1 },
    { "rplist",		do_rplist,	POS_DEAD,    L5,  LOG_NORMAL, 1 },
    { "rpstat",		do_rpstat,	POS_DEAD,    L5,  LOG_NEVER,  1 },
    { "rshow", 		do_rshow,	POS_DEAD,    L5,  LOG_NORMAL, 1 },
    { "tedit",		do_tedit,	POS_DEAD,    L5,  LOG_NORMAL, 1 },
    { "tpdump",		do_tpdump,	POS_DEAD,    L5,  LOG_NEVER,  1 },
    { "tpedit",		do_tpedit,	POS_DEAD,    L5,  LOG_NORMAL, 1 },
    { "tplist",		do_tplist,	POS_DEAD,    L5,  LOG_NORMAL, 1 },
    { "tpstat",		do_tpstat,	POS_DEAD,    L5,  LOG_NEVER,  1 },
/* VIZZWILDS */
    { "wedit",		do_wedit,	POS_DEAD,    L5,  LOG_NORMAL, 1 },
    { "vledit",		do_vledit,	POS_DEAD,    L5,  LOG_NORMAL, 1 },
    { "wlist",		do_wlist,	POS_DEAD,    L5,  LOG_NORMAL, 1 },

    /* Staff management commands */
    { "slist",		do_slist,	POS_DEAD,    ML,  LOG_ALWAYS, 1 },
    { "sadd",		do_sadd,	POS_DEAD,    ML,  LOG_ALWAYS, 1 },
    { "sdelete",	do_sdelete,	POS_DEAD,    ML,  LOG_ALWAYS, 1 },
    { "ssupervisor",	do_ssupervisor,	POS_DEAD,    ML,  LOG_ALWAYS, 1 },
    { "sduty",		do_sduty,	POS_DEAD,    ML,  LOG_ALWAYS, 1 },
    //{ "spromote",	do_spromote,	POS_DEAD,    ML,  LOG_ALWAYS, 1 },
    //{ "sdemote",	do_sdemote,	POS_DEAD,    ML,  LOG_ALWAYS, 1 },


    { "",		0,		POS_DEAD,     0,  LOG_NORMAL, 0 }
};

bool forced_command = FALSE;	// 20070511NIB: Used to prevent forces to do any restricted command

bool check_verbs(CHAR_DATA *ch, char *command, char *argument)
{
	char buf[MIL], *p;
	ITERATOR tit, pit;
	TOKEN_DATA *token;
	OBJ_DATA *obj;
	CHAR_DATA *mob;
	PROG_LIST *prg;
//	SCRIPT_DATA *script;
	unsigned long uid[2];
	int slot;
	int ret_val = PRET_NOSCRIPT, ret; // @@@NIB Default for a trigger loop is NO SCRIPT

	log_stringf("check_verbs: ch(%s), command(%s), argument(%s)", ch->name, command, argument);
//	printf_to_char(ch, "check_verbs: ch(%s), command(%s), argument(%s)", ch->name, command, argument);

	slot = TRIGSLOT_VERB;

	// Save the UID
	uid[0] = ch->id[0];
	uid[1] = ch->id[1];

	// Check for tokens FIRST
	iterator_start(&tit, ch->ltokens);
	while(( token = (TOKEN_DATA *)iterator_nextdata(&tit))) {
		if(token->pIndexData->progs) {
			log_stringf("check_verbs: ch(%s) token(%ld, %s)", ch->name, token->pIndexData->vnum, token->name);
			script_token_addref(token);
			script_destructed = FALSE;
			iterator_start(&pit, token->pIndexData->progs[slot]);
			while((prg = (PROG_LIST *)iterator_nextdata(&pit)) && !script_destructed) {
				log_stringf("check_verbs: ch(%s) token(%ld, %s) trigger(%s, %s)", ch->name, token->pIndexData->vnum, token->name, trigger_name(prg->trig_type), prg->trig_phrase);
				if (is_trigger_type(prg->trig_type,TRIG_VERBSELF) && !str_prefix(command, prg->trig_phrase)) {
					log_stringf("check_verbs: ch(%s) token(%ld, %s) trigger(%s, %s) executing", ch->name, token->pIndexData->vnum, token->name, trigger_name(prg->trig_type), prg->trig_phrase);
					ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, ch, NULL, NULL, NULL, NULL,NULL,NULL,argument,prg->trig_phrase,0,0,0,0,0);
					if( ret != PRET_NOSCRIPT) {
						iterator_stop(&pit);

						script_token_remref(token);
						return ret;
					}

				}
			}
			iterator_stop(&pit);
			script_token_remref(token);
		}
	}
	iterator_stop(&tit);
	if( ret_val != PRET_NOSCRIPT ) return TRUE;

	p = one_argument(argument,buf);
	if(!str_cmp(buf,"here")) {
		ROOM_INDEX_DATA *room = ch->in_room;
		ROOM_INDEX_DATA *source;
		bool isclone;

		if(room->source) {
			source = room->source;
			isclone = TRUE;
			uid[0] = room->id[0];
			uid[1] = room->id[1];
		} else {
			source = room;
			isclone = FALSE;
		}

		script_room_addref(room);

		// Check for tokens FIRST
		iterator_start(&tit, room->ltokens);
		while((token = (TOKEN_DATA *)iterator_nextdata(&tit))) {
			if( token->pIndexData->progs ) {
				script_token_addref(token);
				script_destructed = FALSE;
				iterator_start(&pit, token->pIndexData->progs[slot]);
				while((prg = (PROG_LIST *)iterator_nextdata(&pit)) && !script_destructed) {
					if (is_trigger_type(prg->trig_type,TRIG_VERB) && !str_prefix(command, prg->trig_phrase)) {
						ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, ch, NULL, NULL, NULL, NULL,NULL,NULL,p,prg->trig_phrase,0,0,0,0,0);
						if( ret != PRET_NOSCRIPT) {
							iterator_stop(&tit);
							iterator_stop(&pit);

							script_token_remref(token);
							script_room_remref(room);
							return ret;
						}

					}
				}
				iterator_stop(&pit);
				script_token_remref(token);
			}
		}
		iterator_stop(&tit);

		if(ret_val == PRET_NOSCRIPT && source->progs->progs) {
			script_destructed = FALSE;
			iterator_start(&pit, source->progs->progs[slot]);
			while((prg = (PROG_LIST *)iterator_nextdata(&pit)) && !script_destructed) {
				if (is_trigger_type(prg->trig_type,TRIG_VERB) && !str_prefix(command, prg->trig_phrase)) {
					ret = execute_script(prg->vnum, prg->script, NULL, NULL, room, NULL, ch, NULL, NULL, NULL, NULL,NULL,NULL,p,prg->trig_phrase,0,0,0,0,0);
					if( ret != PRET_NOSCRIPT) {
						iterator_stop(&pit);

						script_room_remref(room);
						return ret;
					}

				}
			}
			iterator_stop(&pit);

		}
		script_room_remref(room);

		if( ret_val != PRET_NOSCRIPT ) return TRUE;
	}

	// Get mobile...
	mob = strcmp(buf,"self") ? get_char_room(ch, NULL, buf) : ch;
	if(mob) {
		script_mobile_addref(mob);

		// Check for tokens FIRST
		iterator_start(&tit, mob->ltokens);
		while((token = (TOKEN_DATA *)iterator_nextdata(&tit))) {
			if( token->pIndexData->progs ) {
				script_token_addref(token);
				script_destructed = FALSE;
				iterator_start(&pit, token->pIndexData->progs[slot]);
				while((prg = (PROG_LIST *)iterator_nextdata(&pit)) && !script_destructed) {
					if (is_trigger_type(prg->trig_type,TRIG_VERB) && !str_prefix(command, prg->trig_phrase)) {
						ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, ch, NULL, NULL, NULL, NULL,NULL,NULL,p,prg->trig_phrase,0,0,0,0,0);
						if( ret != PRET_NOSCRIPT) {
							iterator_stop(&tit);
							iterator_stop(&pit);

							script_token_remref(token);
							script_mobile_remref(mob);
							return ret;
						}

					}
				}
				iterator_stop(&pit);
				script_token_remref(token);
			}
		}
		iterator_stop(&tit);

		if(ret_val == PRET_NOSCRIPT && IS_NPC(mob) && mob->pIndexData->progs) {
			script_destructed = FALSE;
			iterator_start(&pit, mob->pIndexData->progs[slot]);
			while((prg = (PROG_LIST *)iterator_nextdata(&pit)) && !script_destructed) {
				if (is_trigger_type(prg->trig_type,TRIG_VERB) && !str_prefix(command, prg->trig_phrase)) {
					ret = execute_script(prg->vnum, prg->script, mob, NULL, NULL, NULL, ch, NULL, NULL, NULL, NULL,NULL,NULL,p,prg->trig_phrase,0,0,0,0,0);
					if( ret != PRET_NOSCRIPT) {
						iterator_stop(&pit);

						script_mobile_remref(mob);
						return ret;
					}

				}
			}
			iterator_stop(&pit);
		}
		script_mobile_remref(mob);

		if( ret_val != PRET_NOSCRIPT ) return TRUE;
	}

	// Get obj...
	if ((obj = get_obj_here(ch, NULL, buf))) {
		script_object_addref(obj);

		// Check for tokens FIRST
		iterator_start(&tit, obj->ltokens);
		while((token = (TOKEN_DATA *)iterator_nextdata(&tit))) {
			if( token->pIndexData->progs ) {
				script_token_addref(token);
				script_destructed = FALSE;
				iterator_start(&pit, token->pIndexData->progs[slot]);
				while((prg = (PROG_LIST *)iterator_nextdata(&pit)) && !script_destructed) {
					if (is_trigger_type(prg->trig_type,TRIG_VERB) && !str_prefix(command, prg->trig_phrase)) {
						ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, ch, NULL, NULL, NULL, NULL,NULL,NULL,p,prg->trig_phrase,0,0,0,0,0);
						if( ret != PRET_NOSCRIPT) {
							iterator_stop(&tit);
							iterator_stop(&pit);

							script_token_remref(token);
							script_object_remref(obj);
							return ret;
						}

					}
				}
				iterator_stop(&pit);
				script_token_remref(token);
			}
		}
		iterator_stop(&tit);

		if(ret_val == PRET_NOSCRIPT && obj->pIndexData->progs) {
			script_destructed = FALSE;
			iterator_start(&pit, obj->pIndexData->progs[slot]);
			while((prg = (PROG_LIST *)iterator_nextdata(&pit)) && !script_destructed) {
				if (is_trigger_type(prg->trig_type,TRIG_VERB) && !str_prefix(command, prg->trig_phrase)) {
					ret = execute_script(prg->vnum, prg->script, NULL, obj, NULL, NULL, ch, NULL, NULL, NULL, NULL,NULL,NULL,p,prg->trig_phrase,0,0,0,0,0);
					if( ret != PRET_NOSCRIPT) {
						iterator_stop(&pit);

						script_object_remref(obj);
						return ret;
					}

				}
			}
			iterator_stop(&pit);
		}

		script_object_remref(obj);

		if( ret_val != PRET_NOSCRIPT ) return TRUE;
	}

	return FALSE;
}

// The main entry point for executing commands.
// Can be recursively called from 'at', 'order', 'force'.
void interpret( CHAR_DATA *ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    char logline[MAX_INPUT_LENGTH];
    int cmd;
    int trust;
    bool found, allowed;
    char cmd_copy[MAX_INPUT_LENGTH] ;
    char buf[MSL];

    // Strip leading spaces
    while (isspace(*argument))
	argument++;

    if ( argument[0] == '\0' )
	return;

    // Frozen people can't do anything
    if (!IS_NPC(ch) && IS_SET(ch->act, PLR_FREEZE))
    {
	send_to_char( "You're totally frozen!\n\r", ch );
	return;
    }

    // Neither can paralyzed people
    if (ch->paralyzed > 0 && !is_allowed( argument ))
    {
	send_to_char("You are paralyzed and can't move a muscle!\n\r", ch );
	return;
    }

	// Deal with scripted input
	if(ch->desc && ch->desc->input && ch->desc->input_script > 0) {
		int ret;
		SCRIPT_DATA *script = NULL;
		VARIABLE **var = NULL;
		CHAR_DATA *mob = ch->desc->input_mob;
		OBJ_DATA *obj = ch->desc->input_obj;
		ROOM_INDEX_DATA *room = ch->desc->input_room;
		TOKEN_DATA *tok = ch->desc->input_tok;
		char *v = ch->desc->input_var;

		if(ch->desc->input_mob) {
			script = get_script_index(ch->desc->input_script,PRG_MPROG);
			var = &ch->desc->input_mob->progs->vars;
		} else if(ch->desc->input_obj) {
			script = get_script_index(ch->desc->input_script,PRG_OPROG);
			var = &ch->desc->input_obj->progs->vars;
		} else if(ch->desc->input_room) {
			script = get_script_index(ch->desc->input_script,PRG_RPROG);
			var = &ch->desc->input_room->progs->vars;
		} else if(ch->desc->input_tok) {
			script = get_script_index(ch->desc->input_script,PRG_TPROG);
			var = &ch->desc->input_tok->progs->vars;
		}

		// Clear this incase other scripts chain together
		ch->desc->input = FALSE;
		ch->desc->input_var = NULL;
		ch->desc->input_script = 0;
		ch->desc->input_mob = NULL;
		ch->desc->input_obj = NULL;
		ch->desc->input_room = NULL;
		ch->desc->input_tok = NULL;
		if(ch->desc->input_prompt) free_string(ch->desc->input_prompt);
		ch->desc->input_prompt = NULL;

		if(script) {
//			send_to_char("Executing script...\n\r",ch);
			if(v) {
//				send_to_char("Var:",ch);
//				send_to_char(v,ch);
//				send_to_char("...\n\r",ch);
				variables_set_string(var,v,argument,FALSE);
			}

			ret = execute_script(script->vnum, script, mob, obj, room, tok, ch, NULL, NULL, NULL, NULL, NULL, NULL, NULL,NULL,0,0,0,0,0);
			if(ret > 0 && !IS_NPC(ch) && ch->pcdata->quit_on_input)
				do_function(ch, &do_quit, NULL);
		}

		if(v) free_string(v);

		if(script) return;
	}




    strcpy(cmd_copy, argument);

   /*
    * Grab the command word.
    * Special parsing so ' can be a command,
    *   also no spaces needed after punctuation.
    */
    strcpy( logline, argument );
    if ( !isalpha(argument[0]) && !isdigit(argument[0]) )
    {
	command[0] = argument[0];
	command[1] = '\0';
	argument++;
	while ( isspace(*argument) )
	    argument++;
    }
    else
	argument = one_argument( argument, command );

    // Questions which people must answer before they can go on with life!

    // Remove yourself from a church?
    // Disabled pneuma/dp loss for now - Tieryo
    if (ch->remove_question)
    {
	if (!str_prefix(command, "yes"))
	{
	    if (!str_cmp(ch->name, ch->remove_question->name))
	    {
		if (!str_cmp(ch->name, ch->remove_question->church->founder))
		{
		    act("{Y[You have removed yourself.]{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
		    sprintf(buf, "{Y[%s has quit %s]{x\n\r", ch->remove_question->name, ch->church->name);
		    gecho( buf );
//		    ch->pneuma = 0;
//		    ch->deitypoints = 0;
		    extract_church( ch->church );
		    ch->remove_question = NULL;
		    sprintf( buf, "%s has quit.", ch->name );
		    append_church_log( ch->church, ch->name );
		}
		else
		{
		    act("{Y[You have removed yourself.]{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
		    sprintf(buf, "{Y[%s has quit %s]{x\n\r", ch->remove_question->name, ch->church->name);
		    gecho( buf );
		    sprintf( buf, "%s has quit.", ch->name );
		    append_church_log( ch->church, ch->name );
//		    ch->pneuma = 0;
//		    ch->deitypoints = 0;
		    remove_member(ch->remove_question);
		    ch->remove_question = NULL;
		}
	    }
	    return;
	}
	else
        if (!str_prefix(command, "no"))
        {
	    ch->remove_question = NULL;
	    return;
	}
	else
	{
	    send_to_char("Please answer yes or no.\n\r", ch);
	    return;
	}
    }

    if (!IS_NPC(ch) && ch->pcdata->inquiry_subject != NULL) {
	if (command[0] != '\0') {
	    sprintf(buf, "%s %s", command, argument);
	    buf[0] = UPPER(buf[0]);
	    ch->pcdata->inquiry_subject->subject = str_dup(buf);
	    send_to_char("Inquiry added. Starting editor...\n\r", ch);
	    string_append(ch, &ch->pcdata->inquiry_subject->text);
	    ch->pcdata->inquiry_subject = NULL;
	    projects_changed = TRUE;
	}
	else
	    send_to_char("{YEnter inquiry subject:{x ", ch);

	return;
    }

    // Toggle church PK?
    if (ch->pk_question)
    {
	char buf[MAX_STRING_LENGTH];

	if (!str_prefix(command, "yes"))
	{
	    sprintf(buf, "{Y[%s is now a PLAYER KILLING church!]{x\n\r",
		    ch->church->name );
	    gecho( buf );
	    ch->pk_question = FALSE;
	    ch->church->pk = TRUE;
	    return;
	}
	else
        if (!str_prefix(command, "no"))
        {
	    ch->pk_question = FALSE;
	    return;
	}
	else
	{
	    send_to_char("Please answer yes or no.\n\r", ch);
	    return;
	}
    }

    // Toggle personal PK?
    if (ch->personal_pk_question)
    {
	char buf[MAX_STRING_LENGTH];

	if (!str_prefix(command, "yes"))
	{
	    if (!IS_SET( ch->act, PLR_PK ) )
	    {
		SET_BIT( ch->act, PLR_PK );
		send_to_char("You have toggled PK. Good luck!\n\r", ch );
		sprintf( buf, "%s has toggled PK on!", ch->name );
		crier_announce( buf );
		ch->pneuma -= 5000;
	    }
	    else
	    {
		REMOVE_BIT( ch->act, PLR_PK );
		send_to_char("You have toggled PK off.\n\r", ch );
		sprintf( buf, "%s is no longer PK.", ch->name );
		crier_announce( buf );
		ch->pneuma -= 5000;
	    }

	    ch->personal_pk_question = FALSE;
	    return;
	}
	else
        if (!str_prefix(command, "no"))
        {
	    ch->personal_pk_question = FALSE;
	    return;
	}
	else
	{
	    if ( IS_SET( ch->act, PLR_PK ) )
	    {
		send_to_char("Toggle PK off? (y/n)\n\r", ch );
	    }
	    else
	    {
		send_to_char("Toggle PK on? (y/n)\n\r", ch );
	    }
	    return;
	}
    }

    // Cross-zone gohall?
    if (ch->cross_zone_question)
    {
	char buf[MAX_STRING_LENGTH];

	if (!str_prefix(command, "yes"))
	{
	    long pneuma_cost;
	    long dp_cost;

	    pneuma_cost = 500;
	    dp_cost = 50000;

	    if ( ch->church == NULL )
		return;

	    ch->church->pneuma -= pneuma_cost;
	    ch->church->dp -= dp_cost;
	    sprintf( buf, "{Y[%s has recalled cross-zone, draining %ld pneuma and %ld karma!]{x\n\r", ch->name, pneuma_cost, dp_cost );
	    msg_church_members( ch->church, buf );
	    ch->cross_zone_question = FALSE;

	    act("{R$n disappears, leaving a resounding echo of discord.{X", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
	    char_from_room(ch);
	    char_to_room(ch, location_to_room(&ch->church->recall_point));
	    act("$n appears in the room.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
	    do_function(ch, &do_look, "auto");
	    return;
	}
	else
        if (!str_prefix(command, "no"))
        {
	    send_to_char("Cross-zone recall cancelled.\n\r", ch );
	    ch->cross_zone_question = FALSE;
	    return;
	}
	else
	{
	    send_to_char("Recall cross-zone? (yes/no)\n\r", ch );
	    return;
	}
    }

    // Convert church to a different alignment?
    if (!IS_NPC(ch) && ch->pcdata->convert_church != -1)
    {
	char buf[MAX_STRING_LENGTH];

	if (!str_prefix(command, "yes"))
	{
	    long pneuma_cost;
	    long dp_cost;

	    pneuma_cost = 10000;
	    dp_cost = 2500000;

	    if ( ch->church == NULL )
		return;

	    ch->church->pneuma -= pneuma_cost;
	    ch->church->dp -= dp_cost;
	    ch->church->alignment = ch->pcdata->convert_church;

	    sprintf( buf, "{Y[%s has converted to the faith of %s!]{x\n\r",
	        ch->church->name,
		ch->church->alignment == CHURCH_GOOD ? "the Pious" :
  		  ch->church->alignment == CHURCH_NEUTRAL ? "Neutrality" :
		  "Malice" );
	    gecho( buf );
	    ch->pcdata->convert_church = -1;
	    return;
	}
	else
        if (!str_prefix(command, "no"))
        {
	    send_to_char("Church faith conversion cancelled.\n\r", ch );
	    ch->pcdata->convert_church = -1;
	    return;
	}
	else
	{
	    sprintf( buf, "Are you SURE you want to convert to the faith of %s? (y/n)\n\r"
                  "{R***WARNING***:{x all members who cannot follow that faith will be removed on their next login!!!\n\r",
	       ch->pcdata->convert_church == CHURCH_GOOD ? "the Pious" :
	          ch->pcdata->convert_church == CHURCH_NEUTRAL ? "Neutrality" :
		  "Malice" );
	    send_to_char( buf, ch );
	    return;
	}
    }

    // Answer a challenge?
    // Display character names, no more "a Slayer/a Werewolf" displayed to everyone -- Areo
    if (ch->challenged != NULL)
    {
	CHAR_DATA *victim = ch->challenged;
	char buf[MAX_STRING_LENGTH];

	if (!str_prefix(command, "yes"))
	{
	    sprintf(buf, "%s has accepted %s's challenge! May the battle begin!",
	        ch->name, victim->name);
	    crier_announce( buf );

	    sprintf(buf, "{M%s has accepted your challenge!{x\n\r{RYou are transported to the arena!\n\r{x", pers( ch, victim ) );

	    send_to_char(buf, victim);

	    sprintf(buf, "{MYou have accepted %s's challenge!{x\n\r{RYou are transported to the arena!\n\r{x", pers( victim, ch ) );

	    send_to_char(buf, ch);

            if (ch->fighting != NULL)
		stop_fighting(ch, TRUE);

	    if (ch->cast > 0)
		stop_casting(ch, TRUE);

		if (ch->script_wait > 0)
		script_end_failure(ch, TRUE);

		interrupt_script(ch,FALSE);


	    if (victim->fighting != NULL)
		stop_fighting(victim, TRUE);

	    if (victim->cast > 0)
		stop_casting(victim, TRUE);

		if(victim->script_wait > 0)
		script_end_failure(victim, TRUE);

		interrupt_script(victim,FALSE);

		location_from_room(&ch->pcdata->room_before_arena,ch->in_room);
		location_from_room(&victim->pcdata->room_before_arena,victim->in_room);

	    char_from_room(ch);
	    char_from_room(victim);
	    char_to_room(ch, get_room_index(ROOM_VNUM_ARENA));
	    char_to_room(victim, get_room_index(ROOM_VNUM_ARENA));

	    ch->challenged = NULL;
	    return;
	}

	if (!str_prefix(command, "no"))
	{
	    sprintf( buf, "%s has declined %s's challenge!",
		    ch->name, victim->name);
	    crier_announce( buf );

	    sprintf(buf, "{M%s has declined your challenge!{x\n\r", ch->name);
	    send_to_char(buf, victim);

	    sprintf(buf, "{MYou have declined %s's challenge!{x\n\r", victim->name);
	    send_to_char(buf, ch);
	    ch->challenged = NULL;
	    return;
	}

	sprintf(buf, "{M%s has challenged you to a fight to the death in the arena!\n\rDo you accept? (Yes/No)\n\r{x", victim->name);
	send_to_char(buf, ch);
	return;
    }

    // Find command in table.
    found = FALSE;
    if (!IS_SWITCHED(ch))
        trust = get_trust( ch );
    else
        trust = get_trust( ch->desc->original );

    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( command[0] == cmd_table[cmd].name[0]
	&&   !str_prefix( command, cmd_table[cmd].name )
	&&   (!forced_command || (cmd_table[cmd].level < LEVEL_IMMORTAL))	// 20070511NIB - used to prevent script forces from doing imm commands
	&&   (cmd_table[cmd].level <= trust || is_granted_command(ch, cmd_table[cmd].name)))
	{
	    found = TRUE;
	    break;
	}
    }

    allowed = is_allowed(command);

    // Check stuff relevant to interpretation.
    if (IS_AFFECTED(ch, AFF_HIDE) && !allowed)
    {
        affect_strip(ch, gsn_hide);
	REMOVE_BIT(ch->affected_by, AFF_HIDE);
	act("You step out of the shadows.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR );
	act("$n steps out of the shadows.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM );
    }

    if (is_affected(ch, skill_lookup("paralysis"))
    &&  !allowed)
    {
        send_to_char("You can't move a muscle!\n\r", ch );
        return;
    }

    if (ch->paroxysm > 0 && !allowed)
    {
	if (number_percent() < 20)
	{
	    send_to_char("{YYou flail your arms about wildly.{x\n\r", ch);
	    act("$n flails $s arms about wildly, unable to control $mself.{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
	}
	else if (number_percent() < 20)
	{
	    send_to_char("{YYou cartwheel across the floor.{x\n\r", ch);
	    act("{Y$n cartwheels across the floor.{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
	}
	else if (number_percent() < 20)
	{
	    send_to_char("{YYou babble nonsensically and foam at the mouth.{x\n\r", ch );
  	    act("{Y$n babbles nonsensically and foams at the mouth.{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
	}
	else if (number_percent() < 20)
	{
	    send_to_char("{YYour fall to the floor and begin to convulse.{x\n\r", ch );
 	    act("{Y$n collapses to the floor and begins to have seizures.{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM );
	}
	else if (number_percent() < 20)
	{
	    send_to_char("{YYou begin to spin around in circles.{x\n\r", ch);
 	    act("$n spins around dizzifyingly.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
	}
	else
	{
	    send_to_char("{YYou stare blankly at your feet.{x\n\r", ch);
    	    act("$n stares blankly, unable to do anything.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
	}

	return;
    }

    if (ch->cast > 0 && !allowed)
	stop_casting(ch, TRUE);

	if (ch->script_wait > 0 && !allowed)
		script_end_failure(ch, TRUE);

	if(!allowed) interrupt_script(ch,FALSE);

    if (ch->music > 0 && !allowed)
		stop_music(ch, TRUE);

    if (ch->brew > 0 && !allowed)
        return;

    if (ch->repair > 0 && !allowed)
    {
        act("You stop repairing $p.", ch, NULL, NULL, ch->repair_obj, NULL, NULL, NULL, TO_CHAR);
	act("$n stops repairing $p.", ch, NULL, NULL, ch->repair_obj, NULL, NULL, NULL, TO_ROOM);
	ch->repair_obj = NULL;
	ch->repair_amt = 0;
	ch->repair = 0;
    }

    if (ch->hide > 0 && !allowed)
    {
        ch->hide = 0;
        send_to_char("You stop looking for a place to hide.\n\r", ch );
    }

    if (ch->bind > 0 && !allowed)
	return;

    if (ch->bomb > 0 && !allowed)
	return;

    if (ch->recite > 0 && !allowed)
    {
	ch->recite = 0;
	free_string( ch->cast_target_name );
	ch->cast_target_name = NULL;
	send_to_char("{WYou stop reciting.{x\n\r", ch );
    }

    if ((ch->reverie > 0 || ch->trance > 0) && !allowed)
    {
    	send_to_char("You can't break your meditation.\n\r",ch );
	return;
    }

    if (ch->scribe > 0 && !allowed)
	return;

    // You can move while shooting, but not much else
    if (ch->ranged > 0 && !allowed)
    {
	if ( str_cmp( command, "north" )
	&&   str_cmp( command, "east" )
	&&   str_cmp( command, "south" )
	&&   str_cmp( command, "west" )
	&&   str_cmp( command, "northwest" )
	&&   str_cmp( command, "northeast" )
	&&   str_cmp( command, "southwest" )
	&&   str_cmp( command, "southeast" )
	&&   str_cmp( command, "up" )
	&&   str_cmp( command, "down" ))
	    stop_ranged( ch, TRUE );
    }

    if (ch->resurrect > 0 && !allowed)
    {
        send_to_char("You stop resurrecting.\n\r", ch );
        ch->resurrect = 0;
    }

    if (ch->fade > 0 && !allowed)
    {
        send_to_char("You fade back into the real world.\n\r", ch );
        ch->fade = 0;
        ch->fade_dir = -1;		//@@@NIB : 20071020
    }

    // Stop abuse.
    if (IS_NPC(ch) && cmd_table[cmd].level > LEVEL_IMMORTAL)
    {
	sprintf(buf, "interpret: mob %s(%ld) tried immortal command %s",
	    ch->short_descr, ch->pIndexData->vnum, cmd_table[cmd].name);
	log_string(buf);
	return;
    }

    // Log and snoop.
    if ( cmd_table[cmd].log == LOG_NEVER )
	strcpy( logline, "" );

    if (/*ch->tot_level < MAX_LEVEL    Syn - phasing this out.
    &&*/ ((!IS_NPC(ch) && IS_SET(ch->act, PLR_LOG)) || logAll || cmd_table[cmd].log == LOG_ALWAYS))
    {
	char s[2 * MAX_INPUT_LENGTH];
	char *ps;
	int i;

	ps = s;
	sprintf( log_buf, "Log %s: %s",
	    IS_NPC(ch) ? ch->short_descr : ch->name, logline );

	// Make sure that was is displayed is what is typed
	for ( i = 0; log_buf[i]; i++ )
	{
	    *ps++ = log_buf[i];
	    if ( log_buf[i] == '$' )
		*ps++ = '$';
	    if ( log_buf[i] == '{' )
		*ps++ = '{';
	}

	*ps = 0;
	wiznet( s, ch, NULL, WIZ_SECURE, 0, get_trust(ch));
	if ( logline[0] != '\0' )
	    log_string( log_buf );
    }

    if ( ch->desc != NULL && ch->desc->snoop_by != NULL )
    {
	write_to_buffer( ch->desc->snoop_by, "% ",    2 );
	write_to_buffer( ch->desc->snoop_by, logline, 0 );
	write_to_buffer( ch->desc->snoop_by, "\n\r",  2 );
    }

    // Command not found... try other places.
    // Modified 2010-08-16 - Changed order. Command -> Custom verbs -> IMC -> Socials -- Tieryo
    if (!found)
    {
    	if (check_verbs(ch,command,argument))
		return;

	if (!IS_NPC(ch) && imc_command_hook(ch, command, argument))
		return;

	if (check_social(ch, command, argument))
		return;

	send_to_char( "Huh?\n\r", ch);
	return;
    }

    // Command found, let's execute it
    if (ch->position == POS_FEIGN)
    {
	do_function( ch, &do_feign, "");
	if ( !str_cmp( cmd_table[cmd].name, "feign") )
	    return;
    }

    if (ch->position == POS_HELDUP)
    {
        send_to_char( "{YYou are too fearful to move a muscle!{x\n\r", ch );
        return;
    }

    if (ch->heldup != NULL)
    {
	act( "You lose your concentration and $N escapes!", ch, ch->heldup, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
	act( "$n loses $s concentration, freeing you from the holdup!", ch, ch->heldup, NULL, NULL, NULL, NULL, NULL, TO_VICT);
	act( "$n loses $s concentration and $N frees $Mself!", ch, ch->heldup, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
 	stop_holdup(ch);
    }

    // Character not in position for command?
    if ( ch->position < cmd_table[cmd].position )
    {
	switch( ch->position )
	{
	case POS_DEAD:
	    send_to_char( "Lie still; you are DEAD.\n\r", ch );
	    break;

	case POS_MORTAL:
	case POS_INCAP:
	    send_to_char( "You are far too hurt for that.\n\r", ch );
	    break;

	case POS_STUNNED:
	    send_to_char( "You are too stunned to do that.\n\r", ch );
	    break;

	case POS_SLEEPING:
	    send_to_char( "In your dreams, or what?\n\r", ch );
	    break;

	case POS_RESTING:
	    send_to_char( "You are resting at the moment.\n\r", ch);
	    break;

	case POS_SITTING:
	    send_to_char( "Better stand up first.\n\r",ch);
	    break;

	case POS_FIGHTING:
	    send_to_char( "No way!  You are still fighting!\n\r", ch);
	    break;
	}

	return;
    }

    // Dispatch the command
    (*cmd_table[cmd].do_fun) ( ch, argument );

    tail_chain();
}


// function to keep argument safe in all commands -- no static strings
void do_function( CHAR_DATA *ch, DO_FUN *do_fun, char *argument )
{
    char *command_string;

    // copy the string
    command_string = argument ? str_dup(argument) : NULL;

    // dispatch the command
    (*do_fun) (ch, command_string);

    // free the string
    if(command_string) free_string(command_string);
}


// Check if a command is a social and execute it if it is.
bool check_social( CHAR_DATA *ch, char *command, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int cmd;
    bool found;

    found  = FALSE;
    for ( cmd = 0; social_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( command[0] == social_table[cmd].name[0]
	&&   !str_prefix( command, social_table[cmd].name ) )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( !found )
	return FALSE;

    switch ( ch->position )
    {
	case POS_DEAD:
	    send_to_char( "Lie still; you are DEAD.\n\r", ch );
	    return TRUE;

	case POS_INCAP:
	case POS_MORTAL:
	    send_to_char( "You are hurt far too bad for that.\n\r", ch );
	    return TRUE;

	case POS_STUNNED:
	    send_to_char( "You are too stunned to do that.\n\r", ch );
	    return TRUE;

	case POS_SLEEPING:
	    /*
	     * I just know this is the path to a 12" 'if' statement.  :(
	     * But two players asked for it already!  -- Furey
	     */
	    if ( !str_cmp( social_table[cmd].name, "snore" ) )
		break;
	    send_to_char( "In your dreams, or what?\n\r", ch );
	    return TRUE;
    }

    one_argument( argument, arg );
    victim = NULL;
    if ( arg[0] == '\0' ) {
		act( social_table[cmd].others_no_arg, ch, victim, NULL, NULL, NULL, NULL, NULL, TO_ROOM    );
		act( social_table[cmd].char_no_arg,   ch, victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR    );
    }
    else if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
		send_to_char( "They aren't here.\n\r", ch );
    }
    else if ( victim == ch )
    {
		act( social_table[cmd].others_auto,   ch, victim, NULL, NULL, NULL, NULL, NULL, TO_ROOM    );
		act( social_table[cmd].char_auto,     ch, victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR    );
    }
    else
    {
		act( social_table[cmd].others_found,  ch, victim, NULL, NULL, NULL, NULL, NULL, TO_NOTVICT );
		act( social_table[cmd].char_found,    ch, victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR    );
		act( social_table[cmd].vict_found,    ch, victim, NULL, NULL, NULL, NULL, NULL, TO_VICT    );
    }

    // 20140508NIB - Adding EMOTE triggering

    if( victim != NULL )
		p_emoteat_trigger(victim, ch, social_table[cmd].name);
	else
		p_emote_trigger(ch, social_table[cmd].name);

    return TRUE;
}


// Return true if an argument is completely numeric.
bool is_number( char *arg )
{
    if ( *arg == '\0' )
        return FALSE;

    if ( *arg == '+' || *arg == '-' )
        arg++;

    for ( ; *arg != '\0'; arg++ )
    {
        if ( !isdigit( *arg ) )
            return FALSE;
    }

    return TRUE;
}


// Given a string like 14.foo, return 14 and 'foo'
int number_argument( char *argument, char *arg )
{
    char *pdot;
    int number;

    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
	if ( *pdot == '.' )
	{
	    *pdot = '\0';
	    number = atoi( argument );
	    *pdot = '.';
	    strcpy( arg, pdot+1 );
	    return number;
	}
    }

    strcpy( arg, argument );
    return 1;
}


// Given a string like 14*foo, return 14 and 'foo'
int mult_argument(char *argument, char *arg)
{
    char *pdot;
    int number;

    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
        if ( *pdot == '*' )
        {
            *pdot = '\0';
            number = atoi( argument );
            *pdot = '*';
            strcpy( arg, pdot+1 );
            return number;
        }
    }

    strcpy( arg, argument );
    return 1;
}


// Same as one_argument but doesn't lower case the argument
char *one_argument_norm( char *argument, char *arg_first )
{
    char cEnd;

    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*arg_first = *argument;
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
	argument++;

    return argument;
}


// Pick off one argument from a string and return the rest. Understands quotes.
char *one_argument( char *argument, char *arg_first )
{
    char cEnd;

    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*arg_first = LOWER(*argument);
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
	argument++;

    return argument;
}


/*
 *  * Pick off one argument from a string and return the rest.
 *   * Understands quotes.
 *    */
char *one_caseful_argument (char *argument, char *arg_first)
{
    char cEnd;

    while (isspace (*argument))
        argument++;

    cEnd = ' ';
    if (*argument == '\'' || *argument == '"' || *argument == '\'')
        cEnd = *argument++;

    while (*argument != '\0')
    {
        if (*argument == cEnd)
        {
            argument++;
            break;
        }
        *arg_first = *argument;
        arg_first++;
        argument++;
    }
    *arg_first = '\0';

    while (isspace (*argument))
        argument++;

    return argument;
}


// Output a table of commands.
void do_commands( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int cmd;
    int col;

    col = 0;
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( cmd_table[cmd].level <  LEVEL_HERO
        &&   cmd_table[cmd].level <= get_trust( ch )
	&&   cmd_table[cmd].show )
	{
	    sprintf( buf, "%-12s", cmd_table[cmd].name );
	    send_to_char( buf, ch );
	    if ( ++col % 6 == 0 )
		send_to_char( "\n\r", ch );
	}
    }

    if ( col % 6 != 0 )
	send_to_char( "\n\r", ch );
}


// Output a table of imm-only commands.
void do_wizhelp( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int cmd;
    int col;

    col = 0;
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( cmd_table[cmd].level >= LEVEL_HERO
        &&   cmd_table[cmd].level <= get_trust( ch )
	&&   cmd_table[cmd].show)
	{
            sprintf( buf, "%-12s", cmd_table[cmd].name );
            send_to_char( buf, ch );

	    if ( ++col % 6 == 0 )
		send_to_char( "\n\r", ch );
	}
    }

    if ( col % 6 != 0 )
	send_to_char( "\n\r", ch );
}


// Certain informational commands are allowed during paralysis, etc. These are listed here.
bool is_allowed( char *command )
{
    if ( !str_cmp( command, "look")
    || !str_cmp( command, "affects")
    || !str_cmp( command, "whois")
    || !str_cmp( command, "equipment")
    || !str_cmp( command, "inventory")
    || !str_cmp( command, "score") )
        return TRUE;

    return FALSE;
}

// interrupt a chars spell. safe version (no memory leaks)
void stop_music( CHAR_DATA *ch, bool messages )
{

	// Allow for custom messages as well as handling interrupted songs
	if(ch->song_token) {
		ch->tempstore[0] = messages?1:0;	// Tell the script whether to show messages or not
		if( p_percent_trigger(NULL,NULL,NULL,ch->song_token,ch, NULL, NULL,NULL,NULL,TRIG_SPELLINTER, NULL) )
			messages = FALSE;
	}
    free_string( ch->music_target_name );
    ch->music_target_name = NULL;
    ch->music = 0;
    ch->song_num = -1;
    ch->song_token = NULL;
    ch->song_script = NULL;
    ch->song_instrument = NULL;


    if ( messages )
		send_to_char("{YYou stop playing your song.{x\n\r", ch );
}


// interrupt a chars spell. safe version (no memory leaks)
void stop_casting( CHAR_DATA *ch, bool messages )
{

	// Allow for custom messages as well as handling interrupted spells
	if(ch->cast_token) {
		ch->tempstore[0] = messages?1:0;	// Tell the script whether to show messages or not
		if( p_percent_trigger(NULL,NULL,NULL,ch->cast_token,ch, NULL, NULL,NULL,NULL,TRIG_SPELLINTER, NULL) )
			messages = FALSE;
	}
	free_string(ch->casting_failure_message);
	ch->casting_failure_message = NULL;
    free_string( ch->cast_target_name );
    ch->cast_target_name = NULL;
    ch->cast = 0;
    ch->cast_sn = -1;
    ch->cast_token = NULL;
    ch->cast_script = NULL;


    if ( messages )
    {
	send_to_char("{WYou stop your casting.{x\n\r", ch);
	if (number_percent() < 10)
	{
	    send_to_char("{YSmall yellow sparks spiral around you then fade away.{x\n\r", ch);
	    act("$n's magic fizzles and dies.{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
	}
	else
	if (number_percent() < 20)
	{
	    send_to_char("{YYou hear a loud bang as your magic dissipates.{x\n\r", ch);
	    act("{YYou hear a loud bang as $n stops $s casting.{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
	}
	else
	if (number_percent() < 30)
	{
	    send_to_char("{YA puff of smoke billows out of your ears.{x\n\r", ch);
	    act("{YA puff of smoke billows out of $n's ears as $e stops $s casting.{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
	}
	else
	if (number_percent() < 40)
	{
	    send_to_char("{YYour skin turns multicoloured then turns back to normal.{x\n\r", ch);
	    act("{Y$n's skin turns multicoloured momentarily as $e stops $s casting.{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
	}
	else
	if (number_percent() < 50)
	{
	    send_to_char("{YYour magic fizzles and dies.\n\r{x", ch );
	    act("{Y$n's magic fizzles and dies.{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
	}
	else
	if ( number_percent() < 60 )
	{
	    send_to_char("{YEnergy sizzles as you stop your casting.\n\r{x",
		    ch );
	    act("{YEnergy sizzles as $n stops $s casting.{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM );
	}
	else
	if (number_percent() < 70 )
	{
	    send_to_char("{YSparks fly from your fingers as your magic dissipates.{x\n\r", ch );
	    act("{YSparks fly from $n's fingers as $s magic dissipates.{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM );
	}
	else
	if (number_percent() < 80 )
	{
	    send_to_char("{YYou eyes flash with white light as you interrupt your spell.{x\n\r", ch );
	    act("{Y$n's eyes flash with white light as $e interrupts $s spell.{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM );
	}
	else
	if ( number_percent() < 90 )
	{
	    send_to_char("{YYour hair stands on end for a moment as you stop your spell.{x\n\r", ch );
	    act("{Y$n's hair stands on end for a moment as $e finishes $s spell.", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM );
	}
	else
	{
	    send_to_char("{YYour magic dissipates into the air.{x\n\r", ch );
	    act("{Y$n's magic dissipates into the air.{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM );
	}
    }
}


// Stop ranged attacks safely.
void stop_ranged( CHAR_DATA *ch, bool messages )
{
    if ( ch == NULL )
    {
	bug( "stop_ranged: null ch", 0 );
	return;
    }

    if ( ch->projectile_weapon != NULL )
    {
	if ( messages )
	{
	    act("You put down $p.", ch, NULL, NULL, ch->projectile_weapon, NULL, NULL, NULL, TO_CHAR );
	    act("$n puts down $p.", ch, NULL, NULL, ch->projectile_weapon, NULL, NULL, NULL, TO_ROOM );
	}

	ch->ranged = 0;
	ch->projectile_weapon = NULL;
	free_string( ch->projectile_victim );
	ch->projectile_victim = NULL;
	ch->projectile_dir    = -1;
	ch->projectile_range  = 0;
	ch->projectile	      = NULL;
    }
}


bool is_granted_command(CHAR_DATA *ch, char *name)
{
    COMMAND_DATA *cmd;

    if (IS_NPC(ch)) {
	bug("is_granted_command: checking an NPC", 0);
	return FALSE;
    }

    for (cmd = ch->pcdata->commands; cmd != NULL; cmd = cmd->next)
    {
	if (!str_cmp(cmd->name, name))
	    return TRUE;
    }

    return FALSE;
}
