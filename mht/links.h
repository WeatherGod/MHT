
/*********************************************************************
 * FILE: links.H                                                     *
 *                                                                   *
 * AUTHOR: Matthew Miller (mlm)                                      *
 *                                                                   *
 * HISTORY:                                                          *
 *   15 MAR 93 -- (mlm) created                                      *
 *   24 MAR 93 -- (mlm) added operator * for LINKS_TO< x >           *
 *   22 JUN 93 -- (mlm) moved these comments over to new version     *
 *                                                                   *
 * CONTENTS:                                                         *
 *                                                                   *
 *   Templates and macros for making links between objects.  A       *
 *   "link" is simply a pointer to another object, but it is set up  *
 *   so that it will be updated automatically if that other object   *
 *   is deleted, goes out of scope, or decides it doesn't want to    *
 *   be linked to this object anymore.                               *
 *                                                                   *
 *                            Overview                               *
 *                                                                   *
 *   If two classes of objects are to be allowed to link with each   *
 *   other, they should contain member objects defined with the      *
 *   LINKS_TO< TYPE > template.  For example:                        *
 *                                                                   *
 *     class BOY;                                                    *
 *     class GIRL;                                                   *
 *                                                                   *
 *     class BOY                                                     *
 *     {                                                             *
 *       friend class GIRL;                                          *
 *                                                                   *
 *       private:                                                    *
 *                                                                   *
 *         LINKS_TO< GIRL > girlFriends;                             *
 *                                                                   *
 *          .                                                        *
 *          .                                                        *
 *          .                                                        *
 *     };                                                            *
 *                                                                   *
 *     class GIRL                                                    *
 *     {                                                             *
 *       friend class BOY;                                           *
 *                                                                   *
 *       private:                                                    *
 *                                                                   *
 *         LINKS_TO< BOY > boyFriends;                               *
 *                                                                   *
 *          .                                                        *
 *          .                                                        *
 *          .                                                        *
 *     };                                                            *
 *                                                                   *
 *   Here, the "girlFriends" member of class BOY is a list of links  *
 *   to objects of class GIRL, and the "boyFriends" member of class  *
 *   GIRL is a list of links to objects of class BOY.  These two     *
 *   members will be used together.  Whenever a BOY's list of        *
 *   girlFriends contains a link to a GIRL, that GIRL's list of      *
 *   boyFriends will contain a link to that BOY.                     *
 *                                                                   *
 *   An object class may contain any number of lists of links to     *
 *   any other object classes, as long as each of those classes      *
 *   contains a list of links to it.  A class may also contain links *
 *   to objects of its own class.                                    *
 *                                                                   *
 *   Links are created between objects with the macro MAKE_LINK().   *
 *   This takes four arguments, as follows:                          *
 *                                                                   *
 *     MAKE_LINK( obj0, listMember0, obj1, listMember1 )             *
 *       obj0 -- Pointer to one object to be linked.                 *
 *       listMember0 -- Name of the LINKS_TO< TYPE > member of       *
 *         obj0 that the link should be placed on.  TYPE must be     *
 *         the class of obj1.                                        *
 *       obj1 -- Pointer to the other object to be linked.           *
 *       listMember1 -- Name of the LINKS_TO< TYPE > member of       *
 *         obj1 that the link should be placed on.  TYPE must be     *
 *         the class of obj0.                                        *
 *                                                                   *
 *   A pointer to obj0 is added to the list obj1->listMember1, and   *
 *   a pointer to obj1 is added to the list obj0->listMember0.       *
 *                                                                   *
 *   For example, we might have the following member function in     *
 *   class BOY:                                                      *
 *                                                                   *
 *     void BOY::goSteadyWith( GIRL *girl )                          *
 *     {                                                             *
 *       MAKE_LINK( this, girlFriends,                               *
 *                  girl, boyFriends );                              *
 *     }                                                             *
 *                                                                   *
 *   If we now say                                                   *
 *                                                                   *
 *     BOY archie;                                                   *
 *     GIRL veronica;                                                *
 *                                                                   *
 *     archie.goSteadyWith( &veronica );                             *
 *                                                                   *
 *   Then archie.girlFriends contains a pointer to veronica, and     *
 *   veronica.boyFriends contains a pointer to archie.               *
 *                                                                   *
 *   Note that the order of objects in MAKE_LINK() doesn't matter.   *
 *   We might just as well have said                                 *
 *                                                                   *
 *     MAKE_LINK( girl, boyFriends,                                  *
 *                this, girlFriends )                                *
 *                                                                   *
 *   The effect would have been the same.                            *
 *                                                                   *
 *   A list of links can contain any number of links.  We can go on  *
 *   to say                                                          *
 *                                                                   *
 *     BOY jughead;                                                  *
 *     GIRL betty;                                                   *
 *                                                                   *
 *     archie.goSteadyWith( &betty );                                *
 *     jughead.goSteadyWith( &veronica );                            *
 *                                                                   *
 *   Now archie.girlFriends contains pointers to both betty and      *
 *   veronica, and veronica.boyFriends contains pointers to both     *
 *   archie and jughead.                                             *
 *                                                                   *
 *   Lists of links are accessed through PTR_INTO_LINKS_TO< TYPE >   *
 *   objects.  These are used like PTR_INTO_xDLIST_OF< TYPE >        *
 *   objects.  The * operator, when applied to a                     *
 *   PTR_INTO_LINKS_TO< TYPE >, gives a reference to the object that *
 *   the link points to.                                             *
 *                                                                   *
 *   For example, we could add the following member functions to     *
 *   our BOY and GIRL classes (assuming we added a string member     *
 *   called "name", and initialized it to the name of the variable): *
 *                                                                   *
 *     void BOY::printName() { cout << name; }                       *
 *     void GIRL::printName() { cout << name; }                      *
 *                                                                   *
 *     void BOY::printGirlFriends()                                  *
 *     {                                                             *
 *       PTR_INTO_LINKS_TO< GIRL > p;                                *
 *                                                                   *
 *       LOOP_LINKS( p, girlFriends )                                *
 *       {                                                           *
 *         cout << " ";                                              *
 *         (*p).printName();                                         *
 *       }                                                           *
 *     }                                                             *
 *                                                                   *
 *     void GIRL::printBoyFriends()                                  *
 *     {                                                             *
 *       PTR_INTO_LINKS_TO< BOY > p;                                 *
 *                                                                   *
 *       LOOP_LINKS( p, boyFriends )                                 *
 *       {                                                           *
 *         cout << " ";                                              *
 *         (*p).printName();                                         *
 *       }                                                           *
 *     }                                                             *
 *                                                                   *
 *   (LOOP_LINKS is a macro for looping through a set of links.  It  *
 *   is analogous to the LOOP_DLIST macro in "list.H")               *
 *                                                                   *
 *   At this point, the code                                         *
 *                                                                   *
 *     archie.printGirlFriends();                                    *
 *                                                                   *
 *   should print out                                                *
 *                                                                   *
 *     betty veronica                                                *
 *                                                                   *
 *   (assuming betty.name has been initialized to "betty", and       *
 *   veronica.name has been initialized to "veronica").              *
 *                                                                   *
 *   The most important feature of links is that, if a link from     *
 *   object A to object B is removed for any reason, the             *
 *   corresponding link from object B to object A is also removed    *
 *   automatically.  So, for example, we might have the following    *
 *   member function in our BOY class:                               *
 *                                                                   *
 *     void BOY::breakUpWith( GIRL *girl )                           *
 *     {                                                             *
 *       PTR_INTO_LINKS_TO< GIRL > p;                                *
 *                                                                   *
 *       LOOP_LINKS( p, girlFriends )                                *
 *       {                                                           *
 *         if( p.get() == girl )                                     *
 *           p.remove();                                             *
 *       }                                                           *
 *     }                                                             *
 *                                                                   *
 *   If we now say                                                   *
 *                                                                   *
 *     archie.breakUpWith( &veronica );                              *
 *     archie.printGirlFriends(); cout << endl;                      *
 *     veronica.printBoyFriends(); cout << endl;                     *
 *                                                                   *
 *   the result will be                                              *
 *                                                                   *
 *     betty                                                         *
 *     jughead                                                       *
 *                                                                   *
 *   since the link to veronica was removed from archie.girlFriends, *
 *   and the link to archie was removed from veronica.boyFriends     *
 *   (automatically).                                                *
 *                                                                   *
 *   Similarly, if an object is deleted, all links to it will be     *
 *   removed.                                                        *
 *                                                                   *
 *                           LINKS_TO< TYPE >                        *
 *                                                                   *
 *   The LINKS_TO<> template defines a set of links to objects of    *
 *   the given type.  In general, it should be used only as a member *
 *   of an object class.                                             *
 *                                                                   *
 *   A member object declared as LINKS_TO< TYPE > has the following  *
 *   public member functions:                                        *
 *                                                                   *
 *   int isEmpty() const                                             *
 *     Returns 1 if there are no links, 0 if there are.              *
 *                                                                   *
 *   int hasOneMember() const                                        *
 *     Returns 1 if there is one and only one link, 0 if not.        *
 *                                                                   *
 *   int getLength() const                                           *
 *     Returns the number of links.                                  *
 *                                                                   *
 *   void removeHead()                                               *
 *     Removes the first link.                                       *
 *                                                                   *
 *   void removeTail()                                               *
 *     Removes the last link.                                        *
 *                                                                   *
 *   void removeAll()                                                *
 *     Removes all the links.                                        *
 *                                                                   *
 *   TYPE *getHead()                                                 *
 *     Returns a pointer to the first object linked to.              *
 *                                                                   *
 *   TYPE *getTail()                                                 *
 *     Returns a pointer to the last object linked to.               *
 *                                                                   *
 *   TYPE &operator*()                                               *
 *     Returns a reference to the first object linked to.            *
 *                                                                   *
 *                   PTR_INTO_LINKS_TO< TYPE >                       *
 *                                                                   *
 *   This template defines an iterator for a LINKS_TO<> object.      *
 *                                                                   *
 *   The constructor for a PTR_INTO_LINKS_TO< TYPE > object can      *
 *   take a number of different forms:                               *
 *                                                                   *
 *   PTR_INTO_LINKS_TO< TYPE > ptr;                                  *
 *     ptr is initialized to be invalid.                             *
 *                                                                   *
 *   PTR_INTO_LINKS_TO< TYPE > ptr( LINKS_TO< TYPE > &l ); or        *
 *   PTR_INTO_LINKS_TO< TYPE > ptr( LINKS_TO< TYPE > &l,             *
 *                                  START_AT_HEAD );                 *
 *     ptr is initialized to point to the first link in l.           *
 *     (START_AT_HEAD is a macro defined in "list.H").               *
 *                                                                   *
 *   PTR_INTO_LINKS_TO< TYPE > ptr( LINKS_TO< TYPE > &l,             *
 *                                  START_AT_TAIL );                 *
 *     ptr is initialized to point to the last link in l.            *
 *     (START_AT_TAIL is a macro defined in "list.H").               *
 *                                                                   *
 *   An object declared as PTR_INTO_LINKS_TO< TYPE > has the         *
 *   following member functions:                                     *
 *                                                                   *
 *   int isInitialized() const                                       *
 *     Returns 1 if the PTR_INTO_LINKS_TO<> object is pointing into  *
 *     a LINKS_TO<> object, 0 otherwise.                             *
 *                                                                   *
 *   int isValid() const                                             *
 *     Returns 1 if pointing at a valid link, 0 otherwise.           *
 *                                                                   *
 *   int isAtHead() const                                            *
 *     Returns 1 if pointing at first link, 0 otherwise.             *
 *                                                                   *
 *   int isAtTail() const                                            *
 *     Returns 1 if pointing at last link, 0 otherwise.              *
 *                                                                   *
 *   void gotoPrev() or                                              *
 *   PTR_INTO_LINKS_TO< TYPE > &operator--()                         *
 *     Move pointer to the previous link.                            *
 *                                                                   *
 *   void gotoNext() or                                              *
 *   PTR_INTO_LINKS_TO< TYPE > &operator++()                         *
 *     Move pointer to the next link.                                *
 *                                                                   *
 *   void remove()                                                   *
 *     Remove the link that this currently points to.  As with a     *
 *     PTR_INTO_xDLIST_OF<> object, it will still be possible to     *
 *     move the pointer around after the link it points to is        *
 *     removed by this function.                                     *
 *                                                                   *
 *   set( LINKS_TO< TYPE > &l ) or                                   *
 *   set( LINKS_TO< TYPE > &l, START_AT_HEAD )                       *
 *     Point to the first link in l.                                 *
 *                                                                   *
 *   set( LINKS_TO< TYPE > &l, START_AT_TAIL )                       *
 *     Point to the last link in l.                                  *
 *                                                                   *
 *   TYPE *get()                                                     *
 *     Get a pointer to the object connected by the current link.    *
 *                                                                   *
 *   TYPE &operator*()                                               *
 *     Returns a reference to the object connected by the current    *
 *     link.                                                         *
 *                                                                   *
 *                            Macros                                 *
 *                                                                   *
 *   There are three macros defined in this file.  They are:         *
 *                                                                   *
 *   MAKE_LINK( obj0, linksMember0, obj1, linksMember1 )             *
 *     Make a link between two objects.  See 'Overview' above for    *
 *     examples of usage.                                            *
 *                                                                   *
 *   LOOP_LINKS( p, l )                                              *
 *     Loop through a list of links.  If 'l' is of type              *
 *     LINKS_TO< TYPE >, then 'p' must be of type                    *
 *     PTR_INTO_LINKS_TO< TYPE >.  The loop goes from the first      *
 *     link on the list to the last.  In each iteration, 'p' points  *
 *     to one of the links.                                          *
 *                                                                   *
 *   LOOP_LINKSrev( p, l )                                           *
 *     This is the same as LOOP_LINKS( p, l ), except that it goes   *
 *     from the last link to the first.                              *
 *                                                                   *
 *   (See "list.H" for more on the concept of these LOOP_ macros)    *
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

#ifndef LINKS_H
#define LINKS_H

#include "except.h"
#include "list.h"
#include <assert.h>

/*-------------------------------------------------------------------*
 | Declarations of stuff found in this file.
 *-------------------------------------------------------------------*/

