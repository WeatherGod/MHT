
/* The following #defines are at the beginning of this file because
   I often want to change them (by commenting them out in some
   combination)

   They are in this file instead of on the CC command line (e.g.
   "CC -DTSTBUG ...") because changing them here forces recompilation
   of everything they effect, even CC's secret files. */

#ifndef EXCEPT_H

//#define TSTBUG
//#define DEBUG

#endif

/*********************************************************************
 * FILE: except.H                                                    *
 *                                                                   *
 * AUTHOR: Matthew Miller (mlm)                                      *
 *                                                                   *
 * HISTORY:                                                          *
 *   29 JAN 93 -- (mlm) created                                      *
 *    9 MAR 93 -- (mlm) changed to use SAFE_GLOBAL() macro instead   *
 *                of GLOBAL_PTR() macro                              *
 *   28 MAY 93 -- (mlm) removed all attempts to make compatible with *
 *                standard C++ exception handling,                   *
 *                removed TRY and CATCH_ERR,                         *
 *                added #define structure for TSTBUG and DEBUG,      *
 *                changed BEGIN_SCOPE to BGN,                        *
 *                completely rewrote comments                        *
 *   20 FEB 95 -- (mlm) changed naming of secret identifiers from    *
 *                "_name" to "Xname" for portability                 *
 *   21 FEB 95 -- (mlm) added CANT_RECUR macro, changed              *
 *                implementation of m_actionStream in SCOPE_TRACER   *
 *                so that it isn't constructed until it's needed     *
 *                (its construction was causing infinite             *
 *                recursion in new())                                *
 *                                                                   *
 * CONTENTS:                                                         *
 *                                                                   *
 *   Macros and routines useful for tracking down errors.            *
 *                                                                   *
 *                       CANT_RECUR                                  *
 *                                                                   *
 *   When this macro appears at the beginning of a routine, and      *
 *   TSTBUG is defined, it checks to make sure that the routine is   *
 *   not being called recursively.  If the routine IS called         *
 *   recursively, CANT_RECUR will spit a "FATAL BUG" message onto    *
 *   stderr and _exit the program.                                   *
 *                                                                   *
 *                     TSTBUG and DEBUG                              *
 *                                                                   *
 *   There are two manifest defines used to control the amount of    *
 *   self-checking that other modules do.  If neither of them is     *
 *   #defined, hardly any checking should be done at all.            *
 *                                                                   *
 *   TSTBUG --                                                       *
 *     When this is defined, checks are performed which don't reduce *
 *     speed to the point of making the program unusable.  In        *
 *     general, it should be defined whenever new code is being      *
 *     tested.                                                       *
 *                                                                   *
 *   DEBUG --                                                        *
 *     When this is defined, maximal checking is performed.  Every   *
 *     piece of memory that is allocated is checked for corruption,  *
 *     every data structure that is modified is checked for internal *
 *     consistancy, etc.  The program will grind to a near stand-    *
 *     still, but catastrophic bugs will be much easier to find.     *
 *                                                                   *
 *     When DEBUG is defined, TSTBUG is automatically defined as     *
 *     well.                                                         *
 *                                                                   *
 *                          THROW_ERR()                              *
 *                                                                   *
 *   This spits an error message out to stderr, and exits.           *
 *   If BGN has been used (see below), and TSTBUG is defined, then   *
 *   a trace of scopes in which the error ocurred will be spat out   *
 *   as well.                                                        *
 *                                                                   *
 *   THROW_ERR() takes one argument, which is used to construct a    *
 *   text description of the error.  The argument is placed after a  *
 *   stream object and <<.  This means that it can contain a         *
 *   sequence of printable objects, separated by <<'s.  For example: *
 *                                                                   *
 *     THROW_ERR( "Cannot open file '" << fileName << "'" )          *
 *                                                                   *
 *   If fileName is a pointer to the string "bummer.txt", then the   *
 *   string this generates is                                        *
 *                                                                   *
 *     Cannot open file 'bummer.txt'                                 *
 *                                                                   *
 *                   BGN, TRACE, and ACTION()                        *
 *                                                                   *
 *   These three macros provide mechanisms for tracing the activity  *
 *   of the program.  The traced activity is printed out by          *
 *   THROW_ERR() when an error occurs, giving detailed information   *
 *   about where it occured.                                         *
 *                                                                   *
 *   When TSTBUG is not defined, these macros do nothing (and add no *
 *   code).                                                          *
 *                                                                   *
 *   BGN --                                                          *
 *     This should be placed at the beginning of each routine.  It   *
 *     makes a record of the name of the file and the line number    *
 *     that it appears on.  This record is stored on the stack, and  *
 *     contains a pointer to the previous such record on the stack,  *
 *     forming a linked list of active routines.                     *
 *                                                                   *
 *     The record formed by BGN also contains space for a string     *
 *     that can be filled in by the ACTION() macro (see below).      *
 *                                                                   *
 *     When BGN goes out of scope (i.e. the routine returns), the    *
 *     record it created is removed from the stack, and from the     *
 *     linked list.                                                  *
 *                                                                   *
 *   TRACE --                                                        *
 *     This may only be used in routines that contain BGN.  It       *
 *     changes the line number in BGN's record to the line number on *
 *     which it appears.  This is useful for finding the exact line  *
 *     on which an error occured.                                    *
 *                                                                   *
 *   ACTION() --                                                     *
 *     This is just like TRACE, except that it constructs a string   *
 *     in BGN's record, which is added to the output of THROW_ERR(). *
 *                                                                   *
 *     The string is constructed in the same way as the error string *
 *     in THROW_ERR(), so, for example                               *
 *                                                                   *
 *       ACTION( "Opening file '" << fileName << "'" )               *
 *                                                                   *
 *     constructs the string                                         *
 *                                                                   *
 *       Opening file 'bummer.txt'                                   *
 *                                                                   *
 *     (assuming that fileName still points to "bummer.txt").        *
 *                                                                   *
 *   Here's an example of how BGN, TRACE, ACTION(), and THROW_ERR()  *
 *   are used:                                                       *
 *                                                                   *
 *   Suppose we have two files, main.C and foo.C.                    *
 *   main.C contains:                                                *
 *                                                                   *
 *     #include "except.h"                                           *
 *                                                                   *
 *     void foo( void );                                             *
 *                                                                   *
 *     void main()                                                   *
 *     {                                                             *
 *       BGN                // line 7                                *
 *                                                                   *
 *       TRACE              // line 9                                *
 *                                                                   *
 *       {                                                           *
 *         BGN              // line 12                               *
 *                                                                   *
 *         ACTION( "Testing foo()" ); // line 14                     *
 *         foo();                                                    *
 *       }                                                           *
 *     }                                                             *
 *                                                                   *
 *   and foo.C contains:                                             *
 *                                                                   *
 *     #include "except.h"                                           *
 *                                                                   *
 *     void foo( void )                                              *
 *     {                                                             *
 *       BGN                // line 5                                *
 *                                                                   *
 *       THROW_ERR( "Error number " << 1 ) // line 7                 *
 *     }                                                             *
 *                                                                   *
 *   And suppose we compile this with TSTBUG or DEBUG defined.       *
 *   When we run this program, it will output:                       *
 *                                                                   *
 *     ERROR ( line 7 of foo.C):                                     *
 *       Error number 1                                              *
 *     SCOPE TRACE:                                                  *
 *     foo.C 5                                                       *
 *     main.C 14: Testing foo()                                      *
 *     main.C 9                                                      *
 *     ABORTING EXECUTION                                            *
 *                                                                   *
 *                         Indent()                                  *
 *                                                                   *
 *   This is just a simple routine that prints a number of spaces to *
 *   cout.  It takes one argument: the number of spaces to print.    *
 *                                                                   *
 *   I find this useful in printing diagnostics.                     *
 *                                                                   *
 *                         DumpCore()                                *
 *                                                                   *
 *   This routine causes the program to crash and dump core.  It's   *
 *   useful when the program has detected a particularly             *
 *   catastrophic problem.                                           *
 *                                                                   *
 * NOTES:                                                            *
 *                                                                   *
 *   In much of except.H and except.C, I use fprintf() where it      *
 *   would be more C++'ish to use cerr (from iostream.h).  I do this *
 *   because the iostream stuff likes to allocate memory when you    *
 *   use it, and that can cause serious problems if the error you're *
 *   reporting has to do with problems allocating memory.  The       *
 *   stdio stuff is more robust.                                     *
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

#ifndef EXCEPT_H
#define EXCEPT_H

#include <unistd.h>
#include <string>
#include <stdio.h>
#include <iostream>
#include "strstream"
#include <stdlib.h>

//#include "portspecs.h"
#include "safeglobal.h"

#ifdef DECLARE_EXCEPT
  #define GLOBAL
#else
  #define GLOBAL extern
#endif

using std::cout;
using std::endl;
using std::strstream; 
using std::ostrstream;
/*-------------------------------------------------------------------*
 | Automatic declaration of TSTBUG
 *-------------------------------------------------------------------*/

