#include "merc.h"
#include "tables.h"
#include "scripts.h"
#include "recycle.h"
#include "wilds.h"
//#define DEBUG_MODULE
#include "debug.h"

int script_security = INIT_SCRIPT_SECURITY;
int script_call_depth = 0;
int script_lastreturn = PRET_EXECUTED;
bool script_remotecall = FALSE;
bool script_destructed = FALSE;
bool wiznet_script = FALSE;
bool script_force_execute = FALSE;	// Executes the script even if disabled
SCRIPT_CB *script_call_stack = NULL;

ROOM_INDEX_DATA room_used_for_wilderness;
ROOM_INDEX_DATA room_pointer_vlink;
ROOM_INDEX_DATA room_pointer_environment;

bool opc_skip_block(SCRIPT_CB *block,int level,bool endblock);

void script_clear_mobile(CHAR_DATA *ptr)
{
	register int lp;
	register SCRIPT_CB *stack = script_call_stack;

	while(stack) {
		if(stack->info.mob == ptr) {
			stack->info.mob = NULL;
			stack->info.var = NULL;
			stack->info.targ = NULL;
			SET_BIT(stack->flags,SCRIPTEXEC_HALT);
		}
		if(stack->info.ch == ptr) stack->info.ch = NULL;
		if(stack->info.vch == ptr) stack->info.vch = NULL;
		if(stack->info.rch == ptr) stack->info.rch = NULL;
		for(lp = 0; lp < stack->loop; lp++) {
			if(stack->loops[lp].d.l.type == ENT_MOBILE) {
				if(stack->loops[lp].d.l.next.m == ptr) {
					if(stack->loops[lp].d.l.cur.m && ptr != stack->loops[lp].d.l.cur.m->next_in_room)
						stack->loops[lp].d.l.next.m = stack->loops[lp].d.l.cur.m->next_in_room;
				}
			}
		}

		stack = stack->next;
	}
}

void script_clear_object(OBJ_DATA *ptr)
{
	register int lp;
	register SCRIPT_CB *stack = script_call_stack;

	while(stack) {
		if(stack->info.obj == ptr) {
			stack->info.obj = NULL;
			stack->info.var = NULL;
			stack->info.targ = NULL;
			SET_BIT(stack->flags,SCRIPTEXEC_HALT);
		}
		if(stack->info.obj1 == ptr) stack->info.obj1 = NULL;
		if(stack->info.obj2 == ptr) stack->info.obj2 = NULL;
		for(lp = 0; lp < stack->loop; lp++) {
			if(stack->loops[lp].d.l.type == ENT_OBJECT) {
				if(stack->loops[lp].d.l.next.o == ptr) {
					if(stack->loops[lp].d.l.cur.o && ptr != stack->loops[lp].d.l.cur.o->next_content)
						stack->loops[lp].d.l.next.o = stack->loops[lp].d.l.cur.o->next_content;
				}
			}
		}
		stack = stack->next;
	}
}

void script_clear_room(ROOM_INDEX_DATA *ptr)
{
	register SCRIPT_CB *stack = script_call_stack;

	while(stack) {
		if(stack->info.room == ptr) {
			stack->info.room = NULL;
			stack->info.var = NULL;
			stack->info.targ = NULL;
			SET_BIT(stack->flags,SCRIPTEXEC_HALT);
		}
		stack = stack->next;
	}
}

void script_clear_token(TOKEN_DATA *ptr)
{
	register int lp;
	register SCRIPT_CB *stack = script_call_stack;

	while(stack) {
		if(stack->info.token == ptr) {
			stack->info.token = NULL;
			stack->info.var = NULL;
			stack->info.targ = NULL;
			SET_BIT(stack->flags,SCRIPTEXEC_HALT);
		}
		for(lp = 0; lp < stack->loop; lp++) {
			if(stack->loops[lp].d.l.type == ENT_TOKEN) {
				if(stack->loops[lp].d.l.next.t == ptr) {
					if(stack->loops[lp].d.l.cur.t && ptr != stack->loops[lp].d.l.cur.t->next)
						stack->loops[lp].d.l.next.t = stack->loops[lp].d.l.cur.t->next;
				}
			}
		}
		stack = stack->next;
	}
}

void script_clear_affect(AFFECT_DATA *ptr)
{
	register int lp;
	register SCRIPT_CB *stack = script_call_stack;

	while(stack) {
		for(lp = 0; lp < stack->loop; lp++) {
			if(stack->loops[lp].d.l.type == ENT_AFFECT) {
				if(stack->loops[lp].d.l.next.aff == ptr) {
					if(stack->loops[lp].d.l.cur.aff && ptr != stack->loops[lp].d.l.cur.aff->next)
						stack->loops[lp].d.l.next.aff = stack->loops[lp].d.l.cur.aff->next;
				}
			}
		}
		stack = stack->next;
	}
}

void script_clear_list(register void *owner)
{
	register int lp;
	register SCRIPT_CB *stack = script_call_stack;

	while(stack) {
		for(lp = 0; lp < stack->loop; lp++) {
			if(stack->loops[lp].d.l.owner == owner) {
				stack->loops[lp].d.l.owner = NULL;
				stack->loops[lp].d.l.cur.raw = NULL;
				stack->loops[lp].d.l.next.raw = NULL;
			}
		}
		stack = stack->next;
	}
}

void script_mobile_addref(CHAR_DATA *ch)
{
	if(IS_VALID(ch) && IS_NPC(ch) && ch->progs)
		ch->progs->script_ref++;
}

bool script_mobile_remref(CHAR_DATA *ch)
{
	if(IS_VALID(ch) && IS_NPC(ch) && ch->progs) {
		if( ch->progs->script_ref > 0 && !--ch->progs->script_ref ) {
			if( ch->progs->extract_when_done ) {
				// Remove!
				ch->progs->extract_when_done = FALSE;
				extract_char(ch, ch->progs->extract_fPull);
				return TRUE;
			}
		}
	}

	return FALSE;
}

void script_object_addref(OBJ_DATA *obj)
{
	if(IS_VALID(obj) && obj->progs)
		obj->progs->script_ref++;
}

bool script_object_remref(OBJ_DATA *obj)
{
	if(IS_VALID(obj) && obj->progs) {
		if( obj->progs->script_ref > 0 && !--obj->progs->script_ref ) {
			if( obj->progs->extract_when_done ) {
				// Remove!
				obj->progs->extract_when_done = FALSE;
				extract_obj(obj);
				return TRUE;
			}
		}
	}

	return FALSE;
}

void script_room_addref(ROOM_INDEX_DATA *room)
{
	if((room->source || IS_SET(room->room2_flags, ROOM_VIRTUAL_ROOM)) && room->progs)
		room->progs->script_ref++;
}

bool script_room_remref(ROOM_INDEX_DATA *room)
{
	if((room->source || IS_SET(room->room2_flags, ROOM_VIRTUAL_ROOM)) && room->progs) {
		if( room->progs->script_ref > 0 && !--room->progs->script_ref ) {
			if( room->progs->extract_when_done ) {
				// Remove!
				room->progs->extract_when_done = FALSE;
				if(room->source)
					extract_clone_room(room, room->id[0], room->id[1],false);
				else	// Is a WILDS room
					destroy_wilds_vroom(room);
				return TRUE;
			}
		}
	}

	return FALSE;
}

void script_token_addref(TOKEN_DATA *token)
{
	if(IS_VALID(token) && token->progs)
		token->progs->script_ref++;
}

bool script_token_remref(TOKEN_DATA *token)
{
	if(IS_VALID(token) && token->progs) {
		if( token->progs->script_ref > 0 && !--token->progs->script_ref ) {
			if( token->progs->extract_when_done ) {
				// Remove!
				token->progs->extract_when_done = FALSE;
				extract_token(token);
				return TRUE;
			}
		}
	}

	return FALSE;
}

ENT_FIELD *script_entity_fields(int type)
{
	int i;

	for(i=0; entity_type_info[i].type_min < ENT_MAX; i++)
		if( (type >= entity_type_info[i].type_min) && (type <= entity_type_info[i].type_max))
			return entity_type_info[i].fields;

	return NULL;
}

bool script_entity_allow_vars(int type)
{
	int i;

	for(i=0; entity_type_info[i].type_min < ENT_MAX; i++)
		if( (type >= entity_type_info[i].type_min) && (type <= entity_type_info[i].type_max))
			return entity_type_info[i].allow_vars;

	return FALSE;
}

//void compile_error_show(char *msg);
ENT_FIELD *entity_type_lookup(char *name, ENT_FIELD *list)
{
	int i;
//	char buf[MSL];

	if(!list) return NULL;

	for(i=0;list[i].name;i++) {
//		compile_error_show(buf);
//		sprintf(buf,"entity_type_lookup: '%s' '%s'",list[i].name,name);
		if(!str_cmp(name,list[i].name)) {
//			sprintf(buf,"entity_type_lookup: '%s' found",name);
//			compile_error_show(buf);
			return &list[i];
		}
	}

//	sprintf(buf,"entity_type_lookup: '%s' NOT found",name);
//	compile_error_show(buf);
	return NULL;
}

bool script_expression_push(STACK *stk,int val)
{
	if(stk->t >= MAX_STACK) return FALSE;
	stk->s[stk->t++] = val;
	return TRUE;
}

bool script_expression_push_operator(STACK *stk,int op)
{
	if(script_expression_tostack[op] == STK_MAX) return FALSE;
	return script_expression_push(stk,script_expression_tostack[op]);
}

int get_operator(char *keyword)
{
	register int i;
	for(i = 0; script_operators[i]; i++)
		if(!str_cmp(script_operators[i], keyword))
			return(i);
	return -1;
}

int ifcheck_lookup(char *name, int type)
{
	register int i;

	for(i=0;ifcheck_table[i].name;i++)
		if((ifcheck_table[i].type & type) && !str_cmp(name,ifcheck_table[i].name))
			return i;

	return -1;
}

char *ifcheck_get_value(SCRIPT_VARINFO *info,IFCHECK_DATA *ifc,char *text,int *ret,bool *valid)
{
	char buf[MIL];
	int i;
	SCRIPT_PARAM argv[IFC_MAXPARAMS];

	*valid = FALSE;

	// Validate parameters
	if(!info || !ifc || !ret) return NULL;

	if(!ifc->func) return NULL;

	// Clear variables
	memset(argv,0,sizeof(argv));

	text = skip_whitespace(text);

	// Stop when there the param list is full, there's no more text or it hits an
	//	operator
	for(i=0;text && *text && *text != ESCAPE_END && *text != '=' && *text != '<' &&
		*text != '>' && *text != '!' && *text != '&' && i<IFC_MAXPARAMS;i++) {
//		if(wiznet_script) {
//			sprintf(buf,"*text = %02.2X (%c)", *text, isprint(*text) ? *text : ' ');
//			wiznet(buf,NULL,NULL,WIZ_SCRIPTS,0,0);
//		}
		argv[i].buf[0] = 0;
		text = expand_argument(info,text,&argv[i]);
//		if(wiznet_script) {
//			sprintf(buf,"argv[%d].type = %d (%s)", i, argv[i].type, ifcheck_param_type_names[argv[i].type]);
//			wiznet(buf,NULL,NULL,WIZ_SCRIPTS,0,0);
//		}
	}
//	if(wiznet_script) {
//		sprintf(buf,"args = %d", i);
//		wiznet(buf,NULL,NULL,WIZ_SCRIPTS,0,0);
//	}
	if((ifc->func)(info,info->mob,info->obj,info->room,info->token,ret,i,argv))
		*valid = TRUE;

//	if(wiznet_script) {
//		sprintf(buf,"ret = %d", *ret);
//		wiznet(buf,NULL,NULL,WIZ_SCRIPTS,0,0);
//	}
	DBG2EXITVALUE1(PTR,text);
	return text;
}

int ifcheck_comparison(SCRIPT_CB *block)
{
	int lhs, oper, rhs;
	char *text, *p, buf[MIL], buf2[MSL];
	bool valid;
	SCRIPT_CODE *code;
	IFCHECK_DATA *ifc;
	SCRIPT_PARAM arg;

	if(!block) return -1;	// Error

	code = block->cur_line;
	if(!code || code->param < 0 || code->param >= CHK_MAXIFCHECKS)
		 return -1;

	ifc = &ifcheck_table[code->param];

	if(wiznet_script) {
		sprintf(buf2,"Doing ifcheck: %d, '%s'", code->param, ifc->name);
		wiznet(buf2,NULL,NULL,WIZ_SCRIPTS,0,0);
	}

	text = ifcheck_get_value(&block->info,ifc,block->cur_line->rest,&lhs,&valid);

	if(!text) return -1;

	if(!valid) return FALSE;

	if(!ifc->numeric) return (lhs > 0);

	text = one_argument(text, buf);

	oper = get_operator(buf);
	if (oper < 0) return FALSE;

	p = expand_argument(&block->info,text,&arg);
	if(!p || p == text) {
		return -1;
	}

	switch(arg.type) {
	case ENT_NUMBER: rhs = arg.d.num; break;
	case ENT_STRING:
		if(is_number(arg.d.str)) {
			rhs = atoi(arg.d.str);
			break;
		}
	default:
		return FALSE;
	}

	switch(oper) {
	case EVAL_EQ:	return (lhs == rhs);
	case EVAL_GE:	return (lhs >= rhs);
	case EVAL_LE:	return (lhs <= rhs);
	case EVAL_NE:	return (lhs != rhs);
	case EVAL_GT:	return (lhs > rhs);
	case EVAL_LT:	return (lhs < rhs);
	case EVAL_MASK:	return (lhs & rhs);
	default:	return FALSE;
	}
}

bool opc_skip_to_label(SCRIPT_CB *block,int op,int id,bool dir)
{
	int line, last;
	SCRIPT_CODE *code;
	char buf[MIL];

	code = block->script->code;
	last = block->script->lines;

	if(wiznet_script) {
		sprintf(buf,"Skipping to %s with ID %d.", opcode_names[op], id);
		wiznet(buf,NULL,NULL,WIZ_SCRIPTS,0,0);
	}

	if(dir) {	// Forward, after the loop
		for(line = block->line; line < last; line++) {
//			if(wiznet_script) {
//				sprintf(buf,"Checking: Line=%d, Opcode=%d(%s), Level=%d", line+1,code[line].opcode,opcode_names[code[line].opcode],code[line].level);
//				wiznet(buf,NULL,NULL,WIZ_SCRIPTS,0,0);
//			}
			if(code[line].opcode == op && code[line].label == id) {
				block->line = line+1;
				DBG2EXITVALUE2(TRUE);
				return TRUE;
			}
		}
	} else {	// Backward
		for(line = block->line; line >= 0; --line) {
//			if(wiznet_script) {
//				sprintf(buf,"Checking: Line=%d, Opcode=%d(%s), Level=%d", line+1,code[line].opcode,opcode_names[code[line].opcode],code[line].level);
//				wiznet(buf,NULL,NULL,WIZ_SCRIPTS,0,0);
//			}
			if(code[line].opcode == op && code[line].label == id) {
				block->line = line;
				DBG2EXITVALUE2(TRUE);
				return TRUE;
			}
		}
	}

	DBG2EXITVALUE2(FALSE);
	return FALSE;
}

bool opc_skip_block(SCRIPT_CB *block,int level,bool endblock)
{
	int line, last;
	SCRIPT_CODE *code;

//	if(wiznet_script) {
//		sprintf(buf,"Skipping to %s on level %d.", endblock?"ENDIF":"ELSE/ENDIF", level);
//		wiznet(buf,NULL,NULL,WIZ_SCRIPTS,0,0);
//	}

	// Looking for an ELSE/ELSEIF or ENDIF
	if(block->state[level] == OP_IF) {
		code = block->script->code;
		last = block->script->lines;

		line = block->line;
		if(code[line].opcode == OP_ELSEIF || code[line].opcode == OP_ELSEIFNOT) ++line;

		for(; line < last; line++) {
//			if(wiznet_script) {
//				sprintf(buf,"Checking: Line=%d, Opcode=%d(%s), Level=%d", line+1,code[line].opcode,opcode_names[code[line].opcode],code[line].level);
//				wiznet(buf,NULL,NULL,WIZ_SCRIPTS,0,0);
//			}
			if(((!endblock && (code[line].opcode == OP_ELSE || code[line].opcode == OP_ELSEIF || code[line].opcode == OP_ELSEIFNOT)) ||
				code[line].opcode == OP_ENDIF) && code[line].level == level) {
				block->line = line;
				DBG2EXITVALUE2(TRUE);
				return TRUE;
			}
		}
	} else if(block->state[level] == OP_WHILE)
		return opc_skip_to_label(block,OP_ENDWHILE,block->cur_line->label,TRUE);

	DBG2EXITVALUE2(FALSE);
	return FALSE;
}

void opc_next_line(SCRIPT_CB *block)
{
	block->line++;
}

// Function: End script execution
//
// Formats:
// break[ <expression>]
// end[ <expression>]
//
DECL_OPC_FUN(opc_end)
{
	int val;
	SCRIPT_PARAM arg;

	if(block->cur_line->level > 0 && !block->cond[block->cur_line->level-1])
		return opc_skip_block(block,block->cur_line->level-1,FALSE);

	// Anything to evaluate?
	if(block->cur_line->rest[0]) {
		if(!expand_argument(&block->info,block->cur_line->rest,&arg)) {
			block->ret_val = PRET_BADSYNTAX;
			return FALSE;
		}

		switch(arg.type) {
		case ENT_STRING: val = atoi(arg.d.str); break;
		case ENT_NUMBER: val = arg.d.num; break;
		default: val = 0; break;
		}

		DBG3MSG1("val = %d\n", val);
		if(val >= 0) block->ret_val = val;
	}

	return FALSE;
}

DECL_OPC_FUN(opc_if)
{
	int ret;

	if(block->cur_line->level > 0 && !block->cond[block->cur_line->level-1])
		return opc_skip_block(block,block->cur_line->level-1,FALSE);

	if(block->cur_line->opcode == OP_ELSEIF && block->cond[block->cur_line->level])
		return opc_skip_block(block,block->cur_line->level,TRUE);

	ret = ifcheck_comparison(block);
	if(ret < 0) return FALSE;

	block->state[block->cur_line->level] = OP_IF;
	block->cond[block->cur_line->level] = ret;
	opc_next_line(block);
	return TRUE;
}

DECL_OPC_FUN(opc_ifnot)
{
	int ret;

	if(block->cur_line->level > 0 && !block->cond[block->cur_line->level-1])
		return opc_skip_block(block,block->cur_line->level-1,FALSE);

	if(block->cur_line->opcode == OP_ELSEIFNOT && block->cond[block->cur_line->level])
		return opc_skip_block(block,block->cur_line->level,TRUE);

	ret = ifcheck_comparison(block);
	if(ret < 0) return FALSE;

	block->state[block->cur_line->level] = OP_IF;
	block->cond[block->cur_line->level] = !ret;
	opc_next_line(block);
	return TRUE;
}

DECL_OPC_FUN(opc_while)
{
	int ret;

	if(block->cur_line->level > 0 && !block->cond[block->cur_line->level-1])
		return opc_skip_block(block,block->cur_line->level-1,FALSE);

	ret = ifcheck_comparison(block);
	if(ret < 0) return FALSE;

	block->state[block->cur_line->level] = OP_WHILE;
	block->cond[block->cur_line->level] = ret;
	opc_next_line(block);
	return TRUE;
}

DECL_OPC_FUN(opc_whilenot)
{
	int ret;

	if(block->cur_line->level > 0 && !block->cond[block->cur_line->level-1])
		return opc_skip_block(block,block->cur_line->level-1,FALSE);

	ret = ifcheck_comparison(block);
	if(ret < 0) return FALSE;

	block->state[block->cur_line->level] = OP_WHILE;
	block->cond[block->cur_line->level] = !ret;
	opc_next_line(block);
	return TRUE;
}


DECL_OPC_FUN(opc_or)
{
	int ret;

	if(block->cur_line->level > 0 && !block->cond[block->cur_line->level-1])
		return opc_skip_block(block,block->cur_line->level-1,FALSE);

	ret = ifcheck_comparison(block);
	if(ret < 0) return FALSE;

	block->cond[block->cur_line->level] = block->cond[block->cur_line->level] || ret;
	opc_next_line(block);
	return TRUE;
}

DECL_OPC_FUN(opc_nor)
{
	int ret;

	if(block->cur_line->level > 0 && !block->cond[block->cur_line->level-1])
		return opc_skip_block(block,block->cur_line->level-1,FALSE);

	ret = ifcheck_comparison(block);
	if(ret < 0) return FALSE;

	block->cond[block->cur_line->level] = block->cond[block->cur_line->level] || !ret;
	opc_next_line(block);
	return TRUE;
}

DECL_OPC_FUN(opc_and)
{
	int ret;

	if(block->cur_line->level > 0 && !block->cond[block->cur_line->level-1])
		return opc_skip_block(block,block->cur_line->level-1,FALSE);

	ret = ifcheck_comparison(block);
	if(ret < 0) return FALSE;

	block->cond[block->cur_line->level] = block->cond[block->cur_line->level] && ret;
	opc_next_line(block);

	return TRUE;
}

DECL_OPC_FUN(opc_nand)
{
	int ret;

	if(block->cur_line->level > 0 && !block->cond[block->cur_line->level-1])
		return opc_skip_block(block,block->cur_line->level-1,FALSE);

	ret = ifcheck_comparison(block);
	if(ret < 0) return FALSE;

	block->cond[block->cur_line->level] = block->cond[block->cur_line->level] && !ret;
	opc_next_line(block);
	return TRUE;
}

DECL_OPC_FUN(opc_else)
{
	if(block->cond[block->cur_line->level])
		return opc_skip_block(block,block->cur_line->level,TRUE);

	// Since the previous check was false, this block must be true
	block->cond[block->cur_line->level] = TRUE;
	opc_next_line(block);
	return TRUE;
}

DECL_OPC_FUN(opc_endif)
{
	// No need to do anything, just keep going.
	// Invalid structures are handled by the preparser
	opc_next_line(block);
	return TRUE;
}

DECL_OPC_FUN(opc_command)
{
	char buf[MSL];
	if(block->cur_line->level > 0 && !block->cond[block->cur_line->level-1])
		return opc_skip_block(block,block->cur_line->level-1,FALSE);

	// Allow only MOBS to do this...
	if(block->type == IFC_M && block->info.mob) {
		expand_string(&block->info,block->cur_line->rest,buf);
		interpret(block->info.mob,buf);
	}
	// Ignore the others

	opc_next_line(block);
	return TRUE;
}

