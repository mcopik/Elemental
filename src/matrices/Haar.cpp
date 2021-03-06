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
void Haar( Matrix<F>& A, Int n )
{
    DEBUG_ONLY(CSE cse("Haar"))
    // TODO: Replace this with a quadratic scheme similar to Stewart's, which
    //       essentially generates random Householder reflectors
    Gaussian( A, n, n );
    qr::ExplicitUnitary( A );
}

template<typename F>
void ImplicitHaar( Matrix<F>& A, Matrix<F>& t, Matrix<Base<F>>& d, Int n )
{
    DEBUG_ONLY(CSE cse("ImplicitHaar"))
    // TODO: Replace this with a quadratic scheme similar to Stewart's, which
    //       essentially generates random Householder reflectors
    Gaussian( A, n, n );
    QR( A, t, d );
}

template<typename F>
void Haar( ElementalMatrix<F>& A, Int n )
{
    DEBUG_ONLY(CSE cse("Haar"))
    // TODO: Replace this with a quadratic scheme similar to Stewart's, which
    //       essentially generates random Householder reflectors
    Gaussian( A, n, n );
    qr::ExplicitUnitary( A );
}

template<typename F>
void ImplicitHaar
( ElementalMatrix<F>& A,
  ElementalMatrix<F>& t, 
  ElementalMatrix<Base<F>>& d, Int n )
{
    DEBUG_ONLY(CSE cse("Haar"))
    // TODO: Replace this with a quadratic scheme similar to Stewart's, which
    //       essentially generates random Householder reflectors
    Gaussian( A, n, n );
    QR( A, t, d );
}

#define PROTO(F) \
  template void Haar( Matrix<F>& A, Int n ); \
  template void Haar( ElementalMatrix<F>& A, Int n ); \
  template void ImplicitHaar \
  ( Matrix<F>& A, \
    Matrix<F>& t, \
    Matrix<Base<F>>& d, Int n ); \
  template void ImplicitHaar \
  ( ElementalMatrix<F>& A, \
    ElementalMatrix<F>& t, \
    ElementalMatrix<Base<F>>& d, Int n );

#define EL_NO_INT_PROTO
#include "El/macros/Instantiate.h"

} // namespace El
