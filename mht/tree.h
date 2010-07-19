/*********************************************************************
 * FILE: tree.H                                                      *
 *                                                                   *
 * AUTHOR: Matthew Miller (mlm)                                      *
 *                                                                   *
 * HISTORY:                                                          *
 *   13 JUL 93 -- (mlm) created                                      *
 *                                                                   *
 * CONTENTS:                                                         *
 *                                                                   *
 *   Classes, templates, and macros for arbitrary tree structures.   *
 *   By "arbitrary", I mean that each node may have any number of    *
 *   children.                                                       *
 *                                                                   *
 *   There are three kinds of tree structures: store by value,       *
 *   store by address, and intrusive.  For the most part, they work  *
 *   the same way, but they store the data that they contain         *
 *   differently.                                                    *
 *                                                                   *
 *                         Store by value                            *
 *                                                                   *
 *   Store-by-value trees store copies of the objects that are       *
 *   placed on them.  There are no restrictions placed on the        *
 *   objects.                                                        *
 *                                                                   *
 *   This is the best kind of tree to use for trees of basic data    *
 *   types like int or double.                                       *
 *                                                                   *
 *   They are created with the template vTREE_OF<>.                  *
 *                                                                   *
 *                         Store by address                          *
 *                                                                   *
 *   Store-by-address trees store pointers to the objects that are   *
 *   placed on them.  There are no restrictions placed on the        *
 *   objects, but if an object on one of these trees is deleted or   *
 *   goes out of scope, the tree will be left holding a garbage      *
 *   pointer.                                                        *
 *                                                                   *
 *   They are created with the template ptrTREE_OF<>.                *
 *                                                                   *
 *                           Intrusive                               *
 *                                                                   *
 *   An intrusive tree can only be used with objects that inherit    *
 *   from TREEnode and contain the macro MEMBERS_FOR_TREEnode()      *
 *   within their definitions.  Class definitions for classes of     *
 *   objects that will be placed on intrusive trees must look like   *
 *   this:                                                           *
 *                                                                   *
 *     class WHATEVER: public TREEnode                               *
 *     {                                                             *
 *         .                                                         *
 *         .                                                         *
 *         .                                                         *
 *       protected:                                                  *
 *         MEMBERS_FOR_TREEnode( WHATEVER )                          *
 *         .                                                         *
 *         .                                                         *
 *         .                                                         *
 *     };                                                            *
 *                                                                   *
 *   (Note that ther is no ";" after MEMBERS_FOR_TREEnode().)        *
 *                                                                   *
 *   The tree itself is created with the template iTREE_OF<>.        *
 *                                                                   *
 *   When an object is put on an intrusive tree, the tree takes over *
 *   control of it completely.  If the tree is copied, a copy will   *
 *   be made of the object.  If the tree is deleted or goes out of   *
 *   scope, the object will be deleted too.                          *
 *                                                                   *
 *   All objects placed on intrusive trees MUST have been allocated  *
 *   with operator new().  The program is likely to crash otherwise. *
 *                                                                   *
 *   It is safe to delete an object that is on an intrusive tree.    *
 *   The destructor for TREEnode removes the object from the tree    *
 *   properly.                                                       *
 *                                                                   *
 *   Any given object can only be on one intrusive tree at a time.   *
 *                                                                   *
 *   An object on an intrusive tree "knows" that it's on the tree.   *
 *   It has a number of member functions (inherited from TREEnode()  *
 *   or defined in MEMBERS_FOR_TREEnode()) that let it examine and   *
 *   modify the tree that it is on.                                  *
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
 *     This always returns 1.  Public.  (Inherited from DLISTnode.)  *
 *                                                                   *
 *   int isOnTree() const                                            *
 *     This returns 1 if the object has been put onto an intrusive   *
 *     tree.  0 if the object is not on a tree.  Public.             *
 *                                                                   *
 *   int isRoot() const                                              *
 *     Returns 1 if the object is the root object on a tree.         *
 *     Otherwise 0.  Public.                                         *
 *                                                                   *
 *   int isLastSibling() const                                       *
 *     Returns 1 if the object is the last of a group of siblings.   *
 *     Otherwise 0.  Public.                                         *
 *                                                                   *
 *   int isFirstSibling() const                                      *
 *     Returns 1 if the object is the first of a group of siblings.  *
 *     Otherwise 0.  Public.                                         *
 *                                                                   *
 *   int hasOneChild() const                                         *
 *     Returns 1 if the object has exactly one child.  Otherwise 0.  *
 *     Public.                                                       *
 *                                                                   *
 *   int getNumChildren() const                                      *
 *     Returns the number of children that the object has.  Public.  *
 *                                                                   *
 *   int getDepth() const                                            *
 *     Returns the length of the path from the root of the tree to   *
 *     this object.  For example, if the object's parent is the      *
 *     root, getDepth() returns 1.  If the object's grandparent is   *
 *     the root, getDepth() returns 2.  Etc.                         *
 *     Public.                                                       *
 *                                                                   *
 *   TYPE *getParent() const                                         *
 *     Returns a pointer to the parent of this object.  This may     *
 *     only be used if the object is on a tree, and is not the root. *
 *                                                                   *
 *   TYPE *getPrevSibling() const                                    *
 *     Get the object that precedes this one in its parent's list of *
 *     children.  This may only be used if the object is on a tree,  *
 *     and is not the first of the siblings.                         *
 *                                                                   *
 *   TYPE *getNextSibling() const                                    *
 *     Get the object that follows this one in its parent's list of  *
 *     children.  This may only be used if the object is on a tree,  *
 *     and is not the last of the siblings.                          *
 *                                                                   *
 *   TYPE *getFirstChild() const                                     *
 *     Get the first of this object's children.  This may only be    *
 *     used if the object is on a tree, and is not a leaf.           *
 *                                                                   *
 *   TYPE *getLastChild() const                                      *
 *     Get the last of this object's children.  This may only be     *
 *     used if the object is on a tree, and is not a leaf.           *
 *                                                                   *
 *   TYPE *getFirstLeaf() const                                      *
 *     Get the first leaf that is a descendent of this object.  This *
 *     may only be used if the object is on a tree.  (If the object  *
 *     is itself a leaf, then getFirstLeaf() returns a pointer to    *
 *     this object.)                                                 *
 *                                                                   *
 *   TYPE *getLastLeaf() const                                       *
 *     Get the last leaf that is a descendent of this object.  This  *
 *     may only be used if the object is on a tree.  (If the object  *
 *     is itself a leaf, then getFirstLeaf() returns a pointer to    *
 *     this object.)                                                 *
 *                                                                   *
 *   TYPE *getRoot() const                                           *
 *     Get the root of the tree that this object is on.  This may    *
 *     only be used if the object is on a tree.                      *
 *                                                                   *
 *   TYPE *getPreOrderNext() const                                   *
 *     Get the next object that is found in the tree by progressing  *
 *     in "pre-order".  See LOOP_TREE(), below, for a discussion of  *
 *     this ordering.  This may only be used if the object is on a   *
 *     tree.                                                         *
 *                                                                   *
 *   TYPE *getPostOrderNext() const                                  *
 *     Get the next object that is found in the tree by progressing  *
 *     in "post-order".  See LOOP_TREEpostOrder(), below, for a      *
 *     discussion of this ordering.  This may only be used if the    *
 *     object is on a tree.                                          *
 *                                                                   *
 *   void insertParent( TYPE *node )                                 *
 *     Insert "node" into the tree as the parent of this object.     *
 *     This may only be used if the object is on a tree.             *
 *                                                                   *
 *   void insertPrevSibling( TYPE *node )                            *
 *     Insert "node" into the tree as the previous sibling of this   *
 *     object.  This may only be used if the object is on a tree.    *
 *                                                                   *
 *   void insertNextSibling( TYPE *node )                            *
 *     Insert "node" into the tree as the next sibling of this       *
 *     object.  This may only be used if the object is on a tree.    *
 *                                                                   *
 *   void insertFirstChild( TYPE *node )                             *
 *     Insert "node" into the tree as the first child of this        *
 *     object.  This may only be used if the object is on a tree.    *
 *                                                                   *
 *   void insertLastChild( TYPE *node )                              *
 *     Insert "node" into the tree as the last child of this         *
 *     object.  This may only be used if the object is on a tree.    *
 *                                                                   *
 *   TYPE *makeCopy() const                                          *
 *     Returns a pointer to a copy of this object.  This uses a      *
 *     virtual function to ensure that the copy is made properly,    *
 *     even if the object is a descendent of the class where the     *
 *     function is being called.                                     *
 *                                                                   *
 *   void check() const                                              *
 *     If DEBUG is defined, this checks the integrity of the whole   *
 *     tree that contains the object.  If only TSTBUG is defined,    *
 *     this checks only the present object.                          *
 *     If neither is defined, this does nothing.                     *
 *                                                                   *
 *     An error is generated using THROW_ERR if check() finds        *
 *     anything wrong.                                               *
 *                                                                   *
 *     This function is public.                                      *
 *                                                                   *
 *   void checkIsNode() const                                        *
 *   void checkIsNotNode() const                                     *
 *   void checkOnTree() const                                        *
 *   void checkNotOnTree() const                                     *
 *   void checkIsLeaf() const                                        *
 *   void checkNotLeaf() const                                       *
 *   void checkIsRoot() const                                        *
 *   void checkNotRoot() const                                       *
 *   void checkIsFirstSibling() const                                *
 *   void checkNotFirstSibline() const                               *
 *   void checkIsLastSibling() const                                 *
 *   void checkNotLastSibling() const                                *
 *   void checkHasOneChild() const                                   *
 *   void checkNotHasOneChild() const                                *
 *     These check that the described condition is true of the       *
 *     object.  They generate an error with THROW_ERR if it isn't.   *
 *     (For example, checkOnTree() generates an error if the object  *
 *     is not on a tree).                                            *
 *                                                                   *
 *     These functions are public.                                   *
 *                                                                   *
 *                           xTREE_OF<>                              *
 *                                                                   *
 *   Trees are defined with the templates                            *
 *                                                                   *
 *     vTREE_OF< TYPE > -- store by value                            *
 *     ptrTREE_OF< TYPE > -- store by address                        *
 *     iTREE_OF< TYPE > -- intrusive                                 *
 *                                                                   *
 *   For the most part, tree objects declared with these templates   *
 *   have the same member functions, so they'll be described         *
 *   together here.  "xTREE" will be used anywhere that all three    *
 *   kinds of trees are legal.                                       *
 *                                                                   *
 *   In the following, TYPE refers to the type of object stored in   *
 *   the tree.                                                       *
 *                                                                   *
 *   xTREE_OF< TYPE >()                                              *
 *     The constructors for trees take no arguments.                 *
 *                                                                   *
 *   xTREE_OF< TYPE >( const xTREE_OF< TYPE > &tree )                *
 *     The copy constructors for trees make deep copies.             *
 *                                                                   *
 *   ~xTREE_OF< TYPE >()                                             *
 *     The destructor behaves the same way as                        *
 *                                                                   *
 *   xTREE_OF< TYPE > &operator=( const xTREE_OF< TYPE > &tree )     *
 *     Assignment is overloaded so that it makes a call to           *
 *     removeAll() (see below), and then does a deep copy.           *
 *                                                                   *
 *   TYPE &operator*() const                                         *
 *     This returns a reference to the root object.  The tree must   *
 *     not be empty.                                                 *
 *                                                                   *
 *   int isEmpty() const                                             *
 *     Returns 1 if there are no objects on the tree.  0 otherwise.  *
 *                                                                   *
 *   int getHeight() const                                           *
 *     Returns the depth of the first leaf in the tree.              *
 *                                                                   *
 *   TYPE *getRoot() const                                           *
 *     Returns a pointer to the root object.  The tree must not be   *
 *     empty.                                                        *
 *                                                                   *
 *   TYPE *getFirstLeaf() const                                      *
 *     Returns a pointer to the first leaf object in the tree.  The  *
 *     tree must not be empty.                                       *
 *                                                                   *
 *   TYPE *getLastLeaf() const                                       *
 *     Returns a pointer to the last leaf object in the tree.  The   *
 *     tree must not be empty.                                       *
 *                                                                   *
 *   void insertRoot( const TYPE &object ) -- in store-by-value tree *
 *   void insertRoot( TYPE &object ) -- in store-by-address tree     *
 *   void insertRoot( TYPE *object ) -- in intrusive tree            *
 *     Put the given object in the root of the tree.  Make the       *
 *     current root into the given object's child.                   *
 *                                                                   *
 *   void removeRoot()                                               *
 *     Remove the root of the tree.  The new root will be the old    *
 *     root's child.  The old root must have no more than one child. *
 *                                                                   *
 *   void removeAll()                                                *
 *     Empty the tree.                                               *
 *                                                                   *
 *   void check() const                                              *
 *     If DEBUG is defined, this checks the whole tree for internal  *
 *     consistancy.  If TSTBUG is defined, it only checks the links  *
 *     of the root node.  If neither is defined, it does nothing.    *
 *                                                                   *
 *     If this routine finds anything wrong, it generates an error   *
 *     with THROW_ERR.                                               *
 *                                                                   *
 *   void checkEmpty() const                                         *
 *   void checkNotEmpty() const                                      *
 *     These check to see if the described property is true of the   *
 *     tree.  If it isn't, they generate an error with THROW_ERR.    *
 *                                                                   *
 *                          Iterators                                *
 *                                                                   *
 *   The iterators for (or "pointers into") the different kinds of   *
 *   trees are defined with the following templates                  *
 *                                                                   *
 *     PTR_INTO_vTREE_OF< TYPE > -- pointer into store-by-value tree *
 *     PTR_INTO_ptrTREE_OF< TYPE > -- pointer into store-by-address  *
 *                                    tree                           *
 *     PTR_INTO_iTREE_OF< TYPE > -- pointer into intrusive tree      *
 *                                                                   *
 *   At any given time, a PTR_INTO either points at a node in a tree *
 *   (of the appropriate kind), or is invalid.  There are three      *
 *   reasons it might be invalid:                                    *
 *                                                                   *
 *     1. If a PTR_INTO is constructed without being initialized     *
 *        (i.e. no arguments given to the constructor), then         *
 *        it starts out life invalid.                                *
 *                                                                   *
 *     2. If it pointed to a valid node, and then removed that       *
 *        node using its remove() member function (see below), then  *
 *        it becomes invalid.                                        *
 *                                                                   *
 *     3. If it points into a tree, but has moved off it somehow     *
 *        (gone to the parent of the root, the first child of a      *
 *        leaf, the next sibling of a last sibling, etc.), then it   *
 *        is invalid.                                                *
 *                                                                   *
 *   PTR_INTO's that are invalid cannot be used to access objects,   *
 *   but they may be moved to valid positions (in cases 2 and 3),    *
 *   and they can be set to point into new trees (in all three       *
 *   cases).                                                         *
 *                                                                   *
 *   Most of the member functions for PTR_INTO's are the same for    *
 *   all three kinds of trees, so they're all described together     *
 *   here.  In the following, TYPE is the class of object contained  *
 *   in the trees.  "xTREE" is used anywhere that all three kinds    *
 *   of trees are legal.                                             *
 *                                                                   *
 *   PTR_INTO_xTREE_OF< TYPE >()                                     *
 *     The constructor with no arguments makes a PTR_INTO that is    *
 *     invalid.  It cannot be used for anything until it is set      *
 *     to point into a tree, using set() or operator=() (see below). *
 *                                                                   *
 *   PTR_INTO_iTREE_OF< TYPE >( TYPE *obj ) -- intrusive trees only  *
 *     This constructs a PTR_INTO that points at the given object,   *
 *     in the tree that the object is on.  The object must be on a   *
 *     tree.                                                         *
 *                                                                   *
 *   PTR_INTO_xTREE_OF< TYPE >( xTREE_OF< TYPE > &tree )             *
 *   PTR_INTO_xTREE_OF< TYPE >( xTREE_OF< TYPE > &tree,              *
 *                              START_AT_ROOT )                      *
 *     Either of these constructs a PTR_INTO that points at the root *
 *     of the given tree (or is invalid, if the given tree is        *
 *     empty).                                                       *
 *                                                                   *
 *   PTR_INTO_xTREE_OF< TYPE >( xTREE_OF< TYPE > &tree,              *
 *                              START_AT_FIRST_LEAF )                *
 *     This constructs a PTR_INTO that points at the first leaf of   *
 *     the given tree (or is invalid, if the given tree is empty).   *
 *                                                                   *
 *   PTR_INTO_xTREE_OF< TYPE >( xTREE_OF< TYPE > &tree,              *
 *                              START_AT_LAST_LEAF )                 *
 *     This constructs a PTR_INTO that points at the last leaf of    *
 *     the given tree (or is invalid, if the given tree is empty).   *
 *                                                                   *
 *   PTR_INTO_xTREE_OF< TYPE >( const PTR_INTO_xTREE_OF< TYPE > &p ) *
 *     The copy constructor may only be used if the given PTR_INTO   *
 *     does not point to a node that has been removed (i.e. is not   *
 *     made invalid by method 2, above).                             *
 *                                                                   *
 *   void set( TYPE *obj ) -- intrusive trees only                   *
 *     Set the PTR_INTO to point to the given object, in the tree    *
 *     that the object is on.  The object must be on a tree.         *
 *                                                                   *
 *   void set( xTREE_OF< TYPE > &tree )                              *
 *   void set( xTREE_OF< TYPE > &tree, START_AT_ROOT )               *
 *   PTR_INTO_xTREE_OF< TYPE > &operator=( xTREE_OF< TYPE > &tree )  *
 *     Any of these sets the PTR_INTO to point at the root of the    *
 *     given tree (or makes it invalid, if the given tree is empty). *
 *                                                                   *
 *   void set( xTREE_OF< TYPE > &tree, START_AT_FIRST_LEAF )         *
 *     This sets the PTR_INTO to point to the first leaf of the      *
 *     tree (or makes it invalid, if the given tree is empty).       *
 *                                                                   *
 *   void set( xTREE_OF< TYPE > &tree, START_AT_LAST_LEAF )          *
 *     This sets the PTR_INTO to point to the last leaf of the       *
 *     tree (or makes it invalid, if the given tree is empty).       *
 *                                                                   *
 *   void set( const PTR_INTO_xTREE_OF< TYPE > &p )                  *
 *   PTR_INTO_xTREE_OF< TYPE >                                       *
 *       &operator=( const PTR_INTO_xTREE_OF< TYPE > &p )            *
 *     These copy the given PTR_INTO.  They may only be used if the  *
 *     given PTR_INTO does not point to a node that has been         *
 *     removed (i.e. has not been made invalid by method 2, above).  *
 *                                                                   *
 *   int isInitialized() const                                       *
 *     Returns 1 if the object pointed to is on a tree. 0 otherwise. *
 *                                                                   *
 *   int isValid() const                                             *
 *     Returns 1 if the PTR_INTO is valid.  0 otherwise.             *
 *                                                                   *
 *   int isRemoved() const                                           *
 *     Returns 1 if the PTR_INTO has been made invalid by methods    *
 *     1 or 2 above.  0 if it is valid, or has been made invalid     *
 *     by method 3.                                                  *
 *                                                                   *
 *   int isAtRoot() const                                            *
 *     Returns 1 if the PTR_INTO points to the root of a tree.       *
 *     0 otherwise.                                                  *
 *                                                                   *
 *   int isAtFirstSibling() const                                    *
 *     Returns 1 if the node pointed to has no previous sibling.     *
 *     0 otherwise.                                                  *
 *                                                                   *
 *   int isAtLastSibling() const                                     *
 *     Returns 1 if the node pointed to has no next sibling.         *
 *     0 otherwise.                                                  *
 *                                                                   *
 *   int isAtLeaf() const                                            *
 *     Returns 1 if the node pointed to has no children.             *
 *     0 otherwise.                                                  *
 *                                                                   *
 *   int isEqualTo( const TYPE *obj ) -- intrusive trees only        *
 *   int operator==( const TYPE *obj ) -- intrusive trees only       *
 *     These return 1 if the PTR_INTO points to the given object.    *
 *     0 otherwise.                                                  *
 *                                                                   *
 *   int operator!=( const TYPE *obj ) -- intrusive trees only       *
 *     Returns 1 if the PTR_INTO doesn't point to the given object.  *
 *     0 otherwise.                                                  *
 *                                                                   *
 *   int isEqualTo( const PTR_INTO_xTREE_OF< TYPE > &p )             *
 *   int operator==( const PTR_INTO_xTREE_OF< TYPE > &p )            *
 *     These return 1 if the PTR_INTO points to the same object as   *
 *     does p.  0 otherwise.                                         *
 *                                                                   *
 *   int operator!=( const PTR_INTO_xTREE_OF< TYPE > &p )            *
 *     Returns 1 if the PTR_INTO doesn't point to the same object as *
 *     does p.  0 otherwise.                                         *
 *                                                                   *
 *   void insertParent( const TYPE &obj ) -- store-by-value trees    *
 *   void insertParent( TYPE &obj ) -- store-by-address trees        *
 *   void insertParent( TYPE *obj ) -- intrusive trees               *
 *     Insert the given object into the tree as the parent of the    *
 *     object that the PTR_INTO points to.  The PTR_INTO must be     *
 *     valid.                                                        *
 *                                                                   *
 *   void insertPrevSibling( const TYPE &obj ) -- store-by-value     *
 *   void insertPrevSibling( TYPE &obj ) -- store-by-address         *
 *   void insertPrevSibling( TYPE *obj ) -- intrusive                *
 *     Insert the given object into the tree as the previous         *
 *     sibling of the object that the PTR_INTO points to.  The       *
 *     PTR_INTO must be valid.                                       *
 *                                                                   *
 *   void insertNextSibling( const TYPE &obj ) -- store-by-value     *
 *   void insertNextSibling( TYPE &obj ) -- store-by-address         *
 *   void insertNextSibling( TYPE *obj ) -- intrusive                *
 *     Insert the given object into the tree as the next             *
 *     sibling of the object that the PTR_INTO points to.  The       *
 *     PTR_INTO must be valid.                                       *
 *                                                                   *
 *   void insertFirstChild( const TYPE &obj ) -- store-by-value      *
 *   void insertFirstChild( TYPE &obj ) -- store-by-address          *
 *   void insertFirstChild( TYPE *obj ) -- intrusive                 *
 *     Insert the given object into the tree as the first child      *
 *     of the object that the PTR_INTO points to.  The PTR_INTO must *
 *     be valid.                                                     *
 *                                                                   *
 *   void insertLastChild( const TYPE &obj ) -- store-by-value       *
 *   void insertLastChild( TYPE &obj ) -- store-by-address           *
 *   void insertLastChild( TYPE *obj ) -- intrusive                  *
 *     Insert the given object into the tree as the last child       *
 *     of the object that the PTR_INTO points to.  The PTR_INTO must *
 *     be valid.                                                     *
 *                                                                   *
 *   TYPE *get() const                                               *
 *     Returns a pointer to the object that the PTR_INTO points to.  *
 *     This may only be used if the PTR_INTO is valid.               *
 *                                                                   *
 *   TYPE &operator*() const                                         *
 *     Returns a reference to the object that the PTR_INTO points    *
 *     to.  This may only be used if the PTR_INTO is valid.          *
 *                                                                   *
 *   int getDepth() const                                            *
 *     Returns the number of nodes between the node that the         *
 *     PTR_INTO points to and the root.  The PTR_INTO must be valid. *
 *                                                                   *
 *   void gotoParent()                                               *
 *     Move the pointer to the parent of the object it currently     *
 *     points to.  If the PTR_INTO is invalid by method 1, this      *
 *     does nothing.  If it is invalid by method 2, this behaves as  *
 *     if the object hadn't been removed.  If it is invalid by       *
 *     method 3, this does nothing (see BUGS, below).                *
 *                                                                   *
 *   void gotoPrevSibling()                                          *
 *   PTR_INTO_xTREE_OF< TYPE > &operator--()                         *
 *     Move the pointer to the previous sibling of the object it     *
 *     currently points to.  If the PTR_INTO has been made invalid   *
 *     by a call to either gotoPrevSibling() or gotoNextSibling(),   *
 *     then this will go to the last sibling.  If it has been made   *
 *     invalid by any other call to a goto...() routine (method 3),  *
 *     then this will do nothing.  If it has been made invalid by    *
 *     method 2, this behaves as if the object hadn't been removed.  *
 *                                                                   *
 *   void gotoNextSibling()                                          *
 *   PTR_INTO_xTREE_OF< TYPE > &operator++()                         *
 *     Move the pointer to the next sibling of the object it         *
 *     currently points to.  If the PTR_INTO has been made invalid   *
 *     by a call to either gotoPrevSibling() or gotoNextSibling(),   *
 *     then this will go to the first sibling.  If it has been made  *
 *     invalid by any other call to a goto...() routine (method 3),  *
 *     then this will do nothing.  If it has been made invalid by    *
 *     method 2, this behaves as if the object hadn't been removed.  *
 *                                                                   *
 *   void gotoFirstChild()                                           *
 *     Move the pointer to the first child of the object it          *
 *     currently points to.  If the PTR_INTO has been made invalid   *
 *     by method 1 this does nothing.  If it is invalid by method 2, *
 *     this behaves as though the object hadn't been removed.  If    *
 *     it is invalid by method 3, this does nothing                  *
 *     (see BUGS below).                                             *
 *                                                                   *
 *   void gotoLastChild()                                            *
 *     Move the pointer to the last child of the object it           *
 *     currently points to.  If the PTR_INTO has been made invalid   *
 *     by method 1 this does nothing.  If it is invalid by method 2, *
 *     this behaves as though the object hadn't been removed.  If    *
 *     it is invalid by method 3, this does nothing                  *
 *     (see BUGS below).                                             *
 *                                                                   *
 *   void gotoPreOrderNext()                                         *
 *     See LOOP_TREE(), below, for a definition of pre-order.        *
 *     Other behavior of this routine is like the above goto...()    *
 *     routines.                                                     *
 *                                                                   *
 *   void gotoPostOrderNext()                                        *
 *     See LOOP_TREEpostOrder(), for a definition of post-order.     *
 *     Other behavior of this routine is like the above goto...()    *
 *     routines.                                                     *
 *                                                                   *
 *   void removeSubtree()                                            *
 *     Remove the node pointed to, along with all its children (and  *
 *     children's children, etc. etc.)  The PTR_INTO must be valid.  *
 *     It will be left invalid by method 2.                          *
 *                                                                   *
 *   void checkInitialized() const                                   *
 *   void checkNotInitialized() const                                *
 *   void checkValid() const                                         *
 *   void checkNotValid() const                                      *
 *   void checkRemoved() const                                       *
 *   void checkNotRemoved() const                                    *
 *   void checkIsAtRoot() const                                      *
 *   void checkNotAtRoot() const                                     *
 *   void checkIsAtFirstSibling() const                              *
 *   void checkNotAtFirstSibling() const                             *
 *   void checkIsAtLastSibling() const                               *
 *   void checkNotAtLastSibling() const                              *
 *   void checkIsAtLeaf() const                                      *
 *   void checkNotAtLeaf() const                                     *
 *     These check to see if the described property is true of the   *
 *     PTR_INTO.  If it isn't, they generate an error with           *
 *     THROW_ERR.                                                    *
 *                                                                   *
 *                         LOOP MACROS                               *
 *                                                                   *
 *   A set of macros are provided for common types of loops.         *
 *                                                                   *
 *   LOOP_TREE()                                                     *
 *     This macro takes two arguments: a PTR_INTO and a TREE.  The   *
 *     two arguments must both be for the same kind of tree.  It     *
 *     expands to a "for" loop, so the next line of code, or group   *
 *     of lines with curly brackets, will be executed a number of    *
 *     times.                                                        *
 *                                                                   *
 *     The for-loop goes through the given tree in "pre-order", in   *
 *     which each node is visited BEFORE its children are visited.   *
 *     This ordering can be illustrated as:                          *
 *                                                                   *
 *                       +---+                                       *
 *                       | 1 |                                       *
 *                       +---+                                       *
 *                       /   \                                       *
 *                      /     \                                      *
 *                  +---+     +---+                                  *
 *                  | 2 |     | 5 |                                  *
 *                  +---+     +---+                                  *
 *                  /  |       |  \                                  *
 *                 /   |       |   \                                 *
 *             +---+ +---+   +---+ +---+                             *
 *             | 3 | | 4 |   | 6 | | 7 |                             *
 *             +---+ +---+   +---+ +---+                             *
 *                                                                   *
 *   LOOP_TREEpostOrder()                                            *
 *     This macro takes two arguments: a PTR_INTO and a TREE.  The   *
 *     two arguments must both be for the same kind of tree.  It     *
 *     expands to a "for" loop, so the next line of code, or group   *
 *     of lines with curly brackets, will be executed a number of    *
 *     times.                                                        *
 *                                                                   *
 *     The for-loop goes through the given tree in "post-order", in  *
 *     which each node is visited AFTER its children are visited.    *
 *     This ordering can be illustrated as:                          *
 *                                                                   *
 *                       +---+                                       *
 *                       | 7 |                                       *
 *                       +---+                                       *
 *                       /   \                                       *
 *                      /     \                                      *
 *                  +---+     +---+                                  *
 *                  | 3 |     | 6 |                                  *
 *                  +---+     +---+                                  *
 *                  /  |       |  \                                  *
 *                 /   |       |   \                                 *
 *             +---+ +---+   +---+ +---+                             *
 *             | 1 | | 2 |   | 4 | | 5 |                             *
 *             +---+ +---+   +---+ +---+                             *
 *                                                                   *
 *   LOOP_TREEchildren()                                             *
 *     This macro takes two arguments.  The first is a PTR_INTO.     *
 *     The second is either a PTR_INTO of the same type, or, if the  *
 *     tree is intrusive, a pointer to an object.  The macro expands *
 *     to a for-loop that goes through the children of the second    *
 *     argument, from the first child to the last.                   *
 *                                                                   *
 *   LOOP_TREEchildrenRev()                                          *
 *     This is the same as LOOP_TREEchildren() except that it goes   *
 *     from the last child to the first.                             *
 *                                                                   *
 * BUGS:                                                             *
 *                                                                   *
 *   1. If a PTR_INTO becomes invalid by means of a call to          *
 *      gotoNextSibling(), gotoPrevSibling(), operator++(),          *
 *      operator--(), gotoFirstChild(), or gotoLastChild(),          *
 *      then a subsequent call to gotoParent(), gotFirstChild(), or  *
 *      gotoLastChild() will cause undefined results (e.g. it will   *
 *      crash).                                                      *
 *                                                                   *
 *   2. Due to the (perhaps too clever) object hierarchy used for    *
 *      my node objects, there may be some extraneous inherited      *
 *      junk that can be called by TREEnode objects (inherited from  *
 *      DLISTnode).                                                  *
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

#ifndef TREE_H
#define TREE_H

#include "except.h"
#include "list.h"
#include <assert.h>

/*-------------------------------------------------------------------*
 | Declarations of stuff found in this file.
 *-------------------------------------------------------------------*/

