/*  This file is part of the BScript project
    Copyright (C) 2002 Ariya Hidayat <ariyahidayat@yahoo.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/
%{
#include <stdlib.h>
#include <stdio.h>
#include "bscript.h"
#include "intern.h"
#include "blib.h"

/* somewhere else in lex.l */
extern int yyerror(char*);
extern int yylex();
void CXParse( char* );

/* prototypes */
void begin_subroutine(char*);
void gen_spop();
void gen_scopy();
void gen_sswap();
void gen_unop(char);
void gen_binop(char);
void gen_comp(char);
void gen_print();
void gen_pushsymbol (char*);
void gen_popsymbol ( char*);
void gen_pushnumber (double);
void gen_pushstring (char*);
void gen_jump (char*);
void gen_label (char*);
void gen_if (void);
void gen_for ( char*, double step);
void gen_next ( char*, double);
void gen_then (void);
void gen_endif (void);
void gen_call(char*,int);
void gen_return(void);
void gen_while(void);
void gen_wend(void);
void gen_while_label(void);

/* internal compiler stack */
#define MAX_INT_STACK 2048
int intstack_count;
int intstacks[MAX_INT_STACK];

/* push an integer to stack */
void pushint(int x)
{
  intstacks[intstack_count] = x;
  intstack_count++;
}

/* pop an integer from stack */
int popint(void)
{
  intstack_count--;
  return intstacks[intstack_count];
}

/* swap two topmost integers on stack */
void swapint(void)
{
  int temp;

  temp = intstacks[intstack_count-1];
  intstacks[intstack_count-1] = intstacks[intstack_count-2];
  intstacks[intstack_count-2] = temp;
}

/* magic label */
#ifndef MAX_ID_LENGTH
#define MAX_ID_LENGTH 255
#endif
char* magiclabel(int i){
static char name[MAX_ID_LENGTH];
sprintf (name,"magic_%d",i);
return name;
}

/* local variables */
BList* bytecodes;
int autolabel;
int compile_error_code;
int compile_line_no;

/* macro: add code with error-checking */
#define ADD_CODE(code) { \
  if (compile_error_code) return; \
  (code) = code_add(); \
  if (!(code)) return; \
  if (compile_error_code) return; \
  }

/* add a new bytecode, return pointer to the code (NULL upon error) */
BSBytecode* code_add(void)
{
  BSBytecode *code;

  code = b_new( BSBytecode );
  code->type = -1;
  code->argc = 0;
  code->op = '\0';
  code->value = NULL;
  code->label = NULL;
  code->symbol = NULL;
  bytecodes = blist_append( bytecodes, code );
  return code;
}

/* create a bytecode to terminate program */
void gen_terminate (void)
{
  BSBytecode *code;

  ADD_CODE(code);
  code->type = PCODE_TERMINATE;
}

/* create unconditional jump */
void gen_jump (char *label)
{
  BSBytecode *code;

  ADD_CODE(code);
  code->type = PCODE_JUMP;
  code->label = b_strdup(label);
}

/* create a label */
void gen_label (char *label)
{
  BSBytecode *code;

  ADD_CODE(code);
  code->type = PCODE_LABEL;
  code->label = b_strdup(label);
}

/* generate bytecode to pop (anything) from stack */
void gen_spop()
{
  BSBytecode *code;

  ADD_CODE(code);
  code->type = PCODE_SPOP;
}

/* generate bytecode to copy topmost entry on stack */
void gen_scopy()
{
  BSBytecode *code;

  ADD_CODE(code);
  code->type = PCODE_SCOPY;
}

/* generate bytecode to swap topmost entry on stack*/
void gen_sswap()
{
  BSBytecode *code;

  ADD_CODE(code);
  code->type = PCODE_SSWAP;
}

/* create a bytecode to push a symbol */
void gen_pushsymbol ( char* symbol)
{
  BSBytecode *code;

  ADD_CODE(code);
  code->type = PCODE_PUSHSYMBOL;
  code->symbol = b_strdup( symbol );
}

/* generate bytecode for unary operator */
void gen_unop( char o )
{
  BSBytecode *code;

  ADD_CODE(code);
  code->type = PCODE_UNOP;
  code->op = o;

}

/* generate bytecode for unary operator */
void gen_binop( char o )
{
  BSBytecode *code;

  ADD_CODE(code);
  code->type = PCODE_BINOP;
  code->op = o;

}

/* generate bytecode for comparation operator */
/* for possible operator, see BS_COMP_ constants */
void gen_comp( char o )
{
  BSBytecode *code;

  ADD_CODE(code);
  code->type = PCODE_COMP;
  code->op = o;

}

void gen_print()
{
  BSBytecode *code;

  ADD_CODE(code);
  code->type = PCODE_PRINT;
}

/* create a bytecode to pop stack to a symbol */
void gen_popsymbol ( char* symbol )
{
  BSBytecode *code;

  ADD_CODE(code);
  code->type = PCODE_POPSYMBOL;
  code->symbol = b_strdup( symbol );
}

/* create a bytecode to push a number */
void gen_pushnumber (double number)
{
  BSBytecode *code;

  ADD_CODE(code);
  code->type = PCODE_PUSHVALUE;
  code->value = bvariant_new_double( number );
}