#if defined( DEBUG ) && ! defined( TSTBUG )
  #define TSTBUG
#endif

/*-------------------------------------------------------------------*
 | Declarations of stuff found in this file.
 *-------------------------------------------------------------------*/

class SCOPE_TRACER;
class ERR_STREAM;

/*-------------------------------------------------------------------*
 | RECURSION_CHECKER -- class used in CANT_RECUR macro
 *-------------------------------------------------------------------*/

class RECURSION_CHECKER
{
  private:

    int &m_busyFlag;

  public:

    RECURSION_CHECKER( int &busyFlag, int line, const char *file ):
      m_busyFlag( busyFlag )
    {
      if( m_busyFlag )
      {
        fprintf( stderr,
                 "FATAL BUG: recursive call on line %d of '%s'\n",
                 line, file );
        _exit( -1 );
      }

      m_busyFlag = 1;
    }

    ~RECURSION_CHECKER()
    {
      m_busyFlag = 0;
    }
};

/*-------------------------------------------------------------------*
 | CANT_RECUR macro -- make sure that a routine is not called
 |                     recursively
 *-------------------------------------------------------------------*/

#ifdef TSTBUG

  #define CANT_RECUR                                                  \
    static int XbusyFlag;                                             \
    RECURSION_CHECKER Xchecker( XbusyFlag, __LINE__, __FILE__ );      \

