/*
   Copyright (c) 2009-2015, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "El.hpp"

#define COLDIST VR
#define ROWDIST STAR

#include "./setup.hpp"

namespace El {

// Public section
// ##############

// Assignment and reconfiguration
// ==============================

// Make a copy
// -----------
template<typename T>
DM& DM::operator=( const DistMatrix<T,MC,MR>& A )
{ 
    DEBUG_ONLY(CSE cse("[VR,STAR] = [MC,MR]"))
    DistMatrix<T,VC,STAR> A_VC_STAR( A );
    *this = A_VC_STAR;
    return *this;
}

template<typename T>
DM& DM::operator=( const DistMatrix<T,MC,STAR>& A )
{ 
    DEBUG_ONLY(CSE cse("[VR,STAR] = [MC,STAR]"))
    DistMatrix<T,VC,STAR> A_VC_STAR( A );
    *this = A_VC_STAR;
    return *this;
}

template<typename T>
DM& DM::operator=( const DistMatrix<T,STAR,MR>& A )
{ 
    DEBUG_ONLY(CSE cse("[VR,STAR] = [STAR,MR]"))
    auto A_MC_MR = MakeUnique<DistMatrix<T,MC,MR>>( A );
    auto A_VC_STAR = MakeUnique<DistMatrix<T,VC,STAR>>( *A_MC_MR );
    A_MC_MR.reset(); 
    *this = *A_VC_STAR;
    return *this;
}

template<typename T>
DM& DM::operator=( const DistMatrix<T,MD,STAR>& A )
{
    DEBUG_ONLY(CSE cse("[VR,STAR] = [MD,STAR]"))
    // TODO: Optimize this later if important
    DistMatrix<T,STAR,STAR> A_STAR_STAR( A );
    *this = A_STAR_STAR;
    return *this;
}

template<typename T>
DM& DM::operator=( const DistMatrix<T,STAR,MD>& A )
{ 
    DEBUG_ONLY(CSE cse("[VR,STAR] = [STAR,MD]"))
    // TODO: Optimize this later if important
    DistMatrix<T,STAR,STAR> A_STAR_STAR( A );
    *this = A_STAR_STAR;
    return *this;
}

template<typename T>
DM& DM::operator=( const DistMatrix<T,MR,MC>& A )
{ 
    DEBUG_ONLY(CSE cse("[VR,STAR] = [MR,MC]"))
    copy::ColAllToAllDemote( A, *this );
    return *this;
}

template<typename T>
DM& DM::operator=( const DistMatrix<T,MR,STAR>& A )
{ 
    DEBUG_ONLY(CSE cse("[VR,STAR] = [MR,STAR]"))
    copy::PartialColFilter( A, *this );
    return *this;
}

template<typename T>
DM& DM::operator=( const DistMatrix<T,STAR,MC>& A )
{ 
    DEBUG_ONLY(CSE cse("[VR,STAR] = [STAR,MC]"))
    DistMatrix<T,MR,MC> A_MR_MC( A );
    *this = A_MR_MC;
    return *this;
}

template<typename T>
DM& DM::operator=( const DistMatrix<T,VC,STAR>& A )
{ 
    DEBUG_ONLY(CSE cse("[VR,STAR] = [VC,STAR]"))
    copy::ColwiseVectorExchange<T,MC,MR>( A, *this );
    return *this;
}

template<typename T>
DM& DM::operator=( const DistMatrix<T,STAR,VC>& A )
{ 
    DEBUG_ONLY(CSE cse("[VR,STAR] = [STAR,VC]"))
    DistMatrix<T,MR,MC> A_MR_MC( A );
    *this = A_MR_MC;
    return *this;
}

template<typename T>
DM& DM::operator=( const DistMatrix<T,STAR,VR>& A )
{ 
    DEBUG_ONLY(CSE cse("[VR,STAR] = [STAR,VR]"))
    auto A_MC_MR = MakeUnique<DistMatrix<T,MC,MR>>( A );
    auto A_VC_STAR = MakeUnique<DistMatrix<T,VC,STAR>>( *A_MC_MR );
    A_MC_MR.reset(); 
    *this = *A_VC_STAR;
    return *this;
}

template<typename T>
DM& DM::operator=( const DistMatrix<T,STAR,STAR>& A )
{
    DEBUG_ONLY(CSE cse("[VR,STAR] = [STAR,STAR]"))
    copy::ColFilter( A, *this );
    return *this;
}

template<typename T>
DM& DM::operator=( const DistMatrix<T,CIRC,CIRC>& A )
{
    DEBUG_ONLY(CSE cse("[VR,STAR] = [CIRC,CIRC]"))
    copy::Scatter( A, *this );
    return *this;
}

template<typename T>
DM& DM::operator=( const AbstractDistMatrix<T>& A )
{
    DEBUG_ONLY(CSE cse("DM = ADM"))
    #define GUARD(CDIST,RDIST) \
      A.DistData().colDist == CDIST && A.DistData().rowDist == RDIST
    #define PAYLOAD(CDIST,RDIST) \
      auto& ACast = dynamic_cast<const DistMatrix<T,CDIST,RDIST>&>(A); \
      *this = ACast;
    #include "El/macros/GuardAndPayload.h"
    return *this;
}

// Basic queries
// =============
template<typename T>
mpi::Comm DM::DistComm() const EL_NO_EXCEPT
{ return this->grid_->VRComm(); }
template<typename T>
mpi::Comm DM::CrossComm() const EL_NO_EXCEPT
{ return mpi::COMM_SELF; }
template<typename T>
mpi::Comm DM::RedundantComm() const EL_NO_EXCEPT
{ return mpi::COMM_SELF; }
template<typename T>
mpi::Comm DM::ColComm() const EL_NO_EXCEPT
{ return this->grid_->VRComm(); }
template<typename T>
mpi::Comm DM::RowComm() const EL_NO_EXCEPT
{ return mpi::COMM_SELF; }
template<typename T>
mpi::Comm DM::PartialColComm() const EL_NO_EXCEPT
{ return this->grid_->MRComm(); }
template<typename T>
mpi::Comm DM::PartialUnionColComm() const EL_NO_EXCEPT
{ return this->grid_->MCComm(); }

template<typename T>
int DM::ColStride() const EL_NO_EXCEPT
{ return this->grid_->VRSize(); }
template<typename T>
int DM::RowStride() const EL_NO_EXCEPT
{ return 1; }
template<typename T>
int DM::PartialColStride() const EL_NO_EXCEPT
{ return this->grid_->MRSize(); }
template<typename T>
int DM::PartialUnionColStride() const EL_NO_EXCEPT
{ return this->grid_->MCSize(); }
template<typename T>
int DM::DistSize() const EL_NO_EXCEPT
{ return this->grid_->VRSize(); }
template<typename T>
int DM::CrossSize() const EL_NO_EXCEPT
{ return 1; }
template<typename T>
int DM::RedundantSize() const EL_NO_EXCEPT
{ return 1; }

// Instantiate {Int,Real,Complex<Real>} for each Real in {float,double}
// ####################################################################

#define SELF(T,U,V) \
  template DistMatrix<T,COLDIST,ROWDIST>::DistMatrix \
  ( const DistMatrix<T,U,V>& A );
#define OTHER(T,U,V) \
  template DistMatrix<T,COLDIST,ROWDIST>::DistMatrix \
  ( const BlockDistMatrix<T,U,V>& A ); \
  template DistMatrix<T,COLDIST,ROWDIST>& \
           DistMatrix<T,COLDIST,ROWDIST>::operator= \
           ( const BlockDistMatrix<T,U,V>& A )
#define BOTH(T,U,V) \
  SELF(T,U,V); \
  OTHER(T,U,V)
#define PROTO(T) \
  template class DistMatrix<T,COLDIST,ROWDIST>; \
  BOTH( T,CIRC,CIRC); \
  BOTH( T,MC,  MR  ); \
  BOTH( T,MC,  STAR); \
  BOTH( T,MD,  STAR); \
  BOTH( T,MR,  MC  ); \
  BOTH( T,MR,  STAR); \
  BOTH( T,STAR,MC  ); \
  BOTH( T,STAR,MD  ); \
  BOTH( T,STAR,MR  ); \
  BOTH( T,STAR,STAR); \
  BOTH( T,STAR,VC  ); \
  BOTH( T,STAR,VR  ); \
  BOTH( T,VC,  STAR); \
  OTHER(T,VR,  STAR);

#define EL_ENABLE_QUAD
#include "El/macros/Instantiate.h"

} // namespace El
