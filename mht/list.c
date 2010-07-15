
/*********************************************************************
 * FILE: list.C                                                      *
 *                                                                   *
 * AUTHOR: Matthew Miller (mlm)                                      *
 *                                                                   *
 * HISTORY:                                                          *
 *    3 JUN 93 -- (mlm) created                                      *
 *                                                                   *
 * CONTENTS:                                                         *
 *                                                                   *
 *   Routines for doubly-linked lists.  See list.H for details.      *
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

#define DECLARE_LIST

#include "list.h"

/*-------------------------------------------------------------------*
 | DLISTnode::XgetHead() -- find the head of a list
 *-------------------------------------------------------------------*/

DLISTnode *DLISTnode::XgetHead() const
{
  

  DLISTnode const*node;

  checkOnList();

  for( node = m_prev; node->isNode(); node = node->m_prev )
  {
    #ifdef TSTBUG
      assert( node != this );
      //  THROW_ERR( "Dlist has no head" )
    #endif
  }

  return node->m_next;
}

/*-------------------------------------------------------------------*
 | DLISTnode::XgetTail() -- find the tail of a list
 *-------------------------------------------------------------------*/

DLISTnode *DLISTnode::XgetTail() const
{
  

  DLISTnode const*node;

  checkOnList();

  for( node = m_prev; node->isNode(); node = node->m_prev )
  {
    #ifdef TSTBUG
      assert( node != this );
      //  THROW_ERR( "Dlist has no tail" )
    #endif
  }

  return node->m_prev;
}

/*-------------------------------------------------------------------*
 | DLISTnode::check() -- check the integrity of an entire list
 |
 | (Only if DEBUG is defined.)
 *-------------------------------------------------------------------*/

#ifdef DEBUG

  void DLISTnode::check() const
  {
    

    DLISTnode const*node;
    int listHasHeader;

    assert( m_prev != 0 && m_next != 0 );
    //  THROW_ERR( "Bad dlist node -- NULL link" );

    if( ! isOnList() )
      return;

    listHasHeader = 0;
    node = this;
    do
    {
      assert( node->m_prev->m_next == node &&
          node->m_next->m_prev == node );
      //  THROW_ERR( "Bad dlist node -- links don't match" )

      if( ! node->isNode() )
      {
        assert( ! listHasHeader );
        //  THROW_ERR( "More than one header on dlist" )
        listHasHeader = 1;
      }

      node = node->m_next;
    }
    while( node != this );

    assert( listHasHeader );
    //  THROW_ERR( "Dlist without header" )
  }

#endif

/*-------------------------------------------------------------------*
 | DLISTbase::XprependCopy() -- put a copy of a list onto the
 |                              beginning of this one
 *-------------------------------------------------------------------*/

void DLISTbase::XprependCopy( const DLISTbase &dlist )
{
  

  DLISTnode const*node;

  for( node = dlist.XgetTail(); node->isNode(); node = node->m_prev )
    Xprepend( node->XmakeCopy() );
}

/*-------------------------------------------------------------------*
 | DLISTbase::XappendCopy() -- put a copy of a list onto the end
 |                             of this one
 *-------------------------------------------------------------------*/

void DLISTbase::XappendCopy( const DLISTbase &dlist )
{
  

  DLISTnode const*node;

  for( node = dlist.XgetHead(); node->isNode(); node = node->m_next )
    Xappend( node->XmakeCopy() );
}

/*-------------------------------------------------------------------*
 | DLISTbase::Xsplice() -- attach a list onto the end of this one
 |
 | This empties the list it is given.
 *-------------------------------------------------------------------*/

void DLISTbase::Xsplice( DLISTbase &dlist )
{
  

  if( dlist.isEmpty() )
    return;

  if( this->isEmpty() )
  {
    DLISTnode *dlistHead = dlist.XgetHead();
    DLISTnode *dlistTail = dlist.XgetTail();

    dlistHead->m_prev = &m_vnode;
    m_vnode.m_next = dlistHead;

    dlistTail->m_next = &m_vnode;
    m_vnode.m_prev = dlistTail;
  }
  else
  {
    DLISTnode *thisTail = XgetTail();
    DLISTnode *dlistHead = dlist.XgetHead();
    DLISTnode *dlistTail = dlist.XgetTail();

    thisTail->m_next = dlistHead;
    dlistHead->m_prev = thisTail;

    dlistTail->m_next = &m_vnode;
    m_vnode.m_prev = dlistTail;
  }

  dlist.m_vnode.Xreset();

  m_vnode.check();
}

/*-------------------------------------------------------------------*
 | DLISTbase::getLength() -- count the objects on the list
 *-------------------------------------------------------------------*/

int DLISTbase::getLength() const
{
  

  DLISTnode const*node;
  int length;

  length = 0;
  for( node = XgetHead(); node->isNode(); node = node->m_next )
    length++;

  return length;
}

/*-------------------------------------------------------------------*
 | DLISTbase::removeAll() -- remove everything from the list
 *-------------------------------------------------------------------*/

void DLISTbase::removeAll()
{
  

  while( ! isEmpty() )
    removeHead();
}

