
/*********************************************************************
 * FILE: tree.C                                                      *
 *                                                                   *
 * AUTHOR: Matthew Miller (mlm)                                      *
 *                                                                   *
 * HISTORY:                                                          *
 *    3 JUN 93 -- (mlm) created                                      *
 *                                                                   *
 * CONTENTS:                                                         *
 *                                                                   *
 *   Routines for trees.  See tree.H for details.                    *
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

#define DECLARE_TREE

#include "tree.h"

/*-------------------------------------------------------------------*
 | TREEnode::XgetFirstLeaf() -- get the first leaf descended from
 |                              this node
 *-------------------------------------------------------------------*/

TREEnode *TREEnode::XgetFirstLeaf() const
{
    TREEnode const*node;

    for( node = this; ! node->isLeaf(); node = node->XgetFirstChild() )
        ;

    return (TREEnode *)node;
}

/*-------------------------------------------------------------------*
 | TREEnode::XgetLastLeaf() -- get the last leaf descended from this
 |                             node
 *-------------------------------------------------------------------*/

TREEnode *TREEnode::XgetLastLeaf() const
{
    TREEnode const*node;

    for( node = this; ! node->isLeaf(); node = node->XgetLastChild() )
        ;

    return (TREEnode *)node;
}

/*-------------------------------------------------------------------*
 | TREEnode::XgetRoot() -- get the root of the tree that this node
 |                         is on
 *-------------------------------------------------------------------*/

TREEnode *TREEnode::XgetRoot() const
{


    TREEnode const*node;

    checkOnTree();

    for( node = this; ! node->isRoot(); node = node->XgetParent() )
        ;

    return (TREEnode *)node;
}

/*-------------------------------------------------------------------*
 | TREEnode::XgetPreOrderNext() -- get the next node in pre-order
 |
 | See documentation for LOOP_TREE() in the header comments of
 | tree.H for a definition of pre-order.
 *-------------------------------------------------------------------*/

TREEnode *TREEnode::XgetPreOrderNext() const
{


    TREEnode const*node;

    if( ! isLeaf() )
    {
        node = XgetFirstChild();
    }
    else
    {
        for( node = this; node->isLastSibling(); node = node->XgetParent() )
            ;
        node = node->XgetNextSibling();
    }

    return (TREEnode *)node;
}

/*-------------------------------------------------------------------*
 | TREEnode::XgetPostOrderNext() -- get the next node in post-order
 |
 | See documentation for LOOP_TREEpostOrder() in the header comments
 | of tree.H for a definition of post-order.
 *-------------------------------------------------------------------*/

TREEnode *TREEnode::XgetPostOrderNext() const
{


    TREEnode const*node;

    if( isLastSibling() )
    {
        node = XgetParent();
    }
    else
    {
        node = XgetNextSibling()->XgetFirstLeaf();
    }

    return (TREEnode *)node;
}

/*-------------------------------------------------------------------*
 | TREEnode::XinsertParent() -- insert a new node into the tree as
 |                              the parent of this node
 *-------------------------------------------------------------------*/

void TREEnode::XinsertParent( TREEnode *node )
{


    node->m_parent = m_parent;
    Xprepend( node );
    DLISTnode::unlink();
    m_parent = node;
    node->m_childList.prepend( this );

    check();
}

/*-------------------------------------------------------------------*
 | TREEnode::getDepth() -- find the distance between the root of the
 |                         tree and this node
 *-------------------------------------------------------------------*/

int TREEnode::getDepth() const
{


    TREEnode const*node;
    int depth;

    checkIsNode();
    checkOnTree();

    depth = 0;
    for( node = this; ! node->isRoot(); node = node->XgetParent() )
    {
        depth++;
    }

    return depth;
}

/*-------------------------------------------------------------------*
 | TREEnode::check() -- check the integrity of the whole tree that
 |                      this node is on
 *-------------------------------------------------------------------*/

#ifdef DEBUG

void TREEnode::check() const
{
    TREEnode const*node;

    if( isNode() && m_parent == this )
    {
        DLISTnode::check();
        checkNotOnList();

        assert( m_childList.isEmpty() );
        //  THROW_ERR( "Tree node has children but isn't on tree" )

        return;
    }

    for( node = this; node->isNode(); node = node->m_parent )
    {
        assert( node->m_parent != node );
        //  THROW_ERR( "Tree node has children but isn't on tree" )
    }

    node->DLISTnode::check();

    assert( node->m_parent == node );
    //  THROW_ERR( "Corrupted tree -- node->m_parent != node" )
    assert( node->m_childList.isEmpty() ||
            node->m_childList.hasOneMember() );
    //  THROW_ERR( "Tree has more than one root" )

    if( node->m_childList.hasOneMember() )
    {
        node->m_childList.check();
        assert( (*node->m_childList).m_parent == node );
        //  THROW_ERR( "Corrupted tree root -- doesn't point to parent" )
        (*node->m_childList).XrcrsvCheck();
    }
}

#endif

/*-------------------------------------------------------------------*
 | TREEnode::XrcrsvCheck() -- check this node and all its children
 |                            (and children's children, etc.)
 *-------------------------------------------------------------------*/

void TREEnode::XrcrsvCheck() const
{


    PTR_INTO_iDLIST_OF< TREEnode > child;

    m_childList.check();
    LOOP_DLIST( child, *(iDLIST_OF< TREEnode > *)&m_childList )
    {
        assert( (*child).m_parent == this );
        //  THROW_ERR( "Corrupted tree node -- doesn't point to parent" )
        (*child).XrcrsvCheck();
    }
}

/*-------------------------------------------------------------------*
 | TREEnode::XrcrsvCopy() -- make a copy of this node and all its
 |                           children (and children's children, etc.)
 *-------------------------------------------------------------------*/

void TREEnode::XrcrsvCopy( const TREEnode *node )
{


    TREEnode *newNode;

#ifdef COMPILER_HAS_BEEN_DEBUGGED
    while( node->isNode() )
#else
    for( ; node->isNode() ; )
#endif
    {
        newNode = (TREEnode *)node->XmakeCopy();
        XinsertLastChild( newNode );

        if( ! node->isLeaf() )
        {
            newNode->XrcrsvCopy( node->XgetFirstChild() );
        }

        node = node->XgetNextSibling();
    }
}

/*-------------------------------------------------------------------*
 | TREEbase::getHeight() -- get the distance from the root to the
 |                          first leaf
 *-------------------------------------------------------------------*/

int TREEbase::getHeight() const
{


    int height;
    TREEnode const*node;

    height = 0;

    for( node = XgetRoot();
            ! node->isLeaf();
            node = node->XgetFirstChild() )
    {
        height++;
    }

    return height;
}

/*-------------------------------------------------------------------*
 | TREEbase::removeRoot() -- remove the root of the tree
 *-------------------------------------------------------------------*/

void TREEbase::removeRoot()
{


    TREEnode *oldRoot;

    checkNotEmpty();

    oldRoot = XgetRoot();
    // I think this check is invalid.
    // The splice will work, no matter how many children exists.
    // I can't see a reason why there has to be one and only one child.
    //oldRoot->checkHasOneChild();

    m_vnode.m_childList.splice( oldRoot->m_childList );
    delete oldRoot;

    XgetRoot()->m_parent = &m_vnode;

    check();
}

