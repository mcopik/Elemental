/*
   Copyright (c) 2009-2012, Jack Poulson, Lexing Ying, and 
   The University of Texas at Austin.
   All rights reserved.

   Copyright (c) 2013, Jack Poulson, Lexing Ying, and Stanford University.
   All rights reserved.

   Copyright (c) 2013-2014, Jack Poulson and 
   The Georgia Institute of Technology.
   All rights reserved.

   Copyright (c) 2014-2015, Jack Poulson and Stanford University.
   All rights reserved.
   
   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "El.hpp"

namespace El {
namespace ldl {

template<typename F>
Front<F>::Front( Front<F>* parentNode )
: sparseLeaf(false), parent(parentNode), duplicate(nullptr)
{ 
    if( parentNode != nullptr )
    {
        isHermitian = parentNode->isHermitian;
        type = parentNode->type;
    }
}

template<typename F>
Front<F>::Front( DistFront<F>* dupNode )
: sparseLeaf(false), parent(nullptr), duplicate(dupNode)
{
    isHermitian = dupNode->isHermitian;
    type = dupNode->type;
}

template<typename F>
Front<F>::Front
( const SparseMatrix<F>& A, 
  const vector<Int>& reordering,
  const NodeInfo& info,
  bool conjugate )
: sparseLeaf(false), parent(nullptr), duplicate(nullptr)
{
    DEBUG_ONLY(CSE cse("Front::Front"))
    Pull( A, reordering, info, conjugate );
}

template<typename F>
Front<F>::~Front()
{
    for( auto* child : children )
        delete child;
}

template<typename F>
void Front<F>::Pull
( const SparseMatrix<F>& A, 
  const vector<Int>& reordering,
  const NodeInfo& rootInfo,
  bool conjugate )
{
    DEBUG_ONLY(
      CSE cse("Front::Pull");
      if( A.Height() != (Int)reordering.size() )
          LogicError("Mapping was not the right size");
    )
    type = SYMM_2D;
    isHermitian = conjugate;

    // Invert the reordering
    const Int n = reordering.size();
    DEBUG_ONLY(
      if( A.Height() != n || A.Width() != n )
          LogicError("Expected A to be square and consistent with reordering");
    )
    vector<Int> invReorder(n);
    for( Int j=0; j<n; ++j )
        invReorder[reordering[j]] = j;
    
    function<void(const NodeInfo&,Front<F>&)> pull = 
      [&]( const NodeInfo& node, Front<F>& front )
      {
        // Delete any existing children
        for( auto* child : front.children )
            delete child;

        const Int numChildren = node.children.size();
        front.children.resize( numChildren );
        for( Int c=0; c<numChildren; ++c )
        {
            front.children[c] = new Front<F>(&front);
            pull( *node.children[c], *front.children[c] );
        }
        // Mark this node as a sparse leaf if it does not have any children
        if( numChildren == 0 )
            front.sparseLeaf = true;

        const Int lowerSize = node.lowerStruct.size();
        const F* AValBuf = A.LockedValueBuffer();
        const Int* AColBuf = A.LockedTargetBuffer();
        const Int* AOffsetBuf = A.LockedOffsetBuffer();
        if( front.sparseLeaf )
        {
            front.workSparse.Empty();
            Zeros( front.workSparse, node.size, node.size );
            Zeros( front.LDense, lowerSize, node.size );

            // Count the number of sparse entries to queue into the top-left
            Int numEntriesTopLeft = 0;
            for( Int t=0; t<node.size; ++t )
            {
                const Int j = invReorder[node.off+t];
                const Int rowOff = AOffsetBuf[j];
                const Int numConn = AOffsetBuf[j+1] - rowOff;
                for( Int k=0; k<numConn; ++k )
                {
                    const Int iOrig = AColBuf[rowOff+k];
                    const Int i = reordering[iOrig];

                    if( i < node.off+t )
                        continue;
                    else if( i < node.off+node.size )
                        ++numEntriesTopLeft;
                }
            }
            front.workSparse.Reserve( numEntriesTopLeft ); 

            F* LDenseBuf = front.LDense.Buffer();
            const Int LDenseLDim = front.LDense.LDim();
            for( Int t=0; t<node.size; ++t )
            {
                const Int j = invReorder[node.off+t];
                const Int rowOff = AOffsetBuf[j];
                const Int numConn = AOffsetBuf[j+1] - rowOff;
                for( Int k=0; k<numConn; ++k )
                {
                    const Int iOrig = AColBuf[rowOff+k];
                    const Int i = reordering[iOrig];

                    const F transVal = AValBuf[rowOff+k];
                    const F value = ( conjugate ? Conj(transVal) : transVal );

                    if( i < node.off+t )
                        continue;
                    else if( i < node.off+node.size )
                    {
                        // Since SuiteSparse makes use of column-major ordering,
                        // and Elemental uses row-major ordering of its sparse
                        // matrices, we are implicitly storing the transpose.
                        front.workSparse.QueueUpdate( i-node.off, t, transVal );
                    }
                    else
                    {
                        const Int origOff = Find( node.origLowerStruct, i );
                        const Int row = node.origLowerRelInds[origOff];
                        DEBUG_ONLY(
                          if( row < t )
                              LogicError("Tried to touch upper triangle");
                        )
                        LDenseBuf[(row-node.size)+t*LDenseLDim] = value;
                    }
                }
            }
            front.workSparse.ProcessQueues();
            MakeSymmetric( LOWER, front.workSparse, front.isHermitian );
        }
        else
        {
            Zeros( front.LDense, node.size+lowerSize, node.size );
            F* LDenseBuf = front.LDense.Buffer();
            const Int LDenseLDim = front.LDense.LDim();
            for( Int t=0; t<node.size; ++t )
            {
                const Int j = invReorder[node.off+t];
                const Int rowOff = AOffsetBuf[j];
                const Int numConn = AOffsetBuf[j+1] - rowOff;
                for( Int k=0; k<numConn; ++k )
                {
                    const Int iOrig = AColBuf[rowOff+k];
                    const Int i = reordering[iOrig];

                    const F transVal = AValBuf[rowOff+k];
                    const F value = ( conjugate ? Conj(transVal) : transVal );

                    if( i < node.off+t )
                        continue;
                    else if( i < node.off+node.size )
                    {
                        LDenseBuf[(i-node.off)+t*LDenseLDim] = value;
                    }
                    else
                    {
                        const Int origOff = Find( node.origLowerStruct, i );
                        const Int row = node.origLowerRelInds[origOff];
                        DEBUG_ONLY(
                          if( row < t )
                              LogicError("Tried to touch upper triangle");
                        )
                        LDenseBuf[row+t*LDenseLDim] = value;
                    }
                }
            }
        }
      };
    pull( rootInfo, *this );
}

template<typename F>
void Front<F>::PullUpdate
( const SparseMatrix<F>& A, 
  const vector<Int>& reordering,
  const NodeInfo& rootInfo )
{
    DEBUG_ONLY(
      CSE cse("Front::PullUpdate");
      if( A.Height() != (Int)reordering.size() )
          LogicError("Mapping was not the right size");
    )

    // Invert the reordering
    const Int n = reordering.size();
    DEBUG_ONLY(
      if( A.Height() != n || A.Width() != n )
          LogicError("Expected A to be square and consistent with reordering");
    )
    vector<Int> invReorder(n);
    for( Int j=0; j<n; ++j )
        invReorder[reordering[j]] = j;
    
    function<void(const NodeInfo&,Front<F>&)> pull = 
      [&]( const NodeInfo& node, Front<F>& front )
      {
        const Int numChildren = node.children.size();
        for( Int c=0; c<numChildren; ++c )
            pull( *node.children[c], *front.children[c] );

        const F* AValBuf = A.LockedValueBuffer();
        const Int* AColBuf = A.LockedTargetBuffer();
        const Int* AOffsetBuf = A.LockedOffsetBuffer();

        F* LDenseBuf = front.LDense.Buffer();
        const Int LDenseLDim = front.LDense.LDim();

        if( front.sparseLeaf )
        {
            LogicError("Sparse leaves not yet handled in Front::PullUpdate");
        }
        else
        {
            for( Int t=0; t<node.size; ++t )
            {
                const Int j = invReorder[node.off+t];
                const Int rowOff = AOffsetBuf[j];
                const Int numConn = AOffsetBuf[j+1] - rowOff;
                for( Int k=0; k<numConn; ++k )
                {
                    const Int iOrig = AColBuf[rowOff+k];
                    const Int i = reordering[iOrig];

                    const F transVal = AValBuf[rowOff+k];
                    const F value = ( isHermitian ? Conj(transVal) : transVal );
    
                    if( i < node.off+t )
                        continue;
                    else if( i < node.off+node.size )
                    {
                        LDenseBuf[(i-node.off)+t*LDenseLDim] += value;
                    }
                    else
                    {
                        const Int origOff = Find( node.origLowerStruct, i );
                        const Int row = node.origLowerRelInds[origOff];
                        DEBUG_ONLY(
                          if( row < t )
                              LogicError("Tried to touch upper triangle");
                        )
                        LDenseBuf[row+t*LDenseLDim] += value;
                    }
                }
            }
        }
      };
    pull( rootInfo, *this );
}

// TODO: Use lower-level access
template<typename F>
void Front<F>::Push
( SparseMatrix<F>& A, 
  const vector<Int>& reordering,
  const NodeInfo& rootInfo ) const
{
    DEBUG_ONLY(CSE cse("Front::Push"))

    // Invert the reordering
    const Int n = reordering.size();
    vector<Int> invReorder( n ); 
    for( Int j=0; j<n; ++j )
        invReorder[reordering[j]] = j;

    Zeros( A, n, n );

    // Reserve space for the lower triangle
    Int numLower = 0;
    function<void(const Front<F>&)> countLower = 
      [&]( const Front<F>& front )
      {
          for( const Front<F>* child : front.children )
              countLower( *child );
          const Int nodeSize = front.LDense.Width();
          const Int structSize = front.Height() - nodeSize;
          numLower += (nodeSize*(nodeSize+1))/2 + nodeSize*structSize;
      };
    countLower( *this );
    A.Reserve( numLower );

    function<void(const NodeInfo&,const Front<F>&)> 
      push = 
      [&]( const NodeInfo& node, const Front<F>& front )
      {
        const Int numChildren = node.children.size();
        for( Int c=0; c<numChildren; ++c )
            push( *node.children[c], *front.children[c] );

        const Int lowerSize = node.lowerStruct.size();
        if( front.sparseLeaf )
        {
            LogicError("Sparse leaves not yet supported in Front::Push");
        }
        else
        {
            for( Int t=0; t<node.size; ++t )
            {
                const Int j = invReorder[node.off+t];

                // Push in the lower triangle of the diagonal block
                for( Int s=t; s<node.size; ++s )
                {
                    const Int i = invReorder[node.off+s];
                    A.QueueUpdate( i, j, front.LDense.Get(s,t) );
                }

                // Push in the connectivity 
                for( Int s=0; s<lowerSize; ++s )
                {
                    const Int i = invReorder[node.lowerStruct[s]];
                    A.QueueUpdate( i, j, front.LDense.Get(s+node.size,t) );
                }
            }
        }
      };
    push( rootInfo, *this );
    A.ProcessQueues();
    MakeSymmetric( LOWER, A, isHermitian );
}

// TODO: Use lower-level access
template<typename F>
void Front<F>::Unpack( SparseMatrix<F>& A, const NodeInfo& rootInfo ) const
{
    DEBUG_ONLY(CSE cse("Front::Unpack"))
    const Int n = rootInfo.off + rootInfo.size;
    Zeros( A, n, n );

    // Reserve space for the lower triangle
    Int numLower = 0;
    function<void(const Front<F>&)> countLower = 
      [&]( const Front<F>& front )
      {
          for( const Front<F>* child : front.children )
              countLower( *child );
          const Int nodeSize = front.LDense.Width();
          const Int structSize = front.Height() - nodeSize;
          numLower += (nodeSize*(nodeSize+1))/2 + nodeSize*structSize;
      };
    countLower( *this );
    A.Reserve( numLower );

    function<void(const NodeInfo&,const Front<F>&)> push = 
      [&]( const NodeInfo& node, const Front<F>& front )
      {
        const Int numChildren = node.children.size();
        for( Int c=0; c<numChildren; ++c )
            push( *node.children[c], *front.children[c] );

        const Int lowerSize = node.lowerStruct.size();
        if( front.sparseLeaf )
        {
            LogicError("Sparse leaves not yet supported for Front::Unpack");
        }
        else
        {
            for( Int t=0; t<node.size; ++t )
            {
                const Int j = node.off+t;

                // Push in the lower triangle of the diagonal block
                for( Int s=t; s<node.size; ++s )
                {
                    const Int i = node.off+s;
                    A.QueueUpdate( i, j, front.LDense.Get(s,t) );
                }

                // Push in the connectivity 
                for( Int s=0; s<lowerSize; ++s )
                {
                    const Int i = node.lowerStruct[s];
                    A.QueueUpdate( i, j, front.LDense.Get(s+node.size,t) );
                }
            }
        }
      };
    push( rootInfo, *this );
    A.ProcessQueues();
}

template<typename F>
const Front<F>& Front<F>::operator=( const Front<F>& front )
{
    DEBUG_ONLY(CSE cse("Front::operator="))
    isHermitian = front.isHermitian;
    sparseLeaf = front.sparseLeaf;
    type = front.type;
    LDense = front.LDense;
    LSparse = front.LSparse;
    diag = front.diag;
    subdiag = front.subdiag;
    p = front.p;
    workDense = front.workDense;
    workSparse = front.workSparse;
    // Do not copy parent...
    // Delete any existing children
    for( auto* child : children )
        delete child;
    const int numChildren = front.children.size();
    children.resize( numChildren );
    for( int c=0; c<numChildren; ++c )
    {
        children[c] = new Front<F>(this);
        *children[c] = *front.children[c];
    }
    return *this;
}

template<typename F>
Int Front<F>::Height() const
{ return sparseLeaf ? LDense.Height()+LDense.Width() : LDense.Height(); }

template<typename F>
Int Front<F>::NumEntries() const
{
    DEBUG_ONLY(CSE cse("Front::NumEntries"))
    Int numEntries = 0;
    function<void(const Front<F>&)> count =
      [&]( const Front<F>& front )
      {
        for( auto* child : front.children )
            count( *child );

        if( front.sparseLeaf )
        {
            // Add in L
            numEntries += front.LSparse.NumEntries();
            numEntries += front.LDense.Height() * front.LDense.Width();
        }
        else
        {
            // Add in L
            numEntries += front.LDense.Height() * front.LDense.Width();
        }
        // Add in the workspace for the Schur complement
        numEntries += front.workDense.Height()*front.workDense.Width(); 
      };
    count( *this );
    return numEntries;
}

template<typename F>
Int Front<F>::NumTopLeftEntries() const
{
    DEBUG_ONLY(CSE cse("Front::NumTopLeftEntries"))
    Int numEntries = 0;
    function<void(const Front<F>&)> count =
      [&]( const Front<F>& front )
      {
        for( auto* child : front.children )
            count( *child );
        if( front.sparseLeaf )
        {
            numEntries += front.LSparse.NumEntries();
        }
        else
        {
            const Int n = front.LDense.Width();
            numEntries += n*n;
        }
      };
    count( *this );
    return numEntries;
}

template<typename F>
Int Front<F>::NumBottomLeftEntries() const
{
    DEBUG_ONLY(CSE cse("Front::NumBottomLeftEntries"))
    Int numEntries = 0;
    function<void(const Front<F>&)> count =
      [&]( const Front<F>& front )
      {
        for( auto* child : front.children )
            count( *child );
        const Int m = front.LDense.Height();
        const Int n = front.LDense.Width();
        if( front.sparseLeaf )
        {
            numEntries += m*n;
        }
        else
        {
            numEntries += (m-n)*n;
        }
      };
    count( *this );
    return numEntries;
}

template<typename F>
double Front<F>::FactorGFlops() const
{
    DEBUG_ONLY(CSE cse("DistFront::FactorGFlops"))
    double gflops = 0.;
    function<void(const Front<F>&)> count =
      [&]( const Front<F>& front )
      {
        for( auto* child : front.children )
            count( *child );
        const double m = front.LDense.Height();
        const double n = front.LDense.Width();
        double realFrontFlops=0;
        if( front.sparseLeaf )
        {
            // Count the flops from the sparse factorization
            const Int* offsetBuf = front.LSparse.LockedOffsetBuffer();
            for( Int j=0; j<n; ++j )
            {
                const Int nnz = offsetBuf[j+1]-offsetBuf[j];
                realFrontFlops += nnz*(nnz+2.);
            }
            // Count the flops from the dense trsm
            realFrontFlops += m*n;
            // Count the flops from the Schur-complement update
            realFrontFlops += m*m*n;
        }
        else
        {
            realFrontFlops = (n*n*n/3) + (m-n)*n + (m-n)*(m-n)*n;
        }
        gflops += (IsComplex<F>::value ? 4*realFrontFlops
                                       : realFrontFlops)/1.e9;
      };
    count( *this );
    return gflops;
}

template<typename F>
double Front<F>::SolveGFlops( Int numRHS ) const
{
    DEBUG_ONLY(CSE cse("Front::SolveGFlops"))
    double gflops = 0.;
    function<void(const Front<F>&)> count =
      [&]( const Front<F>& front )
      {
        for( auto* child : front.children )
            count( *child );
        const double m = front.LDense.Height();
        const double n = front.LDense.Width();
        double realFrontFlops = 0;
        if( front.sparseLeaf ) 
        {
            const double numEntries = front.LSparse.NumEntries();
            realFrontFlops = (numEntries+m*n)*numRHS;
        }
        else
        {
            realFrontFlops = m*n*numRHS;
        }
        gflops += (IsComplex<F>::value ? 4*realFrontFlops
                                       : realFrontFlops)/1.e9;
      };
    count( *this );
    return gflops;
}

#define PROTO(F) template struct Front<F>;
#define EL_NO_INT_PROTO
#include "El/macros/Instantiate.h"

} // namespace ldl
} // namespace El