/* create a bytecode to push a string */
void gen_pushstring (char *string)
{
  BSBytecode *code;

  ADD_CODE(code);
  code->type = PCODE_PUSHVALUE;
  code->value = bvariant_new_string( string );
}

/* create a conditional branch which jump to given autolabel if FALSE  */
void gen_if (void)
{
  BSBytecode *code;
  int i;

  i = autolabel++;
  pushint(i);
  ADD_CODE(code);
  code->type = PCODE_IF;
  code->label = b_strdup(magiclabel(i));
}

/* goto, then create autolabel */
void gen_then (void)
{
  int i;

  i = autolabel++;
  pushint(i);
  gen_jump(magiclabel(i));

  swapint();
  i = popint();
  gen_label(magiclabel(i));
}

/* create terminating label  */
void gen_endif (void)
{
  int i;

  i = popint();
  gen_label(magiclabel(i));
}

/* create necessary instructions for a loop */
void gen_for ( char* symbol, double step)
{
  BSBytecode* code;
  int i;

  gen_sswap();
  gen_popsymbol (symbol);

  i = autolabel++;
  pushint(i);
  gen_label(magiclabel(i));

  gen_scopy();
  gen_pushsymbol (symbol);

  gen_sswap();

  if ( step > 0 ) gen_comp( BS_COMP_LE );
  else gen_comp( BS_COMP_GE );

  i = autolabel++;
  pushint(i);
  ADD_CODE(code);
  code->type = PCODE_IF;
  code->label = b_strdup(magiclabel(i));
}

/* loop termination */
void gen_next ( char* symbol, double step)
{
  int i;

  gen_pushsymbol (symbol);
  gen_pushnumber (step);
  gen_binop( '+' );
  gen_popsymbol (symbol);

  swapint();
  i = popint();
  gen_jump (magiclabel(i));
  i = popint();
  gen_label (magiclabel(i));

  gen_spop();
}

/* Generate while() starting label */
void gen_while_label(void)
{
  int sl = autolabel++;
  pushint(sl);
  gen_label(magiclabel(sl));
}

/* Create an end label, push that on the stack,
  then create a conditional branch which jumps to the end label
  if false. */
void gen_while(void)
{
 BSBytecode *code;
 int el = autolabel++;
 pushint(el);
 ADD_CODE(code);
 code->type = PCODE_IF;
 code->label = b_strdup(magiclabel(el));
}
/* Pop the start label and generate an unconditional jump
   to it. Then pop the end label and write it. */
void gen_wend(void)
{
 int sl, el;
 el = popint();
 sl = popint();
 gen_jump(magiclabel(sl));
 gen_label(magiclabel(el));
}

void gen_call( char* name, int argc )
{
  BSBytecode *code;

  ADD_CODE(code);
  code->type = PCODE_CALL;
  code->label = b_strdup(name);
  code->argc = argc;
}

/* spawn for a new subroutine */
void begin_subroutine( char* name )
{
  // hack: to prevent accidential stepping thru the subroutine
  // FIXME better mark it with 'sub' flag or something ?
  gen_terminate();

  // FIXME check for nested sub
  // NOTE no need to strdup() cause gen_label does that
  gen_label( name );

  return;
}

/* end of a subroutine */
void gen_return()
{
  BSBytecode *code;

  ADD_CODE(code);
  code->type = PCODE_RETURN;
}

/* parse the script, return list of bytecodes */
BList* bscript_parse( char *script )
{
  /* reset local variables first */
  bytecodes = 0;
  autolabel = 0;
  compile_line_no = 1;
  compile_error_code = 0;
  intstack_count = 0;

  /* this is in lex.l */
  CXParse( script );

  /* if error, return NULL */
  if (compile_error_code)
  {
    /* FIXME deallocate ? */
    return NULL;
    /* compile_lineno */;
  }


  /* make sure there's PCODE_TERMINATE */
  gen_terminate();

  return bytecodes;
}

/* compiler error, to be called from yyparse() */
/* TO DO: create a verbose error handler */
void compile_error (long lineno, char* msg, char* yytext)
{
  //compile_error_code = CXERROR_PARSE;
  compile_error_code = -1;
  fprintf( stderr, "error line %ld: %s before '%s'\n", lineno, msg, yytext );
}

%}%union {
  char* symbol;
  double number;
  char* string;
  }

%token <symbol> SYMBOL
%token <symbol> STRSYMBOL
%token <number> NUMBER
%token <string> STRING

%token T_ENDOFFILE
%token T_SEPARATOR
%token T_MOD T_NOT T_OR T_AND T_XOR
%token T_EQ T_NE T_LT T_LE T_GT T_GE
%token T_IF T_THEN T_ELSE T_ENDIF
%token T_LET
%token T_FOR T_TO T_STEP T_NEXT
%token T_WHILE T_WEND
%token T_PRINT
%token T_SUB
%token T_END
%token T_RANDOMIZE

%left T_SEPARATOR
%left T_OR T_AND T_XOR
%left T_EQ T_NE T_LT T_LE T_GT T_GE

