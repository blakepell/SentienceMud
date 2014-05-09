
#include "merc.h"
#include "scripts.h"

//#define DEBUG_MODULE
#include "debug.h"

static bool wiznet_variables = FALSE;

// Check the validity of the script parameters
//	Should the ids mismatch, reset the field
bool check_varinfo(SCRIPT_VARINFO *info)
{
	bool ret = TRUE;
	return ret;
}

void expand_escape2print(char *str, char *store)
{
	while(*str) {
		if(isprint(*str)) *store++ = *str;
		else {
			sprintf(store,"0x%2.2X", *str);
			store+=4;
		}
		str++;
		if(*str) *store++ = ' ';
	}
	*store = 0;
}

char *expand_skip(register char *str)
{
	register int depth = 0;

	while(*str) {
		if(*str == ESCAPE_VARIABLE || *str == ESCAPE_EXPRESSION || *str == ESCAPE_ENTITY)
			++depth;
		else if(*str == ESCAPE_END && !depth--)
			break;
		++str;
	}

	if(depth > 0) return str;

	return str + 1;
}


static bool push(STACK *stk,int val)
{
	if(stk->t >= MAX_STACK) return FALSE;
	stk->s[stk->t++] = val;
	return TRUE;
}

static int perform_operation(STACK *op,STACK *opr)
{
	int op1, op2, optr;

	optr = pop(opr,STK_EMPTY);

	if(optr == STK_MAX || optr == STK_EMPTY || op->t < script_expression_argstack[optr])
		return ERROR4;

	op2 = op1 = 0;
	if(script_expression_argstack[optr] > 1) op2 = pop(op,-1);
	if(script_expression_argstack[optr] > 0) op1 = pop(op,-1);

	switch(optr) {
	case STK_ADD:	op1 += op2; break;
	case STK_SUB:	op1 -= op2; break;
	case STK_MUL:	op1 *= op2; break;
	case STK_DIV:	if(!op2) return ERROR3; op1 /= op2; break;
	case STK_MOD:	if(!op2) return ERROR3; op1 = op1 % op2; break;
	case STK_RAND:	op1 = number_range(op1,op2);	break;
	case STK_NOT:	op1 = !op1; break;
	case STK_NEG:	op1 = -op1; break;
	default:	op1 = 0;
	}

	push(op,op1);
	return DONE;
}

static bool push_operator(STACK *stk,int op)
{
	if(script_expression_tostack[op] == STK_MAX) return FALSE;
	return push(stk,script_expression_tostack[op]);
}

static bool process_expession_stack(STACK *stk_op,STACK *stk_opr,int op)
{
	int t;

	if(op == CH_MAX) {
		//printf("invalid operator.\n");
		return FALSE;
	}

	// Iterate through the operators that CAN be popped off the stack
	while((t = script_expression_stack_action[op][top(stk_opr,STK_EMPTY)]) == POP &&
		(t = perform_operation(stk_op,stk_opr)) == DONE);

	if(t == DONE) return TRUE;
	else if(t == PUSH) {
		push_operator(stk_opr,op);
		return TRUE;
	} else if(t == DELETE) {
		--stk_opr->t;
		return TRUE;
	}
//	else if(t == ERROR1) printf("unmatched right parentheses.\n");
//	else if(t == ERROR2) printf("unmatched left parentheses.\n");
//	else if(t == ERROR3) printf("division by zero.\n");
//	else if(t == ERROR4) printf("insufficient operators/operands on stack.\n");

	return FALSE;
}

// Used by expand_variable to expand internal variable references into string components
// This will allow for things like...
// $<speed<oretype>>
//
// If 'oretype' is 'iron'... it will translate into... speediron
char *expand_variable_recursive(pVARIABLE vars,char *str,char **store)
{
//	char esc[MSL];
//	char msg[MSL*2];
	char buf[MSL], *p = buf;
	pVARIABLE var;

/*
	{
		expand_escape2print(str,esc);
		sprintf(msg,"expand_variable_recursive->before = \"%s\"",esc);
		wiznet(msg,NULL,NULL,WIZ_SCRIPTS,0,0);
	}
*/

	while(*str && *str != ESCAPE_END) {
		if(*str == ESCAPE_VARIABLE) {
			str = expand_variable_recursive(vars,str+1,&p);
			if(!str) return NULL;
		} else
			*p++ = *str++;
	}
	*p = 0;

/*
	{
		expand_escape2print(str,esc);
		sprintf(msg,"expand_variable_recursive->after = \"%s\"",esc);
		wiznet(msg,NULL,NULL,WIZ_SCRIPTS,0,0);
	}
*/

	var = variable_get(vars,buf);
	if(var) {
		if((var->type == VAR_STRING || var->type == VAR_STRING_S) && var->_.s && *var->_.s) {
			strcpy(*store,var->_.s);
			*store += strlen(var->_.s);
		} else if(var->type == VAR_INTEGER) {
			*store += sprintf(*store,"%d",var->_.i);
		}
/*
	} else if(var) {
		char msg[MSL];
		sprintf(msg,"expand_variable_recursive() -> var '%s' is not a string or is empty\n\r", var->name);
		wiznet(msg,NULL,NULL,WIZ_SCRIPTS,0,0);
		return NULL;
	} else {
		char msg[MSL];
		sprintf(msg,"expand_variable_recursive() -> no variable named '%s'\n\r", buf);
		wiznet(msg,NULL,NULL,WIZ_SCRIPTS,0,0);
		return NULL;
*/
	}

	return str+1;
}

char *expand_variable(pVARIABLE vars,char *str,pVARIABLE *var)
{
	char buf[MIL], *p = buf;

	if(*str == ESCAPE_VARIABLE && wiznet_variables) {
		char msg[MSL];
		sprintf(msg,"expand_variable() -> *str IS ESCAPE_VARIABLE\n\r");
		wiznet(msg,NULL,NULL,WIZ_SCRIPTS,0,0);
	}

	while(*str && *str != ESCAPE_END) {
		if(*str == ESCAPE_VARIABLE) {
			str = expand_variable_recursive(vars,str+1,&p);
			if(!str) {
				if(wiznet_variables) {
					char msg[MSL];
					*p = 0;
					sprintf(msg,"expand_variable(\"%s\") -> NULL str RETURN\n\r", buf);
					wiznet(msg,NULL,NULL,WIZ_SCRIPTS,0,0);
				}
				return NULL;
			}
		} else
			*p++ = *str++;
	}
	*p = 0;

	if(*str != ESCAPE_END) {
		if(wiznet_variables) {
			char msg[MSL];
			sprintf(msg,"expand_variable(\"%s\") -> NO ESCAPE\n\r", buf);
			wiznet(msg,NULL,NULL,WIZ_SCRIPTS,0,0);
		}
		return NULL;
	}

	if(wiznet_variables) {
		char msg[MSL];
		sprintf(msg,"expand_variable(\"%s\")\n\r", buf);
		wiznet(msg,NULL,NULL,WIZ_SCRIPTS,0,0);
	}

	*var = variable_get(vars,buf);

	return str+1;
}

char *expand_name(pVARIABLE vars,char *str,char *store)
{
	char buf[MSL], *p = buf;

	while(*str && *str != ESCAPE_END) {
		if(*str == ESCAPE_VARIABLE) {
			str = expand_variable_recursive(vars,str+1,&p);
			if(!str) return NULL;
		} else
			*p++ = *str++;
	}
	*p = 0;

	if(*str != ESCAPE_END) return NULL;

	strcpy(store,buf);

	return str+1;
}

char *expand_number(char *str, int *num)
{
	char arg[MIL], *s = str, *p = arg;
	while(isdigit(*s)) *p++ = *s++;
 	*p = 0;
 	if(arg[0]) {
		str = s;
		*num = atoi(arg);
	} else
		*num = -1;
	return str;
}

char *expand_ifcheck(SCRIPT_VARINFO *info,char *str,int *value)
{
	int xifc;
	char *rest;
	IFCHECK_DATA *ifc;
	bool valid;

	DBG2ENTRY3(PTR,info,PTR,str,PTR,value);

	if(str[0] >= ESCAPE_EXTRA && str[1] >= ESCAPE_EXTRA) {
		xifc = (((str[0] - ESCAPE_EXTRA)&0x3F) |
			(((str[1] - ESCAPE_EXTRA)&0x3F)<<6));	// 0 - 4095

		if(xifc >= 0 && xifc < CHK_MAXIFCHECKS) {
			ifc = &ifcheck_table[xifc];

			if(ifc->numeric) {
				rest = ifcheck_get_value(info,ifc,str+2,value,&valid);
				if(rest && valid) {
//					strcpy(buf,"expand_ifcheck 1:");
//					for(xifc = 0; xifc < 20 && rest[xifc]; xifc++)
//						sprintf(buf + 15 + 3*xifc," %2.2X", rest[xifc]&0xFF);
//					wiznet(buf,NULL,NULL,WIZ_TESTING,0,0);
					rest = skip_whitespace(rest);
					if(*rest != ESCAPE_END) {
						DBG2EXITVALUE2(INVALID);
						return expand_skip(str);
					}
					++rest;

//					strcpy(buf,"expand_ifcheck 2:");
//					for(xifc = 0; xifc < 20 && rest[xifc]; xifc++)
//						sprintf(buf + 15 + 3*xifc," %2.2X", rest[xifc]&0xFF);
//					wiznet(buf,NULL,NULL,WIZ_TESTING,0,0);

					DBG2EXITVALUE1(PTR,rest);
					return rest;
				}
			}
		}
	}

	*value = 0;
	DBG2EXITVALUE2(INVALID);
	return expand_skip(str);
}


