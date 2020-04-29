#include "bscript.h"
#include "blib.h"
#include "intern.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* this only useful for debugging */
void stack_dump( BSContext *context )
{
  BStack *stack = context->stack;
  int i;

  printf( "stack_dump context=%p stack=%p\n", context, stack );

  for( i = stack->ptr-1; i>=0; i-- )
  {
    BVariant *v;
    v = stack->data[i];
    printf("%3d: ", i );
    printf("%s\n", bvariant_dump(v) );
  }

  printf( "\n" );
}

BSContext*
bs_context_create( BSModule *module )
{
  BSContext *context;

  context = b_new( BSContext );
  context->module = module;
  context->stack = bstack_new();
  context->program_counter = 0;
  context->result = NULL;

  return context;
}

void
bs_context_destroy( BSContext *context )
{
  if( context )
  {
    bstack_free_destruct( context->stack, (BDestructor)bvariant_free );
    bvariant_free( context->result );
    b_free( context );
  }
}

void
bs_context_set_result( BSContext *context, BVariant *result )
{
  if( !context ) return;
  bvariant_free( context->result );
  context->result = result;
}

void bs_context_push_number( BStack* stack, double num )
{
  BVariant *v;

  v = bvariant_new_double( num );
  bstack_push( stack, v );
}

void bs_context_push_string( BStack* stack, char* str )
{
  BVariant *v;

  v = bvariant_new_string( str );
  bstack_push( stack, v );
}

void bs_context_push_address( BStack* stack, int ad )
{
  BVariant *v;

  v = bvariant_new_integer( ad );

  bstack_push( stack, v );
}

double bs_context_pop_number( BStack* stack )
{
  BVariant *v;
  double d;

  v = bstack_pop( stack );
  bvariant_cast( v, BV_DOUBLE );
  d = bvariant_as_double( v );

  bvariant_free( v );
  return d;
}

char* bs_context_pop_string( BStack* stack )
{
  BVariant *v;
  char *s;

  v = bstack_pop( stack );
  bvariant_cast( v, BV_STRING );
  s = bvariant_as_string( v );
  s = b_strdup( s );

  bvariant_free( v );
  return s;
}

int bs_context_pop_address( BStack* stack )
{
  BVariant *v;
  int i;

  v = bstack_pop( stack );
  bvariant_cast( v, BV_INTEGER );
  i = bvariant_as_integer( v );

  bvariant_free( v );
  return i;
}

/* return PC where label is found, -1 if not found */
int bs_context_find_label( BSContext* cx, char* label )
{
  int i;
  BSBytecode *code;

  /* search in main program */
  for( i=0; i<blist_length(cx->module->bytecodes); i++)
  {
    code = blist_nth_data(cx->module->bytecodes,i);
    if (code->type == PCODE_LABEL) if (code->label)
      if ( b_stricmp(code->label, label))
        return i;
  }

  return -1;
}

/* execute a unary operator */
void bs_context_exec_unop( BSContext* cx, char op )
{
  double x;
  long c;

  switch( op ){

  /* negate */
  case '-':
    x = bs_context_pop_number( cx->stack );
    bs_context_push_number( cx->stack, -x );
    break;

  /* bitwise not */
  case '~':
    x = bs_context_pop_number( cx->stack );
    c = (long) x; c = ~c; x = (double) c;
    bs_context_push_number( cx->stack, -x );
    break;

  /* boolean not */
  case '!':
    x = bs_context_pop_number( cx->stack );
    c = (long) x; c = ~c; x = (double) c;
    bs_context_push_number( cx->stack, -x );
    break;


  }
}