DECL_OPC_FUN(opc_gotoline)
{
	int val;
	SCRIPT_PARAM arg;

	if(block->cur_line->level > 0 && !block->cond[block->cur_line->level-1])
		return opc_skip_block(block,block->cur_line->level-1,FALSE);

	// Anything to evaluate?
	if(block->cur_line->rest[0]) {
		if(!expand_argument(&block->info,block->cur_line->rest,&arg)) {
			block->ret_val = PRET_BADSYNTAX;
			return FALSE;
		}

		switch(arg.type) {
		case ENT_STRING: val = atoi(arg.d.str)-1; break;
		case ENT_NUMBER: val = arg.d.num-1; break;
		default: val = -1; break;
		}

		if(val >= 0 && val < block->script->lines) {
			block->line = val;
			block->cur_line = &block->script->code[val];
			if(block->cur_line->level > 0) block->cond[block->cur_line->level-1] = TRUE;
			return TRUE;
		}
	}

	return FALSE;
}

DECL_OPC_FUN(opc_for)
{
	bool skip = FALSE;
	int lp, end, cur, inc;
	char *str1,*str2;
	SCRIPT_PARAM arg;

	if(block->cur_line->level > 0 && !block->cond[block->cur_line->level-1])
		return opc_skip_block(block,block->cur_line->level-1,FALSE);

	for(lp = block->loop; lp-- > 0;)
		if(block->cur_line->label == block->loops[lp].id)
			break;

	// Initialize loop control
	if(lp < 0) {
		lp = block->loop;

		// Variable Name
		str1 = one_argument(block->cur_line->rest,block->loops[lp].var_name);

		if(!block->loops[lp].var_name[0]) {
			block->ret_val = PRET_BADSYNTAX;
			return FALSE;
		}

		if(!(str2 = expand_argument(&block->info,str1,&arg))) {
			block->ret_val = PRET_BADSYNTAX;
			return FALSE;
		}

		switch(arg.type) {
		case ENT_STRING: cur = atoi(arg.d.str); break;
		case ENT_NUMBER: cur = arg.d.num; break;
		default:
			block->ret_val = PRET_BADSYNTAX;
			return FALSE;
		}

		if(!(str1 = expand_argument(&block->info,str2,&arg))) {
			block->ret_val = PRET_BADSYNTAX;
			return FALSE;
		}

		switch(arg.type) {
		case ENT_STRING: end = atoi(arg.d.str); break;
		case ENT_NUMBER: end = arg.d.num; break;
		default:
			block->ret_val = PRET_BADSYNTAX;
			return FALSE;
		}


		if(!(str2 = expand_argument(&block->info,str1,&arg))) {
			block->ret_val = PRET_BADSYNTAX;
			return FALSE;
		}

		switch(arg.type) {
		case ENT_STRING: inc = atoi(arg.d.str); break;
		case ENT_NUMBER: inc = arg.d.num; break;
		default:
			block->ret_val = PRET_BADSYNTAX;
			return FALSE;
		}

		// No increment?  No looping!
		if(!inc) return opc_skip_to_label(block,OP_ENDFOR,block->cur_line->label,TRUE);

		block->loops[lp].id = block->cur_line->label;
		block->loops[lp].d.f.inc = inc;

		// set the directions correctly
		if((inc > 0) == (cur < end)) {
			block->loops[lp].d.f.cur = cur;
			block->loops[lp].d.f.end = end;
		} else {
			block->loops[lp].d.f.cur = end;
			block->loops[lp].d.f.end = cur;
		}

		// Set the variable
		variables_set_integer(block->info.var,block->loops[lp].var_name,block->loops[lp].d.f.cur);
		block->loop++;
		block->cond[block->cur_line->level] = TRUE;
	} else {
		// Continue loop
		block->loops[lp].d.f.cur += block->loops[lp].d.f.inc;

		// Set the variable
		variables_set_integer(block->info.var,block->loops[lp].var_name,block->loops[lp].d.f.cur);

		if(block->loops[lp].d.f.inc < 0 && (block->loops[lp].d.f.cur < block->loops[lp].d.f.end))
			skip = TRUE;
		else if(block->loops[lp].d.f.inc > 0 && (block->loops[lp].d.f.cur > block->loops[lp].d.f.end))
			skip = TRUE;

		if(skip) {
			block->loop--;
			return opc_skip_to_label(block,OP_ENDFOR,block->loops[lp].id,TRUE);
		}
		block->cond[block->cur_line->level] = TRUE;
	}

	opc_next_line(block);
	return TRUE;
}

DECL_OPC_FUN(opc_endfor)
{
	if(block->cur_line->level > 0 && !block->cond[block->cur_line->level-1])
		return opc_skip_block(block,block->cur_line->level-1,FALSE);

	return opc_skip_to_label(block,OP_FOR,block->cur_line->label,FALSE);
}

DECL_OPC_FUN(opc_exitfor)
{
	if(block->cur_line->level > 0 && !block->cond[block->cur_line->level-1])
		return opc_skip_block(block,block->cur_line->level-1,FALSE);

	return opc_skip_to_label(block,OP_ENDFOR,block->cur_line->label,TRUE);
}