#else

  #define CANT_RECUR

#endif

/*-------------------------------------------------------------------*
 | SCOPE_TRACER -- object used in tracing scopes
 |
 | This class should only be used explicitly by the BGN,
 | TRACE, and ACTION macros.
 *-------------------------------------------------------------------*/

class SCOPE_TRACER
{
  private:

    static SCOPE_TRACER *M_currentScope; // pointer to last
                                     //   SCOPE_TRACER object declared
                                     //   (last use of BGN)

    SCOPE_TRACER *m_prevScope;       // SCOPE_TRACER declared before
                                     //   this one
    int m_line;                      // line number
    const char *m_file;              // file name
    char m_action[ 100 ];            // ACTION string
    ostrstream *m_actionStream;      // stream for accessing ACTION
                                     //   string

  public:

    SCOPE_TRACER( int line, const char *file ): // CONSTRUCTOR
      m_prevScope( M_currentScope ),
      m_line( line ),
      m_file( file ),
      m_actionStream( 0 )
    {
      *m_action = 0;
      M_currentScope = this;
    }

    ~SCOPE_TRACER()                  // DESTRUCTOR
    {
      if( m_actionStream )
        delete m_actionStream;

      M_currentScope = m_prevScope;
    }

    void setLine( int line )         // change line number
    {
      m_line = line;
    }

