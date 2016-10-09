/*
   Copyright (c) 2009-2016, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#include <El.hpp>

#include "./Hessenberg/L.hpp"
#include "./Hessenberg/U.hpp"
#include "./Hessenberg/ApplyQ.hpp"
#include "./Hessenberg/FormQ.hpp"

namespace El {

template<typename F>
void Hessenberg
( UpperOrLower uplo,
  Matrix<F>& A,
  Matrix<F>& householderScalars )
{
    DEBUG_CSE
    if( uplo == UPPER )
        hessenberg::U( A, householderScalars );
    else
        hessenberg::L( A, householderScalars );
}

template<typename F> 
void Hessenberg
( UpperOrLower uplo,
  ElementalMatrix<F>& A,
  ElementalMatrix<F>& householderScalars )
{
    DEBUG_CSE
    if( uplo == UPPER )
        hessenberg::U( A, householderScalars );
    else
        hessenberg::L( A, householderScalars );
}

namespace hessenberg {

template<typename F>
void ExplicitCondensed( UpperOrLower uplo, Matrix<F>& A )
{
    DEBUG_CSE
    Matrix<F> householderScalars;
    Hessenberg( uplo, A, householderScalars );
    if( uplo == LOWER )
        MakeTrapezoidal( LOWER, A, 1 );
    else
        MakeTrapezoidal( UPPER, A, -1 );
}

template<typename F> 
void ExplicitCondensed( UpperOrLower uplo, ElementalMatrix<F>& A )
{
    DEBUG_CSE
    DistMatrix<F,STAR,STAR> householderScalars(A.Grid());
    Hessenberg( uplo, A, householderScalars );
    if( uplo == LOWER )
        MakeTrapezoidal( LOWER, A, 1 );
    else
        MakeTrapezoidal( UPPER, A, -1 );
}

} // namespace hessenberg

#define PROTO(F) \
  template void Hessenberg \
  ( UpperOrLower uplo, \
    Matrix<F>& A, \
    Matrix<F>& householderScalars ); \
  template void Hessenberg \
  ( UpperOrLower uplo, \
    ElementalMatrix<F>& A, \
    ElementalMatrix<F>& householderScalars ); \
  template void hessenberg::ExplicitCondensed \
  ( UpperOrLower uplo, Matrix<F>& A ); \
  template void hessenberg::ExplicitCondensed \
  ( UpperOrLower uplo, ElementalMatrix<F>& A ); \
  template void hessenberg::ApplyQ \
  ( LeftOrRight side, UpperOrLower uplo, Orientation orientation, \
    const Matrix<F>& A, \
    const Matrix<F>& householderScalars, \
          Matrix<F>& H ); \
  template void hessenberg::ApplyQ \
  ( LeftOrRight side, UpperOrLower uplo, Orientation orientation, \
    const ElementalMatrix<F>& A, \
    const ElementalMatrix<F>& householderScalars, \
          ElementalMatrix<F>& B ); \
  template void hessenberg::FormQ \
  ( UpperOrLower uplo, \
    const Matrix<F>& A, \
    const Matrix<F>& householderScalars, \
          Matrix<F>& Q ); \
  template void hessenberg::FormQ \
  ( UpperOrLower uplo, \
    const ElementalMatrix<F>& A, \
    const ElementalMatrix<F>& householderScalars, \
          ElementalMatrix<F>& Q );

#define EL_NO_INT_PROTO
#define EL_ENABLE_DOUBLEDOUBLE
#define EL_ENABLE_QUADDOUBLE
#define EL_ENABLE_QUAD
#define EL_ENABLE_BIGFLOAT
#include <El/macros/Instantiate.h>

} // namespace El
