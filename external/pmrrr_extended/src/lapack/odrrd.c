/* dlarrd.f -- translated by f2c (version 20061008) */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <assert.h>

#define imax(a,b) ( (a) > (b) ? (a) : (b) )
#define imin(a,b) ( (a) < (b) ? (a) : (b) )
#define TRUE_ (1)
#define FALSE_ (0)

/* Table of constant values */
static int c__1 = 1;
static int c_n1 = -1;
static int c__3 = 3;
static int c__2 = 2;
static int c__0 = 0;

/* Subroutine */ 
int ext_odrrd_(char *range, char *order, int *n, double *vl,  
	double *vu, int *il, int *iu, double *gers, 
	double *reltol, double *d__, double *e, double *e2, 
	double *pivmin, int *nsplit, int *isplit, int *m, 
	double *w, double *werr, double *wl, double *wu, 
	int *iblock, int *indexw, double *work, int *iwork, 
	int *info)
{
    /* System generated locals */
    int i__1, i__2, i__3;
    double d__1, d__2;

    /* Builtin functions */
    // double log(double);

    /* Local variables */
    int i__, j, ib, ie, je, nb;
    double gl;
    int im, in;
    double gu;
    int iw, jee;
    double eps;
    int nwl;
    double wlu, wul;
    int nwu;
    double tmp1, tmp2;
    int iend, jblk, ioff, iout, itmp1, itmp2, jdisc;
    extern int ext_olsame_(char *, char *);
    int iinfo;
    double atoli;
    int iwoff, itmax;
    double wkill, rtoli, uflow, tnorm;
    // extern double dlamch_(char *);
    int ibegin;
    extern /* Subroutine */ int ext_odebz2_(int *, int *, int *, 
	    int *, int *, int *, double *, double *, 
	    double *, double *, double *, double *, int *, 
	     double *, double *, int *, int *, double *, 
	    int *, int *);
    int irange, idiscl, idumma[1];
    /* extern int ilaenv_(int *, char *, char *, int *, int *,  */
    /* 	    int *, int *); */
    int idiscu;
    int ncnvrg, toofew;


/*  -- LAPACK auxiliary routine (version 3.2.1)                        -- */
/*  -- LAPACK is a software package provided by Univ. of Tennessee,    -- */
/*  -- Univ. of California Berkeley, Univ. of Colorado Denver and NAG Ltd..-- */
/*  -- April 2009                                                      -- */

/*     .. Scalar Arguments .. */
/*     .. */
/*     .. Array Arguments .. */
/*     .. */

/*  Purpose */
/*  ======= */

/*  ODRRD computes the eigenvalues of a symmetric tridiagonal */
/*  matrix T to suitable accuracy. This is an auxiliary code to be */
/*  called from DSTEMR. */
/*  The user may ask for all eigenvalues, all eigenvalues */
/*  in the half-open interval (VL, VU], or the IL-th through IU-th */
/*  eigenvalues. */

/*  To avoid overflow, the matrix must be scaled so that its */
/*  largest element is no greater than overflow**(1/2) * */
/*  underflow**(1/4) in absolute value, and for greatest */
/*  accuracy, it should not be much smaller than that. */

/*  See W. Kahan "Accurate Eigenvalues of a Symmetric Tridiagonal */
/*  Matrix", Report CS41, Computer Science Dept., Stanford */
/*  University, July 21, 1966. */

/*  Arguments */
/*  ========= */

/*  RANGE   (input) CHARACTER */
/*          = 'A': ("All")   all eigenvalues will be found. */
/*          = 'V': ("Value") all eigenvalues in the half-open interval */
/*                           (VL, VU] will be found. */
/*          = 'I': ("Index") the IL-th through IU-th eigenvalues (of the */
/*                           entire matrix) will be found. */

/*  ORDER   (input) CHARACTER */
/*          = 'B': ("By Block") the eigenvalues will be grouped by */
/*                              split-off block (see IBLOCK, ISPLIT) and */
/*                              ordered from smallest to largest within */
/*                              the block. */
/*          = 'E': ("Entire matrix") */
/*                              the eigenvalues for the entire matrix */
/*                              will be ordered from smallest to */
/*                              largest. */

/*  N       (input) INT */
/*          The order of the tridiagonal matrix T.  N >= 0. */

/*  VL      (input) DOUBLE PRECISION */
/*  VU      (input) DOUBLE PRECISION */
/*          If RANGE='V', the lower and upper bounds of the interval to */
/*          be searched for eigenvalues.  Eigenvalues less than or equal */
/*          to VL, or greater than VU, will not be returned.  VL < VU. */
/*          Not referenced if RANGE = 'A' or 'I'. */

/*  IL      (input) INT */
/*  IU      (input) INT */
/*          If RANGE='I', the indices (in ascending order) of the */
/*          smallest and largest eigenvalues to be returned. */
/*          1 <= IL <= IU <= N, if N > 0; IL = 1 and IU = 0 if N = 0. */
/*          Not referenced if RANGE = 'A' or 'V'. */

/*  GERS    (input) DOUBLE PRECISION array, dimension (2*N) */
/*          The N Gerschgorin intervals (the i-th Gerschgorin interval */
/*          is (GERS(2*i-1), GERS(2*i)). */

/*  RELTOL  (input) DOUBLE PRECISION */
/*          The minimum relative width of an interval.  When an interval */
/*          is narrower than RELTOL times the larger (in */
/*          magnitude) endpoint, then it is considered to be */
/*          sufficiently small, i.e., converged.  Note: this should */
/*          always be at least radix*machine epsilon. */

/*  D       (input) DOUBLE PRECISION array, dimension (N) */
/*          The n diagonal elements of the tridiagonal matrix T. */

/*  E       (input) DOUBLE PRECISION array, dimension (N-1) */
/*          The (n-1) off-diagonal elements of the tridiagonal matrix T. */

/*  E2      (input) DOUBLE PRECISION array, dimension (N-1) */
/*          The (n-1) squared off-diagonal elements of the tridiagonal matrix T. */

/*  PIVMIN  (input) DOUBLE PRECISION */
/*          The minimum pivot allowed in the Sturm sequence for T. */

/*  NSPLIT  (input) INT */
/*          The number of diagonal blocks in the matrix T. */
/*          1 <= NSPLIT <= N. */

/*  ISPLIT  (input) INT array, dimension (N) */
/*          The splitting points, at which T breaks up into submatrices. */
/*          The first submatrix consists of rows/columns 1 to ISPLIT(1), */
/*          the second of rows/columns ISPLIT(1)+1 through ISPLIT(2), */
/*          etc., and the NSPLIT-th consists of rows/columns */
/*          ISPLIT(NSPLIT-1)+1 through ISPLIT(NSPLIT)=N. */
/*          (Only the first NSPLIT elements will actually be used, but */
/*          since the user cannot know a priori what value NSPLIT will */
/*          have, N words must be reserved for ISPLIT.) */

/*  M       (output) INT */
/*          The actual number of eigenvalues found. 0 <= M <= N. */
/*          (See also the description of INFO=2,3.) */

/*  W       (output) DOUBLE PRECISION array, dimension (N) */
/*          On exit, the first M elements of W will contain the */
/*          eigenvalue approximations. ODRRD computes an interval */
/*          I_j = (a_j, b_j] that includes eigenvalue j. The eigenvalue */
/*          approximation is given as the interval midpoint */
/*          W(j)= ( a_j + b_j)/2. The corresponding error is bounded by */
/*          WERR(j) = abs( a_j - b_j)/2 */

/*  WERR    (output) DOUBLE PRECISION array, dimension (N) */
/*          The error bound on the corresponding eigenvalue approximation */
/*          in W. */

/*  WL      (output) DOUBLE PRECISION */
/*  WU      (output) DOUBLE PRECISION */
/*          The interval (WL, WU] contains all the wanted eigenvalues. */
/*          If RANGE='V', then WL=VL and WU=VU. */
/*          If RANGE='A', then WL and WU are the global Gerschgorin bounds */
/*                        on the spectrum. */
/*          If RANGE='I', then WL and WU are computed by ODEBZ from the */
/*                        index range specified. */

/*  IBLOCK  (output) INT array, dimension (N) */
/*          At each row/column j where E(j) is zero or small, the */
/*          matrix T is considered to split into a block diagonal */
/*          matrix.  On exit, if INFO = 0, IBLOCK(i) specifies to which */
/*          block (from 1 to the number of blocks) the eigenvalue W(i) */
/*          belongs.  (ODRRD may use the remaining N-M elements as */
/*          workspace.) */

/*  INDEXW  (output) INT array, dimension (N) */
/*          The indices of the eigenvalues within each block (submatrix); */
/*          for example, INDEXW(i)= j and IBLOCK(i)=k imply that the */
/*          i-th eigenvalue W(i) is the j-th eigenvalue in block k. */

/*  WORK    (workspace) DOUBLE PRECISION array, dimension (4*N) */

/*  IWORK   (workspace) INT array, dimension (3*N) */

/*  INFO    (output) INT */
/*          = 0:  successful exit */
/*          < 0:  if INFO = -i, the i-th argument had an illegal value */
/*          > 0:  some or all of the eigenvalues failed to converge or */
/*                were not computed: */
/*                =1 or 3: Bisection failed to converge for some */
/*                        eigenvalues; these eigenvalues are flagged by a */
/*                        negative block number.  The effect is that the */
/*                        eigenvalues may not be as accurate as the */
/*                        absolute and relative tolerances.  This is */
/*                        generally caused by unexpectedly inaccurate */
/*                        arithmetic. */
/*                =2 or 3: RANGE='I' only: Not all of the eigenvalues */
/*                        IL:IU were found. */
/*                        Effect: M < IU+1-IL */
/*                        Cause:  non-monotonic arithmetic, causing the */
/*                                Sturm sequence to be non-monotonic. */
/*                        Cure:   recalculate, using RANGE='A', and pick */
/*                                out eigenvalues IL:IU.  In some cases, */
/*                                increasing the PARAMETER "FUDGE" may */
/*                                make things work. */
/*                = 4:    RANGE='I', and the Gershgorin interval */
/*                        initially used was too small.  No eigenvalues */
/*                        were computed. */
/*                        Probable cause: your machine has sloppy */
/*                                        floating-point arithmetic. */
/*                        Cure: Increase the PARAMETER "FUDGE", */
/*                              recompile, and try again. */

/*  Internal Parameters */
/*  =================== */

/*  FUDGE   DOUBLE PRECISION, default = 2 */
/*          A "fudge factor" to widen the Gershgorin intervals.  Ideally, */
/*          a value of 1 should work, but on machines with sloppy */
/*          arithmetic, this needs to be larger.  The default for */
/*          publicly released versions should be large enough to handle */
/*          the worst machine around.  Note that this has no effect */
/*          on accuracy of the solution. */

/*  Based on contributions by */
/*     W. Kahan, University of California, Berkeley, USA */
/*     Beresford Parlett, University of California, Berkeley, USA */
/*     Jim Demmel, University of California, Berkeley, USA */
/*     Inderjit Dhillon, University of Texas, Austin, USA */
/*     Osni Marques, LBNL/NERSC, USA */
/*     Christof Voemel, University of California, Berkeley, USA */

/*  ===================================================================== */

/*     .. Parameters .. */
/*     .. */
/*     .. Local Scalars .. */
/*     .. */
/*     .. Local Arrays .. */
/*     .. */
/*     .. External Functions .. */
/*     .. */
/*     .. External Subroutines .. */
/*     .. */
/*     .. Intrinsic Functions .. */
/*     .. */
/*     .. Executable Statements .. */

    /* Parameter adjustments */
    --iwork;
    --work;
    --indexw;
    --iblock;
    --werr;
    --w;
    --isplit;
    --e2;
    --e;
    --d__;
    --gers;

    /* Function Body */
    *info = 0;

/*     Decode RANGE */

    if (ext_olsame_(range, "A")) {
	irange = 1;
    } else if (ext_olsame_(range, "V")) {
	irange = 2;
    } else if (ext_olsame_(range, "I")) {
	irange = 3;
    } else {
	irange = 0;
    }

/*     Check for Errors */

    if (irange <= 0) {
	*info = -1;
    } else if (! (ext_olsame_(order, "B") || ext_olsame_(order, 
	    "E"))) {
	*info = -2;
    } else if (*n < 0) {
	*info = -3;
    } else if (irange == 2) {
	if (*vl >= *vu) {
	    *info = -5;
	}
    } else if (irange == 3 && (*il < 1 || *il > imax(1,*n))) {
	*info = -6;
    } else if (irange == 3 && (*iu < imin(*n,*il) || *iu > *n)) {
	*info = -7;
    }

    if (*info != 0) {
	return 0;
    }
/*     Initialize error flags */
    *info = 0;
    ncnvrg = FALSE_;
    toofew = FALSE_;
/*     Quick return if possible */
    *m = 0;
    if (*n == 0) {
	return 0;
    }
/*     Simplification: */
    if (irange == 3 && *il == 1 && *iu == *n) {
	irange = 1;
    }
/*     Get machine constants */
    eps = DBL_EPSILON; // odmch_("P");
    uflow = DBL_MIN;  // odmch_("U");
/*     Special Case when N=1 */
/*     Treat case of 1x1 matrix for quick return */
    if (*n == 1) {
	if (irange == 1 || irange == 2 && d__[1] > *vl && d__[1] <= *vu || 
		irange == 3 && *il == 1 && *iu == 1) {
	    *m = 1;
	    w[1] = d__[1];
/*           The computation error of the eigenvalue is zero */
	    werr[1] = 0.;
	    iblock[1] = 1;
	    indexw[1] = 1;
	}
	return 0;
    }
/*     NB is the minimum vector length for vector bisection, or 0 */
/*     if only scalar is to be done. */
    nb = 1; // ilaenv_(&c__1, "DSTEBZ", " ", n, &c_n1, &c_n1, &c_n1);
    if (nb <= 1) {
	nb = 0;
    }
/*     Find global spectral radius */
    gl = d__[1];
    gu = d__[1];
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
/* Computing MIN */
	d__1 = gl, d__2 = gers[(i__ << 1) - 1];
	gl = fmin(d__1,d__2);
/* Computing MAX */
	d__1 = gu, d__2 = gers[i__ * 2];
	gu = fmax(d__1,d__2);
/* L5: */
    }
