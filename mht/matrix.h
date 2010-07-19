
/*********************************************************************
 * FILE: matrix.H                                                    *
 *                                                                   *
 * AUTHOR: Matthew Miller (mlm)                                      *
 *                                                                   *
 * HISTORY:                                                          *
 *    2 JUN 93 -- (mlm) created                                      *
 *                                                                   *
 * CONTENTS:                                                         *
 *                                                                   *
 *   Definitions for matrices and matrix operations.                 *
 *                                                                   *
 *   A MATRIX can be constructed in the following ways:              *
 *                                                                   *
 *     MATRIX mat( numRows, numCols )                                *
 *       A matrix of size numRows x numCols.  The initial values in  *
 *       the matrix are garbage.                                     *
 *                                                                   *
 *     MATRIX mat( m0 )                                              *
 *       If m0 is another MATRIX, this makes a deep copy.            *
 *                                                                   *
 *   The following operators are overloaded for matrices:            *
 *                                                                   *
 *     MATRIX = double                                               *
 *       Set all the entries in the matrix to the double.            *
 *                                                                   *
 *     MATRIX + MATRIX                                               *
 *       Returns the result of adding two matrices.                  *
 *                                                                   *
 *     MATRIX - MATRIX                                               *
 *       Returns the result of subtracting the second matrix from    *
 *       the first.                                                  *
 *                                                                   *
 *     MATRIX * MATRIX                                               *
 *       Returns the result of standard matrix multiplication.       *
 *                                                                   *
 *     MATRIX * double                                               *
 *       Returns the result of multiplying each entry in the matrix  *
 *       by the double.                                              *
 *                                                                   *
 *     MATRIX( r, c )                                                *
 *       Returns a reference to the element of the matrix at         *
 *       row r, column c.  Both r and c default to 0.                *
 *                                                                   *
 *   A MATRIX has the following member functions:                    *
 *                                                                   *
 *     set( v0, v1, ... )                                            *
 *       Set all the values in the matrix.  The number of arguments  *
 *       must be exactly equal to the size of the matrix (width      *
 *       times height).  If there are too many arguments, the extras *
 *       will be ignored.  If there are too few, the last few        *
 *       values in the matrix will be filled with garbage.           *
 *                                                                   *
 *       The entries are filled in, starting with row 0, column 0,   *
 *       and proceeding in row-major order.  So v0 goes into the     *
 *       entry at row 0, column 0; v1 goes into row 0, column1; etc. *
 *                                                                   *
 *       WARNING: this function is not type-safe.  All the arguments *
 *       MUST be doubles.  No automatic type-casting or error        *
 *       messages will result from a mistake -- only a bug.          *
 *                                                                   *
 *     vset( v0, ap )                                                *
 *       This is the same as set() except that it is given a         *
 *       va_list argument (see stdarg.h).  The value to be placed    *
 *       in row 0, column 0 must be given explicitly.                *
 *                                                                   *
 *     isIdentity()                                                  *
 *                                                                   *
 *       Return 1 if matrix is an identity matrix.  0 otherwise.     *
 *                                                                   *
 *     getNumRows()                                                  *
 *       Return the number of rows in the matrix.                    *
 *                                                                   *
 *     getNumCols()                                                  *
 *       Return the number of columns in the matrix.                 *
 *                                                                   *
 *     getData()                                                     *
 *       Return a pointer to the element of the matrix at row 0,     *
 *       column 0.  The remaining elements are stored after it in    *
 *       row-major order.                                            *
 *                                                                   *
 *       WARNING: under some circumstances, using the matrix on the  *
 *       left-hand-side of an "=" can change the address of its      *
 *       data.  So something like                                    *
 *                                                                   *
 *         p = m0.getData();                                         *
 *         m0 = m1 * m2;                                             *
 *         *p = 0.;                                                  *
 *                                                                   *
 *       is a bad idea.                                              *
 *                                                                   *
 *     trans()                                                       *
 *       Return the transpose of the matrix.                         *
 *                                                                   *
 *     reduce( numRows, numCols )                                    *
 *       Return the portion of the matrix of size numRows x numCols, *
 *       that contains the element at 0,0.                           *
 *                                                                   *
 *     inv()                                                         *
 *       Return the inverse of the matrix.                           *
 *                                                                   *
 *     det()                                                         *
 *       Return the determinant of the matrix.                       *
 *                                                                   *
 *     print( numSpaces )                                            *
 *       Print out the matrix, indenting each row by numSpaces.      *
 *       The values in the matrix are rounded to the nearest         *
 *       10000'th.  No attempt is made to make the columns line up   *
 *       nicely.                                                     *
 *                                                                   *
 *       The indentation defaults to 0.                              *
 *                                                                   *
 * IMPLEMENTATION NOTES:                                             *
 *                                                                   *
 *   The implementation of matrices in matrix.H and matrix.C is a    *
 *   bit unintuitive.  Instead of having a single, straightforward   *
 *   MATRIX class, I have two classes: MATRIX and tmpMATRIX.  A      *
 *   MATRIX is created and used in the ways that one might expect.   *
 *   A tmpMATRIX cannot be created directly.  It can only be         *
 *   created as the result of some operation performed on a MATRIX.  *
 *                                                                   *
 *   The difference between a MATRIX and a tmpMATRIX is that, when   *
 *   you perform an operation on a tmpMATRIX, creating another       *
 *   tmpMATRIX, the new tmpMATRIX might use the same memory as the   *
 *   old tmpMATRIX.  In other words, operations on tmpMATRIX's are   *
 *   destructive.  This significantly reduces the number of memory   *
 *   allocations and deallocations required.                         *
 *                                                                   *
 *   This tmpMATRIX trick works because tmpMATRIX's are only         *
 *   ever constructed as temporary objects.  A temporary object      *
 *   can only ever have one operation performed on it before it is   *
 *   destroyed (if the program is written properly), so why not      *
 *   destroy it during that operation?                               *
 *                                                                   *
 *   Here's an example of how tmpMATRIX's work.  Suppose M0, M1, M2, *
 *   and M3 are all MATRIX's.  Consider the following line of code:  *
 *                                                                   *
 *     M3 = M0 * M1 + M2                                             *
 *                                                                   *
 *   Three operations are performed here:                            *
 *                                                                   *
 *     1. M0 is multiplied by M1, call the result T0                 *
 *     2. M2 is added to T0, call the result T1                      *
 *     3. T1 is copied into M3                                       *
 *                                                                   *
 *   In a normal implementation, in which T0 and T1 are just normal  *
 *   MATRIX's, this would require two allocations and two            *
 *   deallocations of matrix arrays, plus a memory copy to perform   *
 *   the assignment.                                                 *
 *                                                                   *
 *   But here, T0 and T1 are tmpMATRIX's.  The first step, which     *
 *   creates T0, still requires the allocation of a new matrix       *
 *   array.  But in the second step, the addition is performed       *
 *   destructively on the array allocated for T0, and that memory    *
 *   is "stolen" from T0 by T1.  In the third step, M3's array is    *
 *   simply deallocated, and T1's array is stolen by M3.  Thus       *
 *   only one new array is allocated, and no copying of memory is    *
 *   required.                                                       *
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

#ifndef MATRIX_H
#define MATRIX_H

#include <stdarg.h>
#include <string.h>

#include "except.h"
#include <assert.h>

/*-------------------------------------------------------------------*
 | Declarations of stuff found in this file
 *-------------------------------------------------------------------*/

