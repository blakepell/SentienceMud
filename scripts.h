/***************************************************************************
 *                                                                         *
 *    Scripting engine rebuilt by Michael Kurtz (Nibelung)                 *
 *    Used with permission.                                                *
 *                                                                         *
 **************************************************************************/

#ifndef __SCRIPTS_H__
#define __SCRIPTS_H__

#define IFC_NONE	0
#define	IFC_M	(A)	/* Allowed in mprogs */
#define	IFC_O	(B)	/* Allowed in oprogs */
#define	IFC_R	(C)	/* Allowed in rprogs */
#define	IFC_T	(D)	/* Allowed in tprogs */
#define	IFC_A	(E)	/* Allowed in aprogs */

#define IFC_MO	(IFC_M|IFC_O)
#define	IFC_ANY	(IFC_M|IFC_O|IFC_R|IFC_T|IFC_A)	/* Any prog type */

#define IFC_MAXPARAMS		20
#define MAX_STACK		20	/* Adjust as desired */
#define MAX_NAMED_LABELS	256
#define MAX_NESTED_LOOPS	20
#define SYSTEM_SCRIPT_SECURITY	(10)
#define MAX_SCRIPT_SECURITY	(9)
#define MIN_SCRIPT_SECURITY	(0)
#define NO_SCRIPT_SECURITY	(-1)
#define INIT_SCRIPT_SECURITY	(-2)

#define SCRIPT_WIZNET		(A)	/* The script will wiznet to WIZ_SCRIPTS */
#define SCRIPT_DISABLED		(B)	/* The script must be turned off by an IMP */
#define SCRIPT_LUA			(C)	/* This script is a LUA compiled script. */
#define SCRIPT_SECURED		(D)	/* This script will reset security settings to its values */
#define SCRIPT_SYSTEM		(E)	// A system script, may ONLY be called when security is SYSTEM security
#define SCRIPT_INSPECT		(Z)	/* Inspect the script for restricted actions */

#define SCRIPTEXEC_HALT		(A)	/* Kill script execution because the controller entity had been destructed */

/* This should be moved to merc.h and made general */
#define INTERRUPT_CAST		(A)
#define INTERRUPT_MUSIC		(B)
#define INTERRUPT_BREW		(C)
#define INTERRUPT_REPAIR	(D)
#define INTERRUPT_HIDE		(E)
#define INTERRUPT_BIND		(F)
#define INTERRUPT_BOMB		(G)
#define INTERRUPT_RECITE	(H)
#define INTERRUPT_REVERIE	(I)
#define INTERRUPT_TRANCE	(J)
#define INTERRUPT_SCRIBE	(K)
#define INTERRUPT_RANGED	(L)
#define INTERRUPT_RESURRECT	(M)
#define INTERRUPT_FADE		(N)
#define INTERRUPT_SCRIPT	(dd)	/* Used to interrupt whatever script action is going, that is up to the individual scripts to determine that! */
#define INTERRUPT_SILENT	(ee)	/* Used to make the interrupt SILENT */


#define DECL_IFC_FUN(x) bool x (SCRIPT_VARINFO *info, CHAR_DATA *mob,OBJ_DATA *obj,ROOM_INDEX_DATA *room, TOKEN_DATA *token,int *ret,int argc,SCRIPT_PARAM *argv)
#define DECL_OPC_FUN(x) bool x (SCRIPT_CB *block)
#define SCRIPT_CMD(x)	void x (SCRIPT_VARINFO *info, char *argument)

#define ESCAPE_END		0x01
#define ESCAPE_ENTITY		0x02
#define ESCAPE_VARIABLE		0x03
#define ESCAPE_EXPRESSION	0x04
#define ESCAPE_EXTRA		0x05

#define pop(stk,mt) (((stk)->t > 0) ? (stk)->s[--(stk)->t] : (mt))
#define top(stk,mt) (((stk)->t > 0) ? (stk)->s[(stk)->t - 1] : (mt))

typedef struct stack_type {
	int t;
	int s[MAX_STACK];
} STACK;



/* Stack tokens */
enum { STK_NEG, STK_NOT, STK_RAND, STK_MOD, STK_DIV, STK_MUL, STK_SUB, STK_ADD, STK_OPEN, STK_EMPTY, STK_MAX };

/* Character tokens */
enum { CH_OPEN, CH_NEG, CH_NOT, CH_RAND, CH_MOD, CH_DIV, CH_MUL, CH_SUB, CH_ADD, CH_CLOSE, CH_EOS, CH_MAX };

/* Actions to perform when dealing with operator comparisions */
enum {
	PUSH,	/* push operator onto stack */
	POP,	/* pop operator off stack and perform action on operands */
	DELETE,	/* delete the top operator */
	ERROR0,	/* invalid operator */
	ERROR1,	/* unmatched right parenthesis */
	ERROR2,	/* unmatched left parenthesis */
	ERROR3,	/* divide by zero */
	ERROR4,	/* insufficient operators/operands on stack */
	DONE	/* done processing the stack */
};




enum ifcheck_enum {
	/* A */
	CHK_ABS=0,CHK_ACT,CHK_ACT2,
	CHK_AFFECTBIT,CHK_AFFECTBIT2,
	CHK_AFFECTED,CHK_AFFECTED2,CHK_AFFECTEDNAME,CHK_AFFECTEDSPELL,
	CHK_AFFECTGROUP,CHK_AFFECTLOCATION,CHK_AFFECTMODIFIER,
	CHK_AFFECTSKILL,CHK_AFFECTTIMER,

	CHK_AGE,CHK_ALIGN,CHK_ANGLE,
	CHK_AREAHASLAND,CHK_AREAID,CHK_AREALANDX,CHK_AREALANDY,
	CHK_ARENAFIGHTS,CHK_ARENALOSS,CHK_ARENARATIO,CHK_ARENAWINS,CHK_AREAX,CHK_AREAY,

	/* B */
	CHK_BANKBALANCE,CHK_BIT,

	/* C */
	CHK_CANDROP,CHK_CANGET,CHK_CANHUNT,CHK_CANPRACTICE,CHK_CANPUT,
	CHK_CANSCARE,CHK_CARRIEDBY,CHK_CARRIES,CHK_CARRYLEFT,
	CHK_CHURCH,CHK_CHURCHHASRELIC,CHK_CHURCHONLINE,CHK_CHURCHRANK,CHK_CHURCHSIZE,
	CHK_CLAN,CHK_CLASS,CHK_CLONES,
	CHK_COMM,CHK_CONTAINER,CHK_COS,CHK_CPKFIGHTS,CHK_CPKLOSS,CHK_CPKRATIO,CHK_CPKWINS,CHK_CURHIT,
	CHK_CURMANA,CHK_CURMOVE,

	/* D */
	CHK_DANGER,CHK_DAMTYPE,CHK_DAY,CHK_DEATH,CHK_DEATHCOUNT,CHK_DEITY,CHK_DICE,CHK_DRUNK,

	/* E */
	CHK_EXISTS,CHK_EXITEXISTS,CHK_EXITFLAG,

	/* F */
	CHK_FINDPATH,
	CHK_FLAG_ACT,CHK_FLAG_ACT2,CHK_FLAG_AFFECT,CHK_FLAG_AFFECT2,CHK_FLAG_COMM,CHK_FLAG_CONTAINER,CHK_FLAG_CORPSE,
	CHK_FLAG_EXIT,CHK_FLAG_EXTRA,CHK_FLAG_EXTRA2,CHK_FLAG_EXTRA3,CHK_FLAG_EXTRA4,CHK_FLAG_FORM,CHK_FLAG_FURNITURE,CHK_FLAG_IMM,CHK_FLAG_INTERRUPT,
	CHK_FLAG_OFF,CHK_FLAG_PART,CHK_FLAG_PORTAL,CHK_FLAG_RES,CHK_FLAG_ROOM,CHK_FLAG_ROOM2,
	CHK_FLAG_VULN,CHK_FLAG_WEAPON,CHK_FLAG_WEAR,CHK_FULLNESS,CHK_FURNITURE,


	/* G */
	CHK_GOLD,CHK_GROUNDWEIGHT,CHK_GROUPCON,CHK_GROUPDEX,CHK_GROUPHIT,CHK_GROUPINT,
	CHK_GROUPMANA,CHK_GROUPMAXHIT,CHK_GROUPMAXMANA,CHK_GROUPMAXMOVE,CHK_GROUPMOVE,
	CHK_GROUPSTR,CHK_GROUPWIS,CHK_GRPSIZE,

	/* H */
	CHK_HANDSFULL,CHK_HAS,CHK_HASCATALYST,CHK_HASCHECKPOINT,CHK_HASENVIRONMENT,CHK_HASPROMPT,CHK_HASQUEUE,
	CHK_HASSHIP,CHK_HASSPELL,CHK_HASSUBCLASS,
	CHK_HASTARGET,CHK_HASTOKEN,
	CHK_HASVLINK,CHK_HEALREGEN,CHK_HITDAMAGE,CHK_HITDAMCLASS,CHK_HITDAMTYPE,CHK_HITSKILLTYPE,
	CHK_HOUR,CHK_HPCNT,CHK_HUNGER,


	/* I */
	CHK_ID,CHK_ID2,CHK_IDENTICAL,CHK_IMM,CHK_INCHURCH,CHK_INNATURE,CHK_INPUTWAIT,CHK_INWILDS,

	/* IS */
		CHK_ISACTIVE,
		CHK_ISAFFECTCUSTOM,
		CHK_ISAFFECTGROUP,
		CHK_ISAFFECTSKILL,
		CHK_ISAFFECTWHERE,

		CHK_ISAMBUSHING,CHK_ISANGEL,
		CHK_ISBREWING,CHK_ISBUSY,
		CHK_ISCASTFAILURE,CHK_ISCASTING,CHK_ISCASTRECOVERED,CHK_ISCASTROOMBLOCKED,CHK_ISCASTSUCCESS,
		CHK_ISCHARM,CHK_ISCHURCHEXCOM,CHK_ISCHURCHPK,CHK_ISCLONEROOM,CHK_ISCPKPROOF,CHK_ISCROSSZONE,
		CHK_ISDEAD,CHK_ISDELAY,CHK_ISDEMON,
		CHK_ISEVIL,
		CHK_ISFADING,CHK_ISFIGHTING,CHK_ISFLYING,CHK_ISFOLLOW,
		CHK_ISGOOD,
		CHK_ISHUNTING,
		CHK_ISIMMORT,
		CHK_ISKEY,
		CHK_ISLEADER,
		CHK_ISMOONUP,CHK_ISMORPHED,CHK_ISMYSTIC,
		CHK_ISNEUTRAL,CHK_ISNPC,
		CHK_ISON,
		CHK_ISPC,CHK_ISPERSIST,CHK_ISPK,CHK_ISPREY,CHK_ISPULLING,CHK_ISPULLINGRELIC,
		CHK_ISQUESTING,
		CHK_ISREMORT,CHK_ISREPAIRABLE,CHK_ISRESTRUNG,CHK_ISRIDDEN,CHK_ISRIDER,CHK_ISRIDING,CHK_ISROOMDARK,
		CHK_ISSAFE,CHK_ISSCRIBING,CHK_ISSHIFTED,CHK_ISSHOOTING,CHK_ISSHOPKEEPER,CHK_ISSPELL,CHK_ISSUBCLASS,CHK_ISSUSTAINED,
		CHK_ISTARGET,CHK_ISTREASUREROOM,
		CHK_ISVISIBLE,CHK_ISVISIBLETO,
		CHK_ISWORN,

	/* L */
	CHK_LASTRETURN,CHK_LEVEL,CHK_LIQUID,CHK_LISTCONTAINS,CHK_LOSTPARTS,

	/* M */
	CHK_MANAREGEN,CHK_MANASTORE,
	CHK_MAPAREA,CHK_MAPHEIGHT,CHK_MAPID,CHK_MAPVALID,CHK_MAPWIDTH,CHK_MAPX,CHK_MAPY,
	CHK_MATERIAL,
	CHK_MAX,
	CHK_MAXCARRY,CHK_MAXHIT,CHK_MAXMANA,CHK_MAXMOVE,
	CHK_MAXWEIGHT,CHK_MAXXP,
	CHK_MIN,
	CHK_MOBEXISTS,CHK_MOBHERE,CHK_MOBS,CHK_MOBSIZE,CHK_MONEY,CHK_MONKILLS,
	CHK_MONTH,CHK_MOONPHASE,CHK_MOVEREGEN,


	/* N */
	CHK_NAME,CHK_NUMENCHANTS,

	/* O */
	CHK_OBJCOND,CHK_OBJCORPSE,CHK_OBJCOST,CHK_OBJEXISTS,
	CHK_OBJEXTRA,CHK_OBJEXTRA2,CHK_OBJEXTRA3,CHK_OBJEXTRA4,CHK_OBJFRAG,CHK_OBJHERE,
	CHK_OBJMAXWEIGHT,CHK_OBJRANGED,CHK_OBJTIMER,CHK_OBJTYPE,CHK_OBJVAL0,CHK_OBJVAL1,CHK_OBJVAL2,
	CHK_OBJVAL3,CHK_OBJVAL4,CHK_OBJVAL5,CHK_OBJVAL6,CHK_OBJVAL7,CHK_OBJWEAPON,
	CHK_OBJWEAPONSTAT,CHK_OBJWEAR,CHK_OBJWEARLOC,CHK_OBJWEIGHT,CHK_OBJWEIGHTLEFT,
	CHK_OFF,CHK_ORDER,