char *expand_argument_expression(SCRIPT_VARINFO *info,char *str,int *num)
{
	STACK optr,opnd;
	pVARIABLE var;
	int value,op;
	bool first = TRUE;
	bool expect = FALSE;	// FALSE = number/open, TRUE = operator/close
	char *p = str;

	opnd.t = optr.t = 0;

	while(*str && *str != ESCAPE_END) {
		str = skip_whitespace(str);
		if(isdigit(*str)) {	// Constant
			if(expect) {
				// Generate an error - missing an operator
				break;
			}
			str = expand_number(str,&value);
//			sprintf(buf,"expand_argument_expression: number = %d, %2.2X", value, *str&0xFF);
//			wiznet(buf,NULL,NULL,WIZ_TESTING,0,0);
			if(value < 0) {
				// Generate an error - terms in here cannot be negative!
				break;
			}
			if(!push(&opnd,value)) {
				// Generate an error - expression too complex, simplify
				break;
			}
			expect = TRUE;
		} else if(*str == ESCAPE_VARIABLE) {	// Variable
			if(expect) {
				// Generate an error - missing an operator
				break;
			}

			str = expand_variable(*(info->var),str+1,&var);
			if(!str) {
				break;
			}
			if (var) {
				if(var->type == VAR_INTEGER)
					value = var->_.i;
				else if(var->type == VAR_STRING || var->type ==  VAR_STRING_S)
					value = is_number(var->_.s) ? atoi(var->_.s) : 0;
				else
					value = 0;
			} else
				value = 0;

			value = (var && var->type == VAR_INTEGER) ? var->_.i : 0;
//			sprintf(buf,"expand_argument_expression: variable = %d, %2.2X", value, *str&0xFF);
//			wiznet(buf,NULL,NULL,WIZ_TESTING,0,0);
			if(!push(&opnd,value)) {
				// Generate an error - expression too complex, simplify
				break;
			}
			expect = TRUE;
		} else if(*str == ESCAPE_EXPRESSION) {
			if(expect) {
				// Generate an error - missing an operator
				break;
			}

			str = expand_ifcheck(info,str+1,&value);
//			sprintf(buf,"expand_argument_expression: expression = %d, %2.2X", value, *str&0xFF);
//			wiznet(buf,NULL,NULL,WIZ_TESTING,0,0);
			DBG3MSG1("value = %d\n", value);
			if(!str) {
				// Generate an error - ifcheck had an error, it should report it
				break;
			}
			if(!push(&opnd,value)) {
				// Generate an error - expression too complex, simplify
				break;
			}
			expect = TRUE;
		} else {
//			sprintf(buf,"expand_argument_expression: operator = %c, %2.2X", *str, str[1]&0xFF);
//			wiznet(buf,NULL,NULL,WIZ_TESTING,0,0);

			switch(*str) {
			case '+': op = expect ? CH_ADD : CH_MAX; expect=FALSE; break;
			case '-': op = expect ? CH_SUB : CH_NEG; expect=FALSE; break;
			case '*': op = expect ? CH_MUL : CH_MAX; expect=FALSE; break;
			case '/': op = expect ? CH_DIV : CH_MAX; expect=FALSE; break;
			case '%': op = expect ? CH_MOD : CH_MAX; expect=FALSE; break;
			case ':': op = expect ? CH_RAND : CH_MAX; expect=FALSE; break;
			case '!': op = expect ? CH_MAX : CH_NOT; expect=FALSE; break;
			case '(': op = expect ? CH_MAX : CH_OPEN; expect=FALSE; break;
			case ')': op = expect ? CH_CLOSE : CH_MAX; expect=TRUE; break;
			default:  op = CH_MAX; break;
			}

			if(!process_expession_stack(&opnd,&optr,op)) {
				// Generate an error - use what was determined in function
				break;
			}

			++str;
		}
		first = FALSE;
	}

	for(op = 0; op < opnd.t; op++) {
		DBG3MSG2("Operand %d: %d\n", op, opnd.s[op]);
	}

	if(!str || *str != ESCAPE_END || !process_expession_stack(&opnd,&optr,CH_EOS) || opnd.t > 1) {
		*num = 0;
		return expand_skip(str ? str : p);
	} else {
		*num = pop(&opnd,0);
		return str+1;
	}
}


char *expand_argument_variable(SCRIPT_VARINFO *info,char *str,SCRIPT_PARAM *arg)
{
	pVARIABLE var;

	str = expand_variable(*(info->var),str,&var);
	if(!str) return NULL;

	if(!var) {
		arg->type = ENT_NUMBER;
		arg->d.num = 0;
	} else {
		switch(var->type) {
		default: arg->type = ENT_NUMBER; arg->d.num = 0; break;
		case VAR_INTEGER: arg->type = ENT_NUMBER; arg->d.num = var->_.i; break;
		case VAR_STRING:
		case VAR_STRING_S: arg->type = ENT_STRING; arg->d.str = var->_.s; break;
		case VAR_MOBILE: arg->type = ENT_MOBILE; arg->d.mob = var->_.m; break;
		case VAR_OBJECT: arg->type = ENT_OBJECT; arg->d.obj = var->_.o; break;
		case VAR_ROOM: arg->type = ENT_ROOM; arg->d.room = var->_.r; break;
		case VAR_EXIT: arg->type = ENT_EXIT; arg->d.exit = var->_.e; break;
		case VAR_TOKEN: arg->type = ENT_TOKEN; arg->d.token = var->_.t; break;
		}
	}
	return str;

}

void expand_argument_simple_code(SCRIPT_VARINFO *info,unsigned char code,SCRIPT_PARAM *arg)
{
	arg->type = ENT_NUMBER;
	arg->d.num = 0;

	switch(code) {
	case ESCAPE_LI:
		if(info->mob) {
			arg->type = ENT_MOBILE;
			arg->d.mob = info->mob;
		} else if(info->obj) {
			arg->type = ENT_OBJECT;
			arg->d.obj = info->obj;
		} else if(info->room) {
			arg->type = ENT_ROOM;
			arg->d.room = info->room;
		} else if(info->token) {
			arg->type = ENT_TOKEN;
			arg->d.token = info->token;
		}
		break;
	case ESCAPE_LN:
		arg->type = ENT_MOBILE;
		arg->d.mob = info->ch;
		break;
	case ESCAPE_LO:
		arg->type = ENT_OBJECT;
		arg->d.obj = info->obj1;
		break;
	case ESCAPE_LP:
		arg->type = ENT_OBJECT;
		arg->d.obj = info->obj2;
		break;
	case ESCAPE_LQ:
		arg->type = ENT_MOBILE;
		arg->d.mob = (info->targ && (*info->targ)) ? *info->targ : NULL;
		break;
	case ESCAPE_LR:
		arg->type = ENT_MOBILE;
		arg->d.mob = info->rch ? info->rch : get_random_char(info->mob, info->obj, info->room, info->token);
		break;
	case ESCAPE_LT:
		arg->type = ENT_MOBILE;
		arg->d.mob = info->vch;
		break;
	}
}

char *expand_entity_variable(pVARIABLE vars,char *str,SCRIPT_PARAM *arg)
{
	pVARIABLE var;
	str = expand_variable(vars,str,&var);
	if(!str) return NULL;
	switch(*str) {
	case ENTITY_VAR_NUM:
		if(!var) arg->d.num = 0;
		else if(var->type == VAR_INTEGER)
			arg->d.num = var->_.i;
		else return NULL;

		arg->type = ENT_NUMBER;
		break;
	case ENTITY_VAR_STR:
		if(!var) arg->d.str = NULL;
		else if(var->type == VAR_STRING || var->type == VAR_STRING_S)
			arg->d.str = var->_.s;
		else return NULL;

		if(!arg->d.str) arg->d.str = &str_empty[0];
		arg->type = ENT_STRING;
		break;
	case ENTITY_VAR_MOB:
		if(var && var->type == VAR_MOBILE && var->_.m)
			arg->d.mob = var->_.m;
		else return NULL;

		arg->type = ENT_MOBILE;
		break;
	case ENTITY_VAR_OBJ:
		if(var && var->type == VAR_OBJECT && var->_.o)
			arg->d.obj = var->_.o;
		else return NULL;

		arg->type = ENT_OBJECT;
		break;
	case ENTITY_VAR_ROOM:
		if(var && var->type == VAR_ROOM && var->_.r)
			arg->d.room = var->_.r;
		else return NULL;

		arg->type = ENT_ROOM;
		break;
	case ENTITY_VAR_EXIT:
		if(var && var->type == VAR_EXIT && var->_.e)
			arg->d.exit = var->_.e;
		else return NULL;

		arg->type = ENT_EXIT;
		break;
	case ENTITY_VAR_TOKEN:
		if(var && var->type == VAR_TOKEN && var->_.t)
			arg->d.token = var->_.t;
		else return NULL;

		arg->type = ENT_TOKEN;
		break;

	case ENTITY_VAR_AFFECT:
		if(var && var->type == VAR_AFFECT && var->_.aff)
			arg->d.aff = var->_.aff;
		else return NULL;

		arg->type = ENT_AFFECT;
		break;
	}
	return str;
}

char *expand_entity_primary(SCRIPT_VARINFO *info,char *str,SCRIPT_PARAM *arg)
{
	switch(*str) {
	case ENTITY_ENACTOR:
		arg->type = ENT_MOBILE;
		arg->d.mob = info->ch;
		break;
	case ENTITY_OBJ1:
		arg->type = ENT_OBJECT;
		arg->d.obj = info->obj1;
		break;
	case ENTITY_OBJ2:
		arg->type = ENT_OBJECT;
		arg->d.obj = info->obj2;
		break;
	case ENTITY_VICTIM:
		arg->type = ENT_MOBILE;
		arg->d.mob = info->vch;
		break;
	case ENTITY_TARGET:
		arg->type = ENT_MOBILE;
		arg->d.mob = info->targ ? *info->targ : NULL;
		break;
	case ENTITY_RANDOM:
		arg->type = ENT_MOBILE;
		arg->d.mob = info->rch;
		break;
	case ENTITY_HERE:
		if(info->mob)
			arg->d.room = info->mob->in_room;
		else if(info->obj)
			arg->d.room = obj_room(info->obj);
		else if(info->room)
			arg->d.room = info->room;
		else if(info->token)
			arg->d.room = token_room(info->token);
		else return NULL;
		arg->type = ENT_ROOM;
		break;
	case ENTITY_SELF:
		if(info->mob) {
			arg->type = ENT_MOBILE;
			arg->d.mob = info->mob;
		} else if(info->obj) {
			arg->type = ENT_OBJECT;
			arg->d.obj = info->obj;
		} else if(info->room) {
			arg->type = ENT_ROOM;
			arg->d.room = info->room;
		} else if(info->token) {
			arg->type = ENT_TOKEN;
			arg->d.token = info->token;
		} else return NULL;
		break;
	case ENTITY_PHRASE:
		arg->type = ENT_STRING;
		arg->d.str = info->phrase;
		break;
	case ENTITY_TRIGGER:
		arg->type = ENT_STRING;
		arg->d.str = info->trigger;
		break;
	case ENTITY_PRIOR:
		arg->type = ENT_PRIOR;
		arg->d.info = script_get_prior(info);
		break;

	case ESCAPE_VARIABLE:
		str = expand_entity_variable(*(info->var),str+1,arg);
		if(!str) return NULL;
		break;
	default: return NULL;
	}

	return str+1;
}

char *expand_entity_number(SCRIPT_VARINFO *info,char *str,SCRIPT_PARAM *arg)
{
	switch(*str) {
	case ENTITY_NUM_ABS:
		arg->d.num = abs(arg->d.num);
		break;
	default: return NULL;
	}

	return str+1;
}


