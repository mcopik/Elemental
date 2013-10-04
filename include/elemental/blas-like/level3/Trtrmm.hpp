/*
   Copyright (c) 2009-2013, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef ELEM_BLAS_TRTRMM_HPP
#define ELEM_BLAS_TRTRMM_HPP

#include "./Trtrmm/Unblocked.hpp"
#include "./Trtrmm/LVar1.hpp"
#include "./Trtrmm/UVar1.hpp"

namespace elem {

template<typename T>
inline void
LocalTrtrmm
( UpperOrLower uplo, DistMatrix<T,STAR,STAR>& A, bool conjugate=false )
{
#ifndef RELEASE
    CallStackEntry entry("LocalTrtrmm");
#endif
    Trtrmm( uplo, A.Matrix(), conjugate );
}

template<typename T>
inline void
Trtrmm( UpperOrLower uplo, Matrix<T>& A, bool conjugate=false )
{
#ifndef RELEASE
    CallStackEntry entry("Trtrmm");
    if( A.Height() != A.Width() )
        LogicError("A must be square");
#endif
    if( uplo == LOWER )
        internal::TrtrmmLVar1( A, conjugate );
    else
        internal::TrtrmmUVar1( A, conjugate );
}

template<typename T>
inline void
Trtrmm( UpperOrLower uplo, DistMatrix<T>& A, bool conjugate=false )
{
#ifndef RELEASE
    CallStackEntry entry("Trtrmm");
    if( A.Height() != A.Width() )
        LogicError("A must be square");
#endif
    if( uplo == LOWER )
        internal::TrtrmmLVar1( A, conjugate );
    else
        internal::TrtrmmUVar1( A, conjugate );
}

} // namespace elem

#endif // ifndef ELEM_BLAS_TRTRMM_HPP
