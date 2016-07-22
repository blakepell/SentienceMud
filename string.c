/***************************************************************************
 *  File: string.c                                                         *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
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
#include "tables.h"
#include "olc.h"

char *string_linedel(char *, int);
char *string_lineadd(char *, char *, int);
char *numlineas(char *);


void string_edit(CHAR_DATA *ch, char **pString)
{
    send_to_char(" {W-=======- Entering {BEDIT{W Mode -========-{X\n\r", ch);
    send_to_char(" {W    Type {B.h{W on a new line for help\n\r", ch);
    send_to_char(" {W Terminate with a {B~{W or {B@{W on a blank line.{X\n\r", ch);
    send_to_char(" {W-=======================================-{X\n\r\n\r", ch);

    if (*pString == NULL)
    {
        *pString = str_dup("");
    }
    else
    {
        **pString = '\0';
    }

    ch->desc->pString = pString;

    return;
}


void string_append(CHAR_DATA *ch, char **pString)
{
    send_to_char(" {W-=======- Entering {BAPPEND{W Mode -========-{X\n\r", ch);
    send_to_char(" {W    Type {B.h{W on a new line for help\n\r", ch);
    send_to_char(" {W Terminate with a {B~{W or {B@{W on a blank line.{X\n\r", ch);
    send_to_char(" {W-=======================================-{X\n\r\n\r", ch);

    if (*pString == NULL)
    {
        *pString = str_dup("");
    }
    send_to_char(numlineas(*pString), ch);

    /*
    if (*(*pString + strlen(*pString) - 1) != '\r')
	send_to_char("\n\r", ch); */

    ch->desc->pString = pString;

    return;
}


char *string_replace(char * orig, char * old, char * new)
{
    char xbuf[MAX_STRING_LENGTH];
    int i;

    xbuf[0] = '\0';
    strcpy(xbuf, orig);
    if (strstr(orig, old) != NULL)
    {
        i = strlen(orig) - strlen(strstr(orig, old));
        xbuf[i] = '\0';
        strcat(xbuf, new);
        strcat(xbuf, &orig[i+strlen(old)]);
        free_string(orig);
    }

    return str_dup(xbuf);
}