class TREEnode;
class dummyTREEnode;
class TREEbase;
class PTR_INTO_TREEbase;
template< class TYPE > class vTREE_NODE_OF;
template< class TYPE > class vTREE_OF;
template< class TYPE > class PTR_INTO_vTREE_OF;
template< class TYPE > class ptrTREE_NODE_OF;
template< class TYPE > class ptrTREE_OF;
template< class TYPE > class PTR_INTO_ptrTREE_OF;
template< class TYPE > class iTREE_OF;
template< class TYPE > class PTR_INTO_iTREE_OF;

/*-------------------------------------------------------------------*
 | Declarations and macros used to make different versions of
 | PTR_INTO constructors and PTR_INTO::set() member functions
 *-------------------------------------------------------------------*/

class START_AT_ROOTdummy;
#define START_AT_ROOT ((START_AT_ROOTdummy *)0)
#define START_AT_ROOTversion START_AT_ROOTdummy *

class START_AT_FIRST_LEAFdummy;
#define START_AT_FIRST_LEAF ((START_AT_FIRST_LEAFdummy *)0)
#define START_AT_FIRST_LEAFversion START_AT_FIRST_LEAFdummy *

class START_AT_LAST_LEAFdummy;
#define START_AT_LAST_LEAF ((START_AT_LAST_LEAFdummy *)0)
#define START_AT_LAST_LEAFversion START_AT_LAST_LEAFdummy *

