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

#include "blib.h"

#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _USE_MY_FLOAT_
// 使用我自己提供的 float 运算
extern double atof( const char *	string );
extern double strtod( const char * nptr, char ** endptr );
#endif // _USE_MY_FLOAT_

#define object_free(d,o) if((d)) (*(d))((o))

/* ====  MEMORY MANAGEMENT ==== */

static int b_malloc_count = 0;

void b_check_leak(void);

void b_check_leak(void)
{
  if( b_malloc_count > 0 )
  {
    fprintf( stderr, "found memory leak: %d allocation(s) not freed\n", b_malloc_count );
  }
}

bpointer
b_malloc( int size )
{
  static int initialized = 0;
  bpointer p;

  if( !initialized )
  {
    initialized= -1;
    atexit( b_check_leak );
  }

  b_malloc_count++;
  p = (bpointer)malloc( (size_t) size );
  return p;
}

void
b_free( bpointer data )
{
  if( !data ) return;
  free( data );
  b_malloc_count--;
}

/* ====  STRING FUNCTION ==== */

char*
b_strdup( char* str )
{
  char* s;

  s = NULL;
  if( str )
  {
    s = (char*)b_malloc( strlen(str)+1 );
    strcpy( s, str );
  }

  return s;
}

int
b_stricmp(char* p, char* q)
{
  long i;

  if(strlen(p) != strlen(q)) return 0;
  for(i=0; i<(long)strlen(p); i++)
    if(toupper(p[i]) != toupper(q[i])) return 0;
  return -1;
}

char*
b_strlower( char*string )
{
  char *p;

  if( !string ) return NULL;

  for( p=string; *p; p++ )
    *p = tolower(*p);
  return string;
}

char*
b_strupper( char*string )
{
  char *p;

  if( !string ) return NULL;

  for( p=string; *p; p++ )
    *p = toupper(*p);
  return string;
}

char*
b_str_append( char *string, char *p )
{
  char *result;

  result = b_malloc( strlen(string)+strlen(p)+1 );
  strcpy( result, string );
  strcat( result, p );
  return result;
}

char*
b_str_append_char( char *string, char ch )
{
  char *result;
  int len;

  len = strlen( string );
  result = b_malloc( len + 2 );
  strcpy( result, string );
  result[len] = ch;
  result[len+1] = '\0';
  return result;
}

/* ====  SYSTEM RELATED ==== */
char*
b_getenv( char *name )
{
  char *env;

  env = getenv( name );
  if( !env ) return NULL;

  return b_strdup( env );
}

/* prevent any compiler optimization */
double dummy( double num )
{
  static double dummy;
  return dummy=num;
}

static double machine_epsilon = -1;

double
b_machine_epsilon()
{
  if( machine_epsilon < 0 )
  {
    for( machine_epsilon=1.0;
      dummy(machine_epsilon+1.0)>1.0;
      machine_epsilon/=2.0);
    machine_epsilon*=2.0;
  }
  return machine_epsilon;
}

int
b_is_zero( double num )
{
  return fabs(num) < b_machine_epsilon();
}

int
b_is_positive( double num )
{
  return num > b_machine_epsilon();
}

int
b_is_negative( double num )
{
  return num < -b_machine_epsilon();
}


/* ====  LINKED LIST ==== */

BList*
blist_new()
{
  BList *list;

  list = b_new(BList);
  list->data = NULL;
  list->prev = NULL;
  list->next = NULL;

  return list;
}

void
blist_free( BList *list )
{
  if( list )
  {
    blist_free( list->next );
    b_free(list);
  }
}

void
blist_free_destruct( BList *list, BDestructor destructor )
{
  BList *node;
  for( node = list; node; node=node->next )
    object_free( destructor, node->data );
  blist_free( list );
}

BList*
blist_append( BList *list, bpointer data )
{
  BList *new_list;
  BList *last;

  new_list = blist_new();
  new_list->data = data;

  if( list )
  {
    last = blist_last( list );
    last->next = new_list;
    new_list->prev = last;
    return list;
  }
  else
    return new_list;
}

BList*
blist_prepend( BList *list, bpointer data )
{
  BList *new_list;

  new_list = blist_new();
  new_list->data = data;

  if( list )
  {
    if( list->prev )
    {
      list->prev->next = new_list;
      new_list->prev = list->prev;
    }
    list->prev = new_list;
    new_list->next = list;
  }

  return new_list;
}

