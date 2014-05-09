#include "merc.h"
#include "tables.h"
#include "scripts.h"

char *opcode_names[OP_LASTCODE] = {
	"END",
	"IF",
	"IF !",
	"OR",
	"OR !",
	"AND",
	"AND !",
	"ELSEIF",
	"ELSEIF !",
	"ELSE",
	"ENDIF",
	"COMMAND",
	"GOTOLINE",
	"FOR",
	"ENDFOR",
	"EXITFOR",
	"LIST",
	"ENDLIST",
	"EXITLIST",
	"WHILE",
	"WHILE !",
	"ENDWHILE",
	"EXITWHILE",
	"MOB <command>",
	"OBJ <command>",
	"ROOM <command>",
	"TOKEN <command>",
	"TOKEN <command> (other)",
};

char *ifcheck_param_type_names[IFCP_MAX] = {
	"NONE",
	"NUMBER",
	"STRING",
	"MOBILE",
	"OBJECT",
	"ROOM",
	"TOKEN",
	"AREA",
	"EXIT"
};

const ENT_FIELD entity_primary[] = {
	{"enactor",	ENTITY_ENACTOR,		ENT_MOBILE	},
	{"obj1",	ENTITY_OBJ1,		ENT_OBJECT	},
	{"obj2",	ENTITY_OBJ2,		ENT_OBJECT	},
	{"victim",	ENTITY_VICTIM,		ENT_MOBILE	},
	{"target",	ENTITY_TARGET,		ENT_MOBILE	},
	{"random",	ENTITY_RANDOM,		ENT_MOBILE	},
	{"here",	ENTITY_HERE,		ENT_ROOM	},
	{"this",	ENTITY_SELF,		ENT_UNKNOWN	},
	{"self",	ENTITY_SELF,		ENT_UNKNOWN	},
	{"phrase",	ENTITY_PHRASE,		ENT_STRING	},
	{"trigger",	ENTITY_TRIGGER,		ENT_STRING	},
	{"prior",	ENTITY_PRIOR,		ENT_PRIOR	},
	{NULL,		0,			ENT_UNKNOWN	}
};

const ENT_FIELD entity_types[] = {
	{"num",		ENTITY_VAR_NUM,		ENT_NUMBER	},
	{"str",		ENTITY_VAR_STR,		ENT_STRING	},
	{"mob",		ENTITY_VAR_MOB,		ENT_MOBILE	},
	{"obj",		ENTITY_VAR_OBJ,		ENT_OBJECT	},
	{"room",	ENTITY_VAR_ROOM,	ENT_ROOM	},
	{"exit",	ENTITY_VAR_EXIT,	ENT_EXIT	},
	{"token",	ENTITY_VAR_TOKEN,	ENT_TOKEN	},
	{"area",	ENTITY_VAR_AREA,	ENT_AREA	},
	{"skill",	ENTITY_VAR_SKILL,	ENT_SKILL	},
	{"skillinfo",	ENTITY_VAR_SKILLINFO,	ENT_SKILLINFO	},
	{"aff",		ENTITY_VAR_AFFECT,	ENT_AFFECT	},
	{NULL,		0,			ENT_UNKNOWN	}
};

const ENT_FIELD entity_prior[] = {
	{"mob",		ENTITY_PRIOR_MOB,	ENT_MOBILE	},
	{"obj",		ENTITY_PRIOR_OBJ,	ENT_OBJECT	},
	{"room",	ENTITY_PRIOR_ROOM,	ENT_ROOM	},
	{"token",	ENTITY_PRIOR_TOKEN,	ENT_TOKEN	},
	{"enactor",	ENTITY_PRIOR_ENACTOR,	ENT_MOBILE	},
	{"obj1",	ENTITY_PRIOR_OBJ1,	ENT_OBJECT	},
	{"obj2",	ENTITY_PRIOR_OBJ2,	ENT_OBJECT	},
	{"victim",	ENTITY_PRIOR_VICTIM,	ENT_MOBILE	},
	{"target",	ENTITY_PRIOR_TARGET,	ENT_MOBILE	},
	{"random",	ENTITY_PRIOR_RANDOM,	ENT_MOBILE	},
	{"here",	ENTITY_PRIOR_HERE,	ENT_ROOM	},
	{"phrase",	ENTITY_PRIOR_PHRASE,	ENT_STRING	},
	{"trigger",	ENTITY_PRIOR_TRIGGER,	ENT_STRING	},
	{"prior",	ENTITY_PRIOR_PRIOR,	ENT_PRIOR	},
	{NULL,		0,			ENT_UNKNOWN	}
};

const ENT_FIELD entity_number[] = {
	{"abs",		ENTITY_NUM_ABS,		ENT_NUMBER	},
	{NULL,		0,			ENT_UNKNOWN	}
};

const ENT_FIELD entity_string[] = {
	{"len",		ENTITY_STR_LEN,		ENT_NUMBER	},
	{"length",	ENTITY_STR_LEN,		ENT_NUMBER	},
	{NULL,		0,			ENT_UNKNOWN	}
};

const ENT_FIELD entity_mobile[] = {
	{"name",	ENTITY_MOB_NAME,	ENT_STRING	},
	{"short",	ENTITY_MOB_SHORT,	ENT_STRING	},
	{"long",	ENTITY_MOB_LONG,	ENT_STRING	},
	{"sex",		ENTITY_MOB_SEX,		ENT_STRING	},
	{"gender",	ENTITY_MOB_SEX,		ENT_STRING	},
	{"he",		ENTITY_MOB_HE,		ENT_STRING	},
	{"him",		ENTITY_MOB_HIM,		ENT_STRING	},
	{"his",		ENTITY_MOB_HIS,		ENT_STRING	},
	{"hisobj",	ENTITY_MOB_HIS_O,	ENT_STRING	},
	{"himself",	ENTITY_MOB_HIMSELF,	ENT_STRING	},
	{"race",	ENTITY_MOB_RACE,	ENT_STRING	},
	{"room",	ENTITY_MOB_ROOM,	ENT_ROOM	},
	{"house",	ENTITY_MOB_HOUSE,	ENT_ROOM	},
	{"home",	ENTITY_MOB_HOUSE,	ENT_ROOM	},
	{"carrying",	ENTITY_MOB_CARRYING,	ENT_LIST_OBJ	},
	{"inv",		ENTITY_MOB_CARRYING,	ENT_LIST_OBJ	},
	{"tokens",	ENTITY_MOB_TOKENS,	ENT_LIST_TOK	},
	{"affects",	ENTITY_MOB_AFFECTS,	ENT_LIST_AFF	},
	{"mount",	ENTITY_MOB_MOUNT,	ENT_MOBILE	},
	{"rider",	ENTITY_MOB_RIDER,	ENT_MOBILE	},
	{"master",	ENTITY_MOB_MASTER,	ENT_MOBILE	},
	{"leader",	ENTITY_MOB_LEADER,	ENT_MOBILE	},
	{"owner",	ENTITY_MOB_OWNER,	ENT_MOBILE	},
	{"opponent",	ENTITY_MOB_OPPONENT,	ENT_MOBILE	},
	{"cart",	ENTITY_MOB_CART,	ENT_OBJECT	},
	{"bedroll",	ENTITY_MOB_FURNITURE,	ENT_OBJECT	},
	{"on",		ENTITY_MOB_FURNITURE,	ENT_OBJECT	},
	{"target",	ENTITY_MOB_TARGET,	ENT_MOBILE	},
	{"hunting",	ENTITY_MOB_HUNTING,	ENT_MOBILE	},
	{"prey",	ENTITY_MOB_HUNTING,	ENT_MOBILE	},
	{"area",	ENTITY_MOB_AREA,	ENT_AREA	},
	{"eq_light",	ENTITY_MOB_EQ_LIGHT,	ENT_OBJECT	},
	{"eq_finger1",	ENTITY_MOB_EQ_FINGER1,	ENT_OBJECT	},
	{"eq_finger2",	ENTITY_MOB_EQ_FINGER2,	ENT_OBJECT	},
	{"eq_ring",	ENTITY_MOB_EQ_RING,	ENT_OBJECT	},
	{"eq_neck1",	ENTITY_MOB_EQ_NECK1,	ENT_OBJECT	},
	{"eq_neck2",	ENTITY_MOB_EQ_NECK2,	ENT_OBJECT	},
	{"eq_body",	ENTITY_MOB_EQ_BODY,	ENT_OBJECT	},
	{"eq_head",	ENTITY_MOB_EQ_HEAD,	ENT_OBJECT	},
	{"eq_legs",	ENTITY_MOB_EQ_LEGS,	ENT_OBJECT	},
	{"eq_feet",	ENTITY_MOB_EQ_FEET,	ENT_OBJECT	},
	{"eq_arms",	ENTITY_MOB_EQ_ARMS,	ENT_OBJECT	},
	{"eq_waist",	ENTITY_MOB_EQ_WAIST,	ENT_OBJECT	},
	{"eq_about",	ENTITY_MOB_EQ_ABOUT,	ENT_OBJECT	},
	{"eq_shoulder",	ENTITY_MOB_EQ_SHOULDER,	ENT_OBJECT	},
	{"eq_back",	ENTITY_MOB_EQ_BACK,	ENT_OBJECT	},
	{"eq_wrist1",	ENTITY_MOB_EQ_WRIST1,	ENT_OBJECT	},
	{"eq_wrist2",	ENTITY_MOB_EQ_WRIST2,	ENT_OBJECT	},
	{"eq_hands",	ENTITY_MOB_EQ_HANDS,	ENT_OBJECT	},
	{"eq_wield1",	ENTITY_MOB_EQ_WIELD1,	ENT_OBJECT	},
	{"eq_wield2",	ENTITY_MOB_EQ_WIELD2,	ENT_OBJECT	},
	{"eq_hold",	ENTITY_MOB_EQ_HOLD,	ENT_OBJECT	},
	{"eq_shield",	ENTITY_MOB_EQ_SHIELD,	ENT_OBJECT	},
	{"eq_ankle1",	ENTITY_MOB_EQ_ANKLE1,	ENT_OBJECT	},
	{"eq_ankle2",	ENTITY_MOB_EQ_ANKLE2,	ENT_OBJECT	},
	{"eq_ear1",	ENTITY_MOB_EQ_EAR1,	ENT_OBJECT	},
	{"eq_ear2",	ENTITY_MOB_EQ_EAR2,	ENT_OBJECT	},
	{"eq_eyes",	ENTITY_MOB_EQ_EYES,	ENT_OBJECT	},
	{"eq_face",	ENTITY_MOB_EQ_FACE,	ENT_OBJECT	},
	{"eq_tattoo_head",	ENTITY_MOB_EQ_TATTOO_HEAD,	ENT_OBJECT	},
	{"eq_tattoo_torso",	ENTITY_MOB_EQ_TATTOO_TORSO,	ENT_OBJECT	},
	{"eq_tattoo_arm1",	ENTITY_MOB_EQ_TATTOO_ARM1,	ENT_OBJECT	},
	{"eq_tattoo_arm2",	ENTITY_MOB_EQ_TATTOO_ARM2,	ENT_OBJECT	},
	{"eq_tattoo_leg1",	ENTITY_MOB_EQ_TATTOO_LEG1,	ENT_OBJECT	},
	{"eq_tattoo_leg2",	ENTITY_MOB_EQ_TATTOO_LEG2,	ENT_OBJECT	},
	{"eq_lodged_head",	ENTITY_MOB_EQ_LODGED_HEAD,	ENT_OBJECT	},
	{"eq_lodged_torso",	ENTITY_MOB_EQ_LODGED_TORSO,	ENT_OBJECT	},
	{"eq_lodged_arm1",	ENTITY_MOB_EQ_LODGED_ARM1,	ENT_OBJECT	},
	{"eq_lodged_arm2",	ENTITY_MOB_EQ_LODGED_ARM2,	ENT_OBJECT	},
	{"eq_lodged_leg1",	ENTITY_MOB_EQ_LODGED_LEG1,	ENT_OBJECT	},
	{"eq_lodged_leg2",	ENTITY_MOB_EQ_LODGED_LEG2,	ENT_OBJECT	},
	{"eq_entangled",	ENTITY_MOB_EQ_ENTANGLED,	ENT_OBJECT	},
	{"eq_concealed",	ENTITY_MOB_EQ_CONCEALED,	ENT_OBJECT	},
	{"casttoken",	ENTITY_MOB_CASTTOKEN,	ENT_TOKEN	},
	{"casttarget",	ENTITY_MOB_CASTTARGET,	ENT_STRING	},
	{"recall",	ENTITY_MOB_RECALL,	ENT_ROOM	},
	{"next",	ENTITY_MOB_NEXT,	ENT_MOBILE	},
	{NULL,		0,			ENT_UNKNOWN	}
};

