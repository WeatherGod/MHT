
/*********************************************************************
 * FILE: pqueue.C                                                    *
 *                                                                   *
 * AUTHOR: Matthew Miller (mlm)                                      *
 *                                                                   *
 * HISTORY:                                                          *
 *   22 FEB 95 -- (mlm) created                                      *
 *                                                                   *
 * CONTENTS:                                                         *
 *                                                                   *
 *   Definitions of flags for various incompatible features of       *
 *   different c++ compilers.                                        *
 *                                                                   *
 *   TMPLT_HANDLING -- how the compiler handles out-of-line          *
 *                     template functions                            *
 *     0 = Out-of-line functions for template classes must be        *
 *         defined in the include file in which the template is      *
 *         declared.                                                 *
 *     1 = Out-of-line functions for template classes must be        *
 *         defined in a separate source file.                        *
 *                                                                   *
 *   INFINITY -- representation of infinity as a double              *
 *                                                                   *
 *   STATIC_FUNC -- what has to come before the declaration of       *
 *                  a static function inside another function        *
 *     Either "static" or nothing.                                   *
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

#ifndef PORTSPECS_H
#define PORTSPECS_H

/*-------------------------------------------------------------------*
 | SGI compiler (CC)
 *-------------------------------------------------------------------*/

#if defined( __SGICC__ ) // NOTE: I couldn't find the predefined
                         //       token that uniquely defines the SGI
                         //       compiler, CC.  I'm just defining
                         //       this token on the command line.

  #define TMPLT_HANDLING 1
  extern const double __infinity;
  #define INFINITY __infinity
  #define STATIC_FUNC static

/*-------------------------------------------------------------------*
 | GNU compiler (g++)
 *-------------------------------------------------------------------*/

#elif defined( __GNUC__ )

  #define TMPLT_HANDLING 0
  extern const double __infinity;
  #define INFINITY __infinity
  #define STATIC_FUNC static

/*-------------------------------------------------------------------*
 | Unknown compiler -- generate error
 *-------------------------------------------------------------------*/

#else

  #error Unknown compiler -- please modify portspecs.h

#endif

#endif

