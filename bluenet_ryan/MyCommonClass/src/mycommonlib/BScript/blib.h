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

#ifndef __BLIB_H_20050428__
#define __BLIB_H_20050428__

#ifdef __cplusplus
extern "C" {
#endif


#ifndef NULL
#define NULL ((void*)0)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

#ifndef TRUE
#define TRUE (!FALSE)
#endif

typedef void* bpointer;
typedef void (*BDestructor)( bpointer );

/* memory management */
bpointer b_malloc( int );
void b_free( bpointer );
#define b_new(type) ( (type*)b_malloc((unsigned)sizeof(type)) )
#define b_new_count(type,count) ( (type*)b_malloc((unsigned)(count)*sizeof(type)) )

/* string functions */
char* b_strdup( char* );
int b_stricmp( char*, char* );
char *b_strlower( char* );
char *b_strupper( char* );
char *b_str_append( char*, char* );
char *b_str_append_char( char*, char );

/* system related */
char* b_getenv( char* name );
double b_machine_epsilon();
int b_is_zero( double );
int b_is_negative( double );
int b_is_positive( double );

/* linked list */
typedef struct _BList BList;
struct _BList
{
  bpointer data;
  BList *prev;
  BList *next;
};

BList* blist_new();
void blist_free( BList* );
void blist_free_destruct( BList *list, BDestructor destructor );
BList* blist_append( BList*, bpointer );
BList* blist_prepend( BList*, bpointer );
BList* blist_remove( BList*, bpointer );
BList* blist_first( BList* );
BList* blist_last( BList* );
int blist_length( BList* );
BList* blist_nth( BList*, int );
bpointer blist_nth_data( BList*, int );

/* hash table */
typedef struct _BHash BHash;
typedef struct _BHashNode BHashNode;
struct _BHash
{
  int size;
  BHashNode **nodes;
};

struct _BHashNode
{
  char *key;
  bpointer value;
  BHashNode *next;
};

BHash *bhash_new();
void bhash_free( BHash *hash );
void bhash_free_destruct( BHash *hash, BDestructor destructor );
void bhash_insert( BHash *hash, char *key, bpointer value );
void bhash_remove( BHash *hash, char *key, BDestructor destructor);
bpointer bhash_lookup( BHash *hash, char *key );
int b_hash_string( char *string );

/* stack */
typedef struct _BStack BStack;
struct _BStack
{
  bpointer* data;
  int ptr;
};

BStack* bstack_new();
void bstack_free( BStack* );
void bstack_free_destruct( BStack *stack, BDestructor destructor );
void bstack_push( BStack*, bpointer );
bpointer bstack_pop( BStack* );
bpointer bstack_top( BStack* );


/* variant */
typedef struct _BVariant BVariant;
struct _BVariant
{
  int type;
  union
  {
    int b;
    int i;
    double d;
    char *string;
    bpointer *ptr;
  } val;
};

#define BV_INVALID 0
#define BV_BOOLEAN 1
#define BV_INTEGER 2
#define BV_DOUBLE  3
#define BV_STRING  4
#define BV_POINTER 5



BVariant* bvariant_new();
BVariant* bvariant_new_boolean( int );
BVariant* bvariant_new_integer( int );
BVariant* bvariant_new_double( double );
BVariant* bvariant_new_string( char* );
void bvariant_free( BVariant* );
int bvariant_can_cast( BVariant*, int );
BVariant* bvariant_cast( BVariant*, int );
BVariant* bvariant_copy( BVariant* );
BVariant* bvariant_set_boolean( BVariant*, int );
BVariant* bvariant_set_integer( BVariant*, int );
BVariant* bvariant_set_double( BVariant*, double );
BVariant* bvariant_set_pointer( BVariant*, bpointer );
BVariant* bvariant_set_string( BVariant*, char* );
int bvariant_as_boolean( BVariant* );
int bvariant_as_integer( BVariant* );
double bvariant_as_double( BVariant* );
char* bvariant_as_string( BVariant* );
bpointer bvariant_as_pointer( BVariant* );
const char* bvariant_describe_type( BVariant* );
const char* bvariant_dump( BVariant* );

#ifdef __cplusplus
}
#endif

#endif	// __BLIB_H_20050428__
