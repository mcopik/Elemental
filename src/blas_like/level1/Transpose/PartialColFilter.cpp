/*
   Copyright (c) 2009-2015, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "El.hpp"

namespace El {
namespace transpose {

// (V,Partial(U)) |-> (U,V)
template<typename T>
void PartialColFilter
( const ElementalMatrix<T>& A, 
        ElementalMatrix<T>& B, bool conjugate )
{
    DEBUG_ONLY(
      CSE cse("transpose::PartialColFilter");
      if( A.ColDist() != B.RowDist() ||
          A.RowDist() != Partial(B.ColDist()) )
          LogicError("Incompatible distributions");
    )
    unique_ptr<ElementalMatrix<T>>
        AFilt( B.ConstructTranspose(B.Grid(),B.Root()) );
    if( B.ColConstrained() )
        AFilt->AlignRowsWith( B, false );
    if( B.RowConstrained() )
        AFilt->AlignColsWith( B, false );
    Copy( A, *AFilt );
    if( !B.ColConstrained() )
        B.AlignColsWith( *AFilt, false );
    if( !B.RowConstrained() )
        B.AlignRowsWith( *AFilt, false );
    B.Resize( A.Width(), A.Height() );
    Transpose( AFilt->LockedMatrix(), B.Matrix(), conjugate );
}

template<typename T>
void PartialColFilter
( const BlockMatrix<T>& A, 
        BlockMatrix<T>& B, bool conjugate )
{
    DEBUG_ONLY(
      CSE cse("transpose::PartialColFilter");
      if( A.ColDist() != B.RowDist() ||
          A.RowDist() != Partial(B.ColDist()) )
          LogicError("Incompatible distributions");
    )
    unique_ptr<BlockMatrix<T>>
        AFilt( B.ConstructTranspose(B.Grid(),B.Root()) );
    if( B.ColConstrained() )
        AFilt->AlignRowsWith( B, false );
    if( B.RowConstrained() )
        AFilt->AlignColsWith( B, false );
    Copy( A, *AFilt );
    if( !B.ColConstrained() )
        B.AlignColsWith( *AFilt, false );
    if( !B.RowConstrained() )
        B.AlignRowsWith( *AFilt, false );
    B.Resize( A.Width(), A.Height() );
    Transpose( AFilt->LockedMatrix(), B.Matrix(), conjugate );
}

#define PROTO(T) \
  template void PartialColFilter \
  ( const ElementalMatrix<T>& A, \
          ElementalMatrix<T>& B, bool conjugate ); \
  template void PartialColFilter \
  ( const BlockMatrix<T>& A, \
          BlockMatrix<T>& B, bool conjugate );

#define EL_ENABLE_QUAD
#include "El/macros/Instantiate.h"

} // namespace transpose
} // namespace El