const ENT_FIELD entity_object[] = {
	{"name",	ENTITY_OBJ_NAME,	ENT_STRING	},
	{"short",	ENTITY_OBJ_SHORT,	ENT_STRING	},
	{"long",	ENTITY_OBJ_LONG,	ENT_STRING	},
	{"in",		ENTITY_OBJ_CONTAINER,	ENT_OBJECT	},
	{"container",	ENTITY_OBJ_CONTAINER,	ENT_OBJECT	},
	{"on",		ENTITY_OBJ_FURNITURE,	ENT_OBJECT	},
	{"contents",	ENTITY_OBJ_CONTENTS,	ENT_LIST_OBJ	},
	{"inv",		ENTITY_OBJ_CONTENTS,	ENT_LIST_OBJ	},
	{"items",	ENTITY_OBJ_CONTENTS,	ENT_LIST_OBJ	},
	{"owner",	ENTITY_OBJ_OWNER,	ENT_MOBILE	},
	{"room",	ENTITY_OBJ_ROOM,	ENT_ROOM	},
	{"user",	ENTITY_OBJ_CARRIER,	ENT_MOBILE	},
	{"carrier",	ENTITY_OBJ_CARRIER,	ENT_MOBILE	},
	{"wearer",	ENTITY_OBJ_CARRIER,	ENT_MOBILE	},
	{"tokens",	ENTITY_OBJ_TOKENS,	ENT_LIST_TOK	},
	{"target",	ENTITY_OBJ_TARGET,	ENT_MOBILE	},
	{"area",	ENTITY_OBJ_AREA,	ENT_AREA	},
	{"ed",		ENTITY_OBJ_EXTRADESC,	ENT_EXTRADESC	},
	{"next",	ENTITY_OBJ_NEXT,	ENT_OBJECT	},
	{NULL,		0,			ENT_UNKNOWN	}
};

const ENT_FIELD entity_room[] = {
	{"name",	ENTITY_ROOM_NAME,	ENT_STRING	},
	{"mobiles",	ENTITY_ROOM_MOBILES,	ENT_LIST_MOB	},
	{"objects",	ENTITY_ROOM_OBJECTS,	ENT_LIST_OBJ	},
	{"tokens",	ENTITY_ROOM_TOKENS,	ENT_LIST_TOK	},
	{"area",	ENTITY_ROOM_AREA,	ENT_AREA	},
	{"target",	ENTITY_ROOM_TARGET,	ENT_MOBILE	},
	{"north",	ENTITY_ROOM_NORTH,	ENT_EXIT	},
	{"east",	ENTITY_ROOM_EAST,	ENT_EXIT	},
	{"south",	ENTITY_ROOM_SOUTH,	ENT_EXIT	},
	{"west",	ENTITY_ROOM_WEST,	ENT_EXIT	},
	{"up",		ENTITY_ROOM_UP,		ENT_EXIT	},
	{"down",	ENTITY_ROOM_DOWN,	ENT_EXIT	},
	{"northeast",	ENTITY_ROOM_NORTHEAST,	ENT_EXIT	},
	{"northwest",	ENTITY_ROOM_NORTHWEST,	ENT_EXIT	},
	{"southeast",	ENTITY_ROOM_SOUTHEAST,	ENT_EXIT	},
	{"southwest",	ENTITY_ROOM_SOUTHWEST,	ENT_EXIT	},
	{"ed",		ENTITY_ROOM_EXTRADESC,	ENT_EXTRADESC	},
	{"desc",	ENTITY_ROOM_DESC,	ENT_STRING	},

	// Environment stuff
	{"environ",	ENTITY_ROOM_ENVIRON,	ENT_ROOM	},
	{"environment",	ENTITY_ROOM_ENVIRON,	ENT_ROOM	},
	{"outside",	ENTITY_ROOM_ENVIRON,	ENT_ROOM	},
	{"extern",	ENTITY_ROOM_ENVIRON,	ENT_ROOM	},

	{"env_mob",	ENTITY_ROOM_ENVIRON_MOB,	ENT_MOBILE	},
	{"env_obj",	ENTITY_ROOM_ENVIRON_OBJ,	ENT_OBJECT	},
	{"env_room",	ENTITY_ROOM_ENVIRON_ROOM,	ENT_ROOM	},
	{NULL,		0,			ENT_UNKNOWN	}
};

const ENT_FIELD entity_exit[] = {
	{"name",	ENTITY_EXIT_NAME,	ENT_STRING	},
	{"door",	ENTITY_EXIT_DOOR,	ENT_NUMBER	},
	{"src",		ENTITY_EXIT_SOURCE,	ENT_ROOM	},
	{"here",	ENTITY_EXIT_SOURCE,	ENT_ROOM	},
	{"source",	ENTITY_EXIT_SOURCE,	ENT_ROOM	},
	{"dest",	ENTITY_EXIT_REMOTE,	ENT_ROOM	},
	{"remote",	ENTITY_EXIT_REMOTE,	ENT_ROOM	},
	{"destination",	ENTITY_EXIT_REMOTE,	ENT_ROOM	},
	{"state",	ENTITY_EXIT_STATE,	ENT_STRING	},
	{"mate",	ENTITY_EXIT_MATE,	ENT_EXIT	},
	{"north",	ENTITY_EXIT_NORTH,	ENT_EXIT	},
	{"east",	ENTITY_EXIT_EAST,	ENT_EXIT	},
	{"south",	ENTITY_EXIT_SOUTH,	ENT_EXIT	},
	{"west",	ENTITY_EXIT_WEST,	ENT_EXIT	},
	{"up",		ENTITY_EXIT_UP,		ENT_EXIT	},
	{"down",	ENTITY_EXIT_DOWN,	ENT_EXIT	},
	{"northeast",	ENTITY_EXIT_NORTHEAST,	ENT_EXIT	},
	{"northwest",	ENTITY_EXIT_NORTHWEST,	ENT_EXIT	},
	{"southeast",	ENTITY_EXIT_SOUTHEAST,	ENT_EXIT	},
	{"southwest",	ENTITY_EXIT_SOUTHWEST,	ENT_EXIT	},
	{"next",	ENTITY_EXIT_NEXT,	ENT_EXIT	},
	{NULL,		0,			ENT_UNKNOWN	}
};

const ENT_FIELD entity_token[] = {
	{"name",	ENTITY_TOKEN_NAME,	ENT_STRING	},
	{"owner",	ENTITY_TOKEN_OWNER,	ENT_MOBILE	},
	{"timer",	ENTITY_TOKEN_TIMER,	ENT_NUMBER	},
	{"val0",	ENTITY_TOKEN_VAL0,	ENT_NUMBER	},
	{"val1",	ENTITY_TOKEN_VAL1,	ENT_NUMBER	},
	{"val2",	ENTITY_TOKEN_VAL2,	ENT_NUMBER	},
	{"val3",	ENTITY_TOKEN_VAL3,	ENT_NUMBER	},
	{"next",	ENTITY_TOKEN_NEXT,	ENT_TOKEN	},
	{NULL,		0,			ENT_UNKNOWN	}
};

const ENT_FIELD entity_area[] = {
	{"name",	ENTITY_AREA_NAME,	ENT_STRING	},
	{"recall",	ENTITY_AREA_RECALL,	ENT_ROOM	},
	{"post",	ENTITY_AREA_POSTOFFICE,	ENT_ROOM	},
//	{"lvnum",	ENTITY_AREA_LOWERVNUM,	ENT_NUMBER	},
//	{"uvnum",	ENTITY_AREA_UPPERVNUM,	ENT_NUMBER	},
//	{"location",	ENTITY_AREA_LOCATION,	ENT_NUMBER	},
	{NULL,		0,			ENT_UNKNOWN	}
};

const ENT_FIELD entity_list[] = {
	{"count",	ENTITY_LIST_SIZE,	ENT_NUMBER	},
	{"size",	ENTITY_LIST_SIZE,	ENT_NUMBER	},
	{"len",		ENTITY_LIST_SIZE,	ENT_NUMBER	},
	{"length",	ENTITY_LIST_SIZE,	ENT_NUMBER	},
	{"random",	ENTITY_LIST_RANDOM,	ENT_UNKNOWN	},
	{"first",	ENTITY_LIST_FIRST,	ENT_UNKNOWN	},
	{"last",	ENTITY_LIST_LAST,	ENT_UNKNOWN	},
	{NULL,		0,			ENT_UNKNOWN	}
};

const ENT_FIELD entity_skill[] = {
	{"gsn",		ENTITY_SKILL_GSN,		ENT_NUMBER	},
	{"spell",	ENTITY_SKILL_SPELL,		ENT_NUMBER	},
	{"name",	ENTITY_SKILL_NAME,		ENT_STRING	},
	{"beats",	ENTITY_SKILL_BEATS,		ENT_NUMBER	},
	{"timer",	ENTITY_SKILL_BEATS,		ENT_NUMBER	},
	{"levelwarrior",	ENTITY_SKILL_LEVEL_WARRIOR,	ENT_NUMBER	},
	{"levelcleric",	ENTITY_SKILL_LEVEL_CLERIC,	ENT_NUMBER	},
	{"levelmage",	ENTITY_SKILL_LEVEL_MAGE,	ENT_NUMBER	},
	{"levelthief",	ENTITY_SKILL_LEVEL_THIEF,	ENT_NUMBER	},
	{"diffwarrior",	ENTITY_SKILL_DIFFICULTY_WARRIOR,	ENT_NUMBER	},
	{"diffcleric",	ENTITY_SKILL_DIFFICULTY_CLERIC,	ENT_NUMBER	},
	{"diffmage",	ENTITY_SKILL_DIFFICULTY_MAGE,	ENT_NUMBER	},
	{"diffthief",	ENTITY_SKILL_DIFFICULTY_THIEF,	ENT_NUMBER	},
	{"target",	ENTITY_SKILL_TARGET,		ENT_NUMBER	},
	{"position",	ENTITY_SKILL_POSITION,		ENT_NUMBER	},
	{"wearoff",	ENTITY_SKILL_WEAROFF,		ENT_STRING	},
	{"dispel",	ENTITY_SKILL_DISPEL,		ENT_STRING	},
	{"noun",	ENTITY_SKILL_NOUN,		ENT_STRING	},
	{"mana",	ENTITY_SKILL_MANA,		ENT_NUMBER	},
	{"inktype1",	ENTITY_SKILL_INK_TYPE1,		ENT_NUMBER	},
	{"inksize1",	ENTITY_SKILL_INK_SIZE1,		ENT_NUMBER	},
	{"inktype2",	ENTITY_SKILL_INK_TYPE2,		ENT_NUMBER	},
	{"inksize2",	ENTITY_SKILL_INK_SIZE2,		ENT_NUMBER	},
	{"inktype3",	ENTITY_SKILL_INK_TYPE3,		ENT_NUMBER	},
	{"inksize3",	ENTITY_SKILL_INK_SIZE3,		ENT_NUMBER	},
	{NULL,		0,			ENT_UNKNOWN	}
};

const ENT_FIELD entity_skill_info[] = {
	{"skill",	ENTITY_SKILLINFO_SKILL,		ENT_SKILL	},
	{"owner",	ENTITY_SKILLINFO_OWNER,		ENT_MOBILE	},
	{"rating",	ENTITY_SKILLINFO_RATING,	ENT_NUMBER	},
	{NULL,		0,			ENT_UNKNOWN	}
};

const ENT_FIELD entity_affect[] = {
	{"name",	ENTITY_AFFECT_NAME,	ENT_STRING	},
	{"group",	ENTITY_AFFECT_GROUP,	ENT_NUMBER	},
	{"skill",	ENTITY_AFFECT_SKILL,	ENT_NUMBER	},
	{"location",	ENTITY_AFFECT_LOCATION,	ENT_NUMBER	},
	{"mod",		ENTITY_AFFECT_MOD,	ENT_NUMBER	},
	{"duration",	ENTITY_AFFECT_TIMER,	ENT_NUMBER	},
	{"timer",	ENTITY_AFFECT_TIMER,	ENT_NUMBER	},
	{"level",	ENTITY_AFFECT_LEVEL,	ENT_NUMBER	},
	{NULL,		0,			ENT_UNKNOWN	}
};

const ENT_FIELD *entity_type_lists[] = {
	entity_primary,
	entity_number,
	entity_string,
	entity_mobile,
	entity_object,
	entity_room,
	entity_exit,
	entity_token,
	entity_area,
	entity_skill,
	entity_skill_info,
	entity_prior,
	entity_list,	// MOB
	entity_list,	// OBJ
	entity_list,	// TOK
	entity_list,	// AFF
	NULL,
	entity_affect,
	NULL
};

const bool entity_allow_vars[] = {
	TRUE,
	FALSE,
	FALSE,
	TRUE,
	TRUE,
	TRUE,
	FALSE,
	TRUE,
	FALSE,
	FALSE,
	FALSE,
	FALSE,
	FALSE,
	FALSE,
	FALSE,
	FALSE,
	FALSE,
	FALSE,
};


