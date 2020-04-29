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

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bscript.h"
#include "intern.h"
#include "blib.h"

/* workaround some stupid math.h out there */
#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795029
#endif


/* COMPATIBILITY MACRO for srandom() in glibc */

/* handle mingw 2.95.3 */
#ifdef __GNUC__
#ifndef srandom
#define srandom(x) srand((x))  /* srand() is in stdlib.h */
#endif
#endif

/* handle Borland C++ 5.0 */
#ifdef __BORLANDC__
#ifndef srandom
#define srandom(x) srand((x))  /* srand() is in stdlib.h */
#endif
#endif

/* handle Turbo C++ 3.0 */
#ifdef __TURBOC__
#ifndef srandom
#define srandom(x) srand((x))  /* srand() is in stdlib.h */
#endif
#endif

#ifdef _MSC_VER
	#define srandom(x) srand((x))  /* srand() is in stdlib.h */	
	#define asinh	asin
	#define acosh	acos
	#define atanh	atan
#endif //

/* return machine epsilon  */
BVariant*
function_macheps( BSContext *context, BList *args )
{
  BVariant *result;
  double eps;

  /* discard whatever arguments */

  eps = b_machine_epsilon();

  /* set the result */
  result = bvariant_new();
  bvariant_set_double( result, eps );

  return result;
}

/* signum function */
BVariant*
function_sgn( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  BVariant *result;
  double num;
  double signum;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 1 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg, BV_DOUBLE );
  num = bvariant_as_double( arg );
  bvariant_free( arg );

  signum = 0;
  if( b_is_negative(num) ) signum = -1;
  if( b_is_positive(num) ) signum = 1;

  /* set the result */
  result = bvariant_new();
  bvariant_set_double( result, signum );

  return result;
}

/* absolute value */
BVariant*
function_abs( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  BVariant *result;
  double num;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 1 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg, BV_DOUBLE );
  num = bvariant_as_double( arg );
  bvariant_free( arg );

  num = fabs( num );

  /* set the result */
  result = bvariant_new();
  bvariant_set_double( result, num );

  return result;
}

/* exponential */
BVariant*
function_exp( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  BVariant *result;
  double num;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 1 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg, BV_DOUBLE );
  num = bvariant_as_double( arg );
  bvariant_free( arg );

  num = exp( num );

  /* set the result */
  result = bvariant_new();
  bvariant_set_double( result, num );

  return result;
}

/* natural logarithm */
BVariant*
function_log( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  BVariant *result;
  double num;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 1 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg, BV_DOUBLE );
  num = bvariant_as_double( arg );
  bvariant_free( arg );

  /* check argument, cannot be negative */
  if( b_is_negative( num ) )
  {
    /* raise exception */
    return NULL;
  }

  num = log( num );

  /* set the result */
  result = bvariant_new();
  bvariant_set_double( result, num );

  return result;
}

/* base-10 logarithm */
BVariant*
function_log10( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  BVariant *result;
  double num;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 1 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg, BV_DOUBLE );
  num = bvariant_as_double( arg );
  bvariant_free( arg );

  /* check argument, cannot be negative */
  if( b_is_negative( num ) )
  {
    /* raise exception */
    return NULL;
  }

  num = log10( num );

  /* set the result */
  result = bvariant_new();
  bvariant_set_double( result, num );

  return result;
}

/* base-2 logarithm */
BVariant*
function_log2( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  BVariant *result;
  double num;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 1 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg, BV_DOUBLE );
  num = bvariant_as_double( arg );
  bvariant_free( arg );

  /* check argument, cannot be negative */
  if( b_is_negative( num ) )
  {
    /* raise exception */
    return NULL;
  }

  num = log(num)/log(2);

  /* set the result */
  result = bvariant_new();
  bvariant_set_double( result, num );

  return result;
}

/* floor */
BVariant*
function_int( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  BVariant *result;
  double num;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 1 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg, BV_DOUBLE );
  num = bvariant_as_double( arg );
  bvariant_free( arg );

  num = floor( num );

  /* set the result */
  result = bvariant_new();
  bvariant_set_double( result, num );

  return result;
}

