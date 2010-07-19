
/*********************************************************************
 * FILE: stack.H                                                     *
 *                                                                   *
 * AUTHOR: Matthew Miller (mlm)                                      *
 *                                                                   *
 * HISTORY:                                                          *
 *    2 JUN 93 -- (mlm) created                                      *
 *                                                                   *
 * CONTENTS:                                                         *
 *                                                                   *
 *   Template for simple, LIFO stack.                                *
 *                                                                   *
 *   The template is used in one of the following ways:              *
 *                                                                   *
 *     STACK_OF< type > stack;                                       *
 *     STACK_OF< type > stack( size );                               *
 *                                                                   *
 *   Where                                                           *
 *                                                                   *
 *     type = the type of object to be contained in the stack        *
 *     stack = the stack object                                      *
 *     size = the maximum number of objects that can be in the stack *
 *                                                                   *
 *   When size is not given, no memory is allocated, and the stack   *
 *   cannot be used until it is resized (see below).                 *
 *                                                                   *
 *   The stack has the following member functions:                   *
 *                                                                   *
 *     resize( newSize )                                             *
 *       Change the maximum number of objects that can be stored in  *
 *       the stack.                                                  *
 *                                                                   *
 *       This member function may only be used when the stack        *
 *       is empty.                                                   *
 *                                                                   *
 *       New memory is only allocated if newSize is larger than the  *
 *       old size.  This member function can't be used to reduce the *
 *       amount of memory being used.                                *
 *                                                                   *
 *     isEmpty()                                                     *
 *       Return 1 if the stack is empty, 0 otherwise.                *
 *                                                                   *
 *     clear()                                                       *
 *       Empty the stack.                                            *
 *                                                                   *
 *     put( object )                                                 *
 *       Put an object of the right type onto the stack.             *
 *                                                                   *
 *     get()                                                         *
 *       Remove the next object from the stack, and return it.       *
 *                                                                   *
 *     peek()                                                        *
 *       Return the next object on the stack, without removing it.   *
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

#ifndef STACK_H
#define STACK_H

#include "except.h"
#include <assert.h>

template< class TYPE >
class STACK_OF
{
private:

    TYPE *m_data;
    int m_size;
    int m_top;

public:

    STACK_OF():
        m_data( 0 ),
        m_size( 0 ),
        m_top( 0 )
    {
    }

    STACK_OF( int size ):
        m_data( new TYPE[ size + 2 ] ),
        m_size( size ),
        m_top( 0 )
    {
    }

    ~STACK_OF()
    {
        delete [] m_data;
    }

    void resize( int size )
    {
#ifdef TSTBUG
        assert( isEmpty() );
        //  THROW_ERR( "Stack is not empty" )
#endif

        if( size > m_size )
        {
            delete [] m_data;
            m_data = new TYPE[ size ];
            m_size = size;
        }
    }

    int isEmpty() const
    {
        return m_top == 0;
    }

    void clear()
    {
        m_top = 0;
    }

    void put( const TYPE &info )
    {
#ifdef TSTBUG
        assert( m_data != 0 );
        //THROW_ERR( "Trying to put() onto unallocated stack" )
        assert( m_top < m_size );
        //THROW_ERR( "Stack overflow" );
#endif

        m_data[ m_top++ ] = info;
    }

    TYPE get()
    {
#ifdef TSTBUG
        assert( m_data != 0 );
        //  THROW_ERR( "Trying to get() from unallocated stack" )
        assert( m_top != 0 );
        //  THROW_ERR( "Stack underflow" );
#endif

        return m_data[ --m_top ];
    }

    TYPE peek()
    {
#ifdef TSTBUG
        assert( m_data != 0 );
        //  THROW_ERR( "Trying to peek() into unallocated stack" )
        assert( m_top != 0 );
        //  THROW_ERR( "Stack underflow" );
#endif

        return m_data[ m_top - 1 ];
    }
};

#endif

