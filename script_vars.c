#include "merc.h"
#include "db.h"
#include "scripts.h"
#include "wilds.h"

// Prototypes - obviously
pVARIABLE variable_new(void);					// Most likely just INTERNAL
void variable_freedata (pVARIABLE v);
void variable_add(ppVARIABLE list,pVARIABLE var);		// INTERNAL
pVARIABLE variable_create(ppVARIABLE list,char *name, bool index, bool clear);	// INTERNAL

pVARIABLE variable_head = NULL;
pVARIABLE variable_tail = NULL;
pVARIABLE variable_index_head = NULL;
pVARIABLE variable_index_tail = NULL;
pVARIABLE variable_freechain = NULL;

bool variable_validname(char *str)
{
	if(!str || !*str) return FALSE;

	while(*str) {
		if(!isprint(*str) || *str == '~') return FALSE;
		++str;
	}

	return TRUE;
}

pVARIABLE variable_new(void)
{
	pVARIABLE v;

	if (variable_freechain) {
		v = variable_freechain;
		variable_freechain = variable_freechain->next;
	} else
		v = (pVARIABLE)malloc(sizeof(VARIABLE));

	if(v) memset(v,0,sizeof(VARIABLE));

	return v;
}

void variable_freedata (pVARIABLE v)
{
	if(v->type == VAR_STRING && v->_.s) free_string(v->_.s);
}

void variable_free (pVARIABLE v)
{
	if(!v) return;

	variable_freedata(v);

	free_string(v->name);

	if(v->index) {
		if(v->global_prev) v->global_prev->global_next = v->global_next;
		else variable_index_head = v->global_next;
		if(variable_index_tail == v) variable_index_tail = v->global_prev;
	} else {
		if(v->global_prev) v->global_prev->global_next = v->global_next;
		else variable_head = v->global_next;
		if(variable_tail == v) variable_tail = v->global_prev;
	}
	if(v->global_next) v->global_next->global_prev = v->global_prev;

	v->global_prev = NULL;
	v->global_next = NULL;
	v->type = VAR_UNKNOWN;
	v->next = variable_freechain;
	variable_freechain = v;
}

void variable_freelist(ppVARIABLE list)
{
	pVARIABLE cur,next;

	for(cur = *list;cur;cur = next) {
		next = cur->next;
		variable_free(cur);
	}

	*list = NULL;
}

pVARIABLE variable_get(pVARIABLE list,char *name)
{
	for(;list;list = list->next) {
		if(!str_cmp(list->name,name)) break;
	}
	return list;
}

void variable_add(ppVARIABLE list,pVARIABLE var)
{
	register pVARIABLE cur;

	if(!*list)
		*list = var;
	else if(str_cmp((*list)->name,var->name) > 0) {
		var->next = *list;
		*list = var;
	} else {
		for(cur = *list; cur->next && str_cmp(cur->next->name,var->name) < 0;cur = cur->next);

		var->next = cur->next;
		cur->next = var;
	}
}

pVARIABLE variable_create(ppVARIABLE list,char *name, bool index, bool clear)
{
	pVARIABLE var;

	var = variable_get(*list,name);
	if(var) {
		if(clear) variable_freedata(var);
	} else {
		var = variable_new();
		if(var) {
			var->name = str_dup(name);
			variable_add(list,var);
			var->save = FALSE;
			var->index = index;

			if(index) {
				if(variable_index_tail) variable_index_tail->global_next = var;
				else variable_index_head = var;
				var->global_prev = variable_tail;
				variable_index_tail = var;
			} else {
				if(variable_tail) variable_tail->global_next = var;
				else variable_head = var;
				var->global_prev = variable_tail;
				variable_tail = var;
			}
		}
	}

	return var;
}

#define varset(n,c,t,v,f) \
bool variables_set_##n (ppVARIABLE list,char *name,t v) \
{ \
	pVARIABLE var = variable_create(list,name,FALSE,TRUE); \
 \
	if(!var) return FALSE; \
 \
	var->type = VAR_##c; \
	var->_.f = v; \
 \
	return TRUE; \
}

varset(integer,INTEGER,int,num,i)
varset(room,ROOM,ROOM_INDEX_DATA*,r,r)
varset(exit,EXIT,EXIT_DATA*,e,e)
varset(mobile,MOBILE,CHAR_DATA*,m,m)
varset(object,OBJECT,OBJ_DATA*,o,o)
varset(token,TOKEN,TOKEN_DATA*,t,t)
varset(affect,AFFECT,AFFECT_DATA*,aff,aff)

bool variables_set_skill (ppVARIABLE list,char *name,int sn)
{
	pVARIABLE var = variable_create(list,name,FALSE,TRUE);

	if(!var) return FALSE;

	var->type = VAR_SKILL;
	var->_.sn = sn;

	return TRUE;
}

