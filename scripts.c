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

//void compile_error_show(char *msg);
const ENT_FIELD *entity_type_lookup(char *name,const ENT_FIELD *list)
{
	int i;
//	char buf[MSL];

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
	(ifc->func)(info->mob,info->obj,info->room,info->token,ret,i,argv);
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
	char *str1,*str2;
	ROOM_INDEX_DATA *here;
	EXIT_DATA *ex;
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

		// Get the LIST
		if(!(str2 = expand_argument(&block->info,str1,&arg))) {
			block->ret_val = PRET_BADSYNTAX;
			return FALSE;
		}


		switch(arg.type) {
		case ENT_EXIT:
			if(!arg.d.exit)
				return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);
			block->loops[lp].d.l.type = ENT_EXIT;
			block->loops[lp].d.l.cur.x = arg.d.exit;
			block->loops[lp].d.l.next.x = NULL;
			block->loops[lp].d.l.owner = arg.d.exit ? arg.d.exit->from_room : NULL;
			if(arg.d.exit) {
				here = arg.d.exit->from_room;
				for(i=arg.d.exit->orig_door + 1; i < MAX_DIR && !here->exit[i]; i++);
				if(i < MAX_DIR)
					block->loops[lp].d.l.next.x = here->exit[i];
			}
			// Set the variable
			variables_set_exit(block->info.var,block->loops[lp].var_name,arg.d.exit);
			break;

		case ENT_LIST_MOB:
			if(!arg.d.list.ptr.mob || !*arg.d.list.ptr.mob)
				return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);

			block->loops[lp].d.l.type = ENT_MOBILE;
			block->loops[lp].d.l.cur.m = *arg.d.list.ptr.mob;
			block->loops[lp].d.l.next.m = block->loops[lp].d.l.cur.m->next_in_room;
			block->loops[lp].d.l.owner = arg.d.list.owner;
			// Set the variable
			variables_set_mobile(block->info.var,block->loops[lp].var_name,*arg.d.list.ptr.mob);
			break;

		case ENT_LIST_OBJ:
			if(!arg.d.list.ptr.obj || !*arg.d.list.ptr.obj)
				return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);

			block->loops[lp].d.l.type = ENT_OBJECT;
			block->loops[lp].d.l.cur.o = *arg.d.list.ptr.obj;
			block->loops[lp].d.l.next.o = block->loops[lp].d.l.cur.o->next_content;
			block->loops[lp].d.l.owner = arg.d.list.owner;
			// Set the variable
			variables_set_object(block->info.var,block->loops[lp].var_name,*arg.d.list.ptr.obj);
			break;

		case ENT_LIST_TOK:
			if(!arg.d.list.ptr.tok || !*arg.d.list.ptr.tok)
				return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);

			block->loops[lp].d.l.type = ENT_TOKEN;
			block->loops[lp].d.l.cur.t = *arg.d.list.ptr.tok;
			block->loops[lp].d.l.next.t = block->loops[lp].d.l.cur.t->next;
			block->loops[lp].d.l.owner = arg.d.list.owner;
			// Set the variable
			variables_set_token(block->info.var,block->loops[lp].var_name,*arg.d.list.ptr.tok);
			break;

		case ENT_LIST_AFF:
			if(!arg.d.list.ptr.aff || !*arg.d.list.ptr.aff)
				return opc_skip_to_label(block,OP_ENDLIST,block->cur_line->label,TRUE);

			block->loops[lp].d.l.type = ENT_AFFECT;
			block->loops[lp].d.l.cur.aff = *arg.d.list.ptr.aff;
			block->loops[lp].d.l.next.aff = block->loops[lp].d.l.cur.aff->next;
			block->loops[lp].d.l.owner = arg.d.list.owner;
			// Set the variable
			variables_set_affect(block->info.var,block->loops[lp].var_name,*arg.d.list.ptr.aff);
			break;


		default:
			block->ret_val = PRET_BADSYNTAX;
			return FALSE;
		}

		block->loops[lp].id = block->cur_line->label;
		block->loop++;
		block->cond[block->cur_line->level] = TRUE;
	} else {
		// Continue loop
		switch(block->loops[lp].d.l.type) {
		case ENT_EXIT:
			block->loops[lp].d.l.cur.x = ex = block->loops[lp].d.l.next.x;

			// Set the variable
			variables_set_exit(block->info.var,block->loops[lp].var_name,ex);

			if(!ex) {
				skip = TRUE;
				break;
			}

			here = ex->from_room;
			for(i=ex->orig_door + 1; i < MAX_DIR && !here->exit[i]; i++);
			if(i < MAX_DIR) block->loops[lp].d.l.next.x = here->exit[i];
			break;

		case ENT_MOBILE:
			block->loops[lp].d.l.cur.m = block->loops[lp].d.l.next.m;
			// Set the variable
			variables_set_mobile(block->info.var,block->loops[lp].var_name,block->loops[lp].d.l.cur.m);

			if(!block->loops[lp].d.l.cur.m) {
				skip = TRUE;
				break;
			}

			block->loops[lp].d.l.next.m = block->loops[lp].d.l.cur.m->next_in_room;
			break;

		case ENT_OBJECT:
			block->loops[lp].d.l.cur.o = block->loops[lp].d.l.next.o;
			// Set the variable
			variables_set_object(block->info.var,block->loops[lp].var_name,block->loops[lp].d.l.cur.o);

			if(!block->loops[lp].d.l.cur.o) {
				skip = TRUE;
				break;
			}

			block->loops[lp].d.l.next.o = block->loops[lp].d.l.cur.o->next_content;
			break;

		case ENT_TOKEN:
			block->loops[lp].d.l.cur.t = block->loops[lp].d.l.next.t;
			// Set the variable
			variables_set_token(block->info.var,block->loops[lp].var_name,block->loops[lp].d.l.cur.t);

			if(!block->loops[lp].d.l.cur.t) {
				skip = TRUE;
				break;
			}

			block->loops[lp].d.l.next.t = block->loops[lp].d.l.cur.t->next;
			break;

		case ENT_AFFECT:
			block->loops[lp].d.l.cur.aff = block->loops[lp].d.l.next.aff;
			// Set the variable
			variables_set_affect(block->info.var,block->loops[lp].var_name,block->loops[lp].d.l.cur.aff);

			if(!block->loops[lp].d.l.cur.aff) {
				skip = TRUE;
				break;
			}

			block->loops[lp].d.l.next.aff = block->loops[lp].d.l.cur.aff->next;
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
	OBJ_DATA *obj1,OBJ_DATA *obj2,CHAR_DATA *vch,CHAR_DATA *rch,
	char *phrase, char *trigger)
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
	if (IS_SET(script->flags,SCRIPT_DISABLED))
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
	block.info.rch = rch;
	block.script = script;

	saved_security = script_security;
//	if(script_security == INIT_SCRIPT_SECURITY)
//		script_security = (port == PORT_NORMAL) ? script->security : MAX_SCRIPT_SECURITY;
	if(script_security == INIT_SCRIPT_SECURITY ||
		(IS_SET(script->flags,SCRIPT_SECURED) && (script->security > script_security)) ||
		script->security < script_security)
		script_security = script->security;

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
	int i, nth = 1, vnum = 0;
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
		if(tokens->pIndexData->vnum == vnum &&
			(!match[0] || tokens->value[0] == values[0]) &&
			(!match[1] || tokens->value[1] == values[1]) &&
			(!match[2] || tokens->value[2] == values[2]) &&
			(!match[3] || tokens->value[3] == values[3]) &&
			(!--nth))
			break;
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
	// Prevent a recursive loop with cloned rooms
	if (recursive_environment(room, ch, NULL, NULL)) {
		bug("do_mob_transfer - Selected location would cause an infinite environment loop.", 0);
		return;
	}

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


bool has_trigger(PROG_LIST **bank, int trigger)
{
	int slot;
	PROG_LIST *prog;

//	DBG2ENTRY2(PTR,bank,NUM,trigger);

//	DBG3MSG3("trigger = %d, name = '%s', slot = %d\n",trigger, trigger_table[trigger].name,trigger_slots[trigger]);

	slot = trigger_slots[trigger];

	if(bank) {
		for (prog = bank[slot]; prog; prog = prog->next)
			if (is_trigger_type(prog->trig_type,trigger)) {
//				DBG2EXITVALUE2(TRUE);
				return TRUE;
			}
	}

//	DBG2EXITVALUE2(FALSE);
	return FALSE;
}

int trigger_index(char *name, int type)
{
	int i;
//	char buf[MSL];

	for (i = 0; trigger_table[i].name; i++) {
//		sprintf(buf,"trigger[%d]: '%s', '%s', %d, %d, %d, %d, %d",
//			i,trigger_table[i].name,name,type,
//			trigger_table[i].mob,
//			trigger_table[i].obj,
//			trigger_table[i].room,
//			trigger_table[i].token);
//		log_string(buf);

		if (!str_cmp(trigger_table[i].name, name))
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

//	plogf("is_trigger_type: %d, %s, %d, %d\n\r", tindex, trigger_table[tindex].name, trigger_table[tindex].value, type);

	return (trigger_table[tindex].value == type);
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
		if(trigger_table[type].value == TRIG_SPELLCAST) {
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


/*
 * A general purpose string trigger. Matches argument to a string trigger
 * phrase.
 */
int p_act_trigger(char *argument, CHAR_DATA *mob, OBJ_DATA *obj, ROOM_INDEX_DATA *room,
	CHAR_DATA *ch, const void *arg1, const void *arg2, int type)
{
	PROG_LIST *prg;
	TOKEN_DATA *token, *tnext;
	char string1[MSL];
	char string2[MSL];
	int slot;
	int ret_val = PRET_NOSCRIPT, ret; // @@@NIB Default for a trigger loop is NO SCRIPT

//	DBG2ENTRY8(STR,argument,PTR,mob,PTR,obj,PTR,room,PTR,ch,PTR,arg1,PTR,arg2,NUM,type);

	if ((mob && obj) || (mob && room) || (obj && room)) {
		bug("Multiple program types in ACT trigger.", 0);
		PRETURN;
	}

	slot = trigger_slots[type];
	str_lower(argument,string1);

//	sprintf(buf,"ACT(%s,%d): %s",trigger_table[type].name, slot, argument);
//	wiznet(buf, NULL, NULL,WIZ_SCRIPTS,0,0);


	if (mob) {
		// Check for tokens FIRST
		for(token = mob->tokens; token; token = tnext) {
			tnext = token->next;
			if(token->pIndexData->progs) {
				script_destructed = FALSE;
				for (prg = token->pIndexData->progs[slot]; prg && !script_destructed; prg = prg->next) {
					if (is_trigger_type(prg->trig_type,type)) {
						str_lower(prg->trig_phrase,string2);
						if (strstr(string1, string2)) {
							ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, ch, (void *)arg1, (void *)arg2, (void *)arg2, NULL,argument,prg->trig_phrase);
							SETPRETX;
						}
					}
				}
				// Since the token didn't destruct, this is valid
				if(!script_destructed) tnext = token->next;
			}
		}

		if(IS_NPC(mob) && mob->pIndexData->progs) {
			script_destructed = FALSE;
			for (prg = mob->pIndexData->progs[slot]; prg && !script_destructed; prg = prg->next) {
				if (is_trigger_type(prg->trig_type,type)) {
					str_lower(prg->trig_phrase,string2);
					if (strstr(string1, string2)) {
						ret = execute_script(prg->vnum, prg->script, mob, NULL, NULL, NULL, ch, (void *)arg1, (void *)arg2, (void *)arg2, NULL,argument,prg->trig_phrase);
						SETPRETX;
					}
				}
			}
		}
	} else if (obj) {
		if(obj_room(obj) && obj->pIndexData->progs) {
			script_destructed = FALSE;
			for (prg = obj->pIndexData->progs[slot]; prg && !script_destructed; prg = prg->next) {
				if (is_trigger_type(prg->trig_type,type)) {
					str_lower(prg->trig_phrase,string2);
					if (strstr(string1, string2)) {
						ret = execute_script(prg->vnum, prg->script, NULL, obj, NULL, NULL, ch, (void *)arg1, (void *)arg2, (void *)arg2, NULL,argument,prg->trig_phrase);
						SETPRETX;
					}
				}
			}
		}
	} else if (room) {
		if (room->source) {
			if(room->source->progs->progs) {
				script_destructed = FALSE;
				for (prg = room->source->progs->progs[slot]; prg && !script_destructed; prg = prg->next) {
					if (is_trigger_type(prg->trig_type,type)) {
						str_lower(prg->trig_phrase,string2);
						if (strstr(string1, string2)) {
							ret = execute_script(prg->vnum, prg->script, NULL, NULL, room, NULL, ch, (void *)arg1, (void *)arg2, (void *)arg2, NULL,argument,prg->trig_phrase);
							SETPRETX; // @@@NIB
						}
					}
				}
			}
		} else if(room->progs->progs) {
			script_destructed = FALSE;
			for (prg = room->progs->progs[slot]; prg && !script_destructed; prg = prg->next) {
				if (is_trigger_type(prg->trig_type,type)) {
					str_lower(prg->trig_phrase,string2);
					if (strstr(string1, string2)) {
						ret = execute_script(prg->vnum, prg->script, NULL, NULL, room, NULL, ch, (void *)arg1, (void *)arg2, (void *)arg2, NULL,argument,prg->trig_phrase);
						SETPRETX; // @@@NIB
					}
				}
			}
		}
	} else
		bug("ACT trigger with no program type.", 0);
	PRETURN;
}



// Similar to p_act_trigger, except it does EXACT match
int p_exact_trigger(char *argument, CHAR_DATA *mob, OBJ_DATA *obj, ROOM_INDEX_DATA *room,
	CHAR_DATA *ch, const void *arg1, const void *arg2, int type)
{
	PROG_LIST *prg;
	TOKEN_DATA *token, *tnext;
	int slot;
	int ret_val = PRET_NOSCRIPT, ret; // @@@NIB Default for a trigger loop is NO SCRIPT

	if ((mob && obj) || (mob && room) || (obj && room)) {
		bug("Multiple program types in EXACT trigger.", 0);
		PRETURN;
	}

	slot = trigger_slots[type];

	if (mob) {
		// Check for tokens FIRST
		for(token = mob->tokens; token; token = tnext) {
			tnext = token->next;
			if(token->pIndexData->progs) {
				script_destructed = FALSE;
				for (prg = token->pIndexData->progs[slot]; prg && !script_destructed; prg = prg->next) {
					if (is_trigger_type(prg->trig_type,type)) {
						if (!str_cmp(argument, prg->trig_phrase)) {
							ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, ch, (void *)arg1, (void *)arg2, (void *)arg2, NULL,argument,prg->trig_phrase);
							SETPRETX;
						}
					}
				}
				// Since the token didn't destruct, this is valid
				if(!script_destructed) tnext = token->next;
			}
		}

		if(IS_NPC(mob) && mob->pIndexData->progs) {
			script_destructed = FALSE;
			for (prg = mob->pIndexData->progs[slot]; prg && !script_destructed; prg = prg->next) {
				if (is_trigger_type(prg->trig_type,type)) {
					if (!str_cmp(argument, prg->trig_phrase)) {
						ret = execute_script(prg->vnum, prg->script, mob, NULL, NULL, NULL, ch, (void *)arg1, (void *)arg2, (void *)arg2, NULL,argument,prg->trig_phrase);
						SETPRETX;
					}
				}
			}
		}
	} else if (obj) {
		if(obj_room(obj) && obj->pIndexData->progs) {
			script_destructed = FALSE;
			for (prg = obj->pIndexData->progs[slot]; prg && !script_destructed; prg = prg->next) {
				if (is_trigger_type(prg->trig_type,type)) {
					if (!str_cmp(argument, prg->trig_phrase)) {
						ret = execute_script(prg->vnum, prg->script, NULL, obj, NULL, NULL, ch, (void *)arg1, (void *)arg2, (void *)arg2, NULL,argument,prg->trig_phrase);
						SETPRETX;
					}
				}
			}
		}
	} else if (room) {
		if (room->source) {
			if(room->source->progs->progs) {
				script_destructed = FALSE;
				for (prg = room->source->progs->progs[slot]; prg && !script_destructed; prg = prg->next) {
					if (is_trigger_type(prg->trig_type,type)) {
						if (!str_cmp(argument, prg->trig_phrase)) {
							ret = execute_script(prg->vnum, prg->script, NULL, NULL, room, NULL, ch, (void *)arg1, (void *)arg2, (void *)arg2, NULL,argument,prg->trig_phrase);
							SETPRETX; // @@@NIB
						}
					}
				}
			}
		} else if(room->progs->progs) {
			script_destructed = FALSE;
			for (prg = room->progs->progs[slot]; prg && !script_destructed; prg = prg->next) {
				if (is_trigger_type(prg->trig_type,type)) {
					if (!str_cmp(argument, prg->trig_phrase)) {
						ret = execute_script(prg->vnum, prg->script, NULL, NULL, room, NULL, ch, (void *)arg1, (void *)arg2, (void *)arg2, NULL,argument,prg->trig_phrase);
						SETPRETX; // @@@NIB
					}
				}
			}
		}
	} else
		bug("EXACT trigger with no program type.", 0);
	PRETURN;
}

// Similar to p_act_trigger, except it uses is_name
int p_name_trigger(char *argument, CHAR_DATA *mob, OBJ_DATA *obj, ROOM_INDEX_DATA *room,
	CHAR_DATA *ch, const void *arg1, const void *arg2, int type)
{
	PROG_LIST *prg;
	TOKEN_DATA *token, *tnext;
	int slot;
	int ret_val = PRET_NOSCRIPT, ret; // @@@NIB Default for a trigger loop is NO SCRIPT

	if ((mob && obj) || (mob && room) || (obj && room)) {
		bug("Multiple program types in EXACT trigger.", 0);
		PRETURN;
	}

	slot = trigger_slots[type];

	if (mob) {
		// Check for tokens FIRST
		for(token = mob->tokens; token; token = tnext) {
			tnext = token->next;
			if(token->pIndexData->progs) {
				script_destructed = FALSE;
				for (prg = token->pIndexData->progs[slot]; prg && !script_destructed; prg = prg->next) {
					if (is_trigger_type(prg->trig_type,type)) {
						if (is_name(argument, prg->trig_phrase)) {
							ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, ch, (void *)arg1, (void *)arg2, (void *)arg2, NULL,argument,prg->trig_phrase);
							SETPRETX;
						}
					}
				}
				// Since the token didn't destruct, this is valid
				if(!script_destructed) tnext = token->next;
			}
		}

		if(IS_NPC(mob) && mob->pIndexData->progs) {
			script_destructed = FALSE;
			for (prg = mob->pIndexData->progs[slot]; prg && !script_destructed; prg = prg->next) {
				if (is_trigger_type(prg->trig_type,type)) {
					if (is_name(argument, prg->trig_phrase)) {
						ret = execute_script(prg->vnum, prg->script, mob, NULL, NULL, NULL, ch, (void *)arg1, (void *)arg2, (void *)arg2, NULL,argument,prg->trig_phrase);
						SETPRETX;
					}
				}
			}
		}
	} else if (obj) {
		if(obj_room(obj) && obj->pIndexData->progs) {
			script_destructed = FALSE;
			for (prg = obj->pIndexData->progs[slot]; prg && !script_destructed; prg = prg->next) {
				if (is_trigger_type(prg->trig_type,type)) {
					if (is_name(argument, prg->trig_phrase)) {
						ret = execute_script(prg->vnum, prg->script, NULL, obj, NULL, NULL, ch, (void *)arg1, (void *)arg2, (void *)arg2, NULL,argument,prg->trig_phrase);
						SETPRETX;
					}
				}
			}
		}
	} else if (room) {
		if (room->source) {
			if(room->source->progs->progs) {
				script_destructed = FALSE;
				for (prg = room->source->progs->progs[slot]; prg && !script_destructed; prg = prg->next) {
					if (is_trigger_type(prg->trig_type,type)) {
						if (is_name(argument, prg->trig_phrase)) {
							ret = execute_script(prg->vnum, prg->script, NULL, NULL, room, NULL, ch, (void *)arg1, (void *)arg2, (void *)arg2, NULL,argument,prg->trig_phrase);
							SETPRETX; // @@@NIB
						}
					}
				}
			}
		} else if(room->progs->progs) {
			script_destructed = FALSE;
			for (prg = room->progs->progs[slot]; prg && !script_destructed; prg = prg->next) {
				if (is_trigger_type(prg->trig_type,type)) {
					if (is_name(argument, prg->trig_phrase)) {
						ret = execute_script(prg->vnum, prg->script, NULL, NULL, room, NULL, ch, (void *)arg1, (void *)arg2, (void *)arg2, NULL,argument,prg->trig_phrase);
						SETPRETX; // @@@NIB
					}
				}
			}
		}
	} else
		bug("EXACT trigger with no program type.", 0);
	PRETURN;
}


/*
 * A general purpose percentage trigger. Checks if a random percentage
 * number is less than trigger phrase
 */
int p_percent_trigger_phrase(CHAR_DATA *mob, OBJ_DATA *obj, ROOM_INDEX_DATA *room,
	TOKEN_DATA *token, CHAR_DATA *ch, const void *arg1, const void *arg2, int type, char *phrase)
{
	PROG_LIST *prg;
	TOKEN_DATA *tnext;
	int slot;
	int ret_val = PRET_NOSCRIPT, ret; // @@@NIB Default for a trigger loop is NO SCRIPT


//	DBG2ENTRY8(PTR,mob,PTR,obj,PTR,room,PTR,token,PTR,ch,PTR,arg1,PTR,arg2,NUM,type);

	if ((mob && obj) || (mob && room) || (obj && room) ||
		(mob && token) || (obj && token) || (room && token)) {
		bug("Multiple program types in PERCENT trigger.", 0);
		return PRET_BADTYPE;
	}

//	DBG3MSG3("Trigger = %d(%s), Slot = %d\n", type, trigger_table[type].name, trigger_slots[type]);

	if(0 && ch && type == TRIG_PRESPELL) {
		char buf[MSL];

		sprintf(buf,"PRESPELL: %d, %d\n\r", TRIGSLOT_SPELL, trigger_slots[TRIG_PRESPELL]);
		send_to_char(buf,ch);
	}

	slot = trigger_slots[type];

	if (mob) {
		// Check for tokens FIRST
		for(token = mob->tokens; token; token = tnext) {
			tnext = token->next;
			if(token->pIndexData->progs) {
				script_destructed = FALSE;
				for (prg = token->pIndexData->progs[slot]; prg && !script_destructed; prg = prg->next) {
					if (is_trigger_type(prg->trig_type,type) && number_percent() < atoi(prg->trig_phrase)) {
						ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, ch, (void *)arg1, (void *)arg2, (void *)arg2, NULL,phrase,NULL);
						SETPRET;
					}
				}

				// Since the token didn't destruct, this is valid
				if(!script_destructed) tnext = token->next;
			}
		}


		if(IS_NPC(mob) && mob->pIndexData->progs) {
			script_destructed = FALSE;
			for (prg = mob->pIndexData->progs[slot]; prg && !script_destructed; prg = prg->next) {
				if (is_trigger_type(prg->trig_type,type) && number_percent() < atoi(prg->trig_phrase)) {
					ret = execute_script(prg->vnum, prg->script, mob, NULL, NULL, NULL, ch, (void *)arg1, (void *)arg2, (void *)arg2, NULL,phrase,NULL);
					SETPRET;
				}
			}
		}
	} else if (obj) {
		if(obj_room(obj) && obj->pIndexData->progs) {
			script_destructed = FALSE;
			for (prg = obj->pIndexData->progs[slot]; prg && !script_destructed; prg = prg->next) {
				if (is_trigger_type(prg->trig_type,type) && number_percent() < atoi(prg->trig_phrase)) {
					ret = execute_script(prg->vnum, prg->script, NULL, obj, NULL, NULL, ch, (void *)arg1, (void *)arg2, (void *)arg2, NULL,phrase,NULL);
					SETPRET;
				}
			}
		}
	} else if (room) {
		script_destructed = FALSE;
		if(room->source) {
			if (room->source->progs->progs) {
				for (prg = room->source->progs->progs[slot]; prg && !script_destructed; prg = prg->next) {
					if (is_trigger_type(prg->trig_type,type) && number_percent() < atoi(prg->trig_phrase)) {
						ret = execute_script(prg->vnum, prg->script, NULL, NULL, room, NULL, ch, (void *)arg1, (void *)arg2, (void *)arg2, NULL,phrase,NULL);
						SETPRET;
					}
				}
			}
		} else if(room->progs->progs) {
			for (prg = room->progs->progs[slot]; prg && !script_destructed; prg = prg->next) {
				if (is_trigger_type(prg->trig_type,type) && number_percent() < atoi(prg->trig_phrase)) {
					ret = execute_script(prg->vnum, prg->script, NULL, NULL, room, NULL, ch, (void *)arg1, (void *)arg2, (void *)arg2, NULL,phrase,NULL);
					SETPRET;
				}
			}
		}

	} else if(token) {
		if(token->pIndexData->progs && trigger_table[type].token) {
			script_destructed = FALSE;
			for (prg = token->pIndexData->progs[slot]; prg && !script_destructed; prg = prg->next) {
				if (is_trigger_type(prg->trig_type,type) && number_percent() < atoi(prg->trig_phrase)) {
					ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, ch, (void *)arg1, (void *)arg2, (void *)arg2, NULL,phrase,NULL);
					SETPRET;
				}
			}
		}
	} else
		bug("PERCENT trigger missing program type.", 0);

	PRETURN;
}


