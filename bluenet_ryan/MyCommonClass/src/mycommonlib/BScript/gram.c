/* A Bison parser, made from gram.y
   by GNU bison 1.35.  */

#define YYBISON 1  /* Identify Bison output.  */

# define	SYMBOL	257
# define	STRSYMBOL	258
# define	NUMBER	259
# define	STRING	260
# define	T_ENDOFFILE	261
# define	T_SEPARATOR	262
# define	T_MOD	263
# define	T_NOT	264
# define	T_OR	265
# define	T_AND	266
# define	T_XOR	267
# define	T_EQ	268
# define	T_NE	269
# define	T_LT	270
# define	T_LE	271
# define	T_GT	272
# define	T_GE	273
# define	T_IF	274
# define	T_THEN	275
# define	T_ELSE	276
# define	T_ENDIF	277
# define	T_LET	278
# define	T_FOR	279
# define	T_TO	280
# define	T_STEP	281
# define	T_NEXT	282
# define	T_WHILE	283
# define	T_WEND	284
# define	T_PRINT	285
# define	T_SUB	286
# define	T_END	287
# define	T_RANDOMIZE	288
# define	UBITNOT	289
# define	UNOT	290
# define	UMINUS	291

#line 19 "gram.y"

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


#line 448 "gram.y"
#ifndef YYSTYPE
typedef union {
  char* symbol;
  double number;
  char* string;
  } yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
#ifndef YYDEBUG
# define YYDEBUG 0
#endif



#define	YYFINAL		151
#define	YYFLAG		-32768
#define	YYNTBASE	50

/* YYTRANSLATE(YYLEX) -- Bison token number corresponding to YYLEX. */
#define YYTRANSLATE(x) ((unsigned)(x) <= 291 ? yytranslate[x] : 76)

/* YYTRANSLATE[YYLEX] -- Bison token number corresponding to YYLEX. */
static const char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    42,     2,
      47,    48,    38,    37,    35,    36,     2,    39,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    49,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    40,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    41,     2,    43,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    44,
      45,    46
};

#if YYDEBUG
static const short yyprhs[] =
{
       0,     0,     3,     5,     9,    10,    12,    14,    16,    19,
      21,    24,    26,    28,    30,    32,    36,    38,    41,    45,
      49,    51,    53,    55,    59,    61,    63,    65,    69,    73,
      77,    81,    85,    89,    93,    97,   101,   105,   108,   111,
     114,   117,   120,   125,   130,   131,   133,   137,   139,   141,
     142,   143,   152,   155,   156,   157,   169,   172,   173,   174,
     175,   183,   185,   188,   191,   193,   195,   199,   203,   205,
     206,   207,   211,   215,   219,   222,   226,   230,   234,   238,
     242,   246,   250
};
static const short yyrhs[] =
{
      51,     7,     0,    52,     0,    51,     8,    52,     0,     0,
      56,     0,    55,     0,    53,     0,    24,    53,     0,    54,
       0,    24,    54,     0,    66,     0,    69,     0,    57,     0,
      62,     0,    31,    73,    74,     0,    34,     0,    34,    56,
       0,     3,    14,    56,     0,     4,    14,    55,     0,     4,
       0,    59,     0,     6,     0,    55,    37,    55,     0,     5,
       0,     3,     0,    58,     0,    47,    56,    48,     0,    56,
       9,    56,     0,    56,    37,    56,     0,    56,    36,    56,
       0,    56,    38,    56,     0,    56,    39,    56,     0,    56,
      40,    56,     0,    56,    11,    56,     0,    56,    12,    56,
       0,    56,    13,    56,     0,    36,    56,     0,    10,    56,
       0,    32,     3,     0,    32,     4,     0,    33,    32,     0,
       3,    47,    60,    48,     0,     4,    47,    60,    48,     0,
       0,    61,     0,    60,    35,    61,     0,    56,     0,    55,
       0,     0,     0,    20,    75,    63,    21,    51,    64,    65,
      23,     0,    22,    51,     0,     0,     0,    25,     3,    14,
      56,    26,    56,    68,    67,     8,    51,    28,     0,    27,
      72,     0,     0,     0,     0,    29,    70,    75,    71,     8,
      51,    30,     0,     5,     0,    37,     5,     0,    36,     5,
       0,    56,     0,    55,     0,    73,    35,    56,     0,    73,
      35,    55,     0,    49,     0,     0,     0,    47,    75,    48,
       0,    75,    11,    75,     0,    75,    12,    75,     0,    10,
      75,     0,    56,    14,    56,     0,    56,    15,    56,     0,
      56,    16,    56,     0,    56,    17,    56,     0,    56,    18,
      56,     0,    56,    19,    56,     0,    55,    14,    55,     0,
      55,    15,    55,     0
};