/* execute a binary operator */
void bs_context_exec_binop( BSContext* cx, char op )
{
  double x, y, z;
  long a, b, c;

  switch( op ){

  case '+':
    y = bs_context_pop_number( cx->stack );
    x = bs_context_pop_number( cx->stack );
    bs_context_push_number( cx->stack, x + y );
    break;

  case '-':
    y = bs_context_pop_number( cx->stack );
    x = bs_context_pop_number( cx->stack );
    bs_context_push_number( cx->stack, x - y );
    break;

  case '%':
    y = bs_context_pop_number( cx->stack );
    x = bs_context_pop_number( cx->stack );
    bs_context_push_number( cx->stack, x - y*(int)(x/y) );
    break;

  case '*':
    y = bs_context_pop_number( cx->stack );
    x = bs_context_pop_number( cx->stack );
    bs_context_push_number( cx->stack, x * y );
    break;

  case '/':
    y = bs_context_pop_number( cx->stack );
    x = bs_context_pop_number( cx->stack );
    //if (IS_SMALL(y)) trigger_error (cx, CXERROR_DIVZERO);
    bs_context_push_number( cx->stack, x / y );
    break;

  /* power */
  case '!':
    y = bs_context_pop_number( cx->stack );
    x = bs_context_pop_number( cx->stack );
    //if (y < 0) trigger_error (cx, CXERROR_DOMERR);
    bs_context_push_number( cx->stack, exp(y*log(x)) );
    break;

  /* bitwise or */
  case '|':
    y = bs_context_pop_number( cx->stack );
    x = bs_context_pop_number( cx->stack );
    a = (long) x;
    b = (long) y;
    c = a | b;
    z = (double) c;
    bs_context_push_number( cx->stack, z );
    break;

  /* bitwise and */
  case '&':
    y = bs_context_pop_number( cx->stack );
    x = bs_context_pop_number( cx->stack );
    a = (long) x;
    b = (long) y;
    c = a & b;
    z = (double) c;
    bs_context_push_number( cx->stack, z );
    break;

  /* bitwise xor */
  case '^':
    y = bs_context_pop_number( cx->stack );
    x = bs_context_pop_number( cx->stack );
    a = (long) x;
    b = (long) y;
    c = a ^ b;
    z = (double) c;
    bs_context_push_number( cx->stack, z );
    break;

  /* concat string */
  case '#':
    {
      char *p, *q;
      char* result;

      q = bs_context_pop_string( cx->stack );
      p = bs_context_pop_string( cx->stack );
      result = (char*) b_malloc(strlen(p)+strlen(q)+1);
      strcpy(result, p);
      strcat(result, q);
      b_free(p);
      b_free(q);
      bs_context_push_string( cx->stack, result );
      b_free(result); /* alredy strdup()'ed */
    }
    break;


  }
}