// Trigger typoes
struct trigger_type trigger_table	[] = {
//	name,				integer value,			slot,			mob?,	obj?,	room?,	token?
{	"act",				TRIG_ACT,			TRIGSLOT_ACTION,	TRUE,	TRUE,	TRUE,	TRUE	},
{	"afterdeath",			TRIG_AFTERDEATH,		TRIGSLOT_REPOP,		FALSE,	FALSE,	FALSE,	TRUE	},
{	"afterkill",			TRIG_AFTERKILL,			TRIGSLOT_FIGHT,		TRUE,	TRUE,	TRUE,	TRUE	},
{	"assist",			TRIG_ASSIST,			TRIGSLOT_FIGHT,		TRUE,	FALSE,	FALSE,	TRUE	},
{	"attack_backstab",		TRIG_ATTACK_BACKSTAB,		TRIGSLOT_ATTACKS,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"attack_bash",			TRIG_ATTACK_BASH,		TRIGSLOT_ATTACKS,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"attack_behead",		TRIG_ATTACK_BEHEAD,		TRIGSLOT_ATTACKS,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"attack_bite",			TRIG_ATTACK_BITE,		TRIGSLOT_ATTACKS,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"attack_blackjack",		TRIG_ATTACK_BLACKJACK,		TRIGSLOT_ATTACKS,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"attack_circle",		TRIG_ATTACK_CIRCLE,		TRIGSLOT_ATTACKS,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"attack_counter",		TRIG_ATTACK_COUNTER,		TRIGSLOT_ATTACKS,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"attack_cripple",		TRIG_ATTACK_CRIPPLE,		TRIGSLOT_ATTACKS,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"attack_dirtkick",		TRIG_ATTACK_DIRTKICK,		TRIGSLOT_ATTACKS,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"attack_disarm",		TRIG_ATTACK_DISARM,		TRIGSLOT_ATTACKS,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"attack_intimidate",		TRIG_ATTACK_INTIMIDATE,		TRIGSLOT_ATTACKS,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"attack_kick",			TRIG_ATTACK_KICK,		TRIGSLOT_ATTACKS,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"attack_rend",			TRIG_ATTACK_REND,		TRIGSLOT_ATTACKS,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"attack_slit",			TRIG_ATTACK_SLIT,		TRIGSLOT_ATTACKS,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"attack_smite",			TRIG_ATTACK_SMITE,		TRIGSLOT_ATTACKS,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"attack_tailkick",		TRIG_ATTACK_TAILKICK,		TRIGSLOT_ATTACKS,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"attack_trample",		TRIG_ATTACK_TRAMPLE,		TRIGSLOT_ATTACKS,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"attack_turn",			TRIG_ATTACK_TURN,		TRIGSLOT_ATTACKS,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"barrier",			TRIG_BARRIER,			TRIGSLOT_ATTACKS,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"blow",				TRIG_BLOW,			TRIGSLOT_GENERAL,	FALSE,	TRUE,	FALSE,	FALSE	},
{	"board",			TRIG_BOARD,			TRIGSLOT_GENERAL,	TRUE,	TRUE,	TRUE,	TRUE	},
{	"brandish",			TRIG_BRANDISH,			TRIGSLOT_GENERAL,	FALSE,	TRUE,	FALSE,	TRUE	},
{	"bribe",			TRIG_BRIBE,			TRIGSLOT_GENERAL,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"bury",				TRIG_BURY,			TRIGSLOT_GENERAL,	FALSE,	TRUE,	TRUE,	TRUE	},
{	"caninterrupt",			TRIG_CANINTERRUPT,		TRIGSLOT_INTERRUPT,	FALSE,	FALSE,	FALSE,	TRUE	},
{	"catalyst",			TRIG_CATALYST,			TRIGSLOT_GENERAL,	FALSE,  TRUE,	FALSE,	FALSE	},
{	"catalystfull",			TRIG_CATALYST_FULL,		TRIGSLOT_GENERAL,	FALSE,  TRUE,	FALSE,	FALSE	},
{	"catalystsrc",			TRIG_CATALYST_SOURCE,		TRIGSLOT_GENERAL,	FALSE,  TRUE,	FALSE,	FALSE	},
{	"check_damage",			TRIG_CHECK_DAMAGE,		TRIGSLOT_DAMAGE,	TRUE, TRUE, TRUE, TRUE	},
{	"close",			TRIG_CLOSE,			TRIGSLOT_GENERAL,	FALSE,  TRUE,	TRUE,	TRUE	},
{	"damage",			TRIG_DAMAGE,			TRIGSLOT_DAMAGE,	TRUE,	TRUE,	TRUE,	TRUE	},
{	"death",			TRIG_DEATH,			TRIGSLOT_REPOP,		TRUE,	FALSE,	TRUE,	TRUE	},
{	"delay",			TRIG_DELAY,			TRIGSLOT_GENERAL,	TRUE,	TRUE,	TRUE,	TRUE	},
{	"drink",			TRIG_DRINK,			TRIGSLOT_GENERAL,	TRUE,	TRUE,	TRUE,	TRUE	},
{	"drop",				TRIG_DROP,			TRIGSLOT_GENERAL,	TRUE,	TRUE,	TRUE,	TRUE	},
{	"eat",				TRIG_EAT,			TRIGSLOT_GENERAL,	TRUE,	TRUE,	TRUE,	TRUE	},
{	"entry",			TRIG_ENTRY,			TRIGSLOT_MOVE,		TRUE,	TRUE,	TRUE,	TRUE	},
{	"exall",			TRIG_EXALL,			TRIGSLOT_MOVE,		TRUE,	TRUE,	TRUE,	TRUE	},
{	"examine",			TRIG_EXAMINE,			TRIGSLOT_GENERAL,	FALSE,	TRUE,	FALSE,	TRUE	},
{	"exit",				TRIG_EXIT,			TRIGSLOT_MOVE,		TRUE,	TRUE,	TRUE,	TRUE	},
{	"expire",			TRIG_EXPIRE,			TRIGSLOT_REPOP,		FALSE,	FALSE,	FALSE,	TRUE	},
{	"extract",			TRIG_EXTRACT,			TRIGSLOT_REPOP,		TRUE,	TRUE,	TRUE,	FALSE	},
{	"fight",			TRIG_FIGHT,			TRIGSLOT_FIGHT,		TRUE,	TRUE,	TRUE,	TRUE	},
{	"flee",				TRIG_FLEE,			TRIGSLOT_GENERAL,	TRUE,	TRUE,	TRUE,	TRUE	},
{	"forcedismount",		TRIG_FORCEDISMOUNT,		TRIGSLOT_GENERAL,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"get",				TRIG_GET,			TRIGSLOT_GENERAL,	TRUE,	TRUE,	TRUE,	TRUE	},
{	"give",				TRIG_GIVE,			TRIGSLOT_GENERAL,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"grall",			TRIG_GRALL,			TRIGSLOT_MOVE,		TRUE,	TRUE,	TRUE,	TRUE	},
{	"greet",			TRIG_GREET,			TRIGSLOT_MOVE,		TRUE,	TRUE,	TRUE,	TRUE	},
{	"hit",				TRIG_HIT,			TRIGSLOT_HITS,		TRUE,	FALSE,	FALSE,	TRUE	},
{	"hpcnt",			TRIG_HPCNT,			TRIGSLOT_GENERAL,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"identify",			TRIG_IDENTIFY,			TRIGSLOT_GENERAL,	FALSE,	TRUE,	FALSE,	TRUE	},
{	"interrupt",			TRIG_INTERRUPT,			TRIGSLOT_INTERRUPT,	FALSE,	FALSE,	FALSE,	TRUE	},
{	"kill",				TRIG_KILL,			TRIGSLOT_GENERAL,	TRUE,	TRUE,	TRUE,	TRUE	},
{	"knock",			TRIG_KNOCK,			TRIGSLOT_GENERAL,	FALSE,	FALSE,	TRUE,	FALSE	},
{	"knocking",			TRIG_KNOCKING,			TRIGSLOT_GENERAL,	FALSE,	FALSE,	TRUE,	FALSE	},
{	"land",				TRIG_LAND,			TRIGSLOT_MOVE,		TRUE,	FALSE,	FALSE,	TRUE	},
{	"level",			TRIG_LEVEL,			TRIGSLOT_GENERAL,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"lore",				TRIG_LORE,			TRIGSLOT_GENERAL,	TRUE,	TRUE,	FALSE,	TRUE	},
{	"lorex",			TRIG_LORE_EX,			TRIGSLOT_GENERAL,	TRUE,	TRUE,	FALSE,	TRUE	},
{	"moon",				TRIG_MOON,			TRIGSLOT_GENERAL,	FALSE,	FALSE,	FALSE,	TRUE	},
{	"mount",			TRIG_MOUNT,			TRIGSLOT_GENERAL,	TRUE,	FALSE,	FALSE,	FALSE	},
{	"open",				TRIG_OPEN,			TRIGSLOT_GENERAL,	FALSE,  TRUE,   TRUE,	TRUE	},
{	"preassist",			TRIG_PREASSIST,			TRIGSLOT_FIGHT,		TRUE,	FALSE,	FALSE,	TRUE	},
{	"prebuy",			TRIG_PREBUY,			TRIGSLOT_GENERAL,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"precast",			TRIG_PRECAST,			TRIGSLOT_GENERAL,	FALSE,	FALSE,	FALSE,	FALSE	},
{	"predeath",			TRIG_PREDEATH,			TRIGSLOT_GENERAL,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"predismount",			TRIG_PREDISMOUNT,		TRIGSLOT_GENERAL,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"predrop",			TRIG_PREDROP,			TRIGSLOT_GENERAL,	FALSE,	TRUE,	FALSE,	TRUE	},
{	"preenter",			TRIG_PREENTER,			TRIGSLOT_MOVE,		FALSE,	FALSE,	TRUE,	TRUE	},
{	"preflee",			TRIG_PREFLEE,			TRIGSLOT_GENERAL,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"preget",			TRIG_PREGET,			TRIGSLOT_GENERAL,	FALSE,	TRUE,	FALSE,	TRUE	},
{	"prekill",			TRIG_PREKILL,			TRIGSLOT_GENERAL,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"premount",			TRIG_PREMOUNT,			TRIGSLOT_GENERAL,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"preput",			TRIG_PREPUT,			TRIGSLOT_GENERAL,	FALSE,	TRUE,	FALSE,	TRUE	},
{	"prepractice",			TRIG_PREPRACTICE,		TRIGSLOT_GENERAL,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"prepracticeother",		TRIG_PREPRACTICEOTHER,		TRIGSLOT_GENERAL,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"prepracticethat",		TRIG_PREPRACTICETHAT,		TRIGSLOT_GENERAL,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"prerecall",			TRIG_PRERECALL,			TRIGSLOT_GENERAL,	FALSE,	FALSE,	TRUE,	TRUE	},
{	"prereckoning",			TRIG_PRERECKONING,		TRIGSLOT_RANDOM,	TRUE,	TRUE,	TRUE,	TRUE	},
{	"preremove",			TRIG_PREREMOVE,			TRIGSLOT_GENERAL,	FALSE,	TRUE,	FALSE,	TRUE	},
{	"prerest",			TRIG_PREREST,			TRIGSLOT_GENERAL,	TRUE,	TRUE,	TRUE,	TRUE	},
{	"preround",			TRIG_PREROUND,			TRIGSLOT_FIGHT,		TRUE,	FALSE,	FALSE,	TRUE	},
{	"presell",			TRIG_PRESELL,			TRIGSLOT_GENERAL,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"presit",			TRIG_PRESIT,			TRIGSLOT_GENERAL,	TRUE,	TRUE,	TRUE,	TRUE	},
{	"presleep",			TRIG_PRESLEEP,			TRIGSLOT_GENERAL,	TRUE,	TRUE,	TRUE,	TRUE	},
{	"prespell",			TRIG_PRESPELL,			TRIGSLOT_SPELL,		FALSE,	FALSE,	FALSE,	TRUE	},
{	"prestand",			TRIG_PRESTAND,			TRIGSLOT_GENERAL,	TRUE,	TRUE,	TRUE,	TRUE	},
{	"pretrain",			TRIG_PRETRAIN,			TRIGSLOT_GENERAL,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"prewake",			TRIG_PREWAKE,			TRIGSLOT_GENERAL,	TRUE,	TRUE,	TRUE,	TRUE	},
{	"prewear",			TRIG_PREWEAR,			TRIGSLOT_GENERAL,	FALSE,	TRUE,	FALSE,	TRUE	},
{	"pull",				TRIG_PULL,			TRIGSLOT_GENERAL,	TRUE,	TRUE,	FALSE,	TRUE	},
{	"pullon",			TRIG_PULL_ON,			TRIGSLOT_GENERAL,	TRUE,	TRUE,	FALSE,	TRUE	},
{	"push",				TRIG_PUSH,			TRIGSLOT_GENERAL,	FALSE,	TRUE,	FALSE,	TRUE	},
{	"pushon",			TRIG_PUSH_ON,			TRIGSLOT_GENERAL,	FALSE,	TRUE,	FALSE,	TRUE	},
{	"put",				TRIG_PUT,			TRIGSLOT_GENERAL,	FALSE,	TRUE,	FALSE,	TRUE	},
{	"quit",				TRIG_QUIT,			TRIGSLOT_GENERAL,	FALSE,	FALSE,	FALSE,	TRUE	},
{	"random",			TRIG_RANDOM,			TRIGSLOT_RANDOM,	TRUE,	TRUE,	TRUE,	TRUE	},
{	"recall",			TRIG_RECALL,			TRIGSLOT_GENERAL,	FALSE,	FALSE,	TRUE,	TRUE	},
{	"recite",			TRIG_RECITE,			TRIGSLOT_GENERAL,	FALSE,	TRUE,	FALSE,	TRUE	},
{	"reckoning",			TRIG_RECKONING,			TRIGSLOT_RANDOM,	TRUE,	TRUE,	TRUE,	TRUE	},
{	"regen",			TRIG_REGEN,			TRIGSLOT_RANDOM,	TRUE,	TRUE,	TRUE,	TRUE	},
{	"regen_hp",			TRIG_REGEN_HP,			TRIGSLOT_RANDOM,	TRUE,	TRUE,	TRUE,	TRUE	},
{	"regen_mana",			TRIG_REGEN_MANA,		TRIGSLOT_RANDOM,	TRUE,	TRUE,	TRUE,	TRUE	},
{	"regen_move",			TRIG_REGEN_MOVE,		TRIGSLOT_RANDOM,	TRUE,	TRUE,	TRUE,	TRUE	},
{	"remove",			TRIG_REMOVE,			TRIGSLOT_GENERAL,	FALSE,  TRUE,	FALSE,	TRUE	},
{	"repop",			TRIG_REPOP,			TRIGSLOT_REPOP,		TRUE,	TRUE,	FALSE,	TRUE	},
{	"rest",				TRIG_REST,			TRIGSLOT_GENERAL,	TRUE,	TRUE,	TRUE,	TRUE	},
{	"sayto",			TRIG_SAYTO,			TRIGSLOT_SPEECH,	TRUE,	TRUE,	FALSE,	TRUE	},
{	"sit",				TRIG_SIT,			TRIGSLOT_GENERAL,	TRUE,	TRUE,	TRUE,	TRUE	},
{	"skill_berserk",		TRIG_SKILL_BERSERK,		TRIGSLOT_GENERAL,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"skill_sneak",			TRIG_SKILL_SNEAK,		TRIGSLOT_GENERAL,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"skill_warcry",			TRIG_SKILL_WARCRY,		TRIGSLOT_GENERAL,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"sleep",			TRIG_SLEEP,			TRIGSLOT_GENERAL,	TRUE,	TRUE,	TRUE,	TRUE	},
{	"speech",			TRIG_SPEECH,			TRIGSLOT_SPEECH,	TRUE,	TRUE,	TRUE,	TRUE	},
{	"spell",			TRIG_SPELL,			TRIGSLOT_SPELL,		FALSE,	FALSE,	FALSE,	TRUE	},
{	"spellcast",			TRIG_SPELLCAST,			TRIGSLOT_SPELL,		TRUE,	TRUE,	TRUE,	TRUE	},
{	"spellinter",			TRIG_SPELLINTER,		TRIGSLOT_SPELL,		FALSE,	FALSE,	FALSE,	TRUE	},
{	"spell_cure",			TRIG_SPELL_CURE,		TRIGSLOT_SPELL,		TRUE,	FALSE,	FALSE,	TRUE	},
{	"spell_dispel",			TRIG_SPELL_DISPEL,		TRIGSLOT_SPELL,		TRUE,	FALSE,	FALSE,	TRUE	},
{	"stand",			TRIG_STAND,			TRIGSLOT_GENERAL,	TRUE,	TRUE,	TRUE,	TRUE	},
{	"startcombat",			TRIG_START_COMBAT,		TRIGSLOT_FIGHT,		TRUE,	TRUE,	TRUE,	TRUE	},
{	"stripaffect",			TRIG_STRIPAFFECT,		TRIGSLOT_REPOP,		FALSE,	FALSE,	FALSE,	TRUE	},
{	"takeoff",			TRIG_TAKEOFF,			TRIGSLOT_MOVE,		TRUE,	FALSE,	FALSE,	TRUE	},
{	"throw",			TRIG_THROW,			TRIGSLOT_GENERAL,	TRUE,	TRUE,	TRUE,	TRUE	},
{	"touch",			TRIG_TOUCH,			TRIGSLOT_GENERAL,	FALSE,	TRUE,	FALSE,	TRUE	},
{	"turn",				TRIG_TURN,			TRIGSLOT_GENERAL,	FALSE,	TRUE,	FALSE,	TRUE	},
{	"turnon",			TRIG_TURN_ON,			TRIGSLOT_GENERAL,	FALSE,	TRUE,	FALSE,	TRUE	},
{	"use",				TRIG_USE,			TRIGSLOT_GENERAL,	FALSE,	TRUE,	FALSE,	TRUE	},
{	"usewith",			TRIG_USEWITH,			TRIGSLOT_GENERAL,	FALSE,	TRUE,	FALSE,	TRUE	},
{	"verb",				TRIG_VERB,			TRIGSLOT_VERB,		TRUE,	TRUE,	TRUE,	TRUE	},
{	"verbself",			TRIG_VERBSELF,			TRIGSLOT_VERB,		FALSE,	FALSE,	FALSE,	TRUE	},
{	"wake",				TRIG_WAKE,			TRIGSLOT_GENERAL,	TRUE,	TRUE,	TRUE,	TRUE	},
{	"weapon_blocked",		TRIG_WEAPON_BLOCKED,		TRIGSLOT_ATTACKS,	FALSE,	TRUE,	FALSE,	TRUE	},
{	"weapon_caught",		TRIG_WEAPON_CAUGHT,		TRIGSLOT_ATTACKS,	FALSE,	TRUE,	FALSE,	TRUE	},
{	"weapon_parried",		TRIG_WEAPON_PARRIED,		TRIGSLOT_ATTACKS,	FALSE,	TRUE,	FALSE,	TRUE	},
{	"wear",				TRIG_WEAR,			TRIGSLOT_GENERAL,	TRUE,	TRUE,	TRUE,	TRUE	},
{	"whisper",			TRIG_WHISPER,			TRIGSLOT_SPEECH,	TRUE,	FALSE,	FALSE,	TRUE	},
{	"xpgain",			TRIG_XPGAIN,			TRIGSLOT_GENERAL,	TRUE,	TRUE,	TRUE,	TRUE	},
{	"zap",				TRIG_ZAP,			TRIGSLOT_GENERAL,	FALSE,	TRUE,	FALSE,	TRUE	},
{	NULL,				-1,				TRIGSLOT_GENERAL,	FALSE,	FALSE,	FALSE,	FALSE	}
};
int trigger_table_size = elementsof(trigger_table);

