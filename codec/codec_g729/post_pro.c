/*
   ITU-T G.729 Annex C - Reference C code for floating point
                         implementation of G.729
                         Version 1.01 of 15.September.98
*/

/*
----------------------------------------------------------------------
                    COPYRIGHT NOTICE
----------------------------------------------------------------------
   ITU-T G.729 Annex C ANSI C source code
   Copyright (C) 1998, AT&T, France Telecom, NTT, University of
   Sherbrooke.  All rights reserved.

----------------------------------------------------------------------
*/

/*
 File : POST_PRO.C
 Used for the floating point version of both
 G.729 main body and G.729A
*/
#include "typedef.h"
#include "ld8a.h"
#include "tab_ld8a.h"

/*------------------------------------------------------------------------*
 * Function post_process()                                                 *
 *                                                                        *
 * Post-processing of output speech.                                      *
 *   - 2nd order high pass filter with cut off frequency at 100 Hz.       *
 *-----------------------------------------------------------------------*/

/*------------------------------------------------------------------------*
 * 2nd order high pass filter with cut off frequency at 100 Hz.           *
 * Designed with SPPACK efi command -40 dB att, 0.25 ri.                  *
 *                                                                        *
 * Algorithm:                                                             *
 *                                                                        *
 *  y[i] = b[0]*x[i] + b[1]*x[i-1] + b[2]*x[i-2]                          *
 *                   + a[1]*y[i-1] + a[2]*y[i-2];                         *
 *                                                                        *
 *     b[3] = {0.93980581E+00, -0.18795834E+01,  0.93980581E+00};         *
 *     a[3] = {0.10000000E+01, +0.19330735E+01, -0.93589199E+00};         *
 *-----------------------------------------------------------------------*/

void init_post_process(struct preproc_state_t * state)
{
  state->x0 = state->x1 = (F)0.0;
  state->y2 = state->y1 = (F)0.0;
}

void post_process(struct preproc_state_t * state,
   FLOAT signal[],      /* (i/o)  : signal                     */
   int lg               /* (i)    : lenght of signal           */
)
{
  int i;
  FLOAT x2;
  FLOAT y0;

  for(i=0; i<lg; i++)
  {
    x2 = state->x1;
    state->x1 = state->x0;
    state->x0 = signal[i];

    y0 = state->y1*a100[1] + state->y2*a100[2] + state->x0*b100[0] + state->x1*b100[1] + x2*b100[2];

    signal[i] = y0;
    state->y2 = state->y1;
    state->y1 = y0;
  }
}

