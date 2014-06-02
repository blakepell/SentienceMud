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

static void *deepcopy_string(void *str)
{
	return str_dup((char *)str);
}

static void deleter_string(void *str) { free_string((char *)str); }

static void *deepcopy_room(void *src)
{
	LIST_ROOM_DATA *room = (LIST_ROOM_DATA *)src;

	LIST_ROOM_DATA *data;

	if( (data = alloc_mem(sizeof(LIST_ROOM_DATA))) )
	{
		data->room = room->room;
		data->id[0] = room->id[0];
		data->id[1] = room->id[1];
		data->id[2] = room->id[2];
		data->id[3] = room->id[3];
	}

	return data;
}

static void *deepcopy_uid(void *src)
{
	LIST_UID_DATA *uid = (LIST_UID_DATA *)src;

	LIST_UID_DATA *data;

	if( (data = alloc_mem(sizeof(LIST_UID_DATA))) )
	{
		data->ptr = uid->ptr;
		data->id[0] = uid->id[0];
		data->id[1] = uid->id[1];
	}

	return data;
}

static void *deepcopy_exit(void *src)
{
	LIST_EXIT_DATA *ex = (LIST_EXIT_DATA *)src;

	LIST_EXIT_DATA *data;

	if( (data = alloc_mem(sizeof(LIST_EXIT_DATA))) )
	{
		data->room = ex->room;
		data->id[0] = ex->id[0];
		data->id[1] = ex->id[1];
		data->id[2] = ex->id[2];
		data->id[3] = ex->id[3];
		data->door = ex->door;
	}

	return data;
}

static void *deepcopy_skill(void *src)
{
	LIST_SKILL_DATA *skill = (LIST_SKILL_DATA *)src;

	LIST_SKILL_DATA *data;

	if( (data = alloc_mem(sizeof(LIST_SKILL_DATA))) )
	{
		data->mob = skill->mob;
		data->sn = skill->sn;
		data->tok = skill->tok;
		data->mid[0] = skill->mid[0];
		data->mid[1] = skill->mid[1];
		data->tid[0] = skill->tid[0];
		data->tid[1] = skill->tid[1];
	}

	return data;
}

static void *deepcopy_area(void *src)
{
	LIST_AREA_DATA *area = (LIST_AREA_DATA *)src;

	LIST_AREA_DATA *data;

	if( (data = alloc_mem(sizeof(LIST_AREA_DATA))) )
	{
		data->area = area->area;
		data->uid = area->uid;
	}

	return data;
}


static void *deepcopy_wilds(void *src)
{
	LIST_WILDS_DATA *wilds = (LIST_WILDS_DATA *)src;

	LIST_WILDS_DATA *data;

	if( (data = alloc_mem(sizeof(LIST_WILDS_DATA))) )
	{
		data->wilds = wilds->wilds;
		data->uid = wilds->uid;
	}

	return data;
}


static LISTCOPY_FUNC *__var_blist_copier[] = {
	NULL,
	deepcopy_room,
	deepcopy_uid,
	deepcopy_uid,
	deepcopy_uid,
	deepcopy_exit,
	deepcopy_skill,
	deepcopy_area,
	deepcopy_wilds,
	NULL
};

static void deleter_room(void *data) { free_mem(data, sizeof(LIST_ROOM_DATA)); }
static void deleter_uid(void *data) { free_mem(data, sizeof(LIST_UID_DATA)); }
static void deleter_exit(void *data) { free_mem(data, sizeof(LIST_EXIT_DATA)); }
static void deleter_skill(void *data) { free_mem(data, sizeof(LIST_SKILL_DATA)); }
static void deleter_area(void *data) { free_mem(data, sizeof(LIST_AREA_DATA)); }
static void deleter_wilds(void *data) { free_mem(data, sizeof(LIST_WILDS_DATA)); }

static LISTDESTROY_FUNC *__var_blist_deleter[] = {
	NULL,
	deleter_room,
	deleter_uid,
	deleter_uid,
	deleter_uid,
	deleter_exit,
	deleter_skill,
	deleter_area,
	deleter_wilds,
	NULL
};

static int __var_blist_size[] = {
	0,
	sizeof(LIST_ROOM_DATA),
	sizeof(LIST_UID_DATA),
	sizeof(LIST_UID_DATA),
	sizeof(LIST_UID_DATA),
	sizeof(LIST_EXIT_DATA),
	sizeof(LIST_SKILL_DATA),
	sizeof(LIST_AREA_DATA),
	sizeof(LIST_WILDS_DATA),
	0
};

