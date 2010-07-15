
/*********************************************************************
 * FILE: matrix.C                                                    *
 *                                                                   *
 * AUTHOR: Matthew Miller (mlm)                                      *
 *                                                                   *
 * HISTORY:                                                          *
 *    2 JUN 93 -- (mlm) created                                      *
 *                                                                   *
 * CONTENTS:                                                         *
 *                                                                   *
 *   Routines for matrix operations.  See matrix.H for details.      *
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

#define DECLARE_MATRIX

#include <stdarg.h>
#include <math.h>
#include <iostream>

#include "vector.h"
#include "matrix.h"

/*-------------------------------------------------------------------*
 | Constants
 *-------------------------------------------------------------------*/

static const double TINY = 1e-20;

/*-------------------------------------------------------------------*
 | Static routines
 *-------------------------------------------------------------------*/

static void luSolve( const MATRIX &lu, int *originalRow,
                     double *colBuf );
static int luDecompose( MATRIX &mat, int *originalRow );

/*-------------------------------------------------------------------*
 | MATRIX::operator=( double ) -- fill a matrix with one value
 *-------------------------------------------------------------------*/

MATRIX &MATRIX::operator=( double val )
{
  

  int i;
  double *p = m_data;

  for( i = 0; i < m_size; i++ )
    *p++ = val;

  return *this;
}

/*-------------------------------------------------------------------*
 | MATRIX::set() -- fill a matrix with an array of values
 *-------------------------------------------------------------------*/

void MATRIX::set( double firstVal, ... )
{
  va_list ap;

  va_start( ap, firstVal );
  vset( firstVal, ap );
  va_end( ap );
}

void MATRIX::vset( double firstVal, va_list ap )
{
  

  int i;
  double *p = m_data;

  *p++ = firstVal;

  for( i = 1; i < m_size; i++ )
    *p++ = va_arg( ap, double );
}

/*-------------------------------------------------------------------*
 | MATRIX::isIdentity() -- determine if the matrix is an identity
 |                         matrix
 *-------------------------------------------------------------------*/

int MATRIX::isIdentity() const
{
  

  int row, col;
  double *p = m_data;

  if( m_numRows != m_numCols )
    return 0;

  for( row = 0; row < m_numRows; row++ )
    for( col = 0; col < m_numCols; col++ )
      if( (row == col && *p != 1) ||
          (row != col && *p != 0) )
        return 0;
      else
        p++;

  return 1;
}

/*-------------------------------------------------------------------*
 | MATRIX::trans() -- compute the transpose of a matrix
 *-------------------------------------------------------------------*/

tmpMATRIX MATRIX::trans()
{
  

  tmpMATRIX tmp( m_numCols, m_numRows );
  int row, col;
  double *thisPtr;
  double *tmpPtr;

  thisPtr = m_data;
  tmpPtr = tmp.getData();

  for( row = 0; row < m_numRows; row++ )
  {
    for( col = 0; col < m_numCols; col++ )
    {
      *tmpPtr = *thisPtr++;
      tmpPtr += m_numRows;
    }
    tmpPtr -= m_size - 1;
  }

  return tmp;
}

/*-------------------------------------------------------------------*
 | MATRIX::reduce() -- reduce a matrix to a given number of rows and
 |                     columns
 *-------------------------------------------------------------------*/

tmpMATRIX MATRIX::reduce( int numRows, int numCols )
{
  

  #ifdef TSTBUG
    assert( numRows <= m_numRows && numCols <= m_numCols );
    //  THROW_ERR( "Can't reduce "
    //             << m_numRows << "x" << m_numCols << " matrix "
    //             << "to"
    //             << numRows << "x" << numCols )
  #endif

  tmpMATRIX tmp( numRows, numCols );
  int row, col;
  double *thisPtr;
  double *tmpPtr;

  thisPtr = m_data;
  tmpPtr = tmp.getData();

  for( row = 0; row < numRows; row++ )
  {
    for( col = 0; col < numCols; col++ )
      *tmpPtr++ = *thisPtr++;
    thisPtr += m_numCols - numCols;
  }

  return tmp;
}