#endif

#if YYDEBUG
/* YYRLINE[YYN] -- source line where rule number YYN was defined. */
static const short yyrline[] =
{
       0,   494,   498,   500,   503,   504,   505,   506,   507,   508,
     509,   510,   511,   512,   513,   514,   515,   516,   519,   523,
     527,   529,   530,   531,   534,   536,   537,   538,   539,   540,
     541,   542,   543,   544,   545,   546,   547,   548,   549,   552,
     554,   555,   559,   564,   569,   571,   572,   575,   577,   580,
     580,   580,   587,   589,   592,   592,   597,   599,   602,   602,
     602,   607,   608,   608,   610,   612,   613,   615,   619,   621,
     624,   625,   626,   627,   628,   629,   630,   631,   632,   633,
     634,   635,   636
};
#endif


#if (YYDEBUG) || defined YYERROR_VERBOSE

/* YYTNAME[TOKEN_NUM] -- String name of the token TOKEN_NUM. */
static const char *const yytname[] =
{
  "$", "error", "$undefined.", "SYMBOL", "STRSYMBOL", "NUMBER", "STRING", 
  "T_ENDOFFILE", "T_SEPARATOR", "T_MOD", "T_NOT", "T_OR", "T_AND", 
  "T_XOR", "T_EQ", "T_NE", "T_LT", "T_LE", "T_GT", "T_GE", "T_IF", 
  "T_THEN", "T_ELSE", "T_ENDIF", "T_LET", "T_FOR", "T_TO", "T_STEP", 
  "T_NEXT", "T_WHILE", "T_WEND", "T_PRINT", "T_SUB", "T_END", 
  "T_RANDOMIZE", "','", "'-'", "'+'", "'*'", "'/'", "'^'", "'|'", "'&'", 
  "'~'", "UBITNOT", "UNOT", "UMINUS", "'('", "')'", "';'", "program", 
  "statement_list", "statement", "assignment", "string_assignment", 
  "string_expression", "expression", "subroutine", "subroutine_call", 
  "string_subroutine_call", "call_list", "call_item", "conditional", "@1", 
  "@2", "conditional_else", "loop", "@3", "loopstep", "while_loop", "@4", 
  "@5", "constant", "printlist", "semicolon", "comparison", 0
};
#endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives. */
static const short yyr1[] =
{
       0,    50,    51,    51,    52,    52,    52,    52,    52,    52,
      52,    52,    52,    52,    52,    52,    52,    52,    53,    54,
      55,    55,    55,    55,    56,    56,    56,    56,    56,    56,
      56,    56,    56,    56,    56,    56,    56,    56,    56,    57,
      57,    57,    58,    59,    60,    60,    60,    61,    61,    63,
      64,    62,    65,    65,    67,    66,    68,    68,    70,    71,
      69,    72,    72,    72,    73,    73,    73,    73,    74,    74,
      75,    75,    75,    75,    75,    75,    75,    75,    75,    75,
      75,    75,    75
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN. */
static const short yyr2[] =
{
       0,     2,     1,     3,     0,     1,     1,     1,     2,     1,
       2,     1,     1,     1,     1,     3,     1,     2,     3,     3,
       1,     1,     1,     3,     1,     1,     1,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     2,     2,
       2,     2,     4,     4,     0,     1,     3,     1,     1,     0,
       0,     8,     2,     0,     0,    11,     2,     0,     0,     0,
       7,     1,     2,     2,     1,     1,     3,     3,     1,     0,
       0,     3,     3,     3,     2,     3,     3,     3,     3,     3,
       3,     3,     3
};

/* YYDEFACT[S] -- default rule to reduce with in state S when YYTABLE
   doesn't specify something else to do.  Zero means the default is an
   error. */
static const short yydefact[] =
{
       4,    25,    20,    24,    22,     0,    70,     0,     0,    58,
       0,     0,     0,    16,     0,     0,     0,     2,     7,     9,
       6,     5,    13,    26,    21,    14,    11,    12,     0,    44,
       0,    44,    25,    38,    20,    70,    70,     0,     0,    49,
       0,     0,     8,    10,     0,    70,    65,    64,    69,    39,
      40,    41,    17,    37,     0,     1,     4,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    18,    48,    47,
       0,    45,    19,     0,    38,    74,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    70,    70,     0,     0,
      59,     0,    68,    15,    27,     3,    23,    28,    34,    35,
      36,    30,    29,    31,    32,    33,     0,    42,    43,    71,
      81,    82,    75,    76,    77,    78,    79,    80,    72,    73,
       4,     0,     0,    67,    66,    46,    50,     0,     4,    53,
      57,     0,     4,     0,     0,    54,    60,    52,    51,    61,
       0,     0,    56,     0,    63,    62,     4,     0,    55,     0,
       0,     0
};

static const short yydefgoto[] =
{
     149,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      70,    71,    25,    88,   129,   133,    26,   143,   135,    27,
      45,   122,   142,    48,    93,    39
};

static const short yypact[] =
{
     103,   -10,    -1,-32768,-32768,   109,    84,    61,    24,-32768,
      94,    67,    15,   109,   109,   109,     9,-32768,-32768,-32768,
       1,   217,-32768,-32768,-32768,-32768,-32768,-32768,   109,    94,
      28,    94,     5,-32768,    19,    84,    84,    14,   184,    80,
      79,    82,-32768,-32768,    88,    84,     1,   217,   -29,-32768,
  -32768,-32768,   217,-32768,   140,-32768,   103,    28,   109,   109,
     109,   109,   109,   109,   109,   109,   109,   217,     1,   217,
     -13,-32768,     1,    37,-32768,-32768,   146,    -9,    28,    28,
     109,   109,   109,   109,   109,   109,    84,    84,    89,   109,
      80,    94,-32768,-32768,-32768,-32768,-32768,   208,   203,   203,
     203,     3,     3,    63,    63,-32768,    94,-32768,-32768,-32768,
       1,     1,   133,   133,   133,   133,   133,   133,-32768,-32768,
     103,   178,    97,     1,   217,-32768,   107,   109,   103,    96,
     198,     6,   103,    98,    13,-32768,-32768,   107,-32768,-32768,
     106,   121,-32768,   125,-32768,-32768,   103,    17,-32768,   138,
     143,-32768
};

static const short yypgoto[] =
{
  -32768,  -113,    91,   141,   147,    38,    -5,-32768,-32768,-32768,
     135,    34,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
  -32768,-32768,-32768,-32768,-32768,   -24
};


#define	YYLAST		257


static const short yytable[] =
{
      33,    38,    86,    87,    28,    47,    91,   126,    52,    53,
      54,    75,    77,    30,    56,   131,    55,    56,   139,   137,
      92,    90,   106,    67,    69,    56,    69,    44,    78,    79,
      74,    76,    34,   147,     4,   107,   136,    29,    57,   109,
      38,    64,    65,    66,    37,   148,    31,    51,    46,   140,
     141,    57,    29,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   118,   119,    40,    41,    31,    68,    72,    68,
      49,    50,   106,    37,    37,   112,   113,   114,   115,   116,
     117,    38,    38,    37,   121,   108,   124,    32,    34,     3,
       4,    86,    87,    28,    35,    96,    30,    32,    34,     3,
       4,    69,    89,    66,     5,   128,     1,     2,     3,     4,
     120,   144,    32,     5,     3,    56,   110,   111,   132,     5,
      14,   138,   130,     6,    37,    37,   145,     7,     8,   123,
      14,    36,     9,   146,    10,    11,    12,    13,   150,    14,
     125,    15,    58,   151,    68,    14,    61,    95,    42,    58,
      15,    59,    60,    61,    43,    58,    15,    59,    60,    61,
      80,    81,    82,    83,    84,    85,    73,     0,     0,    62,
      63,    64,    65,    66,     0,     0,    62,    63,    64,    65,
      66,     0,    62,    63,    64,    65,    66,    58,    94,    59,
      60,    61,     0,    58,    94,    59,    60,    61,    80,    81,
      82,    83,    84,    85,   127,     0,     0,    58,     0,    59,
      60,    61,    58,     0,    62,    63,    64,    65,    66,     0,
      62,    63,    64,    65,    66,   134,    58,     0,    59,    60,
      61,     0,     0,     0,    62,    63,    64,    65,    66,    62,
      63,    64,    65,    66,    62,    63,    64,    65,    66,     0,
       0,     0,     0,    62,    63,    64,    65,    66
};

static const short yycheck[] =
{
       5,     6,    11,    12,    14,    10,    35,   120,    13,    14,
      15,    35,    36,    14,     8,   128,     7,     8,     5,   132,
      49,    45,    35,    28,    29,     8,    31,     3,    14,    15,
      35,    36,     4,   146,     6,    48,    30,    47,    37,    48,
      45,    38,    39,    40,     6,    28,    47,    32,    10,    36,
      37,    37,    47,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    86,    87,     3,     4,    47,    29,    30,    31,
       3,     4,    35,    35,    36,    80,    81,    82,    83,    84,
      85,    86,    87,    45,    89,    48,    91,     3,     4,     5,
       6,    11,    12,    14,    10,    57,    14,     3,     4,     5,
       6,   106,    14,    40,    10,     8,     3,     4,     5,     6,
      21,     5,     3,    10,     5,     8,    78,    79,    22,    10,
      36,    23,   127,    20,    86,    87,     5,    24,    25,    91,
      36,    47,    29,     8,    31,    32,    33,    34,     0,    36,
     106,    47,     9,     0,   106,    36,    13,    56,     7,     9,
      47,    11,    12,    13,     7,     9,    47,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    31,    -1,    -1,    36,
      37,    38,    39,    40,    -1,    -1,    36,    37,    38,    39,
      40,    -1,    36,    37,    38,    39,    40,     9,    48,    11,
      12,    13,    -1,     9,    48,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    26,    -1,    -1,     9,    -1,    11,
      12,    13,     9,    -1,    36,    37,    38,    39,    40,    -1,
      36,    37,    38,    39,    40,    27,     9,    -1,    11,    12,
      13,    -1,    -1,    -1,    36,    37,    38,    39,    40,    36,
      37,    38,    39,    40,    36,    37,    38,    39,    40,    -1,
      -1,    -1,    -1,    36,    37,    38,    39,    40
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/share/bison/bison.simple"

/* Skeleton output parser for bison,

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software
   Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser when
   the %semantic_parser declaration is not specified in the grammar.
   It was written by Richard Stallman by simplifying the hairy parser
   used when %semantic_parser is specified.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

#if ! defined (yyoverflow) || defined (YYERROR_VERBOSE)

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || defined (YYERROR_VERBOSE) */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYLTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
# if YYLSP_NEEDED
  YYLTYPE yyls;
# endif
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAX (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# if YYLSP_NEEDED
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE) + sizeof (YYLTYPE))	\
      + 2 * YYSTACK_GAP_MAX)
