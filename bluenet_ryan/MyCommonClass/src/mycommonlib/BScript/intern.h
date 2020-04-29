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

#ifndef __INTERN_H_20050428__
#define __INTERN_H_20050428__

#include <stdlib.h>

#include "bscript.h"
#include "blib.h"

/* bytecode (kind) */
#define PCODE_TERMINATE  1
#define PCODE_PUSHSYMBOL 2
#define PCODE_POPSYMBOL  3
#define PCODE_PUSHVALUE 4
#define PCODE_JUMP       6
#define PCODE_LABEL      7
#define PCODE_IF         8
#define PCODE_CALL       10
#define PCODE_RETURN     11
#define PCODE_SSWAP 12
#define PCODE_SCOPY 13
#define PCODE_SPOP  14
#define PCODE_UNOP 15
#define PCODE_BINOP 16
#define PCODE_COMP 17
#define PCODE_PRINT 18

/* for PCODE_COMP */
#define BS_COMP_NOT  1
#define BS_COMP_OR   2
#define BS_COMP_AND  3
#define BS_COMP_EQ   4
#define BS_COMP_NE   5
#define BS_COMP_LT   6
#define BS_COMP_LE   7
#define BS_COMP_GT   8
#define BS_COMP_GE   9
#define BS_COMP_SEQ 10
#define BS_COMP_SNE 11

#ifdef __cplusplus
extern "C" {
#endif

/* gram.y */
BList* bscript_parse( char* script );

/* runtime.c - run-time functions */
BVariant* function_macheps( BSContext *context, BList *args );
BVariant* function_sgn( BSContext *context, BList *args );
BVariant* function_abs( BSContext *context, BList *args );
BVariant* function_exp( BSContext *context, BList *args );
BVariant* function_log( BSContext *context, BList *args );
BVariant* function_log10( BSContext *context, BList *args );
BVariant* function_log2( BSContext *context, BList *args );
BVariant* function_int( BSContext *context, BList *args );
BVariant* function_cint( BSContext *context, BList *args );
BVariant* function_sqr( BSContext *context, BList *args );
BVariant* function_pi( BSContext *context, BList *args );
BVariant* function_degree( BSContext *context, BList *args );
BVariant* function_radian( BSContext *context, BList *args );

BVariant* function_sin( BSContext *context, BList *args );
BVariant* function_asin( BSContext *context, BList *args );
BVariant* function_cos( BSContext *context, BList *args );
BVariant* function_acos( BSContext *context, BList *args );
BVariant* function_tan( BSContext *context, BList *args );
BVariant* function_atn( BSContext *context, BList *args );
BVariant* function_sinh( BSContext *context, BList *args );
BVariant* function_asinh( BSContext *context, BList *args );
BVariant* function_cosh( BSContext *context, BList *args );
BVariant* function_acosh( BSContext *context, BList *args );
BVariant* function_tanh( BSContext *context, BList *args );
BVariant* function_atnh( BSContext *context, BList *args );

BVariant* function_len( BSContext *context, BList *args );
BVariant* function_val( BSContext *context, BList *args );
BVariant* function_asc( BSContext *context, BList *args );
BVariant* function_left( BSContext *context, BList *args );
BVariant* function_right( BSContext *context, BList *args );
BVariant* function_mid( BSContext *context, BList *args );
BVariant* function_instr( BSContext *context, BList *args );
BVariant* function_str( BSContext *context, BList *args );
BVariant* function_chr( BSContext *context, BList *args );
BVariant* function_string( BSContext *context, BList *args );
BVariant* function_lower( BSContext *context, BList *args );
BVariant* function_upper( BSContext *context, BList *args );
BVariant* function_space( BSContext *context, BList *args );
BVariant* function_hex( BSContext *context, BList *args );
BVariant* function_oct( BSContext *context, BList *args );

BVariant* function_date( BSContext *context, BList *args );
BVariant* function_time( BSContext *context, BList *args );
BVariant* function_timer( BSContext *context, BList *args );
BVariant* function_environ( BSContext *context, BList *args );
BVariant* function_randomize( BSContext *context, BList *args );
BVariant* function_rnd( BSContext *context, BList *args );

#ifdef __cplusplus
}
#endif

#endif	// __INTERN_H_20050428__