/*-------------------------------------------------------------------*
 | MATRIX::inv() -- invert a matrix
 |
 | This uses the LU Decomposition method described in
 |   William H. Press, et al,
 |   NUMERICAL RECIPES IN C
 |   pp 40-47
 *-------------------------------------------------------------------*/

tmpMATRIX MATRIX::inv()
{
  

  static VECTOR_OF< int > originalRow; originalRow.resize( m_numRows );
  static VECTOR_OF< double > colBuf; colBuf.resize( m_numRows );
  MATRIX lu( *this );
  tmpMATRIX tmp( m_numRows, m_numCols );
  double *p0, *p1;
  int row, col;

  luDecompose( lu, &originalRow[ 0 ] );

  for( col = 0; col < m_numCols; col++ )
  {
    colBuf.clear();
    colBuf[ col ] = 1;

    luSolve( lu, &originalRow[ 0 ], &colBuf[ 0 ] );

    p0 = &tmp( 0, col );
    p1 = &colBuf[ 0 ];
    for( row = 0; row < m_numRows; row++ )
    {
      *p0 = *p1++;
      p0 += m_numCols;
    }
  }

  return tmp;
}

/*-------------------------------------------------------------------*
 | MATRIX::det() -- compute the determinant of a matrix
 |
 | This uses the LU Decomposition method described in
 |   William H. Press, et al,
 |   NUMERICAL RECIPES IN C
 |   pp 40-47
 *-------------------------------------------------------------------*/

double MATRIX::det()
{
  

  #ifdef TSTBUG
    assert( m_numRows == m_numCols );
    //  THROW_ERR( "Can't find determinant of "
    //             << m_numRows << "x" << m_numCols << " matrix -- "
    //             << "must be square" )
  #endif

  MATRIX lu( *this );
  static VECTOR_OF< int > dummyBuf; dummyBuf.resize( m_numRows );
  double d;
  int numSwapsWasOdd;
  double *p0;
  int i;

  numSwapsWasOdd = luDecompose( lu, &dummyBuf[ 0 ] );

  p0 = lu.getData();
  d = numSwapsWasOdd ? -*p0 : *p0;
  for( i = 1; i < m_numRows; i++ )
  {
    p0 += m_numCols + 1;
    d *= *p0;
  }

  return d;
}

/*-------------------------------------------------------------------*
 | MATRIX::print() -- print out a matrix (for debugging)
 *-------------------------------------------------------------------*/

void MATRIX::print( int numSpaces ) const
{
  

  int row, col;
  double *p = m_data;

  #define ROUND( v ) (((v) >= 0) ? (int)((v) + .5) : -(int)(-(v) + .5))
  #define DBL( v ) ((double)ROUND( (v) * 10000 ) / 10000)

  for( row = 0; row < m_numRows; row++ )
  {
    Indent( numSpaces );

    for( col = 0; col < m_numCols; col++ )
    {
      std::cout << DBL( *p ) << " ";
      p++;
    }
    std::cout << std::endl;
  }

  #undef DBL
  #undef ROUND
}

/*-------------------------------------------------------------------*
 | tmpMATRIX( MATRIX, MATRIX ) -- construct a temporary matrix as
 |                                the product of two matrices
 *-------------------------------------------------------------------*/