	/* P */
	CHK_PARTS,
	CHK_PEOPLE,CHK_PERMCON,CHK_PERMDEX,CHK_PERMINT,CHK_PERMSTR,CHK_PERMWIS,
	CHK_PGROUPCON,CHK_PGROUPDEX,CHK_PGROUPINT,CHK_PGROUPSTR,CHK_PGROUPWIS,
	CHK_PKFIGHTS,CHK_PKLOSS,CHK_PKRATIO,CHK_PKWINS,CHK_PLAYEREXISTS,CHK_PLAYERS,
	CHK_PNEUMA,CHK_PORTAL,CHK_PORTALEXIT,CHK_POS,CHK_PRACTICES,


	/* Q */
	CHK_QUEST,


	/* R */
	CHK_RACE,CHK_RAND,CHK_RANDPOINT,
	CHK_RECKONING,CHK_RECKONINGCHANCE,CHK_REGISTER,
	CHK_RES,CHK_ROOM,CHK_ROOMFLAG,CHK_ROOMFLAG2,CHK_ROOMVIEWWILDS,
	CHK_ROOMWEIGHT,CHK_ROOMWILDS,CHK_ROOMX,CHK_ROOMY,CHK_ROOMZ,

	/* S */
	CHK_SAMEGROUP,
	CHK_SCRIPTSECURITY,
	CHK_SECTOR,CHK_SEX,CHK_SIGN,CHK_SILVER,CHK_SIN,CHK_SKEYWORD,CHK_SKILL,
	CHK_STATCON,CHK_STATDEX,CHK_STATINT,CHK_STATSTR,CHK_STATWIS,
	CHK_STONED,CHK_STRLEN,
	CHK_SUBLEVEL,CHK_SUNLIGHT,CHK_SYSTEMTIME,


	/* T */
	CHK_TEMPSTORE1,CHK_TEMPSTORE2,CHK_TEMPSTORE3,CHK_TEMPSTORE4,
	CHK_TESTHARDMAGIC,CHK_TESTSKILL,CHK_TESTSLOWMAGIC,CHK_TESTTOKENSPELL,
	CHK_THIRST,CHK_TIMEOFDAY,CHK_TIMER,CHK_TOKENCOUNT,CHK_TOKENEXISTS,CHK_TOKENTIMER,
	CHK_TOKENTYPE,CHK_TOKENVALUE,CHK_TOTALFIGHTS,CHK_TOTALLOSS,CHK_TOTALPKFIGHTS,
	CHK_TOTALPKLOSS,CHK_TOTALPKRATIO,CHK_TOTALPKWINS,CHK_TOTALQUESTS,CHK_TOTALRATIO,
	CHK_TOTALWINS,CHK_TOXIN,CHK_TRAINS,

	/* U */
	CHK_USES,


	/* V */
	CHK_VALUE_AC,CHK_VALUE_ACSTR,CHK_VALUE_DAMAGE,CHK_VALUE_POSITION,CHK_VALUE_RANGED,CHK_VALUE_RELIC,CHK_VALUE_SECTOR,
	CHK_VALUE_SIZE,CHK_VALUE_TOXIN,CHK_VALUE_TYPE,CHK_VALUE_WEAPON,CHK_VALUE_WEAR,CHK_VARDEFINED,
	CHK_VAREXIT,CHK_VARNUMBER,CHK_VARSTRING,CHK_VNUM,CHK_VULN,


	/* W */
	CHK_WEAPON,CHK_WEAPONTYPE,CHK_WEAPONSKILL,CHK_WEARS,CHK_WEARUSED,CHK_WEIGHT,CHK_WEIGHTLEFT,
	CHK_WIMPY,CHK_WORD,CHK_WORNBY,

	/* X */
	CHK_XP,

	/* Y */
	CHK_YEAR,

	/* Z */



	/* END */

	CHK_MAXIFCHECKS
};





enum variable_enum {
	VAR_UNKNOWN,
	VAR_INTEGER,
	VAR_STRING,
	VAR_STRING_S,		/* Shared, allocated elsewhere! */
	VAR_ROOM,
	VAR_EXIT,
	VAR_MOBILE,
	VAR_OBJECT,
	VAR_TOKEN,
	VAR_AREA,
	VAR_SKILL,			/* A skill INDEX that will reference general skill data */
	VAR_SKILLINFO,		/* Skill reference on a creature { CHAR_DATA *owner, int sn } */
	VAR_CONNECTION,
	VAR_AFFECT,			/* References an affect */
	VAR_WILDS,
	VAR_CHURCH,
	VAR_CLONE_ROOM,		// Used only when loading
	VAR_WILDS_ROOM,		// Used only when loading
	VAR_DOOR,			// Used only when loading
	VAR_CLONE_DOOR,		// Used only when loading
	VAR_WILDS_DOOR,		// Used only when loading
	VAR_MOBILE_ID,		// Used only when loading
	VAR_OBJECT_ID,		// Used only when loading
	VAR_TOKEN_ID,		// Used only when loading
	VAR_SKILLINFO_ID,	// Used only when loading
	VAR_AREA_ID,
	VAR_WILDS_ID,
	VAR_CHURCH_ID,
	VAR_BLLIST_FIRST,
	////////////////////////

	VAR_BLLIST_ROOM,
	VAR_BLLIST_MOB,
	VAR_BLLIST_OBJ,
	VAR_BLLIST_TOK,
	VAR_BLLIST_EXIT,
	VAR_BLLIST_SKILL,
	VAR_BLLIST_AREA,
	VAR_BLLIST_WILDS,

	////////////////////////
	VAR_BLLIST_LAST,
	VAR_PLLIST_FIRST,
	////////////////////////

	VAR_PLLIST_STR,
	VAR_PLLIST_CONN,
	VAR_PLLIST_ROOM,
	VAR_PLLIST_MOB,
	VAR_PLLIST_OBJ,
	VAR_PLLIST_TOK,
	VAR_PLLIST_CHURCH,

	////////////////////////
	VAR_PLLIST_LAST,
	VAR_MAX
};



enum ifcheck_param_types {
	IFCP_NONE = 0,
	IFCP_NUMBER,
	IFCP_STRING,
	IFCP_MOBILE,
	IFCP_OBJECT,
	IFCP_ROOM,
	IFCP_TOKEN,
	IFCP_AREA,
	IFCP_EXIT,
	IFCP_SKILL,
	IFCP_SKILLINFO,
	IFCP_CONN,
	IFCP_WILDS,
	IFCP_MAX
};

enum script_command_enum {
	OP_END = 0,		/* end/break */
	OP_IF,			/* if .... */
	OP_IFNOT,		/* if not ... / if !.... */
	OP_OR,			/* or .... */
	OP_NOR,			/* or not ... / or !.... */
	OP_AND,			/* and .... */
	OP_NAND,		/* and not ... / if !.... */
	OP_ELSEIF,		/* elseif .... */
	OP_ELSEIFNOT,		/* elseif !.... */
	OP_ELSE,		/* else */
	OP_ENDIF,		/* endif */
	OP_COMMAND,		/* ordinary command (MPROGS ONLY) */
	OP_GOTOLINE,		/* Goto */
	OP_FOR,
	OP_ENDFOR,
	OP_EXITFOR,
	OP_LIST,
	OP_ENDLIST,
	OP_EXITLIST,
	OP_WHILE,		/* while .... */
	OP_WHILENOT,		/* while not ... / if !.... */
	OP_ENDWHILE,
	OP_EXITWHILE,
	/* Add new opcodes here... */
	OP_MOB,			/* A mob command */
	OP_OBJ,			/* An obj command */
	OP_ROOM,		/* A room command */
	OP_TOKEN,		/* A token command */
	OP_TOKENOTHER,		/* A token command from other scripts */
/*	OP_AREA,		 An area command */
	OP_LASTCODE
};

struct entity_field_type {
	char *name;
	unsigned char code;
	unsigned char type;
};

enum entity_type_enum {
	ENT_NONE = 0,
	ENT_NULL,
	ENT_NUMBER,
	ENT_STRING,
	ENT_MOBILE,
	ENT_OBJECT,
	ENT_ROOM,
	ENT_EXIT,
	ENT_TOKEN,
	ENT_AREA,
	ENT_WILDS,
	ENT_SKILL,
	ENT_SKILLINFO,
	ENT_CONN,
	ENT_PRIOR,
	ENT_EXTRADESC,
	ENT_AFFECT,
	ENT_CHURCH,
	ENT_SONG,

	//////////////////////////////
	// ALL lists here are designed to be saved
	ENT_BLLIST_MIN,
	ENT_BLLIST_ROOM,
	ENT_BLLIST_MOB,
	ENT_BLLIST_OBJ,
	ENT_BLLIST_TOK,
	ENT_BLLIST_EXIT,
	ENT_BLLIST_SKILL,
	ENT_BLLIST_AREA,
	ENT_BLLIST_WILDS,
	ENT_BLLIST_MAX,
	//////////////////////////////

	//////////////////////////////
	// Only the STRING list is savable due to it being strings
	ENT_PLLIST_MIN,
	ENT_PLLIST_STR,
	ENT_PLLIST_CONN,
	ENT_PLLIST_ROOM,
	ENT_PLLIST_MOB,
	ENT_PLLIST_OBJ,
	ENT_PLLIST_TOK,
	ENT_PLLIST_CHURCH,
	ENT_PLLIST_MAX,
	//////////////////////////////

	//////////////////////////////
	// Owned list - referenced directly off entities
	ENT_OLLIST_MIN,
	ENT_OLLIST_MOB,
	ENT_OLLIST_OBJ,
	ENT_OLLIST_TOK,
	ENT_OLLIST_AFF,
	ENT_OLLIST_MAX,
	//////////////////////////////

	ENT_MOBILE_ID,		// Will act as ENT_MOBILE that does not exist, but will allow access to the UID
	ENT_OBJECT_ID,		// Will act as ENT_OBJECT that does not exist, but will allow access to the UID
	ENT_TOKEN_ID,		// Will act as ENT_TOKEN that does not exist, but will allow access to the UID
	ENT_SKILLINFO_ID,	// Will act as ENT_SKILLINFO that does not exist, but will allow access to the UIDs
	ENT_CHURCH_ID,
	ENT_AREA_ID,
	ENT_WILDS_ID,

	ENT_CLONE_ROOM,		// Will act as ENT_ROOM that does not exist
	ENT_WILDS_ROOM,
	ENT_CLONE_DOOR,
	ENT_WILDS_DOOR,

	ENT_GAME,			// Used to reference things about the game itself
	ENT_PERSIST,		// Grouping of persistant entities
	ENT_HELP,			// Way of accessing help strings, uses the same construct as ENT_EXTRADESC

	ENT_MAX,
	ENT_UNKNOWN = ENT_MAX+1,
	ENT_PRIMARY = ENT_NONE
};

enum entity_primary_enum {
	ENTITY_ENACTOR = ESCAPE_EXTRA,
	ENTITY_OBJ1,
	ENTITY_OBJ2,
	ENTITY_VICTIM,
	ENTITY_TARGET,
	ENTITY_RANDOM,
	ENTITY_HERE,
	ENTITY_SELF,
	ENTITY_PHRASE,
	ENTITY_TRIGGER,
	ENTITY_PRIOR,
	ENTITY_GAME,
	ENTITY_HELP,
	ENTITY_NULL,
	ENTITY_REGISTER1,
	ENTITY_REGISTER2,
	ENTITY_REGISTER3,
	ENTITY_REGISTER4,
	ENTITY_REGISTER5,
};

enum entity_variable_types_enum {
	ENTITY_VAR_NUM = ESCAPE_EXTRA,
	ENTITY_VAR_STR,
	ENTITY_VAR_MOB,
	ENTITY_VAR_OBJ,
	ENTITY_VAR_ROOM,
	ENTITY_VAR_EXIT,
	ENTITY_VAR_TOKEN,
	ENTITY_VAR_AREA,
	ENTITY_VAR_WILDS,
	ENTITY_VAR_SKILL,
	ENTITY_VAR_SKILLINFO,
	ENTITY_VAR_CONN,
	ENTITY_VAR_AFFECT,
	ENTITY_VAR_CHURCH,

	ENTITY_VAR_BLLIST_ROOM,
	ENTITY_VAR_BLLIST_MOB,
	ENTITY_VAR_BLLIST_OBJ,
	ENTITY_VAR_BLLIST_TOK,
	ENTITY_VAR_BLLIST_EXIT,
	ENTITY_VAR_BLLIST_SKILL,
	ENTITY_VAR_BLLIST_AREA,
	ENTITY_VAR_BLLIST_WILDS,

	ENTITY_VAR_PLLIST_STR,
	ENTITY_VAR_PLLIST_CONN,
	ENTITY_VAR_PLLIST_ROOM,
	ENTITY_VAR_PLLIST_MOB,
	ENTITY_VAR_PLLIST_OBJ,
	ENTITY_VAR_PLLIST_TOK,
	ENTITY_VAR_PLLIST_CHURCH,

};