char *expand_entity_string(SCRIPT_VARINFO *info,char *str,SCRIPT_PARAM *arg)
{
	switch(*str) {
	case ENTITY_STR_LEN:
		arg->type = ENT_NUMBER;
		arg->d.num = arg->d.str ? strlen(arg->d.str) : 0;
		break;
	default: return NULL;
	}

	return str+1;
}

char *expand_entity_mobile(SCRIPT_VARINFO *info,char *str,SCRIPT_PARAM *arg)
{
	CHAR_DATA *self = arg->d.mob;
	char *p;
	switch(*str) {
	case ENTITY_MOB_NAME:
		arg->type = ENT_STRING;
		arg->d.str = arg->d.mob ? (char*)arg->d.mob->name : (char*)SOMEONE;
		break;
	case ENTITY_MOB_SHORT:
		arg->type = ENT_STRING;
		p = arg->d.mob ? (char*)((IS_NPC(arg->d.mob) || arg->d.mob->morphed) ? arg->d.mob->short_descr : capitalize(arg->d.mob->name)) : (char*)"no one";
		strcpy(arg->buf,p);
		arg->d.str = arg->buf;
		break;
	case ENTITY_MOB_LONG:
		arg->type = ENT_STRING;
		arg->d.str = arg->d.mob ? (char*)arg->d.mob->long_descr : (char*)&str_empty[0];
		break;
	case ENTITY_MOB_SEX:
		arg->type = ENT_STRING;
		arg->d.str = arg->d.mob ? (char*)male_female[URANGE(0,arg->d.mob->sex,2)] : (char*)male_female[0];
		break;
	case ENTITY_MOB_HE:
		arg->type = ENT_STRING;
		arg->d.str = arg->d.mob ? (char*)he_she[URANGE(0,arg->d.mob->sex,2)] : (char*)SOMEONE;
		break;
	case ENTITY_MOB_HIM:
		arg->type = ENT_STRING;
		arg->d.str = arg->d.mob ? (char*)him_her[URANGE(0,arg->d.mob->sex,2)] : (char*)SOMEONE;
		break;
	case ENTITY_MOB_HIS:
		arg->type = ENT_STRING;
		arg->d.str = arg->d.mob ? (char*)his_her[URANGE(0,arg->d.mob->sex,2)] : (char*)SOMEONES;
		break;
	case ENTITY_MOB_HIS_O:
		arg->type = ENT_STRING;
		arg->d.str = arg->d.mob ? (char*)his_hers[URANGE(0,arg->d.mob->sex,2)] : (char*)SOMEONES;
		break;
	case ENTITY_MOB_HIMSELF:
		arg->type = ENT_STRING;
		arg->d.str = arg->d.mob ? (char*)himself[URANGE(0,arg->d.mob->sex,2)] : (char*)SOMEONE;
		break;
	case ENTITY_MOB_RACE:
		arg->type = ENT_STRING;
		arg->d.str = arg->d.mob ? (char*)race_table[arg->d.mob->race].name : "unknown";
		break;
	case ENTITY_MOB_ROOM:
		arg->type = ENT_ROOM;
		arg->d.room = arg->d.mob ? arg->d.mob->in_room : NULL;
		break;
	case ENTITY_MOB_HOUSE:
		arg->type = ENT_ROOM;
		arg->d.room = arg->d.mob && arg->d.mob->home > 0 ? get_room_index(arg->d.mob->home) : NULL;
		break;
	case ENTITY_MOB_CARRYING:
		arg->type = ENT_LIST_OBJ;
		arg->d.list.ptr.obj = self ? &self->carrying : NULL;
		arg->d.list.owner = self;
		break;
	case ENTITY_MOB_TOKENS:
		arg->type = ENT_LIST_TOK;
		arg->d.list.ptr.tok = self ? &self->tokens : NULL;
		arg->d.list.owner = self;
		break;
	case ENTITY_MOB_AFFECTS:
		arg->type = ENT_LIST_AFF;
		arg->d.list.ptr.aff = self ? &self->affected : NULL;
		arg->d.list.owner = self;
		break;
	case ENTITY_MOB_MOUNT:
		arg->d.mob = arg->d.mob ? arg->d.mob->mount : NULL;
		break;
	case ENTITY_MOB_RIDER:
		arg->d.mob = arg->d.mob ? arg->d.mob->rider : NULL;
		break;
	case ENTITY_MOB_MASTER:
		arg->d.mob = arg->d.mob ? arg->d.mob->master : NULL;
		break;
	case ENTITY_MOB_LEADER:
		arg->d.mob = arg->d.mob ? arg->d.mob->leader : NULL;
		break;
	case ENTITY_MOB_OWNER:
		arg->type = ENT_STRING;
		arg->d.str = arg->d.mob && arg->d.mob->owner ? arg->d.mob->owner : &str_empty[0];
		break;
	case ENTITY_MOB_OPPONENT:
		arg->d.mob = arg->d.mob ? arg->d.mob->fighting : NULL;
		break;
	case ENTITY_MOB_CART:
		arg->type = ENT_OBJECT;
		arg->d.obj = arg->d.mob ? arg->d.mob->pulled_cart : NULL;
		break;
	case ENTITY_MOB_FURNITURE:
		arg->type = ENT_OBJECT;
		arg->d.obj = arg->d.mob ? arg->d.mob->on : NULL;
		break;
	case ENTITY_MOB_TARGET:
		arg->d.mob = (arg->d.mob && arg->d.mob->progs) ? arg->d.mob->progs->target : NULL;
		break;
	case ENTITY_MOB_HUNTING:
		arg->d.mob = arg->d.mob ? arg->d.mob->hunting : NULL;
		break;
	case ENTITY_MOB_AREA:
		arg->type = ENT_AREA;
		arg->d.area = arg->d.mob && arg->d.mob->in_room ? arg->d.mob->in_room->area : NULL;
		break;
	case ENTITY_MOB_EQ_LIGHT:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_LIGHT);
		break;
	case ENTITY_MOB_EQ_FINGER1:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_FINGER_L);
		break;
	case ENTITY_MOB_EQ_FINGER2:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_FINGER_R);
		break;
	case ENTITY_MOB_EQ_NECK1:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_NECK_1);
		break;
	case ENTITY_MOB_EQ_NECK2:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_NECK_2);
		break;
	case ENTITY_MOB_EQ_BODY:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_BODY);
		break;
	case ENTITY_MOB_EQ_HEAD:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_HEAD);
		break;
	case ENTITY_MOB_EQ_LEGS:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_LEGS);
		break;
	case ENTITY_MOB_EQ_FEET:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_FEET);
		break;
	case ENTITY_MOB_EQ_HANDS:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_HANDS);
		break;
	case ENTITY_MOB_EQ_ARMS:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_ARMS);
		break;
	case ENTITY_MOB_EQ_SHIELD:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_SHIELD);
		break;
	case ENTITY_MOB_EQ_ABOUT:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_ABOUT);
		break;
	case ENTITY_MOB_EQ_WAIST:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_WAIST);
		break;
	case ENTITY_MOB_EQ_WRIST1:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_WRIST_L);
		break;
	case ENTITY_MOB_EQ_WRIST2:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_WRIST_R);
		break;
	case ENTITY_MOB_EQ_WIELD1:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_WIELD);
		break;
	case ENTITY_MOB_EQ_HOLD:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_HOLD);
		break;
	case ENTITY_MOB_EQ_WIELD2:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_SECONDARY);
		break;
	case ENTITY_MOB_EQ_RING:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_RING_FINGER);
		break;
	case ENTITY_MOB_EQ_BACK:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_BACK);
		break;
	case ENTITY_MOB_EQ_SHOULDER:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_SHOULDER);
		break;
	case ENTITY_MOB_EQ_ANKLE1:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_ANKLE_L);
		break;
	case ENTITY_MOB_EQ_ANKLE2:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_ANKLE_R);
		break;
	case ENTITY_MOB_EQ_EAR1:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_EAR_L);
		break;
	case ENTITY_MOB_EQ_EAR2:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_EAR_R);
		break;
	case ENTITY_MOB_EQ_EYES:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_EYES);
		break;
	case ENTITY_MOB_EQ_FACE:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_FACE);
		break;
	case ENTITY_MOB_EQ_TATTOO_HEAD:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_TATTOO_HEAD);
		break;
	case ENTITY_MOB_EQ_TATTOO_TORSO:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_TATTOO_TORSO);
		break;
	case ENTITY_MOB_EQ_TATTOO_ARM1:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_TATTOO_ARM_L);
		break;
	case ENTITY_MOB_EQ_TATTOO_ARM2:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_TATTOO_ARM_R);
		break;
	case ENTITY_MOB_EQ_TATTOO_LEG1:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_TATTOO_LEG_L);
		break;
	case ENTITY_MOB_EQ_TATTOO_LEG2:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_TATTOO_LEG_R);
		break;
	case ENTITY_MOB_EQ_LODGED_HEAD:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_LODGED_HEAD);
		break;
	case ENTITY_MOB_EQ_LODGED_TORSO:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_LODGED_TORSO);
		break;
	case ENTITY_MOB_EQ_LODGED_ARM1:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_LODGED_ARM_L);
		break;
	case ENTITY_MOB_EQ_LODGED_ARM2:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_LODGED_ARM_R);
		break;
	case ENTITY_MOB_EQ_LODGED_LEG1:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_LODGED_LEG_L);
		break;
	case ENTITY_MOB_EQ_LODGED_LEG2:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_LODGED_LEG_R);
		break;
	case ENTITY_MOB_EQ_ENTANGLED:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_ENTANGLED);
		break;
	case ENTITY_MOB_EQ_CONCEALED:
		arg->type = ENT_OBJECT;
		arg->d.obj = get_eq_char(arg->d.mob,WEAR_CONCEALED);
		break;
	case ENTITY_MOB_NEXT:
		arg->type = ENT_MOBILE;
		arg->d.mob = (arg->d.mob && arg->d.mob->in_room)?arg->d.mob->next_in_room:NULL;
		break;
	case ESCAPE_VARIABLE:
		str = expand_entity_variable((arg->d.mob && IS_NPC(arg->d.mob))?arg->d.mob->progs->vars:NULL,str+1,arg);
		if(!str) return NULL;
		break;
	case ENTITY_MOB_CASTTOKEN:
		arg->type = ENT_TOKEN;
		arg->d.token = arg->d.mob ? arg->d.mob->cast_token: NULL;
		break;
	case ENTITY_MOB_CASTTARGET:
		arg->type = ENT_STRING;
		arg->d.str = arg->d.mob ? (char*)arg->d.mob->cast_target_name : (char*)&str_empty[0];
		break;
	case ENTITY_MOB_RECALL:
		arg->type = ENT_ROOM;
		arg->d.room = (arg->d.mob && location_isset(&arg->d.mob->recall)) ? location_to_room(&arg->d.mob->recall) : NULL;
		break;
	default: return NULL;
	}

	return str+1;
}