class LINK_NODE;
class LINKSbase;
class PTR_INTO_LINKSbase;

template< class TYPE > class LINKS_TO;
template< class TYPE > class PTR_INTO_LINKS_TO;

/*-------------------------------------------------------------------*
 | LINK_NODE -- representation of a link
 |
 | A LINKS_TO<> object is a list of LINK_NODEs.  Each link between
 | two objects is represented by two LINK_NODEs -- one link node in
 | one object's list, one in the other.  These two nodes are called
 | each other's "partners".
 *-------------------------------------------------------------------*/

class LINK_NODE: public DLISTnode
{
    friend class LINKSbase;
    friend class PTR_INTO_LINKSbase;
    friend void XMakeLinkBase( void *obj0, LINKSbase &links0,
                               void *obj1, LINKSbase &links1 );

private:

    void *m_thatObj;
    LINK_NODE *m_partner;

    LINK_NODE(): DLISTnode(), m_thatObj( 0 ), m_partner( 0 ) {}
    LINK_NODE( const LINK_NODE & )
    {
        assert(false);
        //THROW_ERR( "Trying to copy a LINK_NODE (don't know how)" )
    }

protected:

    MEMBERS_FOR_DLISTnode( LINK_NODE )

public:

    ~LINK_NODE();
};

/*-------------------------------------------------------------------*
 | LINKSbase -- base class for all LINKS_TO<> classes
 *-------------------------------------------------------------------*/