enum entity_prior_enum {
	ENTITY_PRIOR_MOB = ESCAPE_EXTRA,
	ENTITY_PRIOR_OBJ,
	ENTITY_PRIOR_ROOM,
	ENTITY_PRIOR_TOKEN,
	ENTITY_PRIOR_ENACTOR,
	ENTITY_PRIOR_OBJ1,
	ENTITY_PRIOR_OBJ2,
	ENTITY_PRIOR_VICTIM,
	ENTITY_PRIOR_TARGET,
	ENTITY_PRIOR_RANDOM,
	ENTITY_PRIOR_HERE,
	ENTITY_PRIOR_PHRASE,
	ENTITY_PRIOR_TRIGGER,
	ENTITY_PRIOR_REGISTER1,
	ENTITY_PRIOR_REGISTER2,
	ENTITY_PRIOR_REGISTER3,
	ENTITY_PRIOR_REGISTER4,
	ENTITY_PRIOR_REGISTER5,
	ENTITY_PRIOR_PRIOR
};

enum entity_number_enum {
	ENTITY_NUM_ABS = ESCAPE_EXTRA
};

enum entity_string_enum {
	ENTITY_STR_LEN = ESCAPE_EXTRA,
	ENTITY_STR_LOWER,
	ENTITY_STR_UPPER,
	ENTITY_STR_CAPITAL
};

enum entity_mobile_enum {
	ENTITY_MOB_NAME = ESCAPE_EXTRA,
	ENTITY_MOB_SHORT,
	ENTITY_MOB_LONG,
	ENTITY_MOB_FULLDESC,
	ENTITY_MOB_SEX,
	ENTITY_MOB_GENDER,
	ENTITY_MOB_HE,
	ENTITY_MOB_HIM,
	ENTITY_MOB_HIS,
	ENTITY_MOB_HIS_O,
	ENTITY_MOB_HIMSELF,
	ENTITY_MOB_RACE,
	ENTITY_MOB_ROOM,
	ENTITY_MOB_HOUSE,
	ENTITY_MOB_CARRYING,
	ENTITY_MOB_AFFECTS,
	ENTITY_MOB_TOKENS,
	ENTITY_MOB_MOUNT,
	ENTITY_MOB_RIDER,
	ENTITY_MOB_MASTER,
	ENTITY_MOB_LEADER,
	ENTITY_MOB_OWNER,
	ENTITY_MOB_OPPONENT,
	ENTITY_MOB_CART,
	ENTITY_MOB_FURNITURE,
	ENTITY_MOB_TARGET,
	ENTITY_MOB_HUNTING,
	ENTITY_MOB_AREA,
	ENTITY_MOB_EQ_LIGHT,
	ENTITY_MOB_EQ_FINGER1,
	ENTITY_MOB_EQ_FINGER2,
	ENTITY_MOB_EQ_RING,
	ENTITY_MOB_EQ_NECK1,
	ENTITY_MOB_EQ_NECK2,
	ENTITY_MOB_EQ_BODY,
	ENTITY_MOB_EQ_HEAD,
	ENTITY_MOB_EQ_LEGS,
	ENTITY_MOB_EQ_FEET,
	ENTITY_MOB_EQ_ARMS,
	ENTITY_MOB_EQ_WAIST,
	ENTITY_MOB_EQ_ABOUT,
	ENTITY_MOB_EQ_SHOULDER,
	ENTITY_MOB_EQ_BACK,
	ENTITY_MOB_EQ_WRIST1,
	ENTITY_MOB_EQ_WRIST2,
	ENTITY_MOB_EQ_HANDS,
	ENTITY_MOB_EQ_WIELD1,
	ENTITY_MOB_EQ_WIELD2,
	ENTITY_MOB_EQ_HOLD,
	ENTITY_MOB_EQ_SHIELD,
	ENTITY_MOB_EQ_ANKLE1,
	ENTITY_MOB_EQ_ANKLE2,
	ENTITY_MOB_EQ_EAR1,
	ENTITY_MOB_EQ_EAR2,
	ENTITY_MOB_EQ_EYES,
	ENTITY_MOB_EQ_FACE,
	ENTITY_MOB_EQ_TATTOO_HEAD,
	ENTITY_MOB_EQ_TATTOO_TORSO,
	ENTITY_MOB_EQ_TATTOO_UPPER_ARM1,
	ENTITY_MOB_EQ_TATTOO_UPPER_ARM2,
	ENTITY_MOB_EQ_TATTOO_UPPER_LEG1,
	ENTITY_MOB_EQ_TATTOO_UPPER_LEG2,
        ENTITY_MOB_EQ_TATTOO_LOWER_ARM1,
        ENTITY_MOB_EQ_TATTOO_LOWER_ARM2,
        ENTITY_MOB_EQ_TATTOO_LOWER_LEG1,
        ENTITY_MOB_EQ_TATTOO_LOWER_LEG2,
        ENTITY_MOB_EQ_TATTOO_SHOULDER1,
        ENTITY_MOB_EQ_TATTOO_SHOULDER2,
        ENTITY_MOB_EQ_TATTOO_BACK,
	ENTITY_MOB_EQ_LODGED_HEAD,
	ENTITY_MOB_EQ_LODGED_TORSO,
	ENTITY_MOB_EQ_LODGED_ARM1,
	ENTITY_MOB_EQ_LODGED_ARM2,
	ENTITY_MOB_EQ_LODGED_LEG1,
	ENTITY_MOB_EQ_LODGED_LEG2,
	ENTITY_MOB_EQ_ENTANGLED,
	ENTITY_MOB_EQ_CONCEALED,
	ENTITY_MOB_CASTSPELL,
	ENTITY_MOB_CASTTOKEN,
	ENTITY_MOB_CASTTARGET,
	ENTITY_MOB_RECALL,		/* Targets their current recall location */
	ENTITY_MOB_CONNECTION,
	ENTITY_MOB_CHURCH,
	ENTITY_MOB_CLONEROOMS,
	ENTITY_MOB_SONG,
	ENTITY_MOB_SONGTARGET,
	ENTITY_MOB_SONGTOKEN,
	ENTITY_MOB_INSTRUMENT,
	ENTITY_MOB_WORN,
	ENTITY_MOB_NEXT,
	ENTITY_MOB_CHECKPOINT,
};

enum entity_object_enum {
	ENTITY_OBJ_NAME = ESCAPE_EXTRA,
	ENTITY_OBJ_SHORT,
	ENTITY_OBJ_LONG,
	ENTITY_OBJ_FULLDESC,
	ENTITY_OBJ_CONTAINER,
	ENTITY_OBJ_FURNITURE,
	ENTITY_OBJ_CONTENTS,
	ENTITY_OBJ_OWNER,
	ENTITY_OBJ_ROOM,
	ENTITY_OBJ_CARRIER,
	ENTITY_OBJ_TOKENS,
	ENTITY_OBJ_TARGET,
	ENTITY_OBJ_AREA,
	ENTITY_OBJ_EXTRADESC,
	ENTITY_OBJ_CLONEROOMS,
	ENTITY_OBJ_NEXT,
	ENTITY_OBJ_AFFECTS
};

enum entity_room_enum {
	ENTITY_ROOM_NAME = ESCAPE_EXTRA,
	ENTITY_ROOM_MOBILES,
	ENTITY_ROOM_OBJECTS,
	ENTITY_ROOM_TOKENS,
	ENTITY_ROOM_AREA,
	ENTITY_ROOM_TARGET,
	ENTITY_ROOM_NORTH,
	ENTITY_ROOM_EAST,
	ENTITY_ROOM_SOUTH,
	ENTITY_ROOM_WEST,
	ENTITY_ROOM_UP,
	ENTITY_ROOM_DOWN,
	ENTITY_ROOM_NORTHEAST,
	ENTITY_ROOM_NORTHWEST,
	ENTITY_ROOM_SOUTHEAST,
	ENTITY_ROOM_SOUTHWEST,
	ENTITY_ROOM_ENVIRON,
	ENTITY_ROOM_ENVIRON_MOB,
	ENTITY_ROOM_ENVIRON_OBJ,
	ENTITY_ROOM_ENVIRON_ROOM,
	ENTITY_ROOM_ENVIRON_TOKEN,
	ENTITY_ROOM_EXTRADESC,
	ENTITY_ROOM_DESC,
	ENTITY_ROOM_WILDS,
	ENTITY_ROOM_CLONES,
	ENTITY_ROOM_CLONEROOMS,
};

enum entity_exit_enum {
	ENTITY_EXIT_NAME = ESCAPE_EXTRA,
	ENTITY_EXIT_DOOR,
	ENTITY_EXIT_SOURCE,
	ENTITY_EXIT_REMOTE,
	ENTITY_EXIT_STATE,
	ENTITY_EXIT_MATE,
	ENTITY_EXIT_NORTH,
	ENTITY_EXIT_EAST,
	ENTITY_EXIT_SOUTH,
	ENTITY_EXIT_WEST,
	ENTITY_EXIT_UP,
	ENTITY_EXIT_DOWN,
	ENTITY_EXIT_NORTHEAST,
	ENTITY_EXIT_NORTHWEST,
	ENTITY_EXIT_SOUTHEAST,
	ENTITY_EXIT_SOUTHWEST,
	ENTITY_EXIT_NEXT
};

enum entity_token_enum {
	ENTITY_TOKEN_NAME = ESCAPE_EXTRA,
	ENTITY_TOKEN_OWNER,
	ENTITY_TOKEN_OBJECT,
	ENTITY_TOKEN_ROOM,
	ENTITY_TOKEN_TIMER,
	ENTITY_TOKEN_VAL0,
	ENTITY_TOKEN_VAL1,
	ENTITY_TOKEN_VAL2,
	ENTITY_TOKEN_VAL3,
	ENTITY_TOKEN_VAL4,
	ENTITY_TOKEN_VAL5,
	ENTITY_TOKEN_VAL6,
	ENTITY_TOKEN_VAL7,
	ENTITY_TOKEN_NEXT
};

enum entity_area_enum {
	ENTITY_AREA_NAME = ESCAPE_EXTRA,
	ENTITY_AREA_RECALL,
	ENTITY_AREA_POSTOFFICE,
	ENTITY_AREA_LOWERVNUM,
	ENTITY_AREA_UPPERVNUM,
	ENTITY_AREA_ROOMS,
};

enum entity_wilds_enum {
	ENTITY_WILDS_NAME = ESCAPE_EXTRA,	//**
	ENTITY_WILDS_WIDTH,					//**
	ENTITY_WILDS_HEIGHT,				//**
	ENTITY_WILDS_VROOMS,				//**
	ENTITY_WILDS_VLINKS,				//**
};

enum entity_game_enum {
	ENTITY_GAME_NAME = ESCAPE_EXTRA,
	ENTITY_GAME_PORT,
	ENTITY_GAME_PLAYERS,
	ENTITY_GAME_IMMORTALS,
	ENTITY_GAME_ONLINE,
	ENTITY_GAME_PERSIST,
	ENTITY_GAME_AREAS,
	ENTITY_GAME_WILDS,
	ENTITY_GAME_CHURCHES,
};

enum entity_persist_enum {
	ENTITY_PERSIST_MOBS = ESCAPE_EXTRA,
	ENTITY_PERSIST_OBJS,
	ENTITY_PERSIST_ROOMS,
};

enum entity_church_enum {
	ENTITY_CHURCH_NAME = ESCAPE_EXTRA,
	ENTITY_CHURCH_SIZE,
	ENTITY_CHURCH_FLAG,
	ENTITY_CHURCH_FOUNDER,
	ENTITY_CHURCH_FOUNDER_NAME,
	ENTITY_CHURCH_MOTD,
	ENTITY_CHURCH_RULES,
	ENTITY_CHURCH_INFO,
	ENTITY_CHURCH_RECALL,
	ENTITY_CHURCH_TREASURE,
	ENTITY_CHURCH_KEY,
	ENTITY_CHURCH_ONLINE,
	ENTITY_CHURCH_ROSTER,
};


enum entity_conn_enum {
	ENTITY_CONN_PLAYER = ESCAPE_EXTRA,
	ENTITY_CONN_ORIGINAL,
	ENTITY_CONN_HOST,
	ENTITY_CONN_CONNECTION,
	ENTITY_CONN_SNOOPER,
};

enum entity_list_enum {
	ENTITY_LIST_SIZE = ESCAPE_EXTRA,
	ENTITY_LIST_RANDOM,
	ENTITY_LIST_FIRST,
	ENTITY_LIST_LAST
};

enum entity_blist_enum {
	ENTITY_BLLIST_SIZE = ESCAPE_EXTRA,
	ENTITY_BLLIST_RANDOM,
	ENTITY_BLLIST_FIRST,
	ENTITY_BLLIST_LAST
};

enum entity_skill_enum {
	ENTITY_SKILL_GSN = ESCAPE_EXTRA,
	ENTITY_SKILL_SPELL,
	ENTITY_SKILL_NAME,
	ENTITY_SKILL_BEATS,
	ENTITY_SKILL_LEVEL_WARRIOR,
	ENTITY_SKILL_LEVEL_CLERIC,
	ENTITY_SKILL_LEVEL_MAGE,
	ENTITY_SKILL_LEVEL_THIEF,
	ENTITY_SKILL_DIFFICULTY_WARRIOR,
	ENTITY_SKILL_DIFFICULTY_CLERIC,
	ENTITY_SKILL_DIFFICULTY_MAGE,
	ENTITY_SKILL_DIFFICULTY_THIEF,
	ENTITY_SKILL_TARGET,
	ENTITY_SKILL_POSITION,
	ENTITY_SKILL_WEAROFF,
	ENTITY_SKILL_DISPEL,
	ENTITY_SKILL_NOUN,
	ENTITY_SKILL_MANA,
	ENTITY_SKILL_INK_TYPE1,
	ENTITY_SKILL_INK_SIZE1,
	ENTITY_SKILL_INK_TYPE2,
	ENTITY_SKILL_INK_SIZE2,
	ENTITY_SKILL_INK_TYPE3,
	ENTITY_SKILL_INK_SIZE3
};

