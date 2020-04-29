
#include "bscript.h"
#include "intern.h"
#include "blib.h"

#include <stdio.h>

/* please set function name in uppercase cause
   it will be used as key in hash table
   see also bs_eval_builtin_function in context.c */
static void
init_builtin_functions( BSModule *module )
{
  BHash *h;

  if( !module ) return;

  h = bhash_new();

  bhash_insert( h, "SGN",        function_sgn );
  bhash_insert( h, "ABS",        function_abs );
  bhash_insert( h, "EXP",        function_exp );
  bhash_insert( h, "LOG",        function_log );
  bhash_insert( h, "LOG10",      function_log10 );
  bhash_insert( h, "LOG2",       function_log2 );
  bhash_insert( h, "INT",        function_int );
  bhash_insert( h, "CINT",       function_cint );
  bhash_insert( h, "SQR",        function_sqr );
  bhash_insert( h, "PI",         function_pi );
  bhash_insert( h, "DEGREE",     function_degree );
  bhash_insert( h, "RADIAN",     function_radian );

  bhash_insert( h, "SIN",        function_sin );
  bhash_insert( h, "ASIN",       function_asin );
  bhash_insert( h, "COS",        function_cos );
  bhash_insert( h, "ACOS",       function_acos );
  bhash_insert( h, "TAN",        function_tan );
  bhash_insert( h, "ATN",        function_atn );
  bhash_insert( h, "SINH",       function_sinh );
  bhash_insert( h, "ASINH",      function_asinh );
  bhash_insert( h, "COSH",       function_cosh );
  bhash_insert( h, "ACOSH",      function_acosh );
  bhash_insert( h, "TANH",       function_tanh );
  bhash_insert( h, "ATNH",       function_atnh );

  bhash_insert( h, "LEN",        function_len );
  bhash_insert( h, "VAL",        function_val );
  bhash_insert( h, "ASC",        function_asc );
  bhash_insert( h, "LEFT$",      function_left );
  bhash_insert( h, "RIGHT$",     function_right );
  bhash_insert( h, "MID$",       function_mid );
  bhash_insert( h, "INSTR",      function_instr );
  bhash_insert( h, "STR$",       function_str );
  bhash_insert( h, "CHR$",       function_chr );
  bhash_insert( h, "STRING$",    function_string );
  bhash_insert( h, "LOWER$",     function_lower );
  bhash_insert( h, "UPPER$",     function_upper );
  bhash_insert( h, "DATE$",      function_date );
  bhash_insert( h, "TIME$",      function_time );
  bhash_insert( h, "RANDOMIZE",  function_randomize );
  bhash_insert( h, "RND",        function_rnd );

  /* these are typically only found in Microsoft BASICs */
  bhash_insert( h, "HEX$",       function_hex );
  bhash_insert( h, "OCT$",       function_oct );
  bhash_insert( h, "SPACE$",     function_space );
  bhash_insert( h, "SPC",        function_space );
  bhash_insert( h, "TIMER",      function_timer );
  bhash_insert( h, "ENVIRON$",   function_environ );

  /* can act as special variables */
  bhash_insert( h, "_MACHEPS",   function_macheps );

  module->builtin_functions = h;
}

BSModule*
bs_module_create( char *name )
{
  BSModule *module;

  module = b_new( BSModule );
  module->name = b_strdup( name );
  module->code = NULL;
  module->bytecodes = NULL;
  module->objects = NULL;
  module->builtin_functions = NULL;
  module->m_pfnPrint = NULL;			//  CYJ,2005-4-27 add
  module->m_pPrintContext = NULL;

  init_builtin_functions( module );

  return module;
}

void
bytecode_free( bpointer object )
{
  BSBytecode *code = (BSBytecode*) object;

  bvariant_free( code->value );
  b_free( code->label );
  b_free( code->symbol );
  b_free( code );
}

void
symbol_free( bpointer object )
{
  BSSymbol *symbol = (BSSymbol*) object;
  b_free( symbol->name );
  if( symbol->type == BV_STRING ) b_free( symbol->string );
  b_free( symbol );
}

void
bs_module_destroy( BSModule *module )
{
  if( module )
  {
    b_free( module->name );
    b_free( module->code );
    blist_free_destruct( module->bytecodes, bytecode_free );
    bhash_free_destruct( module->objects, symbol_free );
    bhash_free( module->builtin_functions );
    b_free( module );
  }
}

void
bs_module_set_code( BSModule *module, char* code )
{
  if( !module ) return;

  b_free( module->code );
  blist_free_destruct( module->bytecodes, bytecode_free );
  bhash_free_destruct( module->objects, symbol_free );

  /* set the code and compile it */
  module->code = b_strdup( code );
  module->bytecodes = bscript_parse( module->code ); /* in gram.y */
}

void
bs_module_load_file( BSModule *module, char *filename )
{
  FILE *f;
  int filesize;
  char *buffer;

  f = fopen( filename, "rb" );
  if( !f ) return;

  fseek( f, 0L, SEEK_END );
  filesize = ftell( f );
  fseek( f, 0L, SEEK_SET );

  buffer = (char*)b_malloc( filesize + 1 );
  fread( buffer, filesize, 1, f );
  buffer[filesize] = '\0';

  bs_module_set_code( module, buffer );

  b_free( buffer );
}

void
bs_module_execute ( BSModule *module )
{
  BSContext *context;

  if( !module ) return;
  if( !module->bytecodes ) return;

  context = bs_context_create( module );
  context->program_counter = 0;
  bs_context_execute( context );

  bs_context_destroy( context );
}

void
bs_module_dump ( BSModule *module )
{
  BSContext *context;

  if( !module ) return;
  if( !module->bytecodes ) return;

  context = bs_context_create( module );
  bs_context_dump( context );

  bs_context_destroy( context );
}

BSSymbol*
bs_module_access_symbol( BSModule *module, char *name )
{
  BSSymbol *symbol;

  if( !module ) return NULL;

  if( !module->objects )
    module->objects = bhash_new();

  symbol = bhash_lookup( module->objects, name );

  if( !symbol )
  {
    symbol = b_new( BSSymbol );
    symbol->name = b_strdup( name );
    symbol->type = BV_INVALID;
    symbol->number = 0.0;
    symbol->string = NULL;
    bhash_insert( module->objects, name, symbol );
  }

  return symbol;
}

BSInterpreter*
bs_interpreter_create()
{
  BSInterpreter *interpreter;

  interpreter = (BSInterpreter*) b_malloc( sizeof(BSInterpreter) );
  interpreter->name = (char*)0;
  return interpreter;
}

void
bs_interpreter_destroy( BSInterpreter *interpreter )
{
  b_free( interpreter );
}

void
bs_interpreter_run( BSInterpreter *interpreter, char *code )
{
  BSModule *module;
  module = bs_module_create( "bscript" );
  bs_module_set_code( module, code );
  bs_module_execute( module );
  bs_module_destroy( module );
}

void
bs_interpreter_run_file( BSInterpreter *interpreter, char *filename )
{
  BSModule *module;
  module = bs_module_create( "bscript" );
  bs_module_load_file( module, filename );
  bs_module_execute( module );
  bs_module_destroy( module );
}
