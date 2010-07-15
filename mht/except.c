
/*********************************************************************
 * FILE: except.C                                                    *
 *                                                                   *
 * AUTHOR: Matthew Miller (mlm)                                      *
 *                                                                   *
 * HISTORY:                                                          *
 *   29 JAN 93 -- (mlm) created                                      *
 *   20 FEB 95 -- (mlm) changed naming of secret identifiers from    *
 *                "_name" to "Xname" for portability                 *
 *                                                                   *
 * CONTENTS:                                                         *
 *                                                                   *
 *   Routines for error-handling macros and debugging tools.  See    *
 *   except.H for details.                                           *
 *                                                                   *
 * ----------------------------------------------------------------- *
 *                                                                   *
 *             Copyright (c) 1993, NEC Research Institute            *
 *                       All Rights Reserved.                        *
 *                                                                   *
 *   Permission to use, copy, and modify this software and its       *
 *   documentation is hereby granted only under the following terms  *
 *   and conditions.  Both the above copyright notice and this       *
 *   permission notice must appear in all copies of the software,    *
 *   derivative works or modified versions, and any portions         *
 *   thereof, and both notices must appear in supporting             *
 *   documentation.                                                  *
 *                                                                   *
 *   Correspondence should be directed to NEC at:                    *
 *                                                                   *
 *                     Ingemar J. Cox                                *
 *                                                                   *
 *                     NEC Research Institute                        *
 *                     4 Independence Way                            *
 *                     Princeton                                     *
 *                     NJ 08540                                      *
 *                                                                   *
 *                     phone:  609 951 2722                          *
 *                     fax:  609 951 2482                            *
 *                     email:  ingemar@research.nj.nec.com (Inet)    *
 *                                                                   *
 *********************************************************************/

#define DECLARE_EXCEPT
/*
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
*/
#include "except.h"

/*-------------------------------------------------------------------*
 | Indent() -- indent cout
 *-------------------------------------------------------------------*/

void Indent( int numSpaces )
{
  int i;

  for( i = 0; i < numSpaces; i++ )
    std::cout << " ";
}

/*-------------------------------------------------------------------*
 | DumpCore() -- crash the program so that it does a core dump
 *-------------------------------------------------------------------*/
/*
void DumpCore()
{
  int dummy;
  int one = 1;
  int zero = 0;

  fprintf( stderr, "DUMPING CORE ...\n" );
  dummy = one / zero;
  _exit( -1 );          // just in case "one / zero" didn't cause a
                        // crash for some reason
}
*/


/*-------------------------------------------------------------------*
 | TraceScope() -- construct string describing the current scope
 *-------------------------------------------------------------------*/
/*
void TraceScope( char *buf, int bufSize )
{
  static char scope_overflow_msg[] = "** TOO MANY SCOPES **\n";
  SCOPE_TRACER *scope;

  // loop through scopes on stack
  *buf = 0;
  for( scope = SCOPE_TRACER::M_currentScope;
       scope;
       scope = scope->m_prevScope )
  {
    // make sure there's enough room left in buf for the description
    //   of this scope
    if( bufSize <= strlen( scope->m_file ) +      // room for filename
                   1 +                            //  ... " "
                   5 +                            //  ... line number
                   2 +                            //  ... ": "
                   strlen( scope->m_action ) +    //  ... action
                   1 +                            //  ... '\n'
                   strlen( scope_overflow_msg ) ) // plus message if
                           // there isn't enough room for the next one
    {
      sprintf( buf, "%s", scope_overflow_msg );
      break;
    }

    // make sure that the line number is not out of range (probably
    //   only happens if it's garbage)
    if( scope->m_line < 0 )
      scope->m_line = 0;
    if( scope->m_line > 99999 )
      scope->m_line = 99999;

    // use sprintf to build the descriptions, instead of a stream,
    //   since making a stream might compound with an error we've
    //   just detected (such as out of memory)
    if( *scope->m_action != 0 )
      sprintf( buf, "%s %d: %s\n",
                    scope->m_file,
                    scope->m_line,
                    scope->m_action );
    else
      sprintf( buf, "%s %d\n",
                    scope->m_file,
                    scope->m_line );

    // move pointer on to beginning of record for next scope
    while( *buf != 0 )
    {
      buf++;
      bufSize--;
    }
  }
}
*/
/*-------------------------------------------------------------------*
 | XDoErr() -- print out an error and exit
 *-------------------------------------------------------------------*/
/*
void XDoErr( int line, char *file )
{
  static char scopeStr[ 10000 ];

  TraceScope( scopeStr, sizeof( scopeStr ) );

  fprintf( stderr, "\n\n" );
  fprintf( stderr, "ERROR ( line %d of %s )\n", line, file );

  if( XG_msgStreamExists )
    fprintf( stderr, "  %s\n", XG_msg );
  else
    fprintf( stderr, "  THROW_ERR( %s )\n", XG_msg );

  if( *scopeStr )
  {
    fprintf( stderr, "SCOPE TRACE:\n" );
    fprintf( stderr, "%s", scopeStr );
  }

  fprintf( stderr, "ABORTING EXECUTION\n" );
  _exit( -1 );
}
*/