enum entity_skillinfo_enum {
	ENTITY_SKILLINFO_SKILL = ESCAPE_EXTRA,
	ENTITY_SKILLINFO_OWNER,
	ENTITY_SKILLINFO_TOKEN,
	ENTITY_SKILLINFO_RATING
};

enum entity_affect_enum {
	ENTITY_AFFECT_NAME = ESCAPE_EXTRA,
	ENTITY_AFFECT_GROUP,
	ENTITY_AFFECT_SKILL,
	ENTITY_AFFECT_LOCATION,
	ENTITY_AFFECT_MOD,
	ENTITY_AFFECT_TIMER,
	ENTITY_AFFECT_LEVEL
};

enum entity_song_enum {
	ENTITY_SONG_NUMBER = ESCAPE_EXTRA,
	ENTITY_SONG_NAME,
	ENTITY_SONG_SPELL1,
	ENTITY_SONG_SPELL2,
	ENTITY_SONG_SPELL3,
	ENTITY_SONG_TARGET,
	ENTITY_SONG_BEATS,
	ENTITY_SONG_MANA,
	ENTITY_SONG_LEVEL,
};


/* Single letter $* codes ($i, $n) */
#define ESCAPE_UA		0x80
#define ESCAPE_UB		0x81
#define ESCAPE_UC		0x82
#define ESCAPE_UD		0x83
#define ESCAPE_UE		0x84
#define ESCAPE_UF		0x85
#define ESCAPE_UG		0x86
#define ESCAPE_UH		0x87
#define ESCAPE_UI		0x88
#define ESCAPE_UJ		0x89
#define ESCAPE_UK		0x8A
#define ESCAPE_UL		0x8B
#define ESCAPE_UM		0x8C
#define ESCAPE_UN		0x8D
#define ESCAPE_UO		0x8E
#define ESCAPE_UP		0x8F
#define ESCAPE_UQ		0x90
#define ESCAPE_UR		0x91
#define ESCAPE_US		0x92
#define ESCAPE_UT		0x93
#define ESCAPE_UU		0x94
#define ESCAPE_UV		0x95
#define ESCAPE_UW		0x96
#define ESCAPE_UX		0x97
#define ESCAPE_UY		0x98
#define ESCAPE_UZ		0x99
#define ESCAPE_LA		0xA0
#define ESCAPE_LB		0xA1
#define ESCAPE_LC		0xA2
#define ESCAPE_LD		0xA3
#define ESCAPE_LE		0xA4
#define ESCAPE_LF		0xA5
#define ESCAPE_LG		0xA6
#define ESCAPE_LH		0xA7
#define ESCAPE_LI		0xA8
#define ESCAPE_LJ		0xA9
#define ESCAPE_LK		0xAA
#define ESCAPE_LL		0xAB
#define ESCAPE_LM		0xAC
#define ESCAPE_LN		0xAD
#define ESCAPE_LO		0xAE
#define ESCAPE_LP		0xAF
#define ESCAPE_LQ		0xB0
#define ESCAPE_LR		0xB1
#define ESCAPE_LS		0xB2
#define ESCAPE_LT		0xB3
#define ESCAPE_LU		0xB4
#define ESCAPE_LV		0xB5
#define ESCAPE_LW		0xB6
#define ESCAPE_LX		0xB7
#define ESCAPE_LY		0xB8
#define ESCAPE_LZ		0xB9

#define SOMEONE "someone"
#define SOMETHING "something"
#define SOMEONES "someone's"
#define SOMEWHERE "somewhere"

/* RETURN CODE predefines */
#define PRET_EXECUTED		(1)	/* DEFAULT: Though, you can return any non-zero value */
#define PRET_NOSCRIPT		(0)
#define PRET_BADSYNTAX		(-1)	/* Syntax error */
#define PRET_BADVNUM		(-2)	/* Invalid VNUM (script or otherwise) */
#define PRET_BADTYPE		(-3)	/* Invalid script type */
#define PRET_CALLDEPTH		(-4)	/* Too deep on call depth */

/* Macros of course */
#define PRETURN	return ret_val
#define PRETNOERRYES	return ((ret_val < 0) ? PRET_EXECUTED : ret_val)
#define PRETNOERRNO	return ((ret_val < 0) ? PRET_NOSCRIPT : ret_val)
#define SETPRET		do { if(ret != PRET_NOSCRIPT) ret_val = ret; } while(0)
#define SETPRETX	do { if(ret != PRET_NOSCRIPT) { ret_val = ret; return ret_val; } } while(0)
#define ISSETPRET	if( ret_val != PRET_NOSCRIPT ) return ret_val
#define BREAKPRET	if( ret_val != PRET_NOSCRIPT ) break

#define EVAL_EQ            0
#define EVAL_GE            1
#define EVAL_LE            2
#define EVAL_GT            3
#define EVAL_LT            4
#define EVAL_NE            5
#define EVAL_MASK          6

#define MAX_NESTED_LEVEL	24 /* Maximum nested if-else-endif's (stack size) */
#define BEGIN_BLOCK		0 /* Flag: Begin of if-else-endif block */
#define IN_BLOCK		-1 /* Flag: Executable statements */
#define END_BLOCK		-2 /* Flag: End of if-else-endif block */
#define IN_FOR			-3 /* Flag: Executable statements */
#define IN_LIST			-4 /* Flag: Executable statements */
#define IN_WHILE		-5 /* Flag: Begin of if-else-endif block */
#define MAX_CALL_LEVEL		15 /* Maximum nested calls */

struct script_var_type {
	pVARIABLE next;
	pVARIABLE global_prev;
	pVARIABLE global_next;
	char *name;
	bool save;
	bool index;
	int type;
	union {
		void *raw;
		int i;
		char *s;
		ROOM_INDEX_DATA *r;
		CHAR_DATA *m;
		OBJ_DATA *o;
		TOKEN_DATA *t;
		AREA_DATA *a;
		AFFECT_DATA *aff;
		DESCRIPTOR_DATA *conn;
		CHURCH_DATA *church;
		WILDS_DATA *wilds;
		int sn;
		int song;
		struct {
			CHAR_DATA *owner;
			TOKEN_DATA *token;
			int sn;
		} sk;

		// Only used when on persistant entities
		long aid;		// area UID
		long chid;		// Church UID
		long wid;		// wilds UID
		struct {
			unsigned long a;
			unsigned long b;
		} mid;
		struct {
			unsigned long a;
			unsigned long b;
		} oid;
		struct {
			unsigned long a;
			unsigned long b;
		} tid;
		struct {
			ROOM_INDEX_DATA *r;
			unsigned long a;
			unsigned long b;
		} cr;	// Clone Room
		struct {
			int sn;
			unsigned long mid[2];
			unsigned long tid[2];
		} skid;
		struct {	// Used by VAR_EXIT
			ROOM_INDEX_DATA *r;
			int door;
		} door;
		struct {
			ROOM_INDEX_DATA *r;
			unsigned long a;
			unsigned long b;
			int door;
		} cdoor;
		struct {
			unsigned long wuid;
			int x;
			int y;
		} wroom;
		struct {
			unsigned long wuid;
			int x;
			int y;
			int door;
		} wdoor;
		LLIST *list;	// Used for HOMOGENOUS lists only
	} _;
};


/* Defines the information used by an ifcheck */
struct ifcheck_data {
	char *name;			/* name of the ifcheck */
	int type;			/* prog types the ifcheck is usable in */
	char *params;			/* parameter types */
	bool numeric;			/* Return type */
	IFC_FUNC func;			/* Processing function */
	char *help;			/* Help keywords (for accessing the help file) */
};

struct script_code {
	unsigned char opcode;
	unsigned char level;
	short param;
	short length;
	int label;
	char *rest;
};

struct loop_data {
	int id;
	union {
		struct {	/* For loops */
			int cur;
			int end;
			int inc;
		} f;
		struct {	/* List loops */
			int type;
			union {
				CHAR_DATA *m;
				OBJ_DATA *o;
				TOKEN_DATA *t;
				int door;
				AFFECT_DATA *aff;
				void *raw;
				char *str;
			} cur, next;
			struct {
				LLIST *lp;
				ITERATOR it;
			} list;
			void *owner;
		} l;
	} d;
	char var_name[MIL];
	int counter;
	char buf[MSL];
};

/* Running information used in execution */
struct script_control_block {
	SCRIPT_CB *next;
	SCRIPT_DATA *script;		/* Current program being executed */
	SCRIPT_CODE *cur_line;
	int type;			/* Type of program being called */
	int line;			/* Current line of execution */
	int ret_val;
	int level;
	int loop;
	long flags;
	int state[MAX_NESTED_LEVEL];
	bool cond[MAX_NESTED_LEVEL];
	struct loop_data loops[MAX_NESTED_LOOPS];
	SCRIPT_VARINFO info;
};

struct script_parameter {
	int type;
	union {
		int num;
		char *str;
		CHAR_DATA *mob;
		OBJ_DATA *obj;
		ROOM_INDEX_DATA *room;
		AREA_DATA *area;
		TOKEN_DATA *token;
		EXTRA_DESCR_DATA *ed;
		AFFECT_DATA *aff;
		DESCRIPTOR_DATA *conn;
		WILDS_DATA *wilds;
		CHURCH_DATA *church;
		int sn;
		int song;
		struct {
			union {
				CHAR_DATA **mob;
				OBJ_DATA **obj;
				TOKEN_DATA **tok;
				AFFECT_DATA **aff;
			} ptr;
			void *owner;
		} list;
		struct {
			ROOM_INDEX_DATA *r;
			unsigned long a;
			unsigned long b;
		} cr;	// Clone Room
		struct {
			CHAR_DATA *m;
			TOKEN_DATA *t;
			int sn;
			unsigned long mid[2];
			unsigned long tid[2];
		} sk;
		struct {	// Used by VAR_EXIT
			ROOM_INDEX_DATA *r;
			int door;
		} door;
		struct {
			ROOM_INDEX_DATA *r;
			unsigned long a;
			unsigned long b;
			int door;
		} cdoor;
		struct {
			unsigned long wuid;
			int x;
			int y;
		} wroom;
		struct {
			unsigned long wuid;
			int x;
			int y;
			int door;
		} wdoor;

		LLIST *blist;
		long aid;
		long chid;
		long wid;
		unsigned long uid[2];
		SCRIPT_VARINFO *info;
	} d;
	char buf[MSL];
};

struct script_cmd_type {
	char *name;
	void (*func) (SCRIPT_VARINFO *info, char *argument);
	bool restricted;
};

struct _entity_type_info {
	int type_min;
	int type_max;
	ENT_FIELD *fields;
	bool allow_vars;
};

/* Externs */
extern char *ifcheck_param_type_names[IFCP_MAX];
extern char *opcode_names[OP_LASTCODE];
extern struct trigger_type trigger_table[];
extern int trigger_slots[];
extern IFCHECK_DATA ifcheck_table[];
extern OPCODE_FUNC opcode_table[OP_LASTCODE];
extern char *script_operators[];
extern int script_expression_stack_action[CH_MAX][STK_MAX];
extern int script_expression_tostack[CH_MAX+1];
extern int script_expression_argstack[STK_MAX+1];
extern const char *male_female[];
extern const char *he_she[];
extern const char *him_her[];
extern const char *his_hers[];
extern const char *his_her[];
extern const char *himself[];
extern const char *exit_states[];
extern const struct script_cmd_type mob_cmd_table[];
extern const struct script_cmd_type obj_cmd_table[];
extern const struct script_cmd_type room_cmd_table[];
extern const struct script_cmd_type token_cmd_table[];
extern const struct script_cmd_type tokenother_cmd_table[];
extern ENT_FIELD entity_primary[];
extern ENT_FIELD entity_types[];
extern ENT_FIELD entity_number[];
extern ENT_FIELD entity_string[];
extern ENT_FIELD entity_mobile[];
extern ENT_FIELD entity_object[];
extern ENT_FIELD entity_room[];
extern ENT_FIELD entity_exit[];
extern ENT_FIELD entity_token[];
extern ENT_FIELD entity_area[];
extern ENT_FIELD entity_list[];
extern ENT_FIELD *entity_type_lists[];
extern ENT_FIELD entity_skill_info[];
extern ENT_FIELD entity_skill[];
extern ENT_FIELD entity_conn[];
extern ENT_FIELD entity_prior[];
extern bool entity_allow_vars[];
extern bool forced_command;
extern int trigger_table_size;
//extern int trigger_slots_size;
extern ROOM_INDEX_DATA room_used_for_wilderness;
extern ROOM_INDEX_DATA room_pointer_vlink;
extern ROOM_INDEX_DATA room_pointer_environment;
extern struct _entity_type_info entity_type_info[];
extern bool script_destructed;

