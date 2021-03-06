/*
   Copyright (c) 2009-2015, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "El.hpp"

namespace El {

template<typename T>
void HCat
( const Matrix<T>& A,
  const Matrix<T>& B,
        Matrix<T>& C )
{
    DEBUG_ONLY(CSE cse("HCat"))
    if( A.Height() != B.Height() )
        LogicError("Incompatible heights for HCat");
    const Int m = A.Height();
    const Int nA = A.Width();
    const Int nB = B.Width();

    Zeros( C, m, nA+nB );
    auto CL = C( IR(0,m), IR(0,nA)     );
    auto CR = C( IR(0,m), IR(nA,nA+nB) );
    CL = A;
    CR = B;
}

template<typename T>
void VCat
( const Matrix<T>& A,
  const Matrix<T>& B,
        Matrix<T>& C )
{
    DEBUG_ONLY(CSE cse("VCat"))
    if( A.Width() != B.Width() )
        LogicError("Incompatible widths for VCat");
    const Int mA = A.Height();
    const Int mB = B.Height();
    const Int n = A.Width();

    Zeros( C, mA+mB, n );
    auto CT = C( IR(0,mA),     IR(0,n) );
    auto CB = C( IR(mA,mA+mB), IR(0,n) );
    CT = A;
    CB = B;
}

template<typename T>
inline void HCat
( const ElementalMatrix<T>& A,
  const ElementalMatrix<T>& B, 
        ElementalMatrix<T>& CPre )
{
    DEBUG_ONLY(CSE cse("Copy"))
    if( A.Height() != B.Height() )
        LogicError("Incompatible heights for HCat");
    const Int m = A.Height();
    const Int nA = A.Width();
    const Int nB = B.Width();

    DistMatrixWriteProxy<T,T,MC,MR> CProx( CPre );
    auto& C = CProx.Get();

    Zeros( C, m, nA+nB );
    auto CL = C( IR(0,m), IR(0,nA)     );
    auto CR = C( IR(0,m), IR(nA,nA+nB) );
    CL = A;
    CR = B;
}

template<typename T>
void VCat
( const ElementalMatrix<T>& A,
  const ElementalMatrix<T>& B, 
        ElementalMatrix<T>& CPre )
{
    DEBUG_ONLY(CSE cse("VCat"))
    if( A.Width() != B.Width() )
        LogicError("Incompatible widths for VCat");
    const Int mA = A.Height();
    const Int mB = B.Height();
    const Int n = A.Width();

    DistMatrixWriteProxy<T,T,MC,MR> CProx( CPre );
    auto& C = CProx.Get();

    Zeros( C, mA+mB, n );
    auto CT = C( IR(0,mA),     IR(0,n) );
    auto CB = C( IR(mA,mA+mB), IR(0,n) );
    CT = A;
    CB = B;
}

template<typename T>
void HCat
( const SparseMatrix<T>& A,
  const SparseMatrix<T>& B, 
        SparseMatrix<T>& C )
{
    DEBUG_ONLY(CSE cse("HCat"))
    if( A.Height() != B.Height() )
        LogicError("Incompatible heights for HCat"); 

    const Int m = A.Height();
    const Int nA = A.Width();
    const Int nB = B.Width();
    
    const Int numEntriesA = A.NumEntries();
    const Int numEntriesB = B.NumEntries();
    Zeros( C, m, nA+nB );
    C.Reserve( numEntriesA+numEntriesB ); 
    for( Int e=0; e<numEntriesA; ++e )
        C.QueueUpdate( A.Row(e), A.Col(e), A.Value(e) );
    for( Int e=0; e<numEntriesB; ++e )
        C.QueueUpdate( B.Row(e), B.Col(e)+nA, B.Value(e) );
    C.ProcessQueues();
}

template<typename T>
void VCat
( const SparseMatrix<T>& A,
  const SparseMatrix<T>& B, 
        SparseMatrix<T>& C )
{
    DEBUG_ONLY(CSE cse("VCat"))
    if( A.Width() != B.Width() )
        LogicError("Incompatible widths for VCat"); 

    const Int mA = A.Height();
    const Int mB = B.Height();
    const Int n = A.Width();
    
    const Int numEntriesA = A.NumEntries();
    const Int numEntriesB = B.NumEntries();
    Zeros( C, mA+mB, n );
    C.Reserve( numEntriesA+numEntriesB ); 
    for( Int e=0; e<numEntriesA; ++e )
        C.QueueUpdate( A.Row(e), A.Col(e), A.Value(e) );
    for( Int e=0; e<numEntriesB; ++e )
        C.QueueUpdate( B.Row(e)+mA, B.Col(e), B.Value(e) );
    C.ProcessQueues();
}

template<typename T>
void HCat
( const DistSparseMatrix<T>& A,
  const DistSparseMatrix<T>& B, 
        DistSparseMatrix<T>& C )
{
    DEBUG_ONLY(CSE cse("HCat"))
    if( A.Height() != B.Height() )
        LogicError("Incompatible heights for HCat"); 
    /*
    if( A.Comm() != B.Comm() )
        LogicError("A and B had different communicators");
    */

    const Int m = A.Height();
    const Int nA = A.Width();
    const Int nB = B.Width();
    
    const Int numEntriesA = A.NumLocalEntries();
    const Int numEntriesB = B.NumLocalEntries();
    C.SetComm( A.Comm() );
    Zeros( C, m, nA+nB );
    C.Reserve( numEntriesA+numEntriesB ); 
    const Int firstLocalRow = C.FirstLocalRow();
    for( Int e=0; e<numEntriesA; ++e )
        C.QueueLocalUpdate( A.Row(e)-firstLocalRow, A.Col(e), A.Value(e) );
    for( Int e=0; e<numEntriesB; ++e )
        C.QueueLocalUpdate( B.Row(e)-firstLocalRow, B.Col(e)+nA, B.Value(e) );
    C.ProcessLocalQueues();
}