DECL_OPC_FUN(opc_list)
{
	bool skip = FALSE;
	int lp, i;
	char *str1,*str2, *str;
	char buf[MSL];
	LLIST_UID_DATA *uid;
	LLIST_ROOM_DATA *lrd;
	LLIST_EXIT_DATA *led;
	LLIST_SKILL_DATA *lsk;
	LLIST_AREA_DATA *lar;
	LLIST_WILDS_DATA *lwd;
	DESCRIPTOR_DATA *conn;
	CHAR_DATA *ch;
	OBJ_DATA *obj;
	TOKEN_DATA *tok;
	ROOM_INDEX_DATA *here;
	EXIT_DATA *ex;
	CHURCH_DATA *church;
	SCRIPT_PARAM arg;

	if(block->cur_line->level > 0 && !block->cond[block->cur_line->level-1])
		return opc_skip_block(block,block->cur_line->level-1,FALSE);

	for(lp = block->loop; lp-- > 0;)
		if(block->cur_line->label == block->loops[lp].id)
			break;

	// Initialize loop control
	if(lp < 0) {
		lp = block->loop;

		// Variable Name
		str1 = one_argument(block->cur_line->rest,block->loops[lp].var_name);

		log_stringf("opc_list: initializing for loop variable '%s'", block->loops[lp].var_name);

		if(!block->loops[lp].var_name[0]) {
			block->ret_val = PRET_BADSYNTAX;
			return FALSE;
		}

		// Get the LIST
		if(!(str2 = expand_argument(&block->info,str1,&arg))) {
			block->ret_val = PRET_BADSYNTAX;
			return FALSE;
		}

		block->loops[lp].counter = 1;

		switch(arg.type) {
		case ENT_STRING:
			log_stringf("opc_list: list type ENT_STRING");
			if(IS_NULLSTR(arg.d.str))
				return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);

			strncpy(block->loops[lp].buf, arg.d.str, MSL-1);
			str = one_argument_norm(block->loops[lp].buf, buf);

			block->loops[lp].d.l.type = ENT_STRING;
			block->loops[lp].d.l.cur.str = NULL;
			block->loops[lp].d.l.next.str = str;
			block->loops[lp].d.l.owner = NULL;


			// Set the variable
			variables_set_string(block->info.var,block->loops[lp].var_name,buf,FALSE);
			break;
		case ENT_EXIT:
			log_stringf("opc_list: list type ENT_EXIT");
			if(!arg.d.door.r)
				return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);

			here = arg.d.door.r;
			ex = here->exit[arg.d.door.door];
			block->loops[lp].d.l.type = ENT_EXIT;
			block->loops[lp].d.l.cur.door = arg.d.door.door;
			block->loops[lp].d.l.next.door = MAX_DIR;
			block->loops[lp].d.l.owner = arg.d.door.r;

			if(ex) {
				if(here->wilds)
					log_stringf("opc_list: %s(%ld,%ld,%ld)", dir_name[arg.d.door.door], here->wilds->uid, here->x, here->y);
				else if(here->source)
					log_stringf("opc_list: %s(%ld,%s,%ld,%ld)", dir_name[arg.d.door.door], here->vnum, here->name, here->id[0], here->id[1]);
				else
					log_stringf("opc_list: %s(%ld,%s)", dir_name[arg.d.door.door], here->vnum, here->name);
			} else
				log_stringf("opc_list: exit(<END>)");


			if(ex) {
				here = arg.d.door.r;
				for(i=arg.d.door.door + 1; i < MAX_DIR && !here->exit[i]; i++);
				block->loops[lp].d.l.next.door = i;
			}
			// Set the variable
			variables_set_exit(block->info.var,block->loops[lp].var_name,ex);
			break;

		case ENT_OLLIST_MOB:
			log_stringf("opc_list: list type ENT_MOBILE");
			if(!arg.d.list.ptr.mob || !*arg.d.list.ptr.mob)
				return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);

			block->loops[lp].d.l.type = ENT_MOBILE;
			block->loops[lp].d.l.cur.m = *arg.d.list.ptr.mob;
			block->loops[lp].d.l.next.m = block->loops[lp].d.l.cur.m->next_in_room;
			block->loops[lp].d.l.owner = arg.d.list.owner;

			if(block->loops[lp].d.l.cur.m) {
				ch = block->loops[lp].d.l.cur.m;
				if(!IS_NPC(ch))
					log_stringf("opc_list: player(%s,%ld,%ld)", ch->name, ch->id[0], ch->id[1]);
				else
					log_stringf("opc_list: mobile(%ld,%ld,%ld)", ch->pIndexData->vnum, ch->id[0], ch->id[1]);
			} else
				log_stringf("opc_list: mobile(<END>)");

			// Set the variable
			variables_set_mobile(block->info.var,block->loops[lp].var_name,*arg.d.list.ptr.mob);
			break;

		case ENT_OLLIST_OBJ:
			log_stringf("opc_list: list type ENT_OBJECT");
			if(!arg.d.list.ptr.obj || !*arg.d.list.ptr.obj)
				return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);

			block->loops[lp].d.l.type = ENT_OBJECT;
			block->loops[lp].d.l.cur.o = *arg.d.list.ptr.obj;
			block->loops[lp].d.l.next.o = block->loops[lp].d.l.cur.o->next_content;
			block->loops[lp].d.l.owner = arg.d.list.owner;

			if(block->loops[lp].d.l.cur.o)
				log_stringf("opc_list: object(%ld,%ld,%ld)", block->loops[lp].d.l.cur.o->pIndexData->vnum, block->loops[lp].d.l.cur.o->id[0], block->loops[lp].d.l.cur.o->id[1]);
			else
				log_stringf("opc_list: object(<END>)");

			// Set the variable
			variables_set_object(block->info.var,block->loops[lp].var_name,*arg.d.list.ptr.obj);
			break;

		case ENT_OLLIST_TOK:
			log_stringf("opc_list: list type ENT_TOKEN");
			if(!arg.d.list.ptr.tok || !*arg.d.list.ptr.tok)
				return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);

			block->loops[lp].d.l.type = ENT_TOKEN;
			block->loops[lp].d.l.cur.t = *arg.d.list.ptr.tok;
			block->loops[lp].d.l.next.t = block->loops[lp].d.l.cur.t->next;
			block->loops[lp].d.l.owner = arg.d.list.owner;

			if(block->loops[lp].d.l.cur.t)
				log_stringf("opc_list: token(%ld,%ld,%ld)", block->loops[lp].d.l.cur.t->pIndexData->vnum, block->loops[lp].d.l.cur.t->id[0], block->loops[lp].d.l.cur.t->id[1]);
			else
				log_stringf("opc_list: token(<END>)");

			// Set the variable
			variables_set_token(block->info.var,block->loops[lp].var_name,*arg.d.list.ptr.tok);
			break;

		case ENT_OLLIST_AFF:
			log_stringf("opc_list: list type ENT_AFFECT");
			if(!arg.d.list.ptr.aff || !*arg.d.list.ptr.aff)
				return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);

			block->loops[lp].d.l.type = ENT_AFFECT;
			block->loops[lp].d.l.cur.aff = *arg.d.list.ptr.aff;
			block->loops[lp].d.l.next.aff = block->loops[lp].d.l.cur.aff->next;
			block->loops[lp].d.l.owner = arg.d.list.owner;

			if(block->loops[lp].d.l.cur.aff) {
				if(block->loops[lp].d.l.cur.aff->custom_name)
					log_stringf("opc_list: affect(%s)", block->loops[lp].d.l.cur.aff->custom_name);
				else
					log_stringf("opc_list: affect(%s)", skill_table[block->loops[lp].d.l.cur.aff->type].name);
			} else
				log_stringf("opc_list: affect(<END>)");

			// Set the variable
			variables_set_affect(block->info.var,block->loops[lp].var_name,*arg.d.list.ptr.aff);
			break;

		case ENT_PLLIST_STR:
			log_stringf("opc_list: list type ENT_PLLIST_STR");
			if(!arg.d.blist || !arg.d.blist->valid)
				return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);

			block->loops[lp].d.l.type = ENT_PLLIST_STR;
			block->loops[lp].d.l.list.lp = arg.d.blist;
			iterator_start(&block->loops[lp].d.l.list.it,arg.d.blist);
			block->loops[lp].d.l.owner = NULL;

			str = (char *)iterator_nextdata(&block->loops[lp].d.l.list.it);

			if(str)
				log_stringf("opc_list: string(%s)",str);
			else
				log_stringf("opc_list: string(<END>)");

			if( !str ) {
				iterator_stop(&block->loops[lp].d.l.list.it);
				return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);
			}

			// Set the variable
			variables_set_string(block->info.var,block->loops[lp].var_name,str,FALSE);
			break;

		case ENT_BLLIST_MOB:
			log_stringf("opc_list: list type ENT_BLLIST_MOB");
			if(!arg.d.blist || !arg.d.blist->valid)
				return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);

			block->loops[lp].d.l.type = ENT_BLLIST_MOB;
			block->loops[lp].d.l.list.lp = arg.d.blist;
			iterator_start(&block->loops[lp].d.l.list.it,arg.d.blist);
			block->loops[lp].d.l.owner = NULL;

			do {
				uid = (LLIST_UID_DATA *)iterator_nextdata(&block->loops[lp].d.l.list.it);
				if( !uid ) {
					log_stringf("opc_list: mobile(<END>)");
					iterator_stop(&block->loops[lp].d.l.list.it);
					return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);
				}

				// Set the variable
				if( IS_VALID((CHAR_DATA *)uid->ptr) )
					variables_set_mobile(block->info.var,block->loops[lp].var_name,(CHAR_DATA *)uid->ptr);

			} while( !IS_VALID((CHAR_DATA *)uid->ptr) );

			ch = (CHAR_DATA *)(uid->ptr);
			if(!IS_NPC(ch))
				log_stringf("opc_list: player(%s,%ld,%ld)", ch->name, ch->id[0], ch->id[1]);
			else
				log_stringf("opc_list: mobile(%ld,%ld,%ld)", ch->pIndexData->vnum, ch->id[0], ch->id[1]);
			break;

		case ENT_BLLIST_OBJ:
			log_stringf("opc_list: list type ENT_BLLIST_OBJ");
			if(!arg.d.blist || !arg.d.blist->valid)
				return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);

			block->loops[lp].d.l.type = ENT_BLLIST_OBJ;
			block->loops[lp].d.l.list.lp = arg.d.blist;
			iterator_start(&block->loops[lp].d.l.list.it,arg.d.blist);
			block->loops[lp].d.l.owner = NULL;

			do {
				uid = (LLIST_UID_DATA *)iterator_nextdata(&block->loops[lp].d.l.list.it);
				if( !uid ) {
					log_stringf("opc_list: object(<END>)");
					iterator_stop(&block->loops[lp].d.l.list.it);
					return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);
				}

				// Set the variable
				if( IS_VALID((OBJ_DATA *)uid->ptr) )
					variables_set_object(block->info.var,block->loops[lp].var_name,(OBJ_DATA *)uid->ptr);

			} while( !IS_VALID((OBJ_DATA *)uid->ptr) );

			obj = (OBJ_DATA *)(uid->ptr);
			log_stringf("opc_list: object(%ld,%ld,%ld)", obj->pIndexData->vnum, obj->id[0], obj->id[1]);

			break;

		case ENT_BLLIST_TOK:
			log_stringf("opc_list: list type ENT_BLLIST_TOK");
			if(!arg.d.blist || !arg.d.blist->valid)
				return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);

			block->loops[lp].d.l.type = ENT_BLLIST_TOK;
			block->loops[lp].d.l.list.lp = arg.d.blist;
			iterator_start(&block->loops[lp].d.l.list.it,arg.d.blist);
			block->loops[lp].d.l.owner = NULL;

			do {
				uid = (LLIST_UID_DATA *)iterator_nextdata(&block->loops[lp].d.l.list.it);
				if( !uid ) {
					log_stringf("opc_list: token(<END>)");
					iterator_stop(&block->loops[lp].d.l.list.it);
					return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);
				}

				// Set the variable
				if( IS_VALID((TOKEN_DATA *)uid->ptr) )
					variables_set_token(block->info.var,block->loops[lp].var_name,(TOKEN_DATA *)uid->ptr);
			} while( !IS_VALID((TOKEN_DATA *)uid->ptr) );
			tok = (TOKEN_DATA *)(uid->ptr);
			log_stringf("opc_list: token(%ld,%ld,%ld)", tok->pIndexData->vnum, tok->id[0], tok->id[1]);
			break;

		case ENT_BLLIST_ROOM:
			log_stringf("opc_list: list type ENT_BLLIST_ROOM");
			if(!arg.d.blist || !arg.d.blist->valid)
				return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);

			block->loops[lp].d.l.type = ENT_BLLIST_ROOM;
			block->loops[lp].d.l.list.lp = arg.d.blist;
			iterator_start(&block->loops[lp].d.l.list.it,arg.d.blist);
			block->loops[lp].d.l.owner = NULL;

			do {
				lrd = (LLIST_ROOM_DATA *)iterator_nextdata(&block->loops[lp].d.l.list.it);
				log_stringf("opc_list: lrd = %016lX", lrd);
				if( !lrd ) {
					log_stringf("opc_list: room(<END>)");
					iterator_stop(&block->loops[lp].d.l.list.it);
					return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);
				}

				// Set the variable
				if( lrd->room )
					variables_set_room(block->info.var,block->loops[lp].var_name,lrd->room);

			} while( !lrd->room );

			log_stringf("opc_list: lrd->room = %016lX", lrd->room);
			if(lrd->room->wilds)
				log_stringf("opc_list: room(%ld,%ld,%ld)", lrd->room->wilds->uid, lrd->room->x, lrd->room->y);
			else if(lrd->room->source)
				log_stringf("opc_list: room(%ld,%s,%ld,%ld)", lrd->room->vnum, lrd->room->name, lrd->room->id[0], lrd->room->id[1]);
			else
				log_stringf("opc_list: room(%ld,%s)", lrd->room->vnum, lrd->room->name);

			break;

		case ENT_BLLIST_EXIT:
			log_stringf("opc_list: list type ENT_BLLIST_EXIT");
			if(!arg.d.blist || !arg.d.blist->valid)
				return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);

			block->loops[lp].d.l.type = ENT_BLLIST_EXIT;
			block->loops[lp].d.l.list.lp = arg.d.blist;
			iterator_start(&block->loops[lp].d.l.list.it,arg.d.blist);
			block->loops[lp].d.l.owner = NULL;

			do {
				led = (LLIST_EXIT_DATA *)iterator_nextdata(&block->loops[lp].d.l.list.it);
				if( !led ) {
					log_stringf("opc_list: exit(<END>)");
					iterator_stop(&block->loops[lp].d.l.list.it);
					return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);
				}

				// Set the variable
				if( led->room && led->door >= 0 && led->door < MAX_DIR && led->room->exit[led->door])
					variables_set_door(block->info.var,block->loops[lp].var_name,led->room, led->door, FALSE);

			} while( !led->room || led->door < 0 || led->door >= MAX_DIR || !led->room->exit[led->door] );

			if(led->room->wilds)
				log_stringf("opc_list: %s(%ld,%ld,%ld)", dir_name[led->door], led->room->wilds->uid, led->room->x, led->room->y);
			else if(led->room->source)
				log_stringf("opc_list: %s(%ld,%s,%ld,%ld)", dir_name[led->door], led->room->vnum, led->room->name, led->room->id[0], led->room->id[1]);
			else
				log_stringf("opc_list: %s(%ld,%s)", dir_name[led->door], led->room->vnum, led->room->name);

			break;

		case ENT_BLLIST_SKILL:
			log_stringf("opc_list: list type ENT_BLLIST_SKILL");
			if(!arg.d.blist || !arg.d.blist->valid)
				return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);

			block->loops[lp].d.l.type = ENT_BLLIST_SKILL;
			block->loops[lp].d.l.list.lp = arg.d.blist;
			iterator_start(&block->loops[lp].d.l.list.it,arg.d.blist);
			block->loops[lp].d.l.owner = NULL;

			do {
				lsk = (LLIST_SKILL_DATA *)iterator_nextdata(&block->loops[lp].d.l.list.it);
				if( !lsk ) {
					log_stringf("opc_list: skill(<END>)");
					iterator_stop(&block->loops[lp].d.l.list.it);
					return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);
				}

				// Set the variable
				if( IS_VALID(lsk->mob) && (IS_VALID(lsk->tok) || ( lsk->sn > 0 && lsk->sn < MAX_SKILL )) )
					variables_setsave_skillinfo(block->info.var,block->loops[lp].var_name, lsk->mob, lsk->sn, lsk->tok, FALSE);

			} while( !IS_VALID(lsk->mob) || (!IS_VALID(lsk->tok) && ( lsk->sn < 1 || lsk->sn >= MAX_SKILL )) );

			if(lsk->tok)
				log_stringf("opc_list: skill(%ld,%ld,TOKEN,%s)", lsk->mob->id[0], lsk->mob->id[1], lsk->tok->name);
			else
				log_stringf("opc_list: skill(%ld,%ld,SKILL,%s)", lsk->mob->id[0], lsk->mob->id[1], skill_table[lsk->sn].name);

			break;

		case ENT_BLLIST_AREA:
			log_stringf("opc_list: list type ENT_BLLIST_AREA");
			if(!arg.d.blist || !arg.d.blist->valid)
				return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);

			block->loops[lp].d.l.type = ENT_BLLIST_AREA;
			block->loops[lp].d.l.list.lp = arg.d.blist;
			iterator_start(&block->loops[lp].d.l.list.it,arg.d.blist);
			block->loops[lp].d.l.owner = NULL;

			do {
				lar = (LLIST_AREA_DATA *)iterator_nextdata(&block->loops[lp].d.l.list.it);
				if( !lar ) {
					log_stringf("opc_list: area(<END>)");
					iterator_stop(&block->loops[lp].d.l.list.it);
					return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);
				}

				// Set the variable
				if( lar->area )
					variables_setsave_area(block->info.var,block->loops[lp].var_name, lar->area, FALSE);

			} while( !lar->area );

			log_stringf("opc_list: area(%ld,%s)", lar->area->uid, lar->area->name);

			break;

		case ENT_BLLIST_WILDS:
			log_stringf("opc_list: list type ENT_BLLIST_WILDS");
			if(!arg.d.blist || !arg.d.blist->valid)
				return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);

			block->loops[lp].d.l.type = ENT_BLLIST_WILDS;
			block->loops[lp].d.l.list.lp = arg.d.blist;
			iterator_start(&block->loops[lp].d.l.list.it,arg.d.blist);
			block->loops[lp].d.l.owner = NULL;

			do {
				lwd = (LLIST_WILDS_DATA *)iterator_nextdata(&block->loops[lp].d.l.list.it);
				if( !lwd ) {
					log_stringf("opc_list: wilds(<END>)");
					iterator_stop(&block->loops[lp].d.l.list.it);
					return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);
				}

				// Set the variable
				if( lwd->wilds )
					variables_setsave_wilds(block->info.var,block->loops[lp].var_name, lwd->wilds, FALSE);

			} while( !lwd->wilds );

			log_stringf("opc_list: wilds(%ld,%s)", lwd->wilds->uid, lwd->wilds->name);

			break;

		case ENT_PLLIST_CONN:
			log_stringf("opc_list: list type ENT_PLLIST_CONN");
			if(!arg.d.blist || !arg.d.blist->valid)
				return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);

			block->loops[lp].d.l.type = ENT_PLLIST_CONN;
			block->loops[lp].d.l.list.lp = arg.d.blist;
			iterator_start(&block->loops[lp].d.l.list.it,arg.d.blist);
			block->loops[lp].d.l.owner = NULL;

			conn = (DESCRIPTOR_DATA *)iterator_nextdata(&block->loops[lp].d.l.list.it);

			if(conn) {
				if(conn->original)
					log_stringf("opc_list: connection(%s,%d) [SWITCHED]", conn->original->name, conn->original->tot_level);
				else
					log_stringf("opc_list: connection(%s,%d)", conn->character->name, conn->character->tot_level);
			} else
				log_stringf("opc_list: connection(<END>)");

			if( !conn ) {
				iterator_stop(&block->loops[lp].d.l.list.it);
				return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);
			}

			// Set the variable
			variables_set_connection(block->info.var,block->loops[lp].var_name,conn);
			break;

		case ENT_PLLIST_MOB:
			log_stringf("opc_list: list type ENT_PLLIST_MOB");
			if(!arg.d.blist || !arg.d.blist->valid)
				return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);

			block->loops[lp].d.l.type = ENT_PLLIST_MOB;
			block->loops[lp].d.l.list.lp = arg.d.blist;
			iterator_start(&block->loops[lp].d.l.list.it,arg.d.blist);
			block->loops[lp].d.l.owner = NULL;

			ch = (CHAR_DATA *)iterator_nextdata(&block->loops[lp].d.l.list.it);

			if(ch) {
				if(!IS_NPC(ch))
					log_stringf("opc_list: player(%s,%ld,%ld)", ch->name, ch->id[0], ch->id[1]);
				else
					log_stringf("opc_list: mobile(%ld,%ld,%ld)", ch->pIndexData->vnum, ch->id[0], ch->id[1]);
			} else
				log_stringf("opc_list: mobile(<END>)");

			if( !ch ) {
				iterator_stop(&block->loops[lp].d.l.list.it);
				return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);
			}

			// Set the variable
			variables_set_mobile(block->info.var,block->loops[lp].var_name,ch);
			break;

		case ENT_PLLIST_OBJ:
			log_stringf("opc_list: list type ENT_PLLIST_OBJ");
			if(!arg.d.blist || !arg.d.blist->valid)
				return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);

			block->loops[lp].d.l.type = ENT_PLLIST_OBJ;
			block->loops[lp].d.l.list.lp = arg.d.blist;
			iterator_start(&block->loops[lp].d.l.list.it,arg.d.blist);
			block->loops[lp].d.l.owner = NULL;

			obj = (OBJ_DATA *)iterator_nextdata(&block->loops[lp].d.l.list.it);

			if(obj)
				log_stringf("opc_list: object(%ld,%ld,%ld)", obj->pIndexData->vnum, obj->id[0], obj->id[1]);
			else
				log_stringf("opc_list: object(<END>)");

			if( !obj ) {
				iterator_stop(&block->loops[lp].d.l.list.it);
				return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);
			}

			// Set the variable
			variables_set_object(block->info.var,block->loops[lp].var_name,obj);
			break;

		case ENT_PLLIST_ROOM:
			log_stringf("opc_list: list type ENT_PLLIST_ROOM");
			if(!arg.d.blist || !arg.d.blist->valid)
				return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);

			block->loops[lp].d.l.type = ENT_PLLIST_ROOM;
			block->loops[lp].d.l.list.lp = arg.d.blist;
			iterator_start(&block->loops[lp].d.l.list.it,arg.d.blist);
			block->loops[lp].d.l.owner = NULL;

			here = (ROOM_INDEX_DATA *)iterator_nextdata(&block->loops[lp].d.l.list.it);

			if(here) {
				if(here->wilds)
					log_stringf("opc_list: room(%ld,%ld,%ld)", here->wilds->uid, here->x, here->y);
				else if(here->source)
					log_stringf("opc_list: room(%ld,%s,%ld,%ld)", here->vnum, here->name, here->id[0], here->id[1]);
				else
					log_stringf("opc_list: room(%ld,%s)", here->vnum, here->name);
			} else
				log_stringf("opc_list: room(<END>)");

			if( !here ) {
				iterator_stop(&block->loops[lp].d.l.list.it);
				return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);
			}

			// Set the variable
			variables_set_room(block->info.var,block->loops[lp].var_name,here);
			break;

		case ENT_PLLIST_TOK:
			log_stringf("opc_list: list type ENT_PLLIST_TOK");
			if(!arg.d.blist || !arg.d.blist->valid)
				return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);

			block->loops[lp].d.l.type = ENT_PLLIST_TOK;
			block->loops[lp].d.l.list.lp = arg.d.blist;
			iterator_start(&block->loops[lp].d.l.list.it,arg.d.blist);
			block->loops[lp].d.l.owner = NULL;

			tok = (TOKEN_DATA *)iterator_nextdata(&block->loops[lp].d.l.list.it);

			if(tok)
				log_stringf("opc_list: token(%ld,%ld,%ld)", tok->pIndexData->vnum, tok->id[0], tok->id[1]);
			else
				log_stringf("opc_list: token(<END>)");

			if( !tok ) {
				iterator_stop(&block->loops[lp].d.l.list.it);
				return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);
			}

			// Set the variable
			variables_set_token(block->info.var,block->loops[lp].var_name,tok);
			break;

		case ENT_PLLIST_CHURCH:
			log_stringf("opc_list: list type ENT_PLLIST_CHURCH");
			if(!arg.d.blist || !arg.d.blist->valid)
				return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);

			block->loops[lp].d.l.type = ENT_PLLIST_CHURCH;
			block->loops[lp].d.l.list.lp = arg.d.blist;
			iterator_start(&block->loops[lp].d.l.list.it,arg.d.blist);
			block->loops[lp].d.l.owner = NULL;

			church = (CHURCH_DATA *)iterator_nextdata(&block->loops[lp].d.l.list.it);

			log_stringf("opc_list: church(%s)", church ? church->name : "<END>");

			if( !church ) {
				iterator_stop(&block->loops[lp].d.l.list.it);
				return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);
			}

			// Set the variable
			variables_set_church(block->info.var,block->loops[lp].var_name,church);
			break;

		default:
			log_stringf("opc_list: list_type INVALID");
			block->ret_val = PRET_BADSYNTAX;
			return FALSE;
		}

		block->loops[lp].id = block->cur_line->label;
		block->loop++;
		block->cond[block->cur_line->level] = TRUE;
	} else {
		log_stringf("opc_list: next loop variable '%s'", block->loops[lp].var_name);
		block->loops[lp].counter++;

		// Continue loop
		switch(block->loops[lp].d.l.type) {
		case ENT_STRING:
			log_stringf("opc_list: list type ENT_STRING");
			str = block->loops[lp].d.l.next.str;

			if( IS_NULLSTR(str) )
			{
				skip = TRUE;
				break;
			}

			str = one_argument_norm(str,buf);

			variables_set_string(block->info.var,block->loops[lp].var_name,buf,FALSE);

			block->loops[lp].d.l.next.str = str;
			break;


		case ENT_EXIT:
			log_stringf("opc_list: list type ENT_EXIT");
			i = block->loops[lp].d.l.cur.door = block->loops[lp].d.l.next.door;

			if( i >= MAX_DIR ) {
				skip = TRUE;
				break;
			}

			here = (ROOM_INDEX_DATA *)block->loops[lp].d.l.owner;
			ex = here->exit[i];

			if(ex) {
				if(here->wilds)
					log_stringf("opc_list: %s(%ld,%ld,%ld)", dir_name[i], here->wilds->uid, here->x, here->y);
				else if(here->source)
					log_stringf("opc_list: %s(%ld,%s,%ld,%ld)", dir_name[i], here->vnum, here->name, here->id[0], here->id[1]);
				else
					log_stringf("opc_list: %s(%ld,%s)", dir_name[i], here->vnum, here->name);
			} else
				log_stringf("opc_list: exit(<END>)");


			// Set the variable
			variables_set_exit(block->info.var,block->loops[lp].var_name,ex);

			if(!ex) {
				skip = TRUE;
				break;
			}

			for(i++; i < MAX_DIR && !here->exit[i]; i++);

			block->loops[lp].d.l.next.door = i;
			break;

		case ENT_MOBILE:
			log_stringf("opc_list: list type ENT_MOBILE");
			block->loops[lp].d.l.cur.m = block->loops[lp].d.l.next.m;
			// Set the variable
			variables_set_mobile(block->info.var,block->loops[lp].var_name,block->loops[lp].d.l.cur.m);

			if(block->loops[lp].d.l.cur.m) {
				ch = block->loops[lp].d.l.cur.m;
				if(!IS_NPC(ch))
					log_stringf("opc_list: player(%s,%ld,%ld)", ch->name, ch->id[0], ch->id[1]);
				else
					log_stringf("opc_list: mobile(%ld,%ld,%ld)", ch->pIndexData->vnum, ch->id[0], ch->id[1]);
			} else
				log_stringf("opc_list: mobile(<END>)");

			if(!block->loops[lp].d.l.cur.m) {
				skip = TRUE;
				break;
			}

			block->loops[lp].d.l.next.m = block->loops[lp].d.l.cur.m->next_in_room;
			break;

		case ENT_OBJECT:
			log_stringf("opc_list: list type ENT_OBJECT");
			block->loops[lp].d.l.cur.o = block->loops[lp].d.l.next.o;
			// Set the variable
			variables_set_object(block->info.var,block->loops[lp].var_name,block->loops[lp].d.l.cur.o);

			if(block->loops[lp].d.l.cur.o)
				log_stringf("opc_list: object(%ld,%ld,%ld)", block->loops[lp].d.l.cur.o->pIndexData->vnum, block->loops[lp].d.l.cur.o->id[0], block->loops[lp].d.l.cur.o->id[1]);
			else
				log_stringf("opc_list: object(<END>)");

			if(!block->loops[lp].d.l.cur.o) {
				skip = TRUE;
				break;
			}

			block->loops[lp].d.l.next.o = block->loops[lp].d.l.cur.o->next_content;
			break;

		case ENT_TOKEN:
			log_stringf("opc_list: list type ENT_TOKEN");
			block->loops[lp].d.l.cur.t = block->loops[lp].d.l.next.t;
			// Set the variable
			variables_set_token(block->info.var,block->loops[lp].var_name,block->loops[lp].d.l.cur.t);

			if(block->loops[lp].d.l.cur.t)
				log_stringf("opc_list: token(%ld,%ld,%ld)", block->loops[lp].d.l.cur.t->pIndexData->vnum, block->loops[lp].d.l.cur.t->id[0], block->loops[lp].d.l.cur.t->id[1]);
			else
				log_stringf("opc_list: token(<END>)");

			if(!block->loops[lp].d.l.cur.t) {
				skip = TRUE;
				break;
			}

			block->loops[lp].d.l.next.t = block->loops[lp].d.l.cur.t->next;
			break;

		case ENT_AFFECT:
			log_stringf("opc_list: list type ENT_AFFECT");
			block->loops[lp].d.l.cur.aff = block->loops[lp].d.l.next.aff;
			// Set the variable
			variables_set_affect(block->info.var,block->loops[lp].var_name,block->loops[lp].d.l.cur.aff);

			if(block->loops[lp].d.l.cur.aff) {
				if(block->loops[lp].d.l.cur.aff->custom_name)
					log_stringf("opc_list: affect(%s)", block->loops[lp].d.l.cur.aff->custom_name);
				else
					log_stringf("opc_list: affect(%s)", skill_table[block->loops[lp].d.l.cur.aff->type].name);
			} else
				log_stringf("opc_list: affect(<END>)");

			if(!block->loops[lp].d.l.cur.aff) {
				skip = TRUE;
				break;
			}

			block->loops[lp].d.l.next.aff = block->loops[lp].d.l.cur.aff->next;
			break;

		case ENT_PLLIST_STR:
			log_stringf("opc_list: list type ENT_PLLIST_STR");
			str = (char *)iterator_nextdata(&block->loops[lp].d.l.list.it);

			if(str)
				log_stringf("opc_list: string(%s)",str);
			else
				log_stringf("opc_list: string(<END>)");

			// Set the variable
			variables_set_string(block->info.var,block->loops[lp].var_name,str,FALSE);

			if( !str ) {
				iterator_stop(&block->loops[lp].d.l.list.it);
				return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);
			}

			break;

		case ENT_BLLIST_MOB:
			log_stringf("opc_list: list type ENT_BLLIST_MOB");
			while( (uid = (LLIST_UID_DATA *)iterator_nextdata(&block->loops[lp].d.l.list.it)) && !IS_VALID((CHAR_DATA *)uid->ptr) );
			if(uid) {
				ch = (CHAR_DATA *)(uid->ptr);
				if(!IS_NPC(ch))
					log_stringf("opc_list: player(%s,%ld,%ld)", ch->name, ch->id[0], ch->id[1]);
				else
					log_stringf("opc_list: mobile(%ld,%ld,%ld)", ch->pIndexData->vnum, ch->id[0], ch->id[1]);
			} else
				log_stringf("opc_list: mobile(<END>)");

			variables_set_mobile(block->info.var,block->loops[lp].var_name,(CHAR_DATA *)(uid?uid->ptr:NULL));

			if( !uid ) {
				iterator_stop(&block->loops[lp].d.l.list.it);
				skip = TRUE;
				break;
			}

			break;

		case ENT_BLLIST_OBJ:
			log_stringf("opc_list: list type ENT_BLLIST_OBJ");
			while( (uid = (LLIST_UID_DATA *)iterator_nextdata(&block->loops[lp].d.l.list.it)) && !IS_VALID((OBJ_DATA *)uid->ptr) );
			if(uid) {
				obj = (OBJ_DATA *)(uid->ptr);
				log_stringf("opc_list: object(%ld,%ld,%ld)", obj->pIndexData->vnum, obj->id[0], obj->id[1]);
			} else
				log_stringf("opc_list: object(<END>)");

			variables_set_object(block->info.var,block->loops[lp].var_name,(OBJ_DATA *)(uid?uid->ptr:NULL));

			if( !uid ) {
				iterator_stop(&block->loops[lp].d.l.list.it);
				skip = TRUE;
				break;
			}

			break;

		case ENT_BLLIST_TOK:
			log_stringf("opc_list: list type ENT_BLLIST_TOK");
			while( (uid = (LLIST_UID_DATA *)iterator_nextdata(&block->loops[lp].d.l.list.it)) && !IS_VALID((TOKEN_DATA *)uid->ptr) );

			if(uid) {
				tok = (TOKEN_DATA *)(uid->ptr);
				log_stringf("opc_list: token(%ld,%ld,%ld)", tok->pIndexData->vnum, tok->id[0], tok->id[1]);
			} else
				log_stringf("opc_list: token(<END>)");

			variables_set_token(block->info.var,block->loops[lp].var_name,(TOKEN_DATA *)(uid?uid->ptr:NULL));

			if( !uid ) {
				iterator_stop(&block->loops[lp].d.l.list.it);
				skip = TRUE;
				break;
			}

			break;

		case ENT_BLLIST_ROOM:
			log_stringf("opc_list: list type ENT_BLLIST_ROOM");
			while( (lrd = (LLIST_ROOM_DATA *)iterator_nextdata(&block->loops[lp].d.l.list.it)) && !lrd->room );
			if(lrd) {
				if(lrd->room->wilds)
					log_stringf("opc_list: room(%ld,%ld,%ld)", lrd->room->wilds->uid, lrd->room->x, lrd->room->y);
				else if(lrd->room->source)
					log_stringf("opc_list: room(%ld,%s,%ld,%ld)", lrd->room->vnum, lrd->room->name, lrd->room->id[0], lrd->room->id[1]);
				else
					log_stringf("opc_list: room(%ld,%s)", lrd->room->vnum, lrd->room->name);
			} else
				log_stringf("opc_list: room(<END>)");

			variables_set_room(block->info.var,block->loops[lp].var_name,lrd?lrd->room:NULL);

			if( !lrd ) {
				iterator_stop(&block->loops[lp].d.l.list.it);
				skip = TRUE;
				break;
			}
			break;

		case ENT_BLLIST_EXIT:
			log_stringf("opc_list: list type ENT_BLLIST_EXIT");
			while( (led = (LLIST_EXIT_DATA *)iterator_nextdata(&block->loops[lp].d.l.list.it)) &&
				(!led->room || led->door < 0 || led->door >= MAX_DIR || !led->room->exit[led->door]));

			if(led) {
				if(led->room->wilds)
					log_stringf("opc_list: %s(%ld,%ld,%ld)", dir_name[led->door], led->room->wilds->uid, led->room->x, led->room->y);
				else if(led->room->source)
					log_stringf("opc_list: %s(%ld,%s,%ld,%ld)", dir_name[led->door], led->room->vnum, led->room->name, led->room->id[0], led->room->id[1]);
				else
					log_stringf("opc_list: %s(%ld,%s)", dir_name[led->door], led->room->vnum, led->room->name);
			} else
				log_stringf("opc_list: exit(<END>)");

			if( !led ) {
				variables_set_door(block->info.var,block->loops[lp].var_name,NULL, DIR_NORTH, FALSE);
				iterator_stop(&block->loops[lp].d.l.list.it);
				skip = TRUE;
				break;
			}

			variables_set_door(block->info.var,block->loops[lp].var_name,led->room, led->door, FALSE);
			break;

		case ENT_BLLIST_SKILL:
			log_stringf("opc_list: list type ENT_BLLIST_SKILL");
			while( (lsk = (LLIST_SKILL_DATA *)iterator_nextdata(&block->loops[lp].d.l.list.it)) &&
				(!IS_VALID(lsk->mob) || (!IS_VALID(lsk->tok) && ( lsk->sn < 1 || lsk->sn >= MAX_SKILL ))) );
			if(lsk) {
				if(lsk->tok)
					log_stringf("opc_list: skill(%ld,%ld,TOKEN,%s)", lsk->mob->id[0], lsk->mob->id[1], lsk->tok->name);
				else
					log_stringf("opc_list: skill(%ld,%ld,SKILL,%s)", lsk->mob->id[0], lsk->mob->id[1], skill_table[lsk->sn].name);
			} else
				log_stringf("opc_list: skill(<END>)");


			if( !lsk ) {
				variables_setsave_skillinfo(block->info.var,block->loops[lp].var_name, NULL, 0, NULL, FALSE);
				iterator_stop(&block->loops[lp].d.l.list.it);
				skip = TRUE;
				break;
			}

			variables_setsave_skillinfo(block->info.var,block->loops[lp].var_name, lsk->mob, lsk->sn, lsk->tok, FALSE);
			break;

		case ENT_BLLIST_AREA:
			log_stringf("opc_list: list type ENT_BLLIST_AREA");
			while( (lar = (LLIST_AREA_DATA *)iterator_nextdata(&block->loops[lp].d.l.list.it)) && !lar->area );
			if(lar) {
				log_stringf("opc_list: area(%ld,%s)", lar->area->uid, lar->area->name);
			} else
				log_stringf("opc_list: area(<END>)");

			variables_set_area(block->info.var,block->loops[lp].var_name,lar?lar->area:NULL);

			if( !lar ) {
				iterator_stop(&block->loops[lp].d.l.list.it);
				skip = TRUE;
				break;
			}
			break;

		case ENT_BLLIST_WILDS:
			log_stringf("opc_list: list type ENT_BLLIST_WILDS");
			while( (lwd = (LLIST_WILDS_DATA *)iterator_nextdata(&block->loops[lp].d.l.list.it)) && !lwd->wilds );
			if(lwd) {
				log_stringf("opc_list: wilds(%ld,%s)", lwd->wilds->uid, lwd->wilds->name);
			} else
				log_stringf("opc_list: wilds(<END>)");

			variables_set_wilds(block->info.var,block->loops[lp].var_name,lwd?lwd->wilds:NULL);

			if( !lwd ) {
				iterator_stop(&block->loops[lp].d.l.list.it);
				skip = TRUE;
				break;
			}
			break;

		case ENT_PLLIST_CONN:
			log_stringf("opc_list: list type ENT_PLLIST_CONN");
			conn = (DESCRIPTOR_DATA *)iterator_nextdata(&block->loops[lp].d.l.list.it);
			if(conn) {
				if(conn->original)
					log_stringf("opc_list: connection(%s,%d) [SWITCHED]", conn->original->name, conn->original->tot_level);
				else
					log_stringf("opc_list: connection(%s,%d)", conn->character->name, conn->character->tot_level);
			} else
				log_stringf("opc_list: connection(<END>)");

			// Set the variable
			variables_set_connection(block->info.var,block->loops[lp].var_name,conn);

			if( !conn ) {
				iterator_stop(&block->loops[lp].d.l.list.it);
				skip = TRUE;
				break;
			}

			break;

		case ENT_PLLIST_MOB:
			log_stringf("opc_list: list type ENT_PLLIST_MOB");
			ch = (CHAR_DATA *)iterator_nextdata(&block->loops[lp].d.l.list.it);
			if(ch) {
				if(!IS_NPC(ch))
					log_stringf("opc_list: player(%s,%ld,%ld)", ch->name, ch->id[0], ch->id[1]);
				else
					log_stringf("opc_list: mobile(%ld,%ld,%ld)", ch->pIndexData->vnum, ch->id[0], ch->id[1]);
			} else
				log_stringf("opc_list: mobile(<END>)");

			// Set the variable
			variables_set_mobile(block->info.var,block->loops[lp].var_name,ch);

			if( !ch ) {
				iterator_stop(&block->loops[lp].d.l.list.it);
				skip = TRUE;
				break;
			}

			break;

		case ENT_PLLIST_OBJ:
			log_stringf("opc_list: list type ENT_PLLIST_OBJ");
			obj = (OBJ_DATA *)iterator_nextdata(&block->loops[lp].d.l.list.it);
			if(obj)
				log_stringf("opc_list: object(%ld,%ld,%ld)", obj->pIndexData->vnum, obj->id[0], obj->id[1]);
			else
				log_stringf("opc_list: object(<END>)");

			// Set the variable
			variables_set_object(block->info.var,block->loops[lp].var_name,obj);

			if( !obj ) {
				iterator_stop(&block->loops[lp].d.l.list.it);
				skip = TRUE;
				break;
			}

			break;

		case ENT_PLLIST_ROOM:
			log_stringf("opc_list: list type ENT_PLLIST_ROOM");
			here = (ROOM_INDEX_DATA *)iterator_nextdata(&block->loops[lp].d.l.list.it);
			if(here) {
				if(here->wilds)
					log_stringf("opc_list: room(%ld,%ld,%ld)", here->wilds->uid, here->x, here->y);
				else if(here->source)
					log_stringf("opc_list: room(%ld,%s,%ld,%ld)", here->vnum, here->name, here->id[0], here->id[1]);
				else
					log_stringf("opc_list: room(%ld,%s)", here->vnum, here->name);
			} else
				log_stringf("opc_list: room(<END>)");

			// Set the variable
			variables_set_room(block->info.var,block->loops[lp].var_name,here);

			if( !here ) {
				iterator_stop(&block->loops[lp].d.l.list.it);
				skip = TRUE;
				break;
			}

			break;

		case ENT_PLLIST_TOK:
			log_stringf("opc_list: list type ENT_PLLIST_TOK");
			tok = (TOKEN_DATA *)iterator_nextdata(&block->loops[lp].d.l.list.it);
			if(tok)
				log_stringf("opc_list: token(%ld,%ld,%ld)", tok->pIndexData->vnum, tok->id[0], tok->id[1]);
			else
				log_stringf("opc_list: token(<END>)");

			// Set the variable
			variables_set_token(block->info.var,block->loops[lp].var_name,tok);

			if( !tok ) {
				iterator_stop(&block->loops[lp].d.l.list.it);
				skip = TRUE;
				break;
			}
			break;

		case ENT_PLLIST_CHURCH:
			log_stringf("opc_list: list type ENT_PLLIST_CHURCH");
			church = (CHURCH_DATA *)iterator_nextdata(&block->loops[lp].d.l.list.it);
			log_stringf("opc_list: church(%s)", church ? church->name : "<END>");

			// Set the variable
			variables_set_church(block->info.var,block->loops[lp].var_name,church);

			if( !church ) {
				iterator_stop(&block->loops[lp].d.l.list.it);
				skip = TRUE;
				break;
			}

			break;
		}

		if(skip) {
			block->loop--;
			return opc_skip_to_label(block,OP_ENDLIST,block->loops[lp].id,TRUE);
		}

		block->cond[block->cur_line->level] = TRUE;
	}

	opc_next_line(block);
	return TRUE;
}


