
/*********************************************************************
 * FILE: links.C                                                     *
 *                                                                   *
 * AUTHOR: Matthew Miller (mlm)                                      *
 *                                                                   *
 * HISTORY:                                                          *
 *   23 JUN 93 -- (mlm) created                                      *
 *                                                                   *
 * CONTENTS:                                                         *
 *                                                                   *
 *   Routines for links between objects.  See "links.H" for details. *
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

#include "links.h"

/*-------------------------------------------------------------------*
 | XMakeLinkBase() -- basic routine for making a link between two
 |                    objects
 *-------------------------------------------------------------------*/

void XMakeLinkBase( void *obj0, LINKSbase &links0,
                    void *obj1, LINKSbase &links1 )
{
  /* the following two nodes will be partners */
  LINK_NODE *node0 = new LINK_NODE;
  LINK_NODE *node1 = new LINK_NODE;

  links0.m_list.prepend( node0 );
  links1.m_list.prepend( node1 );

  node0->m_thatObj = obj1;
  node0->m_partner = node1;

  node1->m_thatObj = obj0;
  node1->m_partner = node0;
}

/*-------------------------------------------------------------------*
 | LINK_NODE::~LINK_NODE() -- destructor for LINK_NODE
 |
 | Each LINK_NODE contains a pointer to the object that it links with
 | and a pointer to its partner.  When a LINK_NODE is deleted, it
 | must delete it's partner.  To avoid infinite recursion (the partner
 | trying to delete the node back), the pointer to the object is used
 | as a flag.  If the pointer is null when the destructor is called,
 | it means that the LINK_NODE is being deleted by its partner, and
 | shouldn't try to delete its partner back.
 *-------------------------------------------------------------------*/

LINK_NODE::~LINK_NODE()
{
  if( m_thatObj != 0 )
  {
    m_partner->m_thatObj = 0;
    delete m_partner;
  }
}