void string_add(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    /*
     * Thanks to James Seng
     */
    smash_tilde(argument);

    if (*argument == '.')
    {
        char arg1 [MAX_INPUT_LENGTH];
        char arg2 [MAX_INPUT_LENGTH];
        char arg3 [MAX_INPUT_LENGTH];
        char tmparg3 [MAX_INPUT_LENGTH];
	//char *col_line;

        argument = one_argument(argument, arg1);

	if (!str_cmp (arg1, "./"))
	{
		interpret(ch, argument);
		send_to_char ("Command performed.\n\r", ch);
		return;

	}

	argument = first_arg(argument, arg2, FALSE);
	strcpy(tmparg3, argument);
        argument = first_arg(argument, arg3, FALSE);


	 if (!str_cmp(arg1, ".c"))
        {
            send_to_char("String cleared.\n\r", ch);
	    free_string(*ch->desc->pString);
	    *ch->desc->pString = str_dup("");
            return;
        }

        if (!str_cmp(arg1, ".s"))
        {
            send_to_char("String so far:\n\r", ch);
	    send_to_char(numlineas(*ch->desc->pString), ch);
	    return;
        }

        if (!str_cmp(arg1, ".r"))
        {
            if (arg2[0] == '\0')
            {
                send_to_char(
                    "usage:  .r \"old string\" \"new string\"\n\r", ch);
                return;
            }

            *ch->desc->pString =
                string_replace(*ch->desc->pString, arg2, arg3);
            sprintf(buf, "'%s' replaced with '%s'.\n\r", arg2, arg3);
            send_to_char(buf, ch);
            return;
        }

        if (!str_cmp(arg1, ".f"))
        {
            *ch->desc->pString = format_string(*ch->desc->pString);
            send_to_char("String formatted.\n\r", ch);
            return;
        }

	if (!str_cmp(arg1, ".ld"))
	{
		*ch->desc->pString = string_linedel(*ch->desc->pString, atoi(arg2));
		send_to_char("Line deleted.\n\r", ch);
		return;
	}

	if (!str_cmp(arg1, ".li"))
	{
		*ch->desc->pString = string_lineadd(*ch->desc->pString, tmparg3, atoi(arg2));
		send_to_char("Line inserted.\n\r", ch);
		return;
	}

	if (!str_cmp(arg1, ".lr"))
	{
		*ch->desc->pString = string_linedel(*ch->desc->pString, atoi(arg2));
		*ch->desc->pString = string_lineadd(*ch->desc->pString, tmparg3, atoi(arg2));
		send_to_char("Line replaced.\n\r", ch);
		return;
	}

        if (!str_cmp(arg1, ".h"))
        {
            send_to_char("{WString edit help (commands on blank line):{X\n\r", ch);
            send_to_char("{M.r 'old' 'new'{X - replace a substring \n\r", ch);
            send_to_char("                          (requires '', \"\") \n\r", ch);
            send_to_char("{M.h{X               - get help (this info)\n\r", ch);
            send_to_char("{M.s{X               - show string so far  \n\r", ch);
            send_to_char("{M.f{X               - (word wrap) string  \n\r", ch);
            send_to_char("{M.c{X               - clear string so far \n\r", ch);
            send_to_char("{M.ld <num>{X    - delete line <num>\n\r", ch);
            send_to_char("{M.li <num> <str>{X- insert <str> before line <num>\n\r", ch);
            send_to_char("{M.lr <num> <str>{X- replace line <num> with <str>\n\r", ch);
	    send_to_char("{M./ <command>{X - do a regular command\n\r", ch);
	    send_to_char("{M@{X                - exit the editor          \n\r", ch);
            return;
        }

        send_to_char("SEdit:  Invalid dot command.\n\r", ch);
        return;
    }

    if (*argument == '~' || *argument == '@')
    {
#ifdef ___NO_COMPILE_IN_EDITOR___
	if (ch->desc->editor == ED_MPCODE)
	{
	    //MOB_INDEX_DATA *mob; // @@@NIB : 20070123 : not needed
	    int hash;
	    PROG_LIST *mpl;
	    SCRIPT_DATA *mpc;

	    EDIT_MPCODE(ch, mpc);

	    if(compile_script(ch,mpc,mpc->edit_src,IFC_M))
		send_to_char("Script saved...\n\r", ch);

#if 0
		// Commented out since they all point to the container pointer
		//	not the actual script code
	    if (mpc != NULL)
		for (hash = 0; hash < MAX_KEY_HASH; hash++)
		    for (mob = mob_index_hash[hash]; mob; mob = mob->next)
			for (mpl = mob->progs; mpl; mpl = mpl->next)
			    if (mpl->vnum == mpc->vnum)
			    {
				sprintf(buf, "Fixing mob %ld.\n\r", mob->vnum);
				send_to_char(buf, ch);
				mpl->code = mpc->code;
			    }
#endif
	}

	if (ch->desc->editor == ED_OPCODE) /* for the objprogs */
	{
	    //OBJ_INDEX_DATA *obj; // @@@NIB : 20070123 : not needed
	    int hash;
	    PROG_LIST *opl;
	    SCRIPT_DATA *opc;

	    EDIT_OPCODE(ch, opc);

	    if(compile_script(ch,opc,opc->edit_src,IFC_O))
		send_to_char("Script saved...\n\r", ch);

#if 0
	    if (opc != NULL)
		for (hash = 0; hash < MAX_KEY_HASH; hash++)
		    for (obj = obj_index_hash[hash]; obj; obj = obj->next)
			for (opl = obj->progs; opl; opl = opl->next)
			    if (opl->vnum == opc->vnum)
			    {
				sprintf(buf, "Fixing object %ld.\n\r", obj->vnum);
				send_to_char(buf, ch);
				opl->code = opc->code;
			    }
#endif
	}

	if (ch->desc->editor == ED_RPCODE) /* for the roomprogs */
	{
	    //ROOM_INDEX_DATA *room; // @@@NIB : 20070123 : not needed
	    int hash;
	    PROG_LIST *rpl;
	    SCRIPT_DATA *rpc;

	    EDIT_RPCODE(ch, rpc);

	    if(compile_script(ch,rpc,rpc->edit_src,IFC_R))
		send_to_char("Script saved...\n\r", ch);

#if 0
	    if (rpc != NULL)
		for (hash = 0; hash < MAX_KEY_HASH; hash++)
		    for (room = room_index_hash[hash]; room; room = room->next)
			for (rpl = room->progs->progs; rpl; rpl = rpl->next)
			    if (rpl->vnum == rpc->vnum)
			    {
				sprintf(buf, "Fixing room %ld.\n\r", room->vnum);
				send_to_char(buf, ch);
				rpl->code = rpc->code;
			    }
#endif
	}

#ifdef ___TOKEN_SCRIPTS_YAY___
	if (ch->desc->editor == ED_TPCODE) /* for the tokenprogs */
	{
	    int hash;
	    PROG_LIST *rpl;
	    SCRIPT_DATA *tpc;

	    EDIT_RPCODE(ch, tpc);

	    if(compile_script(ch,tpc,tpc->edit_src,IFC_T))
		send_to_char("Script saved...\n\r", ch);

	}
#endif
#endif

        ch->desc->pString = NULL;
        return;
    }

    strcpy(buf, *ch->desc->pString);

    /*
     * Truncate strings to MAX_STRING_LENGTH.
     * --------------------------------------
     */
    if (strlen(*ch->desc->pString) + strlen(argument) >= (MAX_STRING_LENGTH - 4))
    {
        send_to_char("String too long, last line skipped.\n\r", ch);

	/* Force character out of editing mode. */
        ch->desc->pString = NULL;
        return;
    }

    /*
     * Ensure no tilde's inside string.
     * --------------------------------
     */
    smash_tilde(argument);

    strcat(buf, argument);
    strcat(buf, "\n\r");
    free_string(*ch->desc->pString);
    *ch->desc->pString = str_dup(buf);
    return;
}