/* truncated integer, see also function_int */
/* NOTE int(-2.3) is -3 while cint(-2.3) is -2 */
BVariant*
function_cint( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  BVariant *result;
  double num;
  int i;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 1 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg, BV_DOUBLE );
  num = bvariant_as_double( arg );
  bvariant_free( arg );

  /* truncate the integer */
  i = (int) num;
  num = (double) i;

  /* set the result */
  result = bvariant_new();
  bvariant_set_double( result, num );

  return result;
}

/* square root */
BVariant*
function_sqr( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  BVariant *result;
  double num;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 1 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg, BV_DOUBLE );
  num = bvariant_as_double( arg );
  bvariant_free( arg );

  /* check argument, cannot be negative */
  if( b_is_negative( num ) )
  {
    /* raise exception */
    return NULL;
  }

  num = sqrt( num );

  /* set the result */
  result = bvariant_new();
  bvariant_set_double( result, num );

  return result;
}

/* return PI */
BVariant*
function_pi( BSContext *context, BList *args )
{
  int narg;
  BVariant *result;
  double num;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 0 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  num = M_PI;

  /* set the result */
  result = bvariant_new();
  bvariant_set_double( result, num );

  return result;
}

/* convert angle from radian to degree */
BVariant*
function_degree( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  BVariant *result;
  double radian;
  double degree;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 1 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg, BV_DOUBLE );
  radian = bvariant_as_double( arg );
  bvariant_free( arg );

  degree = radian * 180 / M_PI;

  /* set the result */
  result = bvariant_new();
  bvariant_set_double( result, degree );

  return result;
}

/* convert angle from degree to radian */
BVariant*
function_radian( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  BVariant *result;
  double radian;
  double degree;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 1 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg, BV_DOUBLE );
  degree = bvariant_as_double( arg );
  bvariant_free( arg );

  radian = degree * M_PI / 180;

  /* set the result */
  result = bvariant_new();
  bvariant_set_double( result, radian );

  return result;
}

/* sine function, argument in radian */
BVariant*
function_sin( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  BVariant *result;
  double num;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 1 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg, BV_DOUBLE );
  num = bvariant_as_double( arg );
  bvariant_free( arg );

  num = sin( num );

  /* set the result */
  result = bvariant_new();
  bvariant_set_double( result, num );

  return result;
}

/* arcsine function */
BVariant*
function_asin( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  BVariant *result;
  double num;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 1 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg, BV_DOUBLE );
  num = bvariant_as_double( arg );
  bvariant_free( arg );

  num = asin( num );

  /* set the result */
  result = bvariant_new();
  bvariant_set_double( result, num );

  return result;
}

/* cosine function, argument in radian */
BVariant*
function_cos( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  BVariant *result;
  double num;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 1 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg, BV_DOUBLE );
  num = bvariant_as_double( arg );
  bvariant_free( arg );

  num = cos( num );

  /* set the result */
  result = bvariant_new();
  bvariant_set_double( result, num );

  return result;
}

/* arccosine function */
BVariant*
function_acos( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  BVariant *result;
  double num;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 1 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg, BV_DOUBLE );
  num = bvariant_as_double( arg );
  bvariant_free( arg );

  num = acos( num );

  /* set the result */
  result = bvariant_new();
  bvariant_set_double( result, num );

  return result;
}

/* tangent function, argument in radian */
BVariant*
function_tan( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  BVariant *result;
  double num;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 1 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg, BV_DOUBLE );
  num = bvariant_as_double( arg );
  bvariant_free( arg );

  num = tan( num );

  /* set the result */
  result = bvariant_new();
  bvariant_set_double( result, num );

  return result;
}

/* arctangen function */
BVariant*
function_atn( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  BVariant *result;
  double num;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 1 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg, BV_DOUBLE );
  num = bvariant_as_double( arg );
  bvariant_free( arg );

  num = atan( num );

  /* set the result */
  result = bvariant_new();
  bvariant_set_double( result, num );

  return result;
}