bool variables_set_skillinfo (ppVARIABLE list,char *name,CHAR_DATA *owner, int sn)
{
	pVARIABLE var = variable_create(list,name,FALSE,TRUE);

	if(!var) return FALSE;

	var->type = VAR_SKILLINFO;
	var->_.sk.owner = owner;
	var->_.sk.sn = sn;

	return TRUE;
}

bool variables_setindex_integer (ppVARIABLE list,char *name,int num, bool saved)
{
	pVARIABLE var = variable_create(list,name,TRUE,TRUE);

	if(!var) return FALSE;

	var->type = VAR_INTEGER;
	var->_.i = num;
	var->save = saved;

	return TRUE;
}

bool variables_setindex_room (ppVARIABLE list,char *name,long vnum, bool saved)
{
	pVARIABLE var = variable_create(list,name,TRUE,TRUE);

	if(!var) return FALSE;

	var->type = VAR_ROOM;
	if(fBootDb)
		var->_.i = vnum;
	else
		var->_.r = get_room_index(vnum);
	var->save = saved;

	return TRUE;
}


// Only reason this is seperate is the shared handling
bool variables_setindex_string(ppVARIABLE list,char *name,char *str,bool shared, bool saved)
{
	pVARIABLE var = variable_create(list,name,TRUE,TRUE);

	if(!var) return FALSE;

	if(shared) {
		var->type = VAR_STRING_S;
		var->_.s = str;
	} else {
		var->type = VAR_STRING;
		var->_.s = str_dup(str);
	}
	var->save = saved;

	return TRUE;
}


// Only reason this is seperate is the shared handling
bool variables_set_string(ppVARIABLE list,char *name,char *str,bool shared)
{
	pVARIABLE var = variable_create(list,name,FALSE,TRUE);

	if(!var) return FALSE;

	if(shared) {
		var->type = VAR_STRING_S;
		var->_.s = str;
	} else {
		var->type = VAR_STRING;
		var->_.s = str_dup(str);
	}
	return TRUE;
}

bool variables_append_string(ppVARIABLE list,char *name,char *str)
{
	pVARIABLE var = variable_create(list,name,FALSE,FALSE);
	char *nstr;
	int len;

	if(!var) return FALSE;

	if(var->type == VAR_STRING_S) {
		len = strlen(var->_.s);
		nstr = alloc_mem(len+strlen(str)+1);
		if(!nstr) return FALSE;
		strcpy(nstr,var->_.s);
		strcpy(nstr+len,str);
		var->_.s = nstr;
	} else if(var->type == VAR_STRING) {
		len = strlen(var->_.s);
		nstr = alloc_mem(len+strlen(str)+1);
		if(!nstr) return FALSE;
		strcpy(nstr,var->_.s);
		strcpy(nstr+len,str);
		free_string(var->_.s);
		var->_.s = nstr;
	} else
		var->_.s = str_dup(str);
	var->type = VAR_STRING;
	return TRUE;
}

bool variables_format_string(ppVARIABLE list,char *name)
{
	pVARIABLE var = variable_get(*list,name);

	if(!var || (var->type != VAR_STRING_S && var->type != VAR_STRING)) return FALSE;

	var->_.s = format_string(var->_.s);
	var->type = VAR_STRING;
	return TRUE;
}


bool variable_remove(ppVARIABLE list,char *name)
{
	register pVARIABLE cur,prev;
	int test = 0;

	if(!*list) return FALSE;

	for(prev = NULL,cur = *list;cur && (test = str_cmp(cur->name,name)) < 0; prev = cur, cur = cur->next);

	if(!test) {
		if(prev) prev->next = cur->next;
		else *list = cur->next;

		variable_free(cur);
		return TRUE;
	}

	return FALSE;
}

bool variable_copy(ppVARIABLE list,char *oldname,char *newname)
{
	pVARIABLE oldv, newv;

	if(!(oldv = variable_get(*list,oldname))) return FALSE;

	if(!str_cmp(oldname,newname)) return TRUE;	// Copy to itself is.. dumb.

	if(!(newv = variable_create(list,newname,oldv->index,TRUE))) return FALSE;

	newv->type = oldv->type;
	newv->save = oldv->index ? oldv->save : FALSE;

	switch(newv->type) {
	case VAR_UNKNOWN:	break;
	case VAR_INTEGER:	newv->_.i = oldv->_.i; break;
	case VAR_STRING:	newv->_.s = str_dup(oldv->_.s); break;
	case VAR_STRING_S:	newv->_.s = oldv->_.s; break;
	case VAR_ROOM:		newv->_.r = oldv->_.r; break;
	case VAR_EXIT:		newv->_.e = oldv->_.e; break;
	case VAR_MOBILE:	newv->_.m = oldv->_.m; break;
	case VAR_OBJECT:	newv->_.o = oldv->_.o; break;
	case VAR_TOKEN:		newv->_.t = oldv->_.t; break;
	case VAR_SKILL:		newv->_.sn = oldv->_.sn; break;
	case VAR_SKILLINFO:	newv->_.sk.owner = oldv->_.sk.owner; newv->_.sk.sn = oldv->_.sk.sn; break;
	case VAR_AFFECT:	newv->_.aff = oldv->_.aff; break;
	}

	return TRUE;
}