int trigger_slots[] = {
	TRIGSLOT_ACTION,		// TRIG_ACT
	TRIGSLOT_REPOP,			// TRIG_AFTERDEATH
	TRIGSLOT_FIGHT,			// TRIG_AFTERKILL
	TRIGSLOT_FIGHT,			// TRIG_ASSIST
	TRIGSLOT_ATTACKS,		// TRIG_ATTACK_BACKSTAB
	TRIGSLOT_ATTACKS,		// TRIG_ATTACK_BASH
	TRIGSLOT_ATTACKS,		// TRIG_ATTACK_BEHEAD
	TRIGSLOT_ATTACKS,		// TRIG_ATTACK_BITE
	TRIGSLOT_ATTACKS,		// TRIG_ATTACK_BLACKJACK
	TRIGSLOT_ATTACKS,		// TRIG_ATTACK_CIRCLE
	TRIGSLOT_ATTACKS,		// TRIG_ATTACK_COUNTER
	TRIGSLOT_ATTACKS,		// TRIG_ATTACK_CRIPPLE
	TRIGSLOT_ATTACKS,		// TRIG_ATTACK_DIRTKICK
	TRIGSLOT_ATTACKS,		// TRIG_ATTACK_DISARM
	TRIGSLOT_ATTACKS,		// TRIG_ATTACK_INTIMIDATE
	TRIGSLOT_ATTACKS,		// TRIG_ATTACK_KICK
	TRIGSLOT_ATTACKS,		// TRIG_ATTACK_REND
	TRIGSLOT_ATTACKS,		// TRIG_ATTACK_SLIT
	TRIGSLOT_ATTACKS,		// TRIG_ATTACK_SMITE
	TRIGSLOT_ATTACKS,		// TRIG_ATTACK_TAILKICK
	TRIGSLOT_ATTACKS,		// TRIG_ATTACK_TRAMPLE
	TRIGSLOT_ATTACKS,		// TRIG_ATTACK_TURN
	TRIGSLOT_ATTACKS,		// TRIG_BARRIER
	TRIGSLOT_GENERAL,		// TRIG_BLOW
	TRIGSLOT_GENERAL,		// TRIG_BOARD
	TRIGSLOT_GENERAL,		// TRIG_BRANDISH
//	TRIG_BREATH,		// Used to indicate a BREATH attack, maybe a custom breath
	TRIGSLOT_GENERAL,		// TRIG_BRIBE
	TRIGSLOT_GENERAL,		// TRIG_BURY
	TRIGSLOT_INTERRUPT,		// TRIG_CANINTERRUPT
	TRIGSLOT_GENERAL,		// TRIG_CAST
	TRIGSLOT_GENERAL,		// TRIG_CATALYST
	TRIGSLOT_GENERAL,		// TRIG_CATALYST_FULL
	TRIGSLOT_GENERAL,		// TRIG_CATALYST_SOURCE
	TRIGSLOT_DAMAGE,		// TRIG_CHECK_DAMAGE
	TRIGSLOT_GENERAL,		// TRIG_CLOSE
//	TRIG_CONCEAL,
	TRIGSLOT_DAMAGE,		// TRIG_DAMAGE
	TRIGSLOT_REPOP,			// TRIG_DEATH
	TRIGSLOT_GENERAL,		// TRIG_DELAY
//	TRIG_DRAG,
//	TRIG_DRAGGED,
	TRIGSLOT_GENERAL,		// TRIG_DRINK
	TRIGSLOT_GENERAL,		// TRIG_DROP
	TRIGSLOT_GENERAL,		// TRIG_EAT
	TRIGSLOT_MOVE,			// TRIG_ENTRY
	TRIGSLOT_MOVE,			// TRIG_EXALL
	TRIGSLOT_GENERAL,		// TRIG_EXAMINE
	TRIGSLOT_MOVE,			// TRIG_EXIT
	TRIGSLOT_REPOP,			// TRIG_EXPIRE
	TRIGSLOT_REPOP,			// TRIG_EXTRACT
	TRIGSLOT_FIGHT,			// TRIG_FIGHT
//	TRIG_FILL,
	TRIGSLOT_GENERAL,		// TRIG_FLEE
	TRIGSLOT_GENERAL,		// TRIG_FORCEDISMOUNT
	TRIGSLOT_GENERAL,		// TRIG_GET
	TRIGSLOT_GENERAL,		// TRIG_GIVE
	TRIGSLOT_MOVE,			// TRIG_GRALL
	TRIGSLOT_MOVE,			// TRIG_GREET
	TRIGSLOT_HITS,			// TRIG_HIT
	TRIGSLOT_GENERAL,		// TRIG_HPCNT
	TRIGSLOT_GENERAL,		// TRIG_IDENTIFY
	TRIGSLOT_INTERRUPT,		// TRIG_INTERRUPT
	TRIGSLOT_GENERAL,		// TRIG_KILL
	TRIGSLOT_GENERAL,		// TRIG_KNOCK
	TRIGSLOT_GENERAL,		// TRIG_KNOCKING
	TRIGSLOT_MOVE,			// TRIG_LAND
	TRIGSLOT_GENERAL,		// TRIG_LEVEL
	TRIGSLOT_GENERAL,		// TRIG_LORE
	TRIGSLOT_GENERAL,		// TRIG_LORE_EX
	TRIGSLOT_GENERAL,		// TRIG_MOON
	TRIGSLOT_GENERAL,		// TRIG_MOUNT
	TRIGSLOT_GENERAL,		// TRIG_OPEN
//	TRIG_PICKLOCK,
//	TRIG_POUR,
	TRIGSLOT_FIGHT,			// TRIG_PREASSIST
//	TRIG_PREBOARD,
	TRIGSLOT_GENERAL,		// TRIG_PREBUY
	TRIGSLOT_GENERAL,		// TRIG_PRECAST
//	TRIG_PRECLOSE,
//	TRIG_PRECONCEAL,
	TRIGSLOT_GENERAL,		// TRIG_PREDEATH
	TRIGSLOT_GENERAL,		// TRIG_PREDISMOUNT
//	TRIG_PREDRAG,
	TRIGSLOT_GENERAL,		// TRIG_PREDROP
//	TRIG_PREENCHANT,
	TRIGSLOT_MOVE,			// TRIG_PREENTER
//	TRIG_PREFILL,
	TRIGSLOT_GENERAL,		// TRIG_PREFLEE
	TRIGSLOT_GENERAL,		// TRIG_PREGET
	TRIGSLOT_GENERAL,		// TRIG_PREKILL
//	TRIG_PRELOCK,
	TRIGSLOT_GENERAL,		// TRIG_PREMOUNT
//	TRIG_PREOPEN,
//	TRIG_PREPICKLOCK,
//	TRIG_PREPOUR,
	TRIGSLOT_GENERAL,		// TRIG_PREPRACTICE
	TRIGSLOT_GENERAL,		// TRIG_PREPRACTICEOTHER
	TRIGSLOT_GENERAL,		// TRIG_PREPRACTICETHAT
	TRIGSLOT_GENERAL,		// TRIG_PREPUT
	TRIGSLOT_GENERAL,		// TRIG_PRERECALL
	TRIGSLOT_RANDOM,		// TRIG_PRERECKONING
	TRIGSLOT_GENERAL,		// TRIG_PREREMOVE
	TRIGSLOT_GENERAL,		// TRIG_PREREST
	TRIGSLOT_FIGHT,			// TRIG_PREROUND
	TRIGSLOT_GENERAL,		// TRIG_PRESELL
	TRIGSLOT_GENERAL,		// TRIG_PRESIT
	TRIGSLOT_GENERAL,		// TRIG_PRESLEEP
	TRIGSLOT_SPELL,			// TRIG_PRESPELL
	TRIGSLOT_GENERAL,		// TRIG_PRESTAND
	TRIGSLOT_GENERAL,		// TRIG_PRETRAIN
//	TRIG_PREUNLOCK,
	TRIGSLOT_GENERAL,		// TRIG_PREWAKE
	TRIGSLOT_GENERAL,		// TRIG_PREWEAR
	TRIGSLOT_GENERAL,		// TRIG_PULL
	TRIGSLOT_GENERAL,		// TRIG_PULL_ON
	TRIGSLOT_GENERAL,		// TRIG_PUSH
	TRIGSLOT_GENERAL,		// TRIG_PUSH_ON
	TRIGSLOT_GENERAL,		// TRIG_PUT
	TRIGSLOT_GENERAL,		// TRIG_QUIT
	TRIGSLOT_RANDOM,		// TRIG_RANDOM
	TRIGSLOT_GENERAL,		// TRIG_RECALL
	TRIGSLOT_GENERAL,		// TRIG_RECITE
	TRIGSLOT_RANDOM,		// TRIG_RECKONING
	TRIGSLOT_RANDOM,		// TRIG_REGEN
	TRIGSLOT_RANDOM,		// TRIG_REGEN_HP
	TRIGSLOT_RANDOM,		// TRIG_REGEN_MANA
	TRIGSLOT_RANDOM,		// TRIG_REGEN_MOVE
	TRIGSLOT_GENERAL,		// TRIG_REMOVE
	TRIGSLOT_REPOP,			// TRIG_REPOP
	TRIGSLOT_GENERAL,		// TRIG_REST
	TRIGSLOT_SPEECH,		// TRIG_SAYTO
	TRIGSLOT_GENERAL,		// TRIG_SIT
	TRIGSLOT_GENERAL,		// TRIG_SKILL_BERSERK
	TRIGSLOT_GENERAL,		// TRIG_SKILL_SNEAK
	TRIGSLOT_GENERAL,		// TRIG_SKILL_WARCRY
	TRIGSLOT_GENERAL,		// TRIG_SLEEP
	TRIGSLOT_SPEECH,		// TRIG_SPEECH
	TRIGSLOT_SPELL,			// TRIG_SPELL
	TRIGSLOT_SPELL,			// TRIG_SPELLCAST
	TRIGSLOT_SPELL,			// TRIG_SPELLINTER
	TRIGSLOT_SPELL,			// TRIG_SPELL_CURE
	TRIGSLOT_SPELL,			// TRIG_SPELL_DISPEL
	TRIGSLOT_GENERAL,		// TRIG_STAND
	TRIGSLOT_FIGHT,			// TRIG_START_COMBAT
//	TRIG_STRIKE,
	TRIGSLOT_REPOP,			// TRIG_STRIPAFFECT
	TRIGSLOT_MOVE,			// TRIG_TAKEOFF
	TRIGSLOT_GENERAL,		// TRIG_THROW
	TRIGSLOT_GENERAL,		// TRIG_TOUCH
	TRIGSLOT_GENERAL,		// TRIG_TURN
	TRIGSLOT_GENERAL,		// TRIG_TURN_ON
	TRIGSLOT_GENERAL,		// TRIG_USE
	TRIGSLOT_GENERAL,		// TRIG_USEWITH
	TRIGSLOT_VERB,			// TRIG_VERB
	TRIGSLOT_VERB,			// TRIG_VERBSELF
	TRIGSLOT_GENERAL,		// TRIG_WAKE
	TRIGSLOT_ATTACKS,		// TRIG_WEAPON_BLOCKED
	TRIGSLOT_ATTACKS,		// TRIG_WEAPON_CAUGHT
	TRIGSLOT_ATTACKS,		// TRIG_WEAPON_PARRIED
	TRIGSLOT_GENERAL,		// TRIG_WEAR
	TRIGSLOT_SPEECH,		// TRIG_WHISPER
	TRIGSLOT_GENERAL,		// TRIG_XPGAIN
	TRIGSLOT_GENERAL,		// TRIG_ZAP
};
int trigger_slots_size = elementsof(trigger_slots);