/*     Compute global Gerschgorin bounds and spectral diameter */
/* Computing MAX */
    d__1 = fabs(gl), d__2 = fabs(gu);
    tnorm = fmax(d__1,d__2);
    gl = gl - tnorm * 2. * eps * *n - *pivmin * 4.;
    gu = gu + tnorm * 2. * eps * *n + *pivmin * 4.;
/*     [JAN/28/2009] remove the line below since SPDIAM variable not use */
/*     SPDIAM = GU - GL */
/*     Input arguments for ODEBZ: */
/*     The relative tolerance.  An interval (a,b] lies within */
/*     "relative tolerance" if  b-a < RELTOL*max(|a|,|b|), */
    rtoli = *reltol;
/*     Set the absolute tolerance for interval convergence to zero to force */
/*     interval convergence based on relative size of the interval. */
/*     This is dangerous because intervals might not converge when RELTOL is */
/*     small. But at least a very small number should be selected so that for */
/*     strongly graded matrices, the code can get relatively accurate */
/*     eigenvalues. */
    atoli = uflow * 4. + *pivmin * 4.;
    if (irange == 3) {
/*        RANGE='I': Compute an interval containing eigenvalues */
/*        IL through IU. The initial interval [GL,GU] from the global */
/*        Gerschgorin bounds GL and GU is refined by ODEBZ. */
	itmax = (int) ((log(tnorm + *pivmin) - log(*pivmin)) / log(2.)) + 
		2;
	work[*n + 1] = gl;
	work[*n + 2] = gl;
	work[*n + 3] = gu;
	work[*n + 4] = gu;
	work[*n + 5] = gl;
	work[*n + 6] = gu;
	iwork[1] = -1;
	iwork[2] = -1;
	iwork[3] = *n + 1;
	iwork[4] = *n + 1;
	iwork[5] = *il - 1;
	iwork[6] = *iu;

	ext_odebz2_(&c__3, &itmax, n, &c__2, &c__2, &nb, &atoli, &rtoli, pivmin, &
		d__[1], &e[1], &e2[1], &iwork[5], &work[*n + 1], &work[*n + 5]
, &iout, &iwork[1], &w[1], &iblock[1], &iinfo);
	if (iinfo != 0) {
	    *info = iinfo;
	    return 0;
	}
/*        On exit, output intervals may not be ordered by ascending negcount */
	if (iwork[6] == *iu) {
	    *wl = work[*n + 1];
	    wlu = work[*n + 3];
	    nwl = iwork[1];
	    *wu = work[*n + 4];
	    wul = work[*n + 2];
	    nwu = iwork[4];
	} else {
	    *wl = work[*n + 2];
	    wlu = work[*n + 4];
	    nwl = iwork[2];
	    *wu = work[*n + 3];
	    wul = work[*n + 1];
	    nwu = iwork[3];
	}
/*        On exit, the interval [WL, WLU] contains a value with negcount NWL, */
/*        and [WUL, WU] contains a value with negcount NWU. */
	if (nwl < 0 || nwl >= *n || nwu < 1 || nwu > *n) {
	    *info = 4;
	    return 0;
	}
    } else if (irange == 2) {
	*wl = *vl;
	*wu = *vu;
    } else if (irange == 1) {
	*wl = gl;
	*wu = gu;
    }
