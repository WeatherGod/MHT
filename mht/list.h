
/*********************************************************************
 * FILE: list.H                                                      *
 *                                                                   *
 * AUTHOR: Matthew Miller (mlm)                                      *
 *                                                                   *
 * HISTORY:                                                          *
 *    3 JUN 93 -- (mlm) created                                      *
 *                                                                   *
 * CONTENTS:                                                         *
 *                                                                   *
 *   Classes, templates, and macros for doubly-linked lists.         *
 *                                                                   *
 *   There are three kinds of doubly-linked lists: store by value,   *
 *   store by address, and intrusive.  For the most part, they work  *
 *   the same way, but they store the data that they contain         *
 *   differently.                                                    *
 *                                                                   *
 *                         Store by value                            *
 *                                                                   *
 *   Store-by-value lists store copies of the objects that are       *
 *   placed on them.  There are no restrictions placed on the        *
 *   objects.                                                        *
 *                                                                   *
 *   This is the best kind of list to use for lists of basic data    *
 *   types like int or double.                                       *
 *                                                                   *
 *   They are created with the template vDLIST_OF<>.                 *
 *                                                                   *
 *                         Store by address                          *
 *                                                                   *
 *   Store-by-address lists store pointers to the objects that are   *
 *   placed on them.  There are no restrictions placed on the        *
 *   objects, but if an object on one of these lists is deleted or   *
 *   goes out of scope, the list will be left holding a garbage      *
 *   pointer.                                                        *
 *                                                                   *
 *   They are created with the template ptrDLIST_OF<>.               *
 *                                                                   *
 *                           Intrusive                               *
 *                                                                   *
 *   An intrusive list can only be used with objects that inherit    *
 *   from DLISTnode and contain the macro MEMBERS_FOR_DLISTnode()    *
 *   within their definitions.  Class definitions for classes of     *
 *   objects that will be placed on intrusive lists must look like   *
 *   this:                                                           *
 *                                                                   *
 *     class WHATEVER: public DLISTnode                              *
 *     {                                                             *
 *         .                                                         *
 *         .                                                         *
 *         .                                                         *
 *       protected:                                                  *
 *         MEMBERS_FOR_DLISTnode( WHATEVER )                         *
 *         .                                                         *
 *         .                                                         *
 *         .                                                         *
 *     };                                                            *
 *                                                                   *
 *   (Note that ther is no ";" after MEMBERS_FOR_DLISTnode().)       *
 *                                                                   *
 *   The list itself is created with the template iDLIST_OF<>.       *
 *                                                                   *
 *   When an object is put on an intrusive list, the list takes over *
 *   control of it completely.  If the list is copied, a copy will   *
 *   be made of the object.  If the list is deleted or goes out of   *
 *   scope, the object will be deleted too.                          *
 *                                                                   *
 *   All objects placed on intrusive lists MUST have been allocated  *
 *   with operator new().  The program is likely to crash otherwise. *
 *                                                                   *
 *   It is safe to delete an object that is on an intrusive list.    *
 *   The destructor for DLISTnode removes the object from the list   *
 *   properly.                                                       *
 *                                                                   *
 *   Any given object can only be on one intrusive list at a time.   *
 *                                                                   *
 *   An object on an intrusive list "knows" that it's on the list.   *
 *   It has a number of member functions (inherited from DLISTnode() *
 *   or defined in MEMBERS_FOR_DLISTnode()) that let it examine and  *
 *   modify the list that it is on.                                  *
 *                                                                   *
 *   Many of the below functions have "dangerous" versions that      *
 *   don't do any type or error checking.  The names of the          *
 *   dangerous versions are the same as their safe counterparts,     *
 *   except that they begin with "X".                                *
 *                                                                   *
 *   In the following description of these member functions, TYPE    *
 *   refers to the class of object that has been defined (the place  *
 *   held by "WHATEVER", above).  Except where otherwise noted, all  *
 *   the following functions are protected.                          *
 *                                                                   *
 *   int isNode() const                                              *
 *     This always returns 1.  Public.                               *
 *                                                                   *
 *   int isOnList() const                                            *
 *     This returns 1 if the object has been put onto an intrusive   *
 *     list.  0 if the object is not on a list.  Public.             *
 *                                                                   *
 *   int isHead() const                                              *
 *     Returns 1 if the object is the head object on a list.         *
 *     Otherwise 0.  Public.                                         *
 *                                                                   *
 *   int isTail() const                                              *
 *     Returns 1 if the object is the tail object on a list.         *
 *     Otherwise 0.  Public.                                         *
 *                                                                   *
 *   TYPE *getPrev() const                                           *
 *     Returns a pointer to the object that precedes this one on the *
 *     list.  This may only be used if the object is on a list, and  *
 *     it is not the head object.                                    *
 *                                                                   *
 *   TYPE *getNext() const                                           *
 *     Returns a pointer to the object that follows this one on the  *
 *     list.  This may only be used if the object is on a list, and  *
 *     it is not the tail object.                                    *
 *                                                                   *
 *   TYPE *getHead() const                                           *
 *     Returns a pointer to the head object of the list that this    *
 *     object is on.  This may only be used if the object is on a    *
 *     list.                                                         *
 *                                                                   *
 *   TYPE *getTail() const                                           *
 *     Returns a pointer to the tail object of the list that this    *
 *     object is on.  This may only be used if the object is on a    *
 *     list.                                                         *
 *                                                                   *
 *   void prepend( TYPE *node )                                      *
 *     Inserts node onto the list that this object is on,            *
 *     immediately preceding this object.  This may only be used if  *
 *     the object is on a list, and node is not on a list.           *
 *                                                                   *
 *   void append( TYPE *node )                                       *
 *     Inserts node onto the list that this object is on,            *
 *     immediately after this object.  This may only be used if      *
 *     the object is on a list, and node is not on a list.           *
 *                                                                   *
 *   void unlink()                                                   *
 *     Remove the object from the list that it's on.  This may be    *
 *     used at any time.  If the object is not on a list, it does    *
 *     nothing.                                                      *
 *                                                                   *
 *   TYPE *makeCopy() const                                          *
 *     Returns a pointer to a copy of this object.  This uses a      *
 *     virtual function to ensure that the copy is made properly,    *
 *     even if the object is a descendent of the class where the     *
 *     function is being called.                                     *
 *                                                                   *
 *   void check() const                                              *
 *     If DEBUG is defined, this checks the integrity of the whole   *
 *     list that contains the object.  If only TSTBUG is defined,    *
 *     this checks only the present object and its two neighbors.    *
 *     If neither is defined, this does nothing.                     *
 *                                                                   *
 *     An error is generated using THROW_ERR if check() finds        *
 *     anything wrong.                                               *
 *                                                                   *
 *     This function is public.                                      *
 *                                                                   *
 *   void checkOnList() const                                        *
 *   void checkNotOnList() const                                     *
 *   void checkIsNode() const                                        *
 *   void checkIsNotNode() const                                     *
 *   void checkIsHead() const                                        *
 *   void checkIsNotHead() const                                     *
 *   void checkIsTail() const                                        *
 *   void checkIsNotTail() const                                     *
 *     These check that the described condition is true of the       *
 *     object.  They generate an error with THROW_ERR if it isn't.   *
 *     (For example, checkOnList() generates an error if the object  *
 *     is not on a list).                                            *
 *                                                                   *
 *     These functions are public.                                   *
 *                                                                   *
 *                           xDLIST_OF<>                             *
 *                                                                   *
 *   Doubly linked lists are defined with the templates              *
 *                                                                   *
 *     vDLIST_OF< TYPE > -- store by value                           *
 *     ptrDLIST_OF< TYPE > -- store by address                       *
 *     iDLIST_OF< TYPE > -- intrusive                                *
 *                                                                   *
 *   For the most part, list objects declared with these templates   *
 *   have the same member functions, so they'll be described         *
 *   together here.  "xDLIST" will be used anywhere that all three   *
 *   kinds of lists are legal.                                       *
 *                                                                   *
 *   In the following, TYPE refers to the type of object stored in   *
 *   the list.                                                       *
 *                                                                   *
 *   xDLIST_OF< TYPE >()                                             *
 *     The constructor for lists take no arguments.                  *
 *                                                                   *
 *   xDLIST_OF< TYPE >( const xDLIST_OF< TYPE > &list )              *
 *     The copy constructors make deep copies.                       *
 *                                                                   *
 *   ~xDLIST_OF< TYPE >()                                            *
 *     The destructors call removeAll() (see below).                 *
 *                                                                   *
 *   xDLIST_OF< TYPE > &operator=( const xDLIST_OF< TYPE > &list )   *
 *     Assignment is overloaded so that it makes a call to           *
 *     removeAll() (see below), and then does a deep copy.           *
 *                                                                   *
 *   TYPE &operator*()                                               *
 *     This returns a reference to the head object.  The list must   *
 *     not be empty.                                                 *
 *                                                                   *
 *   int isEmpty() const                                             *
 *     Returns 1 if there are no objects on the list. 0 otherwise.   *
 *                                                                   *
 *   int hasOneMember() const                                        *
 *     Returns 1 if there is one and only one object on the list.    *
 *     0 otherwise.                                                  *
 *                                                                   *
 *   int getLength() const                                           *
 *     Returns the number of objects on the list.                    *
 *                                                                   *
 *   TYPE *getHead() const                                           *
 *     Returns a pointer to the head object.  The list must not be   *
 *     empty.                                                        *
 *                                                                   *
 *   TYPE *getTail() const                                           *
 *     Returns a pointer to the tail object.  The list must not be   *
 *     empty.                                                        *
 *                                                                   *
 *   void prepend( const TYPE &object ) -- for store-by-value lists  *
 *   void prepend( TYPE &object ) -- for store-by-address lists      *
 *   void prepend( TYPE *object ) -- for intrusive lists             *
 *     Put the given object onto the list at the beginning (it       *
 *     becomes the head object).                                     *
 *                                                                   *
 *   void append( const TYPE &object ) -- for store-by-value lists   *
 *   void append( TYPE &object ) -- for store-by-address lists       *
 *   void append( TYPE *object ) -- for intrusive lists              *
 *     Put the given object onto the list at the end (it becomes the *
 *     tail object).                                                 *
 *                                                                   *
 *   void prependCopy( const xDLIST_OF< TYPE > &list )               *
 *     Insert a copy of the given list onto the beginning of this    *
 *     list.  The kind of list given in the argument must match the  *
 *     kind of list this member function is being used on.           *
 *                                                                   *
 *   void appendCopy( const xDLIST_OF< TYPE > &list )                *
 *     Insert a copy of the given list onto the end of this          *
 *     list.  The kind of list given in the argument must match the  *
 *     kind of list this member function is being used on.           *
 *                                                                   *
 *   void splice( xDLIST_OF< TYPE > &list )                          *
 *     Take all the objects off the given list, and put them onto    *
 *     the end of this list.  This is much faster than appendCopy(), *
 *     but it leaves the given list empty.  The kind of list given   *
 *     must match the kind of list this member function is being     *
 *     used on.                                                      *
 *                                                                   *
 *   void removeHead()                                               *
 *     Removes the head object from the list.  If the list is        *
 *     store-by-value, then only the stored copy of the object is    *
 *     deleted.  If it is store-by-address, the object is left       *
 *     alone (only the record of its address is deleted).  If it     *
 *     is intrusive, the object itself is deleted.                   *
 *                                                                   *
 *     This function can only be used on non-empty lists.            *
 *                                                                   *
 *   void removeTail()                                               *
 *     Removes the tail object from the list.  If the list is        *
 *     store-by-value, then only the stored copy of the object is    *
 *     deleted.  If it is store-by-address, the object is left       *
 *     alone (only the record of its address is deleted).  If it     *
 *     is intrusive, the object itself is deleted.                   *
 *                                                                   *
 *     This function can only be used on non-empty lists.            *
 *                                                                   *
 *   void removeAll()                                                *
 *     Remove all the objects from the list.  If the list is         *
 *     store-by-value, then only the stored copies of the objects    *
 *     are deleted.  If it is store-by-address, then the objects     *
 *     are left alone (only the record of their addresses are        *
 *     deleted).  If it is intrusive, then the objects themselves    *
 *     are deleted.                                                  *
 *                                                                   *
 *     This function can be used at any time.                        *
 *                                                                   *
 *   void check() const                                              *
 *     If DEBUG is defined, this checks the integrity of the entire  *
 *     list.  If TSTBUG is defined, it only checks the links to the  *
 *     head and tail of the list.  If neither is defined, it does    *
 *     nothing.                                                      *
 *                                                                   *
 *     If this routine finds anything wrong, it generates an error   *
 *     with THROW_ERR.                                               *
 *                                                                   *
 *   void checkEmpty() const                                         *
 *   void checkNotEmpty() const                                      *
 *   void checkHasOneMember() const                                  *
 *   void checkNotHasOneMember() const                               *
 *     These check to see if the described property is true of the   *
 *     list.  If it isn't, they generate an error with THROW_ERR.    *
 *                                                                   *
 *                          Iterators                                *
 *                                                                   *
 *   The iterators for (or "pointers into") the different kinds of   *
 *   doubly-linked lists are defined with the following templates    *
 *                                                                   *
 *     PTR_INTO_vDLIST_OF< TYPE > -- pointer into store-by-value     *
 *                                   list                            *
 *     PTR_INTO_ptrDLIST_OF< TYPE > -- pointer into                  *
 *                                     store-by-address list         *
 *     PTR_INTO_iDLIST_OF< TYPE > -- pointer into intrusive list     *
 *                                                                   *
 *   At any given time, a PTR_INTO points either points at a node    *
 *   in a linked list (of the appropriate kind), or is invalid.      *
 *   There are three reasons it might be invalid:                    *
 *                                                                   *
 *     1. If a PTR_INTO is constructed without being initialized     *
 *        (i.e. no arguments are given to the constructor), then     *
 *        it starts out life invalid.                                *
 *                                                                   *
 *     2. If it pointed to a valid node, and then removed that       *
 *        node using its remove() member function (see below), then  *
 *        it becomes invalid.                                        *
 *                                                                   *
 *     3. If it points into a list, but has moved off either end     *
 *        of the it, or it points into an empty list, then it is     *
 *        invalid.                                                   *
 *                                                                   *
 *   PTR_INTO's that are invalid cannot be used to access objects,   *
 *   but they may be moved to valid positions (in cases 2 and 3),    *
 *   and they can be set to point into new lists (in all three       *
 *   cases).                                                         *
 *                                                                   *
 *   Most of the member functions for PTR_INTO's are the same for    *
 *   all three kinds of lists, so they're all described together     *
 *   here.  In the following, TYPE is the class of object contained  *
 *   in the lists.  "xDLIST" is used anywhere that all three kinds   *
 *   of lists are legal.                                             *
 *                                                                   *
 *   PTR_INTO_xDLIST_OF< TYPE >()                                    *
 *     The constructor with no arguments makes a PTR_INTO that is    *
 *     invalid.  It cannot be used for anything until it is set      *
 *     to point into a list, using set() or operator=() (see below). *
 *                                                                   *
 *   PTR_INTO_iDLIST_OF< TYPE >( TYPE *obj ) -- intrusive lists only *
 *     This constructs a PTR_INTO that points at the given object,   *
 *     in the list that the object is on.  The object must be on a   *
 *     list.                                                         *
 *                                                                   *
 *   PTR_INTO_xDLIST_OF< TYPE >( xDLIST_OF< TYPE > &list )           *
 *   PTR_INTO_xDLIST_OF< TYPE >( xDLIST_OF< TYPE > &list,            *
 *                               START_AT_HEAD )                     *
 *     Either of these constructs a PTR_INTO that points at the head *
 *     of the given list (or is invalid, if the given list is        *
 *     empty).                                                       *
 *                                                                   *
 *   PTR_INTO_xDLIST_OF< TYPE >( xDLIST_OF< TYPE > &list,            *
 *                               START_AT_TAIL )                     *
 *     If START_AT_TAIL is given as an argument to the constructor,  *
 *     then the PTR_INTO is initialized to point to the tail of the  *
 *     list instead of the head.                                     *
 *                                                                   *
 *   PTR_INTO_xDLIST_OF< TYPE >( const PTR_INTO_xDLIST_OF< TYPE > &p)*
 *     The copy constructor may only be used if the given PTR_INTO   *
 *     does not point to a node that has been removed (i.e. is not   *
 *     made invalid by method 2, above).                             *
 *                                                                   *
 *   void set( TYPE *obj ) -- intrusive lists only                   *
 *     Set the PTR_INTO to point to the given object, in the list    *
 *     that the object is on.  The object must be on a list.         *
 *                                                                   *
 *   void set( xDLIST_OF< TYPE > &list )                             *
 *   void set( xDLIST_OF< TYPE > &list, START_AT_HEAD )              *
 *   PTR_INTO_xDLIST_OF< TYPE > &operator=( xDLIST_OF< TYPE > &list )*
 *     Any of these sets the PTR_INTO to point at the head           *
 *     of the given list (or makes it invalid, if the given list is  *
 *     empty).                                                       *
 *                                                                   *
 *   void set( xDLIST_OF< TYPE > &list, START_AT_TAIL )              *
 *     If START_AT_TAIL is given as an argument to set,              *
 *     then the PTR_INTO is set to point to the tail of the          *
 *     list instead of the head.                                     *
 *                                                                   *
 *   void set( const PTR_INTO_xDLIST_OF< TYPE > &ptr )               *
 *   PTR_INTO_xDLIST_OF< TYPE >                                      *
 *       &operator=( const PTR_INTO_xDLIST_OF< TYPE > &ptr )         *
 *     These copy the given PTR_INTO.  They may only be used if the  *
 *     given PTR_INTO does not point to a node that has been         *
 *     removed (i.e. has not been made invalid by method 2, above).  *
 *                                                                   *
 *   int isInitialized() const                                       *
 *     Returns 1 if the object pointed to is on a list. 0 otherwise. *
 *                                                                   *
 *   int isValid() const                                             *
 *     Returns 1 if the PTR_INTO is valid.  0 otherwise.             *
 *                                                                   *
 *   int isRemoved() const                                           *
 *     Returns 1 if the PTR_INTO has been made invalid by methods    *
 *     1 or 2 above.  0 if it is valid, or has been made invalid     *
 *     by method 3.                                                  *
 *                                                                   *
 *   int isAtHead() const                                            *
 *     Returns 1 if the object pointed to is the head of a list.     *
 *     0 otherwise.                                                  *
 *                                                                   *
 *   int isAtTail() const                                            *
 *     Returns 1 if the object pointed to is the tail of a list.     *
 *     0 otherwise.                                                  *
 *                                                                   *
 *   int isEqualTo( const TYPE *obj ) -- intrusive lists only        *
 *   int operator==( const TYPE *obj ) -- intrusive lists only       *
 *     These return 1 if the PTR_INTO points to the given object.    *
 *     0 otherwise.                                                  *
 *                                                                   *
 *   int operator!=( const TYPE *obj ) -- intrusive lists only       *
 *     Return 1 if the PTR_INTO doesn't point to the given object.   *
 *     0 otherwise.                                                  *
 *                                                                   *
 *   int isEqualTo( const PTR_INTO_xDLIST_OF< TYPE > &ptr )          *
 *   int operator==( const PTR_INTO_xDLIST_OF< TYPE > &ptr )         *
 *     Return 1 if the PTR_INTO points to the same object as ptr.    *
 *     0 otherwise.                                                  *
 *                                                                   *
 *   int operator!=( const PTR_INTO_xDLIST_OF< TYPE > &ptr )         *
 *     Return 1 if the PTR_INTO doesn't point to the same object     *
 *     as ptr.  0 otherwise                                          *
 *                                                                   *
 *   void prepend( const TYPE &obj ) -- store-by-value lists         *
 *   void prepend( TYPE &obj ) -- store-by-address lists             *
 *   void prepend( TYPE *obj ) -- intrusive lists                    *
 *     Insert the given object into the list before the object that  *
 *     the PTR_INTO points to.  If the PTR_INTO is invalid, the      *
 *     behavior of this routine depends on the method by which it    *
 *     became invalid.  Refering to the methods above:               *
 *                                                                   *
 *       Invalid by method 1 or 2; use of this routine is illegal.   *
 *                                                                   *
 *       Invalid by method 3; the object is inserted onto the tail   *
 *       of the list.                                                *
 *                                                                   *
 *   void append( const TYPE &obj ) -- store-by-value lists          *
 *   void append( TYPE &obj ) -- store-by-address lists              *
 *   void append( TYPE *obj ) -- intrusive lists                     *
 *     Insert the given object into the list after the object that   *
 *     the PTR_INTO points to.  If the PTR_INTO is invalid, the      *
 *     behavior of this routine depends on the method by which it    *
 *     became invalid.  Refering to the methods above:               *
 *                                                                   *
 *       Invalid by method 1 or 2; use of this routine is illegal.   *
 *                                                                   *
 *       Invalid by method 3; the object is inserted onto the tail   *
 *       of the list.                                                *
 *                                                                   *
 *   TYPE *get() const                                               *
 *     Returns a pointer to the object that the PTR_INTO points to.  *
 *     This may only be used if the PTR_INTO is valid.               *
 *                                                                   *
 *   TYPE &operator*() const                                         *
 *     Returns a reference to the object that the PTR_INTO points    *
 *     to.  This may only be used if the PTR_INTO is valid.          *
 *                                                                   *
 *   void gotoPrev()                                                 *
 *   void operator--()                                               *
 *     Move the pointer to the previous object in the list.  If the  *
 *     PTR_INTO is invalid by method 1, this does nothing.  If it is *
 *     invalid by method 2, this behaves as if the object hadn't     *
 *     been removed.  If it is invalid by method 3, this moves the   *
 *     PTR_INTO to the tail of the list.                             *
 *                                                                   *
 *   void gotoNext()                                                 *
 *   void operator++()                                               *
 *     Move the pointer to the next object in the list.  If the      *
 *     PTR_INTO is invalid by method 1, this does nothing.  If it is *
 *     invalid by method 2, this behaves as if the object hadn't     *
 *     been removed.  If it is invalid by method 3, this moves the   *
 *     PTR_INTO to the head of the list.                             *
 *                                                                   *
 *   void gotoHead()                                                 *
 *     Move the pointer to the head of the list.  If the PTR_INTO    *
 *     is invalid by method 1, this may not be used.                 *
 *                                                                   *
 *   void gotoTail()                                                 *
 *     Move the pointer to the tail of the list.  If the PTR_INTO    *
 *     is invalid by method 1, this may not be used.                 *
 *                                                                   *
 *   void remove()                                                   *
 *     Remove the object that the PTR_INTO points to.  The PTR_INTO  *
 *     must be valid.                                                *
 *                                                                   *
 *     In store-by-value lists, only the copy of the object is       *
 *     actually deleted by this.  In store-by-address, only the      *
 *     record of the object's address is deleted, not the object     *
 *     itself.  In intrusive lists, the object itself is deleted.    *
 *                                                                   *
 *     When a PTR_INTO removes an object, it first copies that       *
 *     object's link information into a dummy object.  That link     *
 *     information may then be used by the various goto...()         *
 *     routines.  This means that, for example, the following        *
 *     loop will work:                                               *
 *                                                                   *
 *       for( p = list; p.isValid(); ++p )                           *
 *         if( (*p).isNotWanted() )                                  *
 *           p.remove();                                             *
 *                                                                   *
 *     (assuming p is a PTR_INTO, list is a DLIST, and the objects   *
 *     on the DLIST have a member function called isNotWanted())     *
 *                                                                   *
 *   void checkInitialized() const                                   *
 *   void checkNotInitialized() const                                *
 *   void checkValid() const                                         *
 *   void checkNotValid() const                                      *
 *   void checkRemoved() const                                       *
 *   void checkNotRemoved() const                                    *
 *   void checkAtHead() const                                        *
 *   void checkNotAtHead() const                                     *
 *   void checkAtTail() const                                        *
 *   void checkNotAtTail() const                                     *
 *     These check to see if the described property is true of the   *
 *     PTR_INTO.  If it isn't, they generate an error with           *
 *     THROW_ERR.                                                    *
 *                                                                   *
 *                     LOOP_DLIST() and LOOP_DLISTrev()              *
 *                                                                   *
 *   Two macros are provided for making common kinds of loops.       *
 *                                                                   *
 *   LOOP_DLIST()                                                    *
 *     This macro takes two arguments: a PTR_INTO and a DLIST.  The  *
 *     two arguments must both be for the same kind of list.         *
 *     It expands to a "for" loop, so the next line of code, or      *
 *     group of lines within curly brackets, will be executed a      *
 *     number of times.                                              *
 *                                                                   *
 *     The for-loop goes through the given list from head to tail.   *
 *     The given PTR_INTO points to the current object in the list   *
 *     in each iteration.                                            *
 *                                                                   *
 *     For example:                                                  *
 *                                                                   *
 *       iDLIST_OF< int > intList;                                   *
 *       PTR_INTO_iDLIST_OF< int > ptr;                              *
 *         .                                                         *
 *         .                                                         *
 *         .                                                         *
 *       LOOP_DLIST( ptr, intList )                                  *
 *       {                                                           *
 *         cout << *ptr << endl;                                     *
 *       }                                                           *
 *                                                                   *
 *     This prints out the contents of a list of integers.           *
 *                                                                   *
 *   LOOP_DLISTrev()                                                 *
 *     This is almost identical to LOOP_DLIST(), except that it goes *
 *     from tail to head instead of head to tail.                    *
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

#ifndef LIST_H
#define LIST_H

#include "except.h"
#include <assert.h>

/*-------------------------------------------------------------------*
 | Declarations of stuff found in this file.
 *-------------------------------------------------------------------*/