IFCHECK_DATA ifcheck_table[] = {
// name			prog type	params	return	function		help reference
{ "abs",		IFC_ANY,	"N",	FALSE,	ifc_abs,		"ifcheck abs" },
{ "act",		IFC_ANY,	"ES",	FALSE,	ifc_act,		"ifcheck act" },
{ "act2",		IFC_ANY,	"ES",	FALSE,	ifc_act2,		"ifcheck act2" },
{ "affected",		IFC_ANY,	"ES",	FALSE,	ifc_affected,		"ifcheck affected" },
{ "affected2",		IFC_ANY,	"ES",	FALSE,	ifc_affected2,		"ifcheck affected2" },
{ "affectedname",	IFC_ANY,	"ES",	FALSE,	ifc_affectedname,	"ifcheck affectedname" },
{ "affectedspell",	IFC_ANY,	"ES",	FALSE,	ifc_affectedspell,	"ifcheck affectedspell" },
{ "age",		IFC_ANY,	"E",	TRUE,	ifc_age,		"ifcheck age" },
{ "align",		IFC_ANY,	"E",	TRUE,	ifc_align,		"ifcheck align" },
{ "angle",		IFC_ANY,	"E",	TRUE,	ifc_angle,		"ifcheck angle" },
{ "areahasland",	IFC_ANY,	"",	FALSE,	ifc_areahasland,	"ifcheck areahasland" },
{ "areaid",		IFC_ANY,	"",	TRUE,	ifc_areaid,		"ifcheck areaid" },
{ "arealandx",		IFC_ANY,	"",	TRUE,	ifc_arealandx,		"ifcheck arealandx" },
{ "arealandy",		IFC_ANY,	"",	TRUE,	ifc_arealandy,		"ifcheck arealandy" },
{ "arenafights",	IFC_ANY,	"E",	TRUE,	ifc_arenafights,	"ifcheck arenafights" },
{ "arenaloss",		IFC_ANY,	"E",	TRUE,	ifc_arenaloss,		"ifcheck arenaloss" },
{ "arenaratio",		IFC_ANY,	"E",	TRUE,	ifc_arenaratio,		"ifcheck arenaratio" },
{ "arenawins",		IFC_ANY,	"E",	TRUE,	ifc_arenawins,		"ifcheck arenawins" },
{ "areax",		IFC_ANY,	"",	TRUE,	ifc_areax,		"ifcheck areax" },
{ "areay",		IFC_ANY,	"",	TRUE,	ifc_areay,		"ifcheck areay" },
{ "bankbalance",	IFC_ANY,	"",	TRUE,	ifc_bankbalance,	"ifcheck bit" },
{ "bit",		IFC_ANY,	"",	TRUE,	ifc_bit,		"ifcheck bit" },
{ "canhunt",		IFC_ANY,	"E",	FALSE,	ifc_canhunt,		"ifcheck canhunt" },
{ "canpractice",	IFC_ANY,	"ES",	FALSE,	ifc_canpractice,	"ifcheck canpractice" },
{ "canscare",		IFC_ANY,	"E",	FALSE,	ifc_canscare,		"ifcheck canscare" },
{ "carriedby",		IFC_O,		"E",	FALSE,	ifc_carriedby,		"ifcheck carriedby" },
{ "carries",		IFC_ANY,	"ES",	FALSE,	ifc_carries,		"ifcheck carries" },
{ "carryleft",		IFC_ANY,	"E",	TRUE,	ifc_carryleft,		"ifcheck carryleft" },
{ "church",		IFC_ANY,	"ES",	FALSE,	ifc_church,		"ifcheck church" },
{ "churchonline",	IFC_ANY,	"E",	TRUE,	ifc_churchonline,	"ifcheck churchonline" },
{ "churchrank",		IFC_ANY,	"E",	TRUE,	ifc_churchrank,		"ifcheck churchrank" },
{ "clan",		IFC_NONE,	"ES",	FALSE,	ifc_clan,		"ifcheck clan" },
{ "class",		IFC_NONE,	"ES",	FALSE,	ifc_class,		"ifcheck class" },
{ "clones",		IFC_M,		"E",	TRUE,	ifc_clones,		"ifcheck clones" },
{ "container",		IFC_ANY,	"ES",	FALSE,	ifc_container,		"ifcheck container" },
{ "cos",		IFC_ANY,	"N",	TRUE,	ifc_cos,		"ifcheck cos" },
{ "cpkfights",		IFC_ANY,	"E",	TRUE,	ifc_cpkfights,		"ifcheck cpkfights" },
{ "cpkloss",		IFC_ANY,	"E",	TRUE,	ifc_cpkloss,		"ifcheck cpkloss" },
{ "cpkratio",		IFC_ANY,	"E",	TRUE,	ifc_cpkratio,		"ifcheck cpkratio" },
{ "cpkwins",		IFC_ANY,	"E",	TRUE,	ifc_cpkwins,		"ifcheck cpkwins" },
{ "curhit",		IFC_ANY,	"E",	TRUE,	ifc_curhit,		"ifcheck curhit" },
{ "curmana",		IFC_ANY,	"E",	TRUE,	ifc_curmana,		"ifcheck curmana" },
{ "curmove",		IFC_ANY,	"E",	TRUE,	ifc_curmove,		"ifcheck curmove" },
{ "damtype",		IFC_ANY,	"ES",	FALSE,	ifc_damtype,		"ifcheck damtype" },
{ "danger",		IFC_ANY,	"E",	TRUE,	ifc_danger,		"ifcheck danger" },
{ "day",		IFC_ANY,	"",	TRUE,	ifc_day,		"ifcheck day" },
{ "death",		IFC_ANY,	"ES",	FALSE,	ifc_death,		"ifcheck death" },
{ "deathcount",		IFC_ANY,	"E",	TRUE,	ifc_deathcount,		"ifcheck deathcount" },
{ "deitypoint",		IFC_ANY,	"E",	TRUE,	ifc_deity,		"ifcheck deitypoint" },
{ "dice",		IFC_ANY,	"NNn",	TRUE,	ifc_dice,		"ifcheck dice" },
{ "drunk",		IFC_ANY,	"E",	TRUE,	ifc_drunk,		"ifcheck drunk" },
{ "exists",		IFC_NONE,	"S",	FALSE,	ifc_exists,		"ifcheck exists" },
{ "exitexists",		IFC_ANY,	"eS",	FALSE,	ifc_exitexists,		"ifcheck exitexists" },
{ "exitflag",		IFC_ANY,	"ESS",	FALSE,	ifc_exitflag,		"ifcheck exitflag" },
{ "findpath",		IFC_ANY,	"",	TRUE,	ifc_findpath,		"ifcheck findpath" },
{ "flagact",		IFC_ANY,	"",	TRUE,	ifc_flag_act,		"ifcheck flagact" },
{ "flagact2",		IFC_ANY,	"",	TRUE,	ifc_flag_act2,		"ifcheck flagact2" },
{ "flagaffect",		IFC_ANY,	"",	TRUE,	ifc_flag_affect,	"ifcheck flagaffect" },
{ "flagaffect2",	IFC_ANY,	"",	TRUE,	ifc_flag_affect2,	"ifcheck flagaffect2" },
{ "flagcontainer",	IFC_ANY,	"",	TRUE,	ifc_flag_container,	"ifcheck flagcontainer" },
{ "flagexit",		IFC_ANY,	"",	TRUE,	ifc_flag_exit,		"ifcheck flagexit" },
{ "flagextra",		IFC_ANY,	"",	TRUE,	ifc_flag_extra,		"ifcheck flagextra" },
{ "flagextra2",		IFC_ANY,	"",	TRUE,	ifc_flag_extra2,	"ifcheck flagextra2" },
{ "flagextra3",		IFC_ANY,	"",	TRUE,	ifc_flag_extra3,	"ifcheck flagextra3" },
{ "flagextra4",		IFC_ANY,	"",	TRUE,	ifc_flag_extra4,	"ifcheck flagextra4" },
{ "flagform",		IFC_ANY,	"",	TRUE,	ifc_flag_form,		"ifcheck flagform" },
{ "flagfurniture",	IFC_ANY,	"",	TRUE,	ifc_flag_furniture,	"ifcheck flagfurniture" },
{ "flagimm",		IFC_ANY,	"",	TRUE,	ifc_flag_imm,		"ifcheck flagimm" },
{ "flaginterrupt",	IFC_ANY,	"",	TRUE,	ifc_flag_interrupt,	"ifcheck flaginterrupt" },
{ "flagoff",		IFC_ANY,	"",	TRUE,	ifc_flag_off,		"ifcheck flagoff" },
{ "flagpart",		IFC_ANY,	"",	TRUE,	ifc_flag_part,		"ifcheck flagpart" },
{ "flagportal",		IFC_ANY,	"",	TRUE,	ifc_flag_portal,	"ifcheck flagportal" },
{ "flagres",		IFC_ANY,	"",	TRUE,	ifc_flag_res,		"ifcheck flagres" },
{ "flagroom",		IFC_ANY,	"",	TRUE,	ifc_flag_room,		"ifcheck flagroom" },
{ "flagroom2",		IFC_ANY,	"",	TRUE,	ifc_flag_room2,		"ifcheck flagroom2" },
{ "flagvuln",		IFC_ANY,	"",	TRUE,	ifc_flag_vuln,		"ifcheck flagvuln" },
{ "flagweapon",		IFC_ANY,	"",	TRUE,	ifc_flag_weapon,	"ifcheck flagweapon" },
{ "flagwear",		IFC_ANY,	"",	TRUE,	ifc_flag_wear,		"ifcheck flagwear" },
{ "fullness",		IFC_ANY,	"E",	TRUE,	ifc_fullness,		"ifcheck fullness" },
{ "furniture",		IFC_ANY,	"ES",	FALSE,	ifc_furniture,		"ifcheck furniture" },
{ "gold",		IFC_ANY,	"E",	TRUE,	ifc_gold,		"ifcheck gold" },
{ "groundweight",	IFC_ANY,	"E",	TRUE,	ifc_groundweight,	"ifcheck groundweight" },
{ "groupcon",		IFC_ANY,	"E",	TRUE,	ifc_groupcon,		"ifcheck groupcon" },
{ "groupdex",		IFC_ANY,	"E",	TRUE,	ifc_groupdex,		"ifcheck groupdex" },
{ "grouphit",		IFC_ANY,	"E",	TRUE,	ifc_grouphit,		"ifcheck groupwis" },
{ "groupint",		IFC_ANY,	"E",	TRUE,	ifc_groupint,		"ifcheck groupint" },
{ "groupmana",		IFC_ANY,	"E",	TRUE,	ifc_groupmana,		"ifcheck groupwis" },
{ "groupmaxhit",	IFC_ANY,	"E",	TRUE,	ifc_groupmaxhit,	"ifcheck groupwis" },
{ "groupmaxmana",	IFC_ANY,	"E",	TRUE,	ifc_groupmaxmana,	"ifcheck groupwis" },
{ "groupmaxmove",	IFC_ANY,	"E",	TRUE,	ifc_groupmaxmove,	"ifcheck groupwis" },
{ "groupmove",		IFC_ANY,	"E",	TRUE,	ifc_groupmove,		"ifcheck groupwis" },
{ "groupstr",		IFC_ANY,	"E",	TRUE,	ifc_groupstr,		"ifcheck groupstr" },
{ "groupwis",		IFC_ANY,	"E",	TRUE,	ifc_groupwis,		"ifcheck groupwis" },
{ "grpsize",		IFC_ANY,	"E",	TRUE,	ifc_grpsize,		"ifcheck grpsize" },
{ "handsfull",		IFC_ANY,	"E",	FALSE,	ifc_handsfull,		"ifcheck handsfull" },
{ "has",		IFC_ANY,	"ES",	FALSE,	ifc_has,		"ifcheck has" },
{ "hascatalyst",	IFC_ANY,	"ES",	TRUE,	ifc_hascatalyst,	"ifcheck hascatalyst" },
{ "hasenviroment",	IFC_ANY,	"ES",	TRUE,	ifc_hasenvironment,	"ifcheck hasenvironment" },
{ "hasprompt",		IFC_ANY,	"E",	FALSE,	ifc_hasprompt,		"ifcheck hasprompt" },
{ "hasqueue",		IFC_ANY,	"E",	FALSE,	ifc_hasqueue,		"ifcheck hasqueue" },
{ "hasship",		IFC_NONE,	"E",	FALSE,	ifc_hasship,		"ifcheck hasship" },
{ "hassubclass",	IFC_ANY,	"ES",	FALSE,	ifc_hassubclass,	"ifcheck hassubclass" },
{ "hastarget",		IFC_ANY,	"E",	FALSE,	ifc_hastarget,		"ifcheck hastarget" },
{ "hastoken",		IFC_ANY,	"EN",	FALSE,	ifc_hastoken,		"ifcheck hastoken" },
{ "hasvlink",		IFC_ANY,	"",	TRUE,	ifc_hasvlink,		"ifcheck hasvlink" },
{ "healregen",		IFC_ANY,	"E",	TRUE,	ifc_healregen,		"ifcheck healregen" },
{ "hitdamage",		IFC_ANY,	"E",	TRUE,	ifc_hitdamage,		"ifcheck hitdamage" },
{ "hitdamclass",	IFC_ANY,	"E",	FALSE,	ifc_hitdamclass,	"ifcheck hitdamclass" },
{ "hitdamtype",		IFC_ANY,	"E",	FALSE,	ifc_hitdamtype,		"ifcheck hitdamtype" },
{ "hitskilltype",	IFC_ANY,	"E",	FALSE,	ifc_hitskilltype,	"ifcheck hitskilltype" },
{ "hour",		IFC_ANY,	"E",	TRUE,	ifc_hour,		"ifcheck hour" },
{ "hpcnt",		IFC_ANY,	"E",	TRUE,	ifc_hpcnt,		"ifcheck hpcnt" },
{ "hunger",		IFC_ANY,	"E",	TRUE,	ifc_hunger,		"ifcheck hunger" },
{ "id",			IFC_ANY,	"E",	TRUE,	ifc_id,			"ifcheck id" },
{ "id2",		IFC_ANY,	"E",	TRUE,	ifc_id2,		"ifcheck id2" },
{ "identical",		IFC_ANY,	"EE",	FALSE,	ifc_identical,		"ifcheck identical" },
{ "imm",		IFC_ANY,	"ES",	FALSE,	ifc_imm,		"ifcheck imm" },
{ "innature",		IFC_ANY,	"E",	FALSE,	ifc_innature,		"ifcheck innature" },
{ "inputwait",		IFC_ANY,	"E",	FALSE,	ifc_inputwait,		"ifcheck inputwait" },
{ "inwilds",		IFC_ANY,	"",	FALSE,	ifc_inwilds,		"ifcheck inwilds" },
{ "isactive",		IFC_ANY,	"E",	FALSE,	ifc_isactive,		"ifcheck isactive" },
{ "isambushing",	IFC_ANY,	"E",	FALSE,	ifc_isambushing,	"ifcheck isambushing" },
{ "isangel",		IFC_ANY,	"E",	FALSE,	ifc_isangel,		"ifcheck isangel" },
{ "isbrewing",		IFC_ANY,	"Es",	FALSE,	ifc_isbrewing,		"ifcheck isbrewing" },
{ "iscasting",		IFC_ANY,	"Es",	FALSE,	ifc_iscasting,		"ifcheck iscasting" },
{ "ischarm",		IFC_ANY,	"E",	FALSE,	ifc_ischarm,		"ifcheck ischarm" },
{ "ischurchexcom",	IFC_ANY,	"E",	FALSE,	ifc_ischurchexcom,	"ifcheck ischurchexcom" },
{ "iscloneroom",	IFC_ANY,	"E",	FALSE,	ifc_iscloneroom,	"ifcheck iscloneroom" },
{ "iscpkproof",		IFC_ANY,	"E",	FALSE,	ifc_iscpkproof,		"ifcheck iscpkproof" },
{ "isdead",		IFC_ANY,	"E",	FALSE,	ifc_isdead,		"ifcheck dead" },
{ "isdelay",		IFC_ANY,	"E",	FALSE,	ifc_isdelay,		"ifcheck isdelay" },
{ "isdemon",		IFC_ANY,	"E",	FALSE,	ifc_isdemon,		"ifcheck isdemon" },
{ "isevil",		IFC_ANY,	"E",	FALSE,	ifc_isevil,		"ifcheck isevil" },
{ "isfading",		IFC_ANY,	"Es",	FALSE,	ifc_isfading,		"ifcheck isfading" },
{ "isfighting",		IFC_ANY,	"Ee",	FALSE,	ifc_isfighting,		"ifcheck isfighting" },
{ "isflying",		IFC_ANY,	"E",	FALSE,	ifc_isflying,		"ifcheck isflying" },
{ "isfollow",		IFC_ANY,	"E",	FALSE,	ifc_isfollow,		"ifcheck isfollow" },
{ "isgood",		IFC_ANY,	"E",	FALSE,	ifc_isgood,		"ifcheck isgood" },
{ "ishunting",		IFC_ANY,	"E",	FALSE,	ifc_ishunting,		"ifcheck ishunting" },
{ "isimmort",		IFC_ANY,	"E",	FALSE,	ifc_isimmort,		"ifcheck isimmort" },
{ "iskey",		IFC_ANY,	"E",	FALSE,	ifc_iskey,		"ifcheck iskey" },
{ "isleader",		IFC_ANY,	"E",	FALSE,	ifc_isleader,		"ifcheck isleader" },
{ "ismoonup",		IFC_ANY,	"",	FALSE,	ifc_ismoonup,		"ifcheck ismoonup" },
{ "ismorphed",		IFC_ANY,	"E",	FALSE,	ifc_ismorphed,		"ifcheck ismorphed" },
{ "ismystic",		IFC_ANY,	"E",	FALSE,	ifc_ismystic,		"ifcheck ismystic" },
{ "isneutral",		IFC_ANY,	"E",	FALSE,	ifc_isneutral,		"ifcheck isneutral" },
{ "isnpc",		IFC_ANY,	"E",	FALSE,	ifc_isnpc,		"ifcheck isnpc" },
{ "ison",		IFC_ANY,	"E",	FALSE,	ifc_ison,		"ifcheck ison" },
{ "ispc",		IFC_ANY,	"E",	FALSE,	ifc_ispc,		"ifcheck ispc" },
{ "isprey",		IFC_ANY,	"E",	FALSE,	ifc_isprey,		"ifcheck isprey" },
{ "ispulling",		IFC_ANY,	"Ee",	FALSE,	ifc_ispulling,		"ifcheck ispulling" },
{ "ispullingrelic",	IFC_ANY,	"Es",	FALSE,	ifc_ispullingrelic,	"ifcheck ispullingrelic" },
{ "isquesting",		IFC_ANY,	"E",	FALSE,	ifc_isquesting,		"ifcheck isquesting" },
{ "isremort",		IFC_ANY,	"E",	FALSE,	ifc_isremort,		"ifcheck isremort" },
{ "isrepairable",	IFC_ANY,	"E",	FALSE,	ifc_isrepairable,	"ifcheck isrepairable" },
{ "isrestrung",		IFC_ANY,	"E",	FALSE,	ifc_isrestrung,		"ifcheck isrestrung" },
{ "isridden",		IFC_ANY,	"E",	FALSE,	ifc_isridden,		"ifcheck isridden" },
{ "isrider",		IFC_ANY,	"E",	FALSE,	ifc_isrider,		"ifcheck isrider" },
{ "isriding",		IFC_ANY,	"E",	FALSE,	ifc_isriding,		"ifcheck isriding" },
{ "isroomdark",		IFC_ANY,	"E",	FALSE,	ifc_isroomdark,		"ifcheck isroomdark" },
{ "issafe",		IFC_ANY,	"Es",	FALSE,	ifc_issafe,		"ifcheck issafe" },
{ "isscribing",		IFC_ANY,	"Es",	FALSE,	ifc_isscribing,		"ifcheck isscribing" },
{ "isshifted",		IFC_ANY,	"E",	FALSE,	ifc_isshifted,		"ifcheck isshifted" },
{ "isshooting",		IFC_ANY,	"Es",	FALSE,	ifc_isshooting,		"ifcheck isshooting" },
{ "isshopkeeper",	IFC_ANY,	"E",	FALSE,	ifc_isshopkeeper,	"ifcheck isshopkeeper" },
{ "isspell",		IFC_ANY,	"E",	FALSE,	ifc_isspell,		"ifcheck isspell" },
{ "issubclass",		IFC_ANY,	"ES",	FALSE,	ifc_issubclass,		"ifcheck issubclass" },
{ "issustained",	IFC_ANY,	"E",	FALSE,	ifc_issustained,	"ifcheck issustained" },
{ "istarget",		IFC_ANY,	"E",	FALSE,	ifc_istarget,		"ifcheck istarget" },
{ "istattooing",	IFC_ANY,	"E",	FALSE,	ifc_istattooing,	"ifcheck istattooing" },
{ "isvisible",		IFC_ANY,	"E",	FALSE,	ifc_isvisible,		"ifcheck isvisible" },
{ "isvisibleto",	IFC_ANY,	"E",	FALSE,	ifc_isvisibleto,	"ifcheck isvisibleto" },
{ "isworn",		IFC_ANY,	"E",	FALSE,	ifc_isworn,		"ifcheck isworn" },
{ "lastreturn",		IFC_ANY,	"",	TRUE,	ifc_lastreturn,		"ifcheck lastreturn" },
{ "level",		IFC_ANY,	"E",	TRUE,	ifc_level,		"ifcheck level" },
{ "liquid",		IFC_ANY,	"ES",	FALSE,	ifc_liquid,		"ifcheck liquid" },
{ "lostparts",		IFC_ANY,	"ES",	FALSE,	ifc_lostparts,		"ifcheck lostparts" },
{ "manaregen",		IFC_ANY,	"E",	TRUE,	ifc_manaregen,		"ifcheck manaregen" },
{ "manastore",		IFC_ANY,	"E",	TRUE,	ifc_manastore,		"ifcheck manastore" },
{ "maparea",		IFC_ANY,	"",	TRUE,	ifc_maparea,		"ifcheck maparea" },
{ "mapheight",		IFC_ANY,	"",	TRUE,	ifc_mapheight,		"ifcheck mapheight" },
{ "mapid",		IFC_ANY,	"",	TRUE,	ifc_mapid,		"ifcheck mapid" },
{ "mapvalid",		IFC_ANY,	"",	FALSE,	ifc_mapvalid,		"ifcheck mapvalid" },
{ "mapwidth",		IFC_ANY,	"",	TRUE,	ifc_mapwidth,		"ifcheck mapwidth" },
{ "mapx",		IFC_ANY,	"",	TRUE,	ifc_mapx,		"ifcheck mapx" },
{ "mapy",		IFC_ANY,	"",	TRUE,	ifc_mapy,		"ifcheck mapy" },
{ "material",		IFC_ANY,	"ES",	FALSE,	ifc_material,		"ifcheck material" },
{ "maxcarry",		IFC_ANY,	"E",	TRUE,	ifc_maxcarry,		"ifcheck maxcarry" },
{ "maxhit",		IFC_ANY,	"E",	TRUE,	ifc_maxhit,		"ifcheck maxhit" },
{ "maxmana",		IFC_ANY,	"E",	TRUE,	ifc_maxmana,		"ifcheck maxmana" },
{ "maxmove",		IFC_ANY,	"E",	TRUE,	ifc_maxmove,		"ifcheck maxmove" },
{ "maxweight",		IFC_ANY,	"E",	TRUE,	ifc_maxweight,		"ifcheck maxweight" },
{ "maxxp",		IFC_ANY,	"E",	TRUE,	ifc_maxxp,		"ifcheck maxxp" },
{ "mobexists",		IFC_ANY,	"S",	FALSE,	ifc_mobexists,		"ifcheck mobexists" },
{ "mobhere",		IFC_ANY,	"S",	FALSE,	ifc_mobhere,		"ifcheck mobhere" },
{ "mobs",		IFC_ANY,	"E",	TRUE,	ifc_mobs,		"ifcheck mobs" },
{ "mobsize",		IFC_ANY,	"E",	TRUE,	ifc_mobsize,		"ifcheck mobsize" },
{ "money",		IFC_ANY,	"E",	TRUE,	ifc_money,		"ifcheck money" },
{ "monkills",		IFC_ANY,	"E",	TRUE,	ifc_monkills,		"ifcheck monkills" },
{ "month",		IFC_ANY,	"",	TRUE,	ifc_month,		"ifcheck month" },
{ "moonphase",		IFC_ANY,	"",	TRUE,	ifc_moonphase,		"ifcheck moonphase" },
{ "moveregen",		IFC_ANY,	"E",	TRUE,	ifc_moveregen,		"ifcheck moveregen" },
{ "name",		IFC_ANY,	"ES",	FALSE,	ifc_name,		"ifcheck name" },
{ "numenchants",	IFC_ANY,	"ES",	TRUE,	ifc_numenchants,	"ifcheck numenchants" },
{ "objcond",		IFC_ANY,	"E",	TRUE,	ifc_objcond,		"ifcheck objcond" },
{ "objcost",		IFC_ANY,	"E",	TRUE,	ifc_objcost,		"ifcheck objcost" },
{ "objexists",		IFC_ANY,	"S",	FALSE,	ifc_objexists,		"ifcheck objexists" },
{ "objextra",		IFC_ANY,	"ES",	FALSE,	ifc_objextra,		"ifcheck objextra" },
{ "objextra2",		IFC_ANY,	"ES",	FALSE,	ifc_objextra2,		"ifcheck objextra2" },
{ "objextra3",		IFC_ANY,	"ES",	FALSE,	ifc_objextra3,		"ifcheck objextra3" },
{ "objextra4",		IFC_ANY,	"ES",	FALSE,	ifc_objextra4,		"ifcheck objextra4" },
{ "objfrag",		IFC_ANY,	"E",	FALSE,	ifc_objfrag,		"ifcheck objfrag" },
{ "objhere",		IFC_ANY,	"ES",	FALSE,	ifc_objhere,		"ifcheck objhere" },
{ "objmaxweight",	IFC_ANY,	"E",	TRUE,	ifc_objmaxweight,	"ifcheck objmaxweight" },
{ "objtimer",		IFC_ANY,	"E",	TRUE,	ifc_objtimer,		"ifcheck objtimer" },
{ "objtype",		IFC_ANY,	"E",	FALSE,	ifc_objtype,		"ifcheck objtype" },
{ "objval0",		IFC_ANY,	"E",	TRUE,	ifc_objval0,		"ifcheck objval0" },
{ "objval1",		IFC_ANY,	"E",	TRUE,	ifc_objval1,		"ifcheck objval1" },
{ "objval2",		IFC_ANY,	"E",	TRUE,	ifc_objval2,		"ifcheck objval2" },
{ "objval3",		IFC_ANY,	"E",	TRUE,	ifc_objval3,		"ifcheck objval3" },
{ "objval4",		IFC_ANY,	"E",	TRUE,	ifc_objval4,		"ifcheck objval4" },
{ "objval5",		IFC_ANY,	"E",	TRUE,	ifc_objval5,		"ifcheck objval5" },
{ "objval6",		IFC_ANY,	"E",	TRUE,	ifc_objval6,		"ifcheck objval6" },
{ "objval7",		IFC_ANY,	"E",	TRUE,	ifc_objval7,		"ifcheck objval7" },
{ "objwear",		IFC_ANY,	"ES",	FALSE,	ifc_objwear,		"ifcheck objwear" },
{ "objwearloc",		IFC_ANY,	"ES",	FALSE,	ifc_objwearloc,		"ifcheck objwear" },
{ "objweight",		IFC_ANY,	"E",	TRUE,	ifc_objweight,		"ifcheck objweight" },
{ "objweightleft",	IFC_ANY,	"E",	TRUE,	ifc_objweightleft,	"ifcheck objweightleft" },
{ "off",		IFC_ANY,	"ES",	FALSE,	ifc_off,		"ifcheck off" },
{ "order",		IFC_M|IFC_O,	"E",	TRUE,	ifc_order,		"ifcheck order" },
{ "parts",		IFC_ANY,	"ES",	FALSE,	ifc_parts,		"ifcheck parts" },
{ "people",		IFC_ANY,	"E",	TRUE,	ifc_people,		"ifcheck people" },
{ "permcon",		IFC_ANY,	"E",	TRUE,	ifc_permcon,		"ifcheck permcon" },
{ "permdex",		IFC_ANY,	"E",	TRUE,	ifc_permdex,		"ifcheck permdex" },
{ "permint",		IFC_ANY,	"E",	TRUE,	ifc_permint,		"ifcheck permint" },
{ "permstr",		IFC_ANY,	"E",	TRUE,	ifc_permstr,		"ifcheck permstr" },
{ "permwis",		IFC_ANY,	"E",	TRUE,	ifc_permwis,		"ifcheck permwis" },
{ "pgroupcon",		IFC_ANY,	"E",	TRUE,	ifc_pgroupcon,		"ifcheck pgroupcon" },
{ "pgroupdex",		IFC_ANY,	"E",	TRUE,	ifc_pgroupdex,		"ifcheck pgroupdex" },
{ "pgroupint",		IFC_ANY,	"E",	TRUE,	ifc_pgroupint,		"ifcheck pgroupint" },
{ "pgroupstr",		IFC_ANY,	"E",	TRUE,	ifc_pgroupstr,		"ifcheck pgroupstr" },
{ "pgroupwis",		IFC_ANY,	"E",	TRUE,	ifc_pgroupwis,		"ifcheck pgroupwis" },
{ "pkfights",		IFC_ANY,	"E",	TRUE,	ifc_pkfights,		"ifcheck pkfights" },
{ "pkloss",		IFC_ANY,	"E",	TRUE,	ifc_pkloss,		"ifcheck pkloss" },
{ "pkratio",		IFC_ANY,	"E",	TRUE,	ifc_pkratio,		"ifcheck pkratio" },
{ "pkwins",		IFC_ANY,	"E",	TRUE,	ifc_pkwins,		"ifcheck pkwins" },
{ "players",		IFC_ANY,	"E",	TRUE,	ifc_players,		"ifcheck players" },
{ "pneuma",		IFC_ANY,	"E",	TRUE,	ifc_pneuma,		"ifcheck pneuma" },
{ "portal",		IFC_ANY,	"ES",	FALSE,	ifc_portal,		"ifcheck portal" },
{ "portalexit",		IFC_ANY,	"ES",	FALSE,	ifc_portalexit,		"ifcheck portalexit" },
{ "pos",		IFC_ANY,	"ES",	FALSE,	ifc_pos,		"ifcheck pos" },
{ "practices",		IFC_ANY,	"E",	TRUE,	ifc_practices,		"ifcheck practices" },
{ "questpoint",		IFC_ANY,	"E",	TRUE,	ifc_quest,		"ifcheck questpoint" },
{ "race",		IFC_ANY,	"ES",	FALSE,	ifc_race,		"ifcheck race" },
{ "rand",		IFC_ANY,	"Nn",	FALSE,	ifc_rand,		"ifcheck rand" },
{ "randpoint",		IFC_ANY,	"Nn",	FALSE,	ifc_randpoint,		"ifcheck randpoint" },
{ "reckoning",		IFC_ANY,	"",	TRUE,	ifc_reckoning,		"ifcheck reckoning" },
{ "reckoningchance",	IFC_ANY,	"",	TRUE,	ifc_reckoningchance,	"ifcheck reckoningchance" },
{ "res",		IFC_ANY,	"ES",	FALSE,	ifc_res,		"ifcheck res" },
{ "room",		IFC_ANY,	"E",	TRUE,	ifc_room,		"ifcheck room" },
{ "roomflag",		IFC_ANY,	"ES",	FALSE,	ifc_roomflag,		"ifcheck roomflag" },
{ "roomflag2",		IFC_ANY,	"ES",	FALSE,	ifc_roomflag2,		"ifcheck roomflag2" },
{ "roomviewwilds",	IFC_ANY,	"E",	TRUE,	ifc_roomviewwilds,	"ifcheck roomviewwilds" },
{ "roomweight",		IFC_ANY,	"E",	TRUE,	ifc_roomweight,		"ifcheck roomweight" },
{ "roomwilds",		IFC_ANY,	"E",	TRUE,	ifc_roomwilds,		"ifcheck roomwilds" },
{ "roomx",		IFC_ANY,	"E",	TRUE,	ifc_roomx,		"ifcheck roomx" },
{ "roomy",		IFC_ANY,	"E",	TRUE,	ifc_roomy,		"ifcheck roomy" },
{ "roomz",		IFC_ANY,	"E",	TRUE,	ifc_roomz,		"ifcheck roomz" },
{ "samegroup",		IFC_ANY,	"ES",	FALSE,	ifc_samegroup,		"ifcheck samegroup" },
{ "scriptsecurity",	IFC_ANY,	"",	TRUE,	ifc_scriptsecurity,	"ifcheck systemtime" },
{ "sector",		IFC_ANY,	"ES",	FALSE,	ifc_sector,		"ifcheck sector" },
{ "sex",		IFC_ANY,	"E",	TRUE,	ifc_sex,		"ifcheck sex" },
{ "sign",		IFC_ANY,	"N",	TRUE,	ifc_sign,		"ifcheck sign" },
{ "silver",		IFC_ANY,	"E",	TRUE,	ifc_silver,		"ifcheck silver" },
{ "sin",		IFC_ANY,	"N",	TRUE,	ifc_sin,		"ifcheck sin" },
{ "skeyword",		IFC_ANY,	"ES",	FALSE,	ifc_skeyword,		"ifcheck skeyword" },
{ "skill",		IFC_ANY,	"ES",	TRUE,	ifc_skill,		"ifcheck skill" },
{ "statcon",		IFC_ANY,	"E",	TRUE,	ifc_statcon,		"ifcheck statcon" },
{ "statdex",		IFC_ANY,	"E",	TRUE,	ifc_statdex,		"ifcheck statdex" },
{ "statint",		IFC_ANY,	"E",	TRUE,	ifc_statint,		"ifcheck statint" },
{ "statstr",		IFC_ANY,	"E",	TRUE,	ifc_statstr,		"ifcheck statstr" },
{ "statwis",		IFC_ANY,	"E",	TRUE,	ifc_statwis,		"ifcheck statwis" },
{ "strlen",		IFC_ANY,	"E",	TRUE,	ifc_strlen,		"ifcheck strlen" },
{ "sublevel",		IFC_ANY,	"E",	TRUE,	ifc_sublevel,		"ifcheck sublevel" },
{ "systemtime",		IFC_ANY,	"",	TRUE,	ifc_systemtime,		"ifcheck systemtime" },
{ "tempstore1",		IFC_ANY,	"E",	TRUE,	ifc_tempstore1,		"ifcheck tempstore1" },
{ "tempstore2",		IFC_ANY,	"E",	TRUE,	ifc_tempstore2,		"ifcheck tempstore2" },
{ "tempstore3",		IFC_ANY,	"E",	TRUE,	ifc_tempstore3,		"ifcheck tempstore3" },
{ "tempstore4",		IFC_ANY,	"E",	TRUE,	ifc_tempstore4,		"ifcheck tempstore4" },
{ "testhardmagic",	IFC_ANY,	"ES",	FALSE,	ifc_testhardmagic,	"ifcheck testhardmagic" },
{ "testskill",		IFC_ANY,	"ES",	FALSE,	ifc_testskill,		"ifcheck testskill" },
{ "testslowmagic",	IFC_ANY,	"ES",	FALSE,	ifc_testslowmagic,	"ifcheck testslowmagic" },
{ "testtokenspell",	IFC_ANY,	"ES",	FALSE,	ifc_testtokenspell,	"ifcheck testtokenspell" },
{ "thirst",		IFC_ANY,	"E",	TRUE,	ifc_thirst,		"ifcheck thirst" },
{ "timeofday",		IFC_ANY,	"S",	FALSE,	ifc_timeofday,		"ifcheck timeofday" },
{ "timer",		IFC_ANY,	"",	TRUE,	ifc_timer,		"ifcheck timer" },
{ "tokencount",		IFC_ANY,	"En",	TRUE,	ifc_tokencount,		"ifcheck tokencount" },
{ "tokenexists",	IFC_ANY,	"N",	FALSE,	ifc_tokenexists,	"ifcheck tokenexists" },
{ "tokenvalue",		IFC_ANY,	"ENN",	TRUE,	ifc_tokenvalue,		"ifcheck tokenvalue" },
{ "totalfights",	IFC_ANY,	"E",	TRUE,	ifc_totalfights,	"ifcheck totalfights" },
{ "totalloss",		IFC_ANY,	"E",	TRUE,	ifc_totalloss,		"ifcheck totalloss" },
{ "totalpkfights",	IFC_ANY,	"E",	TRUE,	ifc_totalpkfights,	"ifcheck totalpkfights" },
{ "totalpkloss",	IFC_ANY,	"E",	TRUE,	ifc_totalpkloss,	"ifcheck totalpkloss" },
{ "totalpkratio",	IFC_ANY,	"E",	TRUE,	ifc_totalpkratio,	"ifcheck totalpkratio" },
{ "totalpkwins",	IFC_ANY,	"E",	TRUE,	ifc_totalpkwins,	"ifcheck totalpkwins" },
{ "totalquests",	IFC_ANY,	"E",	TRUE,	ifc_totalquests,	"ifcheck totalquests" },
{ "totalratio",		IFC_ANY,	"E",	TRUE,	ifc_totalratio,		"ifcheck totalratio" },
{ "totalwins",		IFC_ANY,	"E",	TRUE,	ifc_totalwins,		"ifcheck totalwins" },
{ "toxin",		IFC_ANY,	"ES",	TRUE,	ifc_toxin,		"ifcheck toxin" },
{ "trains",		IFC_ANY,	"E",	TRUE,	ifc_trains,		"ifcheck trains" },
{ "uses",		IFC_ANY,	"ES",	FALSE,	ifc_uses,		"ifcheck uses" },
{ "valueac",		IFC_ANY,	"",	TRUE,	ifc_value_ac,		"ifcheck valueac" },
{ "valueacstr",		IFC_ANY,	"",	TRUE,	ifc_value_acstr,		"ifcheck valueacstr" },
{ "valuedamage",	IFC_ANY,	"",	TRUE,	ifc_value_damage,	"ifcheck valuedamage" },
{ "valueposition",	IFC_ANY,	"",	TRUE,	ifc_value_position,	"ifcheck valueposition" },
{ "valueranged",	IFC_ANY,	"",	TRUE,	ifc_value_ranged,	"ifcheck valueranged" },
{ "valuerelic",		IFC_ANY,	"",	TRUE,	ifc_value_relic,	"ifcheck valuerelic" },
{ "valuesector",	IFC_ANY,	"",	TRUE,	ifc_value_sector,	"ifcheck valuesector" },
{ "valuesize",		IFC_ANY,	"",	TRUE,	ifc_value_size,		"ifcheck valuesize" },
{ "valuetoxin",		IFC_ANY,	"",	TRUE,	ifc_value_toxin,	"ifcheck valuetoxin" },
{ "valuetype",		IFC_ANY,	"",	TRUE,	ifc_value_type,		"ifcheck valuetype" },
{ "valueweapon",	IFC_ANY,	"",	TRUE,	ifc_value_weapon,	"ifcheck valueweapon" },
{ "valuewear",		IFC_ANY,	"",	TRUE,	ifc_value_wear,		"ifcheck valuewear" },
{ "vardefined",		IFC_ANY,	"SS",	FALSE,	ifc_vardefined,		"ifcheck vardefined" },
{ "varexit",		IFC_ANY,	"SS",	FALSE,	ifc_varexit,		"ifcheck varexit" },
{ "varnumber",		IFC_ANY,	"S",	TRUE,	ifc_varnumber,		"ifcheck varnumber" },
{ "varstring",		IFC_ANY,	"SS",	FALSE,	ifc_varstring,		"ifcheck varstring" },
{ "vnum",		IFC_ANY,	"E",	TRUE,	ifc_vnum,		"ifcheck vnum" },
{ "vuln",		IFC_ANY,	"ES",	FALSE,	ifc_vuln,		"ifcheck vuln" },
{ "weapon",		IFC_ANY,	"ES",	FALSE,	ifc_weapon,		"ifcheck weapon" },
{ "weapontype",		IFC_ANY,	"ES",	FALSE,	ifc_weapontype,		"ifcheck weapontype" },
{ "weaponskill",	IFC_ANY,	"ES",	TRUE,	ifc_weaponskill,	"ifcheck weaponskill" },
{ "wears",		IFC_ANY,	"ES",	FALSE,	ifc_wears,		"ifcheck wears" },
{ "wearused",		IFC_ANY,	"ES",	FALSE,	ifc_wearused,		"ifcheck wearused" },
{ "weight",		IFC_ANY,	"E",	TRUE,	ifc_weight,		"ifcheck weight" },
{ "weightleft",		IFC_ANY,	"E",	TRUE,	ifc_weightleft,		"ifcheck weightleft" },
{ "wimpy",		IFC_ANY,	"E",	TRUE,	ifc_wimpy,		"ifcheck wimpy" },
{ "word",		IFC_ANY,	"E",	FALSE,	ifc_word,		"ifcheck word" },
{ "wornby",		IFC_O,		"E",	FALSE,	ifc_wornby,		"ifcheck wornby" },
{ "xp",			IFC_ANY,	"",	TRUE,	ifc_xp,			"ifcheck xp" },
{ "year",		IFC_ANY,	"",	TRUE,	ifc_year,		"ifcheck year" },
{ NULL, 		0,		"",	FALSE,	NULL,			"" }
};