int p_number_trigger_phrase(CHAR_DATA *mob, OBJ_DATA *obj, ROOM_INDEX_DATA *room,
	TOKEN_DATA *token, CHAR_DATA *ch, const void *arg1, const void *arg2, int type, int number, char *phrase)
{
	PROG_LIST *prg;
	TOKEN_DATA *tnext;
	int slot;
	int ret_val = PRET_NOSCRIPT, ret; // @@@NIB Default for a trigger loop is NO SCRIPT


//	DBG2ENTRY8(PTR,mob,PTR,obj,PTR,room,PTR,token,PTR,ch,PTR,arg1,PTR,arg2,NUM,type);

	if ((mob && obj) || (mob && room) || (obj && room) ||
		(mob && token) || (obj && token) || (room && token)) {
		bug("Multiple program types in NUMBER trigger.", 0);
		return PRET_BADTYPE;
	}

//	DBG3MSG3("Trigger = %d(%s), Slot = %d\n", type, trigger_table[type].name, trigger_slots[type]);

	slot = trigger_slots[type];

	if (mob) {
		// Check for tokens FIRST
		for(token = mob->tokens; token; token = tnext) {
			tnext = token->next;
			if(token->pIndexData->progs) {
				script_destructed = FALSE;
				for (prg = token->pIndexData->progs[slot]; prg && !script_destructed; prg = prg->next) {
					if (is_trigger_type(prg->trig_type,type) && number == atoi(prg->trig_phrase)) {
						ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, ch, (void *)arg1, (void *)arg2, (void *)arg2, NULL,phrase,NULL);
						SETPRET;
					}
				}

				// Since the token didn't destruct, this is valid
				if(!script_destructed) tnext = token->next;
			}
		}


		if(IS_NPC(mob) && mob->pIndexData->progs) {
			script_destructed = FALSE;
			for (prg = mob->pIndexData->progs[slot]; prg && !script_destructed; prg = prg->next) {
				if (is_trigger_type(prg->trig_type,type) && number == atoi(prg->trig_phrase)) {
					ret = execute_script(prg->vnum, prg->script, mob, NULL, NULL, NULL, ch, (void *)arg1, (void *)arg2, (void *)arg2, NULL,phrase,NULL);
					SETPRET;
				}
			}
		}
	} else if (obj) {
		if(obj_room(obj) && obj->pIndexData->progs) {
			script_destructed = FALSE;
			for (prg = obj->pIndexData->progs[slot]; prg && !script_destructed; prg = prg->next) {
				if (is_trigger_type(prg->trig_type,type) && number == atoi(prg->trig_phrase)) {
					ret = execute_script(prg->vnum, prg->script, NULL, obj, NULL, NULL, ch, (void *)arg1, (void *)arg2, (void *)arg2, NULL,phrase,NULL);
					SETPRET;
				}
			}
		}
	} else if (room) {
		script_destructed = FALSE;
		if(room->source) {
			if(room->source->progs->progs) {
				for (prg = room->source->progs->progs[slot]; prg && !script_destructed; prg = prg->next) {
					if (is_trigger_type(prg->trig_type,type) && number == atoi(prg->trig_phrase)) {
						ret = execute_script(prg->vnum, prg->script, NULL, NULL, room, NULL, ch, (void *)arg1, (void *)arg2, (void *)arg2, NULL,phrase,NULL);
						SETPRET;
					}
				}
			}
		} else if(room->progs->progs) {
			for (prg = room->progs->progs[slot]; prg && !script_destructed; prg = prg->next) {
				if (is_trigger_type(prg->trig_type,type) && number == atoi(prg->trig_phrase)) {
					ret = execute_script(prg->vnum, prg->script, NULL, NULL, room, NULL, ch, (void *)arg1, (void *)arg2, (void *)arg2, NULL,phrase,NULL);
					SETPRET;
				}
			}
		}

	} else if(token) {
		if(token->pIndexData->progs && trigger_table[type].token) {
			script_destructed = FALSE;
			for (prg = token->pIndexData->progs[slot]; prg && !script_destructed; prg = prg->next) {
				if (is_trigger_type(prg->trig_type,type) && number == atoi(prg->trig_phrase)) {
					ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, ch, (void *)arg1, (void *)arg2, (void *)arg2, NULL,phrase,NULL);
					SETPRET;
				}
			}
		}
	} else
		bug("PERCENT trigger missing program type.", 0);

	PRETURN;
}