tmpMATRIX::tmpMATRIX( const MATRIX &m0, const MATRIX &m1 ):
  MATRIX( m0.getNumRows(), m1.getNumCols() )
{
  

  #ifdef TSTBUG
    assert( m1.getNumRows() == m0.getNumCols() );
    //  THROW_ERR( "Bad multiply -- "
    //             << m0.getNumRows() << "x" << m0.getNumCols()
    //             << " matrix * "
    //             << m1.getNumRows() << "x" << m1.getNumRows()
    //             << " matrix" )
  #endif

  const int num_m0Cols_m1Rows =      // both the number of rows in m1
              m0.getNumCols();       //   and the number of columns in
                                     //   m0 (they're equal)
  int m0Row;                         // row in m0
  int m0Col_m1Row;                   // both row in m1 and column in m0
  int m1Col;                         // column in m1
  double *thisRowPtr;                // beginning of row m0Row in
                                     //   this->m_data
  double *thisPtr;                   // pointer into this->m_data
  double *m0Ptr;                     // pointer into m0->m_data
  double *m1Ptr;                     // pointer into m1->m_data

  memset( m_data, 0, m_size * sizeof( *m_data ) );

  m0Ptr = m0.getData();
  thisRowPtr = m_data;
  for( m0Row = 0; m0Row < m_numRows; m0Row++ )
  {
    m1Ptr = m1.getData();
    for( m0Col_m1Row = 0;
         m0Col_m1Row < num_m0Cols_m1Rows;
         m0Col_m1Row++ )
    {
      thisPtr = thisRowPtr;
      for( m1Col = 0; m1Col < m_numCols; m1Col++ )
        *thisPtr++ += *m0Ptr * *m1Ptr++;
      m0Ptr++;
    }
    thisRowPtr += m_numCols;
  }
}

/*-------------------------------------------------------------------*
 | tmpMATRIX::add() -- add a matrix to a tmpMATRIX (destructively)
 *-------------------------------------------------------------------*/

tmpMATRIX &tmpMATRIX::add( const MATRIX &src )
{
  

  #ifdef TSTBUG
    assert( m_numRows == src.getNumRows() &&
        m_numCols == src.getNumCols() );
    //  THROW_ERR( "Bad add -- "
    //             << m_numRows << "x" << m_numCols
    //             << " matrix + "
    //             << src.getNumRows() << "x" << src.getNumRows()
    //             << " matrix" )
  #endif

  int i;
  double *p0 = m_data;
  double const * p1 = src.getData();

  for( i = 0; i < m_size; i++ )
    *p0++ += *p1++;

  return *this;
}

/*-------------------------------------------------------------------*
 | tmpMATRIX::subtract() -- subtract a matrix from a tmpMATRIX
 |                          (destructively)
 *-------------------------------------------------------------------*/

tmpMATRIX &tmpMATRIX::subtract( const MATRIX &src )
{
  

  #ifdef TSTBUG
    assert( m_numRows == src.getNumRows() &&
        m_numCols == src.getNumCols() );
    //  THROW_ERR( "Bad subtract -- "
    //             << m_numRows << "x" << m_numCols
    //             << " matrix - "
    //             << src.getNumRows() << "x" << src.getNumRows()
    //             << " matrix" )
  #endif

  int i;
  double *p0 = m_data;
  double const * p1 = src.getData();

  for( i = 0; i < m_size; i++ )
    *p0++ -= *p1++;

  return *this;
}

/*-------------------------------------------------------------------*
 | tmpMATRIX::subtractFrom() -- subtract a tmpMATRIX from a matrix
 |                              (destructively)
 *-------------------------------------------------------------------*/

tmpMATRIX &tmpMATRIX::subtractFrom( const MATRIX &src )
{
  

  #ifdef TSTBUG
    assert( m_numRows == src.getNumRows() &&
        m_numCols == src.getNumCols() );
    //  THROW_ERR( "Bad subtract -- "
    //             << m_numRows << "x" << m_numCols
    //             << " matrix - "
    //             << src.getNumRows() << "x" << src.getNumRows()
    //             << " matrix" )
  #endif

  int i;
  double *p0 = m_data;
  double const * p1 = src.getData();

  for( i = 0; i < m_size; i++ )
  {
    *p0 = *p1 - *p0;
    p0++;
    p1++;
  }

  return *this;
}

/*-------------------------------------------------------------------*
 | tmpMATRIX::multiply() -- multiply a tmpMATRIX by a scaler value
 |                          (destructively)
 *-------------------------------------------------------------------*/

tmpMATRIX &tmpMATRIX::multiply( double src )
{
  

  int i;
  double *p = m_data;

  for( i = 0; i < m_size; i++ )
    *p++ *= src;

  return *this;
}