BList*
blist_remove( BList *list, bpointer data )
{
  BList *tmp;

  tmp = list;
  while( tmp )
  {
    if( tmp->data != data )
      tmp = tmp->next;
    else
    {
      if( tmp->prev ) tmp->prev->next = tmp->next;
      if( tmp->next ) tmp->next->prev = tmp->prev;
      if( list == tmp )
        list = list->next;
      b_free( tmp );
      break;
    }
  }

  return list;
}

BList*
blist_first( BList *list )
{
  if( list )
    while( list->prev )
      list = list->prev;

  return list;
}

BList*
blist_last( BList *list )
{
  if( list )
    while( list->next )
      list = list->next;

  return list;
}

int
blist_length( BList *list )
{
  int length = 0;

  while( list )
  {
    length++;
    list = list->next;
  }

  return length;
}

BList*
blist_nth( BList *list, int n )
{
  while( (n-- > 0) && list )
    list = list->next;
  return list;
}

bpointer
blist_nth_data( BList *list, int n )
{
  while( (n-- > 0) && list )
    list = list->next;
  return list ? list->data : NULL;
}

/* ====  HASH TABLE ==== */

int
b_hash_string( char *string )
{
  int h;
  char *p;

  p = string;
  for( h=0; *p; p++ )
     h = (h<<5)-h+*p;
  return h;
}

BHashNode*
bhash_node_new( char *key, bpointer value )
{
  BHashNode *node;

  node = b_new(BHashNode);
  node->key = b_strdup( key );
  node->value = value;
  node->next = NULL;

  return node;
}

void
bhash_node_free( BHashNode *node )
{
  if( node )
  {
    b_free( node->key );
    b_free( node );
  }
}

void
bhash_node_free_destruct( BHashNode *node, BDestructor destructor )
{
  if( node )
  {	  
	  if( node->key )			//	防止重复释放
		  *node->key = 0;
	  object_free( destructor, node->value );    
	  b_free( node->key );
	  b_free( node );
  }
}

BHash*
bhash_new()
{
  BHash *hash;
  int i;

  hash = b_new(BHash);
  hash->size = 23;  /* must be prime number */
  hash->nodes = b_new_count(BHashNode*,hash->size);

  for( i=0; i<hash->size; i++ )
    hash->nodes[i] = NULL;

  return hash;
}

void
bhash_free( BHash *hash )
{
  if( hash )
  {
    int i;
    BHashNode *node;

    for( i=0; i<hash->size; i++ )
      for( node=hash->nodes[i]; node; )
      {
        BHashNode *next = node->next;
        bhash_node_free( node );
	node = next;
      }

    b_free( hash->nodes );
    b_free( hash );
  }
}

void
bhash_free_destruct( BHash *hash, BDestructor destructor )
{
  if( hash )
  {
    int i;
    BHashNode *node;

    for( i=0; i<hash->size; i++ )
      for( node=hash->nodes[i]; node; )
      {
		  BHashNode *next = node->next;
		  bhash_node_free_destruct( node, destructor );
		  node = next;
      }

    b_free( hash->nodes );
    b_free( hash );
  }
}


BHashNode**
bhash_lookup_pnode( BHash *hash, char *key )
{
  BHashNode **pnode;
  int val;

  val = b_hash_string( key ) % hash->size;
  while( val < 0 ) val += hash->size;
  pnode = hash->nodes + val;

  while( *pnode )
  {
    if( !strcmp((*pnode)->key,key) )  break;
    pnode = &(*pnode)->next;
  }

  return pnode;
}

void
bhash_insert( BHash *hash, char *key, bpointer value )
{
  BHashNode **pnode;

  pnode = bhash_lookup_pnode( hash, key );
  if( *pnode )
    (*pnode)->value = value;
  else
    *pnode = bhash_node_new( key, value );
}

///-------------------------------------------------------
/// CYJ,2005-4-28
/// 函数功能:
///		remove one hash item
/// 输入参数:
///		无
/// 返回参数:
///		无
void bhash_remove( BHash *hash, char *key, BDestructor destructor)
{
	BHashNode *pnode;
	BHashNode *pLastNode;
	int val;
	
	val = b_hash_string( key ) % hash->size;
	while( val < 0 ) val += hash->size;
	pnode = hash->nodes[val];
	pLastNode = NULL;
	
	while( pnode )
	{
		if(!strcmp( pnode->key,key ) )
		{						// find
			if( pLastNode )
				pLastNode->next = pnode->next;
			else
				hash->nodes[val] = pnode->next;
			bhash_node_free_destruct( pnode, destructor );
			return;
		}
		pLastNode = pnode;
		pnode = pnode->next;
	}	
}