int p_bribe_trigger(CHAR_DATA *mob, CHAR_DATA *ch, int amount)
{
	PROG_LIST *prg;
	int slot, ret_val = PRET_NOSCRIPT, ret; // @@@NIB Default for a trigger loop is NO SCRIPT

	slot = trigger_slots[TRIG_BRIBE];

	if(IS_NPC(mob) && mob->pIndexData->progs) {
		script_destructed = FALSE;
		for (prg = mob->pIndexData->progs[slot]; prg && !script_destructed; prg = prg->next) {
			if (is_trigger_type(prg->trig_type,TRIG_BRIBE) && amount >= atoi(prg->trig_phrase)) {
				if((ret = execute_script(prg->vnum, prg->script, mob, NULL, NULL, NULL, ch, NULL, NULL, NULL, NULL,NULL,NULL)) != PRET_NOSCRIPT) {
					ret_val = ret;
					break;
				}
			}
		}
	}
	PRETURN;
}


int p_exit_trigger(CHAR_DATA *ch, int dir, int type)
{
	CHAR_DATA *mob, *mnext;
	OBJ_DATA *obj, *onext;
	ROOM_INDEX_DATA *room;
	TOKEN_DATA *token, *tnext;
	PROG_LIST *prg;
	int ret_val = PRET_NOSCRIPT, ret; // @@@NIB Default for a trigger loop is NO SCRIPT

	// If not in a valid room, there's nothing to check!
	if (!ch || !ch->in_room)
		PRETURN; // @@@NIB

	if (type == PRG_MPROG) {
		for (mob = ch->in_room->people; mob != NULL; mob = mnext) {
			mnext = mob->next_in_room;

			// Check for tokens FIRST
			for(token = mob->tokens; token; token = tnext) {
				tnext = token->next;
				if(token->pIndexData->progs) {
					script_destructed = FALSE;
					for (prg = token->pIndexData->progs[TRIGSLOT_MOVE]; prg && !script_destructed; prg = prg->next) {
						if (is_trigger_type(prg->trig_type,TRIG_EXIT) &&
							dir == atoi(prg->trig_phrase) &&
							mob->position == mob->pIndexData->default_pos &&
							can_see(mob, ch)) {
							ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, ch, NULL, NULL, NULL, NULL,NULL,NULL);
							SETPRETX;
						} else if (is_trigger_type(prg->trig_type,TRIG_EXALL) && dir == atoi(prg->trig_phrase)) {
							ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, ch, NULL, NULL, NULL, NULL,NULL,NULL);
							SETPRETX;
						}
					}

					// Since the token didn't destruct, this is valid
					if(!script_destructed) tnext = token->next;

				}
			}

			if (IS_NPC(mob) && mob->pIndexData->progs) {
				script_destructed = FALSE;
				for (prg = mob->pIndexData->progs[TRIGSLOT_MOVE]; prg && !script_destructed; prg = prg->next) {
					/*
					 * Exit trigger works only if the mobile is not busy
					 * (fighting etc.). If you want to be sure all players
					 * are caught, use ExAll trigger
					 */
					if (is_trigger_type(prg->trig_type,TRIG_EXIT) &&
						dir == atoi(prg->trig_phrase) &&
						mob->position == mob->pIndexData->default_pos &&
						can_see(mob, ch)) {
						ret = execute_script(prg->vnum, prg->script, mob, NULL, NULL, NULL, ch, NULL, NULL, NULL, NULL,NULL,NULL);
						SETPRETX; // @@@NIB
					} else if (is_trigger_type(prg->trig_type,TRIG_EXALL) && dir == atoi(prg->trig_phrase)) {
						ret = execute_script(prg->vnum, prg->script, mob, NULL, NULL, NULL, ch, NULL, NULL, NULL, NULL,NULL,NULL);
						SETPRETX; // @@@NIB
					}
				}

				// Since the mob didn't destruct, this is valid
				if(!script_destructed) mnext = mob->next_in_room;
			}

		}
	} else if (type == PRG_OPROG) {
		for (obj = ch->in_room->contents; obj != NULL; obj = onext) {
			onext = obj->next_content;
			if (obj->pIndexData->progs) {
				script_destructed = FALSE;
				for (prg = obj->pIndexData->progs[TRIGSLOT_MOVE]; prg && !script_destructed; prg = prg->next) {
					if (is_trigger_type(prg->trig_type,TRIG_EXALL) && dir == atoi(prg->trig_phrase)) {
						ret = execute_script(prg->vnum, prg->script, NULL, obj, NULL, NULL, ch, NULL, NULL, NULL, NULL,NULL,NULL);
						SETPRETX; // @@@NIB
					}
				}

				// Since the object didn't destruct, this is valid
				if(!script_destructed) onext = obj->next_content;
			}
		}

		for (mob = ch->in_room->people; mob; mob = mnext) {
			mnext = mob->next_in_room;
			for (obj = mob->carrying; obj; obj = onext) {
				onext = obj->next_content;
				if (obj->pIndexData->progs) {
					script_destructed = FALSE;
					for (prg = obj->pIndexData->progs[TRIGSLOT_MOVE]; prg && !script_destructed; prg = prg->next) {
						if (is_trigger_type(prg->trig_type,TRIG_EXALL) && dir == atoi(prg->trig_phrase)) {
							ret = execute_script(prg->vnum, prg->script, NULL, obj, NULL, NULL, ch, NULL, NULL, NULL, NULL,NULL,NULL);
							SETPRETX; // @@@NIB
						}
					}

					// Since the object didn't destruct, this is valid
					if(!script_destructed) onext = obj->next_content;
				}
			}
		}
	} else if (type == PRG_RPROG) {
		room = ch->in_room;

		if (room->source) {
			if (room->source->progs->progs) {
				script_destructed = FALSE;
				for (prg = room->source->progs->progs[TRIGSLOT_MOVE]; prg && !script_destructed; prg = prg->next) {
					if (is_trigger_type(prg->trig_type,TRIG_EXALL) && dir == atoi(prg->trig_phrase)) {
						ret = execute_script(prg->vnum, prg->script, NULL, NULL, room, NULL, ch, NULL, NULL, NULL, NULL,NULL,NULL);
						SETPRETX; // @@@NIB
					}
				}
			}
		} else if (room->progs->progs) {
			script_destructed = FALSE;
			for (prg = room->progs->progs[TRIGSLOT_MOVE]; prg && !script_destructed; prg = prg->next) {
				if (is_trigger_type(prg->trig_type,TRIG_EXALL) && dir == atoi(prg->trig_phrase)) {
					ret = execute_script(prg->vnum, prg->script, NULL, NULL, room, NULL, ch, NULL, NULL, NULL, NULL,NULL,NULL);
					SETPRETX; // @@@NIB
				}
			}
		}
	}

	PRETURN; // @@@NIB
}