char *expand_entity_object(SCRIPT_VARINFO *info,char *str,SCRIPT_PARAM *arg)
{
	OBJ_DATA *self = arg->d.obj;
	switch(*str) {
	case ENTITY_OBJ_NAME:
		arg->type = ENT_STRING;
		arg->d.str = arg->d.obj ? arg->d.obj->name : SOMETHING;
		break;
	case ENTITY_OBJ_SHORT:
		arg->type = ENT_STRING;
		arg->d.str = arg->d.obj ? arg->d.obj->short_descr : SOMETHING;
		break;
	case ENTITY_OBJ_LONG:
		arg->type = ENT_STRING;
		arg->d.str = arg->d.obj ? arg->d.obj->description : &str_empty[0];
		break;
	case ENTITY_OBJ_CONTAINER:
		arg->d.obj = arg->d.obj ? arg->d.obj->in_obj : NULL;
		break;
	case ENTITY_OBJ_FURNITURE:
		arg->d.obj = arg->d.obj ? arg->d.obj->on : NULL;
		break;
	case ENTITY_OBJ_CONTENTS:
		arg->type = ENT_LIST_OBJ;
		arg->d.list.ptr.obj = self ? &self->contains : NULL;
		arg->d.list.owner = self;
		break;
	case ENTITY_OBJ_OWNER:
		arg->type = ENT_STRING;
		arg->d.str = arg->d.mob && arg->d.mob->owner ? arg->d.mob->owner : &str_empty[0];
		break;
	case ENTITY_OBJ_ROOM:
		arg->type = ENT_ROOM;
		arg->d.room = arg->d.obj ? obj_room(arg->d.obj) : NULL;
		break;
	case ENTITY_OBJ_CARRIER:
		arg->type = ENT_MOBILE;
		arg->d.mob = arg->d.obj ? arg->d.obj->carried_by : NULL;
		break;
	case ENTITY_OBJ_TOKENS:
		arg->type = ENT_LIST_TOK;
//		arg->d.list.ptr.tok = self ? &self->tokens : NULL;
		arg->d.list.ptr.tok = NULL;
		arg->d.list.owner = self;
		break;
	case ENTITY_OBJ_TARGET:
		arg->d.mob = (arg->d.obj && arg->d.obj->progs) ? arg->d.obj->progs->target : NULL;
		break;
	case ENTITY_OBJ_AREA:
		arg->type = ENT_AREA;
		arg->d.room = arg->d.obj ? obj_room(arg->d.obj) : NULL;
		arg->d.area = arg->d.room ? arg->d.room->area : NULL;
		break;
	case ENTITY_OBJ_NEXT:
		arg->type = ENT_OBJECT;
		arg->d.obj = arg->d.obj?arg->d.obj->next_content:NULL;
		break;
	case ENTITY_OBJ_EXTRADESC:
		arg->type = ENT_EXTRADESC;
		arg->d.ed = arg->d.obj?arg->d.obj->extra_descr:NULL;
		break;
	case ESCAPE_VARIABLE:
		str = expand_entity_variable(arg->d.obj?arg->d.obj->progs->vars:NULL,str+1,arg);
		if(!str) return NULL;
		break;
	default: return NULL;
	}

	return str+1;
}

char *expand_entity_room(SCRIPT_VARINFO *info,char *str,SCRIPT_PARAM *arg)
{
	ROOM_INDEX_DATA *room = arg->d.room;
	switch(*str) {
	case ENTITY_ROOM_NAME:
		arg->type = ENT_STRING;
		arg->d.str = arg->d.room ? arg->d.room->name : SOMEWHERE;
		break;
	case ENTITY_ROOM_MOBILES:
		arg->type = ENT_LIST_MOB;
		arg->d.list.ptr.mob = room ? &room->people : NULL;
		arg->d.list.owner = room;
		break;
	case ENTITY_ROOM_OBJECTS:
		arg->type = ENT_LIST_OBJ;
		arg->d.list.ptr.obj = room ? &room->contents : NULL;
		arg->d.list.owner = room;
		break;
	case ENTITY_ROOM_TOKENS:
		arg->type = ENT_LIST_TOK;
//		arg->d.list.ptr.tok = room ? &room->tokens : NULL;
		arg->d.list.ptr.tok = NULL;
		arg->d.list.owner = room;
		break;
	case ENTITY_ROOM_AREA:
		arg->type = ENT_AREA;
		arg->d.area = arg->d.room ? arg->d.room->area : NULL;
		break;
	case ENTITY_ROOM_TARGET:
		arg->type = ENT_MOBILE;
		arg->d.mob = (arg->d.room && arg->d.room->progs) ? arg->d.room->progs->target : NULL;
		break;
	case ENTITY_ROOM_NORTH:
		arg->type = ENT_EXIT;
		arg->d.exit = arg->d.room ? arg->d.room->exit[DIR_NORTH] : NULL;
		break;
	case ENTITY_ROOM_EAST:
		arg->type = ENT_EXIT;
		arg->d.exit = arg->d.room ? arg->d.room->exit[DIR_EAST] : NULL;
		break;
	case ENTITY_ROOM_SOUTH:
		arg->type = ENT_EXIT;
		arg->d.exit = arg->d.room ? arg->d.room->exit[DIR_SOUTH] : NULL;
		break;
	case ENTITY_ROOM_WEST:
		arg->type = ENT_EXIT;
		arg->d.exit = arg->d.room ? arg->d.room->exit[DIR_WEST] : NULL;
		break;
	case ENTITY_ROOM_UP:
		arg->type = ENT_EXIT;
		arg->d.exit = arg->d.room ? arg->d.room->exit[DIR_UP] : NULL;
		break;
	case ENTITY_ROOM_DOWN:
		arg->type = ENT_EXIT;
		arg->d.exit = arg->d.room ? arg->d.room->exit[DIR_DOWN] : NULL;
		break;
	case ENTITY_ROOM_NORTHEAST:
		arg->type = ENT_EXIT;
		arg->d.exit = arg->d.room ? arg->d.room->exit[DIR_NORTHEAST] : NULL;
		break;
	case ENTITY_ROOM_NORTHWEST:
		arg->type = ENT_EXIT;
		arg->d.exit = arg->d.room ? arg->d.room->exit[DIR_NORTHWEST] : NULL;
		break;
	case ENTITY_ROOM_SOUTHEAST:
		arg->type = ENT_EXIT;
		arg->d.exit = arg->d.room ? arg->d.room->exit[DIR_SOUTHEAST] : NULL;
		break;
	case ENTITY_ROOM_SOUTHWEST:
		arg->type = ENT_EXIT;
		arg->d.exit = arg->d.room ? arg->d.room->exit[DIR_SOUTHWEST] : NULL;
		break;
	case ENTITY_ROOM_ENVIRON:
		arg->type = ENT_ROOM;
		arg->d.room = get_environment(arg->d.room);
		break;

	case ENTITY_ROOM_ENVIRON_ROOM:
		arg->type = ENT_ROOM;
		arg->d.room = (arg->d.room->environ_type == ENVIRON_ROOM) ? arg->d.room->environ.room : NULL;
		break;

	case ENTITY_ROOM_ENVIRON_MOB:
		arg->type = ENT_MOBILE;
		arg->d.mob = (arg->d.room->environ_type == ENVIRON_MOBILE) ? arg->d.room->environ.mob : NULL;
		break;

	case ENTITY_ROOM_ENVIRON_OBJ:
		arg->type = ENT_OBJECT;
		arg->d.obj = (arg->d.room->environ_type == ENVIRON_OBJECT) ? arg->d.room->environ.obj : NULL;
		break;

	case ENTITY_ROOM_EXTRADESC:
		arg->type = ENT_EXTRADESC;
		arg->d.ed = arg->d.room?arg->d.room->extra_descr:NULL;
		break;

	case ENTITY_ROOM_DESC:
		arg->type = ENT_STRING;
		arg->d.str = arg->d.room ? arg->d.room->description : "";
		break;

	case ESCAPE_VARIABLE:
		str = expand_entity_variable(arg->d.room?arg->d.room->progs->vars:NULL,str+1,arg);
		if(!str) return NULL;
		break;
	default: return NULL;
	}

	return str+1;
}

char *expand_entity_exit(SCRIPT_VARINFO *info,char *str,SCRIPT_PARAM *arg)
{
	ROOM_INDEX_DATA *room;

	switch(*str) {
	case ENTITY_EXIT_NAME:
		arg->type = ENT_STRING;
		arg->d.str = arg->d.exit ? dir_name[arg->d.exit->orig_door] : SOMEWHERE;
		break;
	case ENTITY_EXIT_DOOR:
		arg->type = ENT_NUMBER;
		arg->d.num = arg->d.exit ? arg->d.exit->orig_door : -1;
		break;
	case ENTITY_EXIT_SOURCE:
		arg->type = ENT_ROOM;
		arg->d.room = arg->d.exit ? arg->d.exit->from_room : NULL;
		break;
	case ENTITY_EXIT_REMOTE:
		arg->type = ENT_ROOM;
		arg->d.room = arg->d.exit ? exit_destination(arg->d.exit) : NULL;
		break;
	case ENTITY_EXIT_STATE:
		arg->type = ENT_STRING;
		if(arg->d.exit) {
			int i;
			if(arg->d.exit->exit_info & EX_BROKEN)
				i = 1;
			else if(arg->d.exit->exit_info & EX_CLOSED) {
				i = 2;
				if(arg->d.exit->exit_info & EX_LOCKED) i++;
				if(arg->d.exit->exit_info & EX_BARRED) i+=2;
			} else i = 0;
			arg->d.str = (char*)exit_states[i];
		} else
			arg->d.str = NULL;
		break;
	case ENTITY_EXIT_MATE:
		arg->type = ENT_EXIT;
		if(arg->d.exit) {
			room = exit_destination(arg->d.exit);
			arg->d.exit = room ? room->exit[rev_dir[arg->d.exit->orig_door]] : NULL;
		}
		break;
	case ENTITY_EXIT_NORTH:
	case ENTITY_EXIT_EAST:
	case ENTITY_EXIT_SOUTH:
	case ENTITY_EXIT_WEST:
	case ENTITY_EXIT_UP:
	case ENTITY_EXIT_DOWN:
	case ENTITY_EXIT_NORTHEAST:
	case ENTITY_EXIT_NORTHWEST:
	case ENTITY_EXIT_SOUTHEAST:
	case ENTITY_EXIT_SOUTHWEST:
		arg->type = ENT_EXIT;
		if(arg->d.exit) {
			room = exit_destination(arg->d.exit);
			arg->d.exit = room ? room->exit[*str - ENTITY_EXIT_NORTH + DIR_NORTH] : NULL;
		}
		break;
	case ENTITY_EXIT_NEXT:
		arg->type = ENT_EXIT;
		if(arg->d.exit) {
			int door = arg->d.exit->orig_door + 1;
			ROOM_INDEX_DATA *room = arg->d.exit->from_room;

			if(room) {
				for(;door < MAX_DIR && !room->exit[door];door++);

				if(door < MAX_DIR) {
					arg->d.exit = room->exit[door];
					break;
				}
			}

			arg->d.exit = NULL;
		}
		break;
	default: return NULL;
	}

	return str+1;
}