class DLISTnode;
class dummyDLISTnode;
class DLISTbase;
class PTR_INTO_DLISTbase;
template< class TYPE > class vDLIST_NODE_OF;
template< class TYPE > class vDLIST_OF;
template< class TYPE > class PTR_INTO_vDLIST_OF;
template< class TYPE > class ptrDLIST_NODE_OF;
template< class TYPE > class ptrDLIST_OF;
template< class TYPE > class PTR_INTO_ptrDLIST_OF;
template< class TYPE > class iDLIST_OF;
template< class TYPE > class PTR_INTO_iDLIST_OF;

/*-------------------------------------------------------------------*
 | Declarations and macros used to make different versions of
 | PTR_INTO constructors and PTR_INTO::set() member functions
 *-------------------------------------------------------------------*/

class START_AT_HEADdummy;
#define START_AT_HEAD ((START_AT_HEADdummy *)0)
#define START_AT_HEADversion START_AT_HEADdummy *

class START_AT_TAILdummy;
#define START_AT_TAIL ((START_AT_TAILdummy *)0)
#define START_AT_TAILversion START_AT_TAILdummy *

/*-------------------------------------------------------------------*
 | DLISTnode -- base class for linked-list nodes
 *-------------------------------------------------------------------*/

class DLISTnode
{
    friend class DLISTbase;
    friend class PTR_INTO_DLISTbase;

private:

    DLISTnode *m_prev;
    DLISTnode *m_next;

protected:

    DLISTnode(): m_prev( this ), m_next( this ) {}
    DLISTnode( const DLISTnode & ): m_prev( this ), m_next( this ) {}

    virtual ~DLISTnode()
    {
        unlink();
    }

    DLISTnode *XgetPrev() const
    {
        return m_prev;
    }
    DLISTnode *XgetNext() const
    {
        return m_next;
    }
    DLISTnode *XgetHead() const;
    DLISTnode *XgetTail() const;

    void Xprepend( DLISTnode *node )
    {
        node->checkIsNode();
        node->checkNotOnList();

        node->m_prev = m_prev;
        node->m_next = this;
        node->m_prev->m_next = node->m_next->m_prev = node;

        check();
    }

    void Xappend( DLISTnode *node )
    {
        node->checkNotOnList();

        node->m_prev = this;
        node->m_next = m_next;
        node->m_prev->m_next = node->m_next->m_prev = node;

        check();
    }

    void unlink()
    {
        m_prev->m_next = m_next;
        m_next->m_prev = m_prev;

        m_prev->check();
        m_next->check();

        m_prev = m_next = this;
    }

    virtual DLISTnode *XmakeCopy() const = 0;

public:

    virtual int isNode() const
    {
        return 1;
    }
    int isOnList() const
    {
        return m_next != this;
    }
    int isHead() const
    {
        return isOnList() && ! m_prev->isNode();
    }
    int isTail() const
    {
        return isOnList() && ! m_next->isNode();
    }

