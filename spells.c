struct	skill_type
{
    char *	name;			/* Name of skill		*/
    sh_int	skill_level[MAX_CLASS];	/* Level needed by class	*/
    sh_int	rating[MAX_CLASS];	/* How hard it is to learn	*/
    SPELL_FUN *	spell_fun;		/* Spell pointer (for spells)	*/
    sh_int	target;			/* Legal targets		*/
    sh_int	minimum_position;	/* Position for caster / user	*/
    sh_int *	pgsn;			/* Pointer to associated gsn	*/
    //sh_int	slot;		 	Syn- reusing this as a racial skill toggle.
    int 	race;			/* If it's a racial skill ONLY, this is the race number. If not, its -1.
					   This doesn't apply for skills that can be gotten from classes, like archery. */

    sh_int	min_mana;		/* Minimum mana used		*/
    sh_int	beats;			/* Waiting time after use	*/
    char *	noun_damage;		/* Damage message		*/
    char *	msg_off;		/* Wear off message		*/
    char *	msg_obj;		/* Wear off message for obects	*/
    char *	msg_disp;		// Dispel message, shown to the room
    int		inks[3][2];
};


{
	"reserved",
	{ 1, 1, 1, 1 }, { 200, 200, 200, 200},
	NULL, TAR_IGNORE, POS_STANDING, NULL,
	-1, 0, 0,
	"", "", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"acid blast",
	{ 16, 31, 31, 16 }, { 4, 4, 4, 4},
	spell_acid_blast, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_acid_blast,
	-1, 20, 4,
	"acid blast", "!Acid Blast!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"acid breath",
	{ 11, 11, 11, 11 }, { 10, 1, 2, 2},
	spell_acid_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_acid_breath,
	-1, 100, 2,
	"blast of acid", "!Acid Breath!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"acrobatics",
	{ 31, 31, 3, 31 }, { 8, 8, 8, 4},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_acro,
	-1, 0, 0,
	"", "!Acrobatics!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"afterburn",
	{ 20, 31, 31, 31 }, { 15, 25, 25, 25 },
	spell_afterburn, TAR_IGNORE, POS_STANDING, &gsn_afterburn,
	0, 150, 10,
	"scorching fireball", "", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"air spells",
	{1, 31, 31, 31}, {15, 20, 30, 30},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_air_spells,
	-1, 0, 0,
	"", "", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"ambush",
	{ 31, 31, 22, 31 }, { 20, 18, 13, 14 },
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_ambush,
	0, 0, 0,
	"", "", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"animate dead",
	{ 12, 31, 31, 31 }, { 5, 5, 4, 4},
	spell_animate_dead, TAR_OBJ_GROUND, POS_STANDING, &gsn_animate_dead,
	-1, 75, 8,
	"", "", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"archery",
	{ 10, 10, 7, 8 }, { 15, 15, 10, 12},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_archery,
	-1, 0, 24,
	"shot", "!Archery!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"armor",
	{ 3, 1, 31, 3 }, { 2, 2, 5, 5},
	spell_armor, TAR_CHAR_DEFENSIVE, POS_STANDING, &gsn_armor,
	-1, 5, 2,
	"", "{CYou feel less armored.{x", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"athletics",
	{ 31, 31, 25, 5 }, { 14, 14, 14, 8 },
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_athletics,
	0, 0, 0,
	"", "", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"avatar shield",
	{ 31, 10, 31, 31 }, { 3, 5, 2, 2},
	spell_avatar_shield, TAR_CHAR_SELF, POS_STANDING, &gsn_avatar_shield,
	-1, 75, 12,
	"", "{CYou feel less resistant to evil attacks.{x", "", "",
	{ { CATALYST_BODY, 2 },{ CATALYST_COSMIC, 2 },{ CATALYST_HOLY, 2 } }
}, {
	"axe",
	{ 1, 1, 1, 1 }, { 6, 6, 5, 4},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_axe,
	-1, 0, 0,
	"", "!Axe!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"backstab",
	{ 31, 31, 15, 31 }, { 0, 0, 5, 0},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_backstab,
	-1, 0, 6,
	"backstab", "!Backstab!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"bar",
	{ 31, 31, 12, 31 }, { 8, 8, 8, 8},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_bar,
	-1, 0, 12,
	"bar", "!Bar!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"bash",
	{ 15, 12, 11, 10}, { 11, 5, 6, 4},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_bash,
	-1, 0, 9,
	"bash", "!Bash!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"behead",
	{ 31, 31, 31, 18 }, { 8, 8, 8, 8},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_behead,
	-1, 0, 12,
	"", "!Behead!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"berserk",
	{ 2, 2, 2, 2 }, { 9, 8, 6, 5},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_berserk,
	PC_RACE_DWARF, 0, 24,
	"", "{CYou feel your pulse slow down.{x", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"bind",
	{ 31, 31, 31, 8 }, { 8, 8, 8, 8},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_bind,
	-1, 0, 0,
	"", "!Bind!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"bite",
	{ 1, 1, 1, 1 }, { 2, 2, 2, 2},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_bite,
	-1, 0, 12,
	"bite", "!Bite!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"blackjack",
	{ 31, 31, 21, 31 }, { 0, 0, 5, 0},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_blackjack,
	-1, 0, 12,
	"blackjack", "!Blackjack!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"bless",
	{ 10, 4, 31, 4 }, { 2, 2, 4, 4},
	spell_bless, TAR_OBJ_CHAR_DEF, POS_STANDING, &gsn_bless,
	-1, 5, 2,
	"", "{CYou feel less righteous.{x", "$p's holy aura fades.", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"blindness",
	{ 6, 4, 31, 6 }, { 3, 3, 4, 5},
	spell_blindness, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_blindness,
	-1, 75, 8,
	"", "You can see again.", "", "",
	{ { CATALYST_MIND, 1 },{ CATALYST_BODY, 1 },{ CATALYST_NONE, 0 } }
}, {
	"blowgun",
	{ 1, 1, 1, 1 }, { 4, 4, 4, 3},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_blowgun,
	-1, 0, 0,
	"", "!Blowgun!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"bomb",
	{ 31, 31, 14, 31 }, { 8, 8, 8, 8},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_bomb,
	-1, 0, 24,
	"bomb", "!Bomb!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"bow",
	{ 1, 1, 1, 1 }, { 10, 10, 8, 10},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_bow,
	-1, 0, 0,
	"arrow", "!Bow!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"breath",
	{ 1, 1, 1, 1 }, { 5, 5, 5, 5},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_breath,
	PC_RACE_DRACONIAN, 0, 12,
	"breath", "!Breath!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"brew",
	{ 31, 13, 31, 31 }, { 8, 8, 6, 4},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_brew,
	-1, 0, 0,
	"", "!Brew!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"burgle",
	{ 31, 31, 15, 31 }, { 0, 0, 9, 0},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_burgle,
	-1, 0, 0,
	"", "!Burgle!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"burning hands",
	{ 3, 31, 31, 31 }, { 3, 3, 3, 3},
	spell_burning_hands, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_burning_hands,
	-1, 15, 4,
	"burning hands", "!Burning Hands!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"call familiar",
	{ 31, 16, 31, 31 }, { 4, 8, 2, 2},
	spell_call_familiar, TAR_IGNORE, POS_STANDING, &gsn_call_familiar,
	-1, 5, 12,
	"", "", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"call lightning",
	{ 13, 9, 31, 9 }, { 7, 7, 6, 7},
	spell_call_lightning, TAR_IGNORE, POS_FIGHTING, &gsn_call_lightning,
	-1, 15, 8,
	"lightning bolt", "!Call Lightning!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"calm",
	{ 18, 8, 31, 8 }, { 5, 5, 4, 4},
	spell_calm, TAR_IGNORE, POS_FIGHTING, &gsn_calm,
	-1, 30, 8,
	"", "{CYou have lost your peace of mind.{x", "", "",
	{ { CATALYST_MIND, 1 },{ CATALYST_HOLY, 1 },{ CATALYST_NONE, 0 } }
}, {
	"cancellation",
	{ 9, 31, 31, 31 }, { 4, 4, 4, 4},
	spell_cancellation, TAR_CHAR_DEFENSIVE, POS_FIGHTING, &gsn_cancellation,
	-1, 20, 12,
	"", "!cancellation!", "", "",
	{ { CATALYST_BODY, 5 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"catch",
	{ 31, 14, 31, 31 }, { 8, 10, 6, 4},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_catch,
	-1, 0, 0,
	"", "!Catch!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"cause critical",
	{ 31, 16, 31, 16 }, { 4, 4, 4, 4},
	spell_cause_critical, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_cause_critical,
	-1, 20, 6,
	"spell", "!Cause Critical!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"cause light",
	{ 31, 1, 31, 1 }, { 1, 1, 2, 2},
	spell_cause_light, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_cause_light,
	-1, 15, 2,
	"spell", "!Cause Light!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"cause serious",
	{ 31, 4, 31, 4 }, { 1, 3, 2, 2},
	spell_cause_serious, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_cause_serious,
	-1, 17, 6,
	"spell", "!Cause Serious!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"chain lightning",
	{ 18, 31, 31, 31 }, { 4, 8, 2, 2},
	spell_chain_lightning, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_chain_lightning,
	-1, 75, 12,
	"lightning", "!Chain Lightning!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"channel",
	{ 14, 31, 31, 31 }, { 4, 1, 2, 2},
	spell_channel, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_channel,
	-1, 50, 9,
	"channel", "!Channel!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"charge",
	{ 1, 1, 1, 1}, { 4, 4, 4, 4},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_charge,
	PC_RACE_MINOTAUR, 0, 3,
	"charge", "!Charge!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"charm person",
	{ 10, 31, 31, 31 }, { 4, 7, 2, 2},
	spell_charm_person, TAR_CHAR_OFFENSIVE, POS_STANDING, &gsn_charm_person,
	-1, 5, 12,
	"", "{YYou feel more self-confident.{x", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"chill touch",
	{ 2, 31, 31, 31 }, { 3, 4, 2, 2},
	spell_chill_touch, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_chill_touch,
	-1, 15, 6,
	"chilling touch", "{CYou feel less cold.{x", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"circle",
	{ 31, 31, 15, 15 }, { 0, 0, 5, 5},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_circle,
	-1, 0, 16,
	"circle", "!Circle!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"cloak of guile",
	{ 31, 31, 6, 31 }, { 5, 5, 5, 5},
	spell_cloak_of_guile, TAR_CHAR_SELF, POS_STANDING, &gsn_cloak_of_guile,
	-1, 50, 12,
	"", "{YYou are no longer invisible to creatures.{x", "", "",
	{ { CATALYST_BODY, 10 },{ CATALYST_ASTRAL, 5 },{ CATALYST_LAW, 5 } }
}, {
	"colour spray",
	{ 8, 31, 31, 31 }, { 3, 3, 2, 2},
	spell_colour_spray, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_colour_spray,
	-1, 15, 6,
	"colour spray", "!Colour Spray!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"combine",
	{ 31, 9, 31, 31 }, { 15, 12, 10, 9 },
	spell_null, TAR_IGNORE, POS_RESTING, &gsn_combine,
	0, 0, 0,
	"", "", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"consume",
	{ 1, 1, 1, 1 }, { 1, 1, 1, 1},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_consume,
	PC_RACE_VAMPIRE, 0, 6,
	"", "!Consume!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"continual light",
	{ 6, 4, 31, 6 }, { 2, 3, 2, 2},
	spell_continual_light, TAR_IGNORE, POS_STANDING, &gsn_continual_light,
	-1, 7, 6,
	"", "!Continual Light!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"control weather",
	{ 15, 19, 31, 31 }, { 3, 3, 2, 2},
	spell_control_weather, TAR_IGNORE, POS_STANDING, &gsn_control_weather,
	-1, 25, 4,
	"", "!Control Weather!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"cosmic blast",
	{ 31, 25, 31, 31 }, { 1, 8, 2, 2},
	spell_cosmic_blast, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_cosmic_blast,
	-1, 75, 10,
	"cosmic blast", "!Cosmic Blast!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"counterspell",
	{ 22, 22, 31, 31 }, { 6, 6, 5, 5},
	spell_counter_spell, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_counterspell,
	-1, 100, 1,
	"counterspell", "!Counterspell!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"create food",
	{ 31, 5, 31, 5 }, { 1, 3, 2, 2},
	spell_create_food, TAR_IGNORE, POS_STANDING, &gsn_create_food,
	-1, 5, 2,
	"", "!Create Food!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"create rose",
	{15, 5, 31, 31}, {6, 4, 8, 9},
	spell_create_rose, TAR_IGNORE, POS_STANDING, &gsn_create_rose,
	0, 50, 8,
	"", "", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"create spring",
	{ 8, 31, 31, 8 }, { 1, 3, 2, 2},
	spell_create_spring, TAR_IGNORE, POS_STANDING, &gsn_create_spring,
	-1, 20, 6,
	"", "!Create Spring!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"create water",
	{ 31, 3, 31, 3 }, { 1, 3, 2, 2},
	spell_create_water, TAR_OBJ_INV, POS_STANDING, &gsn_create_water,
	-1, 5, 2,
	"", "!Create Water!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"crippling touch",
	{ 5, 5, 3, 1 }, { 5, 5, 5, 5},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_crippling_touch,
	PC_RACE_LICH, 0, 0,
	"", "!crippling touch!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"crossbow",
	{ 1, 1, 1, 1 }, { 10, 10, 9, 10},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_crossbow,
	-1, 0, 0,
	"bolt", "!Crossbow!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"cure blindness",
	{ 31, 6, 31, 6 }, { 1, 4, 2, 2},
	spell_cure_blindness, TAR_CHAR_DEFENSIVE, POS_FIGHTING, &gsn_cure_blindness,
	-1, 5, 4,
	"", "!Cure Blindness!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"cure critical",
	{ 10, 10, 10, 10 }, { 4, 4, 4, 4},
	spell_cure_critical, TAR_CHAR_DEFENSIVE, POS_FIGHTING, &gsn_cure_critical,
	-1, 35, 6,
	"", "!Cure Critical!", "", "",
	{ { CATALYST_BODY, 4 },{ CATALYST_BLOOD, 1 },{ CATALYST_NONE, 0 } }
}, {
	"cure disease",
	{ 31, 9, 31, 9 }, { 1, 3, 2, 2},
	spell_cure_disease, TAR_CHAR_DEFENSIVE, POS_FIGHTING, &gsn_cure_disease,
	-1, 20, 6,
	"", "!Cure Disease!", "", "",
	{ { CATALYST_BODY, 2 },{ CATALYST_BLOOD, 1 },{ CATALYST_TOXIN, 3 } }
}, {
	"cure light",
	{ 1, 1, 1, 1 }, { 1, 2, 2, 2},
	spell_cure_light, TAR_CHAR_DEFENSIVE, POS_FIGHTING, &gsn_cure_light,
	-1, 10, 2,
	"", "!Cure Light!", "", "",
	{ { CATALYST_BODY, 1 },{ CATALYST_BLOOD, 1 },{ CATALYST_NONE, 0 } }
}, {
	"cure poison",
	{ 31, 10, 31, 10 }, { 1, 4, 2, 2},
	spell_cure_poison, TAR_CHAR_DEFENSIVE, POS_FIGHTING, &gsn_cure_poison,
	-1, 5, 2,
	"", "!Cure Poison!", "", "",
	{ { CATALYST_BODY, 2 },{ CATALYST_BLOOD, 1 },{ CATALYST_TOXIN, 3 } }
}, {
	"cure serious",
	{ 7, 7, 7, 7 }, { 4, 4, 2, 2},
	spell_cure_serious, TAR_CHAR_DEFENSIVE, POS_FIGHTING, &gsn_cure_serious,
	-1, 15, 6,
	"", "!Cure Serious!", "", "",
	{ { CATALYST_BODY, 2 },{ CATALYST_BLOOD, 1 },{ CATALYST_NONE, 0 } }
}, {
	"cure toxic",
	{ 31, 31, 31, 31 }, { 31, 31, 31, 31},
	spell_cure_toxic, TAR_CHAR_DEFENSIVE, POS_FIGHTING, &gsn_cure_toxic,
	-1, 5, 2,
	"", "!Cure Toxic!", "", "",
	{ { CATALYST_BODY, 5 },{ CATALYST_TOXIN, 2 },{ CATALYST_DEATH, 2 } }
}, {
	"curse",
	{ 10, 10, 31, 10 }, { 1, 4, 2, 2},
	spell_curse, TAR_OBJ_CHAR_OFF, POS_FIGHTING, &gsn_curse,
	-1, 20, 6,
	"curse", "{CThe curse wears off.{x", "$p is no longer impure.", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"dagger",
	{ 1, 1, 1, 1 }, { 2, 3, 2, 2},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_dagger,
	-1, 0, 0,
	"", "!Dagger!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"dark shroud",
	{ 10, 31, 31, 31 }, { 2, 1, 2, 2},
	spell_dark_shroud, TAR_CHAR_DEFENSIVE, POS_STANDING, &gsn_dark_shroud,
	-1, 40, 4,
	"", "{WEverything seems brighter as your dark shroud fades.{x", "", "",
	{ { CATALYST_MIND, 2 },{ CATALYST_DEATH, 2 },{ CATALYST_NONE, 0 } }
}, {
	"death grip",
	{ 13, 31, 31, 31 }, { 6, 1, 2, 2},
	spell_death_grip, TAR_CHAR_SELF, POS_STANDING, &gsn_death_grip,
	-1, 20, 6,
	"", "Your grip on your weapon loosens.", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"deathbarbs",
	{ 31, 31, 13, 31 }, { 6, 6, 6, 6},
	spell_deathbarbs, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_deathbarbs,
	-1, 100, 8,
	"deathbarbs", "!Deathbarbs!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"deathsight",
	{ 5, 31, 31, 31 }, { 2, 1, 2, 2},
	spell_deathsight, TAR_CHAR_SELF, POS_STANDING, &gsn_deathsight,
	-1, 15, 4,
	"", "{DYour perception of the dead fades.{x", "", "",
	{ { CATALYST_MIND, 2 },{ CATALYST_DEATH, 2 },{ CATALYST_NONE, 0 } }
}, {
	"deception",
	{ 8, 8, 8, 8 }, { 8, 8, 8, 8},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_deception,
	-1, 0, 24,
	"deception", "!Deception!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"deep trance",
	{ 8, 5, 31, 31 }, { 7, 6, 8, 9 },
	spell_null, TAR_IGNORE, POS_RESTING, &gsn_deep_trance,
	-1, 0, 0,
	"", "", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"demonfire",
	{ 31, 25, 31, 31 }, { 1, 8, 2, 2},
	spell_demonfire, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_demonfire,
	-1, 20, 8,
	"torments", "!Demonfire!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"destruction",
	{ 8, 31, 31, 31 }, { 5, 10, 10, 10 },
	spell_destruction, TAR_OBJ_INV, POS_STANDING, &gsn_destruction,
	0, 250, 14,
	"", "", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"detect hidden",
	{ 12, 12, 12, 12 }, { 3, 3, 2, 2},
	spell_detect_hidden, TAR_CHAR_SELF, POS_STANDING, &gsn_detect_hidden,
	-1, 5, 2,
	"", "{YYou feel less aware of your surroundings.{x", "", "",
	{ { CATALYST_MIND, 2 },{ CATALYST_LIGHT, 1 },{ CATALYST_EARTH, 1 } }
}, {
	"detect invis",
	{ 3, 8, 31, 8 }, { 3, 3, 2, 2},
	spell_detect_invis, TAR_CHAR_SELF, POS_FIGHTING, &gsn_detect_invis,
	-1, 5, 2,
	"", "{YYou no longer see invisible objects.{x", "", "",
	{ { CATALYST_MIND, 2 },{ CATALYST_LIGHT, 1 },{ CATALYST_AIR, 1 } }
}, {
	"detect magic",
	{ 2, 6, 31, 6 }, { 3, 3, 2, 2},
	spell_detect_magic, TAR_CHAR_SELF, POS_STANDING, &gsn_detect_magic,
	-1, 5, 2,
	"", "{CThe detect magic wears off.{x", "", "",
	{ { CATALYST_MIND, 2 },{ CATALYST_LIGHT, 1 },{ CATALYST_ENERGY, 1 } }
}, {
	"detect traps",
	{31, 31, 12, 31}, {25, 22, 8, 15},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_detect_traps,
	-1, 0, 0,
	"", "", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"dirt kicking",
	{ 16, 16, 16, 16 }, { 20, 20, 4, 4},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_dirt,
	-1, 0, 7,
	"kicked dirt", "You rub the dirt out of your eyes.", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"disarm",
	{ 31, 31, 31, 11 }, { 20, 20, 6, 4},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_disarm,
	-1, 0, 12,
	"", "!Disarm!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"discharge",
	{ 17, 31, 31, 31 }, { 8, 2, 4, 4},
	spell_discharge, TAR_OBJ_INV, POS_STANDING, &gsn_discharge,
	-1, 250, 16,
	"", "!Discharge!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"dispel evil",
	{ 31, 15, 31, 15 }, { 4, 3, 2, 2},
	spell_dispel_evil, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_dispel_evil,
	-1, 15, 6,
	"dispel evil", "!Dispel Evil!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"dispel good",
	{ 31, 15, 31, 15 }, { 4, 4, 2, 2},
	spell_dispel_good, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_dispel_good,
	-1, 15, 6,
	"dispel good", "!Dispel Good!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"dispel magic",
	{ 16, 15, 31, 31 }, { 6, 6, 2, 2},
	spell_dispel_magic, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_dispel_magic,
	-1, 50, 8,
	"", "!Dispel Magic!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"dispel room",
	{ 18, 31, 31, 31 }, { 6, 6, 2, 2},
	spell_dispel_room, TAR_IGNORE, POS_STANDING, &gsn_dispel_room,
	-1, 100, 16,
	"", "!Dispel Room!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"dodge",
	{ 31, 8, 5, 5 }, { 8, 8, 4, 6},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_dodge,
	-1, 0, 0,
	"", "!Dodge!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"dual wield",
	{ 31, 31, 31, 5 }, { 5, 5, 10, 4},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_dual,
	-1, 0, 0,
	"", "!Dual!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"eagle eye",
	{ 21, 31, 31, 31 }, { 8, 6, 6, 6},
	spell_eagle_eye, TAR_CHAR_SELF, POS_STANDING, &gsn_eagle_eye,
	-1, 50, 8,
	"", "", "", "",
	{ { CATALYST_AIR, 4 },{ CATALYST_COSMIC, 4 },{ CATALYST_NONE, 0 } }
}, {
	"earth spells",
	{1, 31, 31, 31}, {15, 20, 30, 30},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_earth_spells,
	-1, 0, 0,
	"", "", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"earth walk",
	{ 20, 31, 31, 31 }, { 15, 25, 25, 25 },
	spell_earth_walk, TAR_IGNORE_CHAR_DEF, POS_STANDING, &gsn_earth_walk,
	0, 150, 10,
	"", "", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"earthquake",
	{ 15, 10, 15, 10 }, { 1, 5, 2, 5},
	spell_earthquake, TAR_IGNORE, POS_FIGHTING, &gsn_earthquake,
	-1, 25, 6,
	"earthquake", "!Earthquake!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"electrical barrier",
	{ 20, 31, 31, 31}, { 15, 15, 0, 0 },
	spell_electrical_barrier, TAR_CHAR_DEFENSIVE, POS_STANDING, &gsn_electrical_barrier,
	-1, 100, 8,
	"electrical wave", "{CThe electrical barrier surrounding you vanishes.{x", "", "",
	{ { CATALYST_BODY, 2 },{ CATALYST_COSMIC, 1 },{ CATALYST_ENERGY, 2 } }
}, {
	"enchant armor",
	{ 10, 15, 31, 31 }, { 3, 2, 4, 4 },
	spell_enchant_armor, TAR_OBJ_INV, POS_STANDING, &gsn_enchant_armor,
	-1, 100, 8,
	"", "!Enchant Armor!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"enchant weapon",
	{ 17, 15, 31, 31 }, { 4, 2, 4, 4},
	spell_enchant_weapon, TAR_OBJ_INV, POS_STANDING, &gsn_enchant_weapon,
	-1, 100, 8,
	"", "!Enchant Weapon!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"energy drain",
	{ 15, 31, 31, 31 }, { 4, 1, 2, 2},
	spell_energy_drain, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_energy_drain,
	-1, 50, 6,
	"energy drain", "!Energy Drain!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"energy field",
	{ 31, 31, 31, 17 }, { 10, 10, 2, 8},
	spell_energy_field, TAR_CHAR_DEFENSIVE, POS_STANDING, &gsn_energy_field,
	-1, 75, 8,
	"", "{WThe humming noise around you fades.{x", "", "",
	{ { CATALYST_BODY, 2 },{ CATALYST_COSMIC, 1 },{ CATALYST_ENERGY, 2 } }
}, {
	"enhanced damage",
	{ 31, 22, 15, 13 }, { 10, 9, 5, 6},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_enhanced_damage,
	-1, 0, 0,
	"", "!Enhanced Damage!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"ensnare",
	{15, 31, 31, 31}, {6, 10, 11, 11},
	spell_ensnare, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_ensnare,
	-1, 50, 7,
	"", "The vines clutching you wither and fall away.", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"entrap",
	{8, 31, 31, 31}, {5, 9, 10, 10},
	spell_entrap, TAR_OBJ_INV, POS_STANDING, &gsn_entrap,
	-1, 45, 10,
	"", "", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"envenom",
	{ 31, 31, 10, 31 }, { 20,20, 5, 20 },
	spell_null, TAR_IGNORE, POS_RESTING, &gsn_envenom,
	-1, 0, 36,
	"", "!Envenom!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"evasion",
	{ 31, 31, 20, 31 }, { 6, 6, 8, 6},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_evasion,
	-1, 0, 12,
	"", "You no longer feel evasive.", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"exorcism",
	{ 31, 15, 31, 31 }, { 4, 5, 2, 2},
	spell_exorcism, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_exorcism,
	-1, 50, 12,
	"exorcism", "!Exorcism!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"exotic",
	{ 1, 1, 1, 1 }, { 2, 3, 2, 2},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_exotic,
	-1, 0, 0,
	"", "!Exotic!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"fade",
	{ 1, 1, 1, 1 }, { 10, 10, 10, 10},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_fade,
	-1, 0, 24,
	"fade", "!Fade!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"faerie fire",
	{ 6, 3, 31, 6 }, { 4, 4, 2, 2},
	spell_faerie_fire, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_faerie_fire,
	-1, 5, 6,
	"faerie fire", "{MThe pink aura around you fades away.{x", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"faerie fog",
	{ 14, 31, 31, 14 }, { 4, 1, 2, 2},
	spell_faerie_fog, TAR_IGNORE, POS_STANDING, &gsn_faerie_fog,
	-1, 12, 6,
	"faerie fog", "!Faerie Fog!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"fast healing",
	{ 12, 12, 11, 6 }, { 8, 5, 6, 4},
	spell_null, TAR_IGNORE, POS_SLEEPING, &gsn_fast_healing,
	-1, 0, 0,
	"", "!Fast Healing!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"fatigue",
	{ 14, 31, 31, 31 }, { 5, 5, 5, 5},
	spell_fatigue, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_fatigue,
	-1, 35, 6,
	"spell_fatigue", "You regain your stamina.", "", "",
	{ { CATALYST_BODY, 2 },{ CATALYST_CHAOS, 2 },{ CATALYST_NONE, 0 } }
}, {
	"feign",
	{ 8, 8, 8, 8 }, { 5, 5, 5, 5},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_feign,
	-1, 0, 12,
	"feign", "!Feign!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"fire barrier",
	{ 12, 31, 31, 31}, { 10, 10, 0, 0 },
	spell_fire_barrier, TAR_CHAR_DEFENSIVE, POS_STANDING, &gsn_fire_barrier,
	-1, 50, 8,
	"fire wave", "{RThe flames protecting you vanish.{x", "", "",
	{ { CATALYST_BODY, 2 },{ CATALYST_COSMIC, 1 },{ CATALYST_FIRE, 2 } }
}, {
	"fire breath",
	{ 1, 1, 1, 1 }, { 10, 1, 2, 2},
	spell_fire_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_fire_breath,
	-1, 200, 2,
	"blast of flame", "The smoke leaves your eyes.", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"fire cloud",
	{ 13, 31, 31, 31 }, { 8, 6, 2, 2},
	spell_fire_cloud, TAR_IGNORE, POS_STANDING, &gsn_fire_cloud,
	-1, 50, 12,
	"fire cloud", "!Fire Cloud!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"fire spells",
	{1, 31, 31, 31}, {15, 20, 30, 30},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_fire_spells,
	-1, 0, 0,
	"", "", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"fireball",
	{ 7, 31, 31, 31 }, { 2, 2, 2, 2},
	spell_fireball, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_fireball,
	-1, 50, 6,
	"fireball", "!Fireball!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"fireproof",
	{ 12, 12, 31, 12 }, { 6, 6, 2, 2},
	spell_fireproof, TAR_OBJ_INV, POS_STANDING, &gsn_fireproof,
	-1, 10, 6,
	"", "", "{R$p's protective aura fades.{x", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"flail",
	{ 1, 1, 1, 1 }, { 6, 3, 6, 4},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_flail,
	-1, 0, 0,
	"", "!Flail!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"flamestrike",
	{ 31, 17, 31, 31 }, { 6, 6, 2, 2},
	spell_flamestrike, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_flamestrike,
	-1, 20, 8,
	"flamestrike", "!Flamestrike!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"flash",
	{ 20, 31, 31, 31 }, { 15, 25, 25, 25 },
	spell_flash, TAR_IGNORE, POS_STANDING, &gsn_flash,
	0, 150, 10,
	"", "", "", "",
	{ { CATALYST_LIGHT, 5 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"flight",
	{ 1, 1, 1, 1 }, { 8, 8, 8, 8},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_flight,
	-1, 0, 12,
	"", "!Flight!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"fly",
	{ 10, 17, 31, 17 }, { 4, 4, 2, 2},
	spell_fly, TAR_CHAR_DEFENSIVE, POS_STANDING, &gsn_fly,
	-1, 10, 4,
	"", "{CYou slowly float to the ground.{x", "", "",
	{ { CATALYST_AIR, 2 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"fourth attack",
	{ 31, 31, 31, 24 }, { 20, 20, 10, 10},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_fourth_attack,
	-1, 0, 0,
	"", "!Fourth Attack!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"frenzy",
	{ 31, 24, 31, 28 }, { 1, 5, 2, 8},
	spell_frenzy, TAR_CHAR_DEFENSIVE, POS_STANDING, &gsn_frenzy,
	-1, 30, 8,
	"", "{CYour rage ebbs.{x", "", "",
	{ { CATALYST_MIND, 2 },{ CATALYST_CHAOS, 2 },{ CATALYST_NONE, 0 } }
}, {
	"frost barrier",
	{ 15, 31, 31, 31}, { 12, 12, 0, 0 },
	spell_frost_barrier, TAR_CHAR_DEFENSIVE, POS_STANDING, &gsn_frost_barrier,
	-1, 75, 8,
	"frost wave", "{BThe air around you heats up as the frost barrier dissipates.{x", "", "",
	{ { CATALYST_BODY, 2 },{ CATALYST_COSMIC, 1 },{ CATALYST_ICE, 2 } }
}, {
	"frost breath",
	{ 7, 7, 7, 7 }, { 10, 1, 2, 2},
	spell_frost_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_frost_breath,
	-1, 125, 2,
	"blast of frost", "!Frost Breath!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"gas breath",
	{ 15, 15, 15, 15 }, { 10, 1, 2, 2},
	spell_gas_breath, TAR_IGNORE, POS_FIGHTING, &gsn_gas_breath,
	-1, 175, 2,
	"blast of gas", "!Gas Breath!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"gate",
	{ 18, 18, 31, 31 }, { 8, 8, 2, 2},
	spell_gate, TAR_IGNORE_CHAR_DEF, POS_FIGHTING, &gsn_gate,
	-1, 80, 6,
	"", "!Gate!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"giant strength",
	{ 15, 31, 31, 18 }, { 4, 4, 2, 4},
	spell_giant_strength, TAR_CHAR_DEFENSIVE, POS_STANDING, &gsn_giant_strength,
	-1, 20, 4,
	"", "{YYou feel weaker.{x", "", "",
	{ { CATALYST_BODY, 2 },{ CATALYST_EARTH, 2 },{ CATALYST_NONE, 0 } }
}, {
	"glacial wave",
	{ 20, 31, 31, 31 }, { 15, 25, 25, 25 },
	spell_glacial_wave, TAR_IGNORE, POS_STANDING, &gsn_glacial_wave,
	0, 150, 10,
	"glacial wave", "", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"glorious bolt",
	{ 31, 25, 31, 31 }, { 2, 8, 4, 4},
	spell_glorious_bolt, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_glorious_bolt,
	-1, 200, 10,
	"glorious bolt", "!Glorious Bolt!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"haggle",
	{ 31, 31, 5, 31 }, { 5, 8, 3, 6},
	spell_null, TAR_IGNORE, POS_RESTING, &gsn_haggle,
	-1, 0, 0,
	"", "!Haggle!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"hand to hand",
	{ 1, 1, 1, 1 }, { 8, 5, 6, 4},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_hand_to_hand,
	-1, 0, 0,
	"", "!Hand to Hand!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"harm",
	{ 31, 24, 31, 24 }, { 1, 5, 2, 5},
	spell_harm, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_harm,
	-1, 35, 6,
	"harm spell", "!Harm!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"harpooning",
	{ 1, 1, 1, 1 }, { 4, 4, 4, 3},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_harpooning,
	-1, 0, 0,
	"", "!Thar She Blows!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"haste",
	{ 8, 31, 31, 31 }, { 6, 1, 2, 2},
	spell_haste, TAR_CHAR_DEFENSIVE, POS_FIGHTING, &gsn_haste,
	-1, 30, 6,
	"", "{YYou feel yourself slow down.{x", "", "",
	{ { CATALYST_BODY, 2 },{ CATALYST_COSMIC, 2 },{ CATALYST_CHAOS, 1 } }
}, {
	"heal",
	{ 15, 15, 15, 15 }, { 6, 6, 6, 6},
	spell_heal, TAR_CHAR_DEFENSIVE, POS_FIGHTING, &gsn_heal,
	-1, 50, 7,
	"", "!Heal!", "", "",
	{ { CATALYST_BODY, 5 },{ CATALYST_BLOOD, 2 },{ CATALYST_NONE, 0 } }
}, {
	"healing aura",
	{ 31, 31, 31, 15 }, { 6, 6, 2, 2},
	spell_healing_aura, TAR_CHAR_DEFENSIVE, POS_STANDING, &gsn_healing_aura,
	-1, 75, 6,
	"healing_aura", "Your healing aura fades.", "", "",
	{ { CATALYST_BODY, 2 },{ CATALYST_COSMIC, 2 },{ CATALYST_BLOOD, 4 } }
}, {
	"healing hands",
	{ 15, 15, 15, 14 }, { 5, 5, 5, 5},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_healing_hands,
	-1, 0, 12,
	"healing hands", "!Healing Hands!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"hide",
	{ 13, 13, 1, 13 }, { 4, 6, 6, 6},
	spell_null, TAR_IGNORE, POS_RESTING, &gsn_hide,
	-1, 0, 12,
	"", "!Hide!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"holdup",
	{ 31, 31, 10, 31 }, { 8, 8, 8, 8},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_holdup,
	-1, 0, 24,
	"holdup", "!Holdup!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"holy shield",
	{ 31, 31, 31, 12 }, { 2, 2, 4, 8},
	spell_holy_shield, TAR_OBJ_INV, POS_STANDING, &gsn_holy_shield,
	-1, 5, 2,
	"", "", "The runes on $p fade.", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"holy sword",
	{ 31, 31, 31, 14 }, { 2, 2, 4, 8},
	spell_holy_sword, TAR_OBJ_INV, POS_STANDING, &gsn_holy_sword,
	-1, 5, 2,
	"", "", "The runes on $p fade.", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"holy word",
	{ 31, 25, 31, 31 }, { 2, 8, 4, 4},
	spell_holy_word, TAR_IGNORE, POS_FIGHTING, &gsn_holy_word,
	-1, 200, 16,
	"divine wrath", "!Holy Word!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"holy wrath",
	{ 1, 1, 1, 1 }, { 5, 5, 5, 5},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_holy_wrath,
	PC_RACE_SLAYER, 0, 0,
	"", "!holy wrath!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"hunt",
	{ 17, 17, 17, 17 }, { 10, 10, 10, 4},
	spell_null, TAR_IGNORE, POS_RESTING, &gsn_hunt,
	-1, 0, 7,
	"", "!Hunt!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"ice shards",
	{ 7, 31, 31, 31 }, { 2, 2, 2, 2},
	spell_ice_shards, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_ice_shards,
	-1, 50, 6,
	"shards of ice", "!Ice Shards!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"ice storm",
	{ 15, 31, 31, 31 }, { 8, 15, 20, 20 },
	spell_ice_storm, TAR_IGNORE, POS_STANDING, &gsn_ice_storm,
	0, 110, 9,
	"", "", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"identify",
	{ 8, 10, 31, 31 }, { 2, 2, 2, 2},
	spell_identify, TAR_OBJ_INV, POS_STANDING, &gsn_identify,
	-1, 12, 4,
	"", "!Identify!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"improved invisibility",
	{ 9, 31, 31, 31 }, { 4, 1, 2, 2},
	spell_improved_invisibility, TAR_CHAR_SELF, POS_STANDING, &gsn_improved_invisibility,
	-1, 50, 8,
	"", "{CYou are no longer invisible.{x", "", "",
	{ { CATALYST_BODY, 2 },{ CATALYST_COSMIC, 2 },{ CATALYST_AIR, 5 } }
}, {
	"inferno",
	{ 18, 31, 31, 31 }, { 6, 6, 2, 2},
	spell_inferno, TAR_IGNORE, POS_STANDING, &gsn_inferno,
	-1, 50, 6,
	"inferno", "!Inferno!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"infravision",
	{ 4, 31, 31, 31 }, { 2, 1, 2, 2},
	spell_infravision, TAR_CHAR_DEFENSIVE, POS_STANDING, &gsn_infravision,
	-1, 5, 4,
	"", "{CYou no longer see in the dark.{x", "", "",
	{ { CATALYST_BODY, 2 },{ CATALYST_COSMIC, 2 },{ CATALYST_LIGHT, 1 } }
}, {
	"infuse",
	{ 31, 15, 31, 31}, { 20, 8, 20, 4},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_infuse,
	-1, 0, 12,
	"infuse", "!Infuse!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"intimidate",
	{ 31, 31, 31, 13 }, { 20, 20, 20, 5},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_intimidate,
	-1, 0, 24,
	"intimidate", "!Intimidate!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"invisibility",
	{ 5, 31, 31, 31 }, { 4, 1, 2, 2},
	spell_invis, TAR_OBJ_CHAR_DEF, POS_STANDING, &gsn_invis,
	-1, 5, 4,
	"", "{CYou are no longer invisible.{x", "$p fades into view.", "",
	{ { CATALYST_BODY, 2 },{ CATALYST_COSMIC, 2 },{ CATALYST_AIR, 2 } }
}, {
	"judge",
	{ 31, 31, 6, 31 }, { 8, 8, 8, 8},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_judge,
	-1, 0, 24,
	"judge", "!Judge!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"kick",
	{ 31, 8, 31, 8 }, { 20, 4, 6, 4},
	spell_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_kick,
	-1, 0, 12,
	"kick", "!Kick!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"kill",
	{ 12, 31, 31, 31 }, { 5, 5, 4, 4},
	spell_kill, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_kill,
	-1, 150, 10,
	"", "", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"leadership",
	{ 31, 31, 31, 18 }, { 8, 8, 8, 8},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_leadership,
	-1, 0, 0,
	"", "!Leadership!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"light shroud",
	{ 31, 31, 31, 16 }, { 10, 10, 2, 8},
	spell_light_shroud, TAR_CHAR_DEFENSIVE, POS_STANDING, &gsn_light_shroud,
	-1, 75, 8,
	"", "{WThe white shroud around your body fades.{x", "", "",
	{ { CATALYST_BODY, 2 },{ CATALYST_LIGHT, 2 },{ CATALYST_HOLY, 2 } }
}, {
	"lightning bolt",
	{ 3, 31, 31, 31 }, { 2, 2, 2, 2},
	spell_lightning_bolt, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_lightning_bolt,
	-1, 15, 6,
	"lightning bolt", "!Lightning Bolt!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"lightning breath",
	{ 21, 21, 21, 21 }, { 1, 1, 2, 2},
	spell_lightning_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_lightning_breath,
	-1, 150, 2,
	"blast of lightning", "!Lightning Breath!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"locate object",
	{ 9, 10, 31, 31 }, { 5, 5, 2, 2},
	spell_locate_object, TAR_IGNORE, POS_STANDING, &gsn_locate_object,
	-1, 20, 6,
	"", "!Locate Object!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"lore",
	{ 14, 11, 11, 31 }, { 6, 6, 4, 8},
	spell_null, TAR_IGNORE, POS_RESTING, &gsn_lore,
	-1, 0, 36,
	"", "!Lore!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"mace",
	{ 1, 1, 1, 1 }, { 5, 2, 3, 3},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_mace,
	-1, 0, 0,
	"", "!Mace!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"magic missile",
	{ 1, 31, 31, 31 }, { 1, 1, 2, 2},
	spell_magic_missile, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_magic_missile,
	-1, 15, 6,
	"magic missile", "!Magic Missile!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"martial arts",
	{ 31, 1, 5, 3 }, { 15, 10, 12, 10 },
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_martial_arts,
	-1, 0, 0,
	"", "", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"mass healing",
	{ 31, 27, 31, 31 }, { 8, 8, 4, 4},
	spell_mass_healing, TAR_IGNORE, POS_STANDING, &gsn_mass_healing,
	-1, 300, 8,
	"", "!Mass Healing!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"mass invis",
	{ 22, 31, 31, 31 }, { 8, 7, 2, 2},
	spell_mass_invis, TAR_IGNORE, POS_STANDING, &gsn_mass_invis,
	-1, 20, 8,
	"", "{CYou are no longer invisible.{x", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"master weather",
	{ 22, 31, 31, 31 }, { 8, 9, 12, 15 },
	spell_master_weather, TAR_IGNORE, POS_STANDING, &gsn_master_weather,
	-1, 200, 12,
	"master weather", "!Master Weather!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"maze",
	{ 22, 31, 31, 31 }, { 5, 5, 4, 4},
	spell_maze, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_maze,
	-1, 100, 8,
	"", "!Maze!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"meditation",
	{ 6, 6, 31, 31 }, { 5, 5, 8, 8},
	spell_null, TAR_IGNORE, POS_SLEEPING, &gsn_meditation,
	-1, 0, 0,
	"", "Meditation", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"mob lore",
	{ 31, 31, 14, 31 }, { 6, 6, 6, 6},
	spell_null, TAR_IGNORE, POS_RESTING, &gsn_mob_lore,
	-1, 0, 36,
	"", "!Mob Lore!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"momentary darkness",
	{ 20, 31, 31, 31 }, { 6, 6, 6, 6},
	spell_momentary_darkness, TAR_IGNORE, POS_FIGHTING, &gsn_momentary_darkness,
	-1, 500, 15,
	"momentary darkness", "!Momentary Darkness!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"morphlock",
	{ 31, 31, 31, 31 }, { 31, 31, 31, 31 },
	spell_morphlock, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_morphlock,
	-1, 50, 12,
	"", "Your body feels free.", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"mount and weapon style",
	{ 31, 31, 31, 6 }, { 10, 9, 5, 10},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_mount_and_weapon_style,
	-1, 0, 0,
	"", "!Mount And Weapon Style!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"music",
	{ 1, 1, 1, 1 }, { 2, 3, 5, 8},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_music,
	-1, 0, 12,
	"", "!Music!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"neurotoxin",
	{ 31, 31, 31, 31 }, {0,0,0,0},
	spell_toxin_neurotoxin, TAR_CHAR_OFFENSIVE, POS_STANDING, &gsn_neurotoxin,
	-1, 0, 1 ,
	"","","", { { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"nexus",
	{ 23, 31, 31, 31 }, { 8, 8, 4, 4},
	spell_nexus, TAR_IGNORE_CHAR_DEF, POS_STANDING, &gsn_nexus,
	-1, 150, 8,
	"", "!Nexus!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"parry",
	{ 31, 31, 31, 15 }, { 8, 8, 6, 8},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_parry,
	-1, 0, 0,
	"", "!Parry!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"pass door",
	{ 19, 31, 31, 31 }, { 6, 1, 2, 2},
	spell_pass_door, TAR_CHAR_SELF, POS_STANDING, &gsn_pass_door,
	-1, 20, 4,
	"", "{CYou feel solid again.{x", "", "",
	{ { CATALYST_BODY, 2 },{ CATALYST_COSMIC, 2 },{ CATALYST_AIR, 2 } }
}, {
	"peek",
	{ 31, 31, 11, 31 }, { 5, 7, 3, 6},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_peek,
	-1, 0, 0,
	"", "!Peek!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"pick lock",
	{ 31, 31, 7, 31 }, { 8, 8, 4, 8},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_pick_lock,
	-1, 0, 12,
	"", "!Pick!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"plague",
	{ 21, 21, 31, 23 }, { 3, 6, 2, 2},
	spell_plague, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_plague,
	-1, 20, 6,
	"sickness", "Your sores vanish.", "", "",
	{ { CATALYST_BODY, 5 },{ CATALYST_TOXIN, 2 },{ CATALYST_DEATH, 1 } }
}, {
	"poison",
	{ 12, 14, 31, 17 }, { 3, 6, 2, 2},
	spell_poison, TAR_OBJ_CHAR_OFF, POS_FIGHTING, &gsn_poison,
	-1, 10, 6,
	"poison", "You feel less sick.", "The poison on $p dries up.", "",
	{ { CATALYST_BODY, 5 },{ CATALYST_TOXIN, 2 },{ CATALYST_BLOOD, 1 } }
}, {
	"polearm",
	{ 1, 1, 1, 1 }, { 6, 6, 6, 4},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_polearm,
	-1, 0, 0,
	"", "!Polearm!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"possess",
	{ 31, 31, 31, 31 }, { 6, 6, 2, 2},
	spell_null, TAR_IGNORE_CHAR_DEF, POS_STANDING, &gsn_possess,
	-1, 100, 12,
	"Possess", "!Possess!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"pursuit",
	{ 31, 31, 31, 13}, { 10, 10, 10, 10},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_pursuit,
	-1, 0, 12,
	"pursuit", "!Persue!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"quarterstaff",
	{ 1, 1, 1, 1 }, { 3, 3, 3, 3},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_quarterstaff,
	-1, 0, 0,
	"", "!Quarterstaff!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"raise dead",
	{ 28, 31, 31, 31 }, { 12, 12, 12, 12},
	spell_raise_dead, TAR_OBJ_GROUND, POS_STANDING, &gsn_raise_dead,
	-1, 1000, 8,
	"", "", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"recharge",
	{ 9, 31, 31, 23 }, { 6, 1, 2, 2 },
	spell_recharge, TAR_OBJ_INV, POS_STANDING, &gsn_recharge,
	-1, 60, 6,
	"", "!Recharge!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"refresh",
	{ 31, 6, 31, 5 }, { 2, 6, 2, 2},
	spell_refresh, TAR_CHAR_DEFENSIVE, POS_FIGHTING, &gsn_refresh,
	-1, 12, 2,
	"refresh", "!Refresh!", "", "",
	{ { CATALYST_BODY, 1 },{ CATALYST_NATURE, 1 },{ CATALYST_NONE, 0 } }
}, {
	"regeneration",
	{ 31, 18, 31, 31 }, { 6, 8, 6, 6},
	spell_regeneration, TAR_CHAR_DEFENSIVE, POS_STANDING, &gsn_regeneration,
	-1, 75, 12,
	"regeneration", "You stop regenerating.", "", "",
	{ { CATALYST_BODY, 5 },{ CATALYST_COSMIC, 5 },{ CATALYST_BLOOD, 5 } }
}, {
	"remove curse",
	{ 31, 18, 31, 18 }, { 3, 6, 2, 2},
	spell_remove_curse, TAR_OBJ_CHAR_DEF, POS_STANDING, &gsn_remove_curse,
	-1, 5, 4,
	"", "!Remove Curse!", "", "",
	{ { CATALYST_BODY, 2 },{ CATALYST_NATURE, 1 },{ CATALYST_LAW, 2 } }
}, {
	"rending",
	{ 2, 2, 2, 1 }, { 8, 8, 7, 6},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_rending,
	PC_RACE_LICH, 0, 0,
	"", "!rending!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"repair",
	{ 11, 11, 11, 11 }, { 21, 15, 13, 11 },
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_repair,
	PC_RACE_DWARF, 0, 10,
	"", "", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"rescue",
	{ 31, 31, 31, 3 }, { 20, 20, 20, 4},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_rescue,
	-1, 0, 12,
	"", "!Rescue!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"resurrect",
	{ 1, 1, 1, 1 }, { 15, 15, 15, 15},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_resurrect,
	-1, 0, 24,
	"resurrect", "!Resurrect!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"reverie",
	{ 31, 15, 31, 15 }, { 8, 8, 8, 8},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_reverie,
	-1, 0, 0,
	"", "!Reverie!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"riding",
	{31,31,31,5}, {20,20,20,12},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_riding,
	0, 0, 0,
	"", "", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"room shield",
	{ 15, 31, 31, 31 }, { 6, 6, 2, 2},
	spell_room_shield, TAR_IGNORE, POS_STANDING, &gsn_room_shield,
	-1, 75, 6,
	"room shield", "!Room Shield!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"sanctuary",
	{ 20, 26, 31, 31 }, { 10, 10, 2, 2},
	spell_sanctuary, TAR_CHAR_DEFENSIVE, POS_STANDING, &gsn_sanctuary,
	-1, 75, 8,
	"", "{WThe white aura around your body fades.{x", "", "The white aura around $n's body vanishes.",
	{ { CATALYST_BODY, 2 },{ CATALYST_COSMIC, 1 },{ CATALYST_HOLY, 2 } }
}, {
	"scan",
	{ 31, 31, 8, 31 }, { 11, 11, 5, 8 },
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_scan,
	0, 0, 0,
	"", "", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"scribe",
	{ 31, 19, 31, 31 }, { 8, 8, 6, 4},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_scribe,
	-1, 0, 0,
	"", "!Scribe!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"scrolls",
	{ 1, 1, 1, 1 }, { 2, 3, 5, 8},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_scrolls,
	-1, 0, 24,
	"", "!Scrolls!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"scry",
	{16, 16, 16, 16 }, { 15, 17, 19, 25},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_scry,
	PC_RACE_LICH, 0, 12,
	"", "!Scry!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"second attack",
	{ 10, 10, 10, 8 }, { 10, 8, 5, 4},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_second_attack,
	-1, 0, 0,
	"", "!Second Attack!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"sense danger",
	{5, 4, 3, 3}, {8, 6, 5, 4},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_sense_danger,
	PC_RACE_SITH, 0, 0,
	"", "", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"shape",
	{ 1, 1, 1, 1 }, { 2, 2, 2, 2},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_shape,
	PC_RACE_VAMPIRE, 0, 12,
	"", "!Shape!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"shield and weapon style",
	{ 31, 31, 31, 6 }, { 10, 9, 5, 10},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_shield_weapon_style,
	-1, 0, 0,
	"", "!Shield And Weapon Style!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"shield block",
	{ 7, 7, 7, 7 }, { 6, 4, 6, 2},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_shield_block,
	-1, 0, 0,
	"", "!Shield!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"shield",
	{ 8, 31, 31, 8 }, { 2, 2, 2, 2},
	spell_shield, TAR_CHAR_DEFENSIVE, POS_STANDING, &gsn_shield,
	-1, 12, 4,
	"", "{WYour force shield shimmers then fades away.{x", "", "The shield protecting $n vanishes.",
	{ { CATALYST_BODY, 2 },{ CATALYST_COSMIC, 2 },{ CATALYST_NONE, 0 } }
}, {
	"shift",
	{ 1, 1, 1, 1 }, { 8, 8, 8, 8},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_shift,
	-1, 0, 12,
	"", "!Shift!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"shocking grasp",
	{ 10, 31, 31, 31 }, { 2, 2, 2, 2},
	spell_shocking_grasp, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_shocking_grasp,
	-1, 15, 4,
	"shocking grasp", "!Shocking Grasp!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"shriek",
	{ 7, 31, 31, 31 }, { 2, 2, 2, 2},
	spell_shriek, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_shriek,
	-1, 50, 6,
	"deafening shriek", "!SHRIEK!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"silence",
	{ 25, 31, 31, 31 }, { 8, 1, 2, 2},
	spell_silence, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_silence,
	-1, 50, 12,
	"", "Your throat clears.", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"single weapon style",
	{ 1, 1, 1, 1 }, { 5, 5, 5, 5},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_single_style,
	-1, 0, 0,
	"", "!Single Weapon Style!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"skull",
	{ 7, 7, 7, 7 }, { 2, 3, 5, 8},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_skull,
	-1, 0, 12,
	"", "!Skull!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"sleep",
	{ 16, 31, 31, 31 }, { 6, 6, 2, 2},
	spell_sleep, TAR_CHAR_OFFENSIVE, POS_STANDING, &gsn_sleep,
	-1, 15, 6,
	"", "You feel less tired.", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"slit throat",
	{ 31, 31, 19, 31 }, { 5, 5, 5, 5},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_slit_throat,
	-1, 0, 12,
	"breath", "!Slit Throat!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"slow",
	{ 23, 31, 31, 31 }, { 6, 6, 2, 2},
	spell_slow, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_slow,
	-1, 30, 8,
	"", "You feel yourself speed up.", "", "$n is no longer moving so slowly.",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"smite",
	{31, 31, 31, 15}, {22, 15, 23, 10},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_smite,
	-1, 0, 12,
	"strike", "", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"sneak",
	{ 10, 10, 10, 10 }, { 6, 4, 4, 6},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_sneak,
	-1, 0, 12,
	"", "You no longer feel stealthy.", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"soul essence",
	{ 30, 31, 31, 31 }, { 12, 12, 12, 12},
	spell_soul_essence, TAR_IGNORE, POS_STANDING, &gsn_soul_essence,
	-1, 1000, 20,
	"", "", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"spear",
	{ 1, 1, 1, 1 }, { 4, 4, 4, 3},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_spear,
	-1, 0, 0,
	"", "!Spear!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"spell deflection",
	{ 15, 31, 31, 31 }, { 14, 1, 2, 2},
	spell_spell_deflection, TAR_CHAR_DEFENSIVE, POS_FIGHTING, &gsn_spell_deflection,
	-1, 50, 6,
	"spell_spell_deflection", "The dazzling crimson aura around you dissipates.", "", "",
	{ { CATALYST_BODY, 2 },{ CATALYST_COSMIC, 2 },{ CATALYST_LAW, 5 } }
}, {
	"spell shield",
	{ 31, 31, 31, 20 }, { 4, 1, 2, 10},
	spell_spell_shield, TAR_CHAR_DEFENSIVE, POS_FIGHTING, &gsn_spell_shield,
	-1, 50, 8,
	"spell_shield", "The spell shield around you fades away.", "", "",
	{ { CATALYST_BODY, 2 },{ CATALYST_COSMIC, 2 },{ CATALYST_ENERGY, 2 } }
}, {
	"spell trap",
	{ 24, 31, 31, 31 }, { 8, 6, 2, 2},
	spell_spell_trap, TAR_IGNORE, POS_STANDING, &gsn_spell_trap,
	-1, 200, 12,
	"spell_trap", "!Spell Trap!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"spirit rack",
	{ 11, 11, 11, 11 }, { 7, 7, 7, 6},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_spirit_rack,
	PC_RACE_LICH, 0, 12,
	"spirit rack", "!spirit rack!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"stake",
	{ 1, 1, 1, 1 }, { 4, 4, 6, 3},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_stake,
	PC_RACE_SLAYER, 0, 9,
	"", "!Stake!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"starflare",
	{20, 31, 31, 31}, {9, 10, 15, 18},
	spell_starflare, TAR_CHAR_OFFENSIVE, POS_STANDING, &gsn_starflare,
	-1, 150, 12,
	"starflare", "", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"staves",
	{ 1, 1, 1, 1 }, { 2, 3, 5, 8},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_staves,
	-1, 0, 10,
	"", "!Staves!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"steal",
	{ 31, 31, 16, 31 }, { 20, 20, 4, 20},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_steal,
	-1, 0, 8,
	"", "!Steal!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"stone skin",
	{ 4, 6, 31, 21}, { 4, 4, 2, 2},
	spell_stone_skin, TAR_CHAR_SELF, POS_STANDING, &gsn_stone_skin,
	-1, 12, 4,
	"", "Your skin feels soft again.", "", "$n's skin regains its normal texture.",
	{ { CATALYST_BODY, 2 },{ CATALYST_EARTH, 3 },{ CATALYST_NONE, 0 } }
}, {
	"stone spikes",
	{16, 31, 31, 31}, {8, 12, 14, 15},
	spell_stone_spikes, TAR_IGNORE, POS_STANDING, &gsn_stone_spikes,
	0, 125, 10,
	"stone spikes", "", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"stone touch",
	{ 4, 6, 31, 21}, { 6, 6, 4, 4},
	spell_stone_touch, TAR_CHAR_OFFENSIVE, POS_STANDING, &gsn_stone_touch,
	-1, 150, 10,
	"", "Your skin feels soft again.", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"subvert",
	{ 31, 31, 26, 31 }, { 0, 0, 5, 0},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_subvert,
	-1, 0, 24,
	"subvert", "!Subvert!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"summon",
	{ 15, 12, 31, 31 }, { 6, 6, 2, 2},
	spell_summon, TAR_IGNORE_CHAR_DEF, POS_STANDING, &gsn_summon,
	-1, 50, 6,
	"", "!Summon!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"survey",
	{ 8, 8, 8, 8 }, { 8, 8, 8, 8},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_survey,
	-1, 0, 24,
	"survey", "!Survey!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"swerve",
	{ 1, 1, 1, 1 }, { 8, 8, 4, 6},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_swerve,
	PC_RACE_DROW, 0, 0,
	"", "!Swerve!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"sword and dagger style",
	{ 31, 31, 31, 6 }, { 10, 9, 5, 10},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_sword_and_dagger_style,
	-1, 0, 0,
	"deadly slice", "!Sword And Dagger Style!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"sword",
	{ 1, 1, 1, 1}, { 5, 6, 3, 2},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_sword,
	-1, 0, 0,
	"", "!sword!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"tail kick",
	{ 1, 1, 1, 1 }, { 5, 5, 5, 5},
	spell_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_tail_kick,
	PC_RACE_SITH, 0, 12,
	"tail kick", "!Tail Kick!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"tattoo",
	{ 31, 19, 31, 31 }, { 8, 8, 6, 4},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_tattoo,
	-1, 0, 0,
	"", "!Tattoo!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"temperance",
	{5,4,3,2}, {3,5,6,6},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_temperance,
	PC_RACE_VAMPIRE, 0, 12,
	"","","", { { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"third attack",
	{ 31, 31, 31, 18 }, { 20, 20, 10, 10},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_third_attack,
	-1, 0, 0,
	"", "!Third Attack!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"third eye",
	{ 31, 16, 31, 31 }, { 6, 5, 2, 2},
	spell_third_eye, TAR_OBJ_INV, POS_STANDING, &gsn_third_eye,
	-1, 55, 6,
	"", "!Third Eye!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"throw",
	{ 31, 12, 12, 31 }, { 0, 8, 8, 0},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_throw,
	-1, 0, 4,
	"throw", "!Throw!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"titanic attack",
	{ 1, 1, 1, 1 }, { 8, 8, 8, 8},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_titanic_attack,
	PC_RACE_TITAN, 0, 0,
	"", "!Titanic Attack!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"toxic fumes",
	{ 31, 31, 31, 31 }, { 31, 31, 31, 31},
	spell_toxic_fumes, TAR_CHAR_OFFENSIVE, POS_STANDING, &gsn_toxic_fumes,
	-1, 70, 10,
	"toxic fumes", "The affects of the toxic fumes subside.", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"toxins",
	{ 13, 13, 13, 13 }, { 5, 5, 5, 5},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_toxins,
	PC_RACE_SITH, 0, 12,
	"", "!Toxins!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"trackless step",
	{31, 7, 31, 31 }, {18, 10, 13, 11},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_trackless_step,
	0, 0, 0,
	"", "", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"trample",
	{31,31,31,11}, {23,22,23,11},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_trample,
	0, 0, 8,
	"charge", "", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"turn undead",
	{ 31, 20, 31, 31 }, { 8, 8, 8, 8},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_turn_undead,
	-1, 0, 12,
	"turn_undead", "!Turn Undead!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"two-handed weapon style",
	{ 31, 31, 31, 11 }, { 10, 9, 5, 10},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_two_handed_style,
	-1, 0, 0,
	"", "!Two Handed Weapon Style!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"underwater breathing",
	{ 7, 12, 31, 31 }, { 7, 1, 2, 2},
	spell_underwater_breathing, TAR_CHAR_SELF, POS_STANDING, &gsn_underwater_breathing,
	-1, 70, 10,
	"", "The gills behind your ears disappear.", "", "",
	{ { CATALYST_BODY, 2 },{ CATALYST_COSMIC, 2 },{ CATALYST_WATER, 2 } }
}, {
	"vision",
	{ 31, 9, 31, 31 }, { 3, 8, 2, 2},
	spell_vision, TAR_IGNORE, POS_STANDING, &gsn_vision,
	-1, 50, 6,
	"", "", "", "",
	{ { CATALYST_AIR, 2 },{ CATALYST_COSMIC, 2 },{ CATALYST_NONE, 0 } }
}, {
	"wands",
	{ 1, 1, 1, 1 }, { 2, 3, 5, 8},
	spell_null, TAR_IGNORE, POS_STANDING, &gsn_wands,
	-1, 0, 10,
	"", "!Wands!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"warcry",
	{ 31, 31, 31, 18 }, { 8, 8, 8, 8},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_warcry,
	-1, 0, 10,
	"", "Your adrenaline thins out and your breathing returns to normal.", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"water spells",
	{1, 31, 31, 31}, {15, 20, 30, 30},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_water_spells,
	-1, 0, 0,
	"", "", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"weaken",
	{ 13, 15, 31, 31 }, { 5, 5, 2, 2},
	spell_weaken, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_weaken,
	-1, 20, 6,
	"spell", "{YYou feel stronger.{x", "", "$n looks stronger.",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"weaving",
	{ 31, 31, 31, 15 }, { 8, 8, 8, 8},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_weaving,
	-1, 0, 12,
	"", "!Weaving!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"web",
	{ 24, 31, 31, 31 }, { 8, 1, 2, 2},
	spell_web, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_web,
	-1, 50, 12,
	"", "The webs holding you in place disappear.", "", "The webs around $n disappear.",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"whip",
	{ 1, 1, 1, 1}, { 6, 5, 5, 4},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_whip,
	-1, 0, 0,
	"", "!Whip!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"wilderness spear style",
	{ 31, 18, 31, 31 }, { 5, 8, 5, 5},
	spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_wilderness_spear_style,
	-1, 0, 0,
	"", "!Wilderness Spear Style!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"wind of confusion",
	{ 25, 31, 31, 31 }, { 14, 1, 2, 2},
	spell_wind_of_confusion, TAR_IGNORE, POS_FIGHTING, &gsn_wind_of_confusion,
	-1, 100, 12,
	"wind of confusion", "", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"wither",
	{ 14, 31, 31, 31 }, { 8, 6, 2, 2},
	spell_wither, TAR_IGNORE, POS_STANDING, &gsn_wither,
	-1, 50, 12,
	"wither", "!Wither!", "", "",
	{ { CATALYST_NONE, 0 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
}, {
	"word of recall",
	{ 12, 12, 12, 12 }, { 4, 4, 2, 2},
	spell_word_of_recall, TAR_CHAR_SELF, POS_RESTING, &gsn_word_of_recall,
	-1, 5, 8,
	"", "!Word of Recall!", "", "",
	{ { CATALYST_LAW, 5 },{ CATALYST_NONE, 0 },{ CATALYST_NONE, 0 } }
},

