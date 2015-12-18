/* Copyright (c) 2010, RWTH Aachen University
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or 
 * without modification, are permitted provided that the following
 * conditions are met:
 *   * Redistributions of source code must retain the above 
 *     copyright notice, this list of conditions and the following
 *     disclaimer.
 *   * Redistributions in binary form must reproduce the above 
 *     copyright notice, this list of conditions and the following 
 *     disclaimer in the documentation and/or other materials 
 *     provided with the distribution.
 *   * Neither the name of the RWTH Aachen University nor the
 *     names of its contributors may be used to endorse or promote 
 *     products derived from this software without specific prior 
 *     written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL RWTH 
 * AACHEN UNIVERSITY BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF 
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE.
 *
 * Coded by Matthias Petschow (petschow@aices.rwth-aachen.de),
 * August 2010, Version 0.6
 *
 * This code was the result of a collaboration between 
 * Matthias Petschow and Paolo Bientinesi. When you use this 
 * code, kindly reference a paper related to this work.
 *
 */

#ifndef SSTRUCTS_H
#define SSTRUCTS_H

#include "mpi.h"
#include "global.h"
#include "counter.h"
#include "queue.h"

typedef struct {
  int              n;
  long double *restrict D;
  long double *restrict E;
  int              nsplit;
  int    *restrict isplit ;
  long double           spdiam;
} in_t;

typedef struct {
  int              n;
  long double           *vl;
  long double           *vu;
  int              *il;
  int              *iu;
  long double *restrict W;
  long double *restrict Werr;
  long double *restrict Wgap;
  int    *restrict Windex;
  int    *restrict iblock;
  int    *restrict iproc;
  long double *restrict Wshifted;
  long double *restrict gersch;
} val_t;

typedef struct {
  int              ldz;
  int              nz;
  double *restrict Z;
  int    *restrict Zsupp;
  int    *restrict Zindex;
} vec_t;

typedef struct {
  int      pid;
  int      nproc;
  MPI_Comm comm;
  int      nthreads;
  int      thread_support;
} proc_t;

typedef struct {
  long double split;
  long double rtol1;
  long double rtol2;
  long double pivmin;
} tol_t;

typedef struct {
  int         num_messages;
  MPI_Request *requests;
  MPI_Status  *stats;
} comm_t;

typedef struct {
  queue_t *r_queue;
  queue_t *s_queue;
  queue_t *c_queue;
} workQ_t;

typedef struct {
  long double lambda;
  int    local_ind;
  int    block_ind;
  int    ind;
} sort_struct_t;

typedef struct {
  int    n;
  long double *D;
  long double *E;
  long double *E2;
  int    il;
  int    iu;
  int    my_il;
  int    my_iu;
  int    nsplit;
  int    *isplit;
  long double bsrtol;
  long double pivmin;
  long double *gersch;
  long double *W;
  long double *Werr;
  int    *Windex;
  int    *iblock;
} auxarg1_t;

typedef struct {
  int          bl_size;
  long double       *D;
  long double       *DE2;
  int          rf_begin;
  int          rf_end;
  long double        *W;
  long double        *Werr;
  long double        *Wgap;
  int            *Windex;
  long double       rtol1;
  long double       rtol2;
  long double       pivmin;
  long double       bl_spdiam;
} auxarg2_t;

typedef struct {
  int          tid;
  proc_t       *procinfo;
  val_t        *Wstruct;
  vec_t        *Zstruct;
  tol_t        *tolstruct;
  workQ_t      *workQ;
  counter_t    *num_left;
} auxarg3_t;

#endif