    void Xreset()
    {
        m_prev = m_next = this;
    }
    void XcopyLinks( const DLISTnode &src )
    {
        m_prev = src.m_prev;
        m_next = src.m_next;
    }

#ifdef DEBUG
    void check() const;
#elif defined( TSTBUG )
    void check() const
    {
        assert( m_prev->m_next == this && m_next->m_prev == this );
        //  THROW_ERR( "Corrupted DLIST" );
    }
#else
    void check() const {}
#endif

#ifdef TSTBUG
    // NOTE: The boolean statements for the assertions have been negated from their original in their if-statements
    void checkOnList() const
    {
        assert( isOnList() );
    }// THROW_ERR( "Node must be on dlist" ); }
    void checkNotOnList() const
    {
        assert( ! isOnList() );
    }// THROW_ERR( "Node mustn't be on dlist" ); }
    void checkIsNode() const
    {
        assert( isNode() );
    }// THROW_ERR( "Must be a real node" ); }
    void checkIsNotNode() const
    {
        assert( ! isNode() );
    }// THROW_ERR( "Mustn't be a node" ); }
    void checkIsHead() const
    {
        assert( isHead() );
    }// THROW_ERR( "Previous must be node" ); }
    void checkIsNotHead() const
    {
        assert( ! isHead() );
    }// THROW_ERR( "Previous mustn't be node" ); }
    void checkIsTail() const
    {
        assert( isTail() );
    }// THROW_ERR( "Next must be node" ); }
    void checkIsNotTail() const
    {
        assert( ! isTail() );
    }// THROW_ERR( "Next mustn't be node" ); }

#else
    void checkOnList() const {}
    void checkNotOnList() const {}
    void checkIsNode() const {}
    void checkIsNotNode() const {}
    void checkIsHead() const {}
    void checkIsNotHead() const {}
    void checkIsTail() const {}
    void checkIsNotTail() const {}
#endif
};

