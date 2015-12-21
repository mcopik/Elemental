/*
   Copyright (c) 2009-2015, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef EL_IMPORTS_PMRRR_HPP
#define EL_IMPORTS_PMRRR_HPP

namespace El {
namespace herm_tridiag_eig {

struct Estimate {
    int numLocalEigenvalues;
    int numGlobalEigenvalues;
};

// Return an upper bound on the number of (local) eigenvalues in the given range
Estimate EigEstimate
( int n,  double* d, double* e, double* w, mpi::Comm comm, 
  double lowerBound, double upperBound, bool xtd = false );

struct Info {
    int numLocalEigenvalues;
    int numGlobalEigenvalues;

    int firstLocalEigenvalue;
};

// Compute all of the eigenvalues
Info Eig( int n, double* d, double* e, double* w, mpi::Comm comm, bool xtd = false );

// Compute all of the eigenpairs
Info Eig
( int n, double* d, double* e, double* w, double* Z, int ldz, mpi::Comm comm, bool xtd = false );

// Compute all of the eigenvalues in [lowerBound,upperBound)
Info Eig
( int n, double* d, double* e, double* w, mpi::Comm comm, 
  double lowerBound, double upperBound, bool xtd = false );

// Compute all of the eigenpairs with eigenvalues in [lowerBound,upperBound)
Info Eig
( int n, double* d, double* e, double* w, double* Z, int ldz, mpi::Comm comm, 
  double lowerBound, double upperBound, bool xtd = false );

// Compute all of the eigenvalues with indices in [lowerBound,upperBound)
Info Eig
( int n, double* d, double* e, double* w, mpi::Comm comm, 
  int lowerBound, int upperBound, bool xtd = false );

// Compute all of the eigenpairs with ordered eigenvalue indices in 
// [lowerBound,upperBound)
Info Eig
( int n, double* d, double* e, double* w, double* Z, int ldz, mpi::Comm comm, 
  int lowerBound, int upperBound, bool xtd = false );

} // namespace herm_tridiag_eig
} // namespace El

#endif // ifndef EL_IMPORTS_PMRRR_HPP
