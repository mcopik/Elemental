/*
   Copyright (c) 2009-2015, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/

// This file should be included into each of the DistMatrix specializations
// as a workaround for the fact that C++11 constructor inheritance is not 
// yet widely supported.

#include "El/blas_like/level1/copy_internal.hpp"

namespace El {

#define DM DistMatrix<T,COLDIST,ROWDIST>
#define EM ElementalMatrix<T>
#define ADM AbstractDistMatrix<T>

// Public section
// ##############

// Constructors and destructors
// ============================

template<typename T>
DM::DistMatrix( const El::Grid& grid, int root )
: EM(grid,root)
{ 
    if( COLDIST == CIRC && ROWDIST == CIRC )
        this->matrix_.SetViewType( OWNER );
    this->SetShifts(); 
}

template<typename T>
DM::DistMatrix( Int height, Int width, const El::Grid& grid, int root )
: EM(grid,root)
{ 
    if( COLDIST == CIRC && ROWDIST == CIRC )
        this->matrix_.SetViewType( OWNER );
    this->SetShifts(); 
    this->Resize(height,width); 
}

template<typename T>
DM::DistMatrix( const DM& A )
: EM(A.Grid())
{
    DEBUG_ONLY(CSE cse("DistMatrix::DistMatrix"))
    if( COLDIST == CIRC && ROWDIST == CIRC )
        this->matrix_.SetViewType( OWNER );
    this->SetShifts();
    if( &A != this )
        *this = A;
    else
        LogicError("Tried to construct DistMatrix with itself");
}

template<typename T>
template<Dist U,Dist V>
DM::DistMatrix( const DistMatrix<T,U,V>& A )
: EM(A.Grid())
{
    DEBUG_ONLY(CSE cse("DistMatrix::DistMatrix"))
    if( COLDIST == CIRC && ROWDIST == CIRC )
        this->matrix_.SetViewType( OWNER );
    this->SetShifts();
    if( COLDIST != U || ROWDIST != V ||
        reinterpret_cast<const DM*>(&A) != this )
        *this = A;
    else
        LogicError("Tried to construct DistMatrix with itself");
}

template<typename T>
DM::DistMatrix( const AbstractDistMatrix<T>& A )
: EM(A.Grid())
{
    DEBUG_ONLY(CSE cse("DM(ADM)"))
    if( COLDIST == CIRC && ROWDIST == CIRC )
        this->matrix_.SetViewType( OWNER );
    this->SetShifts();

    if( A.Wrap() == ELEMENT )
    {
        #define GUARD(CDIST,RDIST) A.ColDist() == CDIST && A.RowDist() == RDIST
        #define PAYLOAD(CDIST,RDIST) \
          auto& ACast = static_cast<const DistMatrix<T,CDIST,RDIST>&>(A); \
          if( COLDIST != CDIST || ROWDIST != RDIST || \
              reinterpret_cast<const DM*>(&A) != this ) \
              *this = ACast; \
          else \
              LogicError("Tried to construct DistMatrix with itself");
        #include "El/macros/GuardAndPayload.h"     
    }
    else
    {
        #define GUARD(CDIST,RDIST) A.ColDist() == CDIST && A.RowDist() == RDIST
        #define PAYLOAD(CDIST,RDIST) \
          auto& ACast = \
            static_cast<const DistMatrix<T,CDIST,RDIST,BLOCK>&>(A); \
          *this = ACast;
        #include "El/macros/GuardAndPayload.h"     
    }
}

template<typename T>
DM::DistMatrix( const ElementalMatrix<T>& A )
: EM(A.Grid())
{
    DEBUG_ONLY(CSE cse("DM(EM)"))
    if( COLDIST == CIRC && ROWDIST == CIRC )
        this->matrix_.SetViewType( OWNER );
    this->SetShifts();
    #define GUARD(CDIST,RDIST) \
      A.DistData().colDist == CDIST && A.DistData().rowDist == RDIST
    #define PAYLOAD(CDIST,RDIST) \
      auto& ACast = static_cast<const DistMatrix<T,CDIST,RDIST>&>(A); \
      if( COLDIST != CDIST || ROWDIST != RDIST || \
          reinterpret_cast<const DM*>(&A) != this ) \
          *this = ACast; \
      else \
          LogicError("Tried to construct DistMatrix with itself");
    #include "El/macros/GuardAndPayload.h"
}

template<typename T>
template<Dist U,Dist V>
DM::DistMatrix( const DistMatrix<T,U,V,BLOCK>& A )
: EM(A.Grid())
{
    DEBUG_ONLY(CSE cse("DistMatrix::DistMatrix"))
    if( COLDIST == CIRC && ROWDIST == CIRC )
        this->matrix_.SetViewType( OWNER );
    this->SetShifts();
    *this = A;
}

template<typename T>
DM::DistMatrix( DM&& A ) EL_NO_EXCEPT : EM(std::move(A)) { }

template<typename T> DM::~DistMatrix() { }

template<typename T> 
DistMatrix<T,COLDIST,ROWDIST>* DM::Copy() const
{ return new DistMatrix<T,COLDIST,ROWDIST>(*this); }

template<typename T> 
DistMatrix<T,COLDIST,ROWDIST>* DM::Construct
( const El::Grid& g, int root ) const
{ return new DistMatrix<T,COLDIST,ROWDIST>(g,root); }

template<typename T> 
DistMatrix<T,ROWDIST,COLDIST>* DM::ConstructTranspose
( const El::Grid& g, int root ) const
{ return new DistMatrix<T,ROWDIST,COLDIST>(g,root); }

template<typename T>
DistMatrix<T,DiagCol<COLDIST,ROWDIST>(),
             DiagRow<COLDIST,ROWDIST>()>* 
DM::ConstructDiagonal
( const El::Grid& g, int root ) const
{ return new DistMatrix<T,DiagCol<COLDIST,ROWDIST>(),
                          DiagRow<COLDIST,ROWDIST>()>(g,root); }

// Operator overloading
// ====================

// Return a view
// -------------
template<typename T>
DM DM::operator()( Range<Int> I, Range<Int> J )
{
    DEBUG_ONLY(CSE cse("DM( ind, ind )"))
    return View( *this, I, J );
}

template<typename T>
const DM DM::operator()( Range<Int> I, Range<Int> J ) const
{
    DEBUG_ONLY(CSE cse("DM( ind, ind ) const"))
    return LockedView( *this, I, J );
}

// Non-contiguous
// --------------
template<typename T>
DM DM::operator()( Range<Int> I, const vector<Int>& J ) const
{
    DEBUG_ONLY(CSE cse("DM( ind, vec ) const"))
    DM ASub( this->Grid() );
    GetSubmatrix( *this, I, J, ASub ); 
    return ASub;
}

template<typename T>
DM DM::operator()( const vector<Int>& I, Range<Int> J ) const
{
    DEBUG_ONLY(CSE cse("DM( vec, ind ) const"))
    DM ASub( this->Grid() );
    GetSubmatrix( *this, I, J, ASub );
    return ASub;
}

template<typename T>
DM DM::operator()( const vector<Int>& I, const vector<Int>& J ) const
{
    DEBUG_ONLY(CSE cse("DM( vec, vec ) const"))
    DM ASub( this->Grid() );
    GetSubmatrix( *this, I, J, ASub );
    return ASub;
}

// Copy
// ----
template<typename T>
DM& DM::operator=( const DM& A )
{
    DEBUG_ONLY(CSE cse("DM = DM"))
    copy::Translate( A, *this );
    return *this;
}

template<typename T>
DM& DM::operator=( const AbstractDistMatrix<T>& A )
{
    DEBUG_ONLY(CSE cse("DM = ADM"))
    // TODO: Use either AllGather or Gather if the distribution of this matrix
    //       is respectively either (STAR,STAR) or (CIRC,CIRC)
    if( A.Wrap() == ELEMENT )
    {
        #define GUARD(CDIST,RDIST) A.ColDist() == CDIST && A.RowDist() == RDIST
        #define PAYLOAD(CDIST,RDIST) \
          auto& ACast = static_cast<const DistMatrix<T,CDIST,RDIST>&>(A); \
          *this = ACast;
        #include "El/macros/GuardAndPayload.h"     
    }
    else
    {
        #define GUARD(CDIST,RDIST) A.ColDist() == CDIST && A.RowDist() == RDIST
        #define PAYLOAD(CDIST,RDIST) \
          auto& ACast = \
            static_cast<const DistMatrix<T,CDIST,RDIST,BLOCK>&>(A); \
          *this = ACast;
        #include "El/macros/GuardAndPayload.h"     
    }
    return *this;
}

template<typename T>
template<Dist U,Dist V>
DM& DM::operator=( const DistMatrix<T,U,V,BLOCK>& A )
{
    DEBUG_ONLY(CSE cse("DM = BDM[U,V]"))
    // TODO: Use either AllGather or Gather if the distribution of this matrix
    //       is respectively either (STAR,STAR) or (CIRC,CIRC)
    const bool elemColCompat = ( A.BlockHeight() == 1 || A.ColStride() == 1 );
    const bool elemRowCompat = ( A.BlockWidth() == 1 || A.RowStride() == 1 );
    if( elemColCompat && elemRowCompat )
    {
        DistMatrix<T,U,V> AElemView(A.Grid());
        AElemView.LockedAttach
        ( A.Height(), A.Width(), A.Grid(),
          A.ColAlign(), A.RowAlign(), A.LockedBuffer(), A.LDim(), A.Root() );
        *this = AElemView;
    }
    else
    {
        copy::GeneralPurpose( A, *this );
    }
    return *this;
}

template<typename T>
DM& DM::operator=( DM&& A )
{
    DEBUG_ONLY(CSE cse("DM = DM&&"))
    if( this->Viewing() || A.Viewing() )
        this->operator=( (const DM&)A );
    else
        EM::operator=( std::move(A) );
    return *this;
}

// Rescaling
// ---------
template<typename T>
const DM& DM::operator*=( T alpha )
{
    DEBUG_ONLY(CSE cse("DM *= T"))
    Scale( alpha, *this );
    return *this;
}

// Addition/subtraction
// --------------------
template<typename T>
const DM& DM::operator+=( const EM& A )
{
    DEBUG_ONLY(CSE cse("DM += DM&"))
    Axpy( T(1), A, *this );
    return *this;
}

template<typename T>
const DM& DM::operator+=( const ADM& A )
{
    DEBUG_ONLY(CSE cse("DM += ADM&"))
    Axpy( T(1), A, *this );
    return *this;
}

template<typename T>
const DM& DM::operator-=( const EM& A )
{
    DEBUG_ONLY(CSE cse("DM -= DM&"))
    Axpy( T(-1), A, *this );
    return *this;
}

template<typename T>
const DM& DM::operator-=( const ADM& A )
{
    DEBUG_ONLY(CSE cse("DM -= ADM&"))
    Axpy( T(-1), A, *this );
    return *this;
}

// Distribution data
// =================

template<typename T>
ElementalData DM::DistData() const { return ElementalData(*this); }

template<typename T>
Dist DM::ColDist() const EL_NO_EXCEPT { return COLDIST; }
template<typename T>
Dist DM::RowDist() const EL_NO_EXCEPT { return ROWDIST; }

template<typename T>
Dist DM::PartialColDist() const EL_NO_EXCEPT { return Partial<COLDIST>(); }
template<typename T>
Dist DM::PartialRowDist() const EL_NO_EXCEPT { return Partial<ROWDIST>(); }

template<typename T>
Dist DM::PartialUnionColDist() const EL_NO_EXCEPT
{ return PartialUnionCol<COLDIST,ROWDIST>(); }
template<typename T>
Dist DM::PartialUnionRowDist() const EL_NO_EXCEPT
{ return PartialUnionRow<COLDIST,ROWDIST>(); }

template<typename T>
Dist DM::CollectedColDist() const EL_NO_EXCEPT { return Collect<COLDIST>(); }
template<typename T>
Dist DM::CollectedRowDist() const EL_NO_EXCEPT { return Collect<ROWDIST>(); }

} // namespace El
