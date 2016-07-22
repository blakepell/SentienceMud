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


/* How to make a string look drunk... by Apex (robink@htsa.hva.nl)   
   Modified and enhanced for envy(2) by the Maniac from Mythran   
   Further mods/upgrades for ROM 2.4 by Kohl Desenee */
char *makedrunk (char *string, CHAR_DATA * ch)
{
    /* This structure defines all changes for a character */
    struct struckdrunk drunk[] =
    {
	{3, 10,
	    {"a", "a", "a", "A", "aa", "ah", "Ah", "ao", "aw", "oa", "ahhhh"}},
	{8, 5,
	    {"b", "b", "b", "B", "B", "vb"}},
	{3, 5,
	    {"c", "c", "C", "cj", "sj", "zj"}},
	{5, 2,
	    {"d", "d", "D"}},
	{3, 3,
	    {"e", "e", "eh", "E"}},
	{4, 5,
	    {"f", "f", "ff", "fff", "fFf", "F"}},
	{8, 2,
	    {"g", "g", "G"}},
	{9, 6,
	    {"h", "h", "hh", "hhh", "Hhh", "HhH", "H"}},
	{7, 6,
	    {"i", "i", "Iii", "ii", "iI", "Ii", "I"}},
	{9, 5,
	    {"j", "j", "jj", "Jj", "jJ", "J"}},
	{7, 2,
	    {"k", "k", "K"}},
	{3, 2,
	    {"l", "l", "L"}},
	{5, 8,
	    {"m", "m", "mm", "mmm", "mmmm", "mmmmm", "MmM", "mM", "M"}},
	{6, 6,
	    {"n", "n", "nn", "Nn", "nnn", "nNn", "N"}},
	{3, 6,
	    {"o", "o", "ooo", "ao", "aOoo", "Ooo", "ooOo"}},
	{3, 2,
	    {"p", "p", "P"}},
	{5, 5,
	    {"q", "q", "Q", "ku", "ququ", "kukeleku"}},
	{4, 2,
	    {"r", "r", "R"}},
	{2, 5,
	    {"s", "ss", "zzZzssZ", "ZSssS", "sSzzsss", "sSss"}},
	{5, 2,
	    {"t", "t", "T"}},
	{3, 6,
	    {"u", "u", "uh", "Uh", "Uhuhhuh", "uhU", "uhhu"}},
	{4, 2,
	    {"v", "v", "V"}},
	{4, 2,
	    {"w", "w", "W"}},
	{5, 6,
	    {"x", "x", "X", "ks", "iks", "kz", "xz"}},
	{3, 2,
	    {"y", "y", "Y"}},
	{2, 9,
	    {"z", "z", "ZzzZz", "Zzz", "Zsszzsz", "szz", "sZZz", "ZSz", "zZ", "Z"}}
    };

    char buf[1024];
    char temp;
    int pos = 0;
    int drunklevel;
    int randomnum;

    /* Check how drunk a person is... */
    if (IS_NPC(ch))
	drunklevel = 0;
    else
	drunklevel = ch->pcdata->condition[COND_DRUNK];

    if (drunklevel > 0)
    {
	do
	{
	    temp = toupper (*string);
	    if ((temp >= 'A') && (temp <= 'Z'))
	    {
		if (drunklevel > drunk[temp - 'A'].min_drunk_level)
		{
		    randomnum = number_range (0, drunk[temp - 'A'].number_of_rep);
		    strcpy (&buf[pos], drunk[temp - 'A'].replacement[randomnum]);
		    pos += strlen (drunk[temp - 'A'].replacement[randomnum]);
		}
		else
		    buf[pos++] = *string;
	    }
	    else
	    {
		if ((temp >= '0') && (temp <= '9'))
		{
		    temp = '0' + number_range (0, 9);
		    buf[pos++] = temp;
		}
		else
		    buf[pos++] = *string;
	    }
	}

	while (*string++);
	
	buf[pos] = '\0';          /* Mark end of the string... */
	strcpy(string, buf);
	
	return(string);
    }

    return (string);
}