%left ','
%left T_MOD
%left '-' '+'
%left '*' '/'
%left '^'

%left '|' '&' '~'

%nonassoc UBITNOT
%nonassoc UNOT
%nonassoc UMINUS

%type <number> loopstep
%type <number> constant
%type <number> call_list

%%

program:
  statement_list T_ENDOFFILE { YYACCEPT; }
;

statement_list:
  statement
| statement_list T_SEPARATOR statement
;

statement:  /* empty */
  | expression { gen_spop(); }
  | string_expression { gen_spop(); }
  | assignment
  | T_LET assignment
  | string_assignment
  | T_LET string_assignment
  | loop
  | while_loop
  | subroutine
  | conditional
  | T_PRINT printlist semicolon
  | T_RANDOMIZE { gen_call("RANDOMIZE",0); gen_spop(); }
  | T_RANDOMIZE expression { gen_call("RANDOMIZE",1); gen_spop(); }
;

assignment:
  SYMBOL T_EQ expression { gen_popsymbol($1);b_free($1); }
;

string_assignment:
  STRSYMBOL T_EQ string_expression { gen_popsymbol($1);b_free($1); }
;

string_expression:
  STRSYMBOL { gen_pushsymbol($1);b_free($1); }
| string_subroutine_call
| STRING { gen_pushstring($1); b_free($1); }
| string_expression '+' string_expression { gen_binop('#'); }
;

expression:
  NUMBER { gen_pushnumber($1); }
| SYMBOL { gen_pushsymbol($1);b_free($1); }
| subroutine_call
| '(' expression ')'
| expression T_MOD expression { gen_binop('%'); }
| expression '+' expression { gen_binop('+'); }
| expression '-' expression { gen_binop('-'); }
| expression '*' expression { gen_binop('*'); }
| expression '/' expression { gen_binop('/'); }
| expression '^' expression { gen_binop('!'); }
| expression T_OR expression { gen_binop('|'); }
| expression T_AND expression { gen_binop('&'); }
| expression T_XOR expression { gen_binop('^'); }
| '-' expression %prec UMINUS { gen_unop('-');}
| T_NOT expression %prec UBITNOT { gen_unop('~'); }
;

subroutine:
  T_SUB SYMBOL { begin_subroutine($2);b_free($2); }
| T_SUB STRSYMBOL { begin_subroutine($2);b_free($2); }
| T_END T_SUB { gen_return(); }
;

/* remember: every subroutine/function has a return value! */
subroutine_call:
  SYMBOL '(' call_list ')' { gen_call($1,$3); b_free($1); }
;

/* remember: every subroutine/function has a return value! */
string_subroutine_call:
  STRSYMBOL '(' call_list ')' { gen_call($1,$3); b_free($1); }
;

/* return number of arguments in the list */
call_list:
  { $$ = 0; }
| call_item  { $$ = 1; }
| call_list ',' call_item { $$ = $1 + 1; }
;

call_item:
  expression
| string_expression
;

conditional:
  T_IF comparison { gen_if(); }
  T_THEN statement_list { gen_then(); }
  conditional_else
  T_ENDIF { gen_endif(); }
  ;

conditional_else:
  T_ELSE statement_list
| /* empty, no else */
;

loop:
  T_FOR SYMBOL T_EQ expression T_TO expression loopstep { gen_for ($2, $7); }
  T_SEPARATOR statement_list T_NEXT { gen_next($2, $7);b_free($2); }
;

loopstep:
  T_STEP constant { $$ = $2; }
| { $$ = 1.0; } /* empty, default to 1 */
;

while_loop:
  T_WHILE { gen_while_label(); } comparison { gen_while(); }
  T_SEPARATOR statement_list T_WEND { gen_wend(); }
;

constant:
 NUMBER {$$=$1;} | '+' NUMBER {$$=$2;} | '-' NUMBER {$$=-$2;};

printlist:
  expression { gen_print(); }
| string_expression { gen_print(); }
| printlist ',' expression
   { gen_pushstring(" "); gen_print(); gen_print(); }
| printlist ',' string_expression
   { gen_pushstring(" "); gen_print(); gen_print(); }
;

semicolon:
  ';'
| { gen_pushstring("\n");gen_print(); }
  ;

comparison:
|  '(' comparison ')'
| comparison T_OR comparison { gen_comp(BS_COMP_OR); }
| comparison T_AND comparison { gen_comp(BS_COMP_AND); }
| T_NOT comparison %prec UNOT { gen_comp(BS_COMP_NOT); }
| expression T_EQ expression { gen_comp(BS_COMP_EQ);  }
| expression T_NE expression { gen_comp(BS_COMP_NE);  }
| expression T_LT expression { gen_comp(BS_COMP_LT);  }
| expression T_LE expression { gen_comp(BS_COMP_LE);  }
| expression T_GT expression { gen_comp(BS_COMP_GT);  }
| expression T_GE expression { gen_comp(BS_COMP_GE);  }
| string_expression T_EQ string_expression { gen_comp(BS_COMP_SEQ); }
| string_expression T_NE string_expression { gen_comp(BS_COMP_SNE); };

%%