DECL_OPC_FUN(opc_endlist)
{
	if(block->cur_line->level > 0 && !block->cond[block->cur_line->level-1])
		return opc_skip_block(block,block->cur_line->level-1,FALSE);

	return opc_skip_to_label(block,OP_LIST,block->cur_line->label,FALSE);
}

DECL_OPC_FUN(opc_exitlist)
{
	if(block->cur_line->level > 0 && !block->cond[block->cur_line->level-1])
		return opc_skip_block(block,block->cur_line->level-1,FALSE);

	return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);
}

DECL_OPC_FUN(opc_endwhile)
{
	if(block->cur_line->level > 0 && !block->cond[block->cur_line->level-1])
		return opc_skip_block(block,block->cur_line->level-1,FALSE);

	return opc_skip_to_label(block,OP_WHILE,block->cur_line->label,FALSE);
}

DECL_OPC_FUN(opc_exitwhile)
{
	if(block->cur_line->level > 0 && !block->cond[block->cur_line->level-1])
		return opc_skip_block(block,block->cur_line->level-1,FALSE);

	return opc_skip_to_label(block,OP_ENDWHILE,block->cur_line->label,TRUE);
}


DECL_OPC_FUN(opc_mob)
{
	if(block->cur_line->level > 0 && !block->cond[block->cur_line->level-1])
		return opc_skip_block(block,block->cur_line->level-1,FALSE);

	// Verify
	if(block->type != IFC_M) {
		// Log the error
		return FALSE;
	}

	DBG3MSG2("Executing: %d(%s)\n", block->cur_line->param,mob_cmd_table[block->cur_line->param].name);

	if(mob_cmd_table[block->cur_line->param].restricted && script_security < MIN_SCRIPT_SECURITY) {
		char buf[MIL];
		sprintf(buf, "Attempted execution of a restricted mob command '%s' with nulled security.",mob_cmd_table[block->cur_line->param].name);
		bug(buf, 0);
	} else {
		(*mob_cmd_table[block->cur_line->param].func) (&block->info,block->cur_line->rest);
		tail_chain();
	}


	opc_next_line(block);
	return TRUE;
}

DECL_OPC_FUN(opc_obj)
{
	if(block->cur_line->level > 0 && !block->cond[block->cur_line->level-1])
		return opc_skip_block(block,block->cur_line->level-1,FALSE);

	// Verify
	if(block->type != IFC_O) {
		// Log the error
		return FALSE;
	}

	DBG3MSG2("Executing: %d(%s)\n", block->cur_line->param,obj_cmd_table[block->cur_line->param].name);

	if(obj_cmd_table[block->cur_line->param].restricted && script_security < MIN_SCRIPT_SECURITY) {
		char buf[MIL];
		sprintf(buf, "Attempted execution of a restricted obj command '%s' with nulled security.",obj_cmd_table[block->cur_line->param].name);
		bug(buf, 0);
	} else {
		(*obj_cmd_table[block->cur_line->param].func) (&block->info,block->cur_line->rest);
		tail_chain();
	}
	opc_next_line(block);
	return TRUE;
}

DECL_OPC_FUN(opc_room)
{
	if(block->cur_line->level > 0 && !block->cond[block->cur_line->level-1])
		return opc_skip_block(block,block->cur_line->level-1,FALSE);

	// Verify
	if(block->type != IFC_R) {
		// Log the error
		return FALSE;
	}

	DBG3MSG2("Executing: %d(%s)\n", block->cur_line->param,room_cmd_table[block->cur_line->param].name);

	if(room_cmd_table[block->cur_line->param].restricted && script_security < MIN_SCRIPT_SECURITY) {
		char buf[MIL];
		sprintf(buf, "Attempted execution of a restricted room command '%s' with nulled security.",room_cmd_table[block->cur_line->param].name);
		bug(buf, 0);
	} else {
		(*room_cmd_table[block->cur_line->param].func) (&block->info,block->cur_line->rest);
		tail_chain();
	}
	opc_next_line(block);
	return TRUE;
}

DECL_OPC_FUN(opc_token)
{
	if(block->cur_line->level > 0 && !block->cond[block->cur_line->level-1])
		return opc_skip_block(block,block->cur_line->level-1,FALSE);

	// Verify
	if(block->type != IFC_T) {
		// Log the error
		return FALSE;
	}

	DBG3MSG2("Executing: %d(%s)\n", block->cur_line->param,token_cmd_table[block->cur_line->param].name);

	if(token_cmd_table[block->cur_line->param].restricted && script_security < MIN_SCRIPT_SECURITY) {
		char buf[MIL];
		sprintf(buf, "Attempted execution of a restricted token command '%s' with nulled security.",token_cmd_table[block->cur_line->param].name);
		bug(buf, 0);
	} else {
		(*token_cmd_table[block->cur_line->param].func) (&block->info,block->cur_line->rest);
		tail_chain();
	}
	opc_next_line(block);
	return TRUE;
}

DECL_OPC_FUN(opc_tokenother)
{
	if(block->cur_line->level > 0 && !block->cond[block->cur_line->level-1])
		return opc_skip_block(block,block->cur_line->level-1,FALSE);

	// Verify
	if(block->type == IFC_T) {
		// Log the error
		return FALSE;
	}

	DBG3MSG2("Executing: %d(%s)\n", block->cur_line->param,tokenother_cmd_table[block->cur_line->param].name);

	if(tokenother_cmd_table[block->cur_line->param].restricted && script_security < MIN_SCRIPT_SECURITY) {
		char buf[MIL];
		sprintf(buf, "Attempted execution of a restricted tokenother command '%s' with nulled security.",tokenother_cmd_table[block->cur_line->param].name);
		bug(buf, 0);
	} else {
		(*tokenother_cmd_table[block->cur_line->param].func) (&block->info,block->cur_line->rest);
		tail_chain();
	}
	opc_next_line(block);
	return TRUE;
}

bool echo_line(SCRIPT_CB *block)
{
	char buf[MSL];
	DBG3MSG4("Executing: Line=%d, Opcode=%d(%s), Level=%d\n", block->line+1,block->cur_line->opcode,opcode_names[block->cur_line->opcode],block->cur_line->level);
	if(wiznet_script) {
		sprintf(buf,"Executing: Line=%d, Opcode=%d(%s), Level=%d", block->line+1,block->cur_line->opcode,opcode_names[block->cur_line->opcode],block->cur_line->level);
		wiznet(buf,NULL,NULL,WIZ_SCRIPTS,0,0);
	}
	return TRUE;
}

void script_dump(SCRIPT_DATA *script)
{
#ifdef DEBUG_MODULE
	int i;
	DBG3MSG2("vnum = %d, lines = %d\n", script->vnum, script->lines);
	if(script->code) {
		for(i=0; i < script->lines; i++) {
			DBG3MSG4("Line %d: Opcode=%d(%s), Level=%d\n", i+1,script->code[i].opcode,opcode_names[script->code[i].opcode],script->code[i].level);
		}
	}

#endif
}

void script_dump_wiznet(SCRIPT_DATA *script)
{
	int i;
	char buf[MSL];
	sprintf(buf,"vnum = %d, lines = %d", script->vnum, script->lines);
	wiznet(buf,NULL,NULL,WIZ_SCRIPTS,0,0);
	if(script->code) {
		for(i=0; i < script->lines; i++) {
			sprintf(buf,"Line %d: Opcode=%d(%s), Level=%d", i+1,script->code[i].opcode,opcode_names[script->code[i].opcode],script->code[i].level);
			wiznet(buf,NULL,NULL,WIZ_SCRIPTS,0,0);
		}
	}
}

int execute_script(long pvnum, SCRIPT_DATA *script, CHAR_DATA *mob, OBJ_DATA *obj,
	ROOM_INDEX_DATA *room, TOKEN_DATA *token, CHAR_DATA *ch,
	OBJ_DATA *obj1,OBJ_DATA *obj2,CHAR_DATA *vch,CHAR_DATA *vch2,CHAR_DATA *rch,
	char *phrase, char *trigger,
	int number1, int number2, int number3, int number4, int number5)
{
	char buf[MSL];
	SCRIPT_CB block;	// Control block
	int saved_call_depth;	// Call depth copied
	int saved_security;	// Security copied
	bool saved_wiznet;
	long vnum = 0, level;

	DBG2ENTRY7(NUM,pvnum,PTR,script,PTR,mob,PTR,obj,PTR,room,PTR,token,PTR,ch);

	script_destructed = FALSE;

	if (!script || !script->code) {
		bug("PROGs: No script to execute for vnum %d.", pvnum);
		return PRET_NOSCRIPT;
	}

	if ((mob && obj) || (mob && room) || (obj && room)) {
		bug("PROGs: program_flow received multiple prog types for vnum.", pvnum);
		return PRET_BADTYPE;
	}

	// Silently return.  Disabled scripts should just pretend they don't exist.
	if (!script_force_execute && IS_SET(script->flags,SCRIPT_DISABLED))
		return PRET_NOSCRIPT;

	// System scripts require system level security, only set at specific times!
	if(IS_SET(script->flags, SCRIPT_SYSTEM) && script_security < SYSTEM_SCRIPT_SECURITY)
		return PRET_NOSCRIPT;


	memset(&block,0,sizeof(block));
	block.info.block = &block;

	saved_wiznet = wiznet_script;
	wiznet_script = (bool)(int)IS_SET(script->flags,SCRIPT_WIZNET);

	if (mob) {
		mob->progs->lastreturn = PRET_EXECUTED;
		vnum = mob->pIndexData->vnum;
		block.type = IFC_M;
		block.info.var = &mob->progs->vars;
		block.info.targ = &mob->progs->target;

	} else if (obj) {
		obj->progs->lastreturn = PRET_EXECUTED;
		vnum = obj->pIndexData->vnum;
		block.type = IFC_O;
		block.info.var = &obj->progs->vars;
		block.info.targ = &obj->progs->target;
	} else if (room) {
		room->progs->lastreturn = PRET_EXECUTED;
		vnum = room->vnum;
		block.type = IFC_R;
		block.info.var = &room->progs->vars;
		block.info.targ = &room->progs->target;
	} else if (token) {
		token->progs->lastreturn = PRET_EXECUTED;
		vnum = token->pIndexData->vnum;
		block.type = IFC_T;
		block.info.var = &token->progs->vars;
		block.info.targ = &token->progs->target;
	} else {
		// Log error
		return PRET_BADTYPE;
	}

	if(phrase) strncpy(block.info.phrase,phrase,MSL);
	if(trigger) strncpy(block.info.trigger,trigger,MSL);

	if(wiznet_script) {
		sprintf(buf,"{BScript{C({W%d{C){D: {B%s{C({W%d{C){D, {B%s{C({W%d{C){D, {B%s{C({W%d{C){D, {B%s{C({W%d{C){D, {B%s{C({W%d{C){x",
			script ? script->vnum : -1,
			mob ? HANDLE(mob) : "(mob)",
			mob ? (int)VNUM(mob) : -1,
			obj ? obj->short_descr : "(obj)",
			obj ? (int)VNUM(obj) : -1,
			room ? room->name : "(room)",
			room ? (int)room->vnum : -1,
			token ? token->name : "(token)",
			token ? (int)VNUM(token) : -1,
			ch ? HANDLE(ch) : "(ch)",
			ch ? (int)VNUM(ch) : -1);
		wiznet(buf, NULL, NULL,WIZ_SCRIPTS,0,0);
		script_dump_wiznet(script);
	}

	// Save script parameters
	block.info.mob = mob;
	block.info.obj = obj;
	block.info.room = room;
	block.info.token = token;
	block.info.ch = ch;
	block.info.obj1 = obj1;
	block.info.obj2 = obj2;
	block.info.vch = vch;
	block.info.vch2 = vch2;
	block.info.rch = rch;
	block.info.registers[0] = number1;
	block.info.registers[1] = number2;
	block.info.registers[2] = number3;
	block.info.registers[3] = number4;
	block.info.registers[4] = number5;
	block.script = script;

	saved_security = script_security;

	// Non-system scripts modify the security
	if(!IS_SET(script->flags, SCRIPT_SYSTEM)) {
		if(script_security == INIT_SCRIPT_SECURITY ||
			(IS_SET(script->flags,SCRIPT_SECURED) && (script->security >= script_security)) ||
			script->security < script_security)
			script_security = script->security;
	}

	// Call depth code
	saved_call_depth = script_call_depth;
	if(!script_call_depth) {
		if(!script->depth)
			script_call_depth = MAX_CALL_LEVEL;
		else
			script_call_depth = script->depth;
	}

	// Init stack
	for (level = 0; level < MAX_NESTED_LEVEL; level++) {
		block.state[level] = IN_BLOCK;
		block.cond[level]  = TRUE;
	}
	block.level = 0;
	block.line = 0;
	block.loop = 0;
	block.ret_val = PRET_EXECUTED;
	block.cur_line = &block.script->code[block.line];
	block.next = script_call_stack;
	script_call_stack = &block;

	DBG3MSG0("Starting script...\n");
	if(wiznet_script) wiznet("Starting script...",NULL,NULL,WIZ_SCRIPTS,0,0);

	// Run script
	// Until the number of lines of the script has been reached or
	//	An opcode function tells it to quit.
	while(block.line < block.script->lines &&
		block.cur_line->opcode < OP_LASTCODE &&
		echo_line(&block) &&
		(*opcode_table[block.cur_line->opcode])(&block) &&
		!IS_SET(block.flags,SCRIPTEXEC_HALT)) {

		if(block.line < block.script->lines)
			block.cur_line = &block.script->code[block.line];
	}

	if(IS_SET(block.flags,SCRIPTEXEC_HALT)) script_destructed = TRUE;

	DBG3MSG0("Completed script...\n");
	if(wiznet_script) wiznet((script_destructed?"Script halted due to entity destruction...":"Completed script..."),NULL,NULL,WIZ_SCRIPTS,0,0);

	wiznet_script = saved_wiznet;

	DBG2EXITVALUE1(NUM,block.ret_val);
	script_security = saved_security;
	script_call_depth = saved_call_depth; // Restore call depth
	script_call_stack = script_call_stack->next; // Back up call stack
	return block.ret_val;
}

/*
 * Get a random PC in the room (for $r parameter)
 */
CHAR_DATA *get_random_char(CHAR_DATA *mob, OBJ_DATA *obj, ROOM_INDEX_DATA *room, TOKEN_DATA *token)
{
    CHAR_DATA *vch, *victim = NULL;
    int now = 0, highest = 0;

    if ((mob && obj) || (mob && room) || (obj && room)) {
	bug("get_random_char received multiple prog types",0);
	return NULL;
    }

    if (mob)
	vch = mob->in_room->people;
    else if (obj)
	vch = obj_room(obj)->people;
    else if (token)
	vch = token_room(token)->people;
    else if (!room) {
	    bug("get_random_char: no room, object, or mob!", 0);
	    return NULL;
    } else
	vch = room->people;

    for (; vch; vch = vch->next_in_room) {
        if (mob && mob != vch && !IS_NPC(vch) && can_see(mob, vch) && (now = number_percent()) > highest) {
            victim = vch;
            highest = now;
        } else if ((now = number_percent()) > highest) {
	    victim = vch;
	    highest = now;
	}
    }

    return victim;
}


/*
 * How many other players / mobs are there in the room
 * iFlag: 0: all, 1: players, 2: mobiles 3: mobs w/ same vnum 4: same group
 */
int count_people_room(CHAR_DATA *mob, OBJ_DATA *obj, ROOM_INDEX_DATA *room, TOKEN_DATA *token, int iFlag)
{
    CHAR_DATA *vch;
    int count;

    if ((mob && obj) || (mob && room) || (obj && room)) {
	bug("count_people_room received multiple prog types",0);
	return 0;
    }

    if (mob && mob->in_room)
	vch = mob->in_room->people;
    else if (obj)
	vch = obj_room(obj)->people;
    else if (token)
	vch = token_room(token)->people;
    else if (room)
        vch = room->people;
    else {
	bug("count_people_room had null room obj and mob.",0);
	return 0;
    }

    for (count = 0; vch; vch = vch->next_in_room) {
	if (mob) {
	    if (mob != vch
	    && (iFlag == 0
	    || (iFlag == 1 && !IS_NPC(vch))
	    || (iFlag == 2 && IS_NPC(vch))
	    || (iFlag == 3 && IS_NPC(mob) && IS_NPC(vch)
	    && mob->pIndexData->vnum == vch->pIndexData->vnum)
	    || (iFlag == 4 && is_same_group(mob, vch)))
	    && can_see(mob, vch))
	  	++count;

	} else if (obj || room) {
	    if (iFlag == 0
	    || (iFlag == 1 && !IS_NPC(vch))
	    || (iFlag == 2 && IS_NPC(vch)))
		++count;
	}
    }

    return (count);
}


/*
 * Get the order of a mob in the room. Useful when several mobs in
 * a room have the same trigger and you want only the first of them
 * to act
 */
//@@@NIB Combined the if statements; once it knew it was a mob,
//		it didn't need to check again.
int get_order(CHAR_DATA *ch, OBJ_DATA *obj)
{
    CHAR_DATA *vch;
    OBJ_DATA *vobj;
    int i;

    if (ch && obj) {
	bug("get_order received multiple prog types",0);
	return 0;
    }

    if (ch) {
	if(!IS_NPC(ch)) return 0;
	vch = ch->in_room->people;

	for (i = 0; vch; vch = vch->next_in_room) {
	    if (vch == ch) return i;

	    if (IS_NPC(vch) && vch->pIndexData->vnum == ch->pIndexData->vnum)
		++i;
	}

    } else {
	if (obj->in_room)
	    vobj = obj->in_room->contents;
	else if (obj->carried_by->in_room->contents)
	    vobj = obj->carried_by->in_room->contents;
	else
	    vobj = NULL;

	for (i = 0; vobj; vobj = vobj->next_content) {
	    if (vobj == obj) return i;

	    if (vobj->pIndexData->vnum == obj->pIndexData->vnum)
		++i;
	}
    }

    return 0;
}

CHAR_DATA *script_get_char_list(CHAR_DATA *mobs, CHAR_DATA *viewer, bool player, int vnum, char *name)
{
	int nth = 1, i = 0;
	char buf[MSL];
	CHAR_DATA *ch;
	if(!mobs) return NULL;

	if(name) {
		nth = number_argument(name,buf);

		if(!player && is_number(buf)) {
			vnum = atol(buf);
			name = NULL;
		} else {
			vnum = 0;
			name = buf;
		}
	}

	if(player) {
		for(ch = mobs; ch; ch = ch->next_in_room)
			if(!IS_NPC(ch) && is_name(name,ch->name) && (!viewer || can_see(viewer,ch)))
				if( ++i == nth ) return ch;

	} else if(vnum > 0) {
		for(ch = mobs; ch; ch = ch->next_in_room)
			if(IS_NPC(ch) && ch->pIndexData->vnum == vnum && (!viewer || can_see(viewer,ch)))
				if( ++i == nth ) return ch;

	} else if(name) {
		for(ch = mobs; ch; ch = ch->next_in_room)
			if(is_name(name,ch->name) && (!viewer || can_see(viewer,ch)))
				if( ++i == nth ) return ch;

	}
	return NULL;
}