char *expand_entity_token(SCRIPT_VARINFO *info,char *str,SCRIPT_PARAM *arg)
{
	switch(*str) {
	case ENTITY_TOKEN_NAME:
		arg->type = ENT_STRING;
		arg->d.str = arg->d.token ? arg->d.token->name : &str_empty[0];
		break;
	case ENTITY_TOKEN_OWNER:
		arg->type = ENT_MOBILE;
		arg->d.mob = arg->d.token ? arg->d.token->player : NULL;
		break;
	case ENTITY_TOKEN_TIMER:
		arg->type = ENT_NUMBER;
		arg->d.num = arg->d.token ? arg->d.token->timer : 0;
		break;
	case ENTITY_TOKEN_VAL0:
		arg->type = ENT_NUMBER;
		arg->d.num = arg->d.token ? arg->d.token->value[0] : 0;
		break;
	case ENTITY_TOKEN_VAL1:
		arg->type = ENT_NUMBER;
		arg->d.num = arg->d.token ? arg->d.token->value[1] : 0;
		break;
	case ENTITY_TOKEN_VAL2:
		arg->type = ENT_NUMBER;
		arg->d.num = arg->d.token ? arg->d.token->value[2] : 0;
		break;
	case ENTITY_TOKEN_VAL3:
		arg->type = ENT_NUMBER;
		arg->d.num = arg->d.token ? arg->d.token->value[3] : 0;
		break;
	case ENTITY_TOKEN_NEXT:
		arg->type = ENT_TOKEN;
		arg->d.token = arg->d.token?arg->d.token->next:NULL;
		break;
	case ESCAPE_VARIABLE:
		str = expand_entity_variable(arg->d.token?arg->d.token->progs->vars:NULL,str+1,arg);
		if(!str) return NULL;
		break;
	default: return NULL;
	}

	return str+1;
}

char *expand_entity_area(SCRIPT_VARINFO *info,char *str,SCRIPT_PARAM *arg)
{
	switch(*str) {
	case ENTITY_AREA_NAME:
		arg->type = ENT_STRING;
		arg->d.str = arg->d.area ? arg->d.area->name : SOMEWHERE;
		break;
	case ENTITY_AREA_RECALL:
		arg->type = ENT_ROOM;
		arg->d.room = (arg->d.area && location_isset(&arg->d.area->recall)) ? location_to_room(&arg->d.area->recall) : NULL;
		break;
	case ENTITY_AREA_POSTOFFICE:
		arg->type = ENT_ROOM;
		arg->d.room = (arg->d.area && arg->d.area->post_office > 0) ? get_room_index(arg->d.area->post_office) : NULL;
		break;
	default: return NULL;
	}

	return str+1;
}

char *expand_entity_list_mob(SCRIPT_VARINFO *info,char *str,SCRIPT_PARAM *arg)
{
	register CHAR_DATA *mob;
	register int count;
	switch(*str) {
	case ENTITY_LIST_SIZE:
		arg->type = ENT_NUMBER;
		count = 0;
		if(arg->d.list.ptr.mob)
			for(mob = *arg->d.list.ptr.mob;mob;mob = mob->next_in_room) count++;
		arg->d.num = count;
		break;
	case ENTITY_LIST_RANDOM:
		if(arg->d.list.ptr.mob) {
			count = 0;
			for(mob = *arg->d.list.ptr.mob;mob;mob = mob->next_in_room) count++;
			if(count > 0) {
				count = number_range(1,count);
				for(mob = *arg->d.list.ptr.mob;count-- > 0;mob = mob->next_in_room);
				arg->d.mob = mob;
			} else
				arg->d.mob = NULL;
		} else
			arg->d.mob = NULL;
		arg->type = ENT_MOBILE;
		break;
	case ENTITY_LIST_FIRST:
		arg->d.mob = arg->d.list.ptr.mob ? *arg->d.list.ptr.mob : NULL;
		arg->type = ENT_MOBILE;
		break;
	case ENTITY_LIST_LAST:
		if(arg->d.list.ptr.mob) {
			for(mob = *arg->d.list.ptr.mob;mob && mob->next_in_room;mob = mob->next_in_room);
			arg->d.mob = mob;
		} else
			arg->d.mob = NULL;
		arg->type = ENT_MOBILE;
		break;
	default: return NULL;
	}

	return str+1;
}

char *expand_entity_list_obj(SCRIPT_VARINFO *info,char *str,SCRIPT_PARAM *arg)
{
	register OBJ_DATA *obj;
	register int count;
	switch(*str) {
	case ENTITY_LIST_SIZE:
		arg->type = ENT_NUMBER;
		count = 0;
		if(arg->d.list.ptr.obj)
			for(obj = *arg->d.list.ptr.obj;obj;obj = obj->next_content) count++;
		arg->d.num = count;
		break;
	case ENTITY_LIST_RANDOM:
		if(arg->d.list.ptr.obj) {
			count = 0;
			for(obj = *arg->d.list.ptr.obj;obj;obj = obj->next_content) count++;
			if(count > 0) {
				count = number_range(1,count);
				for(obj = *arg->d.list.ptr.obj;count-- > 0;obj = obj->next_content);
				arg->d.obj = obj;
			} else
				arg->d.obj = NULL;
		} else
			arg->d.obj = NULL;
		arg->type = ENT_OBJECT;
		break;
	case ENTITY_LIST_FIRST:
		arg->d.obj = arg->d.list.ptr.obj ? *arg->d.list.ptr.obj : NULL;
		arg->type = ENT_OBJECT;
		break;
	case ENTITY_LIST_LAST:
		if(arg->d.list.ptr.obj) {
			for(obj = *arg->d.list.ptr.obj;obj && obj->next_content;obj = obj->next_content);
			arg->d.obj = obj;
		} else
			arg->d.obj = NULL;
		arg->type = ENT_OBJECT;
		break;
	default: return NULL;
	}

	return str+1;
}

char *expand_entity_list_token(SCRIPT_VARINFO *info,char *str,SCRIPT_PARAM *arg)
{
	register TOKEN_DATA *token;
	register int count;
	switch(*str) {
	case ENTITY_LIST_SIZE:
		arg->type = ENT_NUMBER;
		count = 0;
		if(arg->d.list.ptr.tok)
			for(token = *arg->d.list.ptr.tok;token;token = token->next) count++;
		arg->d.num = count;
		break;
	case ENTITY_LIST_RANDOM:
		if(arg->d.list.ptr.tok) {
			count = 0;
			for(token = *arg->d.list.ptr.tok;token;token = token->next) count++;
			if(count > 0) {
				count = number_range(1,count);
				for(token = *arg->d.list.ptr.tok;count-- > 0;token = token->next);
				arg->d.token = token;
			} else
				arg->d.token = NULL;
		} else
			arg->d.token = NULL;
		arg->type = ENT_TOKEN;
		break;
	case ENTITY_LIST_FIRST:
		arg->d.token = arg->d.list.ptr.tok ? *arg->d.list.ptr.tok : NULL;
		arg->type = ENT_TOKEN;
		break;
	case ENTITY_LIST_LAST:
		if(arg->d.list.ptr.tok) {
			for(token = *arg->d.list.ptr.tok;token && token->next;token = token->next);
			arg->d.token = token;
		} else
			arg->d.token = NULL;
		arg->type = ENT_TOKEN;
		break;
	default: return NULL;
	}

	return str+1;
}

char *expand_entity_list_affect(SCRIPT_VARINFO *info,char *str,SCRIPT_PARAM *arg)
{
	register AFFECT_DATA *affect;
	register int count;
	switch(*str) {
	case ENTITY_LIST_SIZE:
		arg->type = ENT_NUMBER;
		count = 0;
		if(arg->d.list.ptr.aff)
			for(affect = *arg->d.list.ptr.aff;affect;affect = affect->next) count++;
		arg->d.num = count;
		break;
	case ENTITY_LIST_RANDOM:
		if(arg->d.list.ptr.aff) {
			count = 0;
			for(affect = *arg->d.list.ptr.aff;affect;affect = affect->next) count++;
			if(count > 0) {
				count = number_range(1,count);
				for(affect = *arg->d.list.ptr.aff;count-- > 0;affect = affect->next);
				arg->d.aff = affect;
			} else
				arg->d.aff = NULL;
		} else
			arg->d.aff = NULL;
		arg->type = ENT_AFFECT;
		break;
	case ENTITY_LIST_FIRST:
		arg->d.aff = arg->d.list.ptr.aff ? *arg->d.list.ptr.aff : NULL;
		arg->type = ENT_AFFECT;
		break;
	case ENTITY_LIST_LAST:
		if(arg->d.list.ptr.aff) {
			for(affect = *arg->d.list.ptr.aff;affect && affect->next;affect = affect->next);
			arg->d.aff = affect;
		} else
			arg->d.aff = NULL;
		arg->type = ENT_AFFECT;
		break;
	default: return NULL;
	}

	return str+1;
}


