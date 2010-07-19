
/*********************************************************************
 * FILE: safeglobal.H                                                *
 *                                                                   *
 * AUTHOR: Matthew Miller (mlm)                                      *
 *                                                                   *
 * HISTORY:                                                          *
 *    9 MAR 93 -- (mlm) created (replaces old "pointer.h")           *
 *   20 FEB 95 -- (mlm) changed naming of secret identifiers from    *
 *                "_name" to "Xname" for portability                 *
 *   21 FEB 95 -- (mlm) changed the reference counter (count) in     *
 *                SAFE_GLOBAL_HOLDER from int to short               *
 *                                                                   *
 * CONTENTS:                                                         *
 *                                                                   *
 *   SAFE_GLOBAL() -- macro for making a global object               *
 *                                                                   *
 *   This macro is meant for use in header files.  It creates a      *
 *   global reference to an object of a given type, and ensures      *
 *   that the object has been allocated and initialized right        *
 *   after the instantiation of SAFE_GLOBAL().                       *
 *                                                                   *
 *   SAFE_GLOBAL() requires that the macro GLOBAL be declared as     *
 *   either "extern" or nothing.                                     *
 *                                                                   *
 *   It isn't obvious why this macro is needed.  I'll explain.       *
 *                                                                   *
 *   Suppose you have an object class, FURRY_BUNNY, and you need     *
 *   to have a global one of these objects.  Suppose also, that      *
 *   FURRY_BUNNY's are complicated, and have constructors.  You      *
 *   might have a header file, rabbitden.H, which would contain      *
 *   the line:                                                       *
 *                                                                   *
 *     extern FURRY_BUNNY G_bugs;                                    *
 *                                                                   *
 *   This would be fine, but there's no guarantee that the           *
 *   constructor for G_bugs is called before the end of the header   *
 *   file.  That can cause a problem if, for example, you have       *
 *   another class, TASMANIAN_DEVIL, which refers to G_bugs in       *
 *   its constructor, like this:                                     *
 *                                                                   *
 *     TASMANIAN_DEVIL::TASMANIAN_DEVIL()                            *
 *     {                                                             *
 *       m_objective = G_bugs.getHead();                             *
 *     }                                                             *
 *                                                                   *
 *   Now, if we have another file that includes rabbitden.H, and     *
 *   declares a global or static TASMANIAN_DEVIL, there's no         *
 *   guarantee that G_bugs will be initialized before the devil      *
 *   is.  The file would say:                                        *
 *                                                                   *
 *     #include "rabbitden.h"                                        *
 *                                                                   *
 *     TASMANIAN_DEVIL G_devil;                                      *
 *                                                                   *
 *   The constructors for G_devil and G_bugs can be called in any    *
 *   order (it probably depends on the linker or something).  If     *
 *   G_devil is constructed before G_bugs, G_bugs.getHead() might    *
 *   not work properly.                                              *
 *                                                                   *
 *   If, instead of declaring G_bugs as "extern", you declare it     *
 *   with SAFE_GLOBAL, like this:                                    *
 *                                                                   *
 *     SAFE_GLOBAL( FURRY_BUNNY, G_bugs );                           *
 *                                                                   *
 *   then the constructor is guaranteed to be run by the time        *
 *   G_bugs is used in any source file, so everything will work      *
 *   correctly.                                                      *
 *                                                                   *
 * NOTES:                                                            *
 *                                                                   *
 *   This file is meant to be included ONLY in header files.  It     *
 *   assumes that the macro GLOBAL has been defined to either        *
 *   nothing or "extern".                                            *
 *                                                                   *
 *   A global variable declared with SAFE_GLOBAL() uses a little     *
 *   more memory than a normal global (a few extra bytes for each    *
 *   source file that the .H file is included in).                   *
 *                                                                   *
 *   The global variable given in SAFE_GLOBAL() (e.g. "G_bugs") is   *
 *   actually declared as a statically declared global.  It is a     *
 *   reference to a global object.  This means that, if it isn't     *
 *   used in a source file, a compiler warning message might result  *
 *   (e.g. "warning:  ::G_bugs defined but not used").  Ignore this  *
 *   warning.                                                        *
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

#ifndef SAFEGLOBAL_H
#define SAFEGLOBAL_H

//#include "portspecs.h"

/*-------------------------------------------------------------------*
 | SAFE_GLOBAL() -- macro to make a safe global variable
 *-------------------------------------------------------------------*/

#define SAFE_GLOBAL( TYPE, var )                                      \
  GLOBAL SAFE_GLOBAL_HOLDER< TYPE > XG_safe##var;                     \
  static SAFE_GLOBAL_INITIALIZER< TYPE >                              \
    XG_safe##var##Initializer( &XG_safe##var );                       \
  static TYPE &var = *XG_safe##var.m_var;                             \
 
/*-------------------------------------------------------------------*
 | SAFE_GLOBAL_HOLDER< TYPE > -- global data structure to hold a
 |                               safe global variable
 |
 | This holds a pointer to the safe global variable, and keeps track
 | of the number of source files that refer to it.  The safe global
 | name, itself ("var" in the definition of SAFE_GLOBAL() above) is
 | a reference to the object that this pointer points to.
 |
 | This structure is not intended for use outside safeglobal.H.
 *-------------------------------------------------------------------*/

template< class TYPE >
struct SAFE_GLOBAL_HOLDER
{
    short m_counter;
    TYPE *m_var;
};

/*-------------------------------------------------------------------*
 | SAFE_GLOBAL_INITIALIZER< TYPE > -- initializer for a safe global
 |
 | Each file that includes a usage of SAFE_GLOBAL() contains one
 | static object of type SAFE_GLOBAL_INITIALIZER< TYPE >.  Since
 | the object is static, it is guaranteed to be constructed before
 | any subsequent line of code.
 |
 | When an object of class SAFE_GLOBAL_INITIALIZER< TYPE > is
 | constructed, it increments the counter for the safe global that
 | it refers to.  If it's the first time that the counter is
 | incremented, then the safe global is created using new.
 |
 | This is a variant of the tactic attributed to Jerry Schwarz --
 | "Initializing Static Variables in C++ Libraries," The C++ Report,
 | Volume 1, Number 2, February 1989
 |
 | (I got it from Bruce Eckel, C++ Inside Out, McGraw-Hill, 1993
 | pp 360-361)
 |
 | This template is not intended for use outside safeglobal.H.
 *-------------------------------------------------------------------*/

template< class TYPE >
class SAFE_GLOBAL_INITIALIZER
{
private:

    SAFE_GLOBAL_HOLDER< TYPE > *m_global;

public:

    SAFE_GLOBAL_INITIALIZER( SAFE_GLOBAL_HOLDER< TYPE > *global ):
        m_global( global )
    {
        if( m_global->m_counter++ == 0 )
        {
            m_global->m_var = new TYPE;
        }
    }

    ~SAFE_GLOBAL_INITIALIZER()
    {
        if( --m_global->m_counter == 0 )
        {
            delete m_global->m_var;
        }
    }
};

#endif