OBJ_DATA *script_get_obj_list(OBJ_DATA *objs, CHAR_DATA *viewer, int worn, int vnum, char *name)
{
	int nth = 1, i = 0;
	char buf[MSL];
	OBJ_DATA *obj;
	if(!objs) return NULL;

	if(name) {
		nth = number_argument(name,buf);

		if(is_number(buf)) {
			vnum = atol(buf);
			name = NULL;
		} else {
			vnum = 0;
			name = buf;
		}
	}

	switch(worn) {
	default:
		if(vnum > 0) {
			for(obj = objs; obj; obj = obj->next_content)
				if(obj->pIndexData->vnum == vnum && (!viewer || can_see_obj(viewer,obj)))
					if( ++i == nth ) return obj;
		} else if(name) {
			for(obj = objs; obj; obj = obj->next_content)
				if(is_name(name,obj->name) && (!viewer || can_see_obj(viewer,obj)))
					if( ++i == nth ) return obj;
		}
		break;
	case 1:
		if(vnum > 0) {
			for(obj = objs; obj; obj = obj->next_content)
				if(obj->wear_loc != WEAR_NONE && obj->pIndexData->vnum == vnum && (!viewer || can_see_obj(viewer,obj)))
					if( ++i == nth ) return obj;
		} else if(name) {
			for(obj = objs; obj; obj = obj->next_content)
				if(obj->wear_loc != WEAR_NONE && is_name(name,obj->name) && (!viewer || can_see_obj(viewer,obj)))
					if( ++i == nth ) return obj;
		}
		break;
	case 2:
		if(vnum > 0) {
			for(obj = objs; obj; obj = obj->next_content)
				if(obj->wear_loc == WEAR_NONE && obj->pIndexData->vnum == vnum && (!viewer || can_see_obj(viewer,obj)))
					if( ++i == nth ) return obj;
		} else if(name) {
			for(obj = objs; obj; obj = obj->next_content)
				if(obj->wear_loc == WEAR_NONE && is_name(name,obj->name) && (!viewer || can_see_obj(viewer,obj)))
					if( ++i == nth ) return obj;
		}
		break;
	}
	return NULL;
}


TOKEN_DATA *token_find_match(SCRIPT_VARINFO *info, TOKEN_DATA *tokens,char *argument)
{
	char *rest;
	int i, nth = 1, vnum = 0, matches;
	int values[MAX_TOKEN_VALUES];
	bool match[MAX_TOKEN_VALUES];
	char buf[MSL];
	SCRIPT_PARAM arg;

	if(!(rest = expand_argument(info,argument,&arg)))
		return NULL;

	if(arg.type == ENT_NUMBER)
		vnum = arg.d.num;
	else if(arg.type == ENT_STRING) {
		nth = number_argument(arg.d.str,buf);
		if(nth < 1 || !is_number(buf))
			return NULL;
		vnum = atoi(buf);
	}


	if(vnum < 1) return NULL;

	for(i=0;*rest && i < MAX_TOKEN_VALUES; i++) {
		argument = rest;
		if(!(rest = expand_argument(info,argument,&arg)))
			return NULL;

		if(arg.type == ENT_NUMBER)
			values[i] = arg.d.num;
		else if(arg.type == ENT_STRING && is_number(arg.d.str))
			values[i] = atoi(arg.d.str);
		else {
			match[i] = FALSE;
			continue;
		}
		match[i] = TRUE;
	}

	for(;i < MAX_TOKEN_VALUES; i++) match[i] = FALSE;

	for(;tokens;tokens = tokens->next) {
		if(tokens->pIndexData->vnum == vnum) {
			for(matches = 0, i = 0; i < MAX_TOKEN_VALUES; i++)
				if( !match[i] || tokens->value[i] == values[i] )
					matches++;

			if( matches == MAX_TOKEN_VALUES && !--nth )
				break;
		}
	}

	return tokens;
}

/*
 * Check if ch has a given item or item type
 * vnum: item vnum or -1
 * item_type: item type or -1
 * fWear: TRUE: item must be worn, FALSE: don't care
 */
bool has_item(CHAR_DATA *ch, long vnum, sh_int item_type, bool fWear)
{
    OBJ_DATA *obj;
    for (obj = ch->carrying; obj; obj = obj->next_content)
	if ((vnum < 0 || obj->pIndexData->vnum == vnum)
	&&   (item_type < 0 || obj->pIndexData->item_type == item_type)
	&&   (!fWear || obj->wear_loc != WEAR_NONE))
	    return TRUE;
    return FALSE;
}


/*
 * Check if there's a mob with given vnum in the room
 */
CHAR_DATA *get_mob_vnum_room(CHAR_DATA *ch, OBJ_DATA *obj, ROOM_INDEX_DATA *room, TOKEN_DATA *token, long vnum)
{
    CHAR_DATA *mob;

    if ((ch && obj) || (ch && room) || (obj && room) ||
    	(ch && token) || (obj && token) || (room && token)) {
	bug("get_mob_vnum_room received multiple prog types",0);
	return NULL;
    }

    if (ch)
	mob = ch->in_room->people;
    else if (obj)
	mob = obj_room(obj)->people;
    else if (token)
	mob = token_room(token)->people;
    else mob = room->people;

    for (; mob; mob = mob->next_in_room)
	if (IS_NPC(mob) && mob->pIndexData->vnum == vnum)
	    return mob;
    return NULL;
}


/*
 * Check if there's an object with given vnum in the room
 */
OBJ_DATA *get_obj_vnum_room(CHAR_DATA *ch, OBJ_DATA *obj, ROOM_INDEX_DATA *room, TOKEN_DATA *token, long vnum)
{
    OBJ_DATA *vobj;

    if ((ch && obj) || (ch && room) || (obj && room) ||
    	(ch && token) || (obj && token) || (room && token)) {
	bug("get_obj_vnum_room received multiple prog types",0);
	return NULL;
    }

    if (ch)
	vobj = ch->in_room->contents;
    else if (obj)
	vobj = obj_room(obj)->contents;
    else if (token)
	vobj = token_room(token)->contents;
    else
	vobj = room->contents;

    for (; vobj; vobj = vobj->next_content)
	if (vobj->pIndexData->vnum == vnum)
	    return vobj;
    return NULL;
}

void get_level_damage(int level, int *num, int *type, bool fRemort, bool fTwo)
{
	*num = (level + 20) / 10;
	*type = (level + 20) / 4;

	if(fTwo) *type = (*type * 7)/5 - 1;
	if(fRemort) {
		*num += 2;
		*type += 2;
	}

	*num = UMAX(1, *num);
	*type = UMAX(8, *type);
}

void do_mob_transfer(CHAR_DATA *ch,ROOM_INDEX_DATA *room,bool quiet)
{
	if (ch->fighting)
		stop_fighting(ch, TRUE);

	move_cart(ch,room,!quiet);

	char_from_room(ch);
	if(room->wilds)
		char_to_vroom(ch, room->wilds, room->x, room->y);
	else
		char_to_room(ch, room);

	if(!quiet) do_look(ch, "auto");
}


bool has_trigger(LLIST **bank, int trigger)
{
	int slot;
	PROG_LIST *trig;
	ITERATOR it;

//	DBG2ENTRY2(PTR,bank,NUM,trigger);

//	DBG3MSG3("trigger = %d, name = '%s', slot = %d\n",trigger, trigger_table[trigger].name,trigger_slots[trigger]);

	slot = trigger_slots[trigger];

	if(bank) {
		iterator_start(&it, bank[slot]);
		while((trig = (PROG_LIST *)iterator_nextdata(&it))) {
			if (is_trigger_type(trig->trig_type,trigger))
				break;
		}
		iterator_stop(&it);

		if(trig)
			return TRUE;
	}

//	DBG2EXITVALUE2(FALSE);
	return FALSE;
}

int trigger_index(char *name, int type)
{
	register int i;

	// Check Cannonical names first, so aliases don't override
	for (i = 0; trigger_table[i].name; i++) {
		if (!str_cmp(trigger_table[i].name, name))
			switch (type) {
			case PRG_MPROG: if (trigger_table[i].mob) return i;
			case PRG_OPROG: if (trigger_table[i].obj) return i;
			case PRG_RPROG: if (trigger_table[i].room) return i;
			case PRG_TPROG: if (trigger_table[i].token) return i;
			}
	}

	// Check Aliases
	for (i = 0; trigger_table[i].name; i++) {
		if (trigger_table[i].alias && is_exact_name(trigger_table[i].alias, name))
			switch (type) {
			case PRG_MPROG: if (trigger_table[i].mob) return i;
			case PRG_OPROG: if (trigger_table[i].obj) return i;
			case PRG_RPROG: if (trigger_table[i].room) return i;
			case PRG_TPROG: if (trigger_table[i].token) return i;
			}
	}

	return -1;
}

bool is_trigger_type(int tindex, int type)
{
	if(tindex < 0) return FALSE;

//	log_stringf("is_trigger_type: %d, %s, %d", tindex, trigger_table[tindex].name, type);

	return (tindex == type);
}

bool mp_same_group(CHAR_DATA *ch,CHAR_DATA *vch,CHAR_DATA *to)
{
	return (ch != vch && ch != to && is_same_group(vch,to));
}

bool rop_same_group(CHAR_DATA *ch,CHAR_DATA *vch,CHAR_DATA *to)
{
	// vch is NULL from these
	return (is_same_group(ch,to));
}


ROOM_INDEX_DATA *get_exit_dest(ROOM_INDEX_DATA *room, char *argument)
{
    EXIT_DATA *ex;

    if (!room || !argument[0])
	return NULL;

    if (!str_cmp(argument, "north"))		ex = room->exit[DIR_NORTH];
    else if (!str_cmp(argument, "south"))	ex = room->exit[DIR_SOUTH];
    else if (!str_cmp(argument, "west"))	ex = room->exit[DIR_WEST];
    else if (!str_cmp(argument, "east"))	ex = room->exit[DIR_EAST];
    else if (!str_cmp(argument, "up"))		ex = room->exit[DIR_UP];
    else if (!str_cmp(argument, "down"))	ex = room->exit[DIR_DOWN];
    else if (!str_cmp(argument, "northeast"))	ex = room->exit[DIR_NORTHEAST];
    else if (!str_cmp(argument, "northwest"))	ex = room->exit[DIR_NORTHWEST];
    else if (!str_cmp(argument, "southeast"))	ex = room->exit[DIR_SOUTHEAST];
    else if (!str_cmp(argument, "southwest"))	ex = room->exit[DIR_SOUTHWEST];
    else return NULL;

    return (ex && ex->u1.to_room) ? ex->u1.to_room : NULL;
}


bool script_change_exit(ROOM_INDEX_DATA *pRoom, ROOM_INDEX_DATA *pToRoom, int door)
{
	EXIT_DATA *pExit;

	if (!pToRoom) {
		sh_int rev;

		if (!pRoom->exit[door]) {
			bug("script_change_exit: Couldn't delete exit. %d", pRoom->vnum);
			return FALSE;
		}

		// Remove ToRoom Exit.
		rev = rev_dir[door];
		pToRoom = pRoom->exit[door]->u1.to_room;

		if (pToRoom->exit[rev]) {
			free_exit(pToRoom->exit[rev]);
			pToRoom->exit[rev] = NULL;
		}

		// Remove this exit.
		free_exit(pRoom->exit[door]);
		pRoom->exit[door] = NULL;

		return TRUE;
	}

	// Rules...
	// EITHER -> ENVIRON ... OK
	// STATIC -> CLONE ..... NOT OK
	// CLONE -> STATIC ..... WILL CLONE
	// CLONE -> CLONE ...... OK

	if(pToRoom != &room_pointer_environment) {
		if(room_is_clone(pRoom)) {
			if(!room_is_clone(pToRoom) && !(pToRoom = create_virtual_room(pToRoom,false)))
				return false;
		} else if(room_is_clone(pToRoom)) {
			bug("script_change_exit: A link cannot be made from a static room to a clone room.\n\r",0);
			return FALSE;
		}
	}

	if(room_is_clone(pRoom)) {
		if(pToRoom != &room_pointer_environment && !room_is_clone(pToRoom)) {
			// Should this be illegal or should it be made into a cloned room?
			bug("script_change_exit: A link cannot be made between static and clone room.\n\r",0);
			return FALSE;
		}
	} else {
		if(pToRoom != &room_pointer_environment && room_is_clone(pToRoom)) {
			bug("script_change_exit: A link cannot be made between static and clone room.\n\r",0);
			return FALSE;
		}
	}

	if(pToRoom != &room_pointer_environment) {
		if (pToRoom->exit[rev_dir[door]]) {
			bug("script_change_exit: Reverse-side exit to room already exists.", 0);
			return FALSE;
		}
	}

	if (!pRoom->exit[door]) pRoom->exit[door] = new_exit();

	pRoom->exit[door]->u1.to_room = pToRoom;
	pRoom->exit[door]->orig_door = door;
	pRoom->exit[door]->from_room = pRoom;

	if(pToRoom != &room_pointer_environment) {
		door = rev_dir[door];
		pExit = new_exit();
		pExit->u1.to_room = pRoom;
		pExit->orig_door = door;
		pExit->from_room = pToRoom;
		pToRoom->exit[door] = pExit;
	} else {
		// Mark it as an environment
		SET_BIT(pRoom->exit[door]->exit_info, EX_ENVIRONMENT);
	}

	return TRUE;
}




char *trigger_name(int type)
{
	if(type >= 0 && type < trigger_table_size && trigger_table[type].name)
		return trigger_table[type].name;

	return "INVALID";
}

char *trigger_phrase(int type, char *phrase)
{
	int sn;
	if(type >= 0 && type < trigger_table_size && trigger_table[type].name) {
		if(type == TRIG_SPELLCAST) {
			sn = atoi(phrase);
			if(sn < 0) return "reserved";
			return skill_table[sn].name;
		}
	}

	return phrase;
}


// Common entry point for all the queued commands!
void script_interpret(SCRIPT_VARINFO *info, char *command)
{
	char *start, buf[MSL];

	start = one_argument(command,buf);

	if(info->mob) {
		if(!str_cmp(buf,"mob")) mob_interpret(info,command);
		else if(!str_cmp(buf,"token")) tokenother_interpret(info,command);
		else {
			expand_string(info,command,buf);
			interpret(info->mob,buf);
		}
		return;
	}

	if(info->obj) {
		if(!str_cmp(buf,"obj")) obj_interpret(info,command);
		else if(!str_cmp(buf,"token")) tokenother_interpret(info,command);
		return;
	}

	if(info->room) {
		if(!str_cmp(buf,"room")) room_interpret(info,command);
		else if(!str_cmp(buf,"token")) tokenother_interpret(info,command);
		return;
	}

	if(info->token) {
		if(!str_cmp(buf,"token")) token_interpret(info,command);
		return;
	}

	// Complain
}

typedef bool (*MATCH_STRING)(char *a, char *b);
typedef bool (*MATCH_NUMBER)(int a, int b);
typedef bool (*MATCH_RANGE)(int a, int b, int c);


// MATCH_STRING
static bool __attribute__ ((unused)) match_substr(register char *a, register char *b)
{
	register char *a2;
	register char *b2;

	if(!a || !b) return FALSE;

	if(!*b) return TRUE;

	if(!*a) return FALSE;

	while(*a) {
		for(a2 = a, b2 = b;(*a2 && *b2 && LOWER(*a2) == LOWER(*b2)); ++a2, ++b2);

		if(!*b2) return TRUE;

		a++;
	}

	return FALSE;
}

// MATCH_STRING
static bool __attribute__ ((unused)) match_exact_name(char *a, char *b)
{
	return !str_cmp(a, b);
}

// MATCH_STRING
static bool __attribute__ ((unused)) match_name(char *a, char *b)
{
	return is_name(b, a);
}

// MATCH_NUMBER
static bool __attribute__ ((unused)) match_random(int a, int b)
{
	return number_range(0, b-1) < a;
}

// MATCH_NUMBER
static bool __attribute__ ((unused)) match_percent(int a, int b)
{
	return number_percent() < a;
}

// MATCH_NUMBER
static bool __attribute__ ((unused)) match_equal(int a, int b)
{
	return a == b;
}

// MATCH_NUMBER
static bool __attribute__ ((unused)) match_lt(int a, int b)
{
	return b < a;
}

// MATCH_NUMBER
static bool __attribute__ ((unused)) match_gt(int a, int b)
{
	return b > a;
}

// MATCH_NUMBER
static bool __attribute__ ((unused)) match_not_equal(int a, int b)
{
	return a != b;
}

// MATCH_NUMBER
static bool __attribute__ ((unused)) match_lte(int a, int b)
{
	return b <= a;
}

// MATCH_NUMBER
static bool __attribute__ ((unused)) match_gte(int a, int b)
{
	return b >= a;
}

// STRING TRIGGER
int test_string_trigger(char *string, MATCH_STRING match, int type,
			CHAR_DATA *mob, OBJ_DATA *obj, ROOM_INDEX_DATA *room,
			CHAR_DATA *enactor, CHAR_DATA *victim, CHAR_DATA *victim2,
			OBJ_DATA *obj1, OBJ_DATA *obj2)
{
	ITERATOR tit;	// Token iterator
	ITERATOR pit;	// Prog iterator
	PROG_LIST *prg;
	TOKEN_DATA *token;
	unsigned long uid[2];
	int slot;
	int ret_val = PRET_NOSCRIPT, ret;

	if ((mob && obj) || (mob && room) || (obj && room)) {
		bug("test_string_trigger: Multiple program types in trigger %d.", type);
		PRETURN;
	}

	slot = trigger_slots[type];


	if (mob) {
		script_mobile_addref(mob);

		// Save the UID
		uid[0] = mob->id[0];
		uid[1] = mob->id[1];

		// Check for tokens FIRST
		iterator_start(&tit, mob->ltokens);
		// Loop Level 1
		while((token = (TOKEN_DATA *)iterator_nextdata(&tit))) {
			if( token->pIndexData->progs ) {
				script_token_addref(token);
				script_destructed = FALSE;
				iterator_start(&pit, token->pIndexData->progs[slot]);
				// Loop Level 2
				while((prg = (PROG_LIST *)iterator_nextdata(&pit)) && !script_destructed) {
					if (is_trigger_type(prg->trig_type,type)) {
						if ((*match)(prg->trig_phrase, string)) {
							ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, enactor, obj1, obj2, victim, victim2,NULL,string,prg->trig_phrase,0,0,0,0,0);
							if( ret != PRET_NOSCRIPT) {
								iterator_stop(&tit);
								iterator_stop(&pit);
								script_token_remref(token);
								script_mobile_remref(mob);
								return ret;
							}
						}
					}
				}
				iterator_stop(&pit);
				script_token_remref(token);
				BREAKPRET;
			}
		}
		iterator_stop(&tit);

		if(ret_val == PRET_NOSCRIPT && !script_destructed && IS_VALID(mob) && mob->id[0] == uid[0] && mob->id[1] == uid[1] && IS_NPC(mob) && mob->pIndexData->progs) {
			iterator_start(&pit, mob->pIndexData->progs[slot]);
			// Loop Level 1:
			while((prg = (PROG_LIST *)iterator_nextdata(&pit)) && !script_destructed) {
				if (is_trigger_type(prg->trig_type,type)) {
					if ((*match)(prg->trig_phrase, string)) {
						ret = execute_script(prg->vnum, prg->script, mob, NULL, NULL, NULL, enactor, obj1, obj2, victim, victim2,NULL,string,prg->trig_phrase,0,0,0,0,0);
						if( ret != PRET_NOSCRIPT) {
							iterator_stop(&pit);
							script_mobile_remref(mob);
							return ret;
						}

					}
				}
				BREAKPRET;
			}
			iterator_stop(&pit);
		}
		script_mobile_remref(mob);

	} else if (obj && obj_room(obj) ) {
		// Save the UID
		uid[0] = obj->id[0];
		uid[1] = obj->id[1];
		script_object_addref(obj);

		// Check for tokens FIRST
		iterator_start(&tit, obj->ltokens);
		while((token = (TOKEN_DATA *)iterator_nextdata(&tit))) {
			if( token->pIndexData->progs ) {
				script_token_addref(token);
				script_destructed = FALSE;
				iterator_start(&pit, token->pIndexData->progs[slot]);
				while((prg = (PROG_LIST *)iterator_nextdata(&pit)) && !script_destructed) {
					if (is_trigger_type(prg->trig_type,type)) {
						if ((*match)(prg->trig_phrase, string)) {
							ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, enactor, obj1, obj2, victim, victim2,NULL,string,prg->trig_phrase,0,0,0,0,0);
							if( ret != PRET_NOSCRIPT) {
								iterator_stop(&tit);
								iterator_stop(&pit);

								script_token_remref(token);
								script_object_remref(obj);
								return ret;
							}

						}
					}
				}
				iterator_stop(&pit);

				script_token_remref(token);
				BREAKPRET;
			}
		}
		iterator_stop(&tit);

		if(ret_val == PRET_NOSCRIPT && !script_destructed && IS_VALID(obj) && obj->id[0] == uid[0] && obj->id[1] == uid[1] && obj->pIndexData->progs) {
			script_destructed = FALSE;
			iterator_start(&pit, obj->pIndexData->progs[slot]);
			while((prg = (PROG_LIST *)iterator_nextdata(&pit)) && !script_destructed) {
				if (is_trigger_type(prg->trig_type,type)) {
					if ((*match)(prg->trig_phrase, string)) {
						ret = execute_script(prg->vnum, prg->script, NULL, obj, NULL, NULL, enactor, obj1, obj2, victim, victim2,NULL,string,prg->trig_phrase,0,0,0,0,0);
						if( ret != PRET_NOSCRIPT) {
							iterator_stop(&pit);
							script_object_remref(obj);
							return ret;
						}

					}
				}
				BREAKPRET;
			}
			iterator_stop(&pit);
		}

		script_object_remref(obj);

	} else if (room) {
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
					if (is_trigger_type(prg->trig_type,type)) {
						if ((*match)(prg->trig_phrase, string)) {
							ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, enactor, obj1, obj2, victim, victim2,NULL,string,prg->trig_phrase,0,0,0,0,0);
							if( ret != PRET_NOSCRIPT) {
								iterator_stop(&tit);
								iterator_stop(&pit);

								script_token_remref(token);
								script_room_remref(room);
								return ret;
							}

						}
					}
				}
				iterator_stop(&pit);
				script_token_remref(token);

				BREAKPRET;
			}
		}
		iterator_stop(&tit);

		if(ret_val == PRET_NOSCRIPT && source->progs->progs) {
			script_destructed = FALSE;
			iterator_start(&pit, source->progs->progs[slot]);
			while((prg = (PROG_LIST *)iterator_nextdata(&pit)) && !script_destructed) {
				if (is_trigger_type(prg->trig_type,type)) {
					if ((*match)(prg->trig_phrase, string)) {
						ret = execute_script(prg->vnum, prg->script, NULL, NULL, room, NULL, enactor, obj1, obj2, victim, victim2,NULL,string,prg->trig_phrase,0,0,0,0,0);
						if( ret != PRET_NOSCRIPT) {
							iterator_stop(&pit);
							script_room_remref(room);
							return ret;
						}
					}
				}
				BREAKPRET;
			}
			iterator_stop(&pit);

		}
		script_room_remref(room);

	} else
		bug("test_string_trigger: no program type for trigger %d.", type);

	PRETURN;
}


