/*
   Copyright (c) 2009-2015, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "El.hpp"
#include "El/blas_like/level1/copy_internal.hpp"

namespace El {
namespace copy {

template<typename T,Dist U,Dist V>
void ColAllToAllPromote
( const DistMatrix<T,        U,                     V   >& A,
        DistMatrix<T,Partial<U>(),PartialUnionRow<U,V>()>& B )
{
    DEBUG_ONLY(CSE cse("copy::ColAllToAllPromote"))
    AssertSameGrids( A, B );

    const Int height = A.Height();
    const Int width = A.Width();
    B.AlignColsAndResize
    ( Mod(A.ColAlign(),B.ColStride()), height, width, false, false );
    if( !B.Participating() )
        return;

    const Int colStride = A.ColStride();
    const Int colStridePart = A.PartialColStride();
    const Int colStrideUnion = A.PartialUnionColStride();
    const Int colRankPart = A.PartialColRank();
    const Int colDiff = B.ColAlign() - Mod(A.ColAlign(),colStridePart);

    const Int maxLocalHeight = MaxLength(height,colStride);
    const Int maxLocalWidth = MaxLength(width,colStrideUnion);
    const Int portionSize = mpi::Pad( maxLocalHeight*maxLocalWidth );

    if( colDiff == 0 )
    {
        if( A.PartialUnionColStride() == 1 )
        {
            Copy( A.LockedMatrix(), B.Matrix() );
        }
        else
        {
            vector<T> buffer;
            FastResize( buffer, 2*colStrideUnion*portionSize );
            T* firstBuf  = &buffer[0];
            T* secondBuf = &buffer[colStrideUnion*portionSize];

            // Pack            
            util::RowStridedPack
            ( A.LocalHeight(), width,
              B.RowAlign(), colStrideUnion,
              A.LockedBuffer(), A.LDim(),
              firstBuf,         portionSize );

            // Simultaneously Gather in columns and Scatter in rows
            mpi::AllToAll
            ( firstBuf,  portionSize,
              secondBuf, portionSize, A.PartialUnionColComm() );

            // Unpack
            util::PartialColStridedUnpack 
            ( height, B.LocalWidth(),
              A.ColAlign(), colStride,
              colStrideUnion, colStridePart, colRankPart,
              B.ColShift(),
              secondBuf,  portionSize,
              B.Buffer(), B.LDim() );
        }
    }
    else
    {
#ifdef EL_UNALIGNED_WARNINGS
        if( A.Grid().Rank() == 0 )
            cerr << "Unaligned PartialColAllToAllPromote" << endl;
#endif
        const Int sendColRankPart = Mod( colRankPart+colDiff, colStridePart );
        const Int recvColRankPart = Mod( colRankPart-colDiff, colStridePart );

        vector<T> buffer;
        FastResize( buffer, 2*colStrideUnion*portionSize );
        T* firstBuf  = &buffer[0];
        T* secondBuf = &buffer[colStrideUnion*portionSize];

        // Pack
        util::RowStridedPack
        ( A.LocalHeight(), width,
          B.RowAlign(), colStrideUnion,
          A.LockedBuffer(), A.LDim(),
          secondBuf,        portionSize );

        // Realign the input
        mpi::SendRecv
        ( secondBuf, colStrideUnion*portionSize, sendColRankPart,
          firstBuf,  colStrideUnion*portionSize, recvColRankPart,
          A.PartialColComm() );

        // Simultaneously Scatter in columns and Gather in rows
        mpi::AllToAll
        ( firstBuf,  portionSize,
          secondBuf, portionSize, A.PartialUnionColComm() );

        // Unpack
        util::PartialColStridedUnpack 
        ( height, B.LocalWidth(),
          A.ColAlign(), colStride,
          colStrideUnion, colStridePart, recvColRankPart,
          B.ColShift(),
          secondBuf,  portionSize,
          B.Buffer(), B.LDim() );
    }
}

template<typename T,Dist U,Dist V>
void ColAllToAllPromote
( const DistMatrix<T,        U,                     V   ,BLOCK>& A,
        DistMatrix<T,Partial<U>(),PartialUnionRow<U,V>(),BLOCK>& B )
{
    DEBUG_ONLY(CSE cse("copy::ColAllToAllPromote"))
    AssertSameGrids( A, B );
    // TODO: More efficient implementation
    GeneralPurpose( A, B );
}

#define PROTO_DIST(T,U,V) \
  template void ColAllToAllPromote \
  ( const DistMatrix<T,        U,                     V   >& A, \
          DistMatrix<T,Partial<U>(),PartialUnionRow<U,V>()>& B ); \
  template void ColAllToAllPromote \
  ( const DistMatrix<T,        U,                     V   ,BLOCK>& A, \
          DistMatrix<T,Partial<U>(),PartialUnionRow<U,V>(),BLOCK>& B );

#define PROTO(T) \
  PROTO_DIST(T,CIRC,CIRC) \
  PROTO_DIST(T,MC,  MR  ) \
  PROTO_DIST(T,MC,  STAR) \
  PROTO_DIST(T,MD,  STAR) \
  PROTO_DIST(T,MR,  MC  ) \
  PROTO_DIST(T,MR,  STAR) \
  PROTO_DIST(T,STAR,MC  ) \
  PROTO_DIST(T,STAR,MD  ) \
  PROTO_DIST(T,STAR,MR  ) \
  PROTO_DIST(T,STAR,STAR) \
  PROTO_DIST(T,STAR,VC  ) \
  PROTO_DIST(T,STAR,VR  ) \
  PROTO_DIST(T,VC,  STAR) \
  PROTO_DIST(T,VR,  STAR) 

#define EL_ENABLE_QUAD
#include "El/macros/Instantiate.h"

} // namespace copy
} // namespace El