/* Ifchecks */
DECL_IFC_FUN(ifc_abs);
DECL_IFC_FUN(ifc_act);
DECL_IFC_FUN(ifc_act2);
DECL_IFC_FUN(ifc_affected);
DECL_IFC_FUN(ifc_affected2);
DECL_IFC_FUN(ifc_affectedname);
DECL_IFC_FUN(ifc_affectedspell);
DECL_IFC_FUN(ifc_age);
DECL_IFC_FUN(ifc_align);
DECL_IFC_FUN(ifc_angle);
DECL_IFC_FUN(ifc_areahasland);
DECL_IFC_FUN(ifc_areaid);
DECL_IFC_FUN(ifc_arealandx);
DECL_IFC_FUN(ifc_arealandy);
DECL_IFC_FUN(ifc_areax);
DECL_IFC_FUN(ifc_areay);
DECL_IFC_FUN(ifc_arenafights);
DECL_IFC_FUN(ifc_arenaloss);
DECL_IFC_FUN(ifc_arenaratio);
DECL_IFC_FUN(ifc_arenawins);
DECL_IFC_FUN(ifc_bankbalance);
DECL_IFC_FUN(ifc_bit);
DECL_IFC_FUN(ifc_candrop);
DECL_IFC_FUN(ifc_canget);
DECL_IFC_FUN(ifc_canhunt);
DECL_IFC_FUN(ifc_canpractice);
DECL_IFC_FUN(ifc_canput);
DECL_IFC_FUN(ifc_canscare);
DECL_IFC_FUN(ifc_carriedby);
DECL_IFC_FUN(ifc_carries);
DECL_IFC_FUN(ifc_carryleft);
DECL_IFC_FUN(ifc_church);
DECL_IFC_FUN(ifc_churchonline);
DECL_IFC_FUN(ifc_churchrank);
DECL_IFC_FUN(ifc_clan);
DECL_IFC_FUN(ifc_class);
DECL_IFC_FUN(ifc_clones);
DECL_IFC_FUN(ifc_container);
DECL_IFC_FUN(ifc_cos);
DECL_IFC_FUN(ifc_cpkfights);
DECL_IFC_FUN(ifc_cpkloss);
DECL_IFC_FUN(ifc_cpkratio);
DECL_IFC_FUN(ifc_cpkwins);
DECL_IFC_FUN(ifc_curhit);
DECL_IFC_FUN(ifc_curmana);
DECL_IFC_FUN(ifc_curmove);
DECL_IFC_FUN(ifc_damtype);
DECL_IFC_FUN(ifc_danger);
DECL_IFC_FUN(ifc_day);
DECL_IFC_FUN(ifc_death);
DECL_IFC_FUN(ifc_deathcount);
DECL_IFC_FUN(ifc_deity);
DECL_IFC_FUN(ifc_dice);
DECL_IFC_FUN(ifc_drunk);
DECL_IFC_FUN(ifc_exists);
DECL_IFC_FUN(ifc_exitexists);
DECL_IFC_FUN(ifc_exitflag);
DECL_IFC_FUN(ifc_findpath);
DECL_IFC_FUN(ifc_flag_act);
DECL_IFC_FUN(ifc_flag_act2);
DECL_IFC_FUN(ifc_flag_affect);
DECL_IFC_FUN(ifc_flag_affect2);
DECL_IFC_FUN(ifc_flag_container);
DECL_IFC_FUN(ifc_flag_exit);
DECL_IFC_FUN(ifc_flag_extra);
DECL_IFC_FUN(ifc_flag_extra2);
DECL_IFC_FUN(ifc_flag_extra3);
DECL_IFC_FUN(ifc_flag_extra4);
DECL_IFC_FUN(ifc_flag_form);
DECL_IFC_FUN(ifc_flag_furniture);
DECL_IFC_FUN(ifc_flag_imm);
DECL_IFC_FUN(ifc_flag_interrupt);
DECL_IFC_FUN(ifc_flag_off);
DECL_IFC_FUN(ifc_flag_part);
DECL_IFC_FUN(ifc_flag_portal);
DECL_IFC_FUN(ifc_flag_res);
DECL_IFC_FUN(ifc_flag_room);
DECL_IFC_FUN(ifc_flag_room2);
DECL_IFC_FUN(ifc_flag_vuln);
DECL_IFC_FUN(ifc_flag_weapon);
DECL_IFC_FUN(ifc_flag_wear);
DECL_IFC_FUN(ifc_fullness);
DECL_IFC_FUN(ifc_furniture);
DECL_IFC_FUN(ifc_gold);
DECL_IFC_FUN(ifc_groundweight);
DECL_IFC_FUN(ifc_groupcon);
DECL_IFC_FUN(ifc_groupdex);
DECL_IFC_FUN(ifc_grouphit);
DECL_IFC_FUN(ifc_groupint);
DECL_IFC_FUN(ifc_groupmana);
DECL_IFC_FUN(ifc_groupmaxhit);
DECL_IFC_FUN(ifc_groupmaxmana);
DECL_IFC_FUN(ifc_groupmaxmove);
DECL_IFC_FUN(ifc_groupmove);
DECL_IFC_FUN(ifc_groupstr);
DECL_IFC_FUN(ifc_groupwis);
DECL_IFC_FUN(ifc_grpsize);
DECL_IFC_FUN(ifc_handsfull);
DECL_IFC_FUN(ifc_has);
DECL_IFC_FUN(ifc_hascatalyst);
DECL_IFC_FUN(ifc_hascheckpoint);
DECL_IFC_FUN(ifc_hasenvironment);
DECL_IFC_FUN(ifc_hasprompt);
DECL_IFC_FUN(ifc_hasqueue);
DECL_IFC_FUN(ifc_hasship);
DECL_IFC_FUN(ifc_hassubclass);
DECL_IFC_FUN(ifc_hastarget);
DECL_IFC_FUN(ifc_hastoken);
DECL_IFC_FUN(ifc_hasvlink);
DECL_IFC_FUN(ifc_healregen);
DECL_IFC_FUN(ifc_hitdamage);
DECL_IFC_FUN(ifc_hitdamclass);
DECL_IFC_FUN(ifc_hitdamtype);
DECL_IFC_FUN(ifc_hitskilltype);
DECL_IFC_FUN(ifc_hour);
DECL_IFC_FUN(ifc_hpcnt);
DECL_IFC_FUN(ifc_hunger);
DECL_IFC_FUN(ifc_id);
DECL_IFC_FUN(ifc_id2);
DECL_IFC_FUN(ifc_identical);
DECL_IFC_FUN(ifc_imm);
DECL_IFC_FUN(ifc_innature);
DECL_IFC_FUN(ifc_inputwait);
DECL_IFC_FUN(ifc_inwilds);
DECL_IFC_FUN(ifc_isactive);
DECL_IFC_FUN(ifc_isambushing);
DECL_IFC_FUN(ifc_isangel);
DECL_IFC_FUN(ifc_isbrewing);
DECL_IFC_FUN(ifc_isbusy);
DECL_IFC_FUN(ifc_iscasting);
DECL_IFC_FUN(ifc_ischarm);
DECL_IFC_FUN(ifc_ischurchexcom);
DECL_IFC_FUN(ifc_iscloneroom);
DECL_IFC_FUN(ifc_iscpkproof);
DECL_IFC_FUN(ifc_isdead);
DECL_IFC_FUN(ifc_isdelay);
DECL_IFC_FUN(ifc_isdemon);
DECL_IFC_FUN(ifc_isevil);
DECL_IFC_FUN(ifc_isfading);
DECL_IFC_FUN(ifc_isfighting);
DECL_IFC_FUN(ifc_isflying);
DECL_IFC_FUN(ifc_isfollow);
DECL_IFC_FUN(ifc_isgood);
DECL_IFC_FUN(ifc_ishunting);
DECL_IFC_FUN(ifc_isimmort);
DECL_IFC_FUN(ifc_iskey);
DECL_IFC_FUN(ifc_isleader);
DECL_IFC_FUN(ifc_ismoonup);
DECL_IFC_FUN(ifc_ismorphed);
DECL_IFC_FUN(ifc_ismystic);
DECL_IFC_FUN(ifc_isneutral);
DECL_IFC_FUN(ifc_isnpc);
DECL_IFC_FUN(ifc_ison);
DECL_IFC_FUN(ifc_ispc);
DECL_IFC_FUN(ifc_isprey);
DECL_IFC_FUN(ifc_ispulling);
DECL_IFC_FUN(ifc_ispullingrelic);
DECL_IFC_FUN(ifc_isquesting);
DECL_IFC_FUN(ifc_isremort);
DECL_IFC_FUN(ifc_isrepairable);
DECL_IFC_FUN(ifc_isrestrung);
DECL_IFC_FUN(ifc_isridden);
DECL_IFC_FUN(ifc_isrider);
DECL_IFC_FUN(ifc_isriding);
DECL_IFC_FUN(ifc_isroomdark);
DECL_IFC_FUN(ifc_issafe);
DECL_IFC_FUN(ifc_isscribing);
DECL_IFC_FUN(ifc_isshifted);
DECL_IFC_FUN(ifc_isshooting);
DECL_IFC_FUN(ifc_isshopkeeper);
DECL_IFC_FUN(ifc_isspell);
DECL_IFC_FUN(ifc_isspell);
DECL_IFC_FUN(ifc_issubclass);
DECL_IFC_FUN(ifc_issustained);
DECL_IFC_FUN(ifc_istarget);
DECL_IFC_FUN(ifc_istattooing);
DECL_IFC_FUN(ifc_isvisible);
DECL_IFC_FUN(ifc_isvisibleto);
DECL_IFC_FUN(ifc_isworn);
DECL_IFC_FUN(ifc_lastreturn);
DECL_IFC_FUN(ifc_level);
DECL_IFC_FUN(ifc_liquid);
DECL_IFC_FUN(ifc_lostparts);
DECL_IFC_FUN(ifc_manaregen);
DECL_IFC_FUN(ifc_manastore);
DECL_IFC_FUN(ifc_maparea);
DECL_IFC_FUN(ifc_mapheight);
DECL_IFC_FUN(ifc_mapid);
DECL_IFC_FUN(ifc_mapvalid);
DECL_IFC_FUN(ifc_mapwidth);
DECL_IFC_FUN(ifc_mapx);
DECL_IFC_FUN(ifc_mapy);
DECL_IFC_FUN(ifc_material);
DECL_IFC_FUN(ifc_max);
DECL_IFC_FUN(ifc_maxcarry);
DECL_IFC_FUN(ifc_maxhit);
DECL_IFC_FUN(ifc_maxmana);
DECL_IFC_FUN(ifc_maxmove);
DECL_IFC_FUN(ifc_maxweight);
DECL_IFC_FUN(ifc_maxxp);
DECL_IFC_FUN(ifc_min);
DECL_IFC_FUN(ifc_mobexists);
DECL_IFC_FUN(ifc_mobhere);
DECL_IFC_FUN(ifc_mobs);
DECL_IFC_FUN(ifc_mobsize);
DECL_IFC_FUN(ifc_money);
DECL_IFC_FUN(ifc_monkills);
DECL_IFC_FUN(ifc_month);
DECL_IFC_FUN(ifc_moonphase);
DECL_IFC_FUN(ifc_moveregen);
DECL_IFC_FUN(ifc_name);
DECL_IFC_FUN(ifc_numenchants);
DECL_IFC_FUN(ifc_objcond);
DECL_IFC_FUN(ifc_objcost);
DECL_IFC_FUN(ifc_objexists);
DECL_IFC_FUN(ifc_objextra);
DECL_IFC_FUN(ifc_objextra2);
DECL_IFC_FUN(ifc_objextra3);
DECL_IFC_FUN(ifc_objextra4);
DECL_IFC_FUN(ifc_objfrag);
DECL_IFC_FUN(ifc_objhere);
DECL_IFC_FUN(ifc_objmaxweight);
DECL_IFC_FUN(ifc_objtimer);
DECL_IFC_FUN(ifc_objtype);
DECL_IFC_FUN(ifc_objval0);
DECL_IFC_FUN(ifc_objval1);
DECL_IFC_FUN(ifc_objval2);
DECL_IFC_FUN(ifc_objval3);
DECL_IFC_FUN(ifc_objval4);
DECL_IFC_FUN(ifc_objval5);
DECL_IFC_FUN(ifc_objval6);
DECL_IFC_FUN(ifc_objval7);
DECL_IFC_FUN(ifc_objwear);
DECL_IFC_FUN(ifc_objwearloc);
DECL_IFC_FUN(ifc_objweight);
DECL_IFC_FUN(ifc_objweightleft);
DECL_IFC_FUN(ifc_off);
DECL_IFC_FUN(ifc_order);
DECL_IFC_FUN(ifc_parts);
DECL_IFC_FUN(ifc_people);
DECL_IFC_FUN(ifc_permcon);
DECL_IFC_FUN(ifc_permdex);
DECL_IFC_FUN(ifc_permint);
DECL_IFC_FUN(ifc_permstr);
DECL_IFC_FUN(ifc_permwis);
DECL_IFC_FUN(ifc_pgroupcon);
DECL_IFC_FUN(ifc_pgroupdex);
DECL_IFC_FUN(ifc_pgroupint);
DECL_IFC_FUN(ifc_pgroupstr);
DECL_IFC_FUN(ifc_pgroupwis);
DECL_IFC_FUN(ifc_pkfights);
DECL_IFC_FUN(ifc_pkloss);
DECL_IFC_FUN(ifc_pkratio);
DECL_IFC_FUN(ifc_pkwins);
DECL_IFC_FUN(ifc_players);
DECL_IFC_FUN(ifc_pneuma);
DECL_IFC_FUN(ifc_portal);
DECL_IFC_FUN(ifc_portalexit);
DECL_IFC_FUN(ifc_pos);
DECL_IFC_FUN(ifc_practices);
DECL_IFC_FUN(ifc_quest);
DECL_IFC_FUN(ifc_race);
DECL_IFC_FUN(ifc_rand);
DECL_IFC_FUN(ifc_randpoint);
DECL_IFC_FUN(ifc_reckoning);
DECL_IFC_FUN(ifc_reckoningchance);
DECL_IFC_FUN(ifc_res);
DECL_IFC_FUN(ifc_room);
DECL_IFC_FUN(ifc_roomflag);
DECL_IFC_FUN(ifc_roomflag2);
DECL_IFC_FUN(ifc_roomviewwilds);
DECL_IFC_FUN(ifc_roomweight);
DECL_IFC_FUN(ifc_roomwilds);
DECL_IFC_FUN(ifc_roomx);
DECL_IFC_FUN(ifc_roomy);
DECL_IFC_FUN(ifc_roomz);
DECL_IFC_FUN(ifc_samegroup);
DECL_IFC_FUN(ifc_scriptsecurity);
DECL_IFC_FUN(ifc_sector);
DECL_IFC_FUN(ifc_sex);
DECL_IFC_FUN(ifc_sign);
DECL_IFC_FUN(ifc_silver);
DECL_IFC_FUN(ifc_sin);
DECL_IFC_FUN(ifc_skeyword);
DECL_IFC_FUN(ifc_skill);
DECL_IFC_FUN(ifc_statcon);
DECL_IFC_FUN(ifc_statdex);
DECL_IFC_FUN(ifc_statint);
DECL_IFC_FUN(ifc_statstr);
DECL_IFC_FUN(ifc_statwis);
DECL_IFC_FUN(ifc_strlen);
DECL_IFC_FUN(ifc_sublevel);
DECL_IFC_FUN(ifc_sunlight);
DECL_IFC_FUN(ifc_systemtime);
DECL_IFC_FUN(ifc_tempstore1);
DECL_IFC_FUN(ifc_tempstore2);
DECL_IFC_FUN(ifc_tempstore3);
DECL_IFC_FUN(ifc_tempstore4);
DECL_IFC_FUN(ifc_testhardmagic);
DECL_IFC_FUN(ifc_testskill);
DECL_IFC_FUN(ifc_testslowmagic);
DECL_IFC_FUN(ifc_testtokenspell);
DECL_IFC_FUN(ifc_thirst);
DECL_IFC_FUN(ifc_timeofday);
DECL_IFC_FUN(ifc_timer);
DECL_IFC_FUN(ifc_tokencount);
DECL_IFC_FUN(ifc_tokenexists);
DECL_IFC_FUN(ifc_tokentype);
DECL_IFC_FUN(ifc_tokentimer);
DECL_IFC_FUN(ifc_tokenvalue);
DECL_IFC_FUN(ifc_totalfights);
DECL_IFC_FUN(ifc_totalloss);
DECL_IFC_FUN(ifc_totalpkfights);
DECL_IFC_FUN(ifc_totalpkloss);
DECL_IFC_FUN(ifc_totalpkratio);
DECL_IFC_FUN(ifc_totalpkwins);
DECL_IFC_FUN(ifc_totalquests);
DECL_IFC_FUN(ifc_totalratio);
DECL_IFC_FUN(ifc_totalwins);
DECL_IFC_FUN(ifc_toxin);
DECL_IFC_FUN(ifc_trains);
DECL_IFC_FUN(ifc_uses);
DECL_IFC_FUN(ifc_value_ac);
DECL_IFC_FUN(ifc_value_acstr);
DECL_IFC_FUN(ifc_value_damage);
DECL_IFC_FUN(ifc_value_position);
DECL_IFC_FUN(ifc_value_ranged);
DECL_IFC_FUN(ifc_value_relic);
DECL_IFC_FUN(ifc_value_sector);
DECL_IFC_FUN(ifc_value_size);
DECL_IFC_FUN(ifc_value_toxin);
DECL_IFC_FUN(ifc_value_type);
DECL_IFC_FUN(ifc_value_weapon);
DECL_IFC_FUN(ifc_value_wear);
DECL_IFC_FUN(ifc_vardefined);
DECL_IFC_FUN(ifc_varexit);
DECL_IFC_FUN(ifc_varnumber);
DECL_IFC_FUN(ifc_varstring);
DECL_IFC_FUN(ifc_vnum);
DECL_IFC_FUN(ifc_vuln);
DECL_IFC_FUN(ifc_weapon);
DECL_IFC_FUN(ifc_weaponskill);
DECL_IFC_FUN(ifc_weapontype);
DECL_IFC_FUN(ifc_wears);
DECL_IFC_FUN(ifc_wearused);
DECL_IFC_FUN(ifc_weight);
DECL_IFC_FUN(ifc_weightleft);
DECL_IFC_FUN(ifc_wimpy);
DECL_IFC_FUN(ifc_word);
DECL_IFC_FUN(ifc_wornby);
DECL_IFC_FUN(ifc_xp);
DECL_IFC_FUN(ifc_year);

