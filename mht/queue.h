
/*********************************************************************
 * FILE: queue.H                                                     *
 *                                                                   *
 * AUTHOR: Matthew Miller (mlm)                                      *
 *                                                                   *
 * HISTORY:                                                          *
 *    2 JUN 93 -- (mlm) created                                      *
 *                                                                   *
 * CONTENTS:                                                         *
 *                                                                   *
 *   Template for simple, FIFO queue.                                *
 *                                                                   *
 *   The template is used in one of the following ways:              *
 *                                                                   *
 *     QUEUE_OF< type > queue;                                       *
 *     QUEUE_OF< type > queue( size );                               *
 *                                                                   *
 *   Where                                                           *
 *                                                                   *
 *     type = the type of object to be contained in the queue        *
 *     queue = the queue object                                      *
 *     size = the maximum number of objects that can be in the queue *
 *                                                                   *
 *   When size is not given, no memory is allocated, and the queue   *
 *   cannot be used until it is resized (see below).                 *
 *                                                                   *
 *   The queue has the following member functions:                   *
 *                                                                   *
 *     resize( newSize )                                             *
 *       Change the maximum number of objects that can be stored in  *
 *       the queue.                                                  *
 *                                                                   *
 *       This member function may only be used when the queue        *
 *       is empty.                                                   *
 *                                                                   *
 *       New memory is only allocated if newSize is larger than the  *
 *       old size.  This member function can't be used to reduce the *
 *       amount of memory being used.                                *
 *                                                                   *
 *     isEmpty()                                                     *
 *       Return 1 if the queue is empty, 0 otherwise.                *
 *                                                                   *
 *     clear()                                                       *
 *       Empty the queue.                                            *
 *                                                                   *
 *     put( object )                                                 *
 *       Put an object of the right type onto the queue.             *
 *                                                                   *
 *     get()                                                         *
 *       Remove the next object from the queue, and return it.       *
 *                                                                   *
 *     peek()                                                        *
 *       Return the next object on the queue, without removing it.   *
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

#ifndef QUEUE_H
#define QUEUE_H

#include "except.h"
#include <assert.h>

template< class TYPE >
class QUEUE_OF
{
private:

    TYPE *m_data;
    int m_size;
    int m_writePos;
    int m_readPos;

public:

    QUEUE_OF():
        m_data( 0 ),
        m_size( 0 ),
        m_writePos( 0 ),
        m_readPos( 0 )
    {
    }

    QUEUE_OF( int size ):
        m_data( new TYPE[ size + 2 ] ),
        m_size( size + 2 ),
        m_writePos( 0 ),
        m_readPos( m_size - 1 )
    {
    }

    ~QUEUE_OF()
    {
        delete [] m_data;
    }

    void resize( int size )
    {
#ifdef TSTBUG
        assert( isEmpty() );
        //  THROW_ERR( "Queue is not empty" )
#endif

        size += 2;
        if( size > m_size )
        {
            delete [] m_data;
            m_data = new TYPE[ size ];
            m_size = size;
            clear();
        }
    }

    int isEmpty() const
    {
        return (m_readPos == m_size - 1 && m_writePos == 0) ||
               (m_readPos + 1 == m_writePos);
    }

    void clear()
    {
        m_writePos = 0;
        m_readPos = m_size - 1;
    }

    void put( const TYPE &info )
    {
#ifdef TSTBUG
        assert( m_data != 0 );
        //  THROW_ERR( "Trying to put() onto unallocated queue" )
#endif

        m_data[ m_writePos++ ] = info;
        if( m_writePos >= m_size )
        {
            m_writePos = 0;
        }

#ifdef TSTBUG
        assert( m_writePos != m_readPos );
        //  THROW_ERR( "Queue overflow" )
#endif
    }

    TYPE get()
    {
#ifdef TSTBUG
        assert( m_data != 0 );
        //  THROW_ERR( "Trying to get() from unallocated queue" )
#endif

        if( ++m_readPos >= m_size )
        {
            m_readPos = 0;
        }

#ifdef TSTBUG
        assert( m_readPos != m_writePos );
        //  THROW_ERR( "Queue underflow" );
#endif

        return m_data[ m_readPos ];
    }

    TYPE peek()
    {
#ifdef TSTBUG
        assert( m_data != 0 );
        //  THROW_ERR( "Trying to peek() into unallocated queue" )
#endif

        int readPos;

        readPos = m_readPos + 1;
        if( readPos >= m_size )
        {
            readPos = 0;
        }

#ifdef TSTBUG
        assert( readPos != m_writePos );
        //  THROW_ERR( "Queue underflow" );
#endif

        return m_data[ readPos ];
    }
};

#endif