# else
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAX)
# endif

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAX;	\
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif


#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");			\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).

   When YYLLOC_DEFAULT is run, CURRENT is set the location of the
   first token.  By default, to implement support for ranges, extend
   its range to the last symbol.  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)       	\
   Current.last_line   = Rhs[N].last_line;	\
   Current.last_column = Rhs[N].last_column;
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#if YYPURE
# if YYLSP_NEEDED
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, &yylloc, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval, &yylloc)
#  endif
# else /* !YYLSP_NEEDED */
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval)
#  endif
# endif /* !YYLSP_NEEDED */
#else /* !YYPURE */
# define YYLEX			yylex ()
#endif /* !YYPURE */


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)
/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
#endif /* !YYDEBUG */

/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif

#ifdef YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif
#endif

#line 315 "/usr/share/bison/bison.simple"


/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
#  define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL
# else
#  define YYPARSE_PARAM_ARG YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
# endif
#else /* !YYPARSE_PARAM */
# define YYPARSE_PARAM_ARG
# define YYPARSE_PARAM_DECL
#endif /* !YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
# ifdef YYPARSE_PARAM
int yyparse (void *);
# else
int yyparse (void);
# endif
#endif

/* YY_DECL_VARIABLES -- depending whether we use a pure parser,
   variables are global, or local to YYPARSE.  */