#define MAKE_LOGICAL(x)(int)((x) == 0.0) ? 0 : -1
void bs_context_exec_comp( BSContext* cx, char comp )
{
  double x, y, z;
  long a, b, c;
  char *p, *q;

  switch( comp )
  {

  /* logical not */
  case BS_COMP_NOT:
    x = bs_context_pop_number( cx->stack );
    c = MAKE_LOGICAL(x);
    c = ~c; z = (double) c;
    bs_context_push_number( cx->stack, z );
    break;

  /* logical or */
  case BS_COMP_OR:
    y = bs_context_pop_number( cx->stack );
    x = bs_context_pop_number( cx->stack );
    a = MAKE_LOGICAL(x);
    b = MAKE_LOGICAL(y);
    c = a || b; z = (double) c;
    bs_context_push_number( cx->stack, z );
    break;

  /* logical and */
  case BS_COMP_AND:
    y = bs_context_pop_number( cx->stack );
    x = bs_context_pop_number( cx->stack );
    a = MAKE_LOGICAL(x);
    b = MAKE_LOGICAL(y);
    c = a && b; z = (double) c;
    bs_context_push_number( cx->stack, z );
    break;

  /* true if numbers are equal */
  case BS_COMP_EQ:
    y = bs_context_pop_number( cx->stack );
    x = bs_context_pop_number( cx->stack );
    c = (x == y); c = MAKE_LOGICAL(c);
    z = (double) c;
    bs_context_push_number( cx->stack, z );
    break;

  /* true if numbers are not equal */
  case BS_COMP_NE:
    y = bs_context_pop_number( cx->stack );
    x = bs_context_pop_number( cx->stack );
    c = (x != y); c = MAKE_LOGICAL(c);
    z = (double) c;
    bs_context_push_number( cx->stack, z );
    break;

  /* true if first number less than second number */
  case BS_COMP_LT:
    y = bs_context_pop_number( cx->stack );
    x = bs_context_pop_number( cx->stack );
    c = (x < y); c = MAKE_LOGICAL(c);
    z = (double) c;
    bs_context_push_number( cx->stack, z );
    break;

  /* true if first number less than or equal to second number */
  case BS_COMP_LE:
    y = bs_context_pop_number( cx->stack );
    x = bs_context_pop_number( cx->stack );
    c = (x <= y); c = MAKE_LOGICAL(c);
    z = (double) c;
    bs_context_push_number( cx->stack, z );
    break;

  /* true if first number greater than second number */
  case BS_COMP_GT:
    y = bs_context_pop_number( cx->stack );
    x = bs_context_pop_number( cx->stack );
    c = (x > y); c = MAKE_LOGICAL(c);
    z = (double) c;
    bs_context_push_number( cx->stack, z );
    break;

  /* true if first number greater than or equal to second number */
  case BS_COMP_GE:
    y = bs_context_pop_number( cx->stack );
    x = bs_context_pop_number( cx->stack );
    c = (x >= y); c = MAKE_LOGICAL(c);
    z = (double) c;
    bs_context_push_number( cx->stack, z );
    break;

  /* return true if two strings are equal */
  case BS_COMP_SEQ:
    /* get the two strings */
    q = bs_context_pop_string( cx->stack );
    p = bs_context_pop_string( cx->stack );
    c = strcmp(p, q) == 0;
    c = MAKE_LOGICAL(c);
    bs_context_push_number( cx->stack, c );
    b_free(p);
    b_free(q);
    break;

  /* return false if two strings are equal */
  case BS_COMP_SNE:
    /* get the two strings */
    q = bs_context_pop_string( cx->stack );
    p = bs_context_pop_string( cx->stack );
    c = strcmp(p, q) != 0;
    c = MAKE_LOGICAL(c);
    bs_context_push_number( cx->stack, c );
    b_free(p);
    b_free(q);
    break;
  }
}



void bs_context_exec_print( BSContext* cx )
{
  BVariant *v;

  v = bstack_pop( cx->stack );
  if( NULL == v )
	  return;

  if( v->type == BV_DOUBLE )
  {
    double number = bvariant_as_double( v );
	if( cx->module && cx->module->m_pfnPrint )
		cx->module->m_pfnPrint( &number, 0, cx->module->m_pPrintContext );
	else
		printf ("%g", number);
  }

  if( v->type == BV_STRING )
  {
    char *string = bvariant_as_string( v );
	if( cx->module && cx->module->m_pfnPrint )
		cx->module->m_pfnPrint( string, 1, cx->module->m_pPrintContext  );
	else
		printf ("%s", string);
  }

  bvariant_free( v );
}

void bs_context_exec_push_symbol( BSContext *cx, char *symbol_name )
{
  BSSymbol *symbol;

  /* key is uppercase, moreover identified in BASIC is case insensitive */
  b_strupper( symbol_name );

  /* might be built-in functions with no argument, such as DATE$ or TIME$ */
  if( bs_eval_builtin_function( cx, symbol_name, 0 ) )
    return;

  symbol = bs_module_access_symbol( cx->module, symbol_name );

  switch (symbol->type){
   case BV_INVALID:
      /* make it as a number */
     bs_context_push_number( cx->stack, 0 );
     break;
   case BV_DOUBLE:
     bs_context_push_number(cx->stack, symbol->number);
     break;
   case BV_STRING:
     bs_context_push_string(cx->stack, symbol->string);
     break;
//   default:
     //trigger_error (cx, CXERROR_BADSYMTYPE);
   }
}