char *format_string_len(char *oldstring,int lens[][2], int lenc,bool mem)
{
	char xbuf[MAX_STRING_LENGTH];
	char xbuf2[MAX_STRING_LENGTH];
	char *rdesc;
	int i = 0, lines = 0, leni, len;
	bool cap = TRUE;

	xbuf[0] = xbuf2[0] = 0;

	// This collapses the string into one line
	for (rdesc = oldstring; *rdesc; rdesc++) {
		if (*rdesc=='\n') {
			if (xbuf2[i-1] != ' ') xbuf2[i++]=' ';
		} else if (*rdesc=='\r') ;
		else if (*rdesc==' ') {
			if (xbuf2[i-1] != ' ') xbuf2[i++]=' ';
		} else if (*rdesc==')') {
			if (xbuf2[i-1]==' ' && xbuf2[i-2]==' ' &&
				(xbuf2[i-3]=='.' || xbuf2[i-3]=='?' || xbuf2[i-3]=='!')) {
				xbuf2[i-2]=*rdesc;
				xbuf2[i-1]=' ';
				xbuf2[i]=' ';
				i++;
			} else
				xbuf2[i++]=*rdesc;
		} else if ((*rdesc=='.' || *rdesc=='?' || *rdesc=='!') && *(rdesc+1) == ' ') {
			if (xbuf2[i-1]==' ' && xbuf2[i-2]==' ' &&
				(xbuf2[i-3]=='.' || xbuf2[i-3]=='?' || xbuf2[i-3]=='!')) {
				xbuf2[i-2]=*rdesc;
				if (*(rdesc+1) != '\"') {
					xbuf2[i-1]=' ';
					xbuf2[i++]=' ';
				} else {
					xbuf2[i-1]='\"';
					xbuf2[i]=' ';
					xbuf2[i+1]=' ';
					i+=2;
					rdesc++;
				}
			} else {
				xbuf2[i]=*rdesc;
				if (*(rdesc+1) != '\"') {
					xbuf2[i+1]=' ';
					xbuf2[i+2]=' ';
					i += 3;
				} else {
					xbuf2[i+1]='\"';
					xbuf2[i+2]=' ';
					xbuf2[i+3]=' ';
					i += 4;
					rdesc++;
				}
			}
			cap = TRUE;
		} else {
			xbuf2[i]=*rdesc;
			if (cap) {
				cap = FALSE;
				xbuf2[i] = UPPER(xbuf2[i]);
			}
			i++;
		}
	}
	xbuf2[i]=0;

	rdesc=xbuf2;

	xbuf[0]=0;
	lines = 0;
	leni = 0;

	for (; ;) {
		// Get the line length for this line
		if((leni+1) < lenc && lines >= lens[leni+1][0])
			++leni;
		len = lens[leni][1];

		// Check if we are the end of the line
		for (i=0; i<len && *(rdesc+i); i++);
		// If the current line will fit completely, break
		if (i<len) break;

		// Find a line break
		for (i=len-(xbuf[0]?0:3) ; --i > 0 && *(rdesc+i)!=' ';);

		// Found a line break
		if (i > 0) {
			strncat(xbuf,rdesc,i);
			strcat(xbuf,"\n\r");
			rdesc += i+1;
			while (*rdesc == ' ') rdesc++;
		// The entire line has no breaks
		} else {
			bug ("No spaces", 0);
			strncat(xbuf,rdesc,len-2);
			strcat(xbuf,"-\n\r");
			rdesc += len - 2;
		}
	}

	// Strip off excess whitespace
	while (*(rdesc+i) && (*(rdesc+i)==' '||
		*(rdesc+i)=='\n'|| *(rdesc+i)=='\r')) i--;

	*(rdesc+i+1)=0;
	strcat(xbuf,rdesc);
	i = strlen(xbuf);
	if (xbuf[i-2] != '\n')
		strcat(xbuf,"\n\r");

	if(mem) free_string(oldstring);
	return(str_dup(xbuf));
}