/* hyperbolic sine function, argument in radian */
BVariant*
function_sinh( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  BVariant *result;
  double num;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 1 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg, BV_DOUBLE );
  num = bvariant_as_double( arg );
  bvariant_free( arg );

  num = sinh( num );

  /* set the result */
  result = bvariant_new();
  bvariant_set_double( result, num );

  return result;
}

/* hyperbolic arcsine function */
BVariant*
function_asinh( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  BVariant *result;
  double num;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 1 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg, BV_DOUBLE );
  num = bvariant_as_double( arg );
  bvariant_free( arg );

  num = asinh( num );

  /* set the result */
  result = bvariant_new();
  bvariant_set_double( result, num );

  return result;
}

/* hyperbolic cosine function, argument in radian */
BVariant*
function_cosh( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  BVariant *result;
  double num;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 1 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg, BV_DOUBLE );
  num = bvariant_as_double( arg );
  bvariant_free( arg );

  num = cosh( num );

  /* set the result */
  result = bvariant_new();
  bvariant_set_double( result, num );

  return result;
}

/* hyperbolic arccosine function */
BVariant*
function_acosh( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  BVariant *result;
  double num;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 1 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg, BV_DOUBLE );
  num = bvariant_as_double( arg );
  bvariant_free( arg );

  num = acosh( num );

  /* set the result */
  result = bvariant_new();
  bvariant_set_double( result, num );

  return result;
}

/* hyperbolic tangent function, argument in radian */
BVariant*
function_tanh( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  BVariant *result;
  double num;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 1 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg, BV_DOUBLE );
  num = bvariant_as_double( arg );
  bvariant_free( arg );

  num = tanh( num );

  /* set the result */
  result = bvariant_new();
  bvariant_set_double( result, num );

  return result;
}

/* hyperbolic arctangen function */
BVariant*
function_atnh( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  BVariant *result;
  double num;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 1 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg, BV_DOUBLE );
  num = bvariant_as_double( arg );
  bvariant_free( arg );

  num = atanh( num );

  /* set the result */
  result = bvariant_new();
  bvariant_set_double( result, num );

  return result;
}



/* get length of string */
BVariant*
function_len( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  BVariant *result;
  char *string;
  double x;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 1 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg, BV_STRING );
  string = b_strdup( bvariant_as_string( arg ) );
  bvariant_free( arg );

  x = (double) strlen( string );
  b_free( string );

  /* set the result */
  result = bvariant_new();
  bvariant_set_double( result, x );

  return result;
}

/* convert a string to a number */
/* FIXME could me implemented without sscanf ? */
BVariant*
function_val( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  BVariant *result;
  char *string;
  double x;
  int i;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 1 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg, BV_STRING );
  string = b_strdup( bvariant_as_string( arg ) );
  bvariant_free( arg );

  i = sscanf (string, "%lf", &x);
  if (i != 1) x = 0;
  b_free( string );

  /* set the result */
  result = bvariant_new();
  bvariant_set_double( result, x );

  return result;
}

/* get ASCII value of first char in string */
BVariant*
function_asc( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  BVariant *result;
  char *string;
  double x;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 1 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg, BV_STRING );
  string = b_strdup( bvariant_as_string( arg ) );
  bvariant_free( arg );

  x = (double) string[0];
  b_free( string );

  /* set the result */
  result = bvariant_new();
  bvariant_set_double( result, x );

  return result;
}

/* get first n characters from a string */
BVariant*
function_left( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg1, *arg2;
  BVariant *result;
  char *string;
  int i;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 2 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg1 = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  arg2 = bvariant_copy( (BVariant*) blist_nth_data( args, 1 ) );
  bvariant_cast( arg1, BV_STRING );
  bvariant_cast( arg2, BV_INTEGER );
  string = b_strdup( bvariant_as_string( arg1 ) );
  i = bvariant_as_integer( arg2 );
  bvariant_free( arg1 );
  bvariant_free( arg2 );

  /* truncate the string */
  if (i < (long)strlen(string)) 
	  string[i] = '\0';

  /* set the result */
  result = bvariant_new();
  bvariant_set_string( result, string );
  b_free( string );

  return result;
}