/* A new type, p_direction_trigger that is almost identical to p_exit_trigger.
   This one just allows different types of trigs (open/close) as the caller */
int p_direction_trigger(CHAR_DATA *ch, ROOM_INDEX_DATA *here, int dir, int type, int trigger)
{
	CHAR_DATA *mob, *mnext;
	OBJ_DATA *obj, *onext;
	ROOM_INDEX_DATA *room;
	TOKEN_DATA *token, *tnext;
	PROG_LIST *prg;
	int slot, ret_val = PRET_NOSCRIPT, ret; // @@@NIB Default for a trigger loop is NO SCRIPT

	// If not in a valid room, there's nothing to check!
	if(!ch || !here)
		PRETURN; // @@@NIB

	slot = trigger_slots[trigger];

	if (type == PRG_MPROG) {
		for (mob = here->people; mob != NULL; mob = mnext) {
			mnext = mob->next_in_room;

			for(token = mob->tokens; token; token = tnext) {
				tnext = token->next;
				if(token->pIndexData->progs) {
					script_destructed = FALSE;
					for (prg = token->pIndexData->progs[slot]; prg && !script_destructed; prg = prg->next) {
						if (is_trigger_type(prg->trig_type,trigger) &&
							dir == atoi(prg->trig_phrase)) {
							ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, ch, NULL, NULL, NULL, NULL,NULL,NULL);
							SETPRETX;
						}
					}

					// Since the token didn't destruct, this is valid
					if(!script_destructed) tnext = token->next;
				}
			}

			if (IS_NPC(mob) && mob->pIndexData->progs) {
				script_destructed = FALSE;
				for (prg = mob->pIndexData->progs[slot]; prg && !script_destructed; prg = prg->next) {
					if (is_trigger_type(prg->trig_type,trigger) &&
						dir == atoi(prg->trig_phrase) &&
						mob->position == mob->pIndexData->default_pos &&
						can_see(mob, ch)) {
						ret = execute_script(prg->vnum, prg->script, mob, NULL, NULL, NULL, ch, NULL, NULL, NULL, NULL,NULL,NULL);
						SETPRETX; // @@@NIB
					}
				}
				// Since the mobile didn't destruct, this is valid
				if(!script_destructed) mnext = mob->next_in_room;
			}
		}

	} else if (type == PRG_OPROG) {
		for (obj = here->contents; obj != NULL; obj = onext) {
			onext = obj->next_content;
			if (obj->pIndexData->progs) {
				script_destructed = FALSE;
				for (prg = obj->pIndexData->progs[slot]; prg && !script_destructed; prg = prg->next) {
					if (is_trigger_type(prg->trig_type,trigger) && dir == atoi(prg->trig_phrase)) {
						ret = execute_script(prg->vnum, prg->script, NULL, obj, NULL, NULL, ch, NULL, NULL, NULL, NULL,NULL,NULL);
						SETPRETX; // @@@NIB
					}
				}

				// Since the object didn't destruct, this is valid
				if(!script_destructed) onext = obj->next_content;
			}
		}

		for (mob = ch->in_room->people; mob; mob = mnext) {
			mnext = mob->next_in_room;
			for (obj = mob->carrying; obj; obj = onext) {
				onext = obj->next_content;
				if (obj->pIndexData->progs) {
					script_destructed = FALSE;
					for (prg = obj->pIndexData->progs[slot]; prg && !script_destructed; prg = prg->next) {
						if (is_trigger_type(prg->trig_type,trigger) && dir == atoi(prg->trig_phrase)) {
							ret = execute_script(prg->vnum, prg->script, NULL, obj, NULL, NULL, ch, NULL, NULL, NULL, NULL,NULL,NULL);
							SETPRETX; // @@@NIB
						}
					}

					// Since the object didn't destruct, this is valid
					if(!script_destructed) onext = obj->next_content;
				}
			}
		}
	} else if (type == PRG_RPROG) {
		room = here;

		if (room->source) {
			if (room->source->progs->progs) {
				script_destructed = FALSE;
				for (prg = room->source->progs->progs[slot]; prg && !script_destructed; prg = prg->next) {
					if (is_trigger_type(prg->trig_type,trigger) && dir == atoi(prg->trig_phrase)) {
						ret = execute_script(prg->vnum, prg->script, NULL, NULL, room, NULL, ch, NULL, NULL, NULL, NULL,NULL,NULL);
						SETPRETX; // @@@NIB
					}
				}
			}
		} else if (room->progs->progs) {
			script_destructed = FALSE;
			for (prg = room->progs->progs[slot]; prg && !script_destructed; prg = prg->next) {
				if (is_trigger_type(prg->trig_type,trigger) && dir == atoi(prg->trig_phrase)) {
					ret = execute_script(prg->vnum, prg->script, NULL, NULL, room, NULL, ch, NULL, NULL, NULL, NULL,NULL,NULL);
					SETPRETX; // @@@NIB
				}
			}
		}
	}

	PRETURN; // @@@NIB
}