/*-------------------------------------------------------------------*
 | dummyDLISTnode -- dummy node class
 |
 | DLISTbase and PTR_INTO_DLISTbase both contain member objects of
 | this class.
 |
 | The one in DLISTbase is used to point to the head and tail of the
 | list.  It is, in fact, a node on a circular version of the list,
 | marking the beginning and end of the list by returning 0 from
 | its isNode() member.
 |
 | The one in PTR_INTO_DLISTbase is used to hold the linkage info
 | that was in objects which have been remove()'ed.  It is also used
 | to give the PTR_INTO something to point to when it isn't valid.
 | This actually saves alot of time, since we don't have to go
 | checking for validity every time we move the pointer.
 |
 | dummyDLISTnode is intended ONLY for use in these two object
 | classes.
 *-------------------------------------------------------------------*/

class dummyDLISTnode: public DLISTnode
{
    friend class DLISTbase;
    friend class PTR_INTO_DLISTbase;

protected:

    virtual DLISTnode *XmakeCopy() const
    {
        return new dummyDLISTnode( *this );
    }

public:

    virtual int isNode() const
    {
        return 0;
    }
};

/*-------------------------------------------------------------------*
 | MEMBERS_FOR_DLISTnode() -- macro that declares those members of
 |                            a DLIST node which can't be inherited
 |
 | If we could inherit from instantiations of templates (my compiler
 | can't), then most of these members could be handled differently.
 | But overloaded operator=, and members that require knowledge of
 | the object's size, can't be inherited anyway.
 *-------------------------------------------------------------------*/

