
/*********************************************************************
 * FILE: vector.H                                                    *
 *                                                                   *
 * AUTHOR: Matthew Miller (mlm)                                      *
 *                                                                   *
 * HISTORY:                                                          *
 *    1 JUN 93 -- (mlm) created                                      *
 *                                                                   *
 * CONTENTS:                                                         *
 *                                                                   *
 *   Template for resizeable, one dimensional array.                 *
 *                                                                   *
 *   The template is used in one of the following ways:              *
 *                                                                   *
 *     VECTOR_OF< type > vec;                                        *
 *     VECTOR_OF< type > vec( highIndex );                           *
 *     VECTOR_OF< type > vec( lowIndex, highIndex );                 *
 *                                                                   *
 *   Where                                                           *
 *                                                                   *
 *     type = the type of object to be contained in the vector       *
 *     vec = the vector object                                       *
 *     highIndex = highest allowable index into vec                  *
 *     lowIndex = lowest allowable index into vec                    *
 *                                                                   *
 *   When lowIndex is not given, it defaults to 0.  So vec( 5 )      *
 *   constructs a 6 element vector, indexed from vec[ 0 ] to         *
 *   vec[ 5 ].                                                       *
 *                                                                   *
 *   When neither lowIndex nor highIndex is given, no memory is      *
 *   allocated, and the vector cannot be used until it is resized    *
 *   (see below).                                                    *
 *                                                                   *
 *   The vector has the following member functions:                  *
 *                                                                   *
 *     operator[]( index ) --                                        *
 *       Access a given element of the vector.                       *
 *                                                                   *
 *     resize( highIndex ), or                                       *
 *     resize( lowIndex, highIndex ) --                              *
 *       Change the limits on the index.  New memory is only         *
 *       allocated if the old buffer isn't big enough.  Resize       *
 *       never causes the vector to free any memory.  That can only  *
 *       be done by destroying the vector and constructing a new     *
 *       one.                                                        *
 *                                                                   *
 *     clear() --                                                    *
 *       Set all the bytes in the vector to 0.  This is dangerous,   *
 *       and is meant only for use with vectors of simple data       *
 *       types.                                                      *
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

#ifndef VECTOR_H
#define VECTOR_H

#include "except.h"
#include <assert.h>
#include <string.h>		// for memset()

template< class TYPE >
  class VECTOR_OF
  {
    private:

      int m_size;
      int m_lowIndex;
      int m_highIndex;
      TYPE *m_buf;
      TYPE *m_data;

    public:

      VECTOR_OF():
        m_size( 0 ),
        m_lowIndex( 0 ),
        m_highIndex( 0 ),
        m_buf( 0 ),
        m_data( 0 )
      {
      }

      VECTOR_OF( int highIndex ):
        m_size( highIndex + 1 ),
        m_lowIndex( 0 ),
        m_highIndex( highIndex ),
        m_buf( new TYPE[ m_size ] ),
        m_data( m_buf )
      {
      }

      VECTOR_OF( int lowIndex, int highIndex ):
        m_size( highIndex - lowIndex + 1 ),
        m_lowIndex( lowIndex ),
        m_highIndex( highIndex ),
        m_buf( new TYPE[ m_size ] ),
        m_data( m_buf - lowIndex )
      {
      }

      ~VECTOR_OF()
      {
        delete [] m_buf;
      }

      TYPE &operator[]( int index )
      {
        #ifdef TSTBUG
          // NOTE: the assertion booleans have been negated from their original if statements.
          assert( m_buf != 0 );
          //  THROW_ERR("Trying to index into unallocated vector")
          assert( m_lowIndex <= index && index <= m_highIndex );
          //  THROW_ERR("Index out of bounds in vector" << index )
        #endif

        return m_data[ index ];
      }

      void resize( int highIndex )
      {
        int newSize = highIndex + 1;

        if( newSize > m_size )
        {
          delete [] m_buf;
          m_buf = new TYPE[ newSize ];
          m_size = newSize;
        }

        m_lowIndex = 0;
        m_highIndex = highIndex;
        m_data = m_buf;
      }

      void resize( int lowIndex, int highIndex )
      {
        int newSize = highIndex - lowIndex + 1;

        if( newSize > m_size )
        {
          delete [] m_buf;
          m_buf = new TYPE[ newSize ];
          m_size = newSize;
        }

        m_lowIndex = lowIndex;
        m_highIndex = highIndex;
        m_data = m_buf - lowIndex;
      }

      void clear()
      {
        if( m_buf != 0 )
          memset( m_buf, 0,
                  (m_highIndex - m_lowIndex + 1) * sizeof( TYPE ) );
      }
  };

#endif

