/*
   Copyright (c) 2009-2015, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef EL_UTIL_HPP
#define EL_UTIL_HPP

namespace El {

// Graph reordering
// ================
struct BisectCtrl
{
    bool sequential;
    Int numDistSeps;
    Int numSeqSeps;
    Int cutoff;
    bool storeFactRecvInds;

    BisectCtrl()
    : sequential(true), numDistSeps(1), numSeqSeps(1), cutoff(1024),
      storeFactRecvInds(false)
    { }
};

Int Bisect
( const Graph& graph,
        Graph& leftChild,
        Graph& rightChild,
        vector<Int>& perm,
  const BisectCtrl& ctrl=BisectCtrl() );

// NOTE: for two or more processes
Int Bisect
( const DistGraph& graph,
        DistGraph& child,
        DistMap& perm,
        bool& onLeft,
  const BisectCtrl& ctrl=BisectCtrl() );

Int NaturalBisect
( Int nx, Int ny, Int nz,
  const Graph& graph,
  Int& nxLeft, Int& nyLeft, Int& nzLeft,
  Graph& leftChild,
  Int& nxRight, Int& nyRight, Int& nzRight,
  Graph& rightChild, vector<Int>& perm );

// NOTE: for two or more processes
Int NaturalBisect
( Int nx, Int ny, Int nz,
  const DistGraph& graph,
  Int& nxChild, Int& nyChild, Int& nzChild,
  DistGraph& child, DistMap& perm, bool& onLeft );

void EnsurePermutation( const vector<Int>& map );
void EnsurePermutation( const DistMap& map );

void BuildChildrenFromPerm
( const Graph& graph, const vector<Int>& perm,
  Int leftChildSize, Graph& leftChild,
  Int rightChildSize, Graph& rightChild );
void BuildChildFromPerm
( const DistGraph& graph, const DistMap& perm,
  Int leftChildSize, Int rightChildSize,
  bool& onLeft, DistGraph& child );

// Median
// ======
template<typename Real,typename=EnableIf<IsReal<Real>>>
ValueInt<Real> Median( const Matrix<Real>& x );
template<typename Real,typename=EnableIf<IsReal<Real>>>
ValueInt<Real> Median( const ElementalMatrix<Real>& x );

// Sort
// ====
template<typename Real,typename=EnableIf<IsReal<Real>>>
void Sort( Matrix<Real>& X, SortType sort=ASCENDING );
template<typename Real,typename=EnableIf<IsReal<Real>>>
void Sort( ElementalMatrix<Real>& X, SortType sort=ASCENDING );

template<typename Real,typename=EnableIf<IsReal<Real>>>
vector<ValueInt<Real>>
TaggedSort( const Matrix<Real>& x, SortType sort=ASCENDING );
template<typename Real,typename=EnableIf<IsReal<Real>>>
vector<ValueInt<Real>>
TaggedSort( const ElementalMatrix<Real>& x, SortType sort=ASCENDING );

} // namespace El

#endif // ifndef EL_UTIL_HPP