class MATRIX;
class tmpMATRIX;

/*-------------------------------------------------------------------*
 | MATRIX -- basic matrix class
 *-------------------------------------------------------------------*/

class MATRIX
{
protected:

    const int m_numRows;
    const int m_numCols;
    const int m_size;
    double *m_data;

    /* make a MATRIX using a matrix array that's already been
       allocated */
    MATRIX( int numRows, int numCols, double *data ):
        m_numRows( numRows ),
        m_numCols( numCols ),
        m_size( numRows * numCols ),
        m_data( data )
    {
    }

public:

    MATRIX( int numRows = 0, int numCols = 0 ):
        m_numRows( numRows ),
        m_numCols( numCols ),
        m_size( numRows * numCols ),
        m_data( new double[ numRows * numCols ] )
    {
    }

    MATRIX( const MATRIX &src ):
        m_numRows( src.m_numRows ),
        m_numCols( src.m_numCols ),
        m_size( src.m_size ),
        m_data( new double[ src.m_numRows * src.m_numCols ] )
    {
        memcpy( m_data, src.m_data, m_size * sizeof( *m_data ) );
    }

    inline MATRIX( const tmpMATRIX &src );

    virtual ~MATRIX()
    {
        if( m_data != 0 )
        {
            delete [] m_data;
        }
        m_data = 0;
    }