DECL_IFC_FUN(ifc_istreasureroom);	// if istreasureroom[ $<mobile|church|name>] $<room|vnum>
DECL_IFC_FUN(ifc_churchhasrelic);	// if churchhasrelic $<mobile|church|name> $<string=relic>
DECL_IFC_FUN(ifc_ischurchpk);	// if ischurchpk $<mobile|church|name>
DECL_IFC_FUN(ifc_iscrosszone);	// if iscrosszone $<mobile|church|name>
DECL_IFC_FUN(ifc_inchurch);	// if isinchurch $<mobile|church|name> $<mobile|name>
DECL_IFC_FUN(ifc_ispersist);	// if ispersist $<mobile|object|room>
DECL_IFC_FUN(ifc_listcontains);	// if listcontains $<list> $<element>
DECL_IFC_FUN(ifc_ispk);			// if ispk $<player>
								// if ispk $<room>[ <boolean=arena>]

DECL_IFC_FUN(ifc_register);		// if register <number> <op> <value>
DECL_IFC_FUN(ifc_comm);			// if comm $<player> <flags>
DECL_IFC_FUN(ifc_flag_comm);		// if flagcomm <flags> == <value>
DECL_IFC_FUN(ifc_stoned);		// if stoned $<player> == <value>

DECL_IFC_FUN(ifc_objcorpse);
DECL_IFC_FUN(ifc_flag_corpse);
DECL_IFC_FUN(ifc_objweapon);
DECL_IFC_FUN(ifc_objweaponstat);
DECL_IFC_FUN(ifc_objranged);

DECL_IFC_FUN(ifc_iscastsuccess);
DECL_IFC_FUN(ifc_iscastfailure);
DECL_IFC_FUN(ifc_iscastroomblocked);
DECL_IFC_FUN(ifc_iscastrecovered);
DECL_IFC_FUN(ifc_churchsize);

DECL_IFC_FUN(ifc_isaffectcustom);
DECL_IFC_FUN(ifc_affectskill);
DECL_IFC_FUN(ifc_isaffectskill);
DECL_IFC_FUN(ifc_affectlocation);
DECL_IFC_FUN(ifc_affectmodifier);
DECL_IFC_FUN(ifc_affecttimer);
DECL_IFC_FUN(ifc_affectgroup);
DECL_IFC_FUN(ifc_isaffectgroup);
DECL_IFC_FUN(ifc_affectbit);
DECL_IFC_FUN(ifc_affectbit2);
DECL_IFC_FUN(ifc_isaffectwhere);


DECL_IFC_FUN(ifc_hasspell);
DECL_IFC_FUN(ifc_playerexists);


/* Opcode functions */
DECL_OPC_FUN(opc_end);
DECL_OPC_FUN(opc_if);
DECL_OPC_FUN(opc_ifnot);
DECL_OPC_FUN(opc_or);
DECL_OPC_FUN(opc_nor);
DECL_OPC_FUN(opc_and);
DECL_OPC_FUN(opc_nand);
DECL_OPC_FUN(opc_else);
DECL_OPC_FUN(opc_endif);
DECL_OPC_FUN(opc_command);
DECL_OPC_FUN(opc_gotoline);
DECL_OPC_FUN(opc_for);
DECL_OPC_FUN(opc_endfor);
DECL_OPC_FUN(opc_exitfor);
DECL_OPC_FUN(opc_list);
DECL_OPC_FUN(opc_endlist);
DECL_OPC_FUN(opc_exitlist);
DECL_OPC_FUN(opc_while);
DECL_OPC_FUN(opc_whilenot);
DECL_OPC_FUN(opc_endwhile);
DECL_OPC_FUN(opc_exitwhile);
DECL_OPC_FUN(opc_mob);
DECL_OPC_FUN(opc_obj);
DECL_OPC_FUN(opc_room);
DECL_OPC_FUN(opc_token);
DECL_OPC_FUN(opc_tokenother);
/* DECL_OPC_FUN(opc_area);	Not yet */


/* General */
char *ifcheck_get_value(SCRIPT_VARINFO *info,IFCHECK_DATA *ifc,char *text,int *ret,bool *valid);
int execute_script(long pvnum, SCRIPT_DATA *script, CHAR_DATA *mob, OBJ_DATA *obj,
	ROOM_INDEX_DATA *room, TOKEN_DATA *token, CHAR_DATA *ch,
	OBJ_DATA *obj1,OBJ_DATA *obj2,CHAR_DATA *vch,CHAR_DATA *vch2,CHAR_DATA *rch, char *phrase, char *trigger,
	int number1, int number2, int number3, int number4, int number5);
void get_level_damage(int level, int *num, int *type, bool fRemort, bool fTwo);
CHAR_DATA *get_random_char(CHAR_DATA *mob, OBJ_DATA *obj, ROOM_INDEX_DATA *room, TOKEN_DATA *token);
int count_people_room(CHAR_DATA *mob, OBJ_DATA *obj, ROOM_INDEX_DATA *room, TOKEN_DATA *token, int iFlag);
int get_order(CHAR_DATA *ch, OBJ_DATA *obj);
bool has_item(CHAR_DATA *ch, long vnum, sh_int item_type, bool fWear);
CHAR_DATA *get_mob_vnum_room(CHAR_DATA *ch, OBJ_DATA *obj, ROOM_INDEX_DATA *room, TOKEN_DATA *token, long vnum);
OBJ_DATA *get_obj_vnum_room(CHAR_DATA *ch, OBJ_DATA *obj, ROOM_INDEX_DATA *room, TOKEN_DATA *token, long vnum);
void do_mob_transfer(CHAR_DATA *ch,ROOM_INDEX_DATA *room,bool quiet);
TOKEN_DATA *token_find_match(SCRIPT_VARINFO *info, TOKEN_DATA *tokens,char *argument);
CHAR_DATA *script_get_char_list(CHAR_DATA *mobs, CHAR_DATA *viewer, bool player, int vnum, char *name);
OBJ_DATA *script_get_obj_list(OBJ_DATA *objs, CHAR_DATA *viewer, int worn, int vnum, char *name);
void script_interpret(SCRIPT_VARINFO *info, char *command);
int trigger_index(char *name, int type);
bool has_trigger(LLIST **bank, int trigger);
bool mp_same_group(CHAR_DATA *ch,CHAR_DATA *vch,CHAR_DATA *to);
bool rop_same_group(CHAR_DATA *ch,CHAR_DATA *vch,CHAR_DATA *to);
bool script_expression_push_operator(STACK *stk,int op);
int ifcheck_lookup(char *name, int type);
ENT_FIELD *entity_type_lookup(char *name,ENT_FIELD *list);
ROOM_INDEX_DATA *get_exit_dest(ROOM_INDEX_DATA *room, char *argument);
bool script_change_exit(ROOM_INDEX_DATA *pRoom, ROOM_INDEX_DATA *pToRoom, int door);
char *trigger_name(int type);
char *trigger_phrase(int type, char *phrase);
char *trigger_phrase_olcshow(int type, char *phrase, bool is_rprog, bool is_tprog);
void script_clear_mobile(CHAR_DATA *ptr);
void script_clear_object(OBJ_DATA *ptr);
void script_clear_room(ROOM_INDEX_DATA *ptr);
void script_clear_token(TOKEN_DATA *ptr);
void script_clear_affect(AFFECT_DATA *ptr);
void script_clear_list(void *owner);
SCRIPT_VARINFO *script_get_prior(SCRIPT_VARINFO *info);

void script_mobile_addref(CHAR_DATA *ch);
bool script_mobile_remref(CHAR_DATA *ch);
void script_object_addref(OBJ_DATA *obj);
bool script_object_remref(OBJ_DATA *obj);
void script_room_addref(ROOM_INDEX_DATA *room);
bool script_room_remref(ROOM_INDEX_DATA *room);
void script_token_addref(TOKEN_DATA *token);
bool script_token_remref(TOKEN_DATA *token);


ENT_FIELD *script_entity_fields(int type);
bool script_entity_allow_vars(int type);
void script_entity_info(int type, ENT_FIELD **fields, bool *vars);

/* Expansion */
bool check_varinfo(SCRIPT_VARINFO *info);
bool expand_string(SCRIPT_VARINFO *info,char *str,char *store);
char *expand_argument(SCRIPT_VARINFO *info,char *str,SCRIPT_PARAM *arg);
char *expand_argument_expression(SCRIPT_VARINFO *info,char *str,int *num);