#define YY_DECL_NON_LSP_VARIABLES			\
/* The lookahead symbol.  */				\
int yychar;						\
							\
/* The semantic value of the lookahead symbol. */	\
YYSTYPE yylval;						\
							\
/* Number of parse errors so far.  */			\
int yynerrs;

#if YYLSP_NEEDED
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES			\
						\
/* Location data for the lookahead symbol.  */	\
YYLTYPE yylloc;
#else
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES
#endif


/* If nonreentrant, generate the variables here. */

#if !YYPURE
YY_DECL_VARIABLES
#endif  /* !YYPURE */

int
yyparse (YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  /* If reentrant, generate the variables here. */
#if YYPURE
  YY_DECL_VARIABLES
#endif  /* !YYPURE */

  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yychar1 = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack. */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;

#if YYLSP_NEEDED
  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
#endif

#if YYLSP_NEEDED
# define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
# define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  YYSIZE_T yystacksize = YYINITDEPTH;


  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
#if YYLSP_NEEDED
  YYLTYPE yyloc;
#endif

  /* When reducing, the number of symbols on the RHS of the reduced
     rule. */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;
#if YYLSP_NEEDED
  yylsp = yyls;
#endif
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  */
# if YYLSP_NEEDED
	YYLTYPE *yyls1 = yyls;
	/* This used to be a conditional around just the two extra args,
	   but that might be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);
	yyls = yyls1;
# else
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);
# endif
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);
# if YYLSP_NEEDED
	YYSTACK_RELOCATE (yyls);
# endif
# undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
#if YYLSP_NEEDED
      yylsp = yyls + yysize - 1;
#endif

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yychar1 = YYTRANSLATE (yychar);

#if YYDEBUG
     /* We have to keep this `#if YYDEBUG', since we use variables
	which are defined only if `YYDEBUG' is set.  */
      if (yydebug)
	{
	  YYFPRINTF (stderr, "Next token is %d (%s",
		     yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise
	     meaning of a token, for further debugging info.  */
# ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
# endif
	  YYFPRINTF (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %d (%s), ",
	      yychar, yytname[yychar1]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to the semantic value of
     the lookahead token.  This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

#if YYLSP_NEEDED
  /* Similarly for the default location.  Let the user run additional
     commands if for instance locations are ranges.  */
  yyloc = yylsp[1-yylen];
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
#endif

#if YYDEBUG
  /* We have to keep this `#if YYDEBUG', since we use variables which
     are defined only if `YYDEBUG' is set.  */
  if (yydebug)
    {
      int yyi;

      YYFPRINTF (stderr, "Reducing via rule %d (line %d), ",
		 yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (yyi = yyprhs[yyn]; yyrhs[yyi] > 0; yyi++)
	YYFPRINTF (stderr, "%s ", yytname[yyrhs[yyi]]);
      YYFPRINTF (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif

  switch (yyn) {

case 1:
#line 495 "gram.y"
{ YYACCEPT; }
    break;
case 5:
#line 504 "gram.y"
{ gen_spop(); }
    break;
case 6:
#line 505 "gram.y"
{ gen_spop(); }
    break;
case 16:
#line 515 "gram.y"
{ gen_call("RANDOMIZE",0); gen_spop(); }
    break;
case 17:
#line 516 "gram.y"
{ gen_call("RANDOMIZE",1); gen_spop(); }
    break;
case 18:
#line 520 "gram.y"
{ gen_popsymbol(yyvsp[-2].symbol);b_free(yyvsp[-2].symbol); }
    break;
case 19:
#line 524 "gram.y"
{ gen_popsymbol(yyvsp[-2].symbol);b_free(yyvsp[-2].symbol); }
    break;
case 20:
#line 528 "gram.y"
{ gen_pushsymbol(yyvsp[0].symbol);b_free(yyvsp[0].symbol); }
    break;
case 22:
#line 530 "gram.y"
{ gen_pushstring(yyvsp[0].string); b_free(yyvsp[0].string); }
    break;
case 23:
#line 531 "gram.y"
{ gen_binop('#'); }
    break;
case 24:
#line 535 "gram.y"
{ gen_pushnumber(yyvsp[0].number); }
    break;
case 25:
#line 536 "gram.y"
{ gen_pushsymbol(yyvsp[0].symbol);b_free(yyvsp[0].symbol); }
    break;
case 28:
#line 539 "gram.y"
{ gen_binop('%'); }
    break;
case 29:
#line 540 "gram.y"
{ gen_binop('+'); }
    break;
case 30:
#line 541 "gram.y"
{ gen_binop('-'); }
    break;
case 31:
#line 542 "gram.y"
{ gen_binop('*'); }
    break;
case 32:
#line 543 "gram.y"
{ gen_binop('/'); }
    break;
case 33:
#line 544 "gram.y"
{ gen_binop('!'); }
    break;
case 34:
#line 545 "gram.y"
{ gen_binop('|'); }
    break;
case 35:
#line 546 "gram.y"
{ gen_binop('&'); }
    break;
case 36:
#line 547 "gram.y"
{ gen_binop('^'); }
    break;
case 37:
#line 548 "gram.y"
{ gen_unop('-');}
    break;
case 38:
#line 549 "gram.y"
{ gen_unop('~'); }
    break;
case 39:
#line 553 "gram.y"
{ begin_subroutine(yyvsp[0].symbol);b_free(yyvsp[0].symbol); }
    break;
case 40:
#line 554 "gram.y"
{ begin_subroutine(yyvsp[0].symbol);b_free(yyvsp[0].symbol); }
    break;
case 41:
#line 555 "gram.y"
{ gen_return(); }
    break;
case 42:
#line 560 "gram.y"
{ gen_call(yyvsp[-3].symbol,(int)yyvsp[-1].number); b_free(yyvsp[-3].symbol); }
    break;
case 43:
#line 565 "gram.y"
{ gen_call(yyvsp[-3].symbol,(int)yyvsp[-1].number); b_free(yyvsp[-3].symbol); }
    break;
case 44:
#line 570 "gram.y"
{ yyval.number = 0; }
    break;
case 45:
#line 571 "gram.y"
{ yyval.number = 1; }
    break;
case 46:
#line 572 "gram.y"
{ yyval.number = yyvsp[-2].number + 1; }
    break;
case 49:
#line 581 "gram.y"
{ gen_if(); }
    break;
case 50:
#line 582 "gram.y"
{ gen_then(); }
    break;
case 51:
#line 584 "gram.y"
{ gen_endif(); }
    break;
case 54:
#line 593 "gram.y"
{ gen_for (yyvsp[-5].symbol, yyvsp[0].number); }
    break;
case 55:
#line 594 "gram.y"
{ gen_next(yyvsp[-9].symbol, yyvsp[-4].number);b_free(yyvsp[-9].symbol); }
    break;
case 56:
#line 598 "gram.y"
{ yyval.number = yyvsp[0].number; }
    break;
case 57:
#line 599 "gram.y"
{ yyval.number = 1.0; }
    break;
case 58:
#line 603 "gram.y"
{ gen_while_label(); }
    break;
case 59:
#line 603 "gram.y"
{ gen_while(); }
    break;
case 60:
#line 604 "gram.y"
{ gen_wend(); }
    break;
case 61:
#line 608 "gram.y"
{yyval.number=yyvsp[0].number;}
    break;
case 62:
#line 608 "gram.y"
{yyval.number=yyvsp[0].number;}
    break;
case 63:
#line 608 "gram.y"
{yyval.number=-yyvsp[0].number;}
    break;
case 64:
#line 611 "gram.y"
{ gen_print(); }
    break;
case 65:
#line 612 "gram.y"
{ gen_print(); }
    break;
case 66:
#line 614 "gram.y"
{ gen_pushstring(" "); gen_print(); gen_print(); }
    break;
case 67:
#line 616 "gram.y"
{ gen_pushstring(" "); gen_print(); gen_print(); }
    break;
case 69:
#line 621 "gram.y"
{ gen_pushstring("\n");gen_print(); }
    break;
case 72:
#line 626 "gram.y"
{ gen_comp(BS_COMP_OR); }
    break;
case 73:
#line 627 "gram.y"
{ gen_comp(BS_COMP_AND); }
    break;
case 74:
#line 628 "gram.y"
{ gen_comp(BS_COMP_NOT); }
    break;
case 75:
#line 629 "gram.y"
{ gen_comp(BS_COMP_EQ);  }
    break;
case 76:
#line 630 "gram.y"
{ gen_comp(BS_COMP_NE);  }
    break;
case 77:
#line 631 "gram.y"
{ gen_comp(BS_COMP_LT);  }
    break;
case 78:
#line 632 "gram.y"
{ gen_comp(BS_COMP_LE);  }
    break;
case 79:
#line 633 "gram.y"
{ gen_comp(BS_COMP_GT);  }
    break;
case 80:
#line 634 "gram.y"
{ gen_comp(BS_COMP_GE);  }
    break;
case 81:
#line 635 "gram.y"
{ gen_comp(BS_COMP_SEQ); }
    break;
case 82:
#line 636 "gram.y"
{ gen_comp(BS_COMP_SNE); }
    break;
}

#line 705 "/usr/share/bison/bison.simple"


  yyvsp -= yylen;
  yyssp -= yylen;
#if YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;
#if YYLSP_NEEDED
  *++yylsp = yyloc;
#endif

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("parse error, unexpected ") + 1;
	  yysize += yystrlen (yytname[YYTRANSLATE (yychar)]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "parse error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[YYTRANSLATE (yychar)]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exhausted");
	}
      else
#endif /* defined (YYERROR_VERBOSE) */
	yyerror ("parse error");
    }
  goto yyerrlab1;


/*--------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action |
`--------------------------------------------------*/
yyerrlab1:
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;
      YYDPRINTF ((stderr, "Discarding token %d (%s).\n",
		  yychar, yytname[yychar1]));
      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;


/*-------------------------------------------------------------------.
| yyerrdefault -- current state does not do anything special for the |
| error token.                                                       |
`-------------------------------------------------------------------*/
yyerrdefault:
#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */

  /* If its default is to accept any token, ok.  Otherwise pop it.  */
  yyn = yydefact[yystate];
  if (yyn)
    goto yydefault;
#endif


/*---------------------------------------------------------------.
| yyerrpop -- pop the current state because it cannot handle the |
| error token                                                    |
`---------------------------------------------------------------*/
yyerrpop:
  if (yyssp == yyss)
    YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#if YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "Error: state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

/*--------------.
| yyerrhandle.  |
`--------------*/
yyerrhandle:
  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

/*---------------------------------------------.
| yyoverflowab -- parser overflow comes here.  |
`---------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}
#line 638 "gram.y"