/*-------------------------------------------------------------------*
 | TREEnode -- base class for tree nodes
 *-------------------------------------------------------------------*/

class TREEnode: public DLISTnode
{
    friend class TREEbase;
    friend class PTR_INTO_TREEbase;

private:

    TREEnode *m_parent;
    iDLIST_OF< TREEnode > m_childList;

protected:

    TREEnode(): DLISTnode(), m_parent( this ), m_childList() {}

    TREEnode( const TREEnode &node ):
        DLISTnode( node ),
        m_parent( this ),
        m_childList()
    {
    }

    virtual ~TREEnode() {}

    TREEnode *XgetParent() const
    {
        return m_parent;
    }
    TREEnode *XgetPrevSibling() const
    {
        return (TREEnode *)XgetPrev();
    }
    TREEnode *XgetNextSibling() const
    {
        return (TREEnode *)XgetNext();
    }
    TREEnode *XgetFirstChild() const
    {
        return (TREEnode *)m_childList.XgetHead();
    }
    TREEnode *XgetLastChild() const
    {
        return (TREEnode *)m_childList.XgetTail();
    }
    TREEnode *XgetFirstLeaf() const;
    TREEnode *XgetLastLeaf() const;
    TREEnode *XgetRoot() const;
    TREEnode *XgetPreOrderNext() const;
    TREEnode *XgetPostOrderNext() const;