/* get last n characters from a string */
BVariant*
function_right( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg1, *arg2;
  BVariant *result;
  char *string;
  int i;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 2 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg1 = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  arg2 = bvariant_copy( (BVariant*) blist_nth_data( args, 1 ) );
  bvariant_cast( arg1, BV_STRING );
  bvariant_cast( arg2, BV_INTEGER );
  string = b_strdup( bvariant_as_string( arg1 ) );
  i = bvariant_as_integer( arg2 );
  bvariant_free( arg1 );
  bvariant_free( arg2 );

  /* copy n last characters  */
  if (i < (long)strlen(string))
    memcpy (string, string + strlen(string) - i, i + 1);

  /* set the result */
  result = bvariant_new();
  bvariant_set_string( result, string );
  b_free( string );

  return result;
}

/* grab portion of a string */
BVariant*
function_mid( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg1, *arg2, *arg3;
  BVariant *result;
  char *string;
  int i, j;

  /* check number of arguments */
  narg = blist_length( args );
  if(( narg != 3 ) && ( narg !=2 ))
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg1 = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg1, BV_STRING );
  string = b_strdup( bvariant_as_string( arg1 ) );
  bvariant_free( arg1 );

  /* this is bad ? */
  if( !string )
    return NULL;

  /* second argument */
  arg2 = bvariant_copy( (BVariant*) blist_nth_data( args, 1 ) );
  bvariant_cast( arg2, BV_INTEGER );
  i = bvariant_as_integer( arg2 );
  bvariant_free( arg2 );
  if( i<0 ) i = 0;

  /* the third is optional */
  if( narg == 3 )
  {
    arg3 = bvariant_copy( (BVariant*) blist_nth_data( args, 2 ) );
    bvariant_cast( arg3, BV_INTEGER );
    j = bvariant_as_integer( arg3 );
    bvariant_free( arg3 );
    if( j<0 ) j = 0;
  }
  else
    j = strlen( string );

  /* more than length of string ? */
  if (i > (long)strlen(string)) 
	  i = strlen(string);
  if (i + j > (long)strlen(string) )
	  j = strlen(string) - i + 1;

  /* do a proper copy */
  /* NOTE in MID$, pos is started from 1 not 0 */
  memcpy (string, string + i - 1, j);
  string[j] = '\0';

  /* set the result */
  result = bvariant_new();
  bvariant_set_string( result, string );
  b_free( string );

  return result;
}

/* search from substring: INSTR$([start-pos],string,pattern)  */
BVariant*
function_instr( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg1, *arg2, *arg3;
  BVariant *result;
  int startpos;
  char *string;
  char *ptr;
  char *pattern;
  char *where;
  int pos;

  /* check number of arguments */
  narg = blist_length( args );
  if(( narg != 3 ) && ( narg !=2 ))
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg1 = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  arg2 = bvariant_copy( (BVariant*) blist_nth_data( args, 1 ) );
  
  /* first argument is optional */
  if( narg == 2 )
  {
    startpos = 1;
    bvariant_cast( arg1, BV_STRING );
    string = b_strdup( bvariant_as_string( arg1 ) );
    bvariant_free( arg1 );
    bvariant_cast( arg2, BV_STRING );
    pattern = b_strdup( bvariant_as_string( arg2 ) );
    bvariant_free( arg2 );
  }
  else
  {
    arg3 = bvariant_copy( (BVariant*) blist_nth_data( args, 2 ) );
    bvariant_cast( arg1, BV_INTEGER );
    startpos = bvariant_as_integer( arg1 );
    bvariant_free( arg1 );
    bvariant_cast( arg2, BV_STRING );
    string = b_strdup( bvariant_as_string( arg2 ) );
    bvariant_free( arg2 );
    bvariant_cast( arg3, BV_STRING );
    pattern = b_strdup( bvariant_as_string( arg3 ) );
    bvariant_free( arg3 );
  }

  /* search for the pattern */
  /* NOTE in INSTR$, startpos is from 1 not 0 */
  ptr = string + startpos - 1;
  where = strstr( ptr, pattern );
  if( !where ) pos = 0;
  else pos = where - string + 1;

  b_free( string );
  b_free( pattern );

  /* set the result */
  result = bvariant_new();
  bvariant_set_double( result, pos );

  return result;
}