/* Compiling */
void compile_error(char *msg);
char *compile_ifcheck(char *str,int type, char **store);
char *compile_expression(char *str,int type, char **store);
char *compile_entity(char *str,int type, char **store);
char *compile_substring(char *str, int type, char **store, bool ifc, bool doquotes, bool recursed);
char *compile_string(char *str, int type, int *length, bool doquotes);
bool compile_script(BUFFER *err_buf,SCRIPT_DATA *script, char *source, int type);

/* Variables */

bool is_trigger_type(int tindex, int type);
bool variable_copy(ppVARIABLE list,char *oldname,char *newname);
bool variable_copylist(ppVARIABLE from,ppVARIABLE to,bool index);
bool variable_copyto(ppVARIABLE from,ppVARIABLE to,char *oldname,char *newname, bool index);
bool variable_fread(ppVARIABLE vars, int type, FILE *fp);
bool variable_fread_area_list(ppVARIABLE vars, char *name, FILE *fp);
bool variable_fread_exit_list(ppVARIABLE vars, char *name, FILE *fp);
bool variable_fread_room_list(ppVARIABLE vars, char *name, FILE *fp);
bool variable_fread_skill_list(ppVARIABLE vars, char *name, FILE *fp);
bool variable_fread_str_list(ppVARIABLE vars, char *name, FILE *fp);
bool variable_fread_uid_list(ppVARIABLE vars, char *name, int type, FILE *fp);
bool variable_fread_wilds_list(ppVARIABLE vars, char *name, FILE *fp);
bool variable_remove(ppVARIABLE list,char *name);
bool variable_setsave(pVARIABLE vars,char *name,bool state);
bool variable_validname(char *str);
bool variables_append_list_area (ppVARIABLE list, char *name, AREA_DATA *area);
bool variables_append_list_connection (ppVARIABLE list, char *name, DESCRIPTOR_DATA *conn);
bool variables_append_list_door (ppVARIABLE list, char *name, ROOM_INDEX_DATA *room, int door);
bool variables_append_list_exit (ppVARIABLE list, char *name, EXIT_DATA *ex);
bool variables_append_list_mob (ppVARIABLE list, char *name, CHAR_DATA *mob);
bool variables_append_list_obj (ppVARIABLE list, char *name, OBJ_DATA *obj);
bool variables_append_list_room (ppVARIABLE list, char *name, ROOM_INDEX_DATA *room);
bool variables_append_list_skill_sn (ppVARIABLE list, char *name, CHAR_DATA *ch, int sn);
bool variables_append_list_skill_token (ppVARIABLE list, char *name, TOKEN_DATA *tok);
bool variables_append_list_str (ppVARIABLE list, char *name, char *str);
bool variables_append_list_token (ppVARIABLE list, char *name, TOKEN_DATA *token);
bool variables_append_list_wilds (ppVARIABLE list, char *name, WILDS_DATA *wilds);
bool variables_append_string(ppVARIABLE list,char *name,char *str);
bool variables_argremove_string_index(ppVARIABLE list,char *name,int argindex);
bool variables_argremove_string_phrase(ppVARIABLE list,char *name,char *phrase);
bool variables_format_string(ppVARIABLE list,char *name);
bool variables_set_affect (ppVARIABLE list,char *name,AFFECT_DATA* aff);
bool variables_set_area (ppVARIABLE list,char *name,AREA_DATA* a);
bool variables_set_church (ppVARIABLE list,char *name,CHURCH_DATA* church);
bool variables_set_connection(ppVARIABLE list,char *name, DESCRIPTOR_DATA *conn);
bool variables_set_door (ppVARIABLE list,char *name, ROOM_INDEX_DATA *room, int door, bool save);
bool variables_set_exit(ppVARIABLE list,char *name,EXIT_DATA *e);
bool variables_set_integer(ppVARIABLE list,char *name,int num);
bool variables_set_list (ppVARIABLE list, char *name, int type, bool save);
bool variables_set_mobile(ppVARIABLE list,char *name,CHAR_DATA *m);
bool variables_set_object(ppVARIABLE list,char *name,OBJ_DATA *o);
bool variables_set_room(ppVARIABLE list,char *name,ROOM_INDEX_DATA *r);
bool variables_set_skill(ppVARIABLE list,char *name,int sn);
bool variables_set_skillinfo(ppVARIABLE list,char *name,CHAR_DATA *owner,int sn, TOKEN_DATA *token);
bool variables_set_string(ppVARIABLE list,char *name,char *str,bool shared);
bool variables_set_token(ppVARIABLE list,char *name,TOKEN_DATA *t);
bool variables_set_wilds (ppVARIABLE list,char *name,WILDS_DATA* wilds);
bool variables_setindex_integer(ppVARIABLE list,char *name,int num, bool saved);
bool variables_setindex_room(ppVARIABLE list,char *name,long vnum, bool saved);
bool variables_setindex_string(ppVARIABLE list,char *name,char *str,bool shared, bool saved);
bool variables_setsave_affect(ppVARIABLE list,char *name,AFFECT_DATA *aff, bool save);
bool variables_setsave_area (ppVARIABLE list, char *name,AREA_DATA* a, bool save);
bool variables_setsave_church (ppVARIABLE list, char *name,CHURCH_DATA* church, bool save);
bool variables_setsave_exit(ppVARIABLE list,char *name,EXIT_DATA *e, bool save);
bool variables_setsave_integer(ppVARIABLE list,char *name,int num, bool save);
bool variables_setsave_mobile(ppVARIABLE list,char *name,CHAR_DATA *m, bool save);
bool variables_setsave_object(ppVARIABLE list,char *name,OBJ_DATA *o, bool save);
bool variables_setsave_room(ppVARIABLE list,char *name,ROOM_INDEX_DATA *r, bool save);
bool variables_setsave_skill(ppVARIABLE list,char *name,int sn, bool save);
bool variables_setsave_skillinfo(ppVARIABLE list,char *name,CHAR_DATA *owner,int sn, TOKEN_DATA *token, bool save);
bool variables_setsave_string(ppVARIABLE list,char *name,char *str,bool shared, bool save);
bool variables_setsave_token(ppVARIABLE list,char *name,TOKEN_DATA *t, bool save);
bool variables_setsave_wilds (ppVARIABLE list, char *name,WILDS_DATA* wilds, bool save);
int variable_fread_type(char *str);
pVARIABLE variable_create(ppVARIABLE list,char *name, bool index, bool clear);
pVARIABLE variable_get(pVARIABLE list,char *name);
pVARIABLE variable_new(void);
void script_varclearon(ppVARIABLE vars, char *argument);
void variable_add(ppVARIABLE list,pVARIABLE var);
void variable_clearfield(int type, void *ptr);
void variable_dynamic_fix_church (CHURCH_DATA *church);
void variable_dynamic_fix_clone_room (ROOM_INDEX_DATA *clone);
void variable_dynamic_fix_mobile (CHAR_DATA *ch);
void variable_dynamic_fix_object(OBJ_DATA *obj);
void variable_dynamic_fix_token (TOKEN_DATA *token);
void variable_fix(pVARIABLE var);
void variable_fix_global(void);
void variable_fix_list(register pVARIABLE list);
void variable_free (pVARIABLE v);
void variable_freedata (pVARIABLE v);
void variable_freelist(ppVARIABLE list);
void variable_fwrite(pVARIABLE var, FILE *fp);
void variable_fwrite_uid_list( char *field, char *name, LLIST *list, FILE *fp);
void variable_index_fix(void);


// Prototype for find_path() in hunt.c
int find_path( long in_room_vnum, long out_room_vnum, CHAR_DATA *ch, int depth, int in_zone );
void script_varseton(SCRIPT_VARINFO *info, ppVARIABLE vars, char *argument);

void script_end_success(CHAR_DATA *ch);
void script_end_failure(CHAR_DATA *ch, bool messages);
void script_end_pulse(CHAR_DATA *ch);