    MATRIX &operator=( double val );

    MATRIX &operator=( const MATRIX &src )
    {
#ifdef TSTBUG
        assert( m_numRows == src.m_numRows && m_numCols == src.m_numCols );
        //THROW_ERR( "Bad assignment -- "
        //<< m_numRows << "x" << m_numCols << " matrix = "
        //<< src.m_numRows << "x" << src.m_numCols
        //<< " matrix" )
#endif

        memcpy( m_data, src.m_data, m_size * sizeof( *m_data ) );
        return *this;
    }

    inline MATRIX &operator=( const tmpMATRIX &src );

    double &operator()( int row = 0, int col = 0 ) const
    {
#ifdef TSTBUG
        assert( 0 <= row && row < m_numRows &&
                0 <= col && col < m_numCols );
        //THROW_ERR( "Matrix index out of bounds. -- <" << row << "," << col << ">"
        //<< " out of bounds in "
        //<< m_numRows << "x" << m_numCols << " matrix" )
#endif

        return m_data[ row * m_numCols + col ];
    }

    void vset( double firstVal, va_list ap );
    void set( double firstVal, ... );

    int isIdentity() const;
    int getNumRows() const
    {
        return m_numRows;
    }
    int getNumCols() const
    {
        return m_numCols;
    }
    double *getData() const
    {
        return m_data;
    }

    tmpMATRIX trans();
    tmpMATRIX reduce( int numRows, int numCols );
    tmpMATRIX inv();
    double det();

    void print( int numSpaces = 0 ) const;
};

/*-------------------------------------------------------------------*
 | tmpMATRIX -- temporary matrix class
 |
 | This class is not intended for use outside of the matrix package
 *-------------------------------------------------------------------*/

class tmpMATRIX: public MATRIX
{
    friend class MATRIX;
public:
    tmpMATRIX ( const tmpMATRIX &src ): MATRIX( src ) {}

private:

    tmpMATRIX( const MATRIX &m0, const MATRIX &m1 ); // m0 * m1
    tmpMATRIX( const MATRIX &src ): MATRIX( src ) {}
    tmpMATRIX( int nRows, int nCols ): MATRIX( nRows, nCols ) {}
    tmpMATRIX( int numRows, int numCols, double *data ):
        MATRIX( numRows, numCols, data )
    {
    }

private:

    tmpMATRIX &add( const MATRIX &src );
    tmpMATRIX &subtract( const MATRIX &src );
    tmpMATRIX &subtractFrom( const MATRIX &src );
    tmpMATRIX &multiply( double src );

    double *stealData() const   /* not really const */
    {
        double *data;

#ifdef TSTBUG
        assert( m_data != 0 );
        //  THROW_ERR( "Trying to steal nonexistant tmpMATRIX data" )
#endif

        data = m_data;
        ((tmpMATRIX *)this)->m_data = 0;

        return data;
    }

public:

    double &operator()( int row = 0, int col = 0 ) const
    {
#ifdef TSTBUG
        assert( 0 <= row && row < m_numRows &&
                0 <= col && col < m_numCols );
        //THROW_ERR( "Matrix index out of bounds -- <" // << row << "," << col << ">"
        //<< " out of bounds in "
        //<< m_numRows << "x" << m_numCols << " matrix" )
#endif

        return m_data[ row * m_numCols + col ];
    }

    tmpMATRIX operator+( const MATRIX &m )
    {
        add( m );
        return *this;
    }

    tmpMATRIX operator-( const MATRIX &m )
    {
        subtract( m );
        return *this;
    }

    tmpMATRIX operator*( double num )
    {
        multiply( num );
        return *this;
    }

    friend inline tmpMATRIX operator+( const MATRIX &m0,
                                       const MATRIX &m1 );
    friend inline tmpMATRIX operator+( const MATRIX &m0,
                                       const tmpMATRIX &m1 );

    friend inline tmpMATRIX operator-( const MATRIX &m0,
                                       const MATRIX &m1 );
    friend inline tmpMATRIX operator-( const MATRIX &m0,
                                       const tmpMATRIX &m1 );

    friend inline tmpMATRIX operator*( const MATRIX &m0,
                                       const MATRIX &m1 );

    friend inline tmpMATRIX operator*( const MATRIX &m,
                                       double num );
};

/*-------------------------------------------------------------------*
 | How to make a MATRIX out of a tmpMATRIX
 |
 | These routines couldn't be defined within the declaration of
 | MATRIX, because they refer to members of tmpMATRIX that hadn't
 | been declared yet.
 *-------------------------------------------------------------------*/

inline MATRIX::MATRIX( const tmpMATRIX &src ):
    m_numRows( src.m_numRows ),
    m_numCols( src.m_numCols ),
    m_size( src.m_size ),
    m_data( src.stealData() )
{
}

inline MATRIX &MATRIX::operator=( const tmpMATRIX &src )
{
#ifdef TSTBUG
    assert( m_numRows == src.m_numRows && m_numCols == src.m_numCols );
    //THROW_ERR( "Bad assignment -- "
    //<< m_numRows << "x" << m_numCols << " matrix = "
    //<< src.m_numRows << "x" << src.m_numCols
    //<< " matrix" )
#endif

    delete [] m_data;
    m_data = src.stealData();

    return *this;
}

/*-------------------------------------------------------------------*
 | operator+() -- all permutations
 *-------------------------------------------------------------------*/

inline tmpMATRIX operator+( const MATRIX &m0,
                            const MATRIX &m1 )
{
    tmpMATRIX tmp( m0 );
    tmp.add( m1 );
    return tmp;
}

inline tmpMATRIX operator+( const MATRIX &m0,
                            const tmpMATRIX &m1 )
{
    tmpMATRIX tmp( m1 );
    tmp.add( m0 );
    return tmp;
}

/*-------------------------------------------------------------------*
 | operator-() -- all permutations
 *-------------------------------------------------------------------*/

inline tmpMATRIX operator-( const MATRIX &m0,
                            const MATRIX &m1 )
{
    tmpMATRIX tmp( m0 );
    tmp.subtract( m1 );
    return tmp;
}

inline tmpMATRIX operator-( const MATRIX &m0,
                            const tmpMATRIX &m1 )
{
    tmpMATRIX tmp( m1 );
    tmp.subtractFrom( m0 );
    return tmp;
}

/*-------------------------------------------------------------------*
 | operator*() -- all permutations
 *-------------------------------------------------------------------*/

inline tmpMATRIX operator*( const MATRIX &m0,
                            const MATRIX &m1 )
{
    tmpMATRIX tmp( m0, m1 );
    return tmp;
}

inline tmpMATRIX operator*( const MATRIX &m,
                            double num )
{
    tmpMATRIX tmp( m );
    tmp.multiply( num );
    return tmp;
}

#endif