char *expand_entity_skill(SCRIPT_VARINFO *info,char *str,SCRIPT_PARAM *arg)
{
	switch(*str) {
	case ENTITY_SKILL_GSN:
		arg->type = ENT_NUMBER;
		arg->d.num = arg->d.sn;	// Redundant!
		break;

	case ENTITY_SKILL_SPELL:
		arg->type = ENT_NUMBER;
		arg->d.num = (arg->d.sn >= 0 && arg->d.sn < MAX_SKILL && skill_table[arg->d.sn].spell_fun && skill_table[arg->d.sn].spell_fun != spell_null) ? 1 : 0;
		break;

	case ENTITY_SKILL_NAME:
		arg->type = ENT_STRING;
		strcpy(arg->buf,(arg->d.sn >= 0 && arg->d.sn < MAX_SKILL && skill_table[arg->d.sn].name) ? skill_table[arg->d.sn].name : "");
		arg->d.str = arg->buf;
		break;

	case ENTITY_SKILL_BEATS:
		arg->type = ENT_NUMBER;
		arg->d.num = (arg->d.sn >= 0 && arg->d.sn < MAX_SKILL) ? skill_table[arg->d.sn].beats : 0;
		break;

	case ENTITY_SKILL_LEVEL_WARRIOR:
		arg->type = ENT_NUMBER;
		arg->d.num = (arg->d.sn >= 0 && arg->d.sn < MAX_SKILL) ? skill_table[arg->d.sn].skill_level[CLASS_WARRIOR] : 0;
		break;

	case ENTITY_SKILL_LEVEL_CLERIC:
		arg->type = ENT_NUMBER;
		arg->d.num = (arg->d.sn >= 0 && arg->d.sn < MAX_SKILL) ? skill_table[arg->d.sn].skill_level[CLASS_CLERIC] : 0;
		break;

	case ENTITY_SKILL_LEVEL_MAGE:
		arg->type = ENT_NUMBER;
		arg->d.num = (arg->d.sn >= 0 && arg->d.sn < MAX_SKILL) ? skill_table[arg->d.sn].skill_level[CLASS_MAGE] : 0;
		break;

	case ENTITY_SKILL_LEVEL_THIEF:
		arg->type = ENT_NUMBER;
		arg->d.num = (arg->d.sn >= 0 && arg->d.sn < MAX_SKILL) ? skill_table[arg->d.sn].skill_level[CLASS_THIEF] : 0;
		break;


	case ENTITY_SKILL_DIFFICULTY_WARRIOR:
		arg->type = ENT_NUMBER;
		arg->d.num = (arg->d.sn >= 0 && arg->d.sn < MAX_SKILL) ? skill_table[arg->d.sn].rating[CLASS_WARRIOR] : 0;
		break;

	case ENTITY_SKILL_DIFFICULTY_CLERIC:
		arg->type = ENT_NUMBER;
		arg->d.num = (arg->d.sn >= 0 && arg->d.sn < MAX_SKILL) ? skill_table[arg->d.sn].rating[CLASS_CLERIC] : 0;
		break;

	case ENTITY_SKILL_DIFFICULTY_MAGE:
		arg->type = ENT_NUMBER;
		arg->d.num = (arg->d.sn >= 0 && arg->d.sn < MAX_SKILL) ? skill_table[arg->d.sn].rating[CLASS_MAGE] : 0;
		break;

	case ENTITY_SKILL_DIFFICULTY_THIEF:
		arg->type = ENT_NUMBER;
		arg->d.num = (arg->d.sn >= 0 && arg->d.sn < MAX_SKILL) ? skill_table[arg->d.sn].rating[CLASS_THIEF] : 0;
		break;

	case ENTITY_SKILL_TARGET:
		arg->type = ENT_NUMBER;
		arg->d.num = (arg->d.sn >= 0 && arg->d.sn < MAX_SKILL) ? skill_table[arg->d.sn].target : 0;
		break;

	case ENTITY_SKILL_POSITION:
		arg->type = ENT_NUMBER;
		arg->d.num = (arg->d.sn >= 0 && arg->d.sn < MAX_SKILL) ? skill_table[arg->d.sn].minimum_position : POS_DEAD;
		break;

	case ENTITY_SKILL_NOUN:
		arg->type = ENT_STRING;
		strcpy(arg->buf,(arg->d.sn >= 0 && arg->d.sn < MAX_SKILL && skill_table[arg->d.sn].noun_damage) ? skill_table[arg->d.sn].noun_damage: "");
		arg->d.str = arg->buf;
		break;

	case ENTITY_SKILL_WEAROFF:
		arg->type = ENT_STRING;
		strcpy(arg->buf,(arg->d.sn >= 0 && arg->d.sn < MAX_SKILL && skill_table[arg->d.sn].msg_off) ? skill_table[arg->d.sn].msg_off : "");
		arg->d.str = arg->buf;
		break;

	case ENTITY_SKILL_DISPEL:
		arg->type = ENT_STRING;
		strcpy(arg->buf,(arg->d.sn >= 0 && arg->d.sn < MAX_SKILL && skill_table[arg->d.sn].msg_disp) ? skill_table[arg->d.sn].msg_disp : "");
		arg->d.str = arg->buf;
		break;

	case ENTITY_SKILL_MANA:
		arg->type = ENT_NUMBER;
		arg->d.num = (arg->d.sn >= 0 && arg->d.sn < MAX_SKILL) ? skill_table[arg->d.sn].min_mana : 0;
		break;

	case ENTITY_SKILL_INK_TYPE1:
		arg->type = ENT_NUMBER;
		arg->d.num = (arg->d.sn >= 0 && arg->d.sn < MAX_SKILL) ? skill_table[arg->d.sn].inks[0][0] : 0;
		break;

	case ENTITY_SKILL_INK_TYPE2:
		arg->type = ENT_NUMBER;
		arg->d.num = (arg->d.sn >= 0 && arg->d.sn < MAX_SKILL) ? skill_table[arg->d.sn].inks[1][0] : 0;
		break;

	case ENTITY_SKILL_INK_TYPE3:
		arg->type = ENT_NUMBER;
		arg->d.num = (arg->d.sn >= 0 && arg->d.sn < MAX_SKILL) ? skill_table[arg->d.sn].inks[2][0] : 0;
		break;

	case ENTITY_SKILL_INK_SIZE1:
		arg->type = ENT_NUMBER;
		arg->d.num = (arg->d.sn >= 0 && arg->d.sn < MAX_SKILL) ? skill_table[arg->d.sn].inks[0][1] : 0;
		break;

	case ENTITY_SKILL_INK_SIZE2:
		arg->type = ENT_NUMBER;
		arg->d.num = (arg->d.sn >= 0 && arg->d.sn < MAX_SKILL) ? skill_table[arg->d.sn].inks[1][1] : 0;
		break;

	case ENTITY_SKILL_INK_SIZE3:
		arg->type = ENT_NUMBER;
		arg->d.num = (arg->d.sn >= 0 && arg->d.sn < MAX_SKILL) ? skill_table[arg->d.sn].inks[2][1] : 0;
		break;

	default: return NULL;
	}

	return str+1;
}

char *expand_entity_skillinfo(SCRIPT_VARINFO *info,char *str,SCRIPT_PARAM *arg)
{
	switch(*str) {
	case ENTITY_SKILLINFO_SKILL:
		arg->type = ENT_SKILL;
		arg->d.sn = arg->d.sk.sn;
		break;

	case ENTITY_SKILLINFO_OWNER:
		arg->type = ENT_MOBILE;
		arg->d.mob = arg->d.sk.owner;
		break;

	case ENTITY_SKILLINFO_RATING:
		arg->type = ENT_NUMBER;
		arg->d.num = (arg->d.sk.owner) ? get_skill(arg->d.sk.owner,arg->d.sk.sn) : 0;
		break;

	default: return NULL;
	}

	return str+1;
}

char *expand_entity_affect(SCRIPT_VARINFO *info,char *str,SCRIPT_PARAM *arg)
{
	printf("expand_entity_affect() called\n\r");
	switch(*str) {
	case ENTITY_AFFECT_NAME:
		arg->type = ENT_STRING;
		if(arg->d.aff) {
			if(arg->d.aff->custom_name)
				arg->d.str = arg->d.aff->custom_name;
			else
				arg->d.str = skill_table[arg->d.aff->type].name;
		} else
			arg->d.str = &str_empty[0];
		printf("expand_entity_affect(NAME)-> \"%s\"\n\r", arg->d.str);
		break;

	case ENTITY_AFFECT_GROUP:
		arg->type = ENT_NUMBER;
		arg->d.num = arg->d.aff ? arg->d.aff->group : -1;
		break;

	case ENTITY_AFFECT_SKILL:
		arg->type = ENT_NUMBER;
		arg->d.num = arg->d.aff ? arg->d.aff->type : 0;
		break;

	case ENTITY_AFFECT_LOCATION:
		arg->type = ENT_NUMBER;
		arg->d.num = arg->d.aff ? arg->d.aff->location : -1;
		break;

	case ENTITY_AFFECT_MOD:
		arg->type = ENT_NUMBER;
		arg->d.num = arg->d.aff ? arg->d.aff->modifier : 0;
		break;

	case ENTITY_AFFECT_LEVEL:
		arg->type = ENT_NUMBER;
		arg->d.num = arg->d.aff ? arg->d.aff->level : 0;
		break;

	case ENTITY_AFFECT_TIMER:
		arg->type = ENT_NUMBER;
		arg->d.num = arg->d.aff ? arg->d.aff->duration : 0;
		break;

	default: return NULL;
	}

	return str+1;
}


char *expand_entity_prior(SCRIPT_VARINFO *info,char *str,SCRIPT_PARAM *arg)
{
	info = arg->d.info;

	switch(*str) {
	case ENTITY_PRIOR_MOB:
		arg->type = ENT_MOBILE;
		arg->d.mob = info ? info->mob : NULL;
		break;

	case ENTITY_PRIOR_OBJ:
		arg->type = ENT_OBJECT;
		arg->d.obj = info ? info->obj : NULL;
		break;

	case ENTITY_PRIOR_ROOM:
		arg->type = ENT_ROOM;
		arg->d.room = info ? info->room : NULL;
		break;

	case ENTITY_PRIOR_TOKEN:
		arg->type = ENT_TOKEN;
		arg->d.token = info ? info->token : NULL;
		break;

	case ENTITY_PRIOR_ENACTOR:
		arg->type = ENT_MOBILE;
		arg->d.mob = info->ch;
		break;
	case ENTITY_PRIOR_OBJ1:
		arg->type = ENT_OBJECT;
		arg->d.obj = info->obj1;
		break;
	case ENTITY_PRIOR_OBJ2:
		arg->type = ENT_OBJECT;
		arg->d.obj = info->obj2;
		break;
	case ENTITY_PRIOR_VICTIM:
		arg->type = ENT_MOBILE;
		arg->d.mob = info->vch;
		break;
	case ENTITY_PRIOR_TARGET:
		arg->type = ENT_MOBILE;
		arg->d.mob = info->targ ? *info->targ : NULL;
		break;
	case ENTITY_PRIOR_RANDOM:
		arg->type = ENT_MOBILE;
		arg->d.mob = info->rch;
		break;
	case ENTITY_PRIOR_HERE:
		if(info->mob)
			arg->d.room = info->mob->in_room;
		else if(info->obj)
			arg->d.room = obj_room(info->obj);
		else if(info->room)
			arg->d.room = info->room;
		else if(info->token)
			arg->d.room = token_room(info->token);
		else return NULL;
		arg->type = ENT_ROOM;
		break;

	case ENTITY_PRIOR_PHRASE:
		arg->type = ENT_STRING;
		arg->d.str = info->phrase;
		break;
	case ENTITY_PRIOR_TRIGGER:
		arg->type = ENT_STRING;
		arg->d.str = info->trigger;
		break;

	case ENTITY_PRIOR_PRIOR:
		arg->type = ENT_PRIOR;
		arg->d.info = script_get_prior(arg->d.info);
		break;

	default: return NULL;
	}

	return str+1;
}