bool variable_copyto(ppVARIABLE from,ppVARIABLE to,char *oldname,char *newname, bool index)
{
	pVARIABLE oldv, newv;

	if(from == to) return variable_copy(from,oldname,newname);

	if(!(oldv = variable_get(*from,oldname))) return FALSE;

	if(!(newv = variable_create(to,newname,index,TRUE))) return FALSE;

	newv->type = oldv->type;
	newv->save = oldv->index ? oldv->save : FALSE;

	switch(newv->type) {
	case VAR_UNKNOWN:	break;
	case VAR_INTEGER:	newv->_.i = oldv->_.i; break;
	case VAR_STRING:	newv->_.s = str_dup(oldv->_.s); break;
	case VAR_STRING_S:	newv->_.s = oldv->_.s; break;
	case VAR_ROOM:		newv->_.r = oldv->_.r; break;
	case VAR_EXIT:		newv->_.e = oldv->_.e; break;
	case VAR_MOBILE:	newv->_.m = oldv->_.m; break;
	case VAR_OBJECT:	newv->_.o = oldv->_.o; break;
	case VAR_TOKEN:		newv->_.t = oldv->_.t; break;
	case VAR_SKILL:		newv->_.sn = oldv->_.sn; break;
	case VAR_SKILLINFO:	newv->_.sk.owner = oldv->_.sk.owner; newv->_.sk.sn = oldv->_.sk.sn; break;
	case VAR_AFFECT:	newv->_.aff = oldv->_.aff; break;
	}

	return TRUE;
}

bool variable_copylist(ppVARIABLE from,ppVARIABLE to,bool index)
{
	pVARIABLE oldv, newv;

	if(from == to) return TRUE;

	for(oldv = *from; oldv; oldv = oldv->next) {
		if(!(newv = variable_create(to,oldv->name,index,TRUE))) continue;

		newv->type = oldv->type;
		newv->save = oldv->index ? oldv->save : FALSE;

		switch(newv->type) {
		case VAR_UNKNOWN:	break;
		case VAR_INTEGER:	newv->_.i = oldv->_.i; break;
		case VAR_STRING:	newv->_.s = str_dup(oldv->_.s); break;
		case VAR_STRING_S:	newv->_.s = oldv->_.s; break;
		case VAR_ROOM:		newv->_.r = oldv->_.r; break;
		case VAR_EXIT:		newv->_.e = oldv->_.e; break;
		case VAR_MOBILE:	newv->_.m = oldv->_.m; break;
		case VAR_OBJECT:	newv->_.o = oldv->_.o; break;
		case VAR_TOKEN:		newv->_.t = oldv->_.t; break;
		case VAR_SKILL:		newv->_.sn = oldv->_.sn; break;
		case VAR_SKILLINFO:	newv->_.sk.owner = oldv->_.sk.owner; newv->_.sk.sn = oldv->_.sk.sn; break;
		case VAR_AFFECT:	newv->_.aff = oldv->_.aff; break;
		}
	}

	return TRUE;
}

bool variable_setsave(pVARIABLE vars,char *name,bool state)
{
	pVARIABLE v = variable_get(vars,name);

	if(v) {
		v->save = state;
		return TRUE;
	}

	return FALSE;
}

void variable_clearfield(int type, void *ptr)
{
	register pVARIABLE cur = variable_head;

	if(!ptr) return;

	while(cur) {
		// Don't reset the type, just the pointer.
		if(cur->type == type && cur->_.raw == ptr)
			cur->_.raw = NULL;
		// Special case for MOBILES, clear out any skill reference.
		else if(cur->type == VAR_SKILLINFO && type == VAR_MOBILE && cur->_.sk.owner == ptr) {
			cur->_.sk.owner = NULL;
			cur->_.sk.sn = -1;
		}
		cur = cur->global_next;
	}
}

void variable_index_fix(void)
{
	register pVARIABLE cur = variable_index_head;

	while(cur) {
		if(cur->type == VAR_ROOM) {
			if(cur->_.i > 0) cur->_.r = get_room_index(cur->_.i);
		}
		cur = cur->global_next;
	}
}