int p_give_trigger(CHAR_DATA *mob, OBJ_DATA *obj, ROOM_INDEX_DATA *room,
			CHAR_DATA *ch, OBJ_DATA *dropped, int type)
{
	TOKEN_DATA *token, *tnext;
	char buf[MIL], *p;
	PROG_LIST  *prg;
	int slot;
	int ret_val = PRET_NOSCRIPT, ret; // @@@NIB Default for a trigger loop is NO SCRIPT

	if ((mob && obj) || (mob && room) || (obj && room)) {
		bug("Multiple program types in GIVE trigger.", 0);
		PRETURN; // @@@NIB
	}

	slot = trigger_slots[type];

	if (mob) {
		for(token = mob->tokens; token; token = tnext) {
			tnext = token->next;
			if(token->pIndexData->progs) {
				script_destructed = FALSE;
				for (prg = token->pIndexData->progs[slot]; prg && !script_destructed; prg = prg->next) {
					if (is_trigger_type(prg->trig_type,type)) {
						p = prg->trig_phrase;
						if (is_number(p)) {
							if (dropped->pIndexData->vnum == atol(p)) {
								ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, ch, dropped, NULL, NULL, NULL,NULL,NULL);
								SETPRETX; // @@@NIB
							}
						} else {
							// Dropped object name argument, e.g. 'sword'
							while(*p && !script_destructed) {
								p = one_argument(p, buf);
								if (is_name(buf, dropped->name) || !str_cmp("all", buf)) {
									ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, ch, dropped, NULL, NULL, NULL,NULL,NULL);
									SETPRETX; // @@@NIB
								}
							}
						}
					}
				}

				// Since the token didn't destruct, this is valid
				if(!script_destructed) tnext = token->next;
			}
		}

		if (IS_NPC(mob) && mob->pIndexData->progs) {
			script_destructed = FALSE;
			for (prg = mob->pIndexData->progs[slot]; prg && !script_destructed; prg = prg->next) {
				if (is_trigger_type(prg->trig_type,type)) {
					p = prg->trig_phrase;
					if (is_number(p)) {
						if (dropped->pIndexData->vnum == atol(p)) {
							ret = execute_script(prg->vnum, prg->script, mob, NULL, NULL, NULL, ch, dropped, NULL, NULL, NULL,NULL,NULL);
							SETPRETX; // @@@NIB
						}
					} else {
						// Dropped object name argument, e.g. 'sword'
						while(*p && !script_destructed) {
							p = one_argument(p, buf);
							if (is_name(buf, dropped->name) || !str_cmp("all", buf)) {
								ret = execute_script(prg->vnum, prg->script, mob, NULL, NULL, NULL, ch, dropped, NULL, NULL, NULL,NULL,NULL);
								SETPRETX; // @@@NIB
							}
						}
					}
				}
			}
		}
	} else if (obj) {
		if (!obj_room(obj)) return PRET_NOSCRIPT; // @@@NIB

		if(obj->pIndexData->progs) {
			script_destructed = FALSE;
			for (prg = obj->pIndexData->progs[slot]; prg && !script_destructed; prg = prg->next) {
				if (is_trigger_type(prg->trig_type,type)) {
					ret = execute_script(prg->vnum, prg->script, NULL, obj, NULL, NULL, ch, obj, NULL, NULL, NULL,NULL,NULL);
					SETPRETX; // @@@NIB
				}
			}
		}
	} else if (room) {
		if(room->source) {
			if(room->source->progs->progs) {
				script_destructed = FALSE;
				for (prg = room->source->progs->progs[slot]; prg && !script_destructed; prg = prg->next)
					if (is_trigger_type(prg->trig_type,type)) {
						p = prg->trig_phrase;
						if (is_number(p)) {
							if (dropped->pIndexData->vnum == atol(p)) {
								ret = execute_script(prg->vnum, prg->script, NULL, NULL, room, NULL, ch, dropped, NULL, NULL, NULL,NULL,NULL);
								SETPRETX; // @@@NIB
							}
						} else {
							// Dropped object name argument, e.g. 'sword'
							while(*p && !script_destructed) {
							p = one_argument(p, buf);
							if (is_name(buf, dropped->name) || !str_cmp("all", buf)) {
								ret = execute_script(prg->vnum, prg->script, NULL, NULL, room, NULL, ch, dropped, NULL, NULL, NULL,NULL,NULL);
								SETPRETX; // @@@NIB
							}
						}
					}
				}
			}
		} else if(room->progs->progs) {
			script_destructed = FALSE;
			for (prg = room->progs->progs[slot]; prg && !script_destructed; prg = prg->next)
				if (is_trigger_type(prg->trig_type,type)) {
					p = prg->trig_phrase;
					if (is_number(p)) {
						if (dropped->pIndexData->vnum == atol(p)) {
							ret = execute_script(prg->vnum, prg->script, NULL, NULL, room, NULL, ch, dropped, NULL, NULL, NULL,NULL,NULL);
							SETPRETX; // @@@NIB
						}
					} else {
						// Dropped object name argument, e.g. 'sword'
						while(*p && !script_destructed) {
						p = one_argument(p, buf);
						if (is_name(buf, dropped->name) || !str_cmp("all", buf)) {
							ret = execute_script(prg->vnum, prg->script, NULL, NULL, room, NULL, ch, dropped, NULL, NULL, NULL,NULL,NULL);
							SETPRETX; // @@@NIB
						}
					}
				}
			}
		}
	}

	PRETURN; // @@@NIB
}