char *expand_entity_extradesc(SCRIPT_VARINFO *info,char *str,SCRIPT_PARAM *arg)
{
//	EXTRA_DESCR_DATA *ed;
	char buf[MSL];
	switch(*str) {
	case ESCAPE_VARIABLE:
		if(!arg->d.ed || !info->var) return NULL;

/*
		{
			char store[MSL];
			char msg[MSL*2];
			expand_escape2print(str,store);
			sprintf(msg,"expand_entity_extradesc->str = \"%s\"",store);
			wiznet(msg,NULL,NULL,WIZ_SCRIPTS,0,0);
		}
*/

		wiznet_variables = TRUE;
		str = expand_name(*info->var,str+1,buf);
		wiznet_variables = FALSE;
		if(!str) return NULL;

/*
		{
			char msg[MSL*2];
			sprintf(msg,"expand_entity_extradesc->\"%s\"",buf);
			wiznet(msg,NULL,NULL,WIZ_SCRIPTS,0,0);
		}
*/

		arg->type = ENT_STRING;
		arg->d.str = get_extra_descr(buf, arg->d.ed);
		if (!arg->d.str) arg->d.str = str_dup("");
		break;
	default: return NULL;
	}

	return str+1;
}

char *expand_argument_entity(SCRIPT_VARINFO *info,char *str,SCRIPT_PARAM *arg)
{
	char *next;
	arg->type = ENT_NONE;
	arg->d.num = 0;

	while(str && *str && *str != ESCAPE_END) {
		switch(arg->type) {
		case ENT_PRIMARY:	next = expand_entity_primary(info,str,arg); break;
		case ENT_NUMBER:	next = expand_entity_number(info,str,arg); break;
		case ENT_STRING:	next = expand_entity_string(info,str,arg); break;
		case ENT_MOBILE:	next = expand_entity_mobile(info,str,arg); break;
		case ENT_OBJECT:	next = expand_entity_object(info,str,arg); break;
		case ENT_ROOM:		next = expand_entity_room(info,str,arg); break;
		case ENT_EXIT:		next = expand_entity_exit(info,str,arg); break;
		case ENT_TOKEN:		next = expand_entity_token(info,str,arg); break;
		case ENT_AREA:		next = expand_entity_area(info,str,arg); break;
		case ENT_LIST_MOB:	next = expand_entity_list_mob(info,str,arg); break;
		case ENT_LIST_OBJ:	next = expand_entity_list_obj(info,str,arg); break;
		case ENT_LIST_TOK:	next = expand_entity_list_token(info,str,arg); break;
		case ENT_LIST_AFF:	next = expand_entity_list_affect(info,str,arg); break;
		case ENT_SKILL:		next = expand_entity_skill(info,str,arg); break;
		case ENT_SKILLINFO:	next = expand_entity_skillinfo(info,str,arg); break;
		case ENT_EXTRADESC:	next = expand_entity_extradesc(info,str,arg); break;
		case ENT_AFFECT:	next = expand_entity_affect(info,str,arg); break;
		case ENT_PRIOR:		next = expand_entity_prior(info,str,arg); break;
		default:		next = NULL; break;
		}

		if(next) str = next;
		else {
			str = expand_skip(str);
			arg->type = ENT_NONE;
			arg->d.num = 0;
		}
	}
	return str && *str ? str+1 : str;
}

char *expand_string_entity(SCRIPT_VARINFO *info,char *str,char **store)
{
	SCRIPT_PARAM arg;

	str = expand_argument_entity(info,str,&arg);
	if(!str || arg.type == ENT_NONE) return NULL;

	switch(arg.type) {
	default:		strcpy(*store," {D<{x@{W@{x@{D>{x "); *store += 19;	break;	// Don't display anything
	case ENT_NUMBER:	*store += sprintf(*store,"%d", arg.d.num); break;
	case ENT_STRING:	*store += sprintf(*store,"%s", arg.d.str ? arg.d.str : "(null)"); break;
	case ENT_MOBILE:	*store += sprintf(*store,"%s", arg.d.mob ? (IS_NPC(arg.d.mob) ? arg.d.mob->short_descr : arg.d.mob->name) : SOMEONE); break;
	case ENT_OBJECT:	*store += sprintf(*store,"%s", arg.d.obj ? arg.d.obj->short_descr : SOMETHING); break;
	case ENT_ROOM:		*store += sprintf(*store,"%s", arg.d.room ? arg.d.room->name : SOMEWHERE); break;
	case ENT_EXIT:		*store += sprintf(*store,"%s", arg.d.exit ? dir_name[arg.d.exit->orig_door] : SOMEWHERE); break;
	case ENT_TOKEN:		*store += sprintf(*store,"%s", (arg.d.token && arg.d.token->pIndexData) ? arg.d.token->pIndexData->name : SOMETHING); break;
	case ENT_AREA:		*store += sprintf(*store,"%s", arg.d.area ? arg.d.area->name : SOMEWHERE); break;
	}

	return str;
}

void expand_string_simple_code(SCRIPT_VARINFO *info,unsigned char code,char **store)
{
	char buf[MIL], *s = buf;

//	sprintf(buf,"expand_string_simple_code: code = %2.2X", code);
//	wiznet(buf,NULL,NULL,WIZ_TESTING,0,0);

	buf[0] = '\0';
	switch(code) {
	default:
		s = " {W<{x@{D@{x@{W>{x "; break;
//	case ESCAPE_UA:
//	case ESCAPE_UB:
//	case ESCAPE_UC:
//	case ESCAPE_UD:
	case ESCAPE_UE:
		if(info->vch && (!info->mob || can_see(info->mob,info->vch)))
			s = (char*)he_she[URANGE(0, info->vch->sex, 2)];
		else
			s = SOMEONE;
		break;
	case ESCAPE_UF:
		if(info->mob) s = (char*)himself[URANGE(0, info->mob->sex, 2)];
		else s = SOMEONE;
		break;
//	case ESCAPE_UG:
//	case ESCAPE_UH:
	case ESCAPE_UI:
		if(info->mob) s = info->mob->short_descr;
		else if(info->obj) s = info->obj->short_descr;
		else if(info->room) s = info->room->name;
		else if(info->token) s = info->token->pIndexData->name;
		break;
	case ESCAPE_UJ:
		if (!info->rch) info->rch = get_random_char(info->mob, info->obj, info->room, info->token);
		if(info->rch && (!info->mob || can_see(info->mob,info->rch)))
			s = (char*)he_she[URANGE(0, info->rch->sex, 2)];
		else
			s = SOMEONE;
		break;
	case ESCAPE_UK:
		if (!info->rch) info->rch = get_random_char(info->mob, info->obj, info->room, info->token);
		if(info->rch && (!info->mob || can_see(info->mob,info->rch)))
			s = (char*)him_her[URANGE(0, info->rch->sex, 2)];
		else
			s = SOMEONE;
		break;
	case ESCAPE_UL:
		if (!info->rch) info->rch = get_random_char(info->mob, info->obj, info->room, info->token);
		if(info->rch && (!info->mob || can_see(info->mob,info->rch)))
			s = (char*)his_hers[URANGE(0, info->rch->sex, 2)];
		else
			s = SOMEONES;
		break;
	case ESCAPE_UM:
		if(info->vch && (!info->mob || can_see(info->mob,info->vch)))
			s = (char*)him_her[URANGE(0, info->vch->sex, 2)];
		else
			s = SOMEONE;
		break;
	case ESCAPE_UN:
		if(info->ch && (!info->mob || can_see(info->mob,info->ch)))
			s = ((IS_NPC(info->ch) || info->ch->morphed) ? info->ch->short_descr : capitalize(info->ch->name));
		else
			s = SOMEONE;
		break;
	case ESCAPE_UO:
		if(info->obj1 && (!info->mob || can_see_obj(info->mob,info->obj1)))
			s = info->obj1->short_descr;
		else
			s = SOMETHING;
		break;
	case ESCAPE_UP:
		if(info->obj2 && (!info->mob || can_see_obj(info->mob,info->obj2)))
			s = info->obj2->short_descr;
		else
			s = SOMETHING;
		break;
	case ESCAPE_UQ:
		if(info->targ && (*info->targ) && (!info->mob || can_see(info->mob,(*info->targ))))
			s = ((IS_NPC((*info->targ)) || (*info->targ)->morphed) ? (*info->targ)->short_descr : capitalize((*info->targ)->name));
		else
			s = SOMEONE;
		break;
	case ESCAPE_UR:
		if (!info->rch) info->rch = get_random_char(info->mob, info->obj, info->room, info->token);
		if(info->rch && (!info->mob || can_see(info->mob,info->rch)))
			s = ((IS_NPC(info->rch) || info->rch->morphed) ? info->rch->short_descr : capitalize(info->rch->name));
		else
			s = SOMEONE;
		break;
	case ESCAPE_US:
		if(info->vch && (!info->mob || can_see(info->mob,info->vch)))
			s = (char*)his_her[URANGE(0, info->vch->sex, 2)];
		else
			s = SOMEONES;
		break;
	case ESCAPE_UT:
		if(info->vch && (!info->mob || can_see(info->mob,info->vch)))
			s = ((IS_NPC(info->vch) || info->vch->morphed) ? info->vch->short_descr : capitalize(info->vch->name));
		else
			s = SOMEONE;
		break;
	case ESCAPE_UU:
		if(info->vch && (!info->mob || can_see(info->mob,info->vch)))
			s = (char*)his_hers[URANGE(0, info->vch->sex, 2)];
		else
			s = SOMEONES;
		break;
	case ESCAPE_UV:
		if (!info->rch) info->rch = get_random_char(info->mob, info->obj, info->room, info->token);
		if(info->rch && (!info->mob || can_see(info->mob,info->rch)))
			s = (char*)his_hers[URANGE(0, info->rch->sex, 2)];
		else
			s = SOMEONE;
		break;
//	case ESCAPE_UW:
	case ESCAPE_UX:
		if(info->targ && (*info->targ) && (!info->mob || can_see(info->mob,(*info->targ))))
			s = (char*)he_she[URANGE(0, (*info->targ)->sex, 2)];
		else
			s = SOMEONE;
		break;
	case ESCAPE_UY:
		if(info->targ && (*info->targ) && (!info->mob || can_see(info->mob,(*info->targ))))
			s = (char*)him_her[URANGE(0, (*info->targ)->sex, 2)];
		else
			s = SOMEONE;
		break;
	case ESCAPE_UZ:
		if(info->targ && (*info->targ) && (!info->mob || can_see(info->mob,(*info->targ))))
			s = (char*)his_her[URANGE(0, (*info->targ)->sex, 2)];
		else
			s = SOMEONES;
		break;
//	case ESCAPE_LA:
//	case ESCAPE_LB:
//	case ESCAPE_LC:
//	case ESCAPE_LD:
	case ESCAPE_LE:
		if(info->ch && (!info->mob || can_see(info->mob,info->ch)))
			s = (char*)he_she[URANGE(0, info->ch->sex, 2)];
		else
			s = SOMEONE;
		break;
	case ESCAPE_LF:
		if(info->ch && (!info->mob || can_see(info->mob,info->ch)))
			s = (char*)himself[URANGE(0, info->ch->sex, 2)];
		else
			s = SOMEONE;
		break;
//	case ESCAPE_LG:
//	case ESCAPE_LH:
	case ESCAPE_LI:
		if(info->mob) one_argument(info->mob->name,buf);
		else if(info->obj) one_argument(info->obj->name,buf);
		else if(info->room) sprintf(buf,"%d",(int)info->room->vnum);
		else if(info->token) sprintf(buf,"%d",(int)info->token->pIndexData->vnum);
		break;
	case ESCAPE_LJ:
		if(info->mob) s = (char*)he_she[URANGE(0, info->mob->sex, 2)];
		else s = SOMEONE;
		break;
	case ESCAPE_LK:
		if(info->mob) s = (char*)him_her[URANGE(0, info->mob->sex, 2)];
		else s = SOMEONE;
		break;
	case ESCAPE_LL:
		if(info->mob) s = (char*)his_her[URANGE(0, info->mob->sex, 2)];
		else s = SOMEONES;
		break;
	case ESCAPE_LM:
		if(info->ch && (!info->mob || can_see(info->mob,info->ch)))
			s = (char*)him_her[URANGE(0, info->ch->sex, 2)];
		else
			s = SOMEONE;
		break;
	case ESCAPE_LN:
		if(info->ch && (!info->mob || can_see(info->mob,info->ch)))
			one_argument(info->ch->name,buf);
		else
			s = SOMEONE;
		break;
	case ESCAPE_LO:
		if(info->obj1 && (!info->mob || can_see_obj(info->mob,info->obj1)))
			one_argument(info->obj1->name,buf);
		else
			s = SOMETHING;
		break;
	case ESCAPE_LP:
		if(info->obj2 && (!info->mob || can_see_obj(info->mob,info->obj2)))
			one_argument(info->obj2->name,buf);
		else
			s = SOMETHING;
		break;
	case ESCAPE_LQ:
		if(info->targ && (*info->targ) && (!info->mob || can_see(info->mob,(*info->targ))))
			one_argument((*info->targ)->name,buf);
		else
			s = SOMEONE;
		break;
	case ESCAPE_LR:
		if (!info->rch) info->rch = get_random_char(info->mob, info->obj, info->room, info->token);
		if(info->rch && (!info->mob || can_see(info->mob,info->rch)))
			one_argument(info->rch->name,buf);
		else
			s = SOMEONE;
		break;
	case ESCAPE_LS:
		if(info->ch && (!info->mob || can_see(info->mob,info->ch)))
			s = (char*)his_her[URANGE(0, info->ch->sex, 2)];
		else
			s = SOMEONES;
		break;
	case ESCAPE_LT:
		if(info->vch && (!info->mob || can_see(info->mob,info->vch)))
			one_argument(info->vch->name,buf);
		else
			s = SOMEONE;
		break;
	case ESCAPE_LU:
		if(info->ch && (!info->mob || can_see(info->mob,info->ch)))
			s = (char*)his_hers[URANGE(0, info->ch->sex, 2)];
		else
			s = SOMEONE;
		break;
	case ESCAPE_LV:
		if(info->mob) s = (char*)his_hers[URANGE(0, info->mob->sex, 2)];
		else s = SOMEONES;
		break;
	case ESCAPE_LW:
		if(info->targ && (*info->targ) && (!info->mob || can_see(info->mob,(*info->targ))))
			s = (char*)his_hers[URANGE(0, (*info->targ)->sex, 2)];
		else
			s = SOMEONE;
		break;
	case ESCAPE_LX:
		if(info->vch && (!info->mob || can_see(info->mob,info->vch)))
			s = (char*)himself[URANGE(0, info->vch->sex, 2)];
		else
			s = SOMEONE;
		break;
	case ESCAPE_LY:
		if (!info->rch) info->rch = get_random_char(info->mob, info->obj, info->room, info->token);
		if(info->rch && (!info->mob || can_see(info->mob,info->rch)))
			s = (char*)himself[URANGE(0, info->rch->sex, 2)];
		else
			s = SOMEONE;
		break;
	case ESCAPE_LZ:
		if(info->targ && (*info->targ) && (!info->mob || can_see(info->mob,(*info->targ))))
			s = (char*)himself[URANGE(0, (*info->targ)->sex, 2)];
		else
			s = SOMEONE;
		break;
	}

	if(*s) {
		strcpy(*store,s);
		*store += strlen(s);
	}
}