OPCODE_FUNC opcode_table[OP_LASTCODE] = {
	opc_end,
	opc_if,
	opc_ifnot,
	opc_or,
	opc_nor,
	opc_and,
	opc_nand,
	opc_if,
	opc_ifnot,
	opc_else,
	opc_endif,
	opc_command,
	opc_gotoline,
	opc_for,
	opc_endfor,
	opc_exitfor,
	opc_list,
	opc_endlist,
	opc_exitlist,
	opc_while,
	opc_whilenot,
	opc_endwhile,
	opc_exitwhile,
	opc_mob,
	opc_obj,
	opc_room,
	opc_token,
	opc_tokenother
};

char *script_operators[] = { "==", ">=", "<=", ">", "<", "!=", "&", NULL };

// The operator comparison matrix
int script_expression_stack_action[CH_MAX][STK_MAX] = {
// Stack Top:	-(neg)	!	:	%	/	*	-	+	(	empty	  // Current Operator VVV
	{	PUSH,	PUSH,	PUSH,	PUSH,	PUSH,	PUSH,	PUSH,	PUSH,	PUSH,	PUSH },   // (
	{	PUSH,	PUSH,	PUSH,	PUSH,	PUSH,	PUSH,	PUSH,	PUSH,	PUSH,	PUSH },   // -(neg)
	{	PUSH,	PUSH,	PUSH,	PUSH,	PUSH,	PUSH,	PUSH,	PUSH,	PUSH,	PUSH },   // !
	{	PUSH,	PUSH,	PUSH,	PUSH,	PUSH,	PUSH,	PUSH,	PUSH,	PUSH,	PUSH },   // :
	{	POP,	POP,	POP,	POP,	POP,	POP,	PUSH,	PUSH,	PUSH,	PUSH },   // %
	{	POP,	POP,	POP,	POP,	POP,	POP,	PUSH,	PUSH,	PUSH,	PUSH },   // /
	{	POP,	POP,	POP,	POP,	POP,	POP,	PUSH,	PUSH,	PUSH,	PUSH },   // *
	{	POP,	POP,	POP,	POP,	POP,	POP,	POP,	POP,	PUSH,	PUSH },   // -
	{	POP,	POP,	POP,	POP,	POP,	POP,	POP,	POP,	PUSH,	PUSH },   // +
	{	POP,	POP,	POP,	POP,	POP,	POP,	POP,	POP,	DELETE,	ERROR1 }, // )
	{	POP,	POP,	POP,	POP,	POP,	POP,	POP,	POP,	ERROR2,	DONE }    // end-of-string
};