/*
 * A general purpose string trigger. Matches argument to a string trigger
 * phrase.
 */
int p_act_trigger(char *argument, CHAR_DATA *mob, OBJ_DATA *obj, ROOM_INDEX_DATA *room,
	CHAR_DATA *ch, CHAR_DATA *victim, CHAR_DATA *victim2, OBJ_DATA *obj1, OBJ_DATA *obj2, int type)
{
	return test_string_trigger(argument, match_substr, type, mob, obj, room, ch, victim, victim2, obj1, obj1);
}

// Similar to p_act_trigger, except it does EXACT match
int p_exact_trigger(char *argument, CHAR_DATA *mob, OBJ_DATA *obj, ROOM_INDEX_DATA *room,
	CHAR_DATA *ch, CHAR_DATA *victim, CHAR_DATA *victim2, OBJ_DATA *obj1, OBJ_DATA *obj2, int type)
{
	return test_string_trigger(argument, match_exact_name, type, mob, obj, room, ch, victim, victim2, obj1, obj1);
}

// Similar to p_act_trigger, except it uses is_name
int p_name_trigger(char *argument, CHAR_DATA *mob, OBJ_DATA *obj, ROOM_INDEX_DATA *room,
	CHAR_DATA *ch, CHAR_DATA *victim, CHAR_DATA *victim2, OBJ_DATA *obj1, OBJ_DATA *obj2, int type)
{
	return test_string_trigger(argument, match_name, type, mob, obj, room, ch, victim, victim2, obj1, obj1);
}




int test_number_trigger(int number, MATCH_NUMBER match, int type,
			CHAR_DATA *mob, OBJ_DATA *obj, ROOM_INDEX_DATA *room, TOKEN_DATA *token,
			CHAR_DATA *enactor, CHAR_DATA *victim, CHAR_DATA *victim2,
			OBJ_DATA *obj1, OBJ_DATA *obj2,
			char *phrase)
{
	//char buf[MIL];
	ITERATOR tit;	// Token iterator
	ITERATOR pit;	// Prog iterator
	PROG_LIST *prg;
	unsigned long uid[2];
	int slot;
	int ret_val = PRET_NOSCRIPT, ret;

	if ((mob && obj) || (mob && room) || (mob && token) || (obj && room) || (obj && token) || (room && token)) {
		bug("test_number_trigger: Multiple program types in trigger %d.", type);
		PRETURN;
	}

	slot = trigger_slots[type];


	if (mob) {
		script_mobile_addref(mob);

		// Save the UID
		uid[0] = mob->id[0];
		uid[1] = mob->id[1];

		// Check for tokens FIRST
		iterator_start(&tit, mob->ltokens);
		// Loop Level 1
		while((token = (TOKEN_DATA *)iterator_nextdata(&tit))) {
			//sprintf(buf, "test_number_trigger: trigger '%ld', token %ld '%s'",  type, token->pIndexData->vnum, token->name);
			//wiznet(buf, NULL, NULL, WIZ_TESTING, 0, 0);

			if( token->pIndexData->progs ) {
				script_token_addref(token);
				script_destructed = FALSE;
				iterator_start(&pit, token->pIndexData->progs[slot]);
				// Loop Level 2
				while((prg = (PROG_LIST *)iterator_nextdata(&pit)) && !script_destructed) {
					if (is_trigger_type(prg->trig_type,type)) {
						if ((*match)(prg->trig_number, number)) {
							ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, enactor, obj1, obj2, victim, victim2,NULL, phrase, prg->trig_phrase,0,0,0,0,0);
							SETPRET;
						}
					}
				}
				iterator_stop(&pit);
				script_token_remref(token);
			}
		}
		iterator_stop(&tit);

		if(!script_destructed && IS_VALID(mob) && mob->id[0] == uid[0] && mob->id[1] == uid[1] && IS_NPC(mob) && mob->pIndexData->progs) {
			iterator_start(&pit, mob->pIndexData->progs[slot]);
			// Loop Level 1:
			while((prg = (PROG_LIST *)iterator_nextdata(&pit)) && !script_destructed) {
				if (is_trigger_type(prg->trig_type,type)) {
					if ((*match)(prg->trig_number, number)) {
						ret = execute_script(prg->vnum, prg->script, mob, NULL, NULL, NULL, enactor, obj1, obj2, victim, victim2,NULL,phrase,prg->trig_phrase,0,0,0,0,0);
						SETPRET;
					}
				}
			}
			iterator_stop(&pit);
		}
		script_mobile_remref(mob);

	} else if (obj && obj_room(obj) ) {
		// Save the UID
		uid[0] = obj->id[0];
		uid[1] = obj->id[1];
		script_object_addref(obj);

		// Check for tokens FIRST
		iterator_start(&tit, obj->ltokens);
		while((token = (TOKEN_DATA *)iterator_nextdata(&tit))) {
			if( token->pIndexData->progs ) {
				script_token_addref(token);
				script_destructed = FALSE;
				iterator_start(&pit, token->pIndexData->progs[slot]);
				while((prg = (PROG_LIST *)iterator_nextdata(&pit)) && !script_destructed) {
					if (is_trigger_type(prg->trig_type,type)) {
						if ((*match)(prg->trig_number, number)) {
							ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, enactor, obj1, obj2, victim, victim2,NULL,phrase,prg->trig_phrase,0,0,0,0,0);
							SETPRET;
						}
					}
				}
				iterator_stop(&pit);

				script_token_remref(token);
			}
		}
		iterator_stop(&tit);

		if(!script_destructed && IS_VALID(obj) && obj->id[0] == uid[0] && obj->id[1] == uid[1] && obj->pIndexData->progs) {
			script_destructed = FALSE;
			iterator_start(&pit, obj->pIndexData->progs[slot]);
			while((prg = (PROG_LIST *)iterator_nextdata(&pit)) && !script_destructed) {
				if (is_trigger_type(prg->trig_type,type)) {
					if ((*match)(prg->trig_number, number)) {
						ret = execute_script(prg->vnum, prg->script, NULL, obj, NULL, NULL, enactor, obj1, obj2, victim, victim2,NULL,phrase,prg->trig_phrase,0,0,0,0,0);
							SETPRET;
					}
				}
			}
			iterator_stop(&pit);
		}

		script_object_remref(obj);

	} else if (room) {
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
					if (is_trigger_type(prg->trig_type,type)) {
						if ((*match)(prg->trig_number, number)) {
							ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, enactor, obj1, obj2, victim, victim2,NULL,phrase,prg->trig_phrase,0,0,0,0,0);
							SETPRET;
						}
					}
				}
				iterator_stop(&pit);
				script_token_remref(token);
			}
		}
		iterator_stop(&tit);

		if(!script_destructed && source->progs->progs) {
			script_destructed = FALSE;
			iterator_start(&pit, source->progs->progs[slot]);
			while((prg = (PROG_LIST *)iterator_nextdata(&pit)) && !script_destructed) {
				if (is_trigger_type(prg->trig_type,type)) {
					if ((*match)(prg->trig_number, number)) {
						ret = execute_script(prg->vnum, prg->script, NULL, NULL, room, NULL, enactor, obj1, obj2, victim, victim2,NULL,phrase,prg->trig_phrase,0,0,0,0,0);
							SETPRET;
					}
				}
			}
			iterator_stop(&pit);

		}
		script_room_remref(room);
	} else if(token) {
		if( token->pIndexData->progs ) {
			script_token_addref(token);
			script_destructed = FALSE;
			iterator_start(&pit, token->pIndexData->progs[slot]);
			// Loop Level 2
			while((prg = (PROG_LIST *)iterator_nextdata(&pit)) && !script_destructed) {
				if (is_trigger_type(prg->trig_type,type)) {
					if ((*match)(prg->trig_number, number)) {
						ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, enactor, obj1, obj2, victim, victim2,NULL, phrase, prg->trig_phrase,0,0,0,0,0);
							SETPRET;
					}
				}
			}
			iterator_stop(&pit);
			script_token_remref(token);
		}

	} else
		bug("test_number_trigger: no program type for trigger %d.", type);

	PRETURN;
}


int p_percent_trigger(CHAR_DATA *mob, OBJ_DATA *obj, ROOM_INDEX_DATA *room, TOKEN_DATA *token,
	CHAR_DATA *ch, CHAR_DATA *victim, CHAR_DATA *victim2, OBJ_DATA *obj1, OBJ_DATA *obj2, int type, char *phrase)
{
	return test_number_trigger(0, match_percent, type, mob, obj, room, token, ch, victim, victim2, obj1, obj1, phrase);
}


int p_number_trigger(int number, CHAR_DATA *mob, OBJ_DATA *obj, ROOM_INDEX_DATA *room, TOKEN_DATA *token,
	CHAR_DATA *ch, CHAR_DATA *victim, CHAR_DATA *victim2, OBJ_DATA *obj1, OBJ_DATA *obj2, int type, char *phrase)
{
	return test_number_trigger(number, match_equal, type, mob, obj, room, token, ch, victim, victim2, obj1, obj1, phrase);
}

int p_bribe_trigger(CHAR_DATA *mob, CHAR_DATA *ch, int amount)
{
	return test_number_trigger(amount, match_gte, TRIG_BRIBE, mob, NULL, NULL, NULL, ch, NULL, NULL, NULL, NULL, NULL);
}


int test_number_sight_trigger(int number, MATCH_NUMBER match, int type, int typeall,
			CHAR_DATA *mob, OBJ_DATA *obj, ROOM_INDEX_DATA *room,
			CHAR_DATA *enactor, CHAR_DATA *victim, CHAR_DATA *victim2,
			OBJ_DATA *obj1, OBJ_DATA *obj2,
			char *phrase)
{
	ITERATOR tit;	// Token iterator
	ITERATOR pit;	// Prog iterator
	PROG_LIST *prg;
	TOKEN_DATA *token;
	unsigned long uid[2];
	int slot;
	int ret_val = PRET_NOSCRIPT, ret;

	if ((mob && obj) || (mob && room) || (obj && room)) {
		bug("test_number_sight_trigger: Multiple program types in trigger %d.", type);
		PRETURN;
	}

	// They must be in the same slot
	if( trigger_slots[type] != trigger_slots[typeall] )
	{
		bug("test_number_sight_trigger: slot mismatch for sighted trigger %d.", type);
		PRETURN;
	}

	slot = trigger_slots[typeall];

	if (mob) {
		script_mobile_addref(mob);

		// Save the UID
		uid[0] = mob->id[0];
		uid[1] = mob->id[1];

		// Check for tokens FIRST
		iterator_start(&tit, mob->ltokens);
		// Loop Level 1
		while((token = (TOKEN_DATA *)iterator_nextdata(&tit))) {
			if( token->pIndexData->progs ) {
				script_token_addref(token);
				script_destructed = FALSE;
				iterator_start(&pit, token->pIndexData->progs[slot]);
				// Loop Level 2
				while((prg = (PROG_LIST *)iterator_nextdata(&pit)) && !script_destructed) {
					if (type != typeall && is_trigger_type(prg->trig_type,type) &&
						(IS_SET(token->flags, TOKEN_SEE_ALL) ||
							(mob->position == mob->pIndexData->default_pos && can_see(mob, enactor))) &&
						(*match)(prg->trig_number, number)) {
						ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, enactor, obj1, obj2, victim, victim2,NULL, phrase, prg->trig_phrase,0,0,0,0,0);
						if( ret != PRET_NOSCRIPT) {
							iterator_stop(&tit);
							iterator_stop(&pit);
							script_token_remref(token);
							script_mobile_remref(mob);
							return ret;
						}

					} else if (is_trigger_type(prg->trig_type,typeall) &&
						(*match)(prg->trig_number, number)) {
						ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, enactor, obj1, obj2, victim, victim2,NULL, phrase, prg->trig_phrase,0,0,0,0,0);
						if( ret != PRET_NOSCRIPT) {
							iterator_stop(&tit);
							iterator_stop(&pit);
							script_token_remref(token);
							script_mobile_remref(mob);
							return ret;
						}

					}
					BREAKPRET;
				}
				iterator_stop(&pit);
				script_token_remref(token);
				BREAKPRET;
			}
		}
		iterator_stop(&tit);

		if(ret_val == PRET_NOSCRIPT && !script_destructed && IS_VALID(mob) && mob->id[0] == uid[0] && mob->id[1] == uid[1] && IS_NPC(mob) && mob->pIndexData->progs) {
			iterator_start(&pit, mob->pIndexData->progs[slot]);
			// Loop Level 1:
			while((prg = (PROG_LIST *)iterator_nextdata(&pit)) && !script_destructed) {
				if (type != typeall && is_trigger_type(prg->trig_type,type) &&
					mob->position == mob->pIndexData->default_pos &&
					can_see(mob, enactor) &&
					(*match)(prg->trig_number, number)) {
					ret = execute_script(prg->vnum, prg->script, mob, NULL, NULL, NULL, enactor, obj1, obj2, victim, victim2,NULL, phrase, prg->trig_phrase,0,0,0,0,0);
					if( ret != PRET_NOSCRIPT) {
						iterator_stop(&pit);
						script_mobile_remref(mob);
						return ret;
					}

				} else if (is_trigger_type(prg->trig_type,typeall) &&
					(*match)(prg->trig_number, number)) {
					ret = execute_script(prg->vnum, prg->script, mob, NULL, NULL, NULL, enactor, obj1, obj2, victim, victim2,NULL, phrase, prg->trig_phrase,0,0,0,0,0);
					if( ret != PRET_NOSCRIPT) {
						iterator_stop(&pit);
						script_mobile_remref(mob);
						return ret;
					}
				}
				BREAKPRET;
			}
			iterator_stop(&pit);
		}
		script_mobile_remref(mob);

	} else if (obj && obj_room(obj) ) {
		// Save the UID
		uid[0] = obj->id[0];
		uid[1] = obj->id[1];
		script_object_addref(obj);

		// Check for tokens FIRST
		iterator_start(&tit, obj->ltokens);
		while((token = (TOKEN_DATA *)iterator_nextdata(&tit))) {
			if( token->pIndexData->progs ) {
				script_token_addref(token);
				script_destructed = FALSE;
				iterator_start(&pit, token->pIndexData->progs[slot]);
				while((prg = (PROG_LIST *)iterator_nextdata(&pit)) && !script_destructed) {
					if (is_trigger_type(prg->trig_type,typeall)) {
						if ((*match)(prg->trig_number, number)) {
							ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, enactor, obj1, obj2, victim, victim2,NULL,phrase,prg->trig_phrase,0,0,0,0,0);
							if( ret != PRET_NOSCRIPT) {
								iterator_stop(&tit);
								iterator_stop(&pit);
								script_token_remref(token);
								script_object_remref(obj);
								return ret;
							}

						}
					}
				}
				iterator_stop(&pit);

				script_token_remref(token);
				BREAKPRET;
			}
		}
		iterator_stop(&tit);

		if(ret_val == PRET_NOSCRIPT && !script_destructed && IS_VALID(obj) && obj->id[0] == uid[0] && obj->id[1] == uid[1] && obj->pIndexData->progs) {
			script_destructed = FALSE;
			iterator_start(&pit, obj->pIndexData->progs[slot]);
			while((prg = (PROG_LIST *)iterator_nextdata(&pit)) && !script_destructed) {
				if (is_trigger_type(prg->trig_type,typeall)) {
					if ((*match)(prg->trig_number, number)) {
						ret = execute_script(prg->vnum, prg->script, NULL, obj, NULL, NULL, enactor, obj1, obj2, victim, victim2,NULL,phrase,prg->trig_phrase,0,0,0,0,0);
						if( ret != PRET_NOSCRIPT) {
							iterator_stop(&pit);
							script_object_remref(obj);
							return ret;
						}
					}
				}
				BREAKPRET;
			}
			iterator_stop(&pit);
		}

		script_object_remref(obj);

	} else if (room) {
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
					if (is_trigger_type(prg->trig_type,typeall)) {
						if ((*match)(prg->trig_number, number)) {
							ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, enactor, obj1, obj2, victim, victim2,NULL,phrase,prg->trig_phrase,0,0,0,0,0);
							if( ret != PRET_NOSCRIPT) {
								iterator_stop(&tit);
								iterator_stop(&pit);
								script_token_remref(token);
								script_room_remref(room);
								return ret;
							}

						}
					}
					BREAKPRET;
				}
				iterator_stop(&pit);
				script_token_remref(token);

				BREAKPRET;
			}
		}
		iterator_stop(&tit);

		if(ret_val == PRET_NOSCRIPT && source->progs->progs) {
			script_destructed = FALSE;
			iterator_start(&pit, source->progs->progs[slot]);
			while((prg = (PROG_LIST *)iterator_nextdata(&pit)) && !script_destructed) {
				if (is_trigger_type(prg->trig_type,typeall)) {
					if ((*match)(prg->trig_number, number)) {
						ret = execute_script(prg->vnum, prg->script, NULL, NULL, room, NULL, enactor, obj1, obj2, victim, victim2,NULL,phrase,prg->trig_phrase,0,0,0,0,0);
							if( ret != PRET_NOSCRIPT) {
								iterator_stop(&pit);

								script_room_remref(room);
								return ret;
							}
					}
				}
				BREAKPRET;
			}
			iterator_stop(&pit);

		}
		script_room_remref(room);

	} else
		bug("test_number_trigger: no program type for trigger %d.", type);

	PRETURN;
}


int p_location_trigger(CHAR_DATA *ch, ROOM_INDEX_DATA *room, int number, MATCH_NUMBER match, int type, int trig, int trigall)
{
	ITERATOR oit, mit;

	CHAR_DATA *mob;
	OBJ_DATA *obj;
	//TOKEN_DATA *token;
	//PROG_LIST *prg;
	//unsigned long uid[2];
	int ret_val = PRET_NOSCRIPT; // Default for a trigger loop is NO SCRIPT

	// If not in a valid room, there's nothing to check!
	if (!ch || !room)
		PRETURN;

	if (type == PRG_MPROG) {
		iterator_start(&mit, room->lpeople);
		while((ret_val == PRET_NOSCRIPT) && (mob = (CHAR_DATA *)iterator_nextdata(&mit))) {
			ret_val = test_number_sight_trigger(number, match, trig, trigall,
						mob, NULL, NULL, ch, NULL, NULL, NULL, NULL, NULL);
		}

		iterator_stop(&mit);

	} else if (type == PRG_OPROG) {
		// Check carrying inventory
		iterator_start(&oit, ch->lcarrying);
		while((ret_val == PRET_NOSCRIPT) && (obj = (OBJ_DATA *)iterator_nextdata(&oit))) {
			ret_val = test_number_sight_trigger(number, match, trig, trigall,
						NULL, obj, NULL, ch, NULL, NULL, NULL, NULL, NULL);
		}

		iterator_stop(&oit);
		ISSETPRET;

		// Check room contents
		iterator_start(&oit, room->lcontents);
		while((ret_val == PRET_NOSCRIPT) && (obj = (OBJ_DATA *)iterator_nextdata(&oit))) {
			ret_val = test_number_sight_trigger(number, match, trig, trigall,
						NULL, obj, NULL, ch, NULL, NULL, NULL, NULL, NULL);
		}

		iterator_stop(&oit);
		ISSETPRET;

		iterator_start(&mit, room->lpeople);
		while((ret_val == PRET_NOSCRIPT) && (mob = (CHAR_DATA *)iterator_nextdata(&mit))) {
			// Check every other mobile
			if( mob != ch ) {
				iterator_start(&oit, mob->lcarrying);
				while((ret_val == PRET_NOSCRIPT) && (obj = (OBJ_DATA *)iterator_nextdata(&oit))) {
					ret_val = test_number_sight_trigger(number, match, trig, trigall,
								NULL, obj, NULL, ch, NULL, NULL, NULL, NULL, NULL);
				}
				iterator_stop(&oit);
			}
		}
		iterator_stop(&mit);

	} else if (type == PRG_RPROG) {
		ret_val = test_number_sight_trigger(number, match, trig, trigall,
					NULL, NULL, room, ch, NULL, NULL, NULL, NULL, NULL);
	}

	PRETURN;
}

int p_exit_trigger(CHAR_DATA *ch, int dir, int type)
{
	return p_location_trigger(ch, ch ? ch->in_room : NULL, dir, match_equal, type, TRIG_EXIT, TRIG_EXALL);
}

int p_direction_trigger(CHAR_DATA *ch, ROOM_INDEX_DATA *here, int dir, int type, int trigger)
{
	return p_location_trigger(ch, here, dir, match_equal, type, trigger, trigger);
}


static bool __attribute__ ((unused)) match_target_name(register char *a, register char *b)
{
	char buf[MIL];

	while(*a) {
		a = one_argument(a, buf);
		if( is_name(buf, b) || !str_cmp("all", buf) )
			return TRUE;
	}

	return FALSE;
}