/* Commands */
int mpcmd_lookup(char *command);
int opcmd_lookup(char *command);
int rpcmd_lookup(char *command);
int tpcmd_lookup(char *command,bool istoken);
void mob_interpret(SCRIPT_VARINFO *info, char *argument);
void obj_interpret(SCRIPT_VARINFO *info, char *argument);
void room_interpret(SCRIPT_VARINFO *info, char *argument);
void token_interpret(SCRIPT_VARINFO *info, char *argument);
void tokenother_interpret(SCRIPT_VARINFO *info, char *argument);
SCRIPT_CMD(do_mpaddaffect);
SCRIPT_CMD(do_opaddaffect);
SCRIPT_CMD(do_rpaddaffect);
SCRIPT_CMD(do_tpaddaffect);
SCRIPT_CMD(do_mpaddaffectname);
SCRIPT_CMD(do_opaddaffectname);
SCRIPT_CMD(do_rpaddaffectname);
SCRIPT_CMD(do_tpaddaffectname);
SCRIPT_CMD(do_tpadjust);
SCRIPT_CMD(do_mpairshipaddwaypoint);
SCRIPT_CMD(do_mpairshipsetcrash);
SCRIPT_CMD(do_mpalterexit);
SCRIPT_CMD(do_opalterexit);
SCRIPT_CMD(do_rpalterexit);
SCRIPT_CMD(do_tpalterexit);
SCRIPT_CMD(do_mpaltermob);
SCRIPT_CMD(do_opaltermob);
SCRIPT_CMD(do_rpaltermob);
SCRIPT_CMD(do_tpaltermob);
SCRIPT_CMD(do_mpalterobj);
SCRIPT_CMD(do_opalterobj);
SCRIPT_CMD(do_rpalterobj);
SCRIPT_CMD(do_tpalterobj);
SCRIPT_CMD(do_mpalterroom);
SCRIPT_CMD(do_opalterroom);
SCRIPT_CMD(do_rpalterroom);
SCRIPT_CMD(do_tpalterroom);
SCRIPT_CMD(do_mpasound);
SCRIPT_CMD(do_opasound);
SCRIPT_CMD(do_rpasound);
SCRIPT_CMD(do_tpasound);
SCRIPT_CMD(do_mpassist);
SCRIPT_CMD(do_mpat);
SCRIPT_CMD(do_opat);
SCRIPT_CMD(do_rpat);
SCRIPT_CMD(do_mpawardgold);
SCRIPT_CMD(do_mpawardpneuma);
SCRIPT_CMD(do_mpawardprac);
SCRIPT_CMD(do_mpawardqp);
SCRIPT_CMD(do_mpawardxp);
SCRIPT_CMD(do_mpcall);
SCRIPT_CMD(do_opcall);
SCRIPT_CMD(do_rpcall);
SCRIPT_CMD(do_tpcall);
SCRIPT_CMD(do_mpcancel);
SCRIPT_CMD(do_opcancel);
SCRIPT_CMD(do_rpcancel);
SCRIPT_CMD(do_mpcast);
SCRIPT_CMD(do_opcast);
SCRIPT_CMD(do_mpchangevesselname);
SCRIPT_CMD(do_mpchargemoney);
SCRIPT_CMD(do_mpcloneroom);
SCRIPT_CMD(do_opcloneroom);
SCRIPT_CMD(do_rpcloneroom);
SCRIPT_CMD(do_tpcloneroom);
SCRIPT_CMD(do_mpdamage);
SCRIPT_CMD(do_opdamage);
SCRIPT_CMD(do_rpdamage);
SCRIPT_CMD(do_tpdamage);
SCRIPT_CMD(do_mpdecdeity);
SCRIPT_CMD(do_mpdecpneuma);
SCRIPT_CMD(do_mpdecprac);
SCRIPT_CMD(do_mpdecquest);
SCRIPT_CMD(do_mpdectrain);
SCRIPT_CMD(do_mpdelay);
SCRIPT_CMD(do_opdelay);
SCRIPT_CMD(do_rpdelay);
SCRIPT_CMD(do_mpdequeue);
SCRIPT_CMD(do_opdequeue);
SCRIPT_CMD(do_rpdequeue);
SCRIPT_CMD(do_tpdequeue);
SCRIPT_CMD(do_mpdestroyroom);
SCRIPT_CMD(do_opdestroyroom);
SCRIPT_CMD(do_rpdestroyroom);
SCRIPT_CMD(do_tpdestroyroom);
SCRIPT_CMD(do_mpecho);
SCRIPT_CMD(do_opecho);
SCRIPT_CMD(do_rpecho);
SCRIPT_CMD(do_tpecho);
SCRIPT_CMD(do_mpechoaround);
SCRIPT_CMD(do_opechoaround);
SCRIPT_CMD(do_rpechoaround);
SCRIPT_CMD(do_tpechoaround);
SCRIPT_CMD(do_mpechoat);
SCRIPT_CMD(do_opechoat);
SCRIPT_CMD(do_rpechoat);
SCRIPT_CMD(do_tpechoat);
SCRIPT_CMD(do_mpechobattlespam);
SCRIPT_CMD(do_opechobattlespam);
SCRIPT_CMD(do_rpechobattlespam);
SCRIPT_CMD(do_tpechobattlespam);
SCRIPT_CMD(do_mpechochurch);
SCRIPT_CMD(do_opechochurch);
SCRIPT_CMD(do_rpechochurch);
SCRIPT_CMD(do_tpechochurch);
SCRIPT_CMD(do_mpechogrouparound);
SCRIPT_CMD(do_opechogrouparound);
SCRIPT_CMD(do_rpechogrouparound);
SCRIPT_CMD(do_tpechogrouparound);
SCRIPT_CMD(do_mpechogroupat);
SCRIPT_CMD(do_opechogroupat);
SCRIPT_CMD(do_rpechogroupat);
SCRIPT_CMD(do_tpechogroupat);
SCRIPT_CMD(do_mpecholeadaround);
SCRIPT_CMD(do_opecholeadaround);
SCRIPT_CMD(do_rpecholeadaround);
SCRIPT_CMD(do_tpecholeadaround);
SCRIPT_CMD(do_mpecholeadat);
SCRIPT_CMD(do_opecholeadat);
SCRIPT_CMD(do_rpecholeadat);
SCRIPT_CMD(do_tpecholeadat);
SCRIPT_CMD(do_mpechonotvict);
SCRIPT_CMD(do_opechonotvict);
SCRIPT_CMD(do_rpechonotvict);
SCRIPT_CMD(do_tpechonotvict);
SCRIPT_CMD(do_mpechoroom);
SCRIPT_CMD(do_opechoroom);
SCRIPT_CMD(do_rpechoroom);
SCRIPT_CMD(do_tpechoroom);
SCRIPT_CMD(do_mpflee);
SCRIPT_CMD(do_mpforce);
SCRIPT_CMD(do_opforce);
SCRIPT_CMD(do_rpforce);
SCRIPT_CMD(do_tpforce);
SCRIPT_CMD(do_mpforget);
SCRIPT_CMD(do_opforget);
SCRIPT_CMD(do_rpforget);
SCRIPT_CMD(do_tpforget);
SCRIPT_CMD(do_mpgdamage);
SCRIPT_CMD(do_opgdamage);
SCRIPT_CMD(do_rpgdamage);
SCRIPT_CMD(do_tpgdamage);
SCRIPT_CMD(do_mpgecho);
SCRIPT_CMD(do_opgecho);
SCRIPT_CMD(do_rpgecho);
SCRIPT_CMD(do_tpgecho);
SCRIPT_CMD(do_mpgforce);
SCRIPT_CMD(do_opgforce);
SCRIPT_CMD(do_rpgforce);
SCRIPT_CMD(do_tpgforce);
SCRIPT_CMD(do_tpgive);
SCRIPT_CMD(do_mpgoto);
SCRIPT_CMD(do_opgoto);
SCRIPT_CMD(do_tpgoto);
SCRIPT_CMD(do_mpgoxy);
SCRIPT_CMD(do_mpgtransfer);
SCRIPT_CMD(do_opgtransfer);
SCRIPT_CMD(do_rpgtransfer);
SCRIPT_CMD(do_tpgtransfer);
SCRIPT_CMD(do_mphunt);
SCRIPT_CMD(do_mpinput);
SCRIPT_CMD(do_opinput);
SCRIPT_CMD(do_rpinput);
SCRIPT_CMD(do_tpinput);
SCRIPT_CMD(do_mpinterrupt);
SCRIPT_CMD(do_opinterrupt);
SCRIPT_CMD(do_rpinterrupt);
SCRIPT_CMD(do_tpinterrupt);
SCRIPT_CMD(do_mpinvis);
SCRIPT_CMD(do_mpjunk);
SCRIPT_CMD(do_opjunk);
SCRIPT_CMD(do_tpjunk);
SCRIPT_CMD(do_mpkill);
SCRIPT_CMD(do_mplink);
SCRIPT_CMD(do_oplink);
SCRIPT_CMD(do_rplink);
SCRIPT_CMD(do_tplink);
SCRIPT_CMD(do_mpmload);
SCRIPT_CMD(do_opmload);
SCRIPT_CMD(do_rpmload);
SCRIPT_CMD(do_tpmload);
SCRIPT_CMD(do_mpoload);
SCRIPT_CMD(do_opoload);
SCRIPT_CMD(do_rpoload);
SCRIPT_CMD(do_tpoload);
SCRIPT_CMD(do_mpotransfer);
SCRIPT_CMD(do_opotransfer);
SCRIPT_CMD(do_rpotransfer);
SCRIPT_CMD(do_tpotransfer);
SCRIPT_CMD(do_mppeace);
SCRIPT_CMD(do_oppeace);
SCRIPT_CMD(do_rppeace);
SCRIPT_CMD(do_tppeace);
SCRIPT_CMD(do_mpprompt);
SCRIPT_CMD(do_opprompt);
SCRIPT_CMD(do_rpprompt);
SCRIPT_CMD(do_tpprompt);
SCRIPT_CMD(do_mppurge);
SCRIPT_CMD(do_oppurge);
SCRIPT_CMD(do_rppurge);
SCRIPT_CMD(do_tppurge);
SCRIPT_CMD(do_mpqueue);
SCRIPT_CMD(do_opqueue);
SCRIPT_CMD(do_rpqueue);
SCRIPT_CMD(do_tpqueue);
SCRIPT_CMD(do_mpraisedead);
SCRIPT_CMD(do_tpraisedead);
SCRIPT_CMD(do_mprawkill);
SCRIPT_CMD(do_oprawkill);
SCRIPT_CMD(do_rprawkill);
SCRIPT_CMD(do_tprawkill);
SCRIPT_CMD(do_mpremember);
SCRIPT_CMD(do_opremember);
SCRIPT_CMD(do_rpremember);
SCRIPT_CMD(do_tpremember);
SCRIPT_CMD(do_mpremove);
SCRIPT_CMD(do_opremove);
SCRIPT_CMD(do_rpremove);
SCRIPT_CMD(do_tpremove);
SCRIPT_CMD(do_mpresetdice);
SCRIPT_CMD(do_opresetdice);
SCRIPT_CMD(do_rpresetdice);
SCRIPT_CMD(do_tpresetdice);
SCRIPT_CMD(do_mpselfdestruct);
SCRIPT_CMD(do_opselfdestruct);
SCRIPT_CMD(do_mpsettimer);
SCRIPT_CMD(do_opsettimer);
SCRIPT_CMD(do_rpsettimer);
SCRIPT_CMD(do_tpsettimer);
SCRIPT_CMD(do_mpshowroom);
SCRIPT_CMD(do_opshowroom);
SCRIPT_CMD(do_rpshowroom);
SCRIPT_CMD(do_tpshowroom);
SCRIPT_CMD(do_mpskimprove);
SCRIPT_CMD(do_opskimprove);
SCRIPT_CMD(do_rpskimprove);
SCRIPT_CMD(do_tpskimprove);
SCRIPT_CMD(do_mpstringmob);
SCRIPT_CMD(do_opstringmob);
SCRIPT_CMD(do_rpstringmob);
SCRIPT_CMD(do_tpstringmob);
SCRIPT_CMD(do_mpstringobj);
SCRIPT_CMD(do_opstringobj);
SCRIPT_CMD(do_rpstringobj);
SCRIPT_CMD(do_tpstringobj);
SCRIPT_CMD(do_mpstripaffect);
SCRIPT_CMD(do_opstripaffect);
SCRIPT_CMD(do_rpstripaffect);
SCRIPT_CMD(do_tpstripaffect);
SCRIPT_CMD(do_mpstripaffectname);
SCRIPT_CMD(do_opstripaffectname);
SCRIPT_CMD(do_rpstripaffectname);
SCRIPT_CMD(do_tpstripaffectname);
SCRIPT_CMD(do_mptake);
SCRIPT_CMD(do_mpteleport);
SCRIPT_CMD(do_mptransfer);
SCRIPT_CMD(do_optransfer);
SCRIPT_CMD(do_rptransfer);
SCRIPT_CMD(do_tptransfer);
SCRIPT_CMD(do_mpusecatalyst);
SCRIPT_CMD(do_opusecatalyst);
SCRIPT_CMD(do_rpusecatalyst);
SCRIPT_CMD(do_tpusecatalyst);
SCRIPT_CMD(do_mpvarclear);
SCRIPT_CMD(do_opvarclear);
SCRIPT_CMD(do_rpvarclear);
SCRIPT_CMD(do_tpvarclear);
SCRIPT_CMD(do_mpvarclearon);
SCRIPT_CMD(do_opvarclearon);
SCRIPT_CMD(do_rpvarclearon);
SCRIPT_CMD(do_tpvarclearon);
SCRIPT_CMD(do_mpvarcopy);
SCRIPT_CMD(do_opvarcopy);
SCRIPT_CMD(do_rpvarcopy);
SCRIPT_CMD(do_tpvarcopy);
SCRIPT_CMD(do_mpvarsave);
SCRIPT_CMD(do_opvarsave);
SCRIPT_CMD(do_rpvarsave);
SCRIPT_CMD(do_tpvarsave);
SCRIPT_CMD(do_mpvarsaveon);
SCRIPT_CMD(do_opvarsaveon);
SCRIPT_CMD(do_rpvarsaveon);
SCRIPT_CMD(do_tpvarsaveon);
SCRIPT_CMD(do_mpvarset);
SCRIPT_CMD(do_opvarset);
SCRIPT_CMD(do_rpvarset);
SCRIPT_CMD(do_tpvarset);
SCRIPT_CMD(do_mpvarseton);
SCRIPT_CMD(do_opvarseton);
SCRIPT_CMD(do_rpvarseton);
SCRIPT_CMD(do_tpvarseton);
SCRIPT_CMD(do_mpvforce);
SCRIPT_CMD(do_opvforce);
SCRIPT_CMD(do_rpvforce);
SCRIPT_CMD(do_tpvforce);
SCRIPT_CMD(do_mpvis);
SCRIPT_CMD(do_mpzecho);
SCRIPT_CMD(do_opzecho);
SCRIPT_CMD(do_rpzecho);
SCRIPT_CMD(do_tpzecho);
SCRIPT_CMD(do_mpzot);
SCRIPT_CMD(do_opzot);
SCRIPT_CMD(do_rpzot);
SCRIPT_CMD(do_tpzot);
SCRIPT_CMD(do_mpxcall);
SCRIPT_CMD(do_opxcall);
SCRIPT_CMD(do_rpxcall);
SCRIPT_CMD(do_tpxcall);
SCRIPT_CMD(do_mpchargebank);
SCRIPT_CMD(do_opchargebank);
SCRIPT_CMD(do_rpchargebank);
SCRIPT_CMD(do_tpchargebank);
SCRIPT_CMD(do_mpwiretransfer);
SCRIPT_CMD(do_opwiretransfer);
SCRIPT_CMD(do_rpwiretransfer);
SCRIPT_CMD(do_tpwiretransfer);
SCRIPT_CMD(do_tpawardgold);
SCRIPT_CMD(do_tpawardpneuma);
SCRIPT_CMD(do_tpawardprac);
SCRIPT_CMD(do_tpawardqp);
SCRIPT_CMD(do_tpawardxp);
SCRIPT_CMD(do_mpstartcombat);
SCRIPT_CMD(do_opstartcombat);
SCRIPT_CMD(do_rpstartcombat);
SCRIPT_CMD(do_tpstartcombat);
SCRIPT_CMD(do_mppersist);
SCRIPT_CMD(do_oppersist);
SCRIPT_CMD(do_rppersist);
SCRIPT_CMD(do_tppersist);
SCRIPT_CMD(do_mpskill);
SCRIPT_CMD(do_opskill);
SCRIPT_CMD(do_rpskill);
SCRIPT_CMD(do_tpskill);
SCRIPT_CMD(do_mpskillgroup);
SCRIPT_CMD(do_opskillgroup);
SCRIPT_CMD(do_rpskillgroup);
SCRIPT_CMD(do_tpskillgroup);
SCRIPT_CMD(do_mpcondition);
SCRIPT_CMD(do_opcondition);
SCRIPT_CMD(do_rpcondition);
SCRIPT_CMD(do_tpcondition);

SCRIPT_CMD(do_mpscriptwait);
SCRIPT_CMD(do_opscriptwait);
SCRIPT_CMD(do_tpscriptwait);
SCRIPT_CMD(do_tpcastfailure);
SCRIPT_CMD(do_tpcastrecover);


SCRIPT_CMD(do_mpaddspell);
SCRIPT_CMD(do_mpremspell);
SCRIPT_CMD(do_mpalteraffect);
SCRIPT_CMD(do_mpcrier);
SCRIPT_CMD(do_opaddspell);
SCRIPT_CMD(do_opremspell);
SCRIPT_CMD(do_opalteraffect);
SCRIPT_CMD(do_opcrier);
SCRIPT_CMD(do_rpaddspell);
SCRIPT_CMD(do_rpremspell);
SCRIPT_CMD(do_rpalteraffect);
SCRIPT_CMD(do_rpcrier);
SCRIPT_CMD(do_tpaddspell);
SCRIPT_CMD(do_tpremspell);
SCRIPT_CMD(do_tpalteraffect);
SCRIPT_CMD(do_tpcrier);

SCRIPT_CMD(do_mpsaveplayer);
SCRIPT_CMD(do_opsaveplayer);
SCRIPT_CMD(do_rpsaveplayer);
SCRIPT_CMD(do_tpsaveplayer);

SCRIPT_CMD(do_mpcheckpoint);
SCRIPT_CMD(do_opcheckpoint);
SCRIPT_CMD(do_rpcheckpoint);
SCRIPT_CMD(do_tpcheckpoint);

#include "tables.h"

#endif /* !__SCRIPTS_H__ */