    ostrstream &getActionStream()    // get stream for action string
    {
      if( m_actionStream == 0 )
        m_actionStream =
          new ostrstream( m_action, sizeof( m_action ) );

      memset( m_action, 0, sizeof( m_action ) );
      m_actionStream->seekp( 0 );
      return *m_actionStream;
    }

    friend void TraceScope( char *buf, int bufSize ); // construct
                                     // scope trace string
};

#ifdef DECLARE_EXCEPT
  SCOPE_TRACER *SCOPE_TRACER::M_currentScope = 0;
#endif

/*-------------------------------------------------------------------*
 | BGN, ACTION(), and TRACE -- macros for constructing trace of where
 |                             errors occur
 *-------------------------------------------------------------------*/

#ifdef TSTBUG

  #define BGN                                                         \
    SCOPE_TRACER Xscope( __LINE__, __FILE__ );                        \

  #define ACTION( description )                                       \
    {                                                                 \
      Xscope.setLine( __LINE__ );                                     \
      Xscope.getActionStream() << description;                        \
    }                                                                 \

  #define TRACE                                                       \
    Xscope.setLine( __LINE__ );                                       \

#else

  #define BGN  
  #define ACTION( description )
  #define TRACE

#endif

/*-------------------------------------------------------------------*
 | Global variables used in THROW_ERR()
 *-------------------------------------------------------------------*/

GLOBAL char G_errHasHappened;        // set to 1 by THROW_ERR()
GLOBAL char XG_msg[ 1000 ];          // description of error
GLOBAL char XG_msgStreamExists;      // flag set when XG_errStream is
                                     //   initialized

/*-------------------------------------------------------------------*
 | ERR_STREAM -- contains an ostrstream for building XG_msg
 |
 | One object of this class is created, using SAFE_GLOBAL.  It sets
 | up an ostrstream object for constructing XG_msg, and sets a flag
 | to indicate that the ostrstream is ready.  This is necessary
 | because errors might occur during initialization before the
 | ostrstream is constructed.
 *-------------------------------------------------------------------*/

class ERR_STREAM
{
  private:

    ostrstream m_msgStream;

  public:

    ERR_STREAM():
      m_msgStream( XG_msg, sizeof( XG_msg ) )
    {
      if( XG_msgStreamExists )
      {
        fprintf( stderr,
                 "TRYING TO SET UP MORE THAN ONE ERR_STREAM!!!\n" );
        _exit( -1 );
      }
      else
      {
        m_msgStream << "Test"; // make sure this causes no errors
                               //   before we commit to using it
        m_msgStream.seekp( 0 );
        XG_msgStreamExists = 1;
      }
    }

    ~ERR_STREAM()
    {
      XG_msgStreamExists = 0;
    }

    ostrstream &getStream() { return m_msgStream; }
};

SAFE_GLOBAL( ERR_STREAM, XG_errStream )

/*-------------------------------------------------------------------*
 | THROW_ERR() -- macro to spit out an error message and exit
 *-------------------------------------------------------------------*/

#define THROW_ERR( description )                                      \
  {                                                                   \
    G_errHasHappened = 1;                                             \
    if( XG_msgStreamExists )                                          \
      XG_errStream.getStream() << description;                        \
    else                                                              \
      strncpy( XG_msg, #description, sizeof( XG_msg ) );              \
    XDoErr( __LINE__, __FILE__ );                                     \
  }                                                                   \

/*-------------------------------------------------------------------*
 | External routines
 *-------------------------------------------------------------------*/

GLOBAL void Indent( int numSpaces ); // routine to indent cout
GLOBAL void DumpCore();              // routine to crash with core dump
GLOBAL void XDoErr( int line, char *file );

#undef GLOBAL
#endif