    void XinsertParent( TREEnode *node );

    void XinsertPrevSibling( TREEnode *node )
    {
        node->m_parent = m_parent;
        Xprepend( node );
        check();
    }

    void XinsertNextSibling( TREEnode *node )
    {
        node->m_parent = m_parent;
        Xappend( node );
        check();
    }

    void XinsertFirstChild( TREEnode *node )
    {
        node->m_parent = this;
        m_childList.prepend( node );
        check();
    }

    void XinsertLastChild( TREEnode *node )
    {
        node->m_parent = this;
        m_childList.append( node );
        check();
    }

public:

    int isOnTree() const
    {
        return m_parent != this;
    }
    int isLeaf() const
    {
        return m_childList.isEmpty();
    }
    int isRoot() const
    {
        return isOnTree() && ! m_parent->isNode();
    }
    int isLastSibling() const
    {
        return isTail();
    }
    int isFirstSibling() const
    {
        return isHead();
    }
    int hasOneChild() const
    {
        return m_childList.hasOneMember();
    }

    int getNumChildren() const
    {
        return m_childList.getLength();
    }
    int getDepth() const;

    void Xreset()
    {
        DLISTnode::Xreset();
        m_parent = this;
        m_childList.Xreset();
    }

    void XcopyLinks( const TREEnode &src )
    {
        DLISTnode::XcopyLinks( src );
        m_parent = src.m_parent;
    }

#ifdef DEBUG
    void check() const;
#elif defined( TSTBUG )
    void check() const
    {
        DLISTnode::check();
    }
#else
    void check() const {}
#endif

#ifdef TSTBUG
    // NOTE:  all assertions have to negate the boolean expression.
    //        from the original if statements tied to the THROW_ERR statements.
    void checkOnTree() const
    {
        assert( isOnTree() );
    }// THROW_ERR( "Node must be on tree" ) }
    void checkNotOnTree() const
    {
        assert( ! isOnTree() );
    }// THROW_ERR( "Node mustn't be on tree" ) }
    void checkIsLeaf() const
    {
        assert( isLeaf() );
    }// THROW_ERR( "Node must be leaf" ) }
    void checkNotLeaf() const
    {
        assert( ! isLeaf() );
    }// THROW_ERR( "Node mustn't be leaf" ) }
    void checkIsRoot() const
    {
        assert( isRoot() );
    }// THROW_ERR( "Node must be root" ) }
    void checkNotRoot() const
    {
        assert( ! isRoot() );
    }// THROW_ERR( "Node mustn't be root" ) }

