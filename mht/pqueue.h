
/*********************************************************************
 * FILE: pqueue.H                                                    *
 *                                                                   *
 * AUTHOR: Matthew Miller (mlm)                                      *
 *                                                                   *
 * HISTORY:                                                          *
 *    2 JUN 93 -- (mlm) created                                      *
 *                                                                   *
 * CONTENTS:                                                         *
 *                                                                   *
 *   Template for largest-first priority queue.  The type that this  *
 *   template is instantiated with must have operator>() defined.    *
 *   At any given time, the next object that will be returned by     *
 *   get() or peek() (see below) is the one that compares greater    *
 *   than all the others in the queue, using operator>().            *
 *                                                                   *
 *   The priority queue is implemented as a heap.                    *
 *                                                                   *
 *   The template is used in one of the following ways:              *
 *                                                                   *
 *     PQUEUE_OF< type > pqueue;                                     *
 *     PQUEUE_OF< type > pqueue( size );                             *
 *                                                                   *
 *   Where                                                           *
 *                                                                   *
 *     type = the type of object to be contained in the pqueue       *
 *     pqueue = the pqueue object                                    *
 *     size = the maximum number of objects that can be in the       *
 *            pqueue                                                 *
 *                                                                   *
 *   When size is not given, no memory is allocated, and the pqueue  *
 *   cannot be used until it is resized (see below).                 *
 *                                                                   *
 *   The pqueue has the following member functions:                  *
 *                                                                   *
 *     resize( newSize )                                             *
 *       Change the maximum number of objects that can be stored in  *
 *       the pqueue.                                                 *
 *                                                                   *
 *       This member function may only be used when the pqueue       *
 *       is empty.                                                   *
 *                                                                   *
 *       New memory is only allocated if newSize is larger than the  *
 *       old size.  This member function can't be used to reduce the *
 *       amount of memory being used.                                *
 *                                                                   *
 *     isEmpty()                                                     *
 *       Return 1 if the pqueue is empty, 0 otherwise.               *
 *                                                                   *
 *     clear()                                                       *
 *       Empty the pqueue.                                           *
 *                                                                   *
 *     put( object )                                                 *
 *       Put an object of the right type onto the pqueue.            *
 *                                                                   *
 *     get()                                                         *
 *       Remove the next object from the pqueue, and return it.      *
 *                                                                   *
 *     peek()                                                        *
 *       Return the next object on the pqueue, without removing it.  *
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

#ifndef PQUEUE_H
#define PQUEUE_H

#include "except.h"
#include <assert.h>

template< class TYPE >
class PQUEUE_OF
{
private:

    TYPE *m_data;
    int m_size;
    int m_numObjects;

public:

    PQUEUE_OF():
        m_data( 0 ),
        m_size( 0 ),
        m_numObjects( 0 )
    {
    }

    PQUEUE_OF( int size ):
        m_data( new TYPE[ size + 1 ] ),
        m_size( size + 1 ),
        m_numObjects( 0 )
    {
    }

    ~PQUEUE_OF()
    {
        delete [] m_data;
    }

    void resize( int size )
    {
#ifdef TSTBUG
        assert( isEmpty() );
        //  THROW_ERR( "Priority queue is not empty" )
#endif

        size++;
        if( size > m_size )
        {
            delete [] m_data;
            m_data = new TYPE[ size ];
            m_size = size;
        }
    }

    int isEmpty() const
    {
        return m_numObjects == 0;
    }

    void clear()
    {
        m_numObjects = 0;
    }

    void put( const TYPE &info );

    TYPE get()
    {
        TYPE info;

#ifdef TSTBUG
        assert( m_data != 0 );
        //  THROW_ERR( "Trying to get() "
        //             "from unallocated priority queue" )
        assert( ! isEmpty() );
        //  THROW_ERR( "Priority queue underflow" );
#endif

        info = m_data[ 1 ];
        m_data[ 1 ] = m_data[ m_numObjects--];
        heapify( 1 );

        return info;
    }

    TYPE peek()
    {
#ifdef TSTBUG
        assert( m_data != 0 );
        //  THROW_ERR( "Trying to peek() "
        //             "into unallocated priority queue" )
        assert( ! isEmpty() );
        //  THROW_ERR( "Priority queue underflow" );
#endif

        return m_data[ 1 ];
    }

private:

    void heapify( int i );
};

#if TMPLT_HANDLING == 0
#include "pqueue.code"
#endif

#endif

