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

#include "bscript.h"
#include "intern.h"
#include "gram.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Redefine input source */
int CXYYInput(char *buf, int max_size);
#undef YY_INPUT
#define YY_INPUT(buf, result, max_size) \
        (result = CXYYInput(buf, max_size))

/* in cxengine.c */
void compile_error (long, char*, char*);

/* in gram.y */
void yyparse();

/* local stuff */
long compile_lineno;
int yywrap(void);
int yyerror(char*);
int is_keyword (char*);
int yylex();
double create_number (char*);
char* create_string (char*);

%}

digit [0-9]
alphanum [_a-zA-Z][a-zA-Z0-9]*
whitespace [ \t\f\r\v]
remark [Rr][Ee][Mm]

%x RET

%%

<<EOF>> { return T_ENDOFFILE;}

{whitespace}+ {BEGIN(INITIAL);}     /* ignore whitespace */

\n { ++compile_lineno; return T_SEPARATOR; }
: { return T_SEPARATOR; }

{remark}{whitespace}+.* { return T_SEPARATOR;}  /* comments span til eol */
#+.* { return T_SEPARATOR;}  /* UNIX-style comments */
{remark}\n { compile_lineno++; return T_SEPARATOR;}
{remark} {yymore();}


{alphanum}+  {
   int i = is_keyword(yytext);
   if (i) return i;
   yylval.symbol = b_strdup(yytext);
   return SYMBOL;
   }

{alphanum}+"$" {
   int i = is_keyword(yytext);
   if (i) return i;
   yylval.symbol = b_strdup(yytext);
   return STRSYMBOL;
}

(({digit}+|({digit}*\.{digit}*))([eE][-+]?{digit}+)?) {
   yylval.number = create_number(yytext);
   return NUMBER;
}

\"([^\"\n]|"\\\"")*\" {
  yylval.string = create_string(yytext);
  return STRING;
}

"=" return T_EQ;
"<>" return T_NE;
"<" return T_LT;
"<=" return T_LE;
">" return T_GT;
">=" return T_GE;


. return *yytext;


%%

/* dummy function so we won't need libfl.a */
int yywrap(){ return 1; }

/* spawn to compile_error() in cxengine.c */
int yyerror (char *msg)
{
  extern char* yytext;
  compile_error(compile_lineno, msg, yytext);
  return 0;
}

/* return appropriate token if given string is keyword */
/* FIXME use hash-table instead */
#include <string.h>
int is_keyword (char *s)
{
  long i, j;
  struct key { char* name; int token; }
  keywords[] = {
    { "let",        T_LET     },
    { "sub",        T_SUB     },
    { "subroutine", T_SUB     },
    { "end",        T_END     },
    { "mod",        T_MOD     },
    { "not",        T_NOT     },
    { "or",         T_OR      },
    { "and",        T_AND     },
    { "xor",        T_XOR     },
    { "print",      T_PRINT   },
    { "if",         T_IF      },
    { "then",       T_THEN    },
    { "else",       T_ELSE    },
    { "endif",      T_ENDIF   },
    { "for",        T_FOR     },
    { "to",         T_TO      },
    { "step",       T_STEP    },
    { "next",       T_NEXT    },
    { "while",      T_WHILE   },
    { "wend",       T_WEND    },
    { "randomize",  T_RANDOMIZE },
    { (char*) 0L, 0 }};

  for (i = 0; keywords[i].token; i++)
    if(strlen(s) == strlen(keywords[i].name)){
       for (j = 0; j < (long)strlen(s); j++)
         if(toupper(s[j]) != toupper(keywords[i].name[j])) break;
       if (j >= (long)strlen(s)) return keywords[i].token;
       }
  return 0;
}

static char *CXInputStream;
static int InputStreamLength;

/*  YY_INPUT redirects flex to get input from this function */
#ifndef min
        #define min(a,b) ((a) < (b) ? (a) : (b))
#endif
int CXYYInput(char *buf, int max_size)
{
         int n = min(max_size, InputStreamLength );
        if(n > 0)
        {
                memcpy(buf, CXInputStream, n);
                CXInputStream += n;
                InputStreamLength -= n;
        }
        return n;
}

/* CXParse runs the compiler */
void CXParse(char *InputStream)
{
        CXInputStream = InputStream;
        InputStreamLength = strlen(CXInputStream);
        yyrestart(0);
        yyparse();
		yy_delete_buffer( yy_current_buffer );		
}

/* create a number */
double create_number(char* s)
{
   return atof(s);
}

/* create a new string, remove the double-quotes */
/* FIXME escape character ? */
char* create_string(char *s)
{
  char *p;

  p = b_strdup(s);
  if (!p) return p;
  if (*p == '"') memcpy (p, p+1, strlen(p));
  if (p[strlen(p)-1] == '"') p[strlen(p)-1] = '\0';
  return p;
}