void variable_fwrite(pVARIABLE var, FILE *fp)
{
	switch(var->type) {
	case VAR_INTEGER:
		fprintf(fp,"VarInt %s~ %d %d\n", var->name, var->save ? 1 : 0, var->_.i);
		break;
	case VAR_STRING:
	case VAR_STRING_S:	// They all get changed to shared on load...
		fprintf(fp,"VarStr %s~ %d %s\n", var->name, var->save ? 1 : 0, var->_.s ? var->_.s : "");
		break;

	case VAR_ROOM:
		if(var->_.r) {
			if(var->_.r->wilds)
				fprintf(fp,"VarWRoom %s~ %d %d %d %d\n", var->name, var->save ? 1 : 0, (int)var->_.r->wilds->uid, (int)var->_.r->x, (int)var->_.r->y);
			else if(var->_.r->source)
				fprintf(fp,"VarCRoom %s~ %d %d %d %d\n", var->name, var->save ? 1 : 0, (int)var->_.r->source->vnum, (int)var->_.r->id[0], (int)var->_.r->id[1]);
			else
				fprintf(fp,"VarRoom %s~ %d %d\n", var->name, var->save ? 1 : 0, (int)var->_.r->vnum);
		}
		break;
	case VAR_EXIT:
		if(var->_.e && var->_.e->from_room) {
			if(var->_.e->from_room->wilds)
				fprintf(fp,"VarWExit %s~ %d %d %d %d %d\n", var->name, var->save ? 1 : 0, (int)var->_.e->from_room->wilds->uid, (int)var->_.e->from_room->x, (int)var->_.e->from_room->y, var->_.e->orig_door);
			else if(var->_.e->from_room->source)
				fprintf(fp,"VarCExit %s~ %d %d %d %d %d\n", var->name, var->save ? 1 : 0, (int)var->_.e->from_room->source->vnum, (int)var->_.e->from_room->id[0], (int)var->_.e->from_room->id[1], var->_.e->orig_door);
			else
				fprintf(fp,"VarExit %s~ %d %d %d\n", var->name, var->save ? 1 : 0, (int)var->_.e->from_room->vnum, var->_.e->orig_door);
		}
		break;
	case VAR_MOBILE:
		if(var->_.m) {
			if(IS_NPC(var->_.m))
				fprintf(fp,"VarNPC %s~ %d %d %d\n", var->name, var->save ? 1 : 0, (int)var->_.m->id[0], (int)var->_.m->id[1]);
			else
				fprintf(fp,"VarPC %s~ %d %s~\n", var->name, var->save ? 1 : 0, var->_.m->name);
		}
		break;
	case VAR_OBJECT:
		if(var->_.o)
			fprintf(fp,"VarObj %s~ %d %d %d\n", var->name, var->save ? 1 : 0, (int)var->_.o->id[0], (int)var->_.o->id[1]);
		break;

	case VAR_TOKEN:
		if(var->_.t)
			fprintf(fp,"VarTok %s~ %d %d %d\n", var->name, var->save ? 1 : 0, (int)var->_.t->id[0], (int)var->_.t->id[1]);
		break;
	}
}

////////////////////////////////////////////////
//
// Script commands:
//
// varset name type value
//
//   types:				CALL
//     integer <number>			variable_set_integer
//     string <string>			variable_set_string (shared:FALSE)
//     room <entity>			variable_set_room
//     room <vnum>			variable_set_room
//     mobile <entity>			variable_set_mobile
//     mobile <location> <vnum>		variable_set_mobile
//     mobile <location> <name>		variable_set_mobile
//     mobile <mob_list> <vnum>		variable_set_mobile
//     mobile <mob_list> <name>		variable_set_mobile
//     player <entity>			variable_set_mobile
//     player <name>			variable_set_mobile
//     object <entity>			variable_set_object
//     object <location> <vnum>		variable_set_object
//     object <location> <name>		variable_set_object
//     object <obj_list> <vnum>		variable_set_object
//     object <obj_list> <name>		variable_set_object
//     carry <mobile> <vnum>		variable_set_object
//     carry <mobile> <name>		variable_set_object
//     content <object> <vnum>		variable_set_object
//     content <object> <name>		variable_set_object
//     token <token_list> <vnum>	variable_set_token
//     token <token_list> <name>	variable_set_token
//
// Note: <entity> refers to $( ) use
//
// varclear name
// varcopy old new
// varsave name on|off
//	Flags the variable for saving depending on the parameter.
//
// Variable names should be restricted to...
//	1) starting with an alpha character (A-Z and a-z)
//	2) followed by zero or more alphanumeric characters
// It would make checking for them much easier
//