    void checkIsFirstSibling() const
    {
        assert( isFirstSibling() );
        //  THROW_ERR( "Node must be first sibling" )
    }

    void checkNotFirstSibling() const
    {
        assert( ! isFirstSibling() );
        //  THROW_ERR( "Node mustn't be first sibling" )
    }

    void checkIsLastSibling() const
    {
        assert( isLastSibling() );
        //  THROW_ERR( "Node must be last sibling" )
    }

    void checkNotLastSibling() const
    {
        assert( ! isLastSibling() );
        //  THROW_ERR( "Node mustn't be last sibling" )
    }

    void checkHasOneChild() const
    {
        assert( hasOneChild() );
        //  THROW_ERR( "Node must have one and only one child" )
    }

    void checkNotHasOneChild() const
    {
        assert( ! hasOneChild() );
        //  THROW_ERR( "Node mustn't have one and only child" )
    }

#else
    void checkOnTree() const {}
    void checkNotOnTree() const {}
    void checkIsLeaf() const {}
    void checkNotLeaf() const {}
    void checkIsRoot() const {}
    void checkNotRoot() const {}
    void checkIsFirstSibling() const {}
    void checkNotFirstSibling() const {}
    void checkIsLastSibling() const {}
    void checkNotLastSibling() const {}
    void checkHasOneChild() const {}
    void checkNotHasOneChild() const {}
#endif

private:

    void XrcrsvCheck() const;
    void XrcrsvCopy( const TREEnode *node );
};

/*-------------------------------------------------------------------*
 | dummyTREEnode -- dummy node class
 |
 | TREEbase and PTR_INTO_TREEbase both contain member objects of
 | this class.
 |
 | The one in TREEbase is used to point to the root of the tree.
 |
 | The one in PTR_INTO_TREEbase is used to hold the linkage info
 | that was in objects which have been remove()'ed.  It is also used
 | to give the PTR_INTO something to point to when it isn't valid.
 | This actually saves alot of time, since we don't have to go
 | checking for validity every time we move the pointer.
 |
 | dummyTREEnode is intended ONLY for use in these two object
 | classes.
 *-------------------------------------------------------------------*/

class dummyTREEnode: public TREEnode
{
    friend class TREEbase;
    friend class PTR_INTO_TREEbase;

protected:

    virtual DLISTnode *XmakeCopy() const
    {
        return new dummyTREEnode( *this );
    }

public:

    virtual int isNode() const
    {
        return 0;
    }
};

/*-------------------------------------------------------------------*
 | MEMBERS_FOR_TREEnode() -- macro that declares those members of
 |                           a TREE node which can't be inherited
 |
 | If we could inherit from instantiations of templates (my compiler
 | can't), then most of these members could be handled differently.
 | But overloaded operator=, and members that require knowledge of
 | the object's size, can't be inherited anyway.
 *-------------------------------------------------------------------*/