void variable_freedata (pVARIABLE v)
{
	ITERATOR it;

	switch( v->type ) {
	case VAR_STRING:
		if( v->_.s ) free_string(v->_.s);
		break;

	/*
	// This should be handled by the list_destroy itself
	case VAR_PLIST_STR:
		// Strings needs to be freed, if they are not shared strings
		if( v->_.list ) {
			char *str;
			iterator_start(&it, v->_.list);

			while((str = (char*)iterator_nextdata(&it)))
				free_string(str);

			iterator_stop(&it);
		}
		break;

	// These have special structures for referencing them
	case VAR_BLIST_ROOM:
	case VAR_BLIST_MOB:
	case VAR_BLIST_OBJ:
	case VAR_BLIST_TOK:
	case VAR_BLIST_EXIT:
	case VAR_BLIST_SKILL:
	case VAR_BLIST_AREA:
	case VAR_BLIST_WILDS:
		if( v->_.list ) {
			void *ptr;
			iterator_start(&it, v->_.list);

			while((ptr = iterator_nextdata(&it)))
				free_mem(ptr, __var_blist_size[v->type - VAR_BLIST_FIRST]);

			iterator_stop(&it);
		}
		break;
	*/
	}


	// For all list variables, just remove the reference added when created, this will autopurge the list
	// 20140511 NIB - nope, the use of the purge would not allow for list culling
	if (v->type >= VAR_BLIST_FIRST && v->type <= VAR_BLIST_LAST && v->_.list ) {
//		list_remref(v->_.list);
		list_destroy(v->_.list);
		v->_.list = NULL;
	}

	if (v->type >= VAR_PLIST_FIRST && v->type <= VAR_PLIST_LAST && v->_.list ) {
//		list_remref(v->_.list);
		list_destroy(v->_.list);
		v->_.list = NULL;
	}
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

static void variable_dump_list(pVARIABLE list, char *prefix)
{
	while(list) {
		log_stringf("%s: %s", prefix, list->name);

		list = list->global_next;
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
				var->global_prev = variable_index_tail;
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
bool variables_setsave_##n (ppVARIABLE list,char *name,t v, bool save) \
{ \
	pVARIABLE var = variable_create(list,name,FALSE,TRUE); \
 \
	if(!var) return FALSE; \
 \
	var->type = VAR_##c; \
	if( save != TRISTATE ) \
		var->save = save; \
	var->_.f = v; \
 \
	return TRUE; \
} \
 \
bool variables_set_##n (ppVARIABLE list,char *name,t v) \
{ \
	return variables_setsave_##n (list, name, v, TRISTATE); \
} \


varset(integer,INTEGER,int,num,i)
varset(room,ROOM,ROOM_INDEX_DATA*,r,r)
varset(mobile,MOBILE,CHAR_DATA*,m,m)
varset(object,OBJECT,OBJ_DATA*,o,o)
varset(token,TOKEN,TOKEN_DATA*,t,t)
varset(area,AREA,AREA_DATA*,a,a)
varset(wilds,WILDS,WILDS_DATA*,wilds,wilds)
varset(church,CHURCH,CHURCH_DATA*,church,church)
varset(affect,AFFECT,AFFECT_DATA*,aff,aff)

bool variables_set_door (ppVARIABLE list,char *name, ROOM_INDEX_DATA *room, int door, bool save)
{
	pVARIABLE var = variable_create(list,name,FALSE,TRUE);

	if(!var) return FALSE;

	var->type = VAR_EXIT;
	if( save != TRISTATE )
		var->save = save;
	var->_.door.r = room;
	var->_.door.door = door;

	return TRUE;
}


bool variables_set_exit (ppVARIABLE list,char *name, EXIT_DATA *ex)
{
	if( !ex || !ex->from_room) return FALSE;

	return variables_set_door( list, name, ex->from_room, ex->orig_door, TRISTATE );
}

bool variables_setsave_exit (ppVARIABLE list,char *name, EXIT_DATA *ex, bool save)
{
	if( !ex || !ex->from_room) return FALSE;

	return variables_set_door( list, name, ex->from_room, ex->orig_door, save);
}


bool variables_set_skill (ppVARIABLE list,char *name,int sn)
{
	return variables_setsave_skill( list, name, sn, TRISTATE );
}

bool variables_setsave_skill (ppVARIABLE list,char *name,int sn, bool save)
{
	pVARIABLE var = variable_create(list,name,FALSE,TRUE);

	if(!var) return FALSE;

	var->type = VAR_SKILL;
	if( save != TRISTATE )
		var->save = save;
	var->_.sn =  (sn > 0 && sn < MAX_SKILL) ? sn : 0;

	return TRUE;
}

bool variables_set_skillinfo (ppVARIABLE list,char *name,CHAR_DATA *owner, int sn, TOKEN_DATA *token)
{
	return variables_setsave_skillinfo( list, name, owner, sn, token, TRISTATE );
}

bool variables_setsave_skillinfo (ppVARIABLE list,char *name,CHAR_DATA *owner, int sn, TOKEN_DATA *token, bool save)
{
	pVARIABLE var = variable_create(list,name,FALSE,TRUE);

	if(!var) return FALSE;

	var->type = VAR_SKILLINFO;
	if( save != TRISTATE )
		var->save = save;
	var->_.sk.owner = owner;
	var->_.sk.token = token;
	var->_.sk.sn =  (sn > 0 && sn < MAX_SKILL) ? sn : 0;

	return TRUE;
}

static bool variables_set_mobile_id (ppVARIABLE list,char *name,unsigned long a, unsigned long b, bool save)
{
	pVARIABLE var = variable_create(list,name,FALSE,TRUE);

	if(!var) return FALSE;

	var->type = VAR_MOBILE_ID;
	var->save = save;
	var->_.mid.a = a;
	var->_.mid.b = b;

	return TRUE;
}

static bool variables_set_object_id (ppVARIABLE list,char *name,unsigned long a, unsigned long b, bool save)
{
	pVARIABLE var = variable_create(list,name,FALSE,TRUE);

	if(!var) return FALSE;

	var->type = VAR_OBJECT_ID;
	var->save = save;
	var->_.oid.a = a;
	var->_.oid.b = b;

	return TRUE;
}

static bool variables_set_token_id (ppVARIABLE list,char *name,unsigned long a, unsigned long b, bool save)
{
	pVARIABLE var = variable_create(list,name,FALSE,TRUE);

	if(!var) return FALSE;

	var->type = VAR_TOKEN_ID;
	var->save = save;
	var->_.tid.a = a;
	var->_.tid.b = b;

	return TRUE;
}

static bool variables_set_area_id (ppVARIABLE list,char *name, long aid, bool save)
{
	pVARIABLE var = variable_create(list,name,FALSE,TRUE);

	if(!var) return FALSE;

	var->type = VAR_AREA_ID;
	var->save = save;
	var->_.aid = aid;

	return TRUE;
}

static bool variables_set_wilds_id (ppVARIABLE list,char *name, long wid, bool save)
{
	pVARIABLE var = variable_create(list,name,FALSE,TRUE);

	if(!var) return FALSE;

	var->type = VAR_WILDS_ID;
	var->save = save;
	var->_.wid = wid;

	return TRUE;
}

static bool variables_set_church_id (ppVARIABLE list,char *name, long chid, bool save)
{
	pVARIABLE var = variable_create(list,name,FALSE,TRUE);

	if(!var) return FALSE;

	var->type = VAR_CHURCH_ID;
	var->save = save;
	var->_.chid = chid;

	return TRUE;
}

static bool variables_set_clone_room (ppVARIABLE list,char *name, ROOM_INDEX_DATA *source,unsigned long a, unsigned long b, bool save)
{
	pVARIABLE var = variable_create(list,name,FALSE,TRUE);

	if(!var) return FALSE;

	var->type = VAR_CLONE_ROOM;
	var->save = save;
	var->_.cr.r = source;
	var->_.cr.a = a;
	var->_.cr.b = b;

	return TRUE;
}

static bool variables_set_wilds_room (ppVARIABLE list,char *name, unsigned long w, int x, int y, bool save)
{
	pVARIABLE var = variable_create(list,name,FALSE,TRUE);

	if(!var) return FALSE;

	var->type = VAR_WILDS_ROOM;
	var->save = save;
	var->_.wroom.wuid = w;
	var->_.wroom.x = x;
	var->_.wroom.y = y;

	return TRUE;
}

static bool variables_set_clone_door (ppVARIABLE list,char *name, ROOM_INDEX_DATA *source,unsigned long a, unsigned long b, int door, bool save)
{
	pVARIABLE var = variable_create(list,name,FALSE,TRUE);

	if(!var) return FALSE;

	var->type = VAR_CLONE_DOOR;
	var->save = save;
	var->_.cdoor.r = source;
	var->_.cdoor.a = a;
	var->_.cdoor.b = b;
	var->_.cdoor.door = door;

	return TRUE;
}

static bool variables_set_wilds_door (ppVARIABLE list,char *name, unsigned long w, int x, int y, int door, bool save)
{
	pVARIABLE var = variable_create(list,name,FALSE,TRUE);

	if(!var) return FALSE;

	var->type = VAR_WILDS_DOOR;
	var->save = save;
	var->_.wdoor.wuid = w;
	var->_.wdoor.x = x;
	var->_.wdoor.y = y;
	var->_.wdoor.door = door;

	return TRUE;
}

static bool variables_set_skillinfo_id (ppVARIABLE list,char *name, unsigned long ma, unsigned long mb, unsigned long ta, unsigned long tb, int sn, bool save)
{
	pVARIABLE var = variable_create(list,name,FALSE,TRUE);

	if(!var) return FALSE;

	var->type = VAR_SKILLINFO_ID;
	var->save = save;
	var->_.skid.mid[0] = ma;
	var->_.skid.mid[1] = mb;
	var->_.skid.tid[0] = ta;
	var->_.skid.tid[1] = tb;
	var->_.skid.sn = (sn > 0 && sn < MAX_SKILL) ? sn : 0;

	return TRUE;
}

bool variables_set_list (ppVARIABLE list, char *name, int type, bool save)
{
	pVARIABLE var = variable_create(list,name,FALSE,TRUE);

	if(!var) return FALSE;

	var->type = type;
	var->save = save;

	if( type > VAR_BLIST_FIRST && type < VAR_BLIST_LAST )
		var->_.list = list_createx(FALSE, __var_blist_copier[type - VAR_BLIST_FIRST], __var_blist_deleter[type - VAR_BLIST_FIRST]);

	else if( type == VAR_PLIST_STR )
		var->_.list = list_createx(FALSE, deepcopy_string, deleter_string);

	else
		var->_.list = list_create(FALSE);


	// 20140511 NIB - the use of the purge flag here would not allow for list culling
	//if(var->_.list)
	//	list_addref(var->_.list);

	return TRUE;
}


bool variables_set_connection (ppVARIABLE list, char *name, DESCRIPTOR_DATA *conn)
{
	pVARIABLE var = variable_create(list,name,FALSE,TRUE);

	if(!var) return FALSE;

	var->type = VAR_CONNECTION;
	var->save = FALSE;					// These will never save
	var->_.conn = conn;

	return TRUE;
}

bool variables_append_list_str (ppVARIABLE list, char *name, char *str)
{
	char *cpy;
	pVARIABLE var = variable_get(*list, name);

	if( !str || !var || var->type != VAR_PLIST_STR) return FALSE;

	cpy = str_dup(str);
	if( !list_appendlink(var->_.list, cpy) )
		free_string(cpy);

	return TRUE;
}

// Used for loading purposes
static bool variables_append_list_uid (ppVARIABLE list, char *name, int type, unsigned long a, unsigned long b)
{
	LIST_UID_DATA *data;
	pVARIABLE var = variable_get(*list, name);

	if( (!a && !b)  || !var || var->type != type) return FALSE;

	if( !(data = alloc_mem(sizeof(LIST_UID_DATA))) ) return FALSE;

	data->ptr = NULL;
	data->id[0] = a;
	data->id[1] = b;

	if( !list_appendlink(var->_.list, data) )
		free_mem(data,sizeof(LIST_UID_DATA));

	return TRUE;
}

bool variables_append_list_mob (ppVARIABLE list, char *name, CHAR_DATA *mob)
{
	LIST_UID_DATA *data;
	pVARIABLE var = variable_get(*list, name);

	if( !mob || !var || var->type != VAR_BLIST_MOB) return FALSE;

	if( !(data = alloc_mem(sizeof(LIST_UID_DATA))) ) return FALSE;

	data->ptr = mob;
	data->id[0] = mob->id[0];
	data->id[1] = mob->id[1];

	if( !list_appendlink(var->_.list, data) )
		free_mem(data,sizeof(LIST_UID_DATA));

	return TRUE;
}

bool variables_append_list_obj (ppVARIABLE list, char *name, OBJ_DATA *obj)
{
	LIST_UID_DATA *data;
	pVARIABLE var = variable_get(*list, name);

	if( !obj || !var || var->type != VAR_BLIST_OBJ) return FALSE;

	if( !(data = alloc_mem(sizeof(LIST_UID_DATA))) ) return FALSE;

	data->ptr = obj;
	data->id[0] = obj->id[0];
	data->id[1] = obj->id[1];

	if( !list_appendlink(var->_.list, data) )
		free_mem(data,sizeof(LIST_UID_DATA));

	return TRUE;
}

bool variables_append_list_token (ppVARIABLE list, char *name, TOKEN_DATA *token)
{
	LIST_UID_DATA *data;
	pVARIABLE var = variable_get(*list, name);

	if( !token || !var || var->type != VAR_BLIST_TOK) return FALSE;

	if( !(data = alloc_mem(sizeof(LIST_UID_DATA))) ) return FALSE;

	data->ptr = token;
	data->id[0] = token->id[0];
	data->id[1] = token->id[1];

	if( !list_appendlink(var->_.list, data) )
		free_mem(data,sizeof(LIST_UID_DATA));

	return TRUE;
}

bool variables_append_list_area (ppVARIABLE list, char *name, AREA_DATA *area)
{
	LIST_AREA_DATA *data;
	pVARIABLE var = variable_get(*list, name);

	if( !area || !var || var->type != VAR_BLIST_AREA) return FALSE;

	if( !(data = alloc_mem(sizeof(LIST_AREA_DATA))) ) return FALSE;

	data->area = area;
	data->uid = area->uid;

	if( !list_appendlink(var->_.list, data) )
		free_mem(data,sizeof(LIST_AREA_DATA));

	return TRUE;
}

bool variables_append_list_wilds (ppVARIABLE list, char *name, WILDS_DATA *wilds)
{
	LIST_WILDS_DATA *data;
	pVARIABLE var = variable_get(*list, name);

	if( !wilds || !var || var->type != VAR_BLIST_WILDS) return FALSE;

	if( !(data = alloc_mem(sizeof(LIST_WILDS_DATA))) ) return FALSE;

	data->wilds = wilds;
	data->uid = wilds->uid;

	if( !list_appendlink(var->_.list, data) )
		free_mem(data,sizeof(LIST_WILDS_DATA));

	return TRUE;
}

bool variables_append_list_room (ppVARIABLE list, char *name, ROOM_INDEX_DATA *room)
{
	LIST_ROOM_DATA *data;
	pVARIABLE var = variable_get(*list, name);

	if( !room || !var || var->type != VAR_BLIST_ROOM) return FALSE;

	if( !(data = alloc_mem(sizeof(LIST_ROOM_DATA))) ) return FALSE;

	data->room = room;
	if( room->source ) {
		data->id[0] = 0;
		data->id[1] = room->source->vnum;
		data->id[2] = room->id[0];
		data->id[3] = room->id[1];
	} else if( room->wilds ) {
		data->id[0] = room->wilds->uid;
		data->id[1] = room->x;
		data->id[2] = room->y;
		data->id[3] = room->z;
	} else {
		data->id[0] = 0;
		data->id[1] = room->source->vnum;
		data->id[2] = 0;
		data->id[3] = 0;
	}

	if( !list_appendlink(var->_.list, data) )
		free_mem(data,sizeof(LIST_ROOM_DATA));

	return TRUE;
}

bool variables_append_list_connection (ppVARIABLE list, char *name, DESCRIPTOR_DATA *conn)
{
	pVARIABLE var = variable_get(*list, name);

	if( !var || var->type != VAR_PLIST_CONN) return FALSE;

	return list_appendlink(var->_.list, conn);
}

static bool variables_append_list_area_id(ppVARIABLE list, char *name, long aid)
{
	LIST_AREA_DATA *data;
	pVARIABLE var = variable_get(*list, name);

	if( !var || var->type != VAR_BLIST_AREA) return FALSE;

	if( !(data = alloc_mem(sizeof(LIST_AREA_DATA))) ) return FALSE;

	data->area = NULL;
	data->uid = aid;

	if( !list_appendlink(var->_.list, data) )
		free_mem(data,sizeof(LIST_AREA_DATA));

	return TRUE;
}

static bool variables_append_list_wilds_id(ppVARIABLE list, char *name, long wid)
{
	LIST_WILDS_DATA *data;
	pVARIABLE var = variable_get(*list, name);

	if( !var || var->type != VAR_BLIST_WILDS) return FALSE;

	if( !(data = alloc_mem(sizeof(LIST_WILDS_DATA))) ) return FALSE;

	data->wilds = NULL;
	data->uid = wid;

	if( !list_appendlink(var->_.list, data) )
		free_mem(data,sizeof(LIST_WILDS_DATA));

	return TRUE;
}


static bool variables_append_list_room_id (ppVARIABLE list, char *name, unsigned long a, unsigned long b, unsigned long c, unsigned long d)
{
	LIST_ROOM_DATA *data;
	pVARIABLE var = variable_get(*list, name);

	if( !var || var->type != VAR_BLIST_ROOM) return FALSE;

	if( !(data = alloc_mem(sizeof(LIST_ROOM_DATA))) ) return FALSE;

	data->room = NULL;
	data->id[0] = a;
	data->id[1] = b;
	data->id[2] = c;
	data->id[3] = d;

	if( !list_appendlink(var->_.list, data) )
		free_mem(data,sizeof(LIST_ROOM_DATA));

	return TRUE;
}

bool variables_append_list_door (ppVARIABLE list, char *name, ROOM_INDEX_DATA *room, int door)
{
	LIST_EXIT_DATA *data;
	pVARIABLE var = variable_get(*list, name);

	if( !room || door < 0 || door >= MAX_DIR || !var || var->type != VAR_BLIST_EXIT) return FALSE;

	if( !(data = alloc_mem(sizeof(LIST_EXIT_DATA))) ) return FALSE;

	data->room = room;
	if( room->source ) {
		data->id[0] = 0;
		data->id[1] = room->source->vnum;
		data->id[2] = room->id[0];
		data->id[3] = room->id[1];
	} else if( room->wilds ) {
		data->id[0] = room->wilds->uid;
		data->id[1] = room->x;
		data->id[2] = room->y;
		data->id[3] = room->z;
	} else {
		data->id[0] = 0;
		data->id[1] = room->source->vnum;
		data->id[2] = 0;
		data->id[3] = 0;
	}
	data->door = door;

	if( !list_appendlink(var->_.list, data) )
		free_mem(data,sizeof(LIST_EXIT_DATA));

	return TRUE;
}

static bool variables_append_list_door_id (ppVARIABLE list, char *name, unsigned long a, unsigned long b, unsigned long c, unsigned long d, int door)
{
	LIST_EXIT_DATA *data;
	pVARIABLE var = variable_get(*list, name);

	if( door < 0 || door >= MAX_DIR || !var || var->type != VAR_BLIST_EXIT) return FALSE;

	if( !(data = alloc_mem(sizeof(LIST_EXIT_DATA))) ) return FALSE;

	data->room = NULL;
	data->id[0] = a;
	data->id[1] = b;
	data->id[2] = c;
	data->id[3] = d;
	data->door = door;

	if( !list_appendlink(var->_.list, data) )
		free_mem(data,sizeof(LIST_EXIT_DATA));

	return TRUE;
}

bool variables_append_list_exit (ppVARIABLE list, char *name, EXIT_DATA *ex)
{
	if( !ex ) return FALSE;

	return variables_append_list_door(list, name, ex->from_room, ex->orig_door);
}

bool variables_append_list_skill_sn (ppVARIABLE list, char *name, CHAR_DATA *ch, int sn)
{
	LIST_SKILL_DATA *data;
	pVARIABLE var = variable_get(*list, name);

	if( !ch || sn < 1 || sn >= MAX_SKILL || !var || var->type != VAR_BLIST_SKILL) return FALSE;

	if( !(data = alloc_mem(sizeof(LIST_SKILL_DATA))) ) return FALSE;

	data->mob = ch;
	data->sn = sn;
	data->tok = NULL;
	data->mid[0] = ch->id[0];
	data->mid[1] = ch->id[1];
	data->tid[0] = 0;
	data->tid[1] = 0;

	if( !list_appendlink(var->_.list, data) )
		free_mem(data,sizeof(LIST_SKILL_DATA));

	return TRUE;
}

bool variables_append_list_skill_token (ppVARIABLE list, char *name, TOKEN_DATA *tok)
{
	LIST_SKILL_DATA *data;
	pVARIABLE var = variable_get(*list, name);

	if( !tok || !tok->player || (tok->type != TOKEN_SKILL && tok->type != TOKEN_SPELL) || !var || var->type != VAR_BLIST_SKILL) return FALSE;

	if( !(data = alloc_mem(sizeof(LIST_SKILL_DATA))) ) return FALSE;

	data->mob = tok->player;
	data->sn = 0;
	data->tok = tok;
	data->mid[0] = tok->player->id[0];
	data->mid[1] = tok->player->id[1];
	data->tid[0] = tok->id[0];
	data->tid[1] = tok->id[1];

	if( !list_appendlink(var->_.list, data) )
		free_mem(data,sizeof(LIST_SKILL_DATA));

	return TRUE;
}

static bool variables_append_list_skill_id (ppVARIABLE list, char *name, unsigned long ma, unsigned long mb, unsigned long ta, unsigned long tb, int sn)
{
	LIST_SKILL_DATA *data;
	pVARIABLE var = variable_get(*list, name);

	if( !var || var->type != VAR_BLIST_SKILL) return FALSE;

	if( !(data = alloc_mem(sizeof(LIST_SKILL_DATA))) ) return FALSE;

	data->mob = NULL;
	data->sn = (sn > 0 && sn < MAX_SKILL) ? sn : 0;;
	data->tok = NULL;
	data->mid[0] = ma;
	data->mid[1] = mb;
	data->tid[0] = ta;
	data->tid[1] = tb;

	if( !list_appendlink(var->_.list, data) )
		free_mem(data,sizeof(LIST_SKILL_DATA));

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

bool variables_setsave_string(ppVARIABLE list,char *name,char *str,bool shared, bool save)
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

	var->save = save;
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
	ITERATOR it;

	if(!(oldv = variable_get(*list,oldname))) return FALSE;

	if(!str_cmp(oldname,newname)) return TRUE;	// Copy to itself is.. dumb.

	if(!(newv = variable_create(list,newname,oldv->index,TRUE))) return FALSE;

	newv->type = oldv->type;
	newv->save = oldv->index ? oldv->save : FALSE;

	switch(newv->type) {
	case VAR_UNKNOWN:		break;
	case VAR_INTEGER:		newv->_.i = oldv->_.i; break;
	case VAR_STRING:		newv->_.s = str_dup(oldv->_.s); break;
	case VAR_STRING_S:		newv->_.s = oldv->_.s; break;
	case VAR_ROOM:			newv->_.r = oldv->_.r; break;
	case VAR_EXIT:			newv->_.door.r = oldv->_.door.r; newv->_.door.door = oldv->_.door.door; break;
	case VAR_MOBILE:		newv->_.m = oldv->_.m; break;
	case VAR_OBJECT:		newv->_.o = oldv->_.o; break;
	case VAR_TOKEN:			newv->_.t = oldv->_.t; break;
	case VAR_AREA:			newv->_.a = oldv->_.a; break;
	case VAR_SKILL:			newv->_.sn = oldv->_.sn; break;
	case VAR_SKILLINFO:		newv->_.sk.owner = oldv->_.sk.owner; newv->_.sk.sn = oldv->_.sk.sn; break;
	case VAR_AFFECT:		newv->_.aff = oldv->_.aff; break;

	case VAR_CONNECTION:	newv->_.conn = oldv->_.conn; break;
	case VAR_WILDS:			newv->_.wilds = oldv->_.wilds; break;
	case VAR_CHURCH:		newv->_.church = oldv->_.church; break;

	case VAR_PLIST_STR:
	case VAR_PLIST_CONN:
	case VAR_PLIST_ROOM:
	case VAR_PLIST_MOB:
	case VAR_PLIST_OBJ:
	case VAR_PLIST_TOK:
	case VAR_PLIST_CHURCH:
	case VAR_BLIST_ROOM:
	case VAR_BLIST_MOB:
	case VAR_BLIST_OBJ:
	case VAR_BLIST_TOK:
	case VAR_BLIST_EXIT:
	case VAR_BLIST_SKILL:
	case VAR_BLIST_AREA:
	case VAR_BLIST_WILDS:
		// All of the lists that require special allocation will be handled auto-magically by list_copy
		newv->_.list = list_copy(oldv->_.list);
		break;

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
	case VAR_EXIT:		newv->_.door.r = oldv->_.door.r; newv->_.door.door = oldv->_.door.door; break;
	case VAR_MOBILE:	newv->_.m = oldv->_.m; break;
	case VAR_OBJECT:	newv->_.o = oldv->_.o; break;
	case VAR_TOKEN:		newv->_.t = oldv->_.t; break;
	case VAR_AREA:		newv->_.a = oldv->_.a; break;
	case VAR_SKILL:		newv->_.sn = oldv->_.sn; break;
	case VAR_SKILLINFO:	newv->_.sk.owner = oldv->_.sk.owner; newv->_.sk.sn = oldv->_.sk.sn; break;
	case VAR_AFFECT:	newv->_.aff = oldv->_.aff; break;

	case VAR_CONNECTION:	newv->_.conn = oldv->_.conn; break;
	case VAR_WILDS:			newv->_.wilds = oldv->_.wilds; break;
	case VAR_CHURCH:		newv->_.church = oldv->_.church; break;

	case VAR_PLIST_STR:
	case VAR_PLIST_CONN:
	case VAR_PLIST_ROOM:
	case VAR_PLIST_MOB:
	case VAR_PLIST_OBJ:
	case VAR_PLIST_TOK:
	case VAR_PLIST_CHURCH:
	case VAR_BLIST_ROOM:
	case VAR_BLIST_MOB:
	case VAR_BLIST_OBJ:
	case VAR_BLIST_TOK:
	case VAR_BLIST_EXIT:
	case VAR_BLIST_SKILL:
	case VAR_BLIST_AREA:
	case VAR_BLIST_WILDS:
		// All of the lists that require special allocation will be handled auto-magically by list_copy
		newv->_.list = list_copy(oldv->_.list);
		break;

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
		case VAR_EXIT:		newv->_.door.r = oldv->_.door.r; newv->_.door.door = oldv->_.door.door; break;
		case VAR_MOBILE:	newv->_.m = oldv->_.m; break;
		case VAR_OBJECT:	newv->_.o = oldv->_.o; break;
		case VAR_TOKEN:		newv->_.t = oldv->_.t; break;
		case VAR_SKILL:		newv->_.sn = oldv->_.sn; break;
		case VAR_SKILLINFO:	newv->_.sk.owner = oldv->_.sk.owner; newv->_.sk.sn = oldv->_.sk.sn; break;
		case VAR_AFFECT:	newv->_.aff = oldv->_.aff; break;

		case VAR_CONNECTION:	newv->_.conn = oldv->_.conn; break;
		case VAR_WILDS:			newv->_.wilds = oldv->_.wilds; break;
		case VAR_CHURCH:		newv->_.church = oldv->_.church; break;

		case VAR_PLIST_STR:
		case VAR_PLIST_CONN:
		case VAR_PLIST_ROOM:
		case VAR_PLIST_MOB:
		case VAR_PLIST_OBJ:
		case VAR_PLIST_TOK:
		case VAR_PLIST_CHURCH:
		case VAR_BLIST_ROOM:
		case VAR_BLIST_MOB:
		case VAR_BLIST_OBJ:
		case VAR_BLIST_TOK:
		case VAR_BLIST_EXIT:
		case VAR_BLIST_SKILL:
		case VAR_BLIST_AREA:
		case VAR_BLIST_WILDS:
			// All of the lists that require special allocation will be handled auto-magically by list_copy
			newv->_.list = list_copy(oldv->_.list);
			break;

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
	unsigned long uid[2];

	if(!ptr) return;

	while(cur) {
		// Special case for MOBILES, clear out any skill reference.
		if(cur->type == VAR_SKILLINFO && type == VAR_MOBILE && cur->_.sk.owner == ptr) {
			CHAR_DATA *owner = cur->_.sk.owner;
			TOKEN_DATA *token = cur->_.sk.token;

			cur->_.skid.sn = cur->_.sk.sn;
			if(owner) {
				cur->_.skid.mid[0] = owner->id[0];
				cur->_.skid.mid[1] = owner->id[1];
			}
			else
			{
				cur->_.skid.mid[0] = 0;
				cur->_.skid.mid[1] = 0;
			}
			if(token) {
				cur->_.skid.tid[0] = token->id[0];
				cur->_.skid.tid[1] = token->id[1];
			}
			else
			{
				cur->_.skid.tid[0] = 0;
				cur->_.skid.tid[1] = 0;
			}
			cur->type = VAR_SKILLINFO_ID;

		} else if(cur->type == VAR_SKILLINFO && type == VAR_TOKEN && cur->_.sk.token == ptr) {
			CHAR_DATA *owner = cur->_.sk.owner;
			TOKEN_DATA *token = cur->_.sk.token;

			cur->_.skid.sn = cur->_.sk.sn;
			if(owner) {
				cur->_.skid.mid[0] = owner->id[0];
				cur->_.skid.mid[1] = owner->id[1];
			}
			else
			{
				cur->_.skid.mid[0] = 0;
				cur->_.skid.mid[1] = 0;
			}
			if(token) {
				cur->_.skid.tid[0] = token->id[0];
				cur->_.skid.tid[1] = token->id[1];
			}
			else
			{
				cur->_.skid.tid[0] = 0;
				cur->_.skid.tid[1] = 0;
			}
			cur->type = VAR_SKILLINFO_ID;
		} else if(cur->type == VAR_EXIT && type == VAR_ROOM && cur->_.door.r == ptr) {
			ROOM_INDEX_DATA *room = cur->_.door.r;
			if(room->wilds) {
				cur->_.wdoor.door = cur->_.door.door;
				cur->_.wdoor.wuid = room->wilds->uid;
				cur->_.wdoor.x = room->x;
				cur->_.wdoor.y = room->y;
				cur->type = VAR_WILDS_DOOR;
			} else if(room->source) {
				cur->_.cdoor.door = cur->_.door.door;
				cur->_.cdoor.r = room->source;
				cur->_.cdoor.a = room->id[0];
				cur->_.cdoor.b = room->id[1];
				cur->type = VAR_CLONE_DOOR;
			}

		} else if(cur->type == VAR_ROOM && type == VAR_ROOM && cur->_.r == ptr) {
			ROOM_INDEX_DATA *room = cur->_.r;
			if(room->wilds) {
				cur->_.wroom.wuid = room->wilds->uid;
				cur->_.wroom.x = room->x;
				cur->_.wroom.y = room->y;
				cur->type = VAR_WILDS_ROOM;
			} else if(room->source) {
				cur->_.cr.r = room->source;
				cur->_.cr.a = room->id[0];
				cur->_.cr.b = room->id[1];
				cur->type = VAR_CLONE_ROOM;
			}

		} else if(cur->type == VAR_OBJECT && type == VAR_OBJECT && cur->_.o == ptr) {
			OBJ_DATA *obj = cur->_.o;

			cur->_.oid.a = obj->id[0];
			cur->_.oid.b = obj->id[1];
			cur->type = VAR_OBJECT_ID;

		} else if(cur->type == VAR_MOBILE && type == VAR_MOBILE && cur->_.m == ptr) {
			CHAR_DATA *mob = cur->_.m;

			cur->_.mid.a = mob->id[0];
			cur->_.mid.b = mob->id[1];
			cur->type = VAR_MOBILE_ID;

		} else if(cur->type == VAR_TOKEN && type == VAR_TOKEN && cur->_.t == ptr) {
			TOKEN_DATA *token = cur->_.t;

			cur->_.tid.a = token->id[0];
			cur->_.tid.b = token->id[1];
			cur->type = VAR_TOKEN_ID;

		} else if(cur->type == VAR_BLIST_ROOM && type == VAR_ROOM && list_isvalid(cur->_.list)) {
			ITERATOR it;
			LIST_ROOM_DATA *lroom;

			iterator_start(&it, cur->_.list);

			while( (lroom = (LIST_ROOM_DATA *)iterator_nextdata(&it)) ) {
				if( lroom->room && lroom->room == ptr ) {
					iterator_remcurrent(&it);
					break;
				}
			}

			iterator_stop(&it);

		} else if(((cur->type == VAR_BLIST_MOB && type == VAR_MOBILE) || (cur->type == VAR_BLIST_OBJ && type == VAR_OBJECT) || (cur->type == VAR_BLIST_TOK && type == VAR_TOKEN)) && list_isvalid(cur->_.list)) {
			ITERATOR it;
			LIST_UID_DATA *luid;

			iterator_start(&it, cur->_.list);

			while( (luid = (LIST_UID_DATA *)iterator_nextdata(&it)) ) {
				if( luid->ptr && luid->ptr == ptr ) {
					iterator_remcurrent(&it);
					break;
				}
			}
			iterator_stop(&it);

		} else if(cur->type == VAR_PLIST_STR && type == VAR_STRING && ptr && list_isvalid(cur->_.list)) {
			list_remlink(cur->_.list, ptr);

		} else if(cur->type == VAR_PLIST_CONN && type == VAR_CONNECTION && ptr && list_isvalid(cur->_.list)) {
			list_remlink(cur->_.list, ptr);

		} else if(cur->type == VAR_PLIST_ROOM && type == VAR_ROOM && ptr && list_isvalid(cur->_.list)) {
			list_remlink(cur->_.list, ptr);

		} else if(cur->type == VAR_PLIST_MOB && type == VAR_MOBILE && ptr && list_isvalid(cur->_.list)) {
			list_remlink(cur->_.list, ptr);

		} else if(cur->type == VAR_PLIST_OBJ && type == VAR_OBJECT && ptr && list_isvalid(cur->_.list)) {
			list_remlink(cur->_.list, ptr);

		} else if(cur->type == VAR_PLIST_TOK && type == VAR_TOKEN && ptr && list_isvalid(cur->_.list)) {
			list_remlink(cur->_.list, ptr);

		} else if(cur->type == VAR_PLIST_CHURCH && type == VAR_CHURCH && ptr && list_isvalid(cur->_.list)) {
			list_remlink(cur->_.list, ptr);

		} else if(cur->type == type && cur->_.raw == ptr)
			cur->_.raw = NULL;

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

// Only fix variables that can be resolved, leave everything else alone
void variable_fix(pVARIABLE var)
{
	register ROOM_INDEX_DATA *room;
	ITERATOR it;
	LIST_UID_DATA *luid;
	LIST_ROOM_DATA *lroom;
	LIST_EXIT_DATA *lexit;
	LIST_SKILL_DATA *lskill;
	LIST_AREA_DATA *larea;
	LIST_WILDS_DATA *lwilds;

	if(var->type == VAR_CLONE_ROOM) {	// Dynamic
		if(var->_.cr.r && (room = get_clone_room(var->_.cr.r, var->_.cr.a, var->_.cr.b)) ) {
			var->_.r = room;
			var->type = VAR_ROOM;
		}
	} else if(var->type == VAR_CLONE_DOOR) {	// Dynamic
		if( var->_.cdoor.r && (room = get_clone_room(var->_.cdoor.r,var->_.cdoor.a,var->_.cdoor.b)) ) {
			var->_.door.door = var->_.cdoor.door;
			var->_.door.r = room;
			var->type = VAR_EXIT;
		}
	} else if(var->type == VAR_WILDS_ROOM) {
		WILDS_DATA *wilds = get_wilds_from_uid(NULL, var->_.wroom.wuid);
		ROOM_INDEX_DATA *vroom;
		if( wilds ) {
			vroom = get_wilds_vroom(wilds, var->_.wroom.x, var->_.wroom.y);

			if( !vroom )
				vroom = create_wilds_vroom(wilds, var->_.wroom.x, var->_.wroom.y);

			var->_.r = vroom;
			var->type = VAR_ROOM;
		}
	} else if(var->type == VAR_WILDS_DOOR) {
		WILDS_DATA *wilds = get_wilds_from_uid(NULL, var->_.wdoor.wuid);
		ROOM_INDEX_DATA *vroom;
		if( wilds ) {
			vroom = get_wilds_vroom(wilds, var->_.wdoor.x, var->_.wdoor.y);

			if( !vroom )
				vroom = create_wilds_vroom(wilds, var->_.wdoor.x, var->_.wdoor.y);

			var->_.door.door = var->_.wdoor.door;
			var->_.door.r = vroom;
			var->type = VAR_EXIT;
		}
	} else if(var->type == VAR_MOBILE_ID) {	// Dynamic
		CHAR_DATA *ch = idfind_mobile(var->_.mid.a, var->_.mid.b);
		if( ch ) {
			var->_.m = ch;
			var->type = VAR_MOBILE;
		}
	} else if(var->type == VAR_OBJECT_ID) {	// Dynamic
		OBJ_DATA *o = idfind_object(var->_.oid.a, var->_.oid.b);
		if( o ) {
			var->_.o = o;
			var->type = VAR_OBJECT;
		}
	} else if(var->type == VAR_TOKEN_ID) {
		TOKEN_DATA *t = idfind_token(var->_.tid.a, var->_.tid.b);
		if( t ) {
			var->_.t = t;
			var->type = VAR_TOKEN;
		}
	} else if(var->type == VAR_AREA_ID) {
		AREA_DATA *area = get_area_from_uid(var->_.aid);
		if( area ) {
			var->_.a = area;
			var->type = VAR_AREA;
		}
	} else if(var->type == VAR_WILDS_ID) {
		WILDS_DATA *wilds = get_wilds_from_uid(NULL, var->_.wid);
		if( wilds ) {
			var->_.wilds = wilds;
			var->type = VAR_WILDS;
		}
	} else if(var->type == VAR_SKILLINFO_ID && (var->_.skid.mid[0] > 0 || var->_.skid.mid[1] > 0)) {
		CHAR_DATA *ch = idfind_mobile(var->_.skid.mid[0], var->_.skid.mid[1]);
		if( ch ) {
			if(var->_.skid.tid[0] > 0 || var->_.skid.tid[1] > 0) {
				TOKEN_DATA *tok = idfind_token_char(ch, var->_.skid.tid[0], var->_.skid.tid[1]);

				if( tok ) {
					var->_.sk.token = tok;
					var->_.sk.sn = 0;
					var->_.sk.owner = ch;
					var->type = VAR_SKILLINFO;
				} else
					var->_.skid.mid[0] = var->_.skid.mid[1] = 0;
			} else {
				int sn = var->_.skid.sn;

				var->_.sk.token = NULL;
				var->_.sk.sn = sn;
				var->_.sk.owner = ch;
				var->type = VAR_SKILLINFO;
			}
		}
	} else if(var->type == VAR_BLIST_MOB && var->_.list) {
		iterator_start(&it, var->_.list);

		while( (luid = (LIST_UID_DATA *)iterator_nextdata(&it)) )
			if( !luid->ptr )
				luid->ptr = idfind_mobile(luid->id[0], luid->id[1]);

		iterator_stop(&it);

	} else if(var->type == VAR_BLIST_OBJ && var->_.list) {
		iterator_start(&it, var->_.list);

		while( (luid = (LIST_UID_DATA *)iterator_nextdata(&it)) )
			if( !luid->ptr )
				luid->ptr = idfind_object(luid->id[0], luid->id[1]);

		iterator_stop(&it);
	} else if(var->type == VAR_BLIST_TOK && var->_.list) {
		iterator_start(&it, var->_.list);

		while( (luid = (LIST_UID_DATA *)iterator_nextdata(&it)) )
			if( !luid->ptr )
				luid->ptr = idfind_token(luid->id[0], luid->id[1]);

		iterator_stop(&it);
	} else if(var->type == VAR_BLIST_AREA && var->_.list) {
		iterator_start(&it, var->_.list);

		while( (larea = (LIST_AREA_DATA *)iterator_nextdata(&it)) )
			if( !larea->area )
				larea->area = get_area_from_uid(larea->uid);

		iterator_stop(&it);
	} else if(var->type == VAR_BLIST_WILDS && var->_.list) {
		iterator_start(&it, var->_.list);

		while( (lwilds = (LIST_WILDS_DATA *)iterator_nextdata(&it)) )
			if( !lwilds->wilds )
				lwilds->wilds = get_wilds_from_uid(NULL, lwilds->uid);

		iterator_stop(&it);
	} else if(var->type == VAR_BLIST_ROOM && var->_.list ) {
		iterator_start(&it, var->_.list);

		while( (lroom = (LIST_ROOM_DATA *)iterator_nextdata(&it)) )
			if( !lroom->room ) {
				if( lroom->id[0] > 0 ) {	// WILDS room
					WILDS_DATA *wilds = get_wilds_from_uid(NULL, lroom->id[0]);
					if( wilds ) {
						lroom->room = get_wilds_vroom( wilds, lroom->id[1], lroom->id[2]);

						if( !lroom->room )
							lroom->room = create_wilds_vroom(wilds, lroom->id[1], lroom->id[2]);
					}

					if( !lroom->room) iterator_remcurrent(&it);

				} else if( lroom->id[2] > 0 || lroom->id[3] > 0) {	// Clone room
					lroom->room = get_room_index(lroom->id[1]);

					if( lroom->room )
						lroom->room = get_clone_room((ROOM_INDEX_DATA *)(lroom->room), lroom->id[2], lroom->id[3]);
				} else if( !(lroom->room = get_room_index(lroom->id[1])) )
					iterator_remcurrent(&it);
			}

		iterator_stop(&it);
	} else if(var->type == VAR_BLIST_EXIT && var->_.list ) {
		iterator_start(&it, var->_.list);

		while( (lexit = (LIST_EXIT_DATA *)iterator_nextdata(&it)) )
			if( !lexit->room ) {
				if( lexit->id[0] > 0 ) {	// WILDS room
					WILDS_DATA *wilds = get_wilds_from_uid(NULL, lexit->id[0]);
					if( wilds ) {
						lexit->room = get_wilds_vroom( wilds, lexit->id[1], lexit->id[2]);

						if( !lexit->room )
							lexit->room = create_wilds_vroom(wilds, lexit->id[1], lexit->id[2]);

					}

					if( !lexit->room) iterator_remcurrent(&it);
				} else if( lexit->id[2] > 0 || lexit->id[3] > 0) {	// Clone room, can wait
					lexit->room = get_room_index(lexit->id[1]);

					if( lexit->room )
						lexit->room = get_clone_room((ROOM_INDEX_DATA *)(lexit->room), lexit->id[2], lexit->id[3]);
				} else if( !(lexit->room = get_room_index(lexit->id[1])) )
					iterator_remcurrent(&it);

			}
		iterator_stop(&it);

	} else if(var->type == VAR_BLIST_SKILL && var->_.list ) {
		iterator_start(&it, var->_.list);

		while( (lskill = (LIST_SKILL_DATA *)iterator_nextdata(&it)) )
			if( !lskill->mob && ((lskill->sn > 0 && lskill->sn < MAX_SKILL) || lskill->tid[0] > 0 || lskill->tid[1] > 0) ) {
				lskill->mob = idfind_mobile(lskill->mid[0],lskill->mid[1]);

				if( lskill->mob && (lskill->tid[0] > 0 || lskill->tid[1] > 0)) {
					lskill->sn = 0;
					lskill->tok = idfind_token_char(lskill->mob, lskill->tid[0], lskill->tid[1]);

					// Can't resolve the token on a found mob, remove "skill" from list
					if( !lskill->tok ) {
						lskill->tid[0] = lskill->tid[1] = 0;
						iterator_remcurrent(&it);
					}
				}
			}

		iterator_stop(&it);

	}
}

void variable_fix_global(void)
{
	register pVARIABLE cur = variable_head;

	while(cur) {

		variable_fix(cur);

		cur = cur->global_next;
	}
}

void variable_fix_list(register pVARIABLE list)
{
	while(list) {
		variable_fix(list);
		list = list->global_next;
	}
}

// The variable_dynamic_fix_* functions deal with things loaded by players logging in.

// Clone rooms AND Clone doors
void variable_dynamic_fix_clone_room (ROOM_INDEX_DATA *clone)
{
	register pVARIABLE cur = variable_head;
	register CHAR_DATA *ch;
	register OBJ_DATA *obj;
	register TOKEN_DATA *token;
	ITERATOR it;

	if( !clone->source ) return;

	while(cur) {
		switch(cur->type) {
		case VAR_CLONE_ROOM:
			if( clone->source == cur->_.cr.r && clone->id[0] == cur->_.cr.a && clone->id[1] == cur->_.cr.b) {
				cur->_.r = clone;
				cur->type = VAR_ROOM;
			}
			break;
		case VAR_CLONE_DOOR:
			if( clone->source == cur->_.cdoor.r && clone->id[0] == cur->_.cdoor.a && clone->id[1] == cur->_.cdoor.b) {
				cur->_.door.door = cur->_.cdoor.door;
				cur->_.door.r = clone;
				cur->type = VAR_EXIT;
			}
			break;
		case VAR_BLIST_ROOM:
			if( cur->_.list ) {
				LIST_ROOM_DATA *lroom;

				iterator_start(&it, cur->_.list);

				while( (lroom = (LIST_ROOM_DATA *)iterator_nextdata(&it)) ) {
					if( !lroom->room ) {
						if( !lroom->id[0] > 0 &&
							lroom->id[1] == clone->source->vnum &&
							clone->id[0] == lroom->id[2] &&
							clone->id[1] == lroom->id[3])
							lroom->room = clone;
					}
				}

				iterator_stop(&it);
			}
			break;

		case VAR_BLIST_EXIT:
			if( cur->_.list ) {
				LIST_EXIT_DATA *lexit;

				iterator_start(&it, cur->_.list);

				while( (lexit = (LIST_EXIT_DATA *)iterator_nextdata(&it)) ) {
					if( !lexit->room ) {
						if( !lexit->id[0] > 0 &&
							lexit->id[1] == clone->source->vnum &&
							clone->id[0] == lexit->id[2] &&
							clone->id[1] == lexit->id[3])
							lexit->room = clone;
					}
				}

				iterator_stop(&it);
			}
			break;
		}

		cur = cur->global_next;
	}

	for( obj = clone->contents; obj; obj = obj->next_content)
		variable_dynamic_fix_object(obj);

	for(token = clone->tokens; token; token = token->next)
		variable_dynamic_fix_token(token);

	for( ch = clone->people; ch; ch = ch->next_in_room)
		variable_dynamic_fix_mobile(ch);
}

void variable_dynamic_fix_object(OBJ_DATA *obj)
{
	register pVARIABLE cur = variable_head;
	register OBJ_DATA *o;
	register ROOM_INDEX_DATA *clone;
	register TOKEN_DATA *token;
	register LIST_UID_DATA *luid;

	while(cur) {
		switch(cur->type) {
		case VAR_OBJECT_ID:
			if( obj->id[0] == cur->_.oid.a && obj->id[1] == cur->_.oid.b) {
				cur->_.o = obj;
				cur->type = VAR_OBJECT;
			}
			break;

		case VAR_BLIST_OBJ:
			if( cur->_.list && cur->_.list->valid ) {

				ITERATOR it;

				iterator_start(&it, cur->_.list);

				while( (luid = (LIST_UID_DATA *)iterator_nextdata(&it)) )
					if( !luid->ptr && obj->id[0] == luid->id[0] && obj->id[1] == luid->id[1])
						luid->ptr = obj;

				iterator_stop(&it);
			}
		}

		cur = cur->global_next;
	}

	for(o = obj->contains; o; o = o->next_content)
		variable_dynamic_fix_object(o);

	for(token = obj->tokens; token; token = token->next)
		variable_dynamic_fix_token(token);

	for(clone = obj->clone_rooms; clone; clone = clone->next_clone)
		variable_dynamic_fix_clone_room(clone);
}

void variable_dynamic_fix_token (TOKEN_DATA *token)
{
	register pVARIABLE cur = variable_head;

	register LIST_UID_DATA *luid;

	while(cur) {
		switch(cur->type) {
		case VAR_TOKEN_ID:
			if( token->id[0] == cur->_.tid.a && token->id[1] == cur->_.tid.b) {
				cur->_.t = token;
				cur->type = VAR_TOKEN;
			}
			break;

		case VAR_BLIST_TOK:
			if( cur->_.list && cur->_.list->valid ) {

				ITERATOR it;

				iterator_start(&it, cur->_.list);

				while( (luid = (LIST_UID_DATA *)iterator_nextdata(&it)) )
					if( !luid->ptr && token->id[0] == luid->id[0] && token->id[1] == luid->id[1])
						luid->ptr = token;

				iterator_stop(&it);
			}
		}

		cur = cur->global_next;
	}



}

void variable_dynamic_fix_mobile (CHAR_DATA *ch)
{
	register pVARIABLE cur = variable_head;
	register OBJ_DATA *o;
	register TOKEN_DATA *token;
	register ROOM_INDEX_DATA *clone;
	register LIST_UID_DATA *luid;

	while(cur) {
		log_stringf("variable_dynamic_fix_mobile: %s, %s, %d", ch->name, cur->name, cur->type);
		switch(cur->type) {
		case VAR_MOBILE_ID:
			log_stringf("variable_dynamic_fix_mobile:VAR_MOBILE_ID[%s]: %08lX - %08lX :: %08lX - %08lX", cur->name, ch->id[0], cur->_.mid.a, ch->id[1], cur->_.mid.b);
			if( ch->id[0] == cur->_.mid.a && ch->id[1] == cur->_.mid.b) {
				cur->_.m = ch;
				cur->type = VAR_MOBILE;
			}
			break;

		case VAR_SKILLINFO_ID:
			if( ch->id[0] == cur->_.skid.mid[0] && ch->id[1] == cur->_.skid.mid[1] ) {
				if(cur->_.skid.tid[0] > 0 || cur->_.skid.tid[1] > 0) {
					TOKEN_DATA *tok = idfind_token_char(ch, cur->_.skid.tid[0], cur->_.skid.tid[1]);

					if( tok ) {
						cur->_.sk.token = tok;
						cur->_.sk.sn = 0;
						cur->_.sk.owner = ch;
						cur->type = VAR_SKILLINFO;
					} else
						cur->_.skid.mid[0] = cur->_.skid.mid[1] = 0;
				} else {
					int sn = cur->_.skid.sn;

					cur->_.sk.token = NULL;
					cur->_.sk.sn = sn;
					cur->_.sk.owner = ch;
					cur->type = VAR_SKILLINFO;
				}
			}
			break;

		case VAR_BLIST_MOB:
			if( cur->_.list && cur->_.list->valid ) {

				ITERATOR it;

				iterator_start(&it, cur->_.list);

				while( (luid = (LIST_UID_DATA *)iterator_nextdata(&it)) )
					if( !luid->ptr && ch->id[0] == luid->id[0] && ch->id[1] == luid->id[1])
						luid->ptr = ch;

				iterator_stop(&it);
			}
		}

		cur = cur->global_next;
	}

	for(o = ch->carrying; o; o = o->next_content)
		variable_dynamic_fix_object(o);

	for(o = ch->locker; o; o = o->next_content)
		variable_dynamic_fix_object(o);

	for(token = ch->tokens; token; token = token->next)
		variable_dynamic_fix_token(token);

	for(clone = ch->clone_rooms; clone; clone = clone->next_clone)
		variable_dynamic_fix_clone_room(clone);
}

void variable_dynamic_fix_church (CHURCH_DATA *church)
{
	register pVARIABLE cur = variable_head;

	while(cur) {
		switch(cur->type) {
		case VAR_CHURCH_ID:
			if( church->uid == cur->_.chid ) {
				cur->_.church = church;
				cur->type = VAR_CHURCH;
			}
			break;
		}
		cur = cur->global_next;
	}

}


void variable_fwrite_uid_list( char *field, char *name, LIST *list, FILE *fp)
{
	if(list && list->valid) {
		LIST_UID_DATA *data;
		ITERATOR it;

		fprintf(fp,"%s %s~\n", field, name);
		iterator_start(&it,list);

		while((data = (LIST_UID_DATA*)iterator_nextdata(&it)))
			fprintf(fp, "UID %ld %ld\n", data->id[0], data->id[1]);

		iterator_stop(&it);
		fprintf(fp,"End\n");
	}

}

void variable_fwrite(pVARIABLE var, FILE *fp)
{
	ITERATOR it;

	switch(var->type) {
	default:
		break;

	case VAR_INTEGER:
		fprintf(fp,"VarInt %s~ %d\n", var->name, var->_.i);
		break;
	case VAR_STRING:
	case VAR_STRING_S:	// They all get changed to shared on load...
		fprintf(fp,"VarStr %s~ %s~\n", var->name, var->_.s ? var->_.s : "");
		break;

	case VAR_ROOM:
		if(var->_.r) {
			if(var->_.r->wilds)
				fprintf(fp,"VarVRoom %s~ %d %d %d\n", var->name, (int)var->_.r->wilds->uid, (int)var->_.r->x, (int)var->_.r->y);
			else if(var->_.r->source)
				fprintf(fp,"VarCRoom %s~ %d %d %d\n", var->name, (int)var->_.r->source->vnum, (int)var->_.r->id[0], (int)var->_.r->id[1]);
			else
				fprintf(fp,"VarRoom %s~ %d\n", var->name, (int)var->_.r->vnum);
		}
		break;

	case VAR_WILDS_ROOM:
		fprintf(fp,"VarVRoom %s~ %d %d %d\n", var->name, (int)var->_.wroom.wuid, (int)var->_.wroom.x, (int)var->_.wroom.y);
		break;

	// Unresolved CLONE ROOMs
	case VAR_CLONE_ROOM:
		if( var->_.cr.r )
			fprintf(fp,"VarCRoom %s~ %d %d %d\n", var->name, (int)var->_.cr.r->vnum, (int)var->_.cr.a,(int)var->_.cr.b);
		break;

	case VAR_EXIT:
	case VAR_DOOR:
		if(var->_.door.r) {
			if(var->_.door.r->wilds)
				fprintf(fp,"VarVExit %s~ %d %d %d %d\n", var->name, (int)var->_.door.r->wilds->uid, (int)var->_.door.r->x, (int)var->_.door.r->y, var->_.door.door);
			else if(var->_.door.r->source)
				fprintf(fp,"VarCExit %s~ %d %d %d %d\n", var->name, (int)var->_.door.r->source->vnum, (int)var->_.door.r->id[0], (int)var->_.door.r->id[1], var->_.door.door);
			else
				fprintf(fp,"VarExit %s~ %d %d\n", var->name, (int)var->_.door.r->vnum, var->_.door.door);
		}
		break;

	case VAR_WILDS_DOOR:
		fprintf(fp,"VarVExit %s~ %d %d %d %d\n", var->name, (int)var->_.wdoor.wuid, (int)var->_.wdoor.x, (int)var->_.wdoor.y, var->_.wdoor.door);
		break;


	// Unresolved EXITS in CLONE ROOMs
	case VAR_CLONE_DOOR:
		if( var->_.cdoor.r )
			fprintf(fp,"VarCExit %s~ %d %d %d %d\n", var->name, (int)var->_.cdoor.r->vnum, (int)var->_.cdoor.a,(int)var->_.cdoor.b, var->_.cdoor.door);
		break;

	case VAR_MOBILE:
		if(var->_.m)
			fprintf(fp,"VarMob %s~ %d %d\n", var->name, (int)var->_.m->id[0], (int)var->_.m->id[1]);
		break;
	case VAR_OBJECT:
		if(var->_.o)
			fprintf(fp,"VarObj %s~ %d %d\n", var->name, (int)var->_.o->id[0], (int)var->_.o->id[1]);
		break;
	case VAR_TOKEN:
		if(var->_.t)
			fprintf(fp,"VarTok %s~ %d %d\n", var->name, (int)var->_.t->id[0], (int)var->_.t->id[1]);
		break;
	case VAR_AREA:
		if(var->_.a)
			fprintf(fp,"VarArea %s~ %ld\n", var->name, var->_.a->uid);
		break;
	case VAR_WILDS:
		if(var->_.wilds)
			fprintf(fp,"VarWilds %s~ %ld\n", var->name, var->_.wilds->uid);
		break;
	case VAR_CHURCH:
		if(var->_.church)
			fprintf(fp,"VarChurch %s~ %ld\n", var->name, var->_.church->uid);
		break;

	// Unresolved UIDs
	case VAR_MOBILE_ID:
		fprintf(fp,"VarMob %s~ %d %d\n", var->name, (int)var->_.mid.a, (int)var->_.mid.b);
		break;
	case VAR_OBJECT_ID:
		fprintf(fp,"VarObj %s~ %d %d\n", var->name, (int)var->_.oid.a, (int)var->_.oid.b);
		break;
	case VAR_TOKEN_ID:
		fprintf(fp,"VarTok %s~ %d %d\n", var->name, (int)var->_.tid.a, (int)var->_.tid.b);
		break;
	case VAR_AREA_ID:
		fprintf(fp,"VarArea %s~ %ld\n", var->name, var->_.aid);
		break;
	case VAR_WILDS_ID:
		fprintf(fp,"VarWilds %s~ %ld\n", var->name, var->_.wid);
		break;
	case VAR_CHURCH_ID:
		fprintf(fp,"VarChurch %s~ %ld\n", var->name, var->_.chid);
		break;

	case VAR_SKILL:
		fprintf(fp,"VarSkill %s~ '%s'\n", var->name, SKILL_NAME(var->_.sn));
		break;

	case VAR_SKILLINFO:
		if(var->_.sk.owner) {
			if( IS_VALID(var->_.sk.token) )
				fprintf(fp,"VarSkInfo %s~ %d %d %d %d ''\n", var->name, (int)var->_.sk.owner->id[0], (int)var->_.sk.owner->id[1], (int)var->_.sk.token->id[0], (int)var->_.sk.token->id[1]);
			else
				fprintf(fp,"VarSkInfo %s~ %d %d 0 0 '%s'\n", var->name, (int)var->_.sk.owner->id[0], (int)var->_.sk.owner->id[1], SKILL_NAME(var->_.sk.sn));
		}
		break;

	case VAR_SKILLINFO_ID:
		fprintf(fp,"VarSkInfo %s~ %d %d %d %d '%s'\n", var->name, (int)var->_.skid.mid[0], (int)var->_.skid.mid[1], (int)var->_.skid.tid[0], (int)var->_.skid.tid[1], SKILL_NAME(var->_.skid.sn));
		break;

	case VAR_PLIST_STR:
		if(var->_.list && var->_.list->valid) {
			char *str;

			fprintf(fp,"VarListStr %s~\n", var->name);
			iterator_start(&it,var->_.list);

			while((str = (char*)iterator_nextdata(&it)))
				fprintf(fp, "String %s~\n", str);

			iterator_stop(&it);
			fprintf(fp,"End\n");
		}
		break;

	case VAR_BLIST_MOB:
		variable_fwrite_uid_list( "VarListMob", var->name, var->_.list, fp);
		break;

	case VAR_BLIST_OBJ:
		variable_fwrite_uid_list( "VarListObj", var->name, var->_.list, fp);
		break;

	case VAR_BLIST_TOK:
		variable_fwrite_uid_list( "VarListTok", var->name, var->_.list, fp);
		break;

	case VAR_BLIST_AREA:
		if(var->_.list && var->_.list->valid) {
			LIST_AREA_DATA *area;

			fprintf(fp,"VarListArea %s~\n", var->name);
			iterator_start(&it,var->_.list);

			while((area = (LIST_AREA_DATA*)iterator_nextdata(&it))) if( area->area ) {
				fprintf(fp, "Area %ld\n", area->uid);
			}

			iterator_stop(&it);
			fprintf(fp,"End\n");
		}
		break;
	case VAR_BLIST_WILDS:
		if(var->_.list && var->_.list->valid) {
			LIST_WILDS_DATA *wilds;

			fprintf(fp,"VarListWilds %s~\n", var->name);
			iterator_start(&it,var->_.list);

			while((wilds = (LIST_WILDS_DATA*)iterator_nextdata(&it))) if( wilds->wilds ) {
				fprintf(fp, "Wilds %ld\n", wilds->uid);
			}

			iterator_stop(&it);
			fprintf(fp,"End\n");
		}
		break;
	case VAR_BLIST_ROOM:
		if(var->_.list && var->_.list->valid) {
			LIST_ROOM_DATA *room;

			fprintf(fp,"VarListRoom %s~\n", var->name);
			iterator_start(&it,var->_.list);

			while((room = (LIST_ROOM_DATA*)iterator_nextdata(&it))) if( room->room ) {
				if(room->room->wilds)
					fprintf(fp, "VRoom %ld %ld %ld %ld\n", room->room->wilds->uid, room->room->x, room->room->y, room->room->z);
				else if(room->room->source)
					fprintf(fp, "CRoom %ld %ld %ld\n", room->room->source->vnum, room->room->id[0], room->room->id[1]);
				else
					fprintf(fp, "Room %ld\n", room->room->vnum);
			}

			iterator_stop(&it);
			fprintf(fp,"End\n");
		}
		break;
	case VAR_BLIST_EXIT:
		if(var->_.list && var->_.list->valid) {
			LIST_EXIT_DATA *room;

			fprintf(fp,"VarListExit %s~\n", var->name);
			iterator_start(&it,var->_.list);

			while((room = (LIST_EXIT_DATA*)iterator_nextdata(&it))) if( room->room ) {
				if(room->room->wilds)
					fprintf(fp, "VRoom %ld %ld %ld %ld %d\n", room->room->wilds->uid, room->room->x, room->room->y, room->room->z, room->door);
				else if(room->room->source)
					fprintf(fp, "CRoom %ld %ld %ld %d\n", room->room->source->vnum, room->room->id[0], room->room->id[1], room->door);
				else
					fprintf(fp, "Room %ld %d\n", room->room->vnum, room->door);
			}

			iterator_stop(&it);
			fprintf(fp,"End\n");
		}
		break;

	case VAR_BLIST_SKILL:
		if(var->_.list && var->_.list->valid) {
			LIST_SKILL_DATA *skill;

			fprintf(fp,"VarListSkill %s~\n", var->name);
			iterator_start(&it,var->_.list);

			while((skill = (LIST_SKILL_DATA*)iterator_nextdata(&it))) if( IS_VALID(skill->mob) ) {
				if( IS_VALID(skill->tok) )
					fprintf(fp, "Token %ld %ld %ld %ld\n", skill->mob->id[0], skill->mob->id[1], skill->tok->id[0], skill->tok->id[1]);
				else
					fprintf(fp, "Skill %ld %ld '%s'\n", skill->mob->id[0], skill->mob->id[1], SKILL_NAME(skill->sn));
			}

			iterator_stop(&it);
			fprintf(fp,"End\n");
		}
		break;
	}
}

bool variable_fread_str_list(ppVARIABLE vars, char *name, FILE *fp)
{
	char *word;
	bool fMatch;

	for(; ;) {
		word   = feof(fp) ? "End" : fread_word(fp);
		fMatch = FALSE;

		if (!str_cmp(word, "End"))
			return TRUE;

		else if (!str_cmp(word, "String")) {

			if( !variables_append_list_str( vars, name, fread_string(fp)) )
				return FALSE;

		} else
			fread_to_eol(fp);

	}

}

bool variable_fread_uid_list(ppVARIABLE vars, char *name, int type, FILE *fp)
{
	char *word;
	bool fMatch;

	for(; ;) {
		word   = feof(fp) ? "End" : fread_word(fp);
		fMatch = FALSE;

		if (!str_cmp(word, "End"))
			return TRUE;

		else if (!str_cmp(word, "UID")) {

			if( !variables_append_list_uid( vars, name, type, fread_number(fp), fread_number(fp)) )
				return FALSE;

		} else
			fread_to_eol(fp);

	}

}

bool variable_fread_room_list(ppVARIABLE vars, char *name, FILE *fp)
{
	char *word;
	bool fMatch;

	for(; ;) {
		word   = feof(fp) ? "End" : fread_word(fp);
		fMatch = FALSE;

		if (!str_cmp(word, "End"))
			return TRUE;

		else if (!str_cmp(word, "VRoom")) {

			if( !variables_append_list_room_id( vars, name, fread_number(fp), fread_number(fp), fread_number(fp), fread_number(fp)) )
				return FALSE;

		} else if (!str_cmp(word, "CRoom")) {

			if( !variables_append_list_room_id( vars, name, 0, fread_number(fp), fread_number(fp), fread_number(fp)) )
				return FALSE;

		} else if (!str_cmp(word, "Room")) {

			if( !variables_append_list_room_id( vars, name, 0, fread_number(fp), 0, 0) )
				return FALSE;

		} else
			fread_to_eol(fp);

	}

}

bool variable_fread_exit_list(ppVARIABLE vars, char *name, FILE *fp)
{
	char *word;
	bool fMatch;

	for(; ;) {
		word   = feof(fp) ? "End" : fread_word(fp);
		fMatch = FALSE;

		if (!str_cmp(word, "End"))
			return TRUE;

		else if (!str_cmp(word, "VRoom")) {

			if( !variables_append_list_door_id( vars, name, fread_number(fp), fread_number(fp), fread_number(fp), fread_number(fp), fread_number(fp)) )
				return FALSE;

		} else if (!str_cmp(word, "CRoom")) {

			if( !variables_append_list_door_id( vars, name, 0, fread_number(fp), fread_number(fp), fread_number(fp), fread_number(fp)) )
				return FALSE;

		} else if (!str_cmp(word, "Room")) {

			if( !variables_append_list_door_id( vars, name, 0, fread_number(fp), 0, 0, fread_number(fp)) )
				return FALSE;

		} else
			fread_to_eol(fp);

	}

}

bool variable_fread_area_list(ppVARIABLE vars, char *name, FILE *fp)
{
	char *word;
	bool fMatch;

	for(; ;) {
		word   = feof(fp) ? "End" : fread_word(fp);
		fMatch = FALSE;

		if (!str_cmp(word, "End"))
			return TRUE;

		else if (!str_cmp(word, "Area")) {

			if( !variables_append_list_area_id( vars, name, fread_number(fp)) )
				return FALSE;

		} else
			fread_to_eol(fp);

	}

}

bool variable_fread_wilds_list(ppVARIABLE vars, char *name, FILE *fp)
{
	char *word;
	bool fMatch;

	for(; ;) {
		word   = feof(fp) ? "End" : fread_word(fp);
		fMatch = FALSE;

		if (!str_cmp(word, "End"))
			return TRUE;

		else if (!str_cmp(word, "Wilds")) {

			if( !variables_append_list_wilds_id( vars, name, fread_number(fp)) )
				return FALSE;

		} else
			fread_to_eol(fp);

	}

}

bool variable_fread_skill_list(ppVARIABLE vars, char *name, FILE *fp)
{
	char *word;
	bool fMatch;

	for(; ;) {
		word   = feof(fp) ? "End" : fread_word(fp);
		fMatch = FALSE;

		if (!str_cmp(word, "End"))
			return TRUE;

		else if (!str_cmp(word, "Skill")) {

			if( !variables_append_list_skill_id (vars, name, fread_number(fp), fread_number(fp), 0, 0, skill_lookup(fread_word(fp))) )
				return FALSE;

		} else if (!str_cmp(word, "Token")) {

			if( !variables_append_list_skill_id (vars, name, fread_number(fp), fread_number(fp), fread_number(fp), fread_number(fp), 0) )
				return FALSE;

		} else
			fread_to_eol(fp);

	}

}

int variable_fread_type(char *str)
{
	if( !str_cmp( str, "VarInt" ) ) return VAR_INTEGER;
	if( !str_cmp( str, "VarStr" ) ) return VAR_STRING_S;
	if( !str_cmp( str, "VarRoom" ) ) return VAR_ROOM;
	if( !str_cmp( str, "VarCRoom" ) || !str_cmp( str, "VarRoomC" ) ) return VAR_CLONE_ROOM;
	if( !str_cmp( str, "VarVRoom" ) || !str_cmp( str, "VarRoomV" ) ) return VAR_WILDS_ROOM;
	if( !str_cmp( str, "VarExit" ) ) return VAR_DOOR;
	if( !str_cmp( str, "VarCExit" ) || !str_cmp( str, "VarExitC" ) ) return VAR_CLONE_DOOR;
	if( !str_cmp( str, "VarVExit" ) || !str_cmp( str, "VarExitV" ) ) return VAR_WILDS_DOOR;
	if( !str_cmp( str, "VarMob" ) ) return VAR_MOBILE_ID;
	if( !str_cmp( str, "VarObj" ) ) return VAR_OBJECT_ID;
	if( !str_cmp( str, "VarTok" ) ) return VAR_TOKEN_ID;
	if( !str_cmp( str, "VarArea" ) ) return VAR_AREA_ID;
	if( !str_cmp( str, "VarWilds" ) ) return VAR_WILDS_ID;
	if( !str_cmp( str, "VarChurch" ) ) return VAR_CHURCH_ID;
	if( !str_cmp( str, "VarSkill" ) ) return VAR_SKILL;
	if( !str_cmp( str, "VarSkInfo" ) ) return VAR_SKILLINFO_ID;
	if( !str_cmp( str, "VarListMob" ) ) return VAR_BLIST_MOB;
	if( !str_cmp( str, "VarListObj" ) ) return VAR_BLIST_OBJ;
	if( !str_cmp( str, "VarListTok" ) ) return VAR_BLIST_TOK;
	if( !str_cmp( str, "VarListRoom" ) ) return VAR_BLIST_ROOM;
	if( !str_cmp( str, "VarListExit" ) ) return VAR_BLIST_EXIT;
	if( !str_cmp( str, "VarListSkill" ) ) return VAR_BLIST_SKILL;
	if( !str_cmp( str, "VarListStr" ) ) return VAR_PLIST_STR;
	if( !str_cmp( str, "VarListArea" ) ) return VAR_BLIST_AREA;
	if( !str_cmp( str, "VarListWilds" ) ) return VAR_BLIST_WILDS;

	return VAR_UNKNOWN;
}

bool variable_fread(ppVARIABLE vars, int type, FILE *fp)
{
	char *name;
	unsigned long a, b, c, d;

	name = fread_string(fp);

	switch(type) {
	case VAR_INTEGER:
		return variables_setsave_integer(vars, name, fread_number(fp), TRUE);

	case VAR_STRING:
	case VAR_STRING_S:	// They all get changed to shared on load...
		return variables_setsave_string(vars, name, fread_string(fp), TRUE, TRUE);

	case VAR_ROOM:
		{
			ROOM_INDEX_DATA *room = get_room_index(fread_number(fp));
			return room && variables_setsave_room(vars, name, room, TRUE);
		}

	case VAR_CLONE_ROOM:
		{
			ROOM_INDEX_DATA *room = get_room_index(fread_number(fp));
			int x = fread_number(fp);
			int y = fread_number(fp);

			// Wait to resolve until AFTER all persistant rooms are loaded
			return room && variables_set_clone_room(vars, name, room, x, y, TRUE);
		}

	case VAR_WILDS_ROOM:
		{
			int wuid = fread_number(fp);
			WILDS_DATA *wilds;
			int x = fread_number(fp);
			int y = fread_number(fp);

			wilds = get_wilds_from_uid(NULL, wuid);
			if( wilds ) {
				ROOM_INDEX_DATA *room = get_wilds_vroom(wilds, x, y);

				// Go ahead and load it
				if( !room )
					room = create_wilds_vroom(wilds,x,y);

				return room && variables_setsave_room(vars, name, room, TRUE);
			} else
				return variables_set_wilds_room(vars,name,wuid, x, y, TRUE);
		}

	case VAR_DOOR:
		{
			ROOM_INDEX_DATA *room = get_room_index(fread_number(fp));

			return room && variables_set_door(vars, name, room, fread_number(fp), TRUE);
		}

	case VAR_CLONE_DOOR:
		{
			ROOM_INDEX_DATA *room = get_room_index(fread_number(fp));

			int x = fread_number(fp);
			int y = fread_number(fp);
			int door = fread_number(fp);

			return room && variables_set_clone_door(vars, name, room, x, y, door, TRUE);
		}

	case VAR_WILDS_DOOR:
		{
			int wuid = fread_number(fp);
			WILDS_DATA *wilds;
			int x = fread_number(fp);
			int y = fread_number(fp);

			wilds = get_wilds_from_uid(NULL, wuid);
			if( wilds ) {
				ROOM_INDEX_DATA *room = get_wilds_vroom(wilds, x, y);

				// Go ahead and load it
				if( !room )
					room = create_wilds_vroom(wilds,x,y);

				return room && variables_set_door(vars, name, room, fread_number(fp), TRUE);
			} else
				return variables_set_wilds_door(vars, name, wuid, x, y, fread_number(fp), TRUE);
		}

	case VAR_MOBILE_ID:
		a = fread_number(fp);
		b = fread_number(fp);
		return variables_set_mobile_id(vars, name, a, b, TRUE);

	case VAR_OBJECT_ID:
		a = fread_number(fp);
		b = fread_number(fp);
		return variables_set_object_id(vars, name, a, b, TRUE);

	case VAR_TOKEN_ID:
		a = fread_number(fp);
		b = fread_number(fp);
		return variables_set_token_id(vars, name, a, b, TRUE);

	case VAR_AREA_ID:
		return variables_set_area_id(vars, name, fread_number(fp), TRUE);

	case VAR_WILDS_ID:
		return variables_set_wilds_id(vars, name, fread_number(fp), TRUE);

	case VAR_CHURCH_ID:
		return variables_set_church_id(vars, name, fread_number(fp), TRUE);

	case VAR_SKILL:
		return variables_setsave_skill(vars, name, skill_lookup(fread_word(fp)), TRUE);

	case VAR_SKILLINFO_ID:
		a = fread_number(fp);
		b = fread_number(fp);
		c = fread_number(fp);
		d = fread_number(fp);

		return variables_set_skillinfo_id (vars, name, a, b, c, d, skill_lookup(fread_word(fp)) , TRUE);

	case VAR_PLIST_STR:
		if( variables_set_list(vars, name, VAR_PLIST_STR, TRUE) )
			return variable_fread_str_list(vars, name, fp);

	case VAR_BLIST_MOB:
	case VAR_BLIST_OBJ:
	case VAR_BLIST_TOK:
		if( variables_set_list(vars, name, type, TRUE) )
			return variable_fread_uid_list(vars, name, type, fp);
		else
			return FALSE;

	case VAR_BLIST_AREA:
		if( variables_set_list(vars, name, VAR_BLIST_AREA, TRUE) )
			return variable_fread_area_list(vars, name, fp);
		else
			return FALSE;

	case VAR_BLIST_WILDS:
		if( variables_set_list(vars, name, VAR_BLIST_WILDS, TRUE) )
			return variable_fread_wilds_list(vars, name, fp);
		else
			return FALSE;

	case VAR_BLIST_ROOM:
		if( variables_set_list(vars, name, VAR_BLIST_ROOM, TRUE) )
			return variable_fread_room_list(vars, name, fp);
		else
			return FALSE;

	case VAR_BLIST_EXIT:
		if( variables_set_list(vars, name, VAR_BLIST_EXIT, TRUE) )
			return variable_fread_exit_list(vars, name, fp);
		else
			return FALSE;

	case VAR_BLIST_SKILL:
		if( variables_set_list(vars, name, VAR_BLIST_SKILL, TRUE) )
			return variable_fread_skill_list(vars, name, fp);
		else
			return FALSE;

	}

	// Ignore rest
	fread_to_eol(fp);
	return TRUE;
}

void script_varclearon(VARIABLE **vars, char *argument)
{
	char name[MIL];

	if(!vars) return;

	// Get name
	argument = one_argument(argument,name);
	if(!name[0]) return;

	variable_remove(vars,name);
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