// Conversions from CH_* to STK_* codes
int script_expression_tostack[CH_MAX+1] = { STK_OPEN, STK_NEG, STK_NOT, STK_RAND, STK_MOD, STK_DIV, STK_MUL, STK_SUB, STK_ADD, STK_MAX, STK_MAX, STK_MAX };

// Number of operands needed for operator when performing it
int script_expression_argstack[STK_MAX+1] = {1,1,2,2,2,2,2,2,0,0,0};

const char *male_female  [] = { "neuter",  "male",  "female" };
const char *he_she  [] = { "it",  "he",  "she" };
const char *him_her [] = { "it",  "him", "her" };
const char *his_her [] = { "its",  "his", "her" };
const char *his_hers_obj [] = { "its",  "his", "her" };
const char *his_hers [] = { "its", "his", "hers" };
const char *himself [] = { "itself", "himself", "herself" };


const char *exit_states[] = {
	"open",
	"bashed",
	"closed",
	"closed and locked",
	"closed and barred",
	"closed, locked and barred"
};


const struct flag_type script_flags[] = {
	{ "wiznet", SCRIPT_WIZNET, TRUE },
	{ "disabled", SCRIPT_DISABLED, TRUE },
	{ "secured", SCRIPT_SECURED, TRUE },
	{ NULL, 0, FALSE },
};

const struct flag_type interrupt_action_types[] = {
	{	"bind",		INTERRUPT_BIND,		TRUE },
	{	"bomb",		INTERRUPT_BOMB,		TRUE },
	{	"brew",		INTERRUPT_BREW,		TRUE },
	{	"cast",		INTERRUPT_CAST,		TRUE },
	{	"fade",		INTERRUPT_FADE,		TRUE },
	{	"hide",		INTERRUPT_HIDE,		TRUE },
	{	"music",	INTERRUPT_MUSIC,	TRUE },
	{	"ranged",	INTERRUPT_RANGED,	TRUE },
	{	"recite",	INTERRUPT_RECITE,	TRUE },
	{	"repair",	INTERRUPT_REPAIR,	TRUE },
	{	"resurrect",	INTERRUPT_RESURRECT,	TRUE },
	{	"reverie",	INTERRUPT_REVERIE,	TRUE },
	{	"scribe",	INTERRUPT_SCRIBE,	TRUE },
	{	"trance",	INTERRUPT_TRANCE,	TRUE },
	{	"silent",	INTERRUPT_SILENT,	TRUE },
	{	NULL,		0,			FALSE }
};