/*     Find Eigenvalues -- Loop Over blocks and recompute NWL and NWU. */
/*     NWL accumulates the number of eigenvalues .le. WL, */
/*     NWU accumulates the number of eigenvalues .le. WU */
    *m = 0;
    iend = 0;
    *info = 0;
    nwl = 0;
    nwu = 0;

    i__1 = *nsplit;
    for (jblk = 1; jblk <= i__1; ++jblk) {
	ioff = iend;
	ibegin = ioff + 1;
	iend = isplit[jblk];
	in = iend - ioff;

	if (in == 1) {
/*           1x1 block */
	    if (*wl >= d__[ibegin] - *pivmin) {
		++nwl;
	    }
	    if (*wu >= d__[ibegin] - *pivmin) {
		++nwu;
	    }
	    if (irange == 1 || *wl < d__[ibegin] - *pivmin && *wu >= d__[
		    ibegin] - *pivmin) {
		++(*m);
		w[*m] = d__[ibegin];
		werr[*m] = 0.;
/*              The gap for a single block doesn't matter for the later */
/*              algorithm and is assigned an arbitrary large value */
		iblock[*m] = jblk;
		indexw[*m] = 1;
	    }
/*        Disabled 2x2 case because of a failure on the following matrix */
/*        RANGE = 'I', IL = IU = 4 */
/*          Original Tridiagonal, d = [ */
/*           -0.150102010615740E+00 */
/*           -0.849897989384260E+00 */
/*           -0.128208148052635E-15 */
/*            0.128257718286320E-15 */
/*          ]; */
/*          e = [ */
/*           -0.357171383266986E+00 */
/*           -0.180411241501588E-15 */
/*           -0.175152352710251E-15 */
/*          ]; */

/*         ELSE IF( IN.EQ.2 ) THEN */
/* *           2x2 block */
/*            DISC = SQRT( (HALF*(D(IBEGIN)-D(IEND)))**2 + E(IBEGIN)**2 ) */
/*            TMP1 = HALF*(D(IBEGIN)+D(IEND)) */
/*            L1 = TMP1 - DISC */
/*            IF( WL.GE. L1-PIVMIN ) */
/*     $         NWL = NWL + 1 */
/*            IF( WU.GE. L1-PIVMIN ) */
/*     $         NWU = NWU + 1 */
/*            IF( IRANGE.EQ.ALLRNG .OR. ( WL.LT.L1-PIVMIN .AND. WU.GE. */
/*     $          L1-PIVMIN ) ) THEN */
/*               M = M + 1 */
/*               W( M ) = L1 */
/* *              The uncertainty of eigenvalues of a 2x2 matrix is very small */
/*               WERR( M ) = EPS * ABS( W( M ) ) * TWO */
/*               IBLOCK( M ) = JBLK */
/*               INDEXW( M ) = 1 */
/*            ENDIF */
/*            L2 = TMP1 + DISC */
/*            IF( WL.GE. L2-PIVMIN ) */
/*     $         NWL = NWL + 1 */
/*            IF( WU.GE. L2-PIVMIN ) */
/*     $         NWU = NWU + 1 */
/*            IF( IRANGE.EQ.ALLRNG .OR. ( WL.LT.L2-PIVMIN .AND. WU.GE. */
/*     $          L2-PIVMIN ) ) THEN */
/*               M = M + 1 */
/*               W( M ) = L2 */
/* *              The uncertainty of eigenvalues of a 2x2 matrix is very small */
/*               WERR( M ) = EPS * ABS( W( M ) ) * TWO */
/*               IBLOCK( M ) = JBLK */
/*               INDEXW( M ) = 2 */
/*            ENDIF */
	} else {
/*           General Case - block of size IN >= 2 */
/*           Compute local Gerschgorin interval and use it as the initial */
/*           interval for ODEBZ */
	    gu = d__[ibegin];
	    gl = d__[ibegin];
	    tmp1 = 0.;
	    i__2 = iend;
	    for (j = ibegin; j <= i__2; ++j) {
/* Computing MIN */
		d__1 = gl, d__2 = gers[(j << 1) - 1];
		gl = fmin(d__1,d__2);
/* Computing MAX */
		d__1 = gu, d__2 = gers[j * 2];
		gu = fmax(d__1,d__2);
/* L40: */
	    }
/*           [JAN/28/2009] */
/*           change SPDIAM by TNORM in lines 2 and 3 thereafter */
/*           line 1: remove computation of SPDIAM (not useful anymore) */
/*           SPDIAM = GU - GL */
/*           GL = GL - FUDGE*SPDIAM*EPS*IN - FUDGE*PIVMIN */
/*           GU = GU + FUDGE*SPDIAM*EPS*IN + FUDGE*PIVMIN */
	    gl = gl - tnorm * 2. * eps * in - *pivmin * 2.;
	    gu = gu + tnorm * 2. * eps * in + *pivmin * 2.;

	    if (irange > 1) {
		if (gu < *wl) {
/*                 the local block contains none of the wanted eigenvalues */
		    nwl += in;
		    nwu += in;
		    goto L70;
		}
/*              refine search interval if possible, only range (WL,WU] matters */
		gl = fmax(gl,*wl);
		gu = fmin(gu,*wu);
		if (gl >= gu) {
		    goto L70;
		}
	    }
/*           Find negcount of initial interval boundaries GL and GU */
	    work[*n + 1] = gl;
	    work[*n + in + 1] = gu;
	    ext_odebz2_(&c__1, &c__0, &in, &in, &c__1, &nb, &atoli, &rtoli, 
		    pivmin, &d__[ibegin], &e[ibegin], &e2[ibegin], idumma, &
		    work[*n + 1], &work[*n + (in << 1) + 1], &im, &iwork[1], &
		    w[*m + 1], &iblock[*m + 1], &iinfo);
	    if (iinfo != 0) {
		*info = iinfo;
		return 0;
	    }

	    nwl += iwork[1];
	    nwu += iwork[in + 1];
	    iwoff = *m - iwork[1];
/*           Compute Eigenvalues */
	    itmax = (int) ((log(gu - gl + *pivmin) - log(*pivmin)) / log(
		    2.)) + 2;
	    ext_odebz2_(&c__2, &itmax, &in, &in, &c__1, &nb, &atoli, &rtoli, 
		    pivmin, &d__[ibegin], &e[ibegin], &e2[ibegin], idumma, &
		    work[*n + 1], &work[*n + (in << 1) + 1], &iout, &iwork[1], 
		     &w[*m + 1], &iblock[*m + 1], &iinfo);
	    if (iinfo != 0) {
		*info = iinfo;
		return 0;
	    }

/*           Copy eigenvalues into W and IBLOCK */
/*           Use -JBLK for block number for unconverged eigenvalues. */
/*           Loop over the number of output intervals from ODEBZ */
	    i__2 = iout;
	    for (j = 1; j <= i__2; ++j) {
/*              eigenvalue approximation is middle point of interval */
		tmp1 = (work[j + *n] + work[j + in + *n]) * .5;
/*              semi length of error interval */
		tmp2 = (d__1 = work[j + *n] - work[j + in + *n], fabs(d__1)) * 
			.5;
		if (j > iout - iinfo) {
/*                 Flag non-convergence. */
		    ncnvrg = TRUE_;
		    ib = -jblk;
		} else {
		    ib = jblk;
		}
		i__3 = iwork[j + in] + iwoff;
		for (je = iwork[j] + 1 + iwoff; je <= i__3; ++je) {
		    w[je] = tmp1;
		    werr[je] = tmp2;
		    indexw[je] = je - iwoff;
		    iblock[je] = ib;
/* L50: */
		}
/* L60: */
	    }

	    *m += im;
	}
L70:
	;
    }