class LINKSbase
{
    friend class PTR_INTO_LINKSbase;
    friend void XMakeLinkBase( void *obj0, LINKSbase &links0,
                               void *obj1, LINKSbase &links1 );

private:

    iDLIST_OF< LINK_NODE > m_list;

protected:

    LINKSbase(): m_list() {}
    virtual ~LINKSbase() {}

    void *baseGetHead() const
    {
        return m_list.getHead()->m_thatObj;
    }
    void *baseGetTail() const
    {
        return m_list.getTail()->m_thatObj;
    }

public:

    int isEmpty() const
    {
        return m_list.isEmpty();
    }
    int hasOneMember() const
    {
        return m_list.hasOneMember();
    }
    int getLength() const
    {
        return m_list.getLength();
    }

    void removeHead()
    {
        m_list.removeHead();
    }
    void removeTail()
    {
        m_list.removeTail();
    }
    void removeAll()
    {
        m_list.removeAll();
    }
};

/*-------------------------------------------------------------------*
 | PTR_INTO_LINKSbase -- base class for all PTR_INTO_LINKS_TO<>
 |                       classes
 *-------------------------------------------------------------------*/

class PTR_INTO_LINKSbase
{
private:

    PTR_INTO_iDLIST_OF< LINK_NODE > m_ptr;

protected:

    PTR_INTO_LINKSbase(): m_ptr() {}
    PTR_INTO_LINKSbase( LINKSbase &links ): m_ptr( links.m_list ) {}
    PTR_INTO_LINKSbase( LINKSbase &links, START_AT_HEADversion ):
        m_ptr( links.m_list, START_AT_HEAD )
    {
    }
    PTR_INTO_LINKSbase( LINKSbase &links, START_AT_TAILversion ):
        m_ptr( links.m_list, START_AT_TAIL )
    {
    }

    virtual ~PTR_INTO_LINKSbase() {}

    void baseSet( LINKSbase &links )
    {
        m_ptr.set( links.m_list );
    }
    void baseSet( LINKSbase &links, START_AT_HEADversion )
    {
        m_ptr.set( links.m_list, START_AT_HEAD );
    }
    void baseSet( LINKSbase &links, START_AT_TAILversion )
    {
        m_ptr.set( links.m_list, START_AT_TAIL );
    }

    void *baseGet() const
    {
        return (*m_ptr).m_thatObj;
    }

public:

    int isInitialized() const
    {
        return m_ptr.isInitialized();
    }
    int isValid() const
    {
        return m_ptr.isValid();
    }
    int isAtHead() const
    {
        return m_ptr.isAtHead();
    }
    int isAtTail() const
    {
        return m_ptr.isAtTail();
    }