int p_use_trigger(CHAR_DATA *ch, OBJ_DATA *obj, int type)
{
	if (obj == NULL) {
		bug("p_use_trigger: received null obj!", 0);
		return PRET_NOSCRIPT;
	}

	if (!obj_room(obj)) return PRET_NOSCRIPT;

	if (type == TRIG_PUSH)
		return p_percent_trigger(NULL, obj, NULL, NULL, ch, NULL, NULL, TRIG_PUSH);
	else if (type == TRIG_TURN)
		return p_percent_trigger(NULL, obj, NULL, NULL, ch, NULL, NULL, TRIG_TURN);
	else if (type == TRIG_PULL)
		return p_percent_trigger(NULL, obj, NULL, NULL, ch, NULL, NULL, TRIG_PULL);
	else if (type == TRIG_USE)
		return p_percent_trigger(NULL, obj, NULL, NULL, ch, NULL, NULL, TRIG_USE);

	return PRET_NOSCRIPT;
}

int p_use_on_trigger(CHAR_DATA *ch, OBJ_DATA *obj, int type, char *argument)
{
	if (obj == NULL) {
		bug("p_use_on_trigger: received null obj!", 0);
		return PRET_NOSCRIPT;
	}

	if (!obj_room(obj)) return PRET_NOSCRIPT;

	if (type == TRIG_PUSH_ON)
		return p_act_trigger(argument, NULL, obj, NULL, ch, NULL, NULL, TRIG_PUSH_ON);
	else if (type == TRIG_TURN_ON)
		return p_act_trigger(argument, NULL, obj, NULL, ch, NULL, NULL, TRIG_TURN_ON);
	else if (type == TRIG_PULL_ON)
		return p_act_trigger(argument, NULL, obj, NULL, ch, NULL, NULL, TRIG_PULL_ON);

	return PRET_NOSCRIPT;
}