int test_vnumname_trigger(char *name, int vnum, int type,
			CHAR_DATA *mob, OBJ_DATA *obj, ROOM_INDEX_DATA *room,
			CHAR_DATA *enactor, CHAR_DATA *victim, CHAR_DATA *victim2,
			OBJ_DATA *obj1, OBJ_DATA *obj2,
			char *phrase)
{
	ITERATOR tit;	// Token iterator
	ITERATOR pit;	// Prog iterator
	PROG_LIST *prg;
	TOKEN_DATA *token;
	unsigned long uid[2];
	int slot;
	int ret_val = PRET_NOSCRIPT, ret;

	if ((mob && obj) || (mob && room) || (obj && room)) {
		bug("test_vnumname_trigger: Multiple program types in trigger %d.", type);
		PRETURN;
	}

	slot = trigger_slots[type];


	if (mob) {
		script_mobile_addref(mob);

		// Save the UID
		uid[0] = mob->id[0];
		uid[1] = mob->id[1];

		// Check for tokens FIRST
		iterator_start(&tit, mob->ltokens);
		// Loop Level 1
		while((token = (TOKEN_DATA *)iterator_nextdata(&tit))) {
			if( token->pIndexData->progs ) {
				script_token_addref(token);
				script_destructed = FALSE;
				iterator_start(&pit, token->pIndexData->progs[slot]);
				// Loop Level 2
				while((prg = (PROG_LIST *)iterator_nextdata(&pit)) && !script_destructed) {
					if (is_trigger_type(prg->trig_type,type)) {
						if ( (prg->numeric && match_equal(prg->trig_number, vnum)) ||
							(!prg->numeric && match_target_name(prg->trig_phrase, name)) ) {
							ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, enactor, obj1, obj2, victim, victim2,NULL, phrase, prg->trig_phrase,0,0,0,0,0);
							if( ret != PRET_NOSCRIPT) {
								iterator_stop(&tit);
								iterator_stop(&pit);

								script_token_remref(token);
								script_mobile_remref(mob);
								return ret;
							}
						}
					}
				}
				iterator_stop(&pit);
				script_token_remref(token);
			}
		}
		iterator_stop(&tit);

		if(!script_destructed && IS_VALID(mob) && mob->id[0] == uid[0] && mob->id[1] == uid[1] && IS_NPC(mob) && mob->pIndexData->progs) {
			iterator_start(&pit, mob->pIndexData->progs[slot]);
			// Loop Level 1:
			while((prg = (PROG_LIST *)iterator_nextdata(&pit)) && !script_destructed) {
				if (is_trigger_type(prg->trig_type,type)) {
					if ( (prg->numeric && match_equal(prg->trig_number, vnum)) ||
						(!prg->numeric && match_target_name(prg->trig_phrase, name)) ) {
						ret = execute_script(prg->vnum, prg->script, mob, NULL, NULL, NULL, enactor, obj1, obj2, victim, victim2,NULL,phrase,prg->trig_phrase,0,0,0,0,0);
							if( ret != PRET_NOSCRIPT) {
								iterator_stop(&pit);

								script_mobile_remref(mob);
								return ret;
							}

					}
				}
				BREAKPRET;
			}
			iterator_stop(&pit);
		}
		script_mobile_remref(mob);

	} else if (obj && obj_room(obj) ) {
		// Save the UID
		uid[0] = obj->id[0];
		uid[1] = obj->id[1];
		script_object_addref(obj);

		// Check for tokens FIRST
		iterator_start(&tit, obj->ltokens);
		while((token = (TOKEN_DATA *)iterator_nextdata(&tit))) {
			if( token->pIndexData->progs ) {
				script_token_addref(token);
				script_destructed = FALSE;
				iterator_start(&pit, token->pIndexData->progs[slot]);
				while((prg = (PROG_LIST *)iterator_nextdata(&pit)) && !script_destructed) {
					if (is_trigger_type(prg->trig_type,type)) {
						if ( (prg->numeric && match_equal(prg->trig_number, vnum)) ||
							(!prg->numeric && match_target_name(prg->trig_phrase, name)) ) {
							ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, enactor, obj1, obj2, victim, victim2,NULL,phrase,prg->trig_phrase,0,0,0,0,0);
							if( ret != PRET_NOSCRIPT) {
								iterator_stop(&tit);
								iterator_stop(&pit);

								script_token_remref(token);
								script_object_remref(obj);
								return ret;
							}

						}
					}
				}
				iterator_stop(&pit);

				script_token_remref(token);
				BREAKPRET;
			}
		}
		iterator_stop(&tit);

		if(ret_val == PRET_NOSCRIPT && !script_destructed && IS_VALID(obj) && obj->id[0] == uid[0] && obj->id[1] == uid[1] && obj->pIndexData->progs) {
			script_destructed = FALSE;
			iterator_start(&pit, obj->pIndexData->progs[slot]);
			while((prg = (PROG_LIST *)iterator_nextdata(&pit)) && !script_destructed) {
				if (is_trigger_type(prg->trig_type,type)) {
					if ( (prg->numeric && match_equal(prg->trig_number, vnum)) ||
						(!prg->numeric && match_target_name(prg->trig_phrase, name)) ) {
						ret = execute_script(prg->vnum, prg->script, NULL, obj, NULL, NULL, enactor, obj1, obj2, victim, victim2,NULL,phrase,prg->trig_phrase,0,0,0,0,0);
							if( ret != PRET_NOSCRIPT) {
								iterator_stop(&pit);

								script_object_remref(obj);
								return ret;
							}

					}
				}
				BREAKPRET;
			}
			iterator_stop(&pit);
		}

		script_object_remref(obj);

	} else if (room) {
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
					if (is_trigger_type(prg->trig_type,type)) {
						if ( (prg->numeric && match_equal(prg->trig_number, vnum)) ||
							(!prg->numeric && match_target_name(prg->trig_phrase, name)) ) {
							ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, enactor, obj1, obj2, victim, victim2,NULL,phrase,prg->trig_phrase,0,0,0,0,0);
							if( ret != PRET_NOSCRIPT) {
								iterator_stop(&tit);
								iterator_stop(&pit);

								script_token_remref(token);
								script_room_remref(room);
								return ret;
							}
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
				if (is_trigger_type(prg->trig_type,type)) {
					if ( (prg->numeric && match_equal(prg->trig_number, vnum)) ||
						(!prg->numeric && match_target_name(prg->trig_phrase, name)) ) {
						ret = execute_script(prg->vnum, prg->script, NULL, NULL, room, NULL, enactor, obj1, obj2, victim, victim2,NULL,phrase,prg->trig_phrase,0,0,0,0,0);
							if( ret != PRET_NOSCRIPT) {
								iterator_stop(&pit);

								script_room_remref(room);
								return ret;
							}

					}
				}
			}
			iterator_stop(&pit);

		}
		script_room_remref(room);

	} else
		bug("test_number_trigger: no program type for trigger %d.", type);

	PRETURN;
}



int p_give_trigger(CHAR_DATA *mob, OBJ_DATA *obj, ROOM_INDEX_DATA *room,
			CHAR_DATA *ch, OBJ_DATA *dropped, int type)
{
	return test_vnumname_trigger(dropped->name, dropped->pIndexData->vnum, type,
					mob, obj, room, ch, NULL, NULL, dropped, NULL, NULL);
}


int p_use_trigger(CHAR_DATA *ch, OBJ_DATA *obj, int type)
{
	if (obj == NULL) {
		bug("p_use_trigger: received null obj!", 0);
		return PRET_NOSCRIPT;
	}

	if (!obj_room(obj)) return PRET_NOSCRIPT;


	if (type == TRIG_PUSH || type == TRIG_TURN || type == TRIG_PULL || type == TRIG_USE)
		return test_number_trigger(0, match_percent, type, NULL, obj, NULL, NULL, ch, NULL, NULL, NULL, NULL, NULL);

	return PRET_NOSCRIPT;
}

int p_use_on_trigger(CHAR_DATA *ch, OBJ_DATA *obj, int type, char *argument)
{
	if (obj == NULL) {
		bug("p_use_on_trigger: received null obj!", 0);
		return PRET_NOSCRIPT;
	}

	if (!obj_room(obj)) return PRET_NOSCRIPT;

	if (type == TRIG_PUSH_ON || type == TRIG_TURN_ON || type == TRIG_PULL_ON)
		return test_string_trigger(argument, match_substr, type, NULL, obj, NULL, ch, NULL, NULL, NULL, NULL);

	return PRET_NOSCRIPT;
}

int p_use_with_trigger(CHAR_DATA *ch, OBJ_DATA *obj, int type, OBJ_DATA *obj1, OBJ_DATA *obj2, CHAR_DATA *victim, CHAR_DATA *victim2)
{
	if (obj == NULL) {
		bug("p_use_with_trigger: received null obj!", 0);
		return PRET_NOSCRIPT;
	}

	if (!obj_room(obj)) return PRET_NOSCRIPT;

	if (type == TRIG_USEWITH)
		return test_number_trigger(0, match_percent, type, NULL, obj, NULL, NULL, ch, victim, victim2, obj1, obj2, NULL);

	return PRET_NOSCRIPT;
}


int p_greet_trigger(CHAR_DATA *ch, int type)
{
	return p_location_trigger(ch, ch ? ch->in_room : NULL, 0, match_percent, type, TRIG_GREET, TRIG_GRALL);
}


int p_hprct_trigger(CHAR_DATA *mob, CHAR_DATA *ch) // @@@NIB
{
	return test_number_trigger((100 * mob->hit / mob->max_hit), match_lt, TRIG_HPCNT, mob, NULL, NULL, NULL, ch, NULL, NULL, NULL, NULL, NULL);
}

int p_emote_trigger(CHAR_DATA *ch, char *emote)
{
	int ret_val = PRET_NOSCRIPT, ret;
	// Check tokens on player

	ret_val = test_string_trigger(emote, match_exact_name, TRIG_EMOTE, ch, NULL, NULL, ch, NULL, NULL, NULL, NULL);


	if( ret_val == PRET_NOSCRIPT && ch->in_room != NULL) {
		CHAR_DATA *mob, *mob_next;

		// Check all mobs in the room
		for (mob = ch->in_room->people; mob != NULL; mob = mob_next) {
			mob_next = mob->next_in_room;

			if (!IS_NPC(mob) || mob->position == mob->pIndexData->default_pos)
			{
				ret = test_string_trigger(emote, match_exact_name, TRIG_EMOTE, mob, NULL, NULL, ch, NULL, NULL, NULL, NULL);

				if( ret != PRET_NOSCRIPT ) {
					ret_val = ret;
					break;
				}
			}
		}
	}

	PRETURN;
}

int p_emoteat_trigger(CHAR_DATA *mob, CHAR_DATA *ch, char *emote)
{
	int trig = (mob != ch) ? TRIG_EMOTEAT : TRIG_EMOTESELF;
	return test_string_trigger(emote, match_exact_name, trig, mob, NULL, NULL, ch, NULL, NULL, NULL, NULL);
}

int script_login(CHAR_DATA *ch) // @@@NIB
{
	ITERATOR tit, oit, pit;
	TOKEN_DATA *token;
	OBJ_DATA *obj;
	PROG_LIST *prg;
	SCRIPT_DATA *script;
	unsigned long uid[2];
	//unsigned long ouid[2];
	int slot;
	int ret_val = PRET_NOSCRIPT, ret; // @@@NIB Default for a trigger loop is NO SCRIPT

	variable_dynamic_fix_mobile(ch);

	// Run the SYSTEM LOGIN ROOM SCRIPT
	script = get_script_index(RPROG_VNUM_PLAYER_INIT,PRG_RPROG);
	if(script) {
		script_force_execute = TRUE;
		script_security = SYSTEM_SCRIPT_SECURITY;
		execute_script(RPROG_VNUM_PLAYER_INIT, script, NULL, NULL, get_room_index(1), NULL, ch, NULL, NULL, NULL, NULL,NULL,NULL,NULL,0,0,0,0,0);
		script_security = INIT_SCRIPT_SECURITY;
		script_force_execute = FALSE;
	}

	// Run the TRIG_LOGIN
	slot = trigger_slots[TRIG_LOGIN];

	// Save the UID
	uid[0] = ch->id[0];
	uid[1] = ch->id[1];

	// Check for tokens FIRST
	iterator_start(&tit, ch->ltokens);
	while(( token = (TOKEN_DATA *)iterator_nextdata(&tit))) {
		if(token->pIndexData->progs) {
			script_token_addref(token);
			script_destructed = FALSE;
			iterator_start(&pit, token->pIndexData->progs[slot]);
			while((prg = (PROG_LIST *)iterator_nextdata(&pit)) && !script_destructed) {
				if (is_trigger_type(prg->trig_type,TRIG_LOGIN) && number_percent() < prg->trig_number) {
					ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, ch, NULL, NULL, NULL, NULL,NULL,NULL,NULL,0,0,0,0,0);
					SETPRET;
				}
			}
			script_token_remref(token);
			iterator_stop(&pit);
		}
	}
	iterator_stop(&tit);

	// Check objects SECOND
	iterator_start(&oit, ch->lcarrying);
	while(( obj = (OBJ_DATA *)iterator_nextdata(&oit))) {
		iterator_start(&tit, obj->ltokens);
		while(( token = (TOKEN_DATA *)iterator_nextdata(&tit))) {
			if(token->pIndexData->progs) {
				script_destructed = FALSE;
				iterator_start(&pit, token->pIndexData->progs[slot]);
				while((prg = (PROG_LIST *)iterator_nextdata(&pit)) && !script_destructed) {
					if (is_trigger_type(prg->trig_type,TRIG_LOGIN) && number_percent() < prg->trig_number) {
						ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, ch, NULL, NULL, NULL,NULL, NULL,NULL,NULL,0,0,0,0,0);
						SETPRET;
					}
				}
				iterator_stop(&pit);
			}
		}
		iterator_stop(&tit);

		if(obj->pIndexData->progs) {
			script_destructed = FALSE;
			iterator_start(&pit, obj->pIndexData->progs[slot]);
			while((prg = (PROG_LIST *)iterator_nextdata(&pit)) && !script_destructed) {
				if (is_trigger_type(prg->trig_type,TRIG_LOGIN) && number_percent() < prg->trig_number) {
					ret = execute_script(prg->vnum, prg->script, NULL, obj, NULL, NULL, ch, NULL, NULL, NULL, NULL,NULL,NULL,NULL,0,0,0,0,0);
					SETPRET;
				}
			}
			iterator_stop(&pit);
		}
	}
	iterator_stop(&oit);

	if(ret_val == PRET_NOSCRIPT && IS_VALID(ch) && ch->id[0] == uid[0] && ch->id[0] == uid[1] && IS_NPC(ch) && ch->pIndexData->progs) {
		script_destructed = FALSE;
		iterator_start(&pit, ch->pIndexData->progs[slot]);
		while((prg = (PROG_LIST *)iterator_nextdata(&pit)) && !script_destructed) {
			if (is_trigger_type(prg->trig_type,TRIG_LOGIN) && number_percent() < prg->trig_number &&
				((ret = execute_script(prg->vnum, prg->script, ch, NULL, NULL, NULL, ch, NULL, NULL, NULL, NULL,NULL,NULL,NULL,0,0,0,0,0)) != PRET_NOSCRIPT)) {
				ret_val = ret;
				break;
			}
		}
		iterator_stop(&pit);
	}

	PRETURN; // @@@NIB
}

void do_ifchecks( CHAR_DATA *ch, char *argument)
{
	char buf[MIL]/*, *pbuf*/;
	BUFFER *buffer;
	int i,j;

	if(!ch->lines) {
		send_to_char("Viewing this with paging off is not permitted due to the number of ifchecks.\n\r",ch);
		return;
	}

	buffer = new_buf();
	if(!buffer) {
		send_to_char("WTF?! Couldn't create the buffer!\n\r",ch);
		return;
	}

	add_buf(buffer,"{WIf-Checks:{x\n\r");
	add_buf(buffer,"{D==============================================================={x\n\r");
	add_buf(buffer,"{WNum  {D| {WName                {D| {W         Types          {D| {WValue {D|{x\n\r");
	add_buf(buffer,"{D---------------------------------------------------------------{x\n\r");

	for(i=0,j=0;ifcheck_table[i].name;i++) if(!*argument || is_name(argument,ifcheck_table[i].name)) {
		sprintf(buf,"{W%4d{D)  {Y%-20.20s %s %s %s %s %s   %s{x\n\r",++j,
			ifcheck_table[i].name,
			((ifcheck_table[i].type & IFC_M) ? "{Gmob" : "   "),
			((ifcheck_table[i].type & IFC_O) ? "{Gobj" : "   "),
			((ifcheck_table[i].type & IFC_R) ? "{Groom" : "    "),
			((ifcheck_table[i].type & IFC_T) ? "{Gtoken" : "     "),
			((ifcheck_table[i].type & IFC_A) ? "{Garea" : "    "),
			(ifcheck_table[i].numeric ? "{B NUM " : "{R T/F "));
		add_buf(buffer,buf);
	}
	add_buf(buffer,"{D==============================================================={x\n\r");

//	pbuf = buf_string(buffer);
//	sprintf(buf,"pbuf = '%.15s{x', %d\n\r", pbuf, strlen(pbuf));

	if(j > 0)
		page_to_char(buf_string(buffer), ch);
//		send_to_char(buf,ch);
	else
		send_to_char("No ifchecks match that name pattern.\n\r",ch);
	free_buf(buffer);
	return;
}


char *get_script_prompt_string(CHAR_DATA *ch, char *key)
{
	STRING_VECTOR *v;
	if(IS_NPC(ch) || !ch->pcdata->script_prompts) return "";

	v = string_vector_find(ch->pcdata->script_prompts,key);
	return v ? v->string : "";
}