bpointer
bhash_lookup( BHash *hash, char *key )
{
  BHashNode *node;
  node = *bhash_lookup_pnode( hash, key );
  return node ? node->value : NULL;
}

/* ====  STACK ==== */

BStack*
bstack_new()
{
  BStack *stack;

  stack = b_new(BStack);
  stack->data = (bpointer*)b_malloc( sizeof(bpointer)*1024 );
  stack->ptr = 0;

  return stack;
}

void
bstack_free( BStack *stack )
{
  if( stack )
  {
    b_free( stack->data );
    b_free( stack );
  }
}

void
bstack_free_destruct( BStack *stack, BDestructor destructor )
{
  if( stack )
  {
    while( bstack_top( stack ) )
      object_free( destructor, bstack_pop( stack ) );
    b_free( stack->data );
    b_free( stack );
  }
}

void
bstack_push( BStack *stack, bpointer data )
{
  stack->data[stack->ptr] = data;
  stack->ptr++;
}

bpointer
bstack_pop( BStack *stack )
{
  if( stack->ptr == 0 ) return NULL;
  return stack->data[ --stack->ptr ];
}

bpointer
bstack_top( BStack *stack )
{
  if( stack->ptr == 0 ) return NULL;
  return stack->data[ stack->ptr-1 ];
}

/* ==== VARIANT ==== */

BVariant*
bvariant_new()
{
  BVariant *var;

  var = b_new(BVariant);
  var->type = BV_INVALID;
  return var;
}

BVariant*
bvariant_new_boolean( int b )
{
  BVariant *var;
  var = b_new(BVariant);
  var->type = BV_BOOLEAN;
  var->val.b = b;
  return var;
}

BVariant*
bvariant_new_integer( int i )
{
  BVariant *var;
  var = b_new(BVariant);
  var->type = BV_INTEGER;
  var->val.i = i;
  return var;
}

BVariant*
bvariant_new_double( double d )
{
  BVariant *var;
  var = b_new(BVariant);
  var->type = BV_DOUBLE;
  var->val.d = d;
  return var;
}

BVariant*
bvariant_new_string( char *s )
{
  BVariant *var;

  var = bvariant_new();
  var->type = BV_STRING;
  var->val.string = b_strdup( s );

  return var;
}

void
bvariant_free( BVariant *var )
{
  if( !var ) return;

  if( var->type == BV_STRING ) b_free( var->val.string );

  b_free( var );
}

void
bvariant_nullify( BVariant *var )
{
  if( !var ) return;
  
  if( var->type == BV_STRING ) 
    b_free( var->val.string );
  var->type = BV_INVALID;
}

BVariant*
bvariant_copy( BVariant *var )
{
  BVariant *new_var;

  switch( var->type )
  {
    case BV_BOOLEAN:
      new_var = bvariant_new_boolean( var->val.b ); break;
    case BV_INTEGER:
      new_var = bvariant_new_integer( var->val.i ); break;
    case BV_DOUBLE:
      new_var = bvariant_new_double( var->val.d ); break;
    case BV_STRING:
      new_var = bvariant_new_string( var->val.string ); break;
    case BV_POINTER:
		new_var = var; break;
//      new_var->val.ptr = var->val.ptr; break;
    default:
      new_var = bvariant_new(); break;
  }

  return new_var;
}

BVariant*
bvariant_set_boolean( BVariant *var, int b )
{
  if( !var ) return var;
  bvariant_nullify( var );
  var->type =  BV_BOOLEAN;
  var->val.b = b;
  return var;
}

BVariant*
bvariant_set_integer( BVariant *var, int i )
{
  if( !var ) return var;
  bvariant_nullify( var );
  var->type =  BV_INTEGER;
  var->val.i = i;
  return var;
}

BVariant*
bvariant_set_double( BVariant *var, double d )
{
  if( !var ) return var;
  bvariant_nullify( var );
  var->type =  BV_DOUBLE;
  var->val.d = d;
  return var;
}

BVariant*
bvariant_set_string( BVariant *var, char* string )
{
  if( !var ) return var;
  bvariant_nullify( var );
  var->type =  BV_STRING;
  var->val.string = b_strdup( string );
  return var;
}

BVariant*
bvariant_set_pointer( BVariant *var, bpointer ptr )
{
  if( !var ) return var;
  bvariant_nullify( var );
  var->type =  BV_POINTER;
  var->val.ptr = ptr;
  return var;
}