int p_use_with_trigger(CHAR_DATA *ch, OBJ_DATA *obj, int type, void *arg1, void *arg2)
{
	if (obj == NULL) {
		bug("p_use_with_trigger: received null obj!", 0);
		return PRET_NOSCRIPT;
	}

	if (!obj_room(obj)) return PRET_NOSCRIPT;

	if (type == TRIG_USEWITH)
		return p_percent_trigger(NULL, obj, NULL, NULL, ch, arg1, arg2, TRIG_USEWITH);

	return PRET_NOSCRIPT;
}


int p_greet_trigger(CHAR_DATA *ch, int type)
{
	CHAR_DATA *mob, *mnext;
	OBJ_DATA *obj, *onext;
	ROOM_INDEX_DATA *room;
	TOKEN_DATA *token, *tnext;
	PROG_LIST *prg;
	int ret_val = PRET_NOSCRIPT, ret; // @@@NIB Default for a trigger loop is NO SCRIPT

	// If not in a valid room, there's nothing to check!
	if (!ch || !ch->in_room)
		PRETURN; // @@@NIB

	if (type == PRG_MPROG) {
		for (mob = ch->in_room->people; mob != NULL; mob = mnext) {
			mnext = mob->next_in_room;

			// Check for tokens FIRST
			for(token = mob->tokens; token; token = tnext) {
				tnext = token->next;
				if(token->pIndexData->progs) {
					script_destructed = FALSE;
					for (prg = token->pIndexData->progs[TRIGSLOT_MOVE]; prg && !script_destructed; prg = prg->next) {
						if (is_trigger_type(prg->trig_type,TRIG_GREET) &&
							number_percent() < atoi(prg->trig_phrase) &&
							mob->position == mob->pIndexData->default_pos &&
							can_see(mob, ch)) {
							ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, ch, NULL, NULL, NULL, NULL,NULL,NULL);
							SETPRETX;
						} else if (is_trigger_type(prg->trig_type,TRIG_GRALL) && number_percent() < atoi(prg->trig_phrase)) {
							ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, ch, NULL, NULL, NULL, NULL,NULL,NULL);
							SETPRETX;
						}
					}

					// Since the object didn't destruct, this is valid
					if(!script_destructed) tnext = token->next;
				}
			}

			if (IS_NPC(mob) && mob->pIndexData->progs) {
				script_destructed = FALSE;
				for (prg = mob->pIndexData->progs[TRIGSLOT_MOVE]; prg && !script_destructed; prg = prg->next) {
					/*
					 * Exit trigger works only if the mobile is not busy
					 * (fighting etc.). If you want to be sure all players
					 * are caught, use ExAll trigger
					 */
					if (is_trigger_type(prg->trig_type,TRIG_GREET) &&
						number_percent() < atoi(prg->trig_phrase) &&
						mob->position == mob->pIndexData->default_pos &&
						can_see(mob, ch)) {
						ret = execute_script(prg->vnum, prg->script, mob, NULL, NULL, NULL, ch, NULL, NULL, NULL, NULL,NULL,NULL);
						SETPRETX; // @@@NIB
					} else if (is_trigger_type(prg->trig_type,TRIG_GRALL) && number_percent() < atoi(prg->trig_phrase)) {
						ret = execute_script(prg->vnum, prg->script, mob, NULL, NULL, NULL, ch, NULL, NULL, NULL, NULL,NULL,NULL);
						SETPRETX; // @@@NIB
					}
				}
				// Since the object didn't destruct, this is valid
				if(!script_destructed) mnext = mob->next_in_room;
			}
		}
	} else if (type == PRG_OPROG) {
		for (obj = ch->in_room->contents; obj != NULL; obj = onext) {
			onext = obj->next_content;
			if (obj->pIndexData->progs) {
				script_destructed = FALSE;
				for (prg = obj->pIndexData->progs[TRIGSLOT_MOVE]; prg && !script_destructed; prg = prg->next) {
					if (is_trigger_type(prg->trig_type,TRIG_GRALL) && number_percent() < atoi(prg->trig_phrase)) {
						ret = execute_script(prg->vnum, prg->script, NULL, obj, NULL, NULL, ch, NULL, NULL, NULL, NULL,NULL,NULL);
						SETPRETX; // @@@NIB
					}
				}
				// Since the object didn't destruct, this is valid
				if(!script_destructed) onext = obj->next_content;
			}
		}

		for (mob = ch->in_room->people; mob; mob = mnext) {
			mnext = mob->next_in_room;
			for (obj = mob->carrying; obj; obj = onext) {
				onext = obj->next_content;
				if (obj->pIndexData->progs) {
					script_destructed = FALSE;
					for (prg = obj->pIndexData->progs[TRIGSLOT_MOVE]; prg && !script_destructed; prg = prg->next) {
						if (is_trigger_type(prg->trig_type,TRIG_GRALL) && number_percent() < atoi(prg->trig_phrase)) {
							ret = execute_script(prg->vnum, prg->script, NULL, obj, NULL, NULL, ch, NULL, NULL, NULL, NULL,NULL,NULL);
							SETPRETX; // @@@NIB
						}
					}
					// Since the object didn't destruct, this is valid
					if(!script_destructed) onext = obj->next_content;
				}
			}
		}
	} else if (type == PRG_RPROG) {
		room = ch->in_room;

		if (room->source) {
			if (room->source->progs->progs) {
				script_destructed = FALSE;
				for (prg = room->source->progs->progs[TRIGSLOT_MOVE]; prg && !script_destructed; prg = prg->next) {
					if (is_trigger_type(prg->trig_type,TRIG_GRALL) && number_percent() < atoi(prg->trig_phrase)) {
						ret = execute_script(prg->vnum, prg->script, NULL, NULL, room, NULL, ch, NULL, NULL, NULL, NULL,NULL,NULL);
						SETPRETX; // @@@NIB
					}
				}
			}
		} else if (room->progs->progs) {
			script_destructed = FALSE;
			for (prg = room->progs->progs[TRIGSLOT_MOVE]; prg && !script_destructed; prg = prg->next) {
				if (is_trigger_type(prg->trig_type,TRIG_GRALL) && number_percent() < atoi(prg->trig_phrase)) {
					ret = execute_script(prg->vnum, prg->script, NULL, NULL, room, NULL, ch, NULL, NULL, NULL, NULL,NULL,NULL);
					SETPRETX; // @@@NIB
				}
			}
		}
	}

	PRETURN; // @@@NIB
}