template<typename T>
void VCat
( const DistSparseMatrix<T>& A,
  const DistSparseMatrix<T>& B, 
        DistSparseMatrix<T>& C )
{
    DEBUG_ONLY(CSE cse("VCat"))
    if( A.Width() != B.Width() )
        LogicError("Incompatible widths for VCat"); 
    /*
    if( A.Comm() != B.Comm() )
        LogicError("A and B had different communicators");
    */

    const Int mA = A.Height();
    const Int mB = B.Height();
    const Int n = A.Width();
    
    const Int numEntriesA = A.NumLocalEntries();
    const Int numEntriesB = B.NumLocalEntries();
    C.SetComm( A.Comm() );
    Zeros( C, mA+mB, n );
    C.Reserve( numEntriesA+numEntriesB, numEntriesA+numEntriesB );
    for( Int e=0; e<numEntriesA; ++e )
        C.QueueUpdate( A.Row(e), A.Col(e), A.Value(e) );
    for( Int e=0; e<numEntriesB; ++e )
        C.QueueUpdate( B.Row(e)+mA, B.Col(e), B.Value(e) );
    C.ProcessQueues();
}

template<typename T>
void HCat
( const DistMultiVec<T>& A,
  const DistMultiVec<T>& B,
        DistMultiVec<T>& C )
{
    DEBUG_ONLY(CSE cse("HCat"))
    if( A.Height() != B.Height() )
        LogicError("A and B must be the same height for HCat");

    const Int m = A.Height();
    const Int nA = A.Width();
    const Int nB = B.Width();
    mpi::Comm comm = A.Comm();

    C.SetComm( comm );
    Zeros( C, m, nA+nB );

    const Int localHeight = C.LocalHeight();
    const auto& ALoc = A.LockedMatrix();
    const auto& BLoc = B.LockedMatrix();
    auto& CLoc = C.Matrix();
    auto CLocL = CLoc( IR(0,localHeight), IR(0,nA)     );
    auto CLocR = CLoc( IR(0,localHeight), IR(nA,nA+nB) );
    CLocL = ALoc;
    CLocR = BLoc;
}

template<typename T>
void VCat
( const DistMultiVec<T>& A,
  const DistMultiVec<T>& B,
        DistMultiVec<T>& C )
{
    DEBUG_ONLY(CSE cse("VCat"))
    if( A.Width() != B.Width() )
        LogicError("A and B must be the same width for VCat");

    const Int mA = A.Height();
    const Int mB = B.Height();
    const Int mLocA = A.LocalHeight();
    const Int mLocB = B.LocalHeight();
    const Int n = A.Width();

    C.SetComm( A.Comm() );
    Zeros( C, mA+mB, n );
    C.Reserve( (mLocA+mLocB)*n );
    for( Int iLoc=0; iLoc<mLocA; ++iLoc )
    {
        const Int i = A.GlobalRow(iLoc);
        for( Int j=0; j<n; ++j )
            C.QueueUpdate( i, j, A.GetLocal(iLoc,j) );
    }
    for( Int iLoc=0; iLoc<mLocB; ++iLoc )
    {
        const Int i = B.GlobalRow(iLoc) + mA;
        for( Int j=0; j<n; ++j )
            C.QueueUpdate( i, j, B.GetLocal(iLoc,j) );
    }
    C.ProcessQueues();
}

#define PROTO(T) \
  template void HCat \
  ( const Matrix<T>& A, \
    const Matrix<T>& B, \
          Matrix<T>& C ); \
  template void VCat \
  ( const Matrix<T>& A, \
    const Matrix<T>& B, \
          Matrix<T>& C ); \
  template void HCat \
  ( const ElementalMatrix<T>& A, \
    const ElementalMatrix<T>& B, \
          ElementalMatrix<T>& C ); \
  template void VCat \
  ( const ElementalMatrix<T>& A, \
    const ElementalMatrix<T>& B, \
          ElementalMatrix<T>& C ); \
  template void HCat \
  ( const SparseMatrix<T>& A, \
    const SparseMatrix<T>& B, \
          SparseMatrix<T>& C ); \
  template void VCat \
  ( const SparseMatrix<T>& A, \
    const SparseMatrix<T>& B, \
          SparseMatrix<T>& C ); \
  template void HCat \
  ( const DistSparseMatrix<T>& A, \
    const DistSparseMatrix<T>& B, \
          DistSparseMatrix<T>& C ); \
  template void VCat \
  ( const DistSparseMatrix<T>& A, \
    const DistSparseMatrix<T>& B, \
          DistSparseMatrix<T>& C ); \
  template void HCat \
  ( const DistMultiVec<T>& A, \
    const DistMultiVec<T>& B, \
          DistMultiVec<T>& C ); \
  template void VCat \
  ( const DistMultiVec<T>& A, \
    const DistMultiVec<T>& B, \
          DistMultiVec<T>& C );

#define EL_ENABLE_QUAD
#include "El/macros/Instantiate.h"

} // namespace El