#define MEMBERS_FOR_DLISTnode( TYPE )                                 \
  TYPE *getPrev() const                                               \
    {checkIsHead(); return (TYPE *)XgetPrev(); }                 \
  TYPE *getNext() const                                               \
    {checkIsTail(); return (TYPE *)XgetNext(); }                 \
  TYPE *getHead() const                                               \
    {checkOnList(); return (TYPE *)XgetHead(); }                 \
  TYPE *getTail() const                                               \
    {checkOnList(); return (TYPE *)XgetTail(); }                 \
  void prepend( TYPE *node )                                          \
    {checkOnList(); Xprepend( node ); }                          \
  void append( TYPE *node )                                           \
    {checkOnList(); Xappend( node ); }                           \
  TYPE *makeCopy() const                                              \
    {return (TYPE *)XmakeCopy(); }                               \
  virtual DLISTnode *XmakeCopy() const                                \
    {return new TYPE( *this ); }                                 \
 
/*-------------------------------------------------------------------*
 | DLISTbase -- base class for all doubly-linked lists
 *-------------------------------------------------------------------*/

class DLISTbase
{
    friend class PTR_INTO_DLISTbase;

private:

    dummyDLISTnode m_vnode;          // see comments for
    //   dummyDLISTnode, above

protected:

    DLISTbase(): m_vnode() {}
    DLISTbase( const DLISTbase &dlist ): m_vnode()
    {
        XappendCopy( dlist );
    }

    virtual ~DLISTbase()
    {
        removeAll();
    }

public:

    DLISTnode *XgetHead() const
    {
        return m_vnode.XgetNext();
    }
    DLISTnode *XgetTail() const
    {
        return m_vnode.XgetPrev();
    }
    void Xprepend( DLISTnode *node )
    {
        m_vnode.Xappend( node );
    }
    void Xappend( DLISTnode *node )
    {
        m_vnode.Xprepend( node );
    }
    void XprependCopy( const DLISTbase &dlist );
    void XappendCopy( const DLISTbase &dlist );
    void Xsplice( DLISTbase &dlist );

public:

    int isEmpty() const
    {
        return ! XgetHead()->isNode();
    }
    int hasOneMember() const
    {
        return ! isEmpty() && XgetHead() == XgetTail();
    }

    int getLength() const;

    void removeHead()
    {
        checkNotEmpty();
        delete XgetHead();
    }
    void removeTail()
    {
        checkNotEmpty();
        delete XgetTail();
    }
    void removeAll();

    void Xreset()
    {
        m_vnode.Xreset();
    }
    void XcopyLinks( const DLISTbase &src )
    {
        m_vnode.XcopyLinks( src.m_vnode );
    }

    void check() const
    {
        m_vnode.check();
    }

#ifdef TSTBUG
    // NOTE: Booleans for assertions have been negated from their originals for the if-statements
    void checkEmpty() const
    {
        assert( isEmpty() );
    }// THROW_ERR( "Dlist must be empty" ); }
    void checkNotEmpty() const
    {
        assert( ! isEmpty() );
    }// THROW_ERR( "Dlist mustn't be empty" ); }

    void checkHasOneMember() const
    {
        assert( hasOneMember() );
        //THROW_ERR( "Dlist must have one and only one member" );
    }

    void checkNotHasOneMember() const
    {
        assert( ! hasOneMember() );
        //  THROW_ERR( "Dlist mustn't have one and only one member" );
    }

#else
    void checkEmpty() const {}
    void checkNotEmpty() const {}
    void checkHasOneMember() const {}
    void checkNotHasOneMember() const {}
#endif
};

/*-------------------------------------------------------------------*
 | MEMBERS_FOR_DLISTbase() -- macro that declares those members of
 |                            a DLIST which can't be inherited
 |
 | If we could inherit from instantiations of templates (my compiler
 | can't), then most of these members could be handled differently.
 | But overloaded operator=, and members that require knowledge of
 | the object's size, can't be inherited anyway.
 *-------------------------------------------------------------------*/

#define MEMBERS_FOR_DLISTbase( DLISTtmplt, TYPE )                     \
  DLISTtmplt(): DLISTbase() {}                                        \
  DLISTtmplt( const DLISTtmplt< TYPE > &dlist ):                      \
    DLISTbase( dlist ) {}                                             \
  void prependCopy( const DLISTtmplt< TYPE > &dlist )                 \
    {XprependCopy( dlist ); }                                    \
  void appendCopy( const DLISTtmplt< TYPE > &dlist )                  \
    {XappendCopy( dlist ); }                                     \
  void splice( DLISTtmplt< TYPE > &dlist )                            \
    {Xsplice( dlist ); }                                         \
  DLISTtmplt< TYPE > &operator=( const DLISTtmplt< TYPE > &dlist )    \
    {removeAll(); XappendCopy( dlist ); return *this; }          \
 
/*-------------------------------------------------------------------*
 | PTR_INTO_DLISTbase -- base class for all PTR_INTO_xDLIST's
 *-------------------------------------------------------------------*/

class PTR_INTO_DLISTbase
{
private:

    dummyDLISTnode m_dummy;          // see comments for
    //   dummyDLISTnode, above
    DLISTnode *m_ptr;

protected:

    PTR_INTO_DLISTbase(): m_dummy(), m_ptr( &m_dummy ) {}

    PTR_INTO_DLISTbase( DLISTnode *ptr ):
        m_dummy(),
        m_ptr( ptr )
    {
    }

    PTR_INTO_DLISTbase( DLISTbase &dlist ):
        m_dummy(),
        m_ptr( dlist.XgetHead() )
    {
    }

    PTR_INTO_DLISTbase( DLISTbase &dlist, START_AT_HEADversion ):
        m_dummy(),
        m_ptr( dlist.XgetHead() )
    {
    }

    PTR_INTO_DLISTbase( DLISTbase &dlist, START_AT_TAILversion ):
        m_dummy(),
        m_ptr( dlist.XgetTail() )
    {
    }

    PTR_INTO_DLISTbase( const PTR_INTO_DLISTbase &ptr ):
        m_dummy(),
        m_ptr( ptr.m_ptr )
    {
        ptr.checkNotRemoved();
    }

    virtual ~PTR_INTO_DLISTbase()
    {
        m_dummy.Xreset();
    }

public:

    void Xset( DLISTnode *ptr )
    {
        m_ptr = ptr;
    }
    void Xset( DLISTbase &dlist )
    {
        m_ptr = dlist.XgetHead();
    }
    void Xset( DLISTbase &dlist, START_AT_HEADversion )
    {
        m_ptr = dlist.XgetHead();
    }
    void Xset( DLISTbase &dlist, START_AT_TAILversion )
    {
        m_ptr = dlist.XgetTail();
    }
    void Xset( const PTR_INTO_DLISTbase &ptr )
    {
        m_ptr = ptr.m_ptr;
    }