void bs_context_exec_pop_symbol( BSContext *cx, char *symbol_name )
{
  BSSymbol *symbol;
  BVariant *v;

  /* key is uppercase, moreover identified in BASIC is case insensitive */
  b_strupper( symbol_name );

  symbol = bs_module_access_symbol( cx->module, symbol_name );

  v = bstack_pop( cx->stack );

  if( symbol->type == BV_INVALID )
     symbol->type = v->type;

  switch(symbol->type)
  {
    case BV_DOUBLE:
      bvariant_cast( v, BV_DOUBLE );
      symbol->number = bvariant_as_double( v );
      break;
    case BV_STRING:
      bvariant_cast( v, BV_STRING );
      symbol->string = b_strdup( bvariant_as_string( v ) );
      break;
    default:
      //trigger_error (cx, CXERROR_BADSYMTYPE);
       break;
  }

  bvariant_free( v );
}

static char*
describe_bytecode( BSBytecode *code )
{
  static char buf[100];

  if( !code ) return NULL;

  switch( code->type )
  {
  case PCODE_TERMINATE:
    sprintf( buf, "END" ); break;
  case PCODE_SPOP:
    sprintf( buf, "POP"); break;
  case PCODE_SCOPY:
    sprintf ( buf, "SCOPY"); break;
  case PCODE_SSWAP:
    sprintf ( buf, "SSWAP"); break;
  case PCODE_UNOP:
    sprintf ( buf, "UNOP %c", code->op); break;
  case PCODE_BINOP:
    sprintf ( buf, "BINOP %c", code->op); break;
  case PCODE_COMP:
    sprintf ( buf, "COMP:%s", "??" ); break;
  case PCODE_PRINT:
    sprintf ( buf, "PRINT"); break;
  case PCODE_PUSHSYMBOL:
    sprintf ( buf, "PUSH   #%s", code->symbol); break;
  case PCODE_POPSYMBOL:
    sprintf ( buf, "POP    #%s", code->symbol); break;
  case PCODE_PUSHVALUE:
    sprintf ( buf, "PUSH   %s", bvariant_dump(code->value)); break;
  case PCODE_JUMP:
    sprintf ( buf, "JUMP   %s", code->label); break;
  case PCODE_IF:
    sprintf ( buf, "JFALSE %s", code->label); break;
  case PCODE_LABEL:
    sprintf ( buf, "%s:", code->label); break;
  case PCODE_CALL:
    sprintf ( buf, "CALL %s", code->label); break;
  case PCODE_RETURN:
    sprintf ( buf, "RETURN"); break;
  default:
    sprintf( buf, "bad!" ); break;
  }

  return buf;
}

void bs_context_dump( BSContext *context )
{
  BSBytecode *code;
  int pc;

  pc = 0;
  while( pc < blist_length(context->module->bytecodes) )
  {
    code = blist_nth_data( context->module->bytecodes, pc );
    printf("%4d:  %s\n", pc+1, describe_bytecode( code ) );
    pc++;
  }
}

