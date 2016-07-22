/***************************************************************************
 *                                                                         *
 *    Scripting engine rebuilt by Michael Kurtz (Nibelung)                 *
 *    Used with permission.                                                *
 *                                                                         *
 **************************************************************************/

#include "merc.h"
#include "scripts.h"

int mpcmd_lookup(char *command);
int opcmd_lookup(char *command);
int rpcmd_lookup(char *command);
int tpcmd_lookup(char *command,bool istoken);
void mob_interpret(SCRIPT_VARINFO *info, char *argument);
void obj_interpret(SCRIPT_VARINFO *info, char *argument);
void room_interpret(SCRIPT_VARINFO *info, char *argument);
void token_interpret(SCRIPT_VARINFO *info, char *argument);
void tokenother_interpret(SCRIPT_VARINFO *info, char *argument);
void script_interpret(SCRIPT_VARINFO *info, char *command);