// Returns TRUE if the spell got through.
// Used for token scripts
bool script_spell_deflection(CHAR_DATA *ch, CHAR_DATA *victim, TOKEN_DATA *token, SCRIPT_DATA *script, int mana)
{
	CHAR_DATA *rch = NULL;
	AFFECT_DATA *af;
	int attempts;
	int lev;
	int type;

	if (!IS_AFFECTED2(victim, AFF2_SPELL_DEFLECTION))
		return TRUE;

	act("{MThe crimson aura around you pulses!{x", victim, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
	act("{MThe crimson aura around $n pulses!{x", victim, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);

	// Find spell deflection
	for (af = victim->affected; af; af = af->next) {
		if (af->type == skill_lookup("spell deflection"))
		break;
	}

	if (!af) return TRUE;

	lev = (af->level * 3)/4;
	lev = URANGE(15, lev, 90);

	if (number_percent() > lev) {
		if (ch) {
			if (ch == victim)
				send_to_char("Your spell gets through your protective crimson aura!\n\r", ch);
			else {
				act("Your spell gets through $N's protective crimson aura!", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
				act("$n's spell gets through your protective crimson aura!", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_VICT);
				act("$n's spell gets through $N's protective crimson aura!", ch, victim, NULL, NULL, NULL, NULL, NULL, TO_NOTVICT);
			}
		}

		return TRUE;
	}

	type = token->pIndexData->value[TOKVAL_SPELL_TARGET];
	/* it bounces to a random person */
	if (type != TAR_IGNORE)
		for (attempts = 0; attempts < 6; attempts++) {
			rch = get_random_char(NULL, NULL, victim->in_room, NULL);
			if ((ch && rch == ch) || rch == victim ||
				((type == TAR_CHAR_OFFENSIVE || type == TAR_OBJ_CHAR_OFF) && ch && is_safe(ch, rch, FALSE))) {
				rch = NULL;
				continue;
			}
		}

	// Loses potency with time
	af->level -= 10;
	if (af->level <= 0) {
		send_to_char("{MThe crimson aura around you vanishes.{x\n\r", victim);
		act("{MThe crimson aura around $n vanishes.{x", victim, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
		affect_remove(victim, af);
		return TRUE;
	}

	if (rch) {
		if (ch) {
			act("{YYour spell bounces off onto $N!{x",  ch, rch, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
			act("{Y$n's spell bounces off onto you!{x", ch, rch, NULL, NULL, NULL, NULL, NULL, TO_VICT);
			act("{Y$n's spell bounces off onto $N!{x",  ch, rch, NULL, NULL, NULL, NULL, NULL, TO_NOTVICT);
		}

		token->value[3] = ch ? ch->tot_level : af->level;

		execute_script(script->vnum, script, NULL, NULL, NULL, token, ch ? ch : rch, NULL, NULL, rch, NULL,NULL,"deflection",NULL,0,0,0,0,0);
	} else if (ch) {
		act("{YYour spell bounces around for a while, then dies out.{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_CHAR);
		act("{Y$n's spell bounces around for a while, then dies out.{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM);
	}

	return FALSE;
}


void token_skill_improve( CHAR_DATA *ch, TOKEN_DATA *token, bool success, int multiplier )
{
	int chance, per;
	char buf[100];
	int rating, max_rating, diff;

	if (IS_NPC(ch))
		return;

	if (IS_SOCIAL(ch))
		return;

	rating = token->value[TOKVAL_SPELL_RATING];
	max_rating = token->pIndexData->value[TOKVAL_SPELL_RATING];
	diff = token->pIndexData->value[TOKVAL_SPELL_DIFFICULTY];

	if(!max_rating) max_rating = 100;

	if(rating < 1 || rating >= max_rating)
		return;

	// check to see if the character has a chance to learn
	chance      = 10 * int_app[get_curr_stat(ch, STAT_INT)].learn;
	multiplier  = UMAX(multiplier,1);
	chance     /= (multiplier * diff * 4);
	chance     += ch->level;

	if (number_range(1,1000) > chance)
		return;

	per = 100 * rating / max_rating;

	// now that the character has a CHANCE to learn, see if they really have
	if (success) {
		chance = URANGE(2, 100 - per, 25);
		if (number_percent() < chance) {
			sprintf(buf,"{WYou have become better at %s!{x\n\r", token->name);
			send_to_char(buf,ch);
			token->value[TOKVAL_SPELL_RATING]++;
			gain_exp(ch, 2 * diff);
		}
	} else {
		chance = URANGE(5, per/2, 30);
		if (number_percent() < chance) {
			sprintf(buf, "{WYou learn from your mistakes, and your %s skill improves.{x\n\r", token->name);
			send_to_char(buf, ch);
			token->value[TOKVAL_SPELL_RATING] += number_range(1,3);
			if(token->value[TOKVAL_SPELL_RATING] >= max_rating)
				token->value[TOKVAL_SPELL_RATING] = max_rating;
			gain_exp(ch,2 * diff);
		}
	}
}

SCRIPT_VARINFO *script_get_prior(SCRIPT_VARINFO *info)
{
	return ((info && info->block && info->block->next) ? &(info->block->next->info) : NULL);
}

int interrupt_script( CHAR_DATA *ch, bool silent )
{
	return p_percent_trigger(ch, NULL, NULL, NULL, ch, NULL, NULL, NULL, NULL, TRIG_INTERRUPT, silent?"silent":NULL);
}


void script_varseton(SCRIPT_VARINFO *info, ppVARIABLE vars, char *argument)
{
	char buf[MIL], name[MIL], *rest, *str = NULL;
	CHAR_DATA *vch = NULL, *mobs = NULL, *viewer = NULL;
	OBJ_DATA *obj = NULL, *objs = NULL;
	TOKEN_DATA *token = NULL, *tokens = NULL;
	ROOM_INDEX_DATA *here = NULL;
	EXIT_DATA *ex = NULL;
	LLIST *blist;
	ITERATOR it;
	int vnum = 0, i, idx;
	unsigned long id1/*, id2*/;
	SCRIPT_PARAM arg;

	if(!info) return;
	viewer = info->mob;

	if(info->mob) here = info->mob->in_room;
	else if(info->obj) here = obj_room(info->obj);
	else if(info->room) here = info->room;
	else if(info->token) here = token_room(info->token);

	if(!viewer && !here) return;

	if(!vars) return;

	// Get name
	argument = one_argument(argument,name);
	if(!name[0]) return;

	// Get type
	argument = one_argument(argument,buf);
	if(!buf[0]) return;

	if(!(rest = expand_argument(info,argument,&arg)))
		return;

	// Saves a number
	// Format: INTEGER <number>
	// Format: INTEGER <numerical string>
	if(!str_cmp(buf,"integer") || !str_cmp(buf,"number")) {
		switch(arg.type) {
		case ENT_NUMBER: variables_set_integer(vars,name,arg.d.num); break;
		case ENT_STRING:
			if(is_number(arg.d.str))
				variables_set_integer(vars,name,atoi(arg.d.str)); break;
		}

	// Format: STRING <string>[ <word index>]
	} else if(!str_cmp(buf,"string")) {
		char tmp[MSL],*p;

		switch(arg.type) {
		case ENT_NUMBER:
			sprintf(tmp,"%d",arg.d.num);
			break;
		case ENT_STRING:
			strcpy(tmp,arg.d.str);
			break;
		default:return;
		}

		if(!(rest = expand_argument(info,rest,&arg)))
			return;

		switch(arg.type) {
		case ENT_NONE:
			variables_set_string(vars,name,tmp,FALSE);
			break;

		case ENT_NUMBER:
			p = tmp;
			for(i=0;i<arg.d.num && p && *p;i++)
				p = one_argument(p,buf);
			if(arg.d.num > 0 && i == arg.d.num)
				variables_set_string(vars,name,buf,FALSE);
			break;
		}

	// Format: APPEND <string>[ <word index>]
	} else if(!str_cmp(buf,"append")) {
		char tmp[MSL], *p;

		switch(arg.type) {
		case ENT_NUMBER:	sprintf(tmp,"%d",arg.d.num); break;
		case ENT_STRING:	strcpy(tmp,arg.d.str); 	break;
		default:return;
		}

		if(!(rest = expand_argument(info,rest,&arg))) return;

		switch(arg.type) {
		case ENT_NONE:		variables_append_string(vars,name,tmp); break;
		case ENT_NUMBER:
			p = tmp;
			for(i=0;i<arg.d.num && p && *p;i++) p = one_argument(p,buf);

			if(arg.d.num > 0 && (i == arg.d.num)) variables_append_string(vars,name,buf);
			break;
		}

	// Format: ARGREMOVE <word index>
	// Format: ARGREMOVE <word to remove>
	} else if(!str_cmp(buf,"ARGREMOVE")) {
		switch(arg.type) {
		case ENT_NUMBER:

			variables_argremove_string_index(vars,name,arg.d.num);
			break;
		case ENT_STRING:

			variables_argremove_string_phrase(vars,name,arg.d.str);
			break;
		}

	// Format: STRFORMAT
	// TO-DO: Add "STRFORMAT[ <width>]"
	} else if(!str_cmp(buf,"strformat")) {
		variables_format_string(vars,name);

	// Copies an extra description
	// Format: ED <OBJECT or ROOM> <keyword>
	} else if(!str_cmp(buf,"ed")) {
		// ed $<object|room> <keyword>
		char tmp[MSL],*p;
		EXTRA_DESCR_DATA *desc;

		switch(arg.type) {
		case ENT_OBJECT: desc = arg.d.obj->extra_descr; break;
		case ENT_ROOM: desc = arg.d.room->extra_descr; break;
		default:return;
		}

		expand_string(info,rest,tmp);

		p = get_extra_descr(tmp, desc);

		variables_set_string(info->var,name,(p ? p : ""),FALSE);

	// Format: ROOM <VNUM> - room vnum
	// Format: ROOM <ROOM> - explicit room
	// Format: ROOM <EXIT> - gets the destination of the exit
	// Format: ROOM <ROOM-LIST> - first room from the list
	// Format: ROOM <ROOM-LIST> <INDEX> - Nth room from the list
	// Format: ROOM <ROOM-LIST> FIRST - first room from the list
	// Format: ROOM <ROOM-LIST> LAST - last room from the list
	// Format: ROOM <ROOM-LIST> RANDOM - random valid room from the list
	} else if(!str_cmp(buf,"room")) {
		switch(arg.type) {
		case ENT_NUMBER:
			variables_set_room(vars,name,get_room_index(arg.d.num));
			break;
		case ENT_ROOM:
			variables_set_room(vars,name,arg.d.room);
			break;
		case ENT_EXIT:
			here = arg.d.door.r ? exit_destination(arg.d.door.r->exit[arg.d.door.door]) : NULL;
			variables_set_room(vars,name,here);
			break;
		case ENT_BLLIST_ROOM:
			blist = arg.d.blist;
			if(!(rest = expand_argument(info,rest,&arg)))
				return;

			idx = 0;
			switch(arg.type) {
			default: break;
			case ENT_STRING:
				if( !str_cmp(arg.d.str, "first"))
					idx = 0;
				else if( !str_cmp(arg.d.str, "last"))
					idx = list_size(blist)-1;
				else if( !str_cmp(arg.d.str, "random"))
					idx = number_range(0, list_size(blist)-1);
				break;

			case ENT_NUMBER:
				idx = arg.d.num;
				break;
			}

			{
				LLIST_ROOM_DATA *lroom;
				iterator_start_nth(&it, blist, idx);
				while((lroom = (LLIST_ROOM_DATA *)iterator_nextdata(&it)) && !lroom->room);
				iterator_stop(&it);

				if(lroom && lroom->room)
					variables_set_room(vars,name,lroom->room);
			}
			break;
		case ENT_PLLIST_ROOM:
			blist = arg.d.blist;
			if(!(rest = expand_argument(info,rest,&arg)))
				return;

			idx = 0;
			switch(arg.type) {
			default: break;
			case ENT_STRING:
				if( !str_cmp(arg.d.str, "first"))
					idx = 0;
				else if( !str_cmp(arg.d.str, "last"))
					idx = list_size(blist)-1;
				else if( !str_cmp(arg.d.str, "random"))
					idx = number_range(0, list_size(blist)-1);
				break;

			case ENT_NUMBER:
				idx = arg.d.num;
				break;
			}

			variables_set_room(vars, name, list_nthdata(blist, idx));
			break;
		}

	// Find the highest room at that coordinate
	// Format: HIGHROOM <MAP UID> <X> <Y> <Z> <GROUND>
	} else if(!str_cmp(buf,"highroom")) {

		// Format: highroom <wilds-uid> <x> <y> <z> <ground>
		WILDS_DATA *wilds;
		int x, y, z;
		bool ground;

		if(arg.type == ENT_NUMBER) {
			wilds = get_wilds_from_uid(NULL,arg.d.num);
			if(wilds) {
				if(!(rest = expand_argument(info,rest,&arg)) && arg.type != ENT_NUMBER) return;
				x = arg.d.num;
				if(x < 0 || x >= wilds->map_size_x) return;

				if(!(rest = expand_argument(info,rest,&arg)) && arg.type != ENT_NUMBER) return;
				y = arg.d.num;
				if(y < 0 || x >= wilds->map_size_y) return;

				if(!(rest = expand_argument(info,rest,&arg)) && arg.type != ENT_NUMBER) return;
				z = arg.d.num;

				if(!(rest = expand_argument(info,rest,&arg)) && arg.type != ENT_STRING) return;
				ground = !str_cmp(arg.d.str,"ground") || !str_cmp(arg.d.str,"true");

				variables_set_room(info->var,name,wilds_seek_down(wilds, x, y, z, ground));
			}
		}

	// Format: EXIT <STRING> - finds the exit at the given direction in the current room
	// Format: EXIT <ROOM> <STRING> - same as EXIT <STRING> but at the given room
	// Format: EXIT <EXIT> - explicit exit
	} else if(!str_cmp(buf,"exit")) {
		switch(arg.type) {
		case ENT_ROOM:
			here = arg.d.room;
			if(!here || !expand_argument(info,rest,&arg))
				return;
			if(arg.type != ENT_STRING) return;
		case ENT_STRING:
			vnum = get_num_dir(arg.d.str);
			if(vnum < 0) {
				if(!str_cmp(arg.d.str,"random"))
					vnum = number_range(0,MAX_DIR-1);
				else if(!str_cmp(arg.d.str,"exists")) {
					for(vnum = number_range(0,MAX_DIR-1), i = 0; i < MAX_DIR && !here->exit[vnum]; i++, vnum = (vnum+1)%MAX_DIR);

					if(!here->exit[vnum]) vnum = -1;
				} else if(!str_cmp(arg.d.str,"open")) {
					for(vnum = number_range(0,MAX_DIR-1), i = 0; i < MAX_DIR && !here->exit[vnum]; i++, vnum = (vnum+1)%MAX_DIR);

					if(!here->exit[vnum] || IS_SET(here->exit[vnum]->exit_info,EX_CLOSED)) vnum = -1;
				}

				if(vnum < 0)
					return;
			}
			ex = here->exit[vnum];
			break;
		case ENT_EXIT:
			ex = arg.d.door.r ? arg.d.door.r->exit[arg.d.door.door] : NULL;
			break;
		}
		variables_set_exit(vars,name,ex);

	// Format: MOBILE <ROOM VNUM or ROOM or MOBLIST> <VNUM or NAME>
	// Format: MOBILE <NAME>
	// Format: MOBILE <MOBILE>
	} else if(!str_cmp(buf,"mobile")) {
		switch(arg.type) {
		case ENT_NUMBER:
			here = get_room_index(arg.d.num);
			mobs = here ? here->people : NULL;
			break;
		case ENT_STRING:
			if(is_number(arg.d.str)) {
				here = get_room_index(atoi(arg.d.str));
				mobs = here ? here->people : NULL;
			} else
				vch = get_char_world(viewer,arg.d.str);
			break;
		case ENT_MOBILE:
			vch = arg.d.mob;
			break;
		case ENT_OLLIST_MOB:
			mobs = arg.d.list.ptr.mob ? *arg.d.list.ptr.mob : NULL;
			break;
		case ENT_ROOM:
			mobs = arg.d.room ? arg.d.room->people : NULL;
			break;
		default: return;
		}

		if(mobs) {
			if(!expand_argument(info,rest,&arg))
				return;
			if(arg.type == ENT_NUMBER)
				vnum = arg.d.num;
			else if(arg.type == ENT_STRING) {
				if(is_number(arg.d.str))
					vnum = atoi(arg.d.str);
				else
					str = arg.d.str;
			} else
				return;

			vch = script_get_char_list(mobs, viewer, FALSE, vnum, str);
		}
		variables_set_mobile(vars,name,vch);

	// Format: PLAYER <ROOM VNUM or ROOM or MOBLIST> <NAME>
	// Format: PLAYER <NAME>
	// Format: PLAYER <PLAYER>
	} else if(!str_cmp(buf,"player")) {
		switch(arg.type) {
		case ENT_STRING:
			vch = get_player(arg.d.str);
			break;
		case ENT_MOBILE:
			vch = arg.d.mob && !IS_NPC(arg.d.mob) ? arg.d.mob : NULL;
			break;
		case ENT_OLLIST_MOB:
			mobs = arg.d.list.ptr.mob ? *arg.d.list.ptr.mob : NULL;
			if(mobs) {
				if(!expand_argument(info,rest,&arg))
					return;
				if(arg.type == ENT_STRING && !is_number(arg.d.str))
					str = arg.d.str;
				else
					mobs = NULL;
			}
			break;
		default: return;
		}
		if(mobs) vch = script_get_char_list(mobs, viewer, TRUE, 0, str);
		variables_set_mobile(vars,name,vch);

	// Format: OBJECT <ROOM VNUM or ROOM or MOBILE or OBJECT or OBJLIST> <VNUM or NAME>
	// Format: OBJECT <NAME>
	} else if(!str_cmp(buf,"object")) {
		switch(arg.type) {
		case ENT_NUMBER:
			here = get_room_index(arg.d.num);
			objs = here ? here->contents : NULL;
			break;
		case ENT_STRING:
			if(is_number(arg.d.str)) {
				here = get_room_index(atoi(arg.d.str));
				objs = here ? here->contents : NULL;
			} else if(viewer)
				obj = get_obj_here(viewer,NULL,arg.d.str);
			else
				obj = get_obj_here(NULL,here,arg.d.str);
			break;
		case ENT_OBJECT:
			obj = arg.d.obj;
			break;
		case ENT_MOBILE:
			objs = arg.d.mob ? arg.d.mob->carrying : NULL;
			break;
		case ENT_OLLIST_OBJ:
			objs = arg.d.list.ptr.obj ? *arg.d.list.ptr.obj : NULL;
			break;
		case ENT_ROOM:
			objs = arg.d.room ? arg.d.room->contents : NULL;
			break;
		default: return;
		}

		if(objs) {
			if(!expand_argument(info,rest,&arg))
				return;
			if(arg.type == ENT_NUMBER)
				vnum = arg.d.num;
			else if(arg.type == ENT_STRING) {
				if(is_number(arg.d.str))
					vnum = atoi(arg.d.str);
				else
					str = arg.d.str;
			} else
				return;
			obj = script_get_obj_list(objs, viewer, 0, vnum, str);
		}
		variables_set_object(vars,name,obj);

	// Format: CARRY <VNUM or NAME>
	// Format: CARRY <MOBILE> <VNUM or NAME>
	// Format: CARRY <OBJLIST> <VNUM or NAME>
	} else if(!str_cmp(buf,"carry")) {
		switch(arg.type) {
		case ENT_NUMBER:
			vnum = arg.d.num;
			objs = viewer ? viewer->carrying : NULL;
			break;
		case ENT_STRING:
			if(is_number(arg.d.str))
				vnum = atoi(arg.d.str);
			else
				str = arg.d.str;
			objs = viewer ? viewer->carrying : NULL;
			break;
		case ENT_MOBILE:
			objs = arg.d.mob ? arg.d.mob->carrying : NULL;
			break;
		case ENT_OLLIST_OBJ:
			objs = arg.d.list.ptr.obj ? *arg.d.list.ptr.obj : NULL;
			break;
		default: return;
		}

		if(objs && arg.type != ENT_NUMBER && arg.type != ENT_STRING) {
			if(!expand_argument(info,rest,&arg))
				return;
			if(arg.type == ENT_NUMBER)
				vnum = arg.d.num;
			else if(arg.type == ENT_STRING) {
				if(is_number(arg.d.str))
					vnum = atoi(arg.d.str);
				else
					str = arg.d.str;
			} else
				return;
		}
		obj = script_get_obj_list(objs, viewer, 2, vnum, str);
		if(obj) variables_set_object(vars,name,obj);

	// Format: WORN <VNUM or NAME>
	// Format: WORN <MOBILE or OBJLIST> <VNUM or NAME>
	} else if(!str_cmp(buf,"worn")) {
		switch(arg.type) {
		case ENT_NUMBER:
			vnum = arg.d.num;
			objs = viewer ? viewer->carrying : NULL;
			break;
		case ENT_STRING:
			if(is_number(arg.d.str))
				vnum = atoi(arg.d.str);
			else
				str = arg.d.str;
			objs = viewer ? viewer->carrying : NULL;
			break;
		case ENT_MOBILE:
			objs = arg.d.mob ? arg.d.mob->carrying : NULL;
			break;
		case ENT_OLLIST_OBJ:
			objs = arg.d.list.ptr.obj ? *arg.d.list.ptr.obj : NULL;
			break;
		default: return;
		}

		if(objs && arg.type != ENT_NUMBER && arg.type != ENT_STRING) {
			if(!expand_argument(info,rest,&arg))
				return;
			if(arg.type == ENT_NUMBER)
				vnum = arg.d.num;
			else if(arg.type == ENT_STRING) {
				if(is_number(arg.d.str))
					vnum = atoi(arg.d.str);
				else
					str = arg.d.str;
			} else
				return;
		}
		obj = script_get_obj_list(objs, viewer, 1, vnum, str);
		variables_set_object(vars,name,obj);

	// Format: CONTENT <OBJECT or OBJLIST> <VNUM or NAME>
	} else if(!str_cmp(buf,"content")) {
		switch(arg.type) {
		case ENT_OLLIST_OBJ:
			objs = arg.d.list.ptr.obj ? *arg.d.list.ptr.obj : NULL;
			break;
		case ENT_OBJECT:
			objs = arg.d.obj ? arg.d.obj->contains : NULL;
			break;
		default: return;
		}

		if(objs) {
			if(!expand_argument(info,rest,&arg))
				return;
			if(arg.type == ENT_NUMBER)
				vnum = arg.d.num;
			else if(arg.type == ENT_STRING) {
				if(is_number(arg.d.str))
					vnum = atoi(arg.d.str);
				else
					str = arg.d.str;
			} else
				return;
			obj = script_get_obj_list(objs, info->mob, 0, vnum, str);
		}
		variables_set_object(vars,name,obj);

	// Format: TOKEN <MOBILE or OBJECT or ROOM or TOKLIST> <Pattern>
	// Format: TOKEN <TOKEN>
	} else if(!str_cmp(buf,"token")) {
		switch(arg.type) {
		case ENT_MOBILE:   tokens = arg.d.mob ? arg.d.mob->tokens : NULL; break;
		case ENT_OBJECT:   tokens = arg.d.obj ? arg.d.obj->tokens : NULL; break;
		case ENT_ROOM:     tokens = arg.d.room ? arg.d.room->tokens : NULL; break;
		case ENT_TOKEN:    token = arg.d.token; break;
		case ENT_OLLIST_TOK: tokens = arg.d.list.ptr.tok ? *arg.d.list.ptr.tok : NULL; break;
		default: return;
		}

		if(tokens) token = token_find_match(info,tokens, rest);
		variables_set_token(vars,name,token);

	// Format: IDMOBILE <IDa> <IDb>
	} else if(!str_cmp(buf,"idmobile")) {
		switch(arg.type) {
		case ENT_NUMBER:
			id1 = arg.d.num;
			if(!expand_argument(info,rest,&arg) || arg.type != ENT_NUMBER)
				return;
			vch = idfind_mobile(id1,arg.d.num);
			break;
		default: return;
		}
		variables_set_mobile(vars,name,vch);

	// Format: IDOBJECT <IDa> <IDb>
	} else if(!str_cmp(buf,"idobject")) {
		switch(arg.type) {
		case ENT_NUMBER:
			id1 = arg.d.num;
			if(!expand_argument(info,rest,&arg) || arg.type != ENT_NUMBER)
				return;
			obj = idfind_object(id1,arg.d.num);
			break;
		default: return;
		}
		variables_set_object(vars,name,obj);

	// Format: IDPLAYER <IDa> <IDb>
	} else if(!str_cmp(buf,"idplayer")) {
		switch(arg.type) {
		case ENT_NUMBER:
			id1 = arg.d.num;
			if(!expand_argument(info,rest,&arg) || arg.type != ENT_NUMBER)
				return;
			vch = idfind_player(id1,arg.d.num);
			break;
		default: return;
		}
		variables_set_mobile(vars,name,vch);

	// Format: CROOM <ROOM VNUM> <IDa> <IDb>
	} else if(!str_cmp(buf,"croom")) {
		switch(arg.type) {
		case ENT_NUMBER:
			here = get_room_index(arg.d.num);
			if(!(rest = expand_argument(info,rest,&arg)) || arg.type != ENT_NUMBER)
				return;
			id1 = arg.d.num;
			if(!expand_argument(info,rest,&arg) || arg.type != ENT_NUMBER)
				return;
			variables_set_room(vars,name,get_clone_room(here,id1,arg.d.num));
			break;
		default: return;
		}

	// Format: SKILL <NAME>
	} else if(!str_cmp(buf,"skill")) {
		switch(arg.type) {
		case ENT_STRING:
			variables_set_skill(vars,name,skill_lookup(arg.d.str));
			break;
		default: return;
		}

	// Format: SKILLINFO <MOBILE> <NAME or TOKEN>
	} else if(!str_cmp(buf,"skillinfo")) {
		switch(arg.type) {
		case ENT_MOBILE:
			vch = arg.d.mob;
			if(!expand_argument(info,rest,&arg))
				return;

			if( arg.type == ENT_STRING )
				variables_set_skillinfo(vars,name,vch,skill_lookup(arg.d.str), NULL);
			else if( arg.type == ENT_TOKEN )
				variables_set_skillinfo(vars,name,vch, 0, arg.d.token);
			break;
		default: return;
		}

	// Format: FINDPATH <ROOM> <ROOM> <DEPTH> <IN-ZONE> <DOORS> - returns the EXIT entity
	} else if(!str_cmp(buf,"findpath")) {
		ROOM_INDEX_DATA *start_room = NULL, *end_room = NULL;
		int depth, in_zone, doors;
		int dir;

		switch(arg.type) {
		case ENT_NUMBER:	start_room = get_room_index(arg.d.num); break;
		case ENT_ROOM:		start_room = arg.d.room; break;
		case ENT_EXIT:		start_room = arg.d.door.r ? exit_destination(arg.d.door.r->exit[arg.d.door.door]) : NULL; break;
		}

		if(!start_room || !(rest = expand_argument(info,rest,&arg)))
			return;

		switch(arg.type) {
		case ENT_NUMBER:	end_room = get_room_index(arg.d.num); break;
		case ENT_ROOM:		end_room = arg.d.room; break;
		case ENT_EXIT:		end_room = arg.d.door.r ? exit_destination(arg.d.door.r->exit[arg.d.door.door]) : NULL; break;
		}

		if(!end_room || !(rest = expand_argument(info,rest,&arg)) || arg.type != ENT_NUMBER)
			return;

		depth = arg.d.num;

		if(!(rest = expand_argument(info,rest,&arg)) || arg.type != ENT_STRING)
			return;

		in_zone = !str_cmp(arg.d.str,"true") || !str_cmp(arg.d.str, "yes") || !str_cmp(arg.d.str, "local");

		if(!(rest = expand_argument(info,rest,&arg)) || arg.type != ENT_STRING)
			return;

		doors = !str_cmp(arg.d.str,"true") || !str_cmp(arg.d.str, "yes") || !str_cmp(arg.d.str, "doors");

		dir = find_path(start_room->vnum, end_room->vnum, NULL, (doors ? -depth : depth), in_zone);
		if( dir < 0 || dir >= MAX_DIR )
			variables_set_exit(vars,name,NULL);
		else
			variables_set_exit(vars,name,start_room->exit[dir]);

	} else
		return;
}

bool valid_spell_token( TOKEN_DATA *token )
{
	ITERATOR pit;
	PROG_LIST *prg = NULL;

	if( IS_VALID(token) && token->pIndexData->progs ) {
		iterator_start(&pit, token->pIndexData->progs[TRIGSLOT_SPELL]);
		while((prg = (PROG_LIST *)iterator_nextdata(&pit)))
			if(prg->trig_type == TRIG_SPELL)
				break;
		iterator_stop(&pit);
	}

	return prg && TRUE;
}

bool visit_script_execute(ROOM_INDEX_DATA *room, void *argv[], int argc, int depth, int door)
{
	int ret;
	SCRIPT_DATA *script = (SCRIPT_DATA *)argv[0];

	ret = execute_script(script->vnum, script, NULL, NULL, room, NULL,
		(CHAR_DATA *)argv[1],	// enactor
		(OBJ_DATA *)argv[2],	// object 1
		(OBJ_DATA *)argv[3],	// object 2
		(CHAR_DATA *)argv[4],	// victim 1
		(CHAR_DATA *)argv[5],	// victim 2
		NULL,					// no random
		(char *)argv[6],		// phrase
		NULL,					// no trigger
		depth,					// register 1
		door,					// register 2
		0,						// register 3
		0,						// register 4
		0);						// register 5

	return ret > 0;
}