int p_hprct_trigger(CHAR_DATA *mob, CHAR_DATA *ch) // @@@NIB
{
	TOKEN_DATA *token, *tnext;
	PROG_LIST *prg;
	int percent, slot;
	int ret_val = PRET_NOSCRIPT, ret; // @@@NIB Default for a trigger loop is NO SCRIPT

	slot = trigger_slots[TRIG_HPCNT];
	percent = (100 * mob->hit / mob->max_hit);

	// Check for tokens FIRST
	for(token = mob->tokens; token; token = tnext) {
		tnext = token->next;
		if(token->pIndexData->progs) {
			script_destructed = FALSE;
			for (prg = token->pIndexData->progs[slot]; prg && !script_destructed; prg = prg->next) {
				if (is_trigger_type(prg->trig_type,TRIG_HPCNT) &&
					percent < atoi(prg->trig_phrase) &&
					((ret = execute_script(prg->vnum, prg->script, NULL, NULL, NULL, token, ch, NULL, NULL, NULL, NULL,NULL,NULL)) != PRET_NOSCRIPT)) {
					ret_val = ret;
					break;
				}
			}
			// Since the object didn't destruct, this is valid
			if(!script_destructed) tnext = token->next;
		}
	}

	if(ret_val == PRET_NOSCRIPT && IS_NPC(mob) && mob->pIndexData->progs) {
		script_destructed = FALSE;
		for (prg = mob->pIndexData->progs[slot]; prg && !script_destructed; prg = prg->next) {
			if ((is_trigger_type(prg->trig_type,TRIG_HPCNT)) &&
				(percent < atoi(prg->trig_phrase)) &&
				((ret = execute_script(prg->vnum, prg->script, mob, NULL, NULL, NULL, ch, NULL, NULL, NULL, NULL,NULL,NULL)) != PRET_NOSCRIPT)) {
				ret_val = ret;
				break; // @@@NIB break only on NON-ZERO, instead of continuing
			}
		}
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

	act("{MThe crimson aura around you pulses!{x", victim, NULL, NULL, TO_CHAR);
	act("{MThe crimson aura around $n pulses!{x", victim, NULL, NULL, TO_ROOM);

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
				act("Your spell gets through $N's protective crimson aura!", ch, NULL, victim, TO_CHAR);
				act("$n's spell gets through your protective crimson aura!", ch, NULL, victim, TO_VICT);
				act("$n's spell gets through $N's protective crimson aura!", ch, NULL, victim, TO_NOTVICT);
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
		act("{MThe crimson aura around $n vanishes.{x", victim, NULL, NULL, TO_ROOM);
		affect_remove(victim, af);
		return TRUE;
	}

	if (rch) {
		if (ch) {
			act("{YYour spell bounces off onto $N!{x", ch, NULL, rch, TO_CHAR);
			act("{Y$n's spell bounces off onto you!{x", ch, NULL, rch, TO_VICT);
			act("{Y$n's spell bounces off onto $N!{x", ch, NULL, rch, TO_NOTVICT);
		}

		token->value[3] = ch ? ch->tot_level : af->level;

		execute_script(script->vnum, script, NULL, NULL, NULL, token, ch ? ch : rch, NULL, NULL, rch, NULL,"deflection",NULL);
	} else if (ch) {
		act("{YYour spell bounces around for a while, then dies out.{x", ch, NULL, NULL, TO_CHAR);
		act("{Y$n's spell bounces around for a while, then dies out.{x", ch, NULL, NULL, TO_ROOM);
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
	return p_percent_trigger_phrase(ch, NULL, NULL, NULL, ch, NULL, NULL, TRIG_INTERRUPT, silent?"silent":NULL);
}