/*     If RANGE='I', then (WL,WU) contains eigenvalues NWL+1,...,NWU */
/*     If NWL+1 < IL or NWU > IU, discard extra eigenvalues. */
    if (irange == 3) {
	idiscl = *il - 1 - nwl;
	idiscu = nwu - *iu;

	if (idiscl > 0) {
	    im = 0;
	    i__1 = *m;
	    for (je = 1; je <= i__1; ++je) {
/*              Remove some of the smallest eigenvalues from the left so that */
/*              at the end IDISCL =0. Move all eigenvalues up to the left. */
		if (w[je] <= wlu && idiscl > 0) {
		    --idiscl;
		} else {
		    ++im;
		    w[im] = w[je];
		    werr[im] = werr[je];
		    indexw[im] = indexw[je];
		    iblock[im] = iblock[je];
		}
/* L80: */
	    }
	    *m = im;
	}
	if (idiscu > 0) {
/*           Remove some of the largest eigenvalues from the right so that */
/*           at the end IDISCU =0. Move all eigenvalues up to the left. */
	    im = *m + 1;
	    for (je = *m; je >= 1; --je) {
		if (w[je] >= wul && idiscu > 0) {
		    --idiscu;
		} else {
		    --im;
		    w[im] = w[je];
		    werr[im] = werr[je];
		    indexw[im] = indexw[je];
		    iblock[im] = iblock[je];
		}
/* L81: */
	    }
	    jee = 0;
	    i__1 = *m;
	    for (je = im; je <= i__1; ++je) {
		++jee;
		w[jee] = w[je];
		werr[jee] = werr[je];
		indexw[jee] = indexw[je];
		iblock[jee] = iblock[je];
/* L82: */
	    }
	    *m = *m - im + 1;
	}
	if (idiscl > 0 || idiscu > 0) {
/*           Code to deal with effects of bad arithmetic. (If N(w) is */
/*           monotone non-decreasing, this should never happen.) */
/*           Some low eigenvalues to be discarded are not in (WL,WLU], */
/*           or high eigenvalues to be discarded are not in (WUL,WU] */
/*           so just kill off the smallest IDISCL/largest IDISCU */
/*           eigenvalues, by marking the corresponding IBLOCK = 0 */
	    if (idiscl > 0) {
		wkill = *wu;
		i__1 = idiscl;
		for (jdisc = 1; jdisc <= i__1; ++jdisc) {
		    iw = 0;
		    i__2 = *m;
		    for (je = 1; je <= i__2; ++je) {
			if (iblock[je] != 0 && (w[je] < wkill || iw == 0)) {
			    iw = je;
			    wkill = w[je];
			}
/* L90: */
		    }
		    iblock[iw] = 0;
/* L100: */
		}
	    }
	    if (idiscu > 0) {
		wkill = *wl;
		i__1 = idiscu;
		for (jdisc = 1; jdisc <= i__1; ++jdisc) {
		    iw = 0;
		    i__2 = *m;
		    for (je = 1; je <= i__2; ++je) {
			if (iblock[je] != 0 && (w[je] >= wkill || iw == 0)) {
			    iw = je;
			    wkill = w[je];
			}
/* L110: */
		    }
		    iblock[iw] = 0;
/* L120: */
		}
	    }
/*           Now erase all eigenvalues with IBLOCK set to zero */
	    im = 0;
	    i__1 = *m;
	    for (je = 1; je <= i__1; ++je) {
		if (iblock[je] != 0) {
		    ++im;
		    w[im] = w[je];
		    werr[im] = werr[je];
		    indexw[im] = indexw[je];
		    iblock[im] = iblock[je];
		}
/* L130: */
	    }
	    *m = im;
	}
	if (idiscl < 0 || idiscu < 0) {
	    toofew = TRUE_;
	}
    }

    if (irange == 1 && *m != *n || irange == 3 && *m != *iu - *il + 1) {
	toofew = TRUE_;
    }
/*     If ORDER='B', do nothing the eigenvalues are already sorted by */
/*        block. */
/*     If ORDER='E', sort the eigenvalues from smallest to largest */
    if (ext_olsame_(order, "E") && *nsplit > 1) {
	i__1 = *m - 1;
	for (je = 1; je <= i__1; ++je) {
	    ie = 0;
	    tmp1 = w[je];
	    i__2 = *m;
	    for (j = je + 1; j <= i__2; ++j) {
		if (w[j] < tmp1) {
		    ie = j;
		    tmp1 = w[j];
		}
/* L140: */
	    }
	    if (ie != 0) {
		tmp2 = werr[ie];
		itmp1 = iblock[ie];
		itmp2 = indexw[ie];
		w[ie] = w[je];
		werr[ie] = werr[je];
		iblock[ie] = iblock[je];
		indexw[ie] = indexw[je];
		w[je] = tmp1;
		werr[je] = tmp2;
		iblock[je] = itmp1;
		indexw[je] = itmp2;
	    }
/* L150: */
	}
    }

    *info = 0;
    if (ncnvrg) {
	++(*info);
    }
    if (toofew) {
	*info += 2;
    }
    return 0;

/*     End of ODRRD */

} /* ext_odrrd_ */