/*-------------------------------------------------------------------*
 | luSolve() -- solve the set of simultaneous equations described in
 |              an LU Decomposition
 |
 | This uses the LU Decomposition method described in
 |   William H. Press, et al,
 |   NUMERICAL RECIPES IN C
 |   pp 40-47
 |
 | The matrix returned by luDecompose, together with the array
 | originalRow, describe a matrix as an LU Decomposition (see below
 | for a description of how to reconstruct the matrix from the
 | decomposition).
 |
 | This interprets the decomposed matrix as the coefficients in a
 | set of simultaneous equations.  It is given the set of
 | right-hand-sides of those equations, and solves for the variables.
 |
 | For example, if we have the set of equations:
 |
 |   4x + 2y = 3
 |   7x + 8y = 21
 |
 | Then, we'd begin by calling luDecompose() with the matrix
 |
 |   4 2
 |   7 8
 |
 | Then, we'd pass the resulting matrix and originalRow array to
 | luSolve(), together with colBuf = { 3, 21 }.  After luSolve(),
 | colBuf contains the values of x and y.
 |
 | NOTE: The code in NUMERICAL RECIPES is not very readable, and not
 | terribly efficient.  I've tried to improve things a bit here, but
 | I don't think I took enough time to understand the algorithm
 | thoroughly.  The variable names have suffered, and might be more
 | confusing than the i's, j's and k's that they replace.
 |
 | My apologies.
 *-------------------------------------------------------------------*/

static void luSolve( const MATRIX &lu, int *originalRow,
                     double *colBuf )
{
  

  int firstNonZeroRow = -1;
  double sum = -1;
  int row;
  double *rowPtr;
  double *p;
  int i;

  /* solve the L part of problem by forward substitution */

  /* this first loop solves up to the first row where the solution
     is not zero */
  for( row = 0; row < lu.getNumRows(); row++ )
  {
    i = originalRow[ row ];
    sum = colBuf[ i ];
    colBuf[ i ] = colBuf[ row ];

    colBuf[ row ] = sum;

    if( sum != 0 )
      break;
  }

  /* this second loop solves the rest */
  if( row < lu.getNumRows() )
  {
    firstNonZeroRow = row;
    rowPtr = lu.getData() + row * lu.getNumCols();

    while( ++row < lu.getNumRows() )
    {
      i = originalRow[ row ];
      sum = colBuf[ i ];
      colBuf[ i ] = colBuf[ row ];

      rowPtr += lu.getNumCols();
      for( i = firstNonZeroRow; i < row; i++ )
        sum -= rowPtr[ i ] * colBuf[ i ];

      colBuf[ row ] = sum;
    }
  }
  else
    rowPtr = lu.getData() + (row - 1) * lu.getNumCols();

  /* solve the U part of the problem by backward substitution */
  p = lu.getData() + row * (lu.getNumCols() + 1);
  while( --row >= 0 )
  {
    p -= lu.getNumCols() + 1;

    sum = colBuf[ row ];

    for( i = row + 1; i < lu.getNumRows(); i++ )
      sum -= rowPtr[ i ] * colBuf[ i ];

    colBuf[ row ] = sum / *p;

    rowPtr -= lu.getNumCols();
  }
}