char *format_string(char *oldstring)
{
	int lens[1][2] = { { 0,77 } };
	return format_string_len(oldstring,lens,1,TRUE);
}


char *first_arg(char *argument, char *arg_first, bool fCase)
{
    char cEnd;

    while (*argument == ' ')
	argument++;

    cEnd = ' ';
    if (*argument == '\'' || *argument == '"'
      || *argument == '%'  || *argument == '(')
    {
        if (*argument == '(')
        {
            cEnd = ')';
            argument++;
        }
        else cEnd = *argument++;
    }

    while (*argument != '\0')
    {
	if (*argument == cEnd)
	{
	    argument++;
	    break;
	}
    if (fCase) *arg_first = LOWER(*argument);
            else *arg_first = *argument;
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while (*argument == ' ')
	argument++;

    return argument;
}


/*
 * Used in olc_act.c for aedit_builder.
 */
char *string_unpad(char * argument)
{
    char buf[MAX_STRING_LENGTH];
    char *s;

    s = argument;

    while (*s == ' ')
        s++;

    strcpy(buf, s);
    s = buf;

    if (*s != '\0')
    {
        while (*s != '\0')
            s++;
        s--;

        while(*s == ' ')
            s--;
        s++;
        *s = '\0';
    }

    free_string(argument);
    return str_dup(buf);
}


/*
 * Same as capitalize but changes the pointer's data.
 * Used in olc_act.c in aedit_builder.
 */
char *string_proper(char * argument)
{
    char *s;

    s = argument;

    while (*s != '\0')
    {
        if (*s != ' ')
        {
            *s = UPPER(*s);
            while (*s != ' ' && *s != '\0')
                s++;
        }
        else
        {
            s++;
        }
    }

    return argument;
}


char *string_linedel(char *string, int line)
{
    char *strtmp = string;
    char buf[MAX_STRING_LENGTH];
    int cnt = 1, tmp = 0;

    buf[0] = '\0';

    for (; *strtmp != '\0'; strtmp++)
    {
	if (cnt != line)
	    buf[tmp++] = *strtmp;

	if (*strtmp == '\n')
	{
	    if (*(strtmp + 1) == '\r')
	    {
		if (cnt != line)
		    buf[tmp++] = *(++strtmp);
		else
		    ++strtmp;
	    }

	    cnt++;
	}
    }

    buf[tmp] = '\0';

    free_string(string);
    return str_dup(buf);
}

char *string_lineadd(char *string, char *newstr, int line)
{
    char *strtmp = string;
    int cnt = 1, tmp = 0;
    bool done = FALSE;
    char buf[MAX_STRING_LENGTH];

    buf[0] = '\0';

    for (; *strtmp != '\0' || (!done && cnt == line); strtmp++)
    {
	if (cnt == line && !done)
	{
	    strcat(buf, newstr);
	    strcat(buf, "\n\r");
	    tmp += strlen(newstr) + 2;
	    cnt++;
	    done = TRUE;
	}

	buf[tmp++] = *strtmp;

	if (done && *strtmp == '\0')
	    break;

	if (*strtmp == '\n')
	{
	    if (*(strtmp + 1) == '\r')
		buf[tmp++] = *(++strtmp);

	    cnt++;
	}

	buf[tmp] = '\0';
    }

    free_string(string);
    return str_dup(buf);
}


char *olc_getline(char *str, char *buf)
{
    int tmp = 0;
    bool found = FALSE;

    while (*str)
    {
	if (*str == '\n')
	{
	    found = TRUE;
	    break;
	}

	buf[tmp++] = *(str++);
    }

    if (found)
    {
	if (*(str + 1) == '\r')
	    str += 2;
	else
	    str += 1;
    }

    buf[tmp] = '\0';

    return str;
}


char *numlineas(char *string)
{
    int cnt = 1;
    static char buf[MAX_STRING_LENGTH*2];
    char buf2[MAX_STRING_LENGTH], tmpb[MAX_STRING_LENGTH];

    buf[0] = '\0';

    while (*string)
    {
	string = olc_getline(string, tmpb);
	sprintf(buf2, "%2d. %s\n\r", cnt++, tmpb);
	strcat(buf, buf2);
    }

    return buf;
}


bool string_argremove_index(char *src, int argindex, char *buf)
{
	char *left_start = src;
	char *left_end = src;
	char *right_start;
	char arg[MIL];
	int i;

	if( !src || !buf || argindex < 0 ) return FALSE;

	strcpy(buf, src);


	// Find the left part
	for(i = 0; i < argindex; i++) {
		left_end = one_argument(left_end, arg);
	}

	if(IS_NULLSTR(left_end)) {
		strcpy(buf, src);
	}

	// Get the right part
	right_start = one_argument(left_end, arg);

	strcpy(buf + (int)(left_end - left_start), right_start);
	return TRUE;
}

bool string_argremove_phrase(char *src, char *phrase, char *buf)
{
	char *left_start = src;
	char *left_end = src;
	char arg[MIL];

	if( !src || !buf || IS_NULLSTR(phrase) ) return FALSE;

	strcpy(buf, src);

	while(!IS_NULLSTR(left_end)) {
		char *rest = one_argument(left_end, arg);

		if(!str_cmp(phrase, arg)) {
			// If the phrase matches, copy the rest to where the left ended.
			strcpy(buf + (int)(left_end - left_start), rest);
			break;

		} else
			left_end = rest;
	}

	return TRUE;
}