/* convert number to string representation */
/* FIXME make it safe against buffer overflow */
BVariant*
function_str( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  BVariant *result;
  char str[256];
  double num;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 1 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg, BV_DOUBLE );
  num = bvariant_as_double( arg );
  bvariant_free( arg );

  sprintf( str, "%g", num );

  /* set the result */
  result = bvariant_new();
  bvariant_set_string( result, str );

  return result;
}

/* get character of given ASCII code */
BVariant*
function_chr( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  BVariant *result;
  char str[2];

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 1 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg, BV_INTEGER );
  str[0] = (char) bvariant_as_integer( arg );
  str[1] = '\0';
  bvariant_free( arg );

  /* set the result */
  result = bvariant_new();
  bvariant_set_string( result, str );

  return result;
}

/* repeat a character n times   */
BVariant*
function_string( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg1, *arg2;
  BVariant *result;
  int i, n;
  char ch;
  char *string;

  /* check number of arguments */
  narg = blist_length( args );
  if(( narg != 3 ) && ( narg !=2 ))
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg1 = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg1, BV_INTEGER );
  n = bvariant_as_integer( arg1 );
  bvariant_free( arg1 );

  arg2 = bvariant_copy( (BVariant*) blist_nth_data( args, 1 ) );
  if( arg2->type == BV_STRING )
  {
    bvariant_cast( arg2, BV_STRING );
    string = bvariant_as_string( arg2 );
    ch = string[0];
  }
  else if( ( arg2->type == BV_INTEGER ) ||
           ( arg2->type == BV_DOUBLE ))
    {
      bvariant_cast( arg2, BV_INTEGER );
      ch = (char)bvariant_as_integer( arg2 );
    }
  else
    ch = 32; /* fallback, use space instead */

  bvariant_free( arg2 );

  /* repeat the characters */
  if( n < 0 ) n = 0;
  string = b_malloc( n+1 );
  for( i=0; i<n; i++ ) string[i] = ch;
  string[n] = '\0';

  /* set the result */
  result = bvariant_new();
  bvariant_set_string( result, string );
  b_free( string );

  return result;
}

/* convert string to lowercase  */
BVariant*
function_lower( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  BVariant *result;
  char *string;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 1 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg, BV_STRING );
  string = b_strdup( bvariant_as_string( arg ) );
  bvariant_free( arg );

  /* convert to lowercase */
  b_strlower( string );

  /* set the result */
  result = bvariant_new();
  bvariant_set_string( result, string );
  b_free( string );

  return result;
}

/* convert string to uppercase  */
BVariant*
function_upper( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  BVariant *result;
  char *string;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 1 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg, BV_STRING );
  string = b_strdup( bvariant_as_string( arg ) );
  bvariant_free( arg );

  /* convert to uppercase */
  b_strupper( string );

  /* set the result */
  result = bvariant_new();
  bvariant_set_string( result, string );
  b_free( string );

  return result;
}

/* return a string of blank spaces 'n' times */
BVariant*
function_space( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  BVariant *result;
  int i, n;
  char *string;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 1 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg, BV_INTEGER );
  n = bvariant_as_integer( arg );
  bvariant_free( arg );

  string = b_malloc( n+1 );
  for( i=0; i<n; i++ ) string[i] = 32;
  string[n] = '\0';

  /* set the result */
  result = bvariant_new();
  bvariant_set_string( result, string );
  b_free( string );

  return result;
}

/* convert number to hexadecimal string  */
/* FIXME buffer overflow ? */
BVariant*
function_hex( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  BVariant *result;
  char str[256];
  int num;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 1 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg, BV_INTEGER );
  num = bvariant_as_integer( arg );
  bvariant_free( arg );

  sprintf( str, "%X", num );

  /* set the result */
  result = bvariant_new();
  bvariant_set_string( result, str );

  return result;
}

