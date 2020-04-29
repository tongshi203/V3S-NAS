/*  This file is part of the BScript project
    Copyright (C) 1999-2004 Ariya Hidayat <ariya@kde.org>

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

#include <stdio.h>

#include "bscript.h"
#include "intern.h"
#include "blib.h"

int main (int argc, char *argv[])
{
  char* sourcefile;
  //BSInterpreter* interpreter;
  BSModule* module;
  int dump = FALSE;

  /* help on usage */
  if( argc <= 1 )
  {
    printf( "usage:\n" );
    printf("  %s [-dump] input-file\n\n", argv[0] );
    return 0;
  }

  if( b_stricmp( argv[1], "-dump" ) )
  {
    sourcefile = argv[2];
    dump = TRUE;
  }
  else
    sourcefile = argv[1];

  if( !sourcefile )
  {
    fprintf( stderr, "No source-file given\n" );
    return 0;
  }

  /*interpreter = bs_interpreter_create();
  bs_interpreter_run_file( interpreter, sourcefile );
  bs_interpreter_destroy( interpreter );*/

  module = bs_module_create( "test" );
  bs_module_load_file( module, sourcefile );

  if( !dump )
    bs_module_execute( module );
  else
    bs_module_dump( module );

  bs_module_destroy( module );

  return 1;
}