BVariant*
bvariant_cast( BVariant *var, int new_type )
{
  if( !var ) return var;

  /* cast from boolean to... */
  if( var->type == BV_BOOLEAN )
  {

    if( new_type == BV_BOOLEAN )
      return var;

    if( new_type == BV_INTEGER )
    {
      int i = var->val.b ? -1 : 0;
      var->type = BV_INTEGER;
      var->val.i = i;
      return var;
    }

    if( new_type == BV_DOUBLE )
    {
      double d = var->val.b ? -1 : 0;
      var->type = BV_DOUBLE;
      var->val.d = d;
      return var;
    }

    if( new_type == BV_STRING )
    {
      char* string = var->val.b ? "TRUE" : "FALSE";
      var->type = BV_STRING;
      var->val.string = b_strdup(string);
      return var;
    }

    if( new_type == BV_POINTER )
    {
      var->type = BV_POINTER;
      var->val.ptr = NULL;
      return var;
    }

    return var;
  }

  /* cast from integer to .... */
  if( var->type == BV_INTEGER )
  {

    if( new_type == BV_BOOLEAN )
    {
      int b = (var->val.i!=0) ? -1 : 0;
      var->type = BV_BOOLEAN;
      var->val.b = b;
      return var;
    }

    if( new_type == BV_INTEGER )
      return var;

    if( new_type == BV_DOUBLE )
    {
      double d = (double) var->val.i;
      var->type = BV_DOUBLE;
      var->val.d = d;
      return var;
    }

    if( new_type == BV_STRING )
    {
      // FIXME !!!
      char buf[100];
      sprintf( buf, "%d", var->val.i );
      var->type = BV_STRING;
      var->val.string = b_strdup( buf );
      return var;
    }

    if( new_type == BV_POINTER )
    {
      var->type = BV_POINTER;
      var->val.ptr = NULL;
      return var;
    }

    return var;

  }

  /* cast from double to .... */
  if( var->type == BV_DOUBLE )
  {

    if( new_type == BV_BOOLEAN )
    {
      // FIXME use machine epsilon here !
      int b = (var->val.d!=0) ? -1 : 0;
      var->type = BV_BOOLEAN;
      var->val.b = b;
      return var;
    }

    if( new_type == BV_INTEGER )
    {
      int i = (int) var->val.d;
      var->type = BV_INTEGER;
      var->val.i = i;
      return var;
    }

    if( new_type == BV_DOUBLE )
      return var;

    if( new_type == BV_STRING )
    {
      // FIXME !!!
      char buf[100];
      sprintf( buf, "%g", var->val.d );
      var->type = BV_STRING;
      var->val.string = b_strdup( buf );
      return var;
    }

    if( new_type == BV_POINTER )
    {
      var->type = BV_POINTER;
      var->val.ptr = NULL;
      return var;
    }

    return var;

  }

  /* cast from string to .... */
  if( var->type == BV_STRING )
  {

    if( new_type == BV_BOOLEAN )
    {
      b_free( var->val.string );
      var->type = BV_BOOLEAN;
      var->val.b = FALSE;
      return var;
    }

    if( new_type == BV_INTEGER )
    {
      char *string = var->val.string;
      b_free( var->val.string );
      var->type = BV_INTEGER;
      var->val.i = atol( string );
      return var;
    }

    if( new_type == BV_DOUBLE )
    {
      char *string = var->val.string;
      b_free( var->val.string );
      var->type = BV_DOUBLE;
      var->val.d = atof( string );
      return var;
    }

    if( new_type == BV_STRING )
      return var;

    if( new_type == BV_POINTER )
    {
      char *string = var->val.string;
      var->type = BV_POINTER;
      var->val.ptr = (bpointer)string;
      return var;
    }

    return var;
  }

  /* cast from pointer to .... */
  if( var->type == BV_POINTER )
  {

    if( new_type == BV_BOOLEAN )
    {
      var->type = BV_BOOLEAN;
      var->val.b = FALSE;
      return var;
    }

    if( new_type == BV_INTEGER )
    {
      var->type = BV_INTEGER;
      var->val.i = 0;
      return var;
    }

    if( new_type == BV_DOUBLE )
    {
      var->type = BV_DOUBLE;
      var->val.d = 0;
      return var;
    }

    if( new_type == BV_STRING )
    {
      var->type = BV_STRING;
      var->val.string = NULL;
      return var;
    }

    if( new_type == BV_POINTER )
      return var;

    return var;
  }

  return var;
}

