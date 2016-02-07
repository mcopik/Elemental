/*
   Copyright (c) 2009-2015, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "El.hpp"

namespace El {

template<typename F>
pair<Base<F>,Base<F>>
ExtremalSingValEst( const SparseMatrix<F>& A, Int basisSize, bool xtd )
{
    DEBUG_ONLY(CSE cse("ExtremalSingValEst"))
    typedef Base<F> Real;
    Matrix<Real> T;
    HermitianEigSubset<Base<F>> subset;
    ProductLanczos( A, T, basisSize );
    const Int k = T.Height();
    if( k == 0 )
        return pair<Real,Real>(0,0);

    Matrix<Real> d, dSub;
    d = GetDiagonal( T );
    dSub = GetDiagonal( T, -1 );
    
    Matrix<Real> w;
    HermitianTridiagEig( d, dSub, w, ASCENDING, subset, xtd );
    
    pair<Real,Real> extremal;
    extremal.first = Sqrt( Max(w.Get(0,0),Real(0)) );
    extremal.second = Sqrt( Max(w.Get(k-1,0),Real(0)) );
    return extremal;
}

template<typename F>
pair<Base<F>,Base<F>>
ExtremalSingValEst( const DistSparseMatrix<F>& A, Int basisSize, bool xtd )
{
    DEBUG_ONLY(CSE cse("ExtremalSingValEst"))
    typedef Base<F> Real;
    Grid grid( A.Comm() );
    DistMatrix<Real,STAR,STAR> T(grid);
    HermitianEigSubset<Base<F>> subset; 
    ProductLanczos( A, T, basisSize );
    const Int k = T.Height();
    if( k == 0 )
        return pair<Real,Real>(0,0);

    auto d = GetDiagonal( T.Matrix() );
    auto dSub = GetDiagonal( T.Matrix(), -1 );
    
    Matrix<Real> w;
    HermitianTridiagEig( d, dSub, w, ASCENDING, subset, xtd );
    
    pair<Real,Real> extremal;
    extremal.first = Sqrt( Max(w.Get(0,0),Real(0)) );
    extremal.second = Sqrt( Max(w.Get(k-1,0),Real(0)) );
    return extremal;
}

template<typename F>
pair<Base<F>,Base<F>>
HermitianExtremalSingValEst( const SparseMatrix<F>& A, Int basisSize, bool xtd )
{
    DEBUG_ONLY(CSE cse("HermitianExtremalSingValEst"))
    typedef Base<F> Real;
    Matrix<Real> T;
    HermitianEigSubset<Base<F>> subset;
    Lanczos( A, T, basisSize );
    const Int k = T.Height();
    if( k == 0 )
        return pair<Real,Real>(0,0);

    Matrix<Real> d, dSub;
    d = GetDiagonal( T );
    dSub = GetDiagonal( T, -1 );
    
    Matrix<Real> w;
    HermitianTridiagEig( d, dSub, w, ASCENDING, subset, xtd );
    
    pair<Real,Real> extremal;
    extremal.second = MaxNorm(w);
    extremal.first = extremal.second;
    for( Int i=0; i<k; ++i )
        extremal.first = Min(extremal.first,Abs(w.Get(i,0)));
    return extremal;
}

template<typename F>
pair<Base<F>,Base<F>>
HermitianExtremalSingValEst( const DistSparseMatrix<F>& A, Int basisSize, bool xtd )
{
    DEBUG_ONLY(CSE cse("HermitianExtremalSingValEst"))
    typedef Base<F> Real;
    Grid grid( A.Comm() );
    DistMatrix<Real,STAR,STAR> T(grid);
    HermitianEigSubset<Base<F>> subset;
    Lanczos( A, T, basisSize );
    const Int k = T.Height();
    if( k == 0 )
        return pair<Real,Real>(0,0);

    auto d = GetDiagonal( T.Matrix() );
    auto dSub = GetDiagonal( T.Matrix(), -1 );

    Matrix<Real> w;
    HermitianTridiagEig( d, dSub, w, ASCENDING, subset, xtd );

    pair<Real,Real> extremal;
    extremal.second = MaxNorm(w);
    extremal.first = extremal.second;
    for( Int i=0; i<k; ++i )
        extremal.first = Min(extremal.first,Abs(w.Get(i,0)));

    return extremal;
}

#define PROTO(F) \
  template pair<Base<F>,Base<F>> ExtremalSingValEst \
  ( const SparseMatrix<F>& A, Int basisSize, bool xtd ); \
  template pair<Base<F>,Base<F>> ExtremalSingValEst \
  ( const DistSparseMatrix<F>& A, Int basisSize, bool xtd ); \
  template pair<Base<F>,Base<F>> HermitianExtremalSingValEst \
  ( const SparseMatrix<F>& A, Int basisSize, bool xtd ); \
  template pair<Base<F>,Base<F>> HermitianExtremalSingValEst \
  ( const DistSparseMatrix<F>& A, Int basisSize, bool xtd );

#define EL_NO_INT_PROTO
#include "El/macros/Instantiate.h"

} // namespace El