    void gotoPrev()
    {
        m_ptr.gotoPrev();
    }
    void gotoNext()
    {
        m_ptr.gotoNext();
    }
    void remove()
    {
        m_ptr.remove();
    }
};

/*-------------------------------------------------------------------*
 | LINKS_TO<> -- template for list of links
 *-------------------------------------------------------------------*/

template< class TYPE >
class LINKS_TO: public LINKSbase
{
public:

    LINKS_TO(): LINKSbase() {}

    TYPE *getHead()
    {
        return (TYPE *)baseGetHead();
    }
    TYPE *getTail()
    {
        return (TYPE *)baseGetTail();
    }
    TYPE &operator*()
    {
        return *getHead();
    }
};

/*-------------------------------------------------------------------*
 | PTR_INTO_LINKS_TO<> -- template for iterator over list of links
 *-------------------------------------------------------------------*/

template< class TYPE >
class PTR_INTO_LINKS_TO: public PTR_INTO_LINKSbase
{
public:

    PTR_INTO_LINKS_TO(): PTR_INTO_LINKSbase() {}
    PTR_INTO_LINKS_TO( LINKS_TO< TYPE > &links ):
        PTR_INTO_LINKSbase( links )
    {
    }
    PTR_INTO_LINKS_TO( LINKS_TO< TYPE > &links,
                       START_AT_HEADversion ):
        PTR_INTO_LINKSbase( links, START_AT_HEAD )
    {
    }
    PTR_INTO_LINKS_TO( LINKS_TO< TYPE > &links,
                       START_AT_TAILversion ):
        PTR_INTO_LINKSbase( links, START_AT_TAIL )
    {
    }

    void set( LINKS_TO< TYPE > &links )
    {
        baseSet( links );
    }
    void set( LINKS_TO< TYPE > &links, START_AT_HEADversion )
    {
        baseSet( links, START_AT_HEAD );
    }
    void set( LINKS_TO< TYPE > &links, START_AT_TAILversion )
    {
        baseSet( links, START_AT_TAIL );
    }

    PTR_INTO_LINKS_TO< TYPE > &operator=( LINKS_TO< TYPE > &links )
    {
        set( links );
        return *this;
    }
    PTR_INTO_LINKS_TO< TYPE >
    &operator=( const PTR_INTO_LINKS_TO< TYPE > &ptr )
    {
        *(PTR_INTO_LINKSbase *)this =
            *(const PTR_INTO_LINKSbase *)&ptr;
        return *this;
    }

    TYPE *get()
    {
        return (TYPE *)baseGet();
    }
    TYPE &operator*()
    {
        return *get();
    }

    PTR_INTO_LINKS_TO< TYPE > &operator--()
    {
        gotoPrev();
        return *this;
    }
    PTR_INTO_LINKS_TO< TYPE > &operator++()
    {
        gotoNext();
        return *this;
    }
};

/*-------------------------------------------------------------------*
 | Routines used in MAKE_LINK() macro
 |
 | XMakeLinkBase() is the base routine for creating a link between
 | two objects.  It takes as arguments, pointers to the two objects
 | and references to their LINKS_TO<> members.  All the arguments are
 | generic.
 |
 | The inline function template MakeLink() takes exactly the same
 | arguments as XMakeLinkBase(), and calls XMakeLinkBase(), but
 | enforces stricter type-checking (provided the compiler is good
 | enough).
 *-------------------------------------------------------------------*/

void XMakeLinkBase( void *obj0, LINKSbase &links0,
                    void *obj1, LINKSbase &links1 );

template< class TYPE0, class TYPE1 >
inline void MakeLink( TYPE0 *obj0, LINKS_TO< TYPE1 > &links0,
                      TYPE1 *obj1, LINKS_TO< TYPE0 > &links1 )
{
    XMakeLinkBase( obj0, links0, obj1, links1 );
}

/*-------------------------------------------------------------------*
 | Macros
 *-------------------------------------------------------------------*/

#define MAKE_LINK( obj0, linksMember0, obj1, linksMember1 ) \
  MakeLink( obj0, obj0->linksMember0, obj1, obj1->linksMember1 );

#define LOOP_LINKS( ptr, links )                                      \
  for( ptr.set( links, START_AT_HEAD ); ptr.isValid(); ++ptr )        \
 
#define LOOP_LINKSrev( ptr, links )                                   \
  for( ptr.set( links, START_AT_TAIL ); ptr.isValid(); --ptr )        \
 
#endif