#define MEMBERS_FOR_TREEnode( TYPE )                                  \
  TYPE *getParent() const                                             \
    { checkNotRoot(); return (TYPE *)XgetParent(); }              \
  TYPE *getPrevSibling() const                                        \
    { checkNotFirstSibling(); return (TYPE *)XgetPrevSibling(); } \
  TYPE *getNextSibling() const                                        \
    { checkNotLastSibling(); return (TYPE *)XgetNextSibling(); }  \
  TYPE *getFirstChild() const                                         \
    { checkNotLeaf(); return (TYPE *)XgetFirstChild(); }          \
  TYPE *getLastChild() const                                          \
    { checkNotLeaf(); return (TYPE *)XgetLastChild(); }           \
  TYPE *getFirstLeaf() const                                          \
    { checkOnTree(); return (TYPE *)XgetFirstLeaf(); }            \
  TYPE *getLastLeaf() const                                           \
    { checkOnTree(); return (TYPE *)XgetLastLeaf(); }             \
  TYPE *getRoot() const                                               \
    { checkOnTree(); return (TYPE *)XgetRoot(); }                 \
  TYPE *getPreOrderNext() const                                       \
    { checkOnTree(); return (TYPE *)XgetPreOrderNext(); }         \
  TYPE *getPostOrderNext() const                                      \
    { checkOnTree(); return (TYPE *)XgetPostOrderNext(); }        \
  void insertParent( TYPE *node )                                     \
    { checkOnTree(); XinsertParent( node ); }                     \
  void insertPrevSibling( TYPE *node )                                \
    { checkOnTree(); checkNotRoot(); XinsertPrevSibling( node ); }\
  void insertNextSibling( TYPE *node )                                \
    { checkOnTree(); checkNotRoot(); XinsertNextSibling( node ); }\
  void insertFirstChild( TYPE *node )                                 \
    { checkOnTree(); XinsertFirstChild( node ); }                 \
  void insertLastChild( TYPE *node )                                  \
    { checkOnTree(); XinsertLastChild( node ); }                  \
  TYPE *makeCopy() const                                              \
    { return (TYPE *)XmakeCopy(); }                               \
  virtual DLISTnode *XmakeCopy() const                                \
    { return new TYPE( *this ); }                                 \
 
/*-------------------------------------------------------------------*
 | TREEbase -- base class for all trees
 *-------------------------------------------------------------------*/

class TREEbase
{
    friend class PTR_INTO_TREEbase;

private:

    dummyTREEnode m_vnode;

protected:

    TREEbase(): m_vnode() {}
    TREEbase( const TREEbase &tree ): m_vnode()
    {
        XcopyTree( tree );
    }

    virtual ~TREEbase() {}

public:

    TREEnode *XgetRoot() const
    {
        return m_vnode.XgetFirstChild();
    }
    TREEnode *XgetFirstLeaf() const
    {
        return m_vnode.XgetFirstLeaf();
    }
    TREEnode *XgetLastLeaf() const
    {
        return m_vnode.XgetLastLeaf();
    }
    void XinsertRoot( TREEnode *node )
    {
        m_vnode.XinsertFirstChild( node );
    }
    void XcopyTree( const TREEbase &tree )
    {
        m_vnode.XrcrsvCopy( &tree.m_vnode );
    }

public:

    int isEmpty() const
    {
        return m_vnode.isLeaf();
    }

    int getHeight() const;

    void removeRoot();
    void removeAll()
    {
        if( ! isEmpty() )
        {
            delete XgetRoot();
        }
    }

    void check() const
    {
        m_vnode.check();
    }

#ifdef TSTBUG
    void checkEmpty() const
    {
        assert( isEmpty() );
    }// THROW_ERR( "Tree must be empty" ); }
    void checkNotEmpty() const
    {
        assert( ! isEmpty() );
    }// THROW_ERR( "Tree mustn't be empty" ); }
#else
    void checkEmpty() const {}
    void checkNotEmpty() const {}
#endif
};

/*-------------------------------------------------------------------*
 | MEMBERS_FOR_TREEbase() -- macro that declares those members of a
 |                           TREE which can't be inherited
 |
 | If we could inherit from instantiations of templates (my compiler
 | can't), then most of these members could be handled differently.
 | But overloaded operator=, and members that require knowledge of
 | the object's size, can't be inherited anyway.
 *-------------------------------------------------------------------*/

#define MEMBERS_FOR_TREEbase( TREEtmplt, TYPE )                       \
  TREEtmplt(): TREEbase() {}                                          \
  TREEtmplt( const TREEtmplt< TYPE > &tree ):                         \
    TREEbase( tree ) {}                                               \
  TREEtmplt< TYPE > &operator=( const TREEtmplt< TYPE > &tree )       \
    { removeAll(); XcopyTree( tree ); return *this; }             \
 
/*-------------------------------------------------------------------*
 | PTR_INTO_TREEbase -- base class for all PTR_INTO_xTREE's
 *-------------------------------------------------------------------*/

class PTR_INTO_TREEbase
{
private:

    dummyTREEnode m_dummy;
    TREEnode *m_ptr;

protected:

    PTR_INTO_TREEbase(): m_dummy(), m_ptr( &m_dummy ) {}

    PTR_INTO_TREEbase( TREEnode *ptr ):
        m_dummy(),
        m_ptr( ptr )
    {
    }

    PTR_INTO_TREEbase( TREEbase &tree ):
        m_dummy(),
        m_ptr( tree.XgetRoot() )
    {
    }

    PTR_INTO_TREEbase( TREEbase &tree, START_AT_ROOTversion ):
        m_dummy(),
        m_ptr( tree.XgetRoot() )
    {
    }

    PTR_INTO_TREEbase( TREEbase &tree, START_AT_FIRST_LEAFversion ):
        m_dummy(),
        m_ptr( tree.XgetFirstLeaf() )
    {
    }

    PTR_INTO_TREEbase( TREEbase &tree, START_AT_LAST_LEAFversion ):
        m_dummy(),
        m_ptr( tree.XgetLastLeaf() )
    {
    }

    PTR_INTO_TREEbase( const PTR_INTO_TREEbase &ptr ):
        m_dummy(),
        m_ptr( ptr.m_ptr )
    {
        ptr.checkNotRemoved();
    }

    virtual ~PTR_INTO_TREEbase()
    {
        m_dummy.Xreset();
    }

public:

    void Xset( TREEnode *ptr )
    {
        m_ptr = ptr;
    }
    void Xset( TREEbase &tree )
    {
        m_ptr = tree.XgetRoot();
    }
    void Xset( TREEbase &tree, START_AT_ROOTversion )
    {
        m_ptr = tree.XgetRoot();
    }
    void Xset( TREEbase &tree, START_AT_FIRST_LEAFversion )
    {
        m_ptr = tree.XgetFirstLeaf();
    }
    void Xset( TREEbase &tree, START_AT_LAST_LEAFversion )
    {
        m_ptr = tree.XgetLastLeaf();
    }
    void Xset( const PTR_INTO_TREEbase &ptr )
    {
        m_ptr = ptr.m_ptr;
    }

    void XinsertParent( TREEnode *node )
    {
        m_ptr->XinsertParent( node );
    }
    void XinsertPrevSibling( TREEnode *node )
    {
        m_ptr->XinsertPrevSibling( node );
    }
    void XinsertNextSibling( TREEnode *node )
    {
        m_ptr->XinsertNextSibling( node );
    }
    void XinsertFirstChild( TREEnode *node )
    {
        m_ptr->XinsertFirstChild( node );
    }
    void XinsertLastChild( TREEnode *node )
    {
        m_ptr->XinsertLastChild( node );
    }

    TREEnode *Xget() const
    {
        return m_ptr;
    }

public:

    int isInitialized() const
    {
        return m_ptr->isOnTree();
    }
    int isValid() const
    {
        return m_ptr->isNode();
    }
    int isRemoved() const
    {
        return m_ptr == &m_dummy;
    }
    int isAtRoot() const
    {
        return m_ptr->isRoot();
    }
    int isAtFirstSibling() const
    {
        return m_ptr->isFirstSibling();
    }
    int isAtLastSibling() const
    {
        return m_ptr->isLastSibling();
    }
    int isAtLeaf() const
    {
        return m_ptr->isLeaf();
    }

    int isEqualTo( const TREEnode *ptr ) const
    {
        return m_ptr == ptr;
    }
    int isEqualTo( const PTR_INTO_TREEbase &ptr ) const
    {
        return m_ptr == ptr.m_ptr;
    }

    int getDepth()
    {
        return m_ptr->getDepth();
    }

    void gotoParent()
    {
        m_ptr = m_ptr->XgetParent();
    }
    void gotoPrevSibling()
    {
        m_ptr = m_ptr->XgetPrevSibling();
    }
    void gotoNextSibling()
    {
        m_ptr = m_ptr->XgetNextSibling();
    }
    void gotoFirstChild()
    {
        m_ptr = m_ptr->XgetFirstChild();
    }
    void gotoLastChild()
    {
        m_ptr = m_ptr->XgetLastChild();
    }
    void gotoPreOrderNext()
    {
        m_ptr = m_ptr->XgetPreOrderNext();
    }
    void gotoPostOrderNext()
    {
        m_ptr = m_ptr->XgetPostOrderNext();
    }

    void removeSubtree()
    {
        checkValid();

        m_dummy.XcopyLinks( *m_ptr );
        delete m_ptr;
        m_ptr = &m_dummy;
    }

#ifdef TSTBUG
    // NOTE: The boolean in the assertions needed to be negated from the original
    //       boolean for the ifs that were tied to the THROW_ERR statements.
    void checkInitialized() const
    {
        assert( isInitialized() );
        //  THROW_ERR( "Pointer into tree must be initialized" );
    }

    void checkNotInitialized() const
    {
        assert( ! isInitialized() );
        //  THROW_ERR( "Pointer into tree mustn't be initialized" );
    }

    void checkValid() const
    {
        assert( isValid() );
        //  THROW_ERR( "Pointer into tree must be valid" );
    }

    void checkNotValid() const
    {
        assert( ! isValid() );
        //  THROW_ERR( "Pointer into tree mustn't be valid" );
    }

    void checkRemoved() const
    {
        assert( isRemoved() );
        //  THROW_ERR( "Pointer into tree must point to removed node" );
    }

    void checkNotRemoved() const
    {
        assert( ! isRemoved() );
        //  THROW_ERR( "Pointer into tree mustn't point to removed node" );
    }

    void checkIsAtRoot() const
    {
        assert( isAtRoot() );
        //  THROW_ERR( "Pointer into tree must be at root" );
    }

    void checkNotAtRoot() const
    {
        assert( ! isAtRoot() );
        //THROW_ERR( "Pointer into tree mustn't be at root" );
    }

    void checkIsAtFirstSibling() const
    {
        assert( isAtFirstSibling() );
        //THROW_ERR( "Pointer into tree must be at first sibling" );
    }

    void checkNotAtFirstSibling() const
    {
        assert( ! isAtFirstSibling() );
        //THROW_ERR( "Pointer into tree mustn't be at first sibling" );
    }

    void checkIsAtLastSibling() const
    {
        assert( isAtLastSibling() );
        //THROW_ERR( "Pointer into tree must be at last sibling" )
    }

    void checkNotAtLastSibling() const
    {
        assert( ! isAtLastSibling() );
        //THROW_ERR( "Pointer into tree mustn't be at last sibling" )
    }

    void checkIsAtLeaf() const
    {
        assert( isAtLastSibling() );
        //  THROW_ERR( "Pointer into tree must be at leaf" )
    }

    void checkNotAtLeaf() const
    {
        assert( ! isAtLastSibling() );
        //  THROW_ERR( "Pointer into tree mustn't be at leaf" )
    }

#else
    void checkInitialized() const {}
    void checkNotInitialized() const {}
    void checkValid() const {}
    void checkNotValid() const {}
    void checkRemoved() const {}
    void checkNotRemoved() const {}
    void checkIsAtRoot() const {}
    void checkNotAtRoot() const {}
    void checkIsAtFirstSibling() const {}
    void checkNotAtFirstSibling() const {}
    void checkIsAtLastSibling() const {}
    void checkNotAtLastSibling() const {}
    void checkIsAtLeaf() const {}
    void checkNotAtLeaf() const {}
#endif
};

/*-------------------------------------------------------------------*
 | MEMBERS_FOR_PTR_INTO_TREEbase() -- macro that declares those
 |                                    members of a PTR_INTO_xTREE
 |                                    which can't be inherited
 *-------------------------------------------------------------------*/

#define MEMBERS_FOR_PTR_INTO_TREEbase( PTRtmplt, TREEtmplt, TYPE )    \
  PTRtmplt(): PTR_INTO_TREEbase() {}                                  \
  PTRtmplt( TREEtmplt< TYPE > &tree ):                                \
    PTR_INTO_TREEbase( tree ) {}                                      \
  PTRtmplt( TREEtmplt< TYPE > &tree, START_AT_ROOTversion ):          \
    PTR_INTO_TREEbase( tree, START_AT_ROOT ) {}                       \
  PTRtmplt( TREEtmplt< TYPE > &tree, START_AT_FIRST_LEAFversion ):    \
    PTR_INTO_TREEbase( tree, START_AT_FIRST_LEAF ) {}                 \
  PTRtmplt( TREEtmplt< TYPE > &tree, START_AT_LAST_LEAFversion ):     \
    PTR_INTO_TREEbase( tree, START_AT_LAST_LEAF ) {}                  \
  PTRtmplt( const PTRtmplt< TYPE > &ptr ):                            \
    PTR_INTO_TREEbase( ptr ) {}                                       \
  void set( TREEtmplt< TYPE > &tree )                                 \
    { Xset( tree ); }                                             \
  void set( TREEtmplt< TYPE > &tree, START_AT_ROOTversion )           \
    { Xset( tree, START_AT_ROOT ); }                              \
  void set( TREEtmplt< TYPE > &tree, START_AT_FIRST_LEAFversion )     \
    { Xset( tree, START_AT_FIRST_LEAF ); }                        \
  void set( TREEtmplt< TYPE > &tree, START_AT_LAST_LEAFversion )      \
    { Xset( tree, START_AT_LAST_LEAF ); }                         \
  void set( const PTRtmplt< TYPE > &ptr )                             \
    { Xset( ptr ); }                                              \
  PTRtmplt< TYPE > &operator=( TREEtmplt< TYPE > &tree )              \
    { set( tree ); return *this; }                                \
  PTRtmplt< TYPE > &operator=( const PTRtmplt< TYPE > &ptr )          \
    { set( ptr ); return *this; }                                 \
  PTRtmplt< TYPE > &operator++()                                      \
    { gotoNextSibling(); return *this; }                          \
  PTRtmplt< TYPE > &operator--()                                      \
    { gotoPrevSibling(); return *this; }                          \
  int operator==( const PTRtmplt< TYPE > ptr ) const                  \
    { return isEqualTo( ptr ); }                                  \
  int operator!=( const PTRtmplt< TYPE > ptr ) const                  \
    { return ! isEqualTo( ptr ); }                                \
 
/*-------------------------------------------------------------------*
 | Templates for store-by-value trees
 *-------------------------------------------------------------------*/

template< class TYPE >
class vTREE_NODE_OF: public TREEnode
{
    friend class vTREE_OF< TYPE >;
    friend class PTR_INTO_vTREE_OF< TYPE >;

private:

    TYPE m_info;

private:

    vTREE_NODE_OF( const TYPE &info ):
        TREEnode(),
        m_info( info )
    {
    }

protected:

    MEMBERS_FOR_TREEnode( vTREE_NODE_OF< TYPE > )
};

template< class TYPE >
class vTREE_OF: public TREEbase
{
public:

    MEMBERS_FOR_TREEbase( vTREE_OF, TYPE )

    TYPE *getRoot() const
    {
        checkNotEmpty();
        return &((vTREE_NODE_OF< TYPE > *)XgetRoot())->m_info;
    }

    TYPE *getFirstLeaf() const
    {
        checkNotEmpty();
        return &((vTREE_NODE_OF< TYPE > *)XgetFirstLeaf())->m_info;
    }

    TYPE *getLastLeaf() const
    {
        checkNotEmpty();
        return &((vTREE_NODE_OF< TYPE > *)XgetLastLeaf())->m_info;
    }

    void insertRoot( const TYPE &info )
    {
        XinsertRoot( new vTREE_NODE_OF< TYPE >( info ) );
    }
    TYPE &operator*() const
    {
        return *getRoot();
    }
};

template< class TYPE >
class PTR_INTO_vTREE_OF: public PTR_INTO_TREEbase
{
public:

    MEMBERS_FOR_PTR_INTO_TREEbase( PTR_INTO_vTREE_OF,
                                   vTREE_OF,
                                   TYPE )

    operator TYPE*()
    {
        return get();
    }

    TYPE *get() const
    {
        checkValid();
        return &((vTREE_NODE_OF< TYPE > *)Xget())->m_info;
    }

    void insertParent( const TYPE &info )
    {
        checkValid();
        XinsertParent( new vTREE_NODE_OF< TYPE >( info ) );
    }

    void insertPrevSibling( const TYPE &info )
    {
        checkValid();
        XinsertPrevSibling( new vTREE_NODE_OF< TYPE >( info ) );
    }

    void insertNextSibling( const TYPE &info )
    {
        checkValid();
        XinsertNextSibling( new vTREE_NODE_OF< TYPE >( info ) );
    }

    void insertFirstChild( const TYPE &info )
    {
        checkValid();
        XinsertFirstChild( new vTREE_NODE_OF< TYPE >( info ) );
    }

    void insertLastChild( const TYPE &info )
    {
        checkValid();
        XinsertLastChild( new vTREE_NODE_OF< TYPE >( info ) );
    }

    TYPE &operator*() const
    {
        return *get();
    }
};

/*-------------------------------------------------------------------*
 | Templates for store-by-address trees
 *-------------------------------------------------------------------*/

template< class TYPE >
class ptrTREE_NODE_OF: public TREEnode
{
    friend class ptrTREE_OF< TYPE >;
    friend class PTR_INTO_ptrTREE_OF< TYPE >;

private:

    TYPE *m_info;

private:

    ptrTREE_NODE_OF( TYPE &info ):
        TREEnode(),
        m_info( &info )
    {
    }

protected:

    MEMBERS_FOR_TREEnode( ptrTREE_NODE_OF< TYPE > )
};

template< class TYPE >
class ptrTREE_OF: public TREEbase
{
public:

    MEMBERS_FOR_TREEbase( ptrTREE_OF, TYPE )

    TYPE *getRoot() const
    {
        checkNotEmpty();
        return ((ptrTREE_NODE_OF< TYPE > *)XgetRoot())->m_info;
    }

    TYPE *getFirstLeaf() const
    {
        checkNotEmpty();
        return ((ptrTREE_NODE_OF< TYPE > *)XgetFirstLeaf())->m_info;
    }

    TYPE *getLastLeaf() const
    {
        checkNotEmpty();
        return ((ptrTREE_NODE_OF< TYPE > *)XgetLastLeaf())->m_info;
    }

    void insertRoot( TYPE &info )
    {
        XinsertRoot( new ptrTREE_NODE_OF< TYPE >( info ) );
    }
    TYPE &operator*() const
    {
        return *getRoot();
    }
};

template< class TYPE >
class PTR_INTO_ptrTREE_OF: public PTR_INTO_TREEbase
{
public:

    MEMBERS_FOR_PTR_INTO_TREEbase( PTR_INTO_ptrTREE_OF,
                                   ptrTREE_OF,
                                   TYPE )

    operator TYPE*()
    {
        return get();
    }

    TYPE *get() const
    {
        checkValid();
        return ((ptrTREE_NODE_OF< TYPE > *)Xget())->m_info;
    }

    void insertParent( TYPE &info )
    {
        checkValid();
        XinsertParent( new ptrTREE_NODE_OF< TYPE >( info ) );
    }

    void insertPrevSibling( TYPE &info )
    {
        checkValid();
        XinsertPrevSibling( new ptrTREE_NODE_OF< TYPE >( info ) );
    }

    void insertNextSibling( TYPE &info )
    {
        checkValid();
        XinsertNextSibling( new ptrTREE_NODE_OF< TYPE >( info ) );
    }

    void insertFirstChild( TYPE &info )
    {
        checkValid();
        XinsertFirstChild( new ptrTREE_NODE_OF< TYPE >( info ) );
    }

    void insertLastChild( TYPE &info )
    {
        checkValid();
        XinsertLastChild( new ptrTREE_NODE_OF< TYPE >( info ) );
    }

    TYPE &operator*() const
    {
        return *get();
    }
};

/*-------------------------------------------------------------------*
 | Templates for intrusive trees
 *-------------------------------------------------------------------*/

template< class TYPE >
class iTREE_OF: public TREEbase
{
public:

    MEMBERS_FOR_TREEbase( iTREE_OF, TYPE )

    TYPE *getRoot() const
    {
        checkNotEmpty();
        return (TYPE *)XgetRoot();
    }
    TYPE *getFirstLeaf() const
    {
        checkNotEmpty();
        return (TYPE *)XgetFirstLeaf();
    }
    TYPE *getLastLeaf() const
    {
        checkNotEmpty();
        return (TYPE *)XgetLastLeaf();
    }
    void insertRoot( TYPE *node )
    {
        XinsertRoot( node );
    }
    TYPE &operator*() const
    {
        return *getRoot();
    }
};

template< class TYPE >
class PTR_INTO_iTREE_OF: public PTR_INTO_TREEbase
{
public:

    MEMBERS_FOR_PTR_INTO_TREEbase( PTR_INTO_iTREE_OF,
                                   iTREE_OF,
                                   TYPE )

    PTR_INTO_iTREE_OF( TYPE *node ):
        PTR_INTO_TREEbase( node )
    {
        node->checkOnTree();
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
    void insertParent( TYPE *node )
    {
        checkValid();
        XinsertParent( node );
    }
    void insertPrevSibling( TYPE *node )
    {
        checkValid();
        XinsertPrevSibling( node );
    }
    void insertNextSibling( TYPE *node )
    {
        checkValid();
        XinsertNextSibling( node );
    }
    void insertFirstChild( TYPE *node )
    {
        checkValid();
        XinsertFirstChild( node );
    }
    void insertLastChild( TYPE *node )
    {
        checkValid();
        XinsertLastChild( node );
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

#define LOOP_TREE( ptr, tree )                                        \
  for( ptr.set( tree, START_AT_ROOT );                                \
       ptr.isValid();                                                 \
       ptr.gotoPreOrderNext() )                                       \
 
#define LOOP_TREEpostOrder( ptr, tree )                               \
  for( ptr.set( tree, START_AT_FIRST_LEAF );                          \
       ptr.isValid();                                                 \
       ptr.gotoPostOrderNext() )                                      \
 
#define LOOP_TREEchildren( ptr, node )                                \
  for( ptr.set( node ), ptr.gotoFirstChild(); ptr.isValid(); ++ptr )  \
 
#define LOOP_TREEchildrenRev( ptr, node )                             \
  for( ptr.set( node ), ptr.gotoLastChild(); ptr.isValid(); --ptr )   \
 
#endif