/* convert number to octal string  */
/* FIXME buffer overflow ? */
BVariant*
function_oct( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  BVariant *result;
  char str[256];
  int num;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 1 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg, BV_INTEGER );
  num = bvariant_as_integer( arg );
  bvariant_free( arg );

  sprintf( str, "%o", num );

  /* set the result */
  result = bvariant_new();
  bvariant_set_string( result, str );

  return result;
}

/* return current date, format is yyyy-mm-dd with fixed length of 10 chars */
BVariant*
function_date( BSContext *context, BList *args )
{
  struct tm *timep;
  time_t atime;
  char datestr[11];
  BVariant *result;

  /* get current date & time */
  time( &atime );
  timep = localtime( &atime );

  /* fill with current date */
  sprintf ( datestr, "%04i-%02i-%02i",
    timep->tm_year+1900, timep->tm_mday, timep->tm_mon+1 );

  /* set the result */
  result = bvariant_new();
  bvariant_set_string( result, datestr );

  return result;
}

/* return current time, format is hh:mm:ss with fixed length of 8 chars */
BVariant*
function_time( BSContext *context, BList *args )
{
  struct tm *timep;
  time_t atime;
  char timestr[9];
  BVariant *result;

  /* get current date & time */
  time( &atime );
  timep = localtime( &atime );

  /* fill with current time */
  sprintf ( timestr, "%02i:%02i:%02i",
      timep->tm_hour, timep->tm_min, timep->tm_sec );

  /* set the result */
  result = bvariant_new();
  bvariant_set_string( result, timestr );

  return result;
}

/* return seconds elapsed since midnight */
BVariant*
function_timer( BSContext *context, BList *args )
{
  struct tm *timep;
  time_t atime;
  double timer;
  BVariant *result;

  /* get current date & time */
  time( &atime );
  timep = localtime( &atime );

  timer = timep->tm_hour*60*60 + timep->tm_min*60 + timep->tm_sec;

  /* set the result */
  result = bvariant_new();
  bvariant_set_double( result, timer );

  return result;
}

/* get an environment variables */
BVariant*
function_environ( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  BVariant *result;
  char empty[] = { '\0' };
  char *name;
  char *value;

  /* check number of arguments */
  narg = blist_length( args );
  if( narg != 1 )
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* get the arguments */
  arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
  bvariant_cast( arg, BV_STRING );
  name = b_strdup( bvariant_as_string( arg ) );
  bvariant_free( arg );

  value = b_getenv( name );
  if( !value )
    value = b_strdup( empty );
  b_free( name );

  /* set the result */
  result = bvariant_new();
  bvariant_set_string( result, value );
  b_free( value );

  return result;
}

static int random_initialized = FALSE;

/* automagically randomize with timer */
void auto_randomize()
{
  struct tm *timep;
  time_t atime;
  time(&atime); timep=localtime(&atime);
  srandom(timep->tm_sec * timep->tm_min);
  random_initialized = TRUE;
}

/* initialize seed for random number generator */
BVariant*
function_randomize( BSContext *context, BList *args )
{
  int narg;
  BVariant *arg;
  int num = -1;

  /* check number of arguments */
  narg = blist_length( args );
  if(( narg != 1 ) && ( narg != 0 ))
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* if specified, get the arguments */
  if( narg == 1 )
  {
    arg = bvariant_copy( (BVariant*) blist_nth_data( args, 0 ) );
    bvariant_cast( arg, BV_INTEGER );
    num = bvariant_as_integer( arg );
    bvariant_free( arg );

    if( num<0 ) num=abs(num);
    srandom( num );
  }
  else
    auto_randomize();

  random_initialized = TRUE;

  /* no result */
  return NULL;
}

/* return random number in 0..1 */
BVariant*
function_rnd( BSContext *context, BList *args )
{
  int narg;
  BVariant *result;
  double num;

  /* check number of arguments */
  narg = blist_length( args );
  if(( narg != 1 ) && ( narg != 0 ))
  {
    /* wrong number of arguments */
    return NULL;
  }

  /* at the moment, we just discard the argument */

  /* in case seed is not initialized yet */
  if( !random_initialized ) auto_randomize();

  num = (double) rand() / RAND_MAX;

  /* set the result */
  result = bvariant_new();
  bvariant_set_double( result, num );

  return result;
}