char *expand_string_expression(SCRIPT_VARINFO *info,char *str,char **store)
{
	int num, x;

	str = expand_argument_expression(info,str,&num);
//	sprintf(buf,"expand_string_expression: str %08X, %2.2X", str, (str ? (*str&0xFF) : 0));
//	wiznet(buf,NULL,NULL,WIZ_TESTING,0,0);

	if(!str) return NULL;

	x = sprintf(*store,"%d",num);

//	sprintf(buf,"expand_string_expression: %d -> '%s'", num, *store);
//	wiznet(buf,NULL,NULL,WIZ_TESTING,0,0);

	DBG3MSG2("num = '%s', str = %02.2X\n", *store, (*str)&0xFF);

	*store += x;

	return str;
}

char *expand_string_variable(SCRIPT_VARINFO *info,char *str,char **store)
{
	pVARIABLE var;
	char buf[MIL];

	str = expand_variable(*(info->var),str, &var);
	if(!str) return NULL;

	if(!var) {
		strcpy(*store,"(null-var)");
		*store += 10;
	} else {
		switch(var->type) {
		case VAR_INTEGER:
			*store += sprintf(*store,"%d",var->_.i); break;
		case VAR_STRING:
		case VAR_STRING_S:
			if(var->_.s && *var->_.s) *store += sprintf(*store,"%s",var->_.s); break;
		case VAR_ROOM:
			*store += sprintf(*store,"%d",var->_.r ? (int)var->_.r->vnum : 0); break;
		case VAR_EXIT:
			*store += sprintf(*store,"%s",var->_.e ? dir_name[var->_.e->orig_door] : "noexit"); break;
		case VAR_MOBILE:
			if(var->_.m) {
				one_argument(var->_.m->name,buf);
				*store += sprintf(*store,"%s",buf);
			}
			break;
		case VAR_OBJECT:
			if(var->_.o) {
				one_argument(var->_.o->name,buf);
				*store += sprintf(*store,"%s",buf);
			}
			break;
		case VAR_TOKEN:
			*store += sprintf(*store,"%d",var->_.t ? (int)var->_.t->pIndexData->vnum : 0); break;
		default:
			strcpy(*store,"(null)");
			*store += 6;
			break;
		}
	}

	return str;
}

void expand_string_dump(char *str)
{
#ifdef DEBUG_MODULE
	char buf[MSL*4+1];
	int i;

	i = 0;
	while(*str)
		i += sprintf(buf+i," %02.2X", (*str++)&0xFF);
	buf[i] = 0;

	printf("str: %s\n", buf);
#endif
}

bool expand_string(SCRIPT_VARINFO *info,char *str,char *store)
{
	char *start = store;
	str = skip_whitespace(str);
	while(*str) {
		if(*str == ESCAPE_ENTITY)
			str = expand_string_entity(info,str+1,&store);
		else if(*str == ESCAPE_EXPRESSION)
			str = expand_string_expression(info,str+1,&store);
		else if(*str == ESCAPE_VARIABLE)
			str = expand_string_variable(info,str+1,&store);
		else if((unsigned char)*str >= ESCAPE_UA && (unsigned char)*str <= ESCAPE_LZ) {
			expand_string_simple_code(info,*str,&store);
			++str;
		} else
			*store++ = *str++;

		if(!str) {
			*store = 0;
			expand_string_dump(start);
			DBG2EXITVALUE2(FALSE);
			return FALSE;
		}
	}

	*store = 0;
//	wiznet("EXPAND_STRING:",NULL,NULL,WIZ_TESTING,0,0);
//	wiznet(start,NULL,NULL,WIZ_TESTING,0,0);
	expand_string_dump(start);
	DBG2EXITVALUE2(TRUE);
	return TRUE;
}

char *one_argument_escape( char *argument, char *arg_first )
{
	int depth;
	char cEnd;

	argument = skip_whitespace(argument);

	cEnd = ' ';
	if ( *argument == '\'' || *argument == '"' )
		cEnd = *argument++;

	while ( *argument && *argument != ESCAPE_END ) {
		if(*argument == ESCAPE_VARIABLE ||
		   *argument == ESCAPE_EXPRESSION ||
		   *argument == ESCAPE_ENTITY) {
			*arg_first++ = *argument++;
			depth = 0;
			while(*argument) {
				if(*argument == ESCAPE_VARIABLE ||
				   *argument == ESCAPE_EXPRESSION ||
				   *argument == ESCAPE_ENTITY)
					++depth;
				else if(*argument == ESCAPE_END && !depth--)
					break;
				*arg_first++ = *argument++;
			}
		}

		if ( *argument == cEnd ) {
			argument++;
			break;
		}
		*arg_first++ = *argument++;
	}
	*arg_first = '\0';

	return skip_whitespace(argument);
}


char *expand_argument(SCRIPT_VARINFO *info,char *str,SCRIPT_PARAM *arg)
{
	char buf[MIL];

	str = skip_whitespace(str);
	arg->type = ENT_NONE;

	if(*str == ESCAPE_ENTITY)
		str = expand_argument_entity(info,str+1,arg);
	else if(*str == ESCAPE_EXPRESSION) {
		arg->type = ENT_NUMBER;
		str = expand_argument_expression(info,str+1,&arg->d.num);
	} else if(*str == ESCAPE_VARIABLE) {
		str = expand_argument_variable(info,str+1,arg);
	} else if((unsigned char)*str >= ESCAPE_UA && (unsigned char)*str <= ESCAPE_LZ) {
		expand_argument_simple_code(info,(unsigned char)(*str++),arg);
	} else if(*str) {
		str = one_argument_escape(str,buf);

		if(is_number(buf)) {
			arg->type = ENT_NUMBER;
			arg->d.num = atoi(buf);
		} else if(expand_string(info,buf,arg->buf)) {
			arg->type = ENT_STRING;
			arg->d.str = arg->buf;
		}
	}

	return skip_whitespace(str);
}
