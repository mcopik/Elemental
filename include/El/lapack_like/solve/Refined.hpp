/*
   Copyright (c) 2009-2015, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef EL_SOLVE_REFINED_HPP
#define EL_SOLVE_REFINED_HPP

// The following routines each have three underlying implementations:
//
//  1) a single right-hand side,
//  2) a pair of right-hand sides, and
//  3) an arbitrary number of right-hand sides.
//
// In the first and second cases, proper iterative refinement is performed.
// In the third case, "batch" iterative refinement is performed for a 
// specified number of iterations. While batch refinement will occasionally 
// result in higher residual norms than necessary (as monotonicity must be
// specifically enforced), and more iterations than necessary may be performed, 
// it is a compromise between performance and accuracy.

// TODO: DistMatrix implementations
// TODO: Simplify once DistMultiVec is eliminated
// TODO: Allow for a choice between max and two norms?

namespace El {

namespace refined_solve {

template<typename F,class ApplyAType,class ApplyAInvType>
inline Int Single
( const ApplyAType& applyA,
  const ApplyAInvType& applyAInv,
        Matrix<F>& b,
        Base<F> relTol,
        Int maxRefineIts,
        bool progress )
{
    DEBUG_ONLY(
      CSE cse("refined_solve::Single");
      if( b.Width() != 1 )
          LogicError("Expected a single right-hand side");
    )
    if( maxRefineIts <= 0 )
    {
        applyAInv( b );
        return 0;
    }

    auto bOrig = b;
    const Base<F> bNorm = MaxNorm( b );

    // Compute the initial guess
    // =========================
    auto x = b;
    applyAInv( x );

    Matrix<F> dx, xCand, y;
    Zeros( y, x.Height(), 1 );

    applyA( x, y );
    b -= y;
    Base<F> errorNorm = MaxNorm( b );
    if( progress )
        Output("original rel error: ",errorNorm/bNorm);

    Int refineIt = 0;
    while( true )
    {
        if( errorNorm/bNorm <= relTol )
        {
            if( progress )
                Output(errorNorm/bNorm," <= ",relTol);
            break;
        }

        // Compute the proposed update to the solution
        // -------------------------------------------
        dx = b;
        applyAInv( dx );
        xCand = x;
        xCand += dx;

        // Check the new residual
        // ----------------------
        applyA( xCand, y );
        b = bOrig;
        b -= y;
        auto newErrorNorm = MaxNorm( b );
        if( progress )
            Output("refined rel error: ",newErrorNorm/bNorm);

        if( newErrorNorm < errorNorm )
            x = xCand;
        else
            break;

        errorNorm = newErrorNorm;
        ++refineIt;
        if( refineIt >= maxRefineIts )
            break;
    }
    b = x;
    return refineIt;
}

template<typename F,class ApplyAType,class ApplyAInvType>
inline Int Pair
( const ApplyAType& applyA,
  const ApplyAInvType& applyAInv,
        Matrix<F>& B,
        Base<F> relTol,
        Int maxRefineIts,
        bool progress )
{
    DEBUG_ONLY(
      CSE cse("refined_solve::Pair");
      if( B.Width() != 2 )
          LogicError("Expected a pair of right-hand sides");
    )
    if( maxRefineIts <= 0 )
    {
        applyAInv( B );
        return 0;
    }

    auto bL = B( ALL, IR(0) );
    auto bR = B( ALL, IR(1) );
    const Base<F> bLNorm = MaxNorm( bL );
    const Base<F> bRNorm = MaxNorm( bR );

    auto BOrig = B;
    auto bOrigL = BOrig( ALL, IR(0) ); 
    auto bOrigR = BOrig( ALL, IR(1) );

    // Compute the initial guess
    // =========================
    auto X = B;
    auto xL = X( ALL, IR(0) );
    auto xR = X( ALL, IR(1) );
    applyAInv( X );

    Matrix<F> dX, XCand, Y;
    Zeros( dX, X.Height(), X.Width() );
    Zeros( XCand, X.Height(), X.Width() );
    Zeros( Y, X.Height(), X.Width() );

    auto dxL = dX( ALL, IR(0) );
    auto dxR = dX( ALL, IR(1) );
    auto xLCand = XCand( ALL, IR(0) );
    auto xRCand = XCand( ALL, IR(1) );
    auto yL = Y( ALL, IR(0) );
    auto yR = Y( ALL, IR(1) );

    applyA( X, Y );
    B -= Y;
    Base<F> errorNormL = MaxNorm( bL );
    Base<F> errorNormR = MaxNorm( bR );
    if( progress )
        Output
        ("original rel errors: ",errorNormL/bLNorm," and ",errorNormR/bRNorm);

    Int refineIt = 0;
    bool leftConv=false, rightConv=false;
    while( true )
    {
        const Base<F> relErrorL = errorNormL/bLNorm;
        const Base<F> relErrorR = errorNormR/bRNorm;
        if( !leftConv && relErrorL <= relTol )
        {
            if( progress )
                Output("Left converged with ",relErrorL," <= ",relTol);
            leftConv = true;
        }
        if( !rightConv && relErrorR <= relTol )
        {
            if( progress )
                Output("Right converged with ",relErrorR," <= ",relTol);
            rightConv = true;
        }
        if( leftConv && rightConv )
            break;

        if( leftConv )
        {
            // Compute the proposed update to the solution
            // -------------------------------------------
            dxR = bR;
            applyAInv( dxR );
            xRCand = xR;
            xRCand += dxR;

            // Check the new residual
            // ----------------------
            applyA( xRCand, yR );
            bR = bOrigR;
            bR -= yR;
            auto newErrorNormR = MaxNorm( bR );
            if( progress )
                Output("Right refined rel error: ",newErrorNormR/bRNorm);

            if( newErrorNormR < errorNormR )
            {
                xR = xRCand;
                errorNormR = newErrorNormR;
            }
            else
                rightConv = true;
        }
        else if( rightConv )
        {
            // Compute the proposed update to the solution
            // -------------------------------------------
            dxL = bL;
            applyAInv( dxL );
            xLCand = xL;
            xLCand += dxL;

            // Check the new residual
            // ----------------------
            applyA( xLCand, yL );
            bL = bOrigL;
            bL -= yL;
            auto newErrorNormL = MaxNorm( bL );
            if( progress )
                Output("Left refined rel error: ",newErrorNormL/bLNorm);

            if( newErrorNormL < errorNormL )
            {
                xL = xLCand;
                errorNormL = newErrorNormL;
            }
            else
                leftConv = true;
        }
        else
        {
            // Compute the proposed update to the solution
            // -------------------------------------------
            dX = B;
            applyAInv( dX );
            XCand = X;
            XCand += dX;

            // Check the new residual
            // ----------------------
            applyA( XCand, Y );
            B = BOrig;
            B -= Y;
            auto newErrorNormL = MaxNorm( bL );
            auto newErrorNormR = MaxNorm( bR );
            if( progress )
            {
                Output("Left refined rel error: ",newErrorNormL/bLNorm);
                Output("Right refined rel error: ",newErrorNormR/bRNorm);
            }

            if( newErrorNormL < errorNormL )
            {
                xL = xLCand;
                errorNormL = newErrorNormL;
            }
            else
                leftConv = true;

            if( newErrorNormR < errorNormR )
            {
                xR = xRCand;
                errorNormR = newErrorNormR;
            }
            else
                rightConv = true;
        }
        ++refineIt;
        if( refineIt >= maxRefineIts )
            break;
    }
    B = X;
    return refineIt;
}

template<typename F,class ApplyAType,class ApplyAInvType>
inline Int Batch
( const ApplyAType& applyA,
  const ApplyAInvType& applyAInv,
        Matrix<F>& B,
        Int maxRefineIts,
        bool progress )
{
    DEBUG_ONLY(CSE cse("refined_solve::Batch"))
    if( maxRefineIts <= 0 )
    {
        applyAInv( B );    
        return 0;
    }

    // TODO: Allow for early exits

    // Compute the initial guesses
    // ===========================
    auto BOrig = B;
    auto X = B;
    applyAInv( X );

    Matrix<F> dX, Y;
    Zeros( Y, X.Height(), X.Width() );
    applyA( X, Y );
    B -= Y;

    Int refineIt = 0;
    while( true )
    {
        // Compute the updates to the solutions
        // ------------------------------------
        dX = B;
        applyAInv( dX );
        X += dX;

        ++refineIt;
        if( refineIt < maxRefineIts )
        {
            // Compute the new residual
            // ------------------------
            applyA( X, Y );
            B = BOrig;
            B -= Y;
        }
        else
            break;
    }
    B = X;
    return refineIt;
}

} // namespace refined_solve

template<typename F,class ApplyAType,class ApplyAInvType>
inline Int RefinedSolve
( const ApplyAType& applyA,
  const ApplyAInvType& applyAInv,
        Matrix<F>& B,
        Base<F> relTol,
        Int maxRefineIts,
        bool progress )
{
    DEBUG_ONLY(CSE cse("RefinedSolve"))
    if( B.Width() == 1 )
        return refined_solve::Single
               ( applyA, applyAInv, B, relTol, maxRefineIts, progress );
    else if( B.Width() == 2 )
        return refined_solve::Pair
               ( applyA, applyAInv, B, relTol, maxRefineIts, progress );
    else
        return refined_solve::Batch
               ( applyA, applyAInv, B, maxRefineIts, progress );
}

namespace refined_solve {

template<typename F,class ApplyAType,class ApplyAInvType>
inline Int PromotedSingle
( const ApplyAType& applyA,
  const ApplyAInvType& applyAInv,
        Matrix<F>& b,
        Base<F> relTol,
        Int maxRefineIts,
        bool progress )
{
    DEBUG_ONLY(
      CSE cse("refined_solve::PromotedSingle");
      if( b.Width() != 1 )
          LogicError("Expected a single right-hand side");
    )
    if( maxRefineIts <= 0 )
    {
        applyAInv( b );
        return 0;
    }
    typedef Base<F> Real;
    typedef Promote<Real> PReal;
    typedef Promote<F> PF;

    Matrix<PF> bProm, bOrigProm;
    Copy( b, bProm );
    Copy( b, bOrigProm );
    const PReal bNorm = MaxNorm( bOrigProm );

    // Compute the initial guess
    // =========================
    applyAInv( b );
    Matrix<PF> xProm;
    Copy( b, xProm );

    Matrix<PF> dxProm, xCandProm, yProm;
    Zeros( yProm, xProm.Height(), 1 );

    applyA( xProm, yProm );
    bProm -= yProm;
    auto errorNorm = MaxNorm( bProm );
    if( progress )
        Output("original rel error: ",errorNorm/bNorm);

    Int refineIt = 0;
    while( true )
    {
        if( errorNorm/bNorm <= relTol )
        {
            if( progress )
                Output(errorNorm/bNorm," <= ",relTol);
            break;
        }

        // Compute the proposed update to the solution
        // -------------------------------------------
        Copy( bProm, b );
        applyAInv( b );
        Copy( b, dxProm );
        xCandProm = xProm;
        xCandProm += dxProm;

        // Check the new residual
        // ----------------------
        applyA( xCandProm, yProm );
        bProm = bOrigProm;
        bProm -= yProm;
        auto newErrorNorm = MaxNorm( bProm );
        if( progress )
            Output("refined rel error: ",newErrorNorm/bNorm);

        if( newErrorNorm < errorNorm )
            xProm = xCandProm;
        else
            break;

        errorNorm = newErrorNorm;
        ++refineIt;
        if( refineIt >= maxRefineIts )
            break;
    }
    Copy( xProm, b );
    return refineIt;
}

template<typename F,class ApplyAType,class ApplyAInvType>
inline Int PromotedPair
( const ApplyAType& applyA,
  const ApplyAInvType& applyAInv,
        Matrix<F>& B,
        Base<F> relTol,
        Int maxRefineIts,
        bool progress )
{
    DEBUG_ONLY(
      CSE cse("refined_solve::PromotedPair");
      if( B.Width() != 2 )
          LogicError("Expected a pair of right-hand sides");
    )
    if( maxRefineIts <= 0 )
    {
        applyAInv( B );
        return 0;
    }
    typedef Base<F> Real;
    typedef Promote<Real> PReal;
    typedef Promote<F> PF;

    Matrix<PF> BProm, BOrigProm;
    Copy( B, BProm );
    Copy( B, BOrigProm );

    auto bL = B( ALL, IR(0) );
    auto bR = B( ALL, IR(1) );
    auto bLProm = BProm( ALL, IR(0) );
    auto bRProm = BProm( ALL, IR(1) );
    auto bLOrigProm = BOrigProm( ALL, IR(0) );
    auto bROrigProm = BOrigProm( ALL, IR(1) );

    const PReal bLNorm = MaxNorm( bLOrigProm );
    const PReal bRNorm = MaxNorm( bROrigProm );

    // Compute the initial guess
    // =========================
    applyAInv( B );
    Matrix<PF> XProm;
    Copy( B, XProm );

    auto xLProm = XProm( ALL, IR(0) );
    auto xRProm = XProm( ALL, IR(1) );

    Matrix<PF> dXProm, XCandProm, YProm;
    Zeros( dXProm, XProm.Height(), XProm.Width() );
    Zeros( XCandProm, XProm.Height(), XProm.Width() );
    Zeros( YProm, XProm.Height(), XProm.Width() );

    auto dxLProm = dXProm( ALL, IR(0) );
    auto dxRProm = dXProm( ALL, IR(1) );
    auto xLCandProm = XCandProm( ALL, IR(0) );
    auto xRCandProm = XCandProm( ALL, IR(1) );
    auto yLProm = YProm( ALL, IR(0) );
    auto yRProm = YProm( ALL, IR(1) );

    applyA( XProm, YProm );
    BProm -= YProm;
    auto errorNormL = MaxNorm( bLProm );
    auto errorNormR = MaxNorm( bRProm );
    if( progress )
        Output
        ("original rel errors: ",errorNormL/bLNorm," and ",errorNormR/bRNorm);

    Int refineIt = 0;
    bool leftConv=false, rightConv=false;
    while( true )
    {
        const PReal relErrorL = errorNormL/bLNorm;
        const PReal relErrorR = errorNormR/bRNorm;
        if( !leftConv && relErrorL <= relTol )
        {
            if( progress )
                Output("Left converged with ",relErrorL," <= ",relTol);
            leftConv = true;
        }
        if( !rightConv && relErrorR <= relTol )
        {
            if( progress )
                Output("Right converged with ",relErrorR," <= ",relTol);
            rightConv = true;
        }
        if( leftConv && rightConv )
            break;
 
        if( leftConv )
        {
            // Compute the proposed update to the solution
            // -------------------------------------------
            Copy( bRProm, bR );
            applyAInv( bR );
            Copy( bR, dxRProm );
            xRCandProm = xRProm;
            xRCandProm += dxRProm;

            // Check the new residual
            // ----------------------
            applyA( xRCandProm, yRProm );
            bRProm = bROrigProm;
            bRProm -= yRProm;
            auto newErrorNormR = MaxNorm( bRProm );
            if( progress )
                Output("Right refined rel error: ",newErrorNormR/bRNorm);

            if( newErrorNormR < errorNormR )
            {
                xRProm = xRCandProm;
                errorNormR = newErrorNormR;
            }
            else
                rightConv = true;
        }
        else if( rightConv )
        {
            // Compute the proposed update to the solution
            // -------------------------------------------
            Copy( bLProm, bL );
            applyAInv( bL );
            Copy( bL, dxLProm );
            xLCandProm = xLProm;
            xLCandProm += dxLProm;

            // Check the new residual
            // ----------------------
            applyA( xLCandProm, yLProm );
            bLProm = bLOrigProm;
            bLProm -= yLProm;
            auto newErrorNormL = MaxNorm( bLProm );
            if( progress )
                Output("Left refined rel error: ",newErrorNormL/bLNorm);

            if( newErrorNormL < errorNormL )
            {
                xLProm = xLCandProm;
                errorNormL = newErrorNormL;
            }
            else
                leftConv = true;
        }
        else
        {
            // Compute the proposed update to the solution
            // -------------------------------------------
            Copy( BProm, B );
            applyAInv( B );
            Copy( B, dXProm );
            XCandProm = XProm;
            XCandProm += dXProm;

            // Check the new residual
            // ----------------------
            applyA( XCandProm, YProm );
            BProm = BOrigProm;
            BProm -= YProm;
            auto newErrorNormL = MaxNorm( bLProm );
            auto newErrorNormR = MaxNorm( bRProm );
            if( progress )
            {
                Output("Left refined rel error: ",newErrorNormL/bLNorm);
                Output("Right refined rel error: ",newErrorNormR/bRNorm);
            }

            if( newErrorNormL < errorNormL )
            {
                xLProm = xLCandProm;
                errorNormL = newErrorNormL;
            }
            else
                leftConv = true;

            if( newErrorNormR < errorNormR )
            {
                xRProm = xRCandProm;
                errorNormR = newErrorNormR;
            }
            else
                rightConv = true;
        }

        ++refineIt;
        if( refineIt >= maxRefineIts )
            break;
    }
    Copy( XProm, B );
    return refineIt;
}

template<typename F,class ApplyAType,class ApplyAInvType>
inline Int PromotedBatch
( const ApplyAType& applyA,
  const ApplyAInvType& applyAInv,
        Matrix<F>& B,
        Int maxRefineIts,
        bool progress )
{
    DEBUG_ONLY(CSE cse("refined_solve::PromotedBatch"))
    if( maxRefineIts <= 0 )
    {
        applyAInv( B );
        return 0;
    }
    typedef Promote<F> PF;

    Matrix<PF> BProm, BOrigProm;
    Copy( B, BProm );
    Copy( B, BOrigProm );

    // Compute the initial guesses
    // ===========================
    applyAInv( B );
    Matrix<PF> XProm;
    Copy( B, XProm );

    Matrix<PF> dXProm, YProm;
    Zeros( YProm, XProm.Height(), XProm.Width() );
    applyA( XProm, YProm );
    BProm -= YProm;

    Int refineIt = 0;
    while( true )
    {
        // Update the solutions
        // --------------------
        Copy( BProm, B );
        applyAInv( B );
        Copy( B, dXProm );
        XProm += dXProm;

        ++refineIt;
        if( refineIt < maxRefineIts )
        {
            // Form the new residuals
            // ----------------------
            applyA( XProm, YProm );
            BProm = BOrigProm;
            BProm -= YProm;
        }
        else
            break;
    }
    Copy( XProm, B );
    return refineIt;
}

} // namespace refined_solve

template<typename F,class ApplyAType,class ApplyAInvType>
inline DisableIf<IsSame<F,Promote<F>>,Int>
PromotedRefinedSolve
( const ApplyAType& applyA,
  const ApplyAInvType& applyAInv,
        Matrix<F>& B,
        Base<F> relTol,
        Int maxRefineIts,
        bool progress )
{
    DEBUG_ONLY(CSE cse("PromotedRefinedSolve"))
    if( B.Width() == 1 )
        return refined_solve::PromotedSingle
               ( applyA, applyAInv, B, relTol, maxRefineIts, progress );
    else if( B.Width() == 2 )
        return refined_solve::PromotedPair
               ( applyA, applyAInv, B, relTol, maxRefineIts, progress );
    else
        return refined_solve::PromotedBatch
               ( applyA, applyAInv, B, maxRefineIts, progress );
}

template<typename F,class ApplyAType,class ApplyAInvType>
inline EnableIf<IsSame<F,Promote<F>>,Int>
PromotedRefinedSolve
( const ApplyAType& applyA,
  const ApplyAInvType& applyAInv,
        Matrix<F>& B,
        Base<F> relTol,
        Int maxRefineIts,
        bool progress )
{
    DEBUG_ONLY(CSE cse("PromotedRefinedSolve"))
    return RefinedSolve( applyA, applyAInv, B, relTol, maxRefineIts, progress );
}

namespace refined_solve {

template<typename F,class ApplyAType,class ApplyAInvType>
inline Int Single
( const ApplyAType& applyA,
  const ApplyAInvType& applyAInv,
        DistMultiVec<F>& b,
        Base<F> relTol,
        Int maxRefineIts,
        bool progress )
{
    DEBUG_ONLY(
      CSE cse("refined_solve::Single");
      if( b.Width() != 1 )
          LogicError("Expected a single right-hand side");
    )
    if( maxRefineIts <= 0 )
    {
        applyAInv( b );
        return 0;
    }
    mpi::Comm comm = b.Comm();
    const int commRank = mpi::Rank(comm);

    auto bOrig = b;
    const Base<F> bNorm = MaxNorm( b );

    // Compute the initial guess
    // =========================
    auto x = b;
    applyAInv( x );

    DistMultiVec<F> dx(comm), xCand(comm), y(comm);
    Zeros( y, x.Height(), 1 );
    applyA( x, y );
    b -= y;
    Base<F> errorNorm = MaxNorm( b );
    if( progress && commRank == 0 )
        Output("original rel error: ",errorNorm/bNorm);

    Int refineIt = 0;
    const Int indent = PushIndent();
    while( true )
    {
        if( errorNorm/bNorm <= relTol )
        {
            if( progress && commRank == 0 )
                Output(errorNorm/bNorm," <= ",relTol);
            break;
        }

        // Compute the proposed update to the solution
        // -------------------------------------------
        dx = b;
        applyAInv( dx );
        xCand = x;
        xCand += dx;

        // Compute the new residual
        // ------------------------
        applyA( xCand, y );
        b = bOrig;
        b -= y;
        Base<F> newErrorNorm = MaxNorm( b );
        if( progress && commRank == 0 )
            Output("refined rel error: ",newErrorNorm/bNorm);

        if( newErrorNorm < errorNorm )
            x = xCand;
        else
            break;

        errorNorm = newErrorNorm;
        ++refineIt;
        if( refineIt >= maxRefineIts )
            break;
    }
    SetIndent( indent );
    b = x;
    return refineIt;
}

template<typename F,class ApplyAType,class ApplyAInvType>
inline Int Batch
( const ApplyAType& applyA,
  const ApplyAInvType& applyAInv,
        DistMultiVec<F>& B,
        Int maxRefineIts,
        bool progress )
{
    DEBUG_ONLY(CSE cse("refined_solve::Batch"))
    if( maxRefineIts <= 0 )
    {
        applyAInv( B );
        return 0;
    }
    mpi::Comm comm = B.Comm();

    auto BOrig = B;

    // Compute the initial guess
    // =========================
    auto X = B;
    applyAInv( X );

    DistMultiVec<F> dX(comm), Y(comm);
    Zeros( Y, X.Height(), X.Width() );
    applyA( X, Y );
    B -= Y;

    Int refineIt = 0;
    const Int indent = PushIndent();
    while( true )
    {
        // Compute the proposed updates to the solutions
        // ---------------------------------------------
        dX = B;
        applyAInv( dX );
        X += dX;

        ++refineIt;
        if( refineIt < maxRefineIts )
        {
            // Compute the new residual
            // ------------------------
            applyA( X, Y );
            B = BOrig;
            B -= Y;
        }
        else
            break;
    }
    SetIndent( indent );
    B = X;
    return refineIt;
}

} // namespace refined_solve

template<typename F,class ApplyAType,class ApplyAInvType>
inline Int RefinedSolve
( const ApplyAType& applyA,
  const ApplyAInvType& applyAInv,
        DistMultiVec<F>& B,
        Base<F> relTol,
        Int maxRefineIts,
        bool progress )
{
    DEBUG_ONLY(CSE cse("RefinedSolve"))
    if( B.Width() == 1 )
        return refined_solve::Single
               ( applyA, applyAInv, B, relTol, maxRefineIts, progress );
    else
        return refined_solve::Batch
               ( applyA, applyAInv, B, maxRefineIts, progress );
}

namespace refined_solve {

template<typename F,class ApplyAType,class ApplyAInvType>
inline Int PromotedSingle
( const ApplyAType& applyA,
  const ApplyAInvType& applyAInv,
        DistMultiVec<F>& b,
        Base<F> relTol,
        Int maxRefineIts,
        bool progress )
{
    DEBUG_ONLY(
      CSE cse("refined_solve::PromotedSingle");
      if( b.Width() != 1 )
          LogicError("Expected a single right-hand side");
    )
    if( maxRefineIts <= 0 )
    {
        applyAInv( b );
        return 0;
    }
    typedef Promote<F> PF;
    mpi::Comm comm = b.Comm();
    const int commRank = mpi::Rank(comm);

    DistMultiVec<PF> bProm(comm), bOrigProm(comm);
    Copy( b, bProm );
    Copy( b, bOrigProm );
    const auto bNorm = MaxNorm( bProm );

    // Compute the initial guess
    // =========================
    applyAInv( b );
    DistMultiVec<PF> xProm(comm);
    Copy( b, xProm );

    DistMultiVec<PF> dxProm(comm), xCandProm(comm), yProm(comm);
    Zeros( yProm, xProm.Height(), 1 );
    applyA( xProm, yProm );
    bProm -= yProm;
    auto errorNorm = MaxNorm( bProm );
    if( progress && commRank == 0 )
        Output("original rel error: ",errorNorm/bNorm);

    Int refineIt = 0;
    const Int indent = PushIndent();
    while( true )
    {
        if( errorNorm/bNorm <= relTol )
        {
            if( progress && commRank == 0 )
                Output(errorNorm/bNorm," <= ",relTol);
            break;
        }

        // Compute the proposed update to the solution
        // -------------------------------------------
        Copy( bProm, b );
        applyAInv( b );
        Copy( b, dxProm );
        xCandProm = xProm;
        xCandProm += dxProm;

        // Check the new residual
        // ----------------------
        applyA( xCandProm, yProm );
        bProm = bOrigProm;
        bProm -= yProm;
        auto newErrorNorm = MaxNorm( bProm );
        if( progress && commRank == 0 )
            Output("refined rel error: ",newErrorNorm/bNorm);

        if( newErrorNorm < errorNorm )
            xProm = xCandProm;
        else
            break;

        errorNorm = newErrorNorm;
        ++refineIt;
        if( refineIt >= maxRefineIts )
            break;
    }
    SetIndent( indent );
    Copy( xProm, b );
    return refineIt;
}

template<typename F,class ApplyAType,class ApplyAInvType>
inline Int PromotedBatch
( const ApplyAType& applyA,
  const ApplyAInvType& applyAInv,
        DistMultiVec<F>& B,
        Int maxRefineIts,
        bool progress )
{
    DEBUG_ONLY(CSE cse("refined_solve::PromotedBatch"))
    if( maxRefineIts <= 0 )
    {
        applyAInv( B );
        return 0;
    }
    typedef Promote<F> PF;
    mpi::Comm comm = B.Comm();

    DistMultiVec<PF> BProm(comm), BOrigProm(comm);
    Copy( B, BProm );
    Copy( B, BOrigProm );

    // Compute the initial guess
    // =========================
    applyAInv( B );
    DistMultiVec<PF> XProm(comm);
    Copy( B, XProm );

    DistMultiVec<PF> dXProm(comm), YProm(comm);
    Zeros( YProm, XProm.Height(), XProm.Width() );
    applyA( XProm, YProm );
    BProm -= YProm;

    Int refineIt = 0;
    const Int indent = PushIndent();
    while( true )
    {
        // Compute the proposed updates to the solutions
        // ---------------------------------------------
        Copy( BProm, B );
        applyAInv( B );
        Copy( B, dXProm );
        XProm += dXProm;

        ++refineIt;
        if( refineIt < maxRefineIts )
        {
            // Form the new residuals
            // ----------------------
            applyA( XProm, YProm );
            BProm = BOrigProm;
            BProm -= YProm;
        }
        else
            break;
    }
    SetIndent( indent );
    Copy( XProm, B );
    return refineIt;
}

} // namespace refined_solve

template<typename F,class ApplyAType,class ApplyAInvType>
inline DisableIf<IsSame<F,Promote<F>>,Int>
PromotedRefinedSolve
( const ApplyAType& applyA,
  const ApplyAInvType& applyAInv,
        DistMultiVec<F>& B,
        Base<F> relTol,
        Int maxRefineIts,
        bool progress )
{
    DEBUG_ONLY(CSE cse("PromotedRefinedSolve"))
    if( B.Width() == 1 )
        return refined_solve::PromotedSingle
               ( applyA, applyAInv, B, relTol, maxRefineIts, progress );
    else
        return refined_solve::PromotedBatch
               ( applyA, applyAInv, B, maxRefineIts, progress );
}

template<typename F,class ApplyAType,class ApplyAInvType>
inline EnableIf<IsSame<F,Promote<F>>,Int>
PromotedRefinedSolve
( const ApplyAType& applyA,
  const ApplyAInvType& applyAInv,
        DistMultiVec<F>& B,
        Base<F> relTol,
        Int maxRefineIts,
        bool progress )
{
    DEBUG_ONLY(CSE cse("PromotedRefinedSolve"))
    return RefinedSolve( applyA, applyAInv, B, relTol, maxRefineIts, progress );
}

} // namespace El

#endif // ifndef EL_SOLVE_REFINED_HPP