void bs_context_execute( BSContext *context )
{
  BSBytecode *code;

  context->program_counter = 0;

  if( !context->module->objects )
    context->module->objects = bhash_new();

  while( context->program_counter >= 0 )
  {
    code = blist_nth_data( context->module->bytecodes, context->program_counter );

    //printf("execute %s:%d\n", context->module->name, context->program_counter );
    //printf("  %s\n", describe_bytecode( code ) );

    /* mark end-of-execution, terminate the context */
    if( code->type == PCODE_TERMINATE )
    {
      context->program_counter = -1;
    }

    /* pop something from stack, just to suck one entry */
    else if( code->type == PCODE_SPOP )
    {
      BVariant *v;
      v = bstack_pop( context->stack );
      bvariant_free( v );
      context->program_counter++;
    }

    /* copy topmost entry on stack */
    else if( code->type == PCODE_SCOPY )
    {
      BVariant *v1;
      BVariant *v2;
      v1 = bstack_pop( context->stack );
      v2 = bvariant_copy( v1 );
      bstack_push( context->stack, v1 );
      bstack_push( context->stack, v2 );
      context->program_counter++;
    }

    /* swap two topmost entries on stack */
    else if( code->type == PCODE_SSWAP )
    {
      BVariant *v1;
      BVariant *v2;
      v1 = bstack_pop( context->stack );
      v2 = bstack_pop( context->stack );
      bstack_push( context->stack, v1 );
      bstack_push( context->stack, v2 );
      context->program_counter++;
    }

    else if( code->type == PCODE_UNOP )
    {
      bs_context_exec_unop( context, code->op );
      context->program_counter++;
    }

    else if( code->type == PCODE_BINOP )
    {
      bs_context_exec_binop( context, code->op );
      context->program_counter++;
    }

    else if( code->type == PCODE_COMP )
    {
      bs_context_exec_comp( context, code->op );
      context->program_counter++;
    }

    else if( code->type == PCODE_PRINT )
    {
      bs_context_exec_print( context );
      context->program_counter++;
    }

    else if( code->type == PCODE_PUSHSYMBOL )
    {
      bs_context_exec_push_symbol( context, code->symbol );
      context->program_counter++;
    }

    else if( code->type == PCODE_POPSYMBOL )
    {
      bs_context_exec_pop_symbol( context, code->symbol );
      context->program_counter++;
    }

    /* a label, just ignore it */
    else if( code->type == PCODE_LABEL )
    {
      context->program_counter++;
    }

    /* jump to a given label */
    else if( code->type == PCODE_JUMP )
    {
      int addr = bs_context_find_label( context, code->label );
      context->program_counter = (addr>=0) ? addr : context->program_counter+1;
    }

    /* call a subroutine */
    else if( code->type == PCODE_CALL)
    {
      if( bs_eval_builtin_function( context, code->label, code->argc ) )
      {
        context->program_counter++;
      }
      else if ( !bs_eval_user_function( context, code->label, code->argc ) )
        {
          // dummy value
          bstack_push( context->stack, bvariant_new() );
          context->program_counter++;
        }
    }

    /* return from a subroutine */
    else if( code->type == PCODE_RETURN )
    {
      context->program_counter = bs_context_pop_address( context->stack );
    }

    /* if popped number false then jump to a given label, else continue */
    else if( code->type == PCODE_IF )
    {
      double x = bs_context_pop_number( context->stack );
      if( (int)(x) ) context->program_counter++;
      else
      {
        int addr = bs_context_find_label( context, code->label );
        context->program_counter = (addr>=0) ? addr : context->program_counter+1;
      }
    }

    /* push a value to stack */
    else if( code->type == PCODE_PUSHVALUE )
    {
      BVariant *value;
      value = bvariant_copy( code->value );
      bstack_push( context->stack, value );
      context->program_counter++;
    }

    else
      context->program_counter++;

    //stack_dump( context );

  }

}

int
bs_eval_user_function( BSContext *context, char *function, int narg )
{
  int addr;

  addr = bs_context_find_label( context, function );
  if( addr < 0 ) return FALSE;

  bs_context_push_address( context->stack, context->program_counter );
  context->program_counter = addr;

  return TRUE;
}

/* try to execute a builtin function, return FALSE if failed */
typedef BVariant* (*BuiltinFunc)(BSContext*,BList*);

int
bs_eval_builtin_function( BSContext *context, char *function, int narg )
{
  BuiltinFunc func;
  BList *args;
  int i;
  BVariant *result;

  if( !context ) return FALSE;
  if( !context->module ) return FALSE;

  /* key is uppercase, see init_builtin_functions in module.c */
  b_strupper( function );

  func = (BuiltinFunc) bhash_lookup( context->module->builtin_functions, function );
  if( !func ) return FALSE;

  /* pull arguments for the function */
  args = NULL;
  for( i=0; i<narg; i++ )
  {
    BVariant *v;
    v = bstack_pop( context->stack );
    args = blist_prepend( args, v );
  }

  context->m_pBuildinFunctionName = function;	//  CYJ,2005-4-28 cyj add
  /* run the function and put return value on stack */
  result = (*func)( context, args );
  if( !result ) result = bvariant_new();
  bstack_push( context->stack, result );

  blist_free_destruct( args, (BDestructor)bvariant_free );

  return TRUE;
}