    void Xprepend( DLISTnode *node )
    {
        m_ptr->Xprepend( node );
    }
    void Xappend( DLISTnode *node )
    {
        m_ptr->Xappend( node );
    }

    DLISTnode *Xget() const
    {
        return m_ptr;
    }

public:

    int isInitialized() const
    {
        return m_ptr->isOnList();
    }
    int isValid() const
    {
        return m_ptr->isNode();
    }
    int isRemoved() const
    {
        return m_ptr == &m_dummy;
    }
    int isAtHead() const
    {
        return m_ptr->isHead();
    }
    int isAtTail() const
    {
        return m_ptr->isTail();
    }

    int isEqualTo( const DLISTnode *ptr ) const
    {
        return m_ptr == ptr;
    }
    int isEqualTo( const PTR_INTO_DLISTbase &ptr ) const
    {
        return m_ptr == ptr.m_ptr;
    }

    void gotoPrev()
    {
        m_ptr = m_ptr->XgetPrev();
    }
    void gotoNext()
    {
        m_ptr = m_ptr->XgetNext();
    }
    void gotoHead()
    {
        m_ptr = m_ptr->XgetHead();
    }
    void gotoTail()
    {
        m_ptr = m_ptr->XgetTail();
    }

    void remove()
    {

        checkValid();

        m_dummy.XcopyLinks( *m_ptr );
        delete m_ptr;
        m_ptr = &m_dummy;
    }

#ifdef TSTBUG
    // NOTE: Assertion booleans have been negated from their original if-statements.
    void checkInitialized() const
    {
        assert( isInitialized() );
        //  THROW_ERR( "Pointer into dlist must be initialized" );
    }

    void checkNotInitialized() const
    {
        assert( ! isInitialized() );
        //  THROW_ERR( "Pointer into dlist mustn't be initialized" );
    }

    void checkValid() const
    {
        assert( isValid() );
        //  THROW_ERR( "Pointer into dlist must be valid" );
    }

    void checkNotValid() const
    {
        assert( ! isValid() );
        //  THROW_ERR( "Pointer into dlist must not be valid" );
    }

    void checkRemoved() const
    {
        assert( isRemoved() );
        //  THROW_ERR( "Pointer into dlist must point to removed node" );
    }

    void checkNotRemoved() const
    {
        assert( ! isRemoved() );
        //  THROW_ERR( "Pointer into dlist "
        //             "mustn't point to removed node" );
    }

    void checkAtHead() const
    {
        assert( isAtHead() );
        //  THROW_ERR( "Pointer into dlist must be at head" );
    }

    void checkNotAtHead() const
    {
        // Odd... this was originally identical to checkAtHead()...
        assert( ! isAtHead() );
        //  THROW_ERR( "Pointer into dlist mustn't be at head" );
    }

    void checkAtTail() const
    {
        assert( isAtTail() );
        //  THROW_ERR( "Pointer into dlist must be at tail" );
    }

    void checkNotAtTail() const
    {
        // Odd... this was originally identical to checkNotAtTail()
        assert( ! isAtTail() );
        //  THROW_ERR( "Pointer into dlist mustn't be at tail" );
    }

#else
    void checkInitialized() const {}
    void checkNotInitialized() const {}
    void checkValid() const {}
    void checkNotValid() const {}
    void checkRemoved() const {}
    void checkNotRemoved() const {}
    void checkAtHead() const {}
    void checkNotAtHead() const {}
    void checkAtTail() const {}
    void checkNotAtTail() const {}
#endif
};

/*-------------------------------------------------------------------*
 | MEMBERS_FOR_PTR_INTO_DLISTbase() -- macro that declares those
 |                                     members of a PTR_INTO a
 |                                     DLIST which can't be inherited
 |
 | If we could inherit from instantiations of templates (my compiler
 | can't), then most of these members could be handled differently.
 | But overloaded operator=, and members that require knowledge of
 | the object's size, can't be inherited anyway.
 *-------------------------------------------------------------------*/

#define MEMBERS_FOR_PTR_INTO_DLISTbase( PTRtmplt, DLISTtmplt, TYPE )  \
  PTRtmplt(): PTR_INTO_DLISTbase() {}                                 \
  PTRtmplt( DLISTtmplt< TYPE > &dlist ):                              \
    PTR_INTO_DLISTbase( dlist ) {}                                    \
  PTRtmplt( DLISTtmplt< TYPE > &dlist, START_AT_HEADversion ):        \
    PTR_INTO_DLISTbase( dlist, START_AT_HEAD ) {}                     \
  PTRtmplt( DLISTtmplt< TYPE > &dlist, START_AT_TAILversion ):        \
    PTR_INTO_DLISTbase( dlist, START_AT_TAIL ) {}                     \
  PTRtmplt( const PTRtmplt< TYPE > &ptr ):                            \
    PTR_INTO_DLISTbase( ptr ) {}                                      \
  void set( DLISTtmplt< TYPE > &dlist )                               \
    {Xset( dlist ); }                                            \
  void set( DLISTtmplt< TYPE > &dlist, START_AT_HEADversion )         \
    {Xset( dlist, START_AT_HEAD ); }                             \
  void set( DLISTtmplt< TYPE > &dlist, START_AT_TAILversion )         \
    {Xset( dlist, START_AT_TAIL ); }                             \
  void set( const PTRtmplt< TYPE > &ptr )                             \
    {ptr.checkNotRemoved(); Xset( ptr ); }                       \
  PTRtmplt< TYPE > &operator=( DLISTtmplt< TYPE > &dlist )            \
    {set( dlist ); return *this; }                               \
  PTRtmplt< TYPE > &operator=( const PTRtmplt< TYPE > &ptr )          \
    {set( ptr ); return *this; }                                 \
  PTRtmplt< TYPE > &operator++()                                      \
    {gotoNext(); return *this; }                                 \
  PTRtmplt< TYPE > &operator--()                                      \
    {gotoPrev(); return *this; }                                 \
  int operator==( const PTRtmplt< TYPE > ptr ) const                  \
    {return isEqualTo( ptr ); }                                  \
  int operator!=( const PTRtmplt< TYPE > ptr ) const                  \
    {return ! isEqualTo( ptr ); }                                \
 
/*-------------------------------------------------------------------*
 | Templates for store-by-value doubly-linked lists
 *-------------------------------------------------------------------*/

template< class TYPE >
class vDLIST_NODE_OF: public DLISTnode
{
    friend class vDLIST_OF< TYPE >;
    friend class PTR_INTO_vDLIST_OF< TYPE >;

private:

    TYPE m_info;

private:

    vDLIST_NODE_OF( const TYPE &info ):
        DLISTnode(),
        m_info( info )
    {
    }

protected:

    MEMBERS_FOR_DLISTnode( vDLIST_NODE_OF< TYPE > )
};

template< class TYPE >
class vDLIST_OF: public DLISTbase
{
public:

    MEMBERS_FOR_DLISTbase( vDLIST_OF, TYPE )

    TYPE *getHead() const
    {
        checkNotEmpty();
        return &((vDLIST_NODE_OF< TYPE > *)XgetHead())->m_info;
    }

    TYPE *getTail() const
    {
        checkNotEmpty();
        return &((vDLIST_NODE_OF< TYPE > *)XgetTail())->m_info;
    }

    void prepend( const TYPE &node )
    {
        Xprepend( new vDLIST_NODE_OF< TYPE >( node ) );
    }
    void append( const TYPE &node )
    {
        Xappend( new vDLIST_NODE_OF< TYPE >( node ) );
    }
    TYPE &operator*() const
    {
        return *getHead();
    }
};

template< class TYPE >
class PTR_INTO_vDLIST_OF: public PTR_INTO_DLISTbase
{
public:

    MEMBERS_FOR_PTR_INTO_DLISTbase( PTR_INTO_vDLIST_OF,
                                    vDLIST_OF,
                                    TYPE )

    TYPE *get() const
    {
        checkValid();
        return &((vDLIST_NODE_OF< TYPE > *)Xget())->m_info;
    }

    void prepend( const TYPE &node )
    {
        checkNotRemoved();
        Xprepend( new vDLIST_NODE_OF< TYPE >( node ) );
    }

    void append( const TYPE &node )
    {
        checkNotRemoved();
        Xappend( new vDLIST_NODE_OF< TYPE >( node ) );
    }

    TYPE &operator*() const
    {
        return *get();
    }
};

/*-------------------------------------------------------------------*
 | Templates for store-by-address doubly-linked lists
 *-------------------------------------------------------------------*/

template< class TYPE >
class ptrDLIST_NODE_OF: public DLISTnode
{
    friend class ptrDLIST_OF< TYPE >;
    friend class PTR_INTO_ptrDLIST_OF< TYPE >;

private:

    TYPE *m_info;

private:

    ptrDLIST_NODE_OF( TYPE &info ):
        DLISTnode(),
        m_info( &info )
    {
    }

protected:

    MEMBERS_FOR_DLISTnode( ptrDLIST_NODE_OF< TYPE > )
};

template< class TYPE >
class ptrDLIST_OF: public DLISTbase
{
public:

    MEMBERS_FOR_DLISTbase( ptrDLIST_OF, TYPE )

    TYPE *getHead() const
    {
        checkNotEmpty();
        return ((ptrDLIST_NODE_OF< TYPE > *)XgetHead())->m_info;
    }

    TYPE *getTail() const
    {
        checkNotEmpty();
        return ((ptrDLIST_NODE_OF< TYPE > *)XgetTail())->m_info;
    }

    void prepend( TYPE &node )
    {
        Xprepend( new ptrDLIST_NODE_OF< TYPE >( node ) );
    }
    void append( TYPE &node )
    {
        Xappend( new ptrDLIST_NODE_OF< TYPE >( node ) );
    }
    TYPE &operator*() const
    {
        return *getHead();
    }
};

template< class TYPE >
class PTR_INTO_ptrDLIST_OF: public PTR_INTO_DLISTbase
{
public:

    MEMBERS_FOR_PTR_INTO_DLISTbase( PTR_INTO_ptrDLIST_OF,
                                    ptrDLIST_OF,
                                    TYPE )

    TYPE *get() const
    {
        checkValid();
        return ((ptrDLIST_NODE_OF< TYPE > *)Xget())->m_info;
    }

    void prepend( TYPE &node )
    {
        checkNotRemoved();
        Xprepend( new ptrDLIST_NODE_OF< TYPE >( node ) );
    }

    void append( TYPE &node )
    {
        checkNotRemoved();
        Xappend( new ptrDLIST_NODE_OF< TYPE >( node ) );
    }

    TYPE &operator*() const
    {
        return *get();
    }
};

/*-------------------------------------------------------------------*
 | Templates for intrusive doubly-linked lists
 *-------------------------------------------------------------------*/

template< class TYPE >
class iDLIST_OF: public DLISTbase
{
public:

    MEMBERS_FOR_DLISTbase( iDLIST_OF, TYPE )

    TYPE *getHead() const
    {
        checkNotEmpty();
        return (TYPE *)XgetHead();
    }
    TYPE *getTail() const
    {
        checkNotEmpty();
        return (TYPE *)XgetTail();
    }
    void prepend( TYPE *node )
    {
        Xprepend( node );
    }
    void append( TYPE *node )
    {
        Xappend( node );
    }
    TYPE &operator*() const
    {
        return *getHead();
    }
};

template< class TYPE >
class PTR_INTO_iDLIST_OF: public PTR_INTO_DLISTbase
{
public:

    MEMBERS_FOR_PTR_INTO_DLISTbase( PTR_INTO_iDLIST_OF,
                                    iDLIST_OF,
                                    TYPE )

    PTR_INTO_iDLIST_OF( TYPE *node ):
        PTR_INTO_DLISTbase( node )
    {
        node->checkOnList();
    }

    operator TYPE*()
    {
        return get();
    }

    TYPE *get() const
    {
        checkValid();
        return (TYPE *)Xget();
    }
    void set( TYPE *ptr )
    {
        Xset( ptr );
    }
    void prepend( TYPE *node )
    {
        checkNotRemoved();
        Xprepend( node );
    }
    void append( TYPE *node )
    {
        checkNotRemoved();
        Xappend( node );
    }
    TYPE &operator*() const
    {
        return *get();
    }
    int operator==( const TYPE *ptr ) const
    {
        return isEqualTo( ptr );
    }
    int operator!=( const TYPE *ptr ) const
    {
        return ! isEqualTo( ptr );
    }
};

/*-------------------------------------------------------------------*
 | Looping macros
 *-------------------------------------------------------------------*/

#define LOOP_DLIST( ptr, dlist )                                      \
  for( ptr.set( dlist, START_AT_HEAD ); ptr.isValid(); ++ptr )        \
 
#define LOOP_DLISTrev( ptr, dlist )                                   \
  for( ptr.set( dlist, START_AT_TAIL ); ptr.isValid(); --ptr )        \
 
#endif