int
bvariant_can_cast( BVariant *var, int new_type )
{
  if( !var ) return FALSE;

  if( var->type == BV_BOOLEAN )
  {
    switch(new_type)
    {
      case BV_BOOLEAN: return TRUE;
      case BV_INTEGER: return TRUE;
      case BV_DOUBLE : return TRUE;
      case BV_STRING : return TRUE;
      case BV_POINTER: return FALSE;
      default: return FALSE;
    }
  }

  if( var->type == BV_INTEGER )
  {
    switch(new_type)
    {
      case BV_BOOLEAN: return TRUE;
      case BV_INTEGER: return TRUE;
      case BV_DOUBLE : return TRUE;
      case BV_STRING : return TRUE;
      case BV_POINTER: return FALSE;
      default: return FALSE;
    }
  }

  if( var->type == BV_DOUBLE )
  {
    switch(new_type)
    {
      case BV_BOOLEAN: return TRUE;
      case BV_INTEGER: return TRUE;
      case BV_DOUBLE : return TRUE;
      case BV_STRING : return TRUE;
      case BV_POINTER: return FALSE;
      default: return FALSE;
    }
  }

  if( var->type == BV_STRING )
  {
    switch(new_type)
    {
      case BV_BOOLEAN: return FALSE;
      case BV_INTEGER: return FALSE;
      case BV_DOUBLE : return FALSE;
      case BV_STRING : return TRUE;
      case BV_POINTER: return TRUE;
      default: return FALSE;
    }
  }

  if( var->type == BV_POINTER )
  {
    switch(new_type)
    {
      case BV_BOOLEAN: return FALSE;
      case BV_INTEGER: return FALSE;
      case BV_DOUBLE : return FALSE;
      case BV_STRING : return TRUE;
      case BV_POINTER: return TRUE;
      default: return FALSE;
    }
  }


  return FALSE;
}

int
bvariant_as_boolean( BVariant *var )
{
  int b = FALSE;

  if( var ) if( var->type == BV_BOOLEAN ) b = var->val.b;

  return b;
}

int
bvariant_as_integer( BVariant *var )
{
  int i = 0;

  if( var ) if( var->type == BV_INTEGER ) i = var->val.i;

  return i;
}

double
bvariant_as_double( BVariant *var )
{
  double d = 0.0;

  if( var ) if( var->type == BV_DOUBLE ) d = var->val.d;

  return d;
}

char*
bvariant_as_string( BVariant *var )
{
  char *string = NULL;

  if( var ) if( var->type == BV_STRING ) string = var->val.string;

  return string;
}

bpointer
bvariant_as_pointer( BVariant *var )
{
  bpointer ptr = NULL;

  if( var ) if( var->type == BV_POINTER ) ptr = var->val.ptr;

  return ptr;
}

const char*
bvariant_describe_type( BVariant *var )
{
  if( !var ) return "Unknown";

  switch( var->type )
  {
    case BV_BOOLEAN:
      return "Boolean";
    case BV_INTEGER:
      return "Integer";
    case BV_DOUBLE:
      return "Double";
    case BV_STRING:
      return "String";
    case BV_POINTER:
      return "Pointer";
    default:
      return "Invalid";
  }

  return NULL;
}

/* only take first 32 chars, escape the characters */
char*
proper_stringfy( char* string )
{
  static char result[97];
  int i;
  char ch;

  result[0] = '\0';
  for( i=0; i < 32; i++ )
  {
    ch = string[i];
    if( !ch ) break;
    if( ch >= 32 )
      sprintf( result, "%s%c", result, ch );
    else if ( ch == '\n' )
      sprintf( result, "%s\\n", result );
    else
      sprintf( result, "%s\\%d", result, ch );
  }
  return result;
}

const char*
bvariant_dump( BVariant *var )
{
  static char result[100];

  if( !var )
  {
    strcpy( result, "(null)" );
    return result;
  }

  switch( var->type )
  {
    case BV_BOOLEAN:
      sprintf( result, "Boolean: %s",
        bvariant_as_boolean(var) ? "True" : "False" );
      break;
    case BV_INTEGER:
      sprintf( result, "Integer: %d", bvariant_as_integer(var) );
      break;
    case BV_DOUBLE:
      sprintf( result, "Double: %g", bvariant_as_double(var) );
      break;
    case BV_STRING:
      sprintf( result, "String: [%s]", proper_stringfy( bvariant_as_string( var ) ) );
      //sprintf( result, "String: %s", bvariant_as_string( var ) );
      break;
    case BV_POINTER:
      sprintf( result, "Pointer: %p", bvariant_as_pointer(var) );
      break;
    default:
      sprintf( result, "Unknown (type %d)", var->type );
      break;
  }

  return result;
}