/*-------------------------------------------------------------------*
 | luDecompose() -- compute the LU Decomposition of a matrix
 |
 | This uses the LU Decomposition method described in
 |   William H. Press, et al,
 |   NUMERICAL RECIPES IN C
 |   pp 40-47
 |
 | To understand what this routine is doing, imagine constructing
 | the following matrices after it has run:
 |
 |   L( r, c ) = mat( r, c ) when r > c
 |               0, when r < c
 |               1, when r == c
 |     (This is the lower left triangle of mat, excluding mat's
 |     diagonal.  All the diagonal elements are set to 1.  All
 |     others are set to 0.)
 |
 |   U( r, c ) = mat( r, c ) when r <= c
 |               0, when r > c
 |     (This is the upper right triangle of mat, including mat's
 |     diagonal.  All other elements are set to 0.)
 |
 |   P( r, c ) = 1, when originalRow[ r ] == c
 |               0, when originalRow[ r ] != c
 |     (This is the "pivot" matrix.  It records a shuffling of the
 |     rows that was performed to reduce round-off and overflow
 |     errors.)
 |
 | The following property holds:
 |
 |   P * L * U == the original value of mat, before luDecompose()
 |
 | This is useful because it's easy to invert triangular matrices
 | like L and U.
 |
 | NOTE: The code in NUMERICAL RECIPES is not very readable, and not
 | terribly efficient.  I've tried to improve things a bit here, but
 | I don't think I took enough time to understand the algorithm
 | thoroughly.  The variable names have suffered, and might be more
 | confusing than the i's, j's and k's that they replace.
 |
 | My apologies.
 *-------------------------------------------------------------------*/

static int luDecompose( MATRIX &mat, int *originalRow )
{
  

  int numSwapsIsOdd = 0;
  int row, col;
  double *rowPtr, *colPtr;
  double *p0, *p1, *p2;
  const int numRows = mat.getNumRows();
  #define numCols numRows
  double sum;
  static VECTOR_OF< double > scaler; scaler.resize( numRows );
  double biggest;
  int biggestRow;
  double tmpDbl;
  static VECTOR_OF< double > tmpDblArray;
                             tmpDblArray.resize( numCols );
  int i;

  p0 = mat.getData();
  for( row = 0; row < numRows; row++ )
  {
    biggest = fabs( *p0++ );
    for( col = 1; col < numCols; col++ )
      if( (tmpDbl = fabs( *p0++ )) > biggest )
        biggest = tmpDbl;

    #ifdef TSTBUG
      assert( biggest != 0 );
      //  THROW_ERR( "Trying to LU-decompose singular matrix" );
    #endif

    scaler[ row ] = 1. / biggest;
  }

  colPtr = mat.getData();
  for( col = 0; col < numCols; col++ )
  {
    rowPtr = mat.getData();
    p0 = colPtr;
    for( row = 0; row < col; row++ )
    {
      sum = *p0;
      p1 = rowPtr;
      p2 = colPtr;
      for( i = 0; i < row; i++ )
      {
        sum -= *p1++ * *p2;
        p2 += numCols;
      }
      *p0 = sum;
      p0 += numCols;
      rowPtr += numCols;
    }

    biggest = 0;
    for( row = col; row < numRows; row++ )
    {
      sum = *p0;
      p1 = rowPtr;
      p2 = colPtr;
      for( i = 0; i < col; i++ )
      {
        sum -= *p1++ * *p2;
        p2 += numCols;
      }
      *p0 = sum;
      p0 += numCols;
      rowPtr += numCols;

      if( (tmpDbl = scaler[ row ] * fabs( sum )) >= biggest )
      {
        biggest = tmpDbl;
        biggestRow = row;
      }
    }

    if( col != biggestRow )
    {
      p0 = &mat( biggestRow, 0 );
      p1 = &mat( col, 0 );
      memcpy( &tmpDblArray[ 0 ], p0,
              numCols * sizeof( tmpDblArray[ 0 ] ) );
      memcpy( p0, p1, numCols * sizeof( *mat.getData() ) );
      memcpy( p1, &tmpDblArray[ 0 ],
              numCols * sizeof( *mat.getData() ) );

      scaler[ biggestRow ] = scaler[ col ];

      numSwapsIsOdd ^= 1;
    }

    originalRow[ col ] = biggestRow;

    p0 = &mat( col, col );
    if( *p0 == 0 )
      *p0 = TINY;

    if( col != numCols - 1 )
    {
      tmpDbl = 1. / *p0;
      p1 = colPtr + (col + 1) * numCols;
      for( row = col + 1; row < numRows; row++ )
      {
        *p1 *= tmpDbl;
        p1 += numCols;
      }
    }

    colPtr++;
  }

  return numSwapsIsOdd;

  #undef numCols
}

