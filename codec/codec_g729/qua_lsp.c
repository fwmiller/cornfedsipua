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
 File : QUA_LSP.C
 Used for the floating point version of both
 G.729 main body and G.729A
*/

/*----------------------------------------------------------*
 *  qua_lsp.c                                               *
 *  ~~~~~~~~                                                *
 * Functions related to the quantization of LSP's           *
 *----------------------------------------------------------*/
#include <math.h>
#include "typedef.h"
#include "ld8a.h"
#include "tab_ld8a.h"

/* Prototype definitions of static functions */

static void get_wegt( FLOAT     flsp[], FLOAT   wegt[] );
static void relspwed( FLOAT *lsp, FLOAT wegt[M], FLOAT *lspq,
                     const FLOAT lspcb1[][M], const FLOAT lspcb2[][M],
                     const FLOAT fg[MODE][MA_NP][M],
                     FLOAT freq_prev[MA_NP][M], const FLOAT fg_sum[MODE][M],
                     const FLOAT fg_sum_inv[MODE][M], int cod[] );
static void lsp_pre_select( FLOAT rbuf[], const FLOAT lspcb1[][M], int *cand);
static void lsp_select_1( FLOAT rbuf[], const FLOAT lspcb1[], FLOAT wegt[],
                         const FLOAT lspcb2[][M], int        *index );
static void lsp_select_2( FLOAT rbuf[], const FLOAT lspcb1[], FLOAT wegt[],
                         const FLOAT lspcb2[][M], int *index );
static void lsp_last_select( FLOAT      tdist[MODE], int        *mode_index );
static void lsp_get_tdist( FLOAT        wegt[], FLOAT   buf[],
                          FLOAT *tdist, FLOAT   rbuf[], const FLOAT   fg_sum[] );
static void lsp_qua_cs( struct lsp_cod_state_t *,
					   FLOAT *freq_in, FLOAT *freqout, int *cod);


void qua_lsp(struct lsp_cod_state_t * state,
  FLOAT lsp[],       /* (i) : Unquantized LSP            */
  FLOAT lsp_q[],     /* (o) : Quantized LSP              */
  int ana[]          /* (o) : indexes                    */
)
{
  int i;
  FLOAT lsf[M], lsf_q[M];  /* domain 0.0<= lsf <PI */

  /* Convert LSPs to LSFs */

  for (i=0; i<M; i++ )
     lsf[i] = (FLOAT)acos(lsp[i]);

  lsp_qua_cs(state, lsf, lsf_q, ana );

  /* Convert LSFs to LSPs */

  for (i=0; i<M; i++ )
     lsp_q[i] = (FLOAT)cos(lsf_q[i]);
}

/*----------------------------------------------------------------------------
 * lsp_encw_reset - set the previous LSP vector
 *----------------------------------------------------------------------------
 */
void lsp_encw_reset(struct lsp_cod_state_t * state)
{
   int  i;
   for(i=0; i<MA_NP; i++)
     copy (freq_prev_reset, &state->freq_prev[i][0], M );
}
/*----------------------------------------------------------------------------
 * lsp_qua_cs - lsp quantizer
 *----------------------------------------------------------------------------
 */
static void lsp_qua_cs(struct lsp_cod_state_t * state,
 FLOAT  *flsp_in,       /*  input : Original LSP parameters      */
 FLOAT  *lspq_out,       /*  output: Quantized LSP parameters     */
 int  *code             /*  output: codes of the selected LSP    */
)
{
   FLOAT        wegt[M];   /* weight coef. */

   get_wegt( flsp_in, wegt );

   relspwed( flsp_in, wegt, lspq_out, lspcb1, lspcb2, fg,
            state->freq_prev, fg_sum, fg_sum_inv, code);
}
/*----------------------------------------------------------------------------
 * relspwed -
 *----------------------------------------------------------------------------
 */
static void relspwed(
 FLOAT  lsp[],                  /*input: unquantized LSP parameters  */
 FLOAT  wegt[],                 /*input: weight coef.                */
 FLOAT  lspq[],                 /*output:quantized LSP parameters    */
 const FLOAT  lspcb1[][M],            /*input: first stage LSP codebook    */
 const FLOAT  lspcb2[][M],            /*input: Second stage LSP codebook   */
 const FLOAT  fg[MODE][MA_NP][M],     /*input: MA prediction coef.         */
 FLOAT  freq_prev[MA_NP][M],    /*input: previous LSP vector         */
 const FLOAT  fg_sum[MODE][M],        /*input: present MA prediction coef. */
 const FLOAT  fg_sum_inv[MODE][M],    /*input: inverse coef.               */
 int    code_ana[]              /*output:codes of the selected LSP   */
)
{
   int  mode, j;
   int  index, mode_index;
   int  cand[MODE], cand_cur;
   int  tindex1[MODE], tindex2[MODE];
   FLOAT        tdist[MODE];
   FLOAT        rbuf[M];
   FLOAT        buf[M];

   for(mode = 0; mode<MODE; mode++) {

      lsp_prev_extract(lsp, rbuf, fg[mode], freq_prev, fg_sum_inv[mode]);

      /*----- search the first stage lsp codebook -----*/
      lsp_pre_select(rbuf, lspcb1, &cand_cur);
      cand[mode]=cand_cur;

      /*----- search the second stage lsp codebook (lower 0-4) ----- */
      lsp_select_1(rbuf, lspcb1[cand_cur], wegt, lspcb2, &index);

      tindex1[mode] = index;

      for(j=0; j<NC; j++)
        buf[j]=lspcb1[cand_cur][j]+lspcb2[index][j];

      lsp_expand_1(buf, GAP1);  /* check */

      /*----- search the second stage lsp codebook (Higher 5-9) ----- */
      lsp_select_2(rbuf, lspcb1[cand_cur], wegt, lspcb2,
                   &index);

      tindex2[mode] = index;

      for(j=NC; j<M; j++)
        buf[j]=lspcb1[cand_cur][j]+lspcb2[index][j];
      lsp_expand_2(buf, GAP1);  /* check */


      /* check */
      lsp_expand_1_2(buf, GAP2);

      lsp_get_tdist(wegt, buf, &tdist[mode], rbuf,
                    fg_sum[mode]);  /* calculate the distortion */

   } /* mode */


   lsp_last_select(tdist, &mode_index); /* select the codes */

   /* pack codes for lsp parameters */
   code_ana[0] = (mode_index<<NC0_B) | cand[mode_index];
   code_ana[1] = (tindex1[mode_index]<<NC1_B) | tindex2[mode_index];

   /* reconstruct quantized LSP parameter and check the stabilty */
   lsp_get_quant(lspcb1, lspcb2, cand[mode_index],
                 tindex1[mode_index], tindex2[mode_index],
                 fg[mode_index],
                 freq_prev,
                 lspq, fg_sum[mode_index]);
}

/*----------------------------------------------------------------------------
 * lsp_pre_select - select the code of first stage lsp codebook
 *----------------------------------------------------------------------------
 */
static void lsp_pre_select(
 FLOAT  rbuf[],         /*input : target vetor             */
 const FLOAT  lspcb1[][M],    /*input : first stage lsp codebook */
 int    *cand           /*output: selected code            */
)
{
   int  i, j;
   FLOAT dmin, dist, temp;

   /* calculate the distortion */

   *cand = 0;
   dmin= FLT_MAX_G729;
   for(i=0; i<NC0; i++) {
      dist =(F)0.;
      for(j=0; j<M; j++){
        temp = rbuf[j]-lspcb1[i][j];
        dist += temp * temp;
      }

      if(dist<dmin)
      {
        dmin=dist;
        *cand=i;
      }
    }
}

/*----------------------------------------------------------------------------
 * lsp_pre_select_1 - select the code of second stage lsp codebook (lower 0-4)
 *----------------------------------------------------------------------------
 */
static void lsp_select_1(
 FLOAT  rbuf[],         /*input : target vector            */
 const FLOAT  lspcb1[],       /*input : first stage lsp codebook */
 FLOAT  wegt[],         /*input : weight coef.             */
 const FLOAT  lspcb2[][M],    /*input : second stage lsp codebook*/
 int    *index          /*output: selected codebook index     */
)
{
   int  j, k1;
   FLOAT        buf[M];
   FLOAT        dist, dmin, tmp;

   for(j=0; j<NC; j++)
        buf[j]=rbuf[j]-lspcb1[j];

   *index = 0;
   dmin=FLT_MAX_G729;
   for(k1 = 0; k1<NC1; k1++) {
      /* calculate the distortion */
      dist = (F)0.;
      for(j=0; j<NC; j++) {
         tmp = buf[j]-lspcb2[k1][j];
         dist += wegt[j] * tmp * tmp;
      }

      if(dist<dmin) {
         dmin = dist;
         *index = k1;
      }
   }
}

/*----------------------------------------------------------------------------
 * lsp_pre_select_2 - select the code of second stage lsp codebook (higher 5-9)
 *----------------------------------------------------------------------------
 */
static void lsp_select_2(
 FLOAT  rbuf[],         /*input : target vector            */
 const FLOAT  lspcb1[],       /*input : first stage lsp codebook */
 FLOAT  wegt[],         /*input : weighting coef.             */
 const FLOAT  lspcb2[][M],    /*input : second stage lsp codebook*/
 int    *index          /*output: selected codebook index    */
)
{
   int  j, k1;
   FLOAT        buf[M];
   FLOAT        dist, dmin, tmp;

   for(j=NC; j<M; j++)
        buf[j]=rbuf[j]-lspcb1[j];


   *index = 0;
   dmin= FLT_MAX_G729;
   for(k1 = 0; k1<NC1; k1++) {
      dist = (F)0.0;
      for(j=NC; j<M; j++) {
        tmp = buf[j] - lspcb2[k1][j];
        dist += wegt[j] * tmp * tmp;
      }

      if(dist<dmin) {
         dmin = dist;
         *index = k1;
      }
   }
}

/*----------------------------------------------------------------------------
 * lsp_get_tdist - calculate the distortion
 *----------------------------------------------------------------------------
 */
static void lsp_get_tdist(
 FLOAT  wegt[],         /*input : weight coef.          */
 FLOAT  buf[],          /*input : candidate LSP vector  */
 FLOAT  *tdist,         /*output: distortion            */
 FLOAT  rbuf[],         /*input : target vector         */
 const FLOAT  fg_sum[]        /*input : present MA prediction coef.  */
)
{
   int  j;
   FLOAT        tmp;

   *tdist = (F)0.0;
   for(j=0; j<M; j++) {
      tmp = (buf[j] - rbuf[j]) * fg_sum[j];
      *tdist += wegt[j] * tmp * tmp;
   }
}

/*----------------------------------------------------------------------------
 * lsp_last_select - select the mode
 *----------------------------------------------------------------------------
 */
static void lsp_last_select(
 FLOAT  tdist[],        /*input : distortion         */
 int    *mode_index     /*output: the selected mode  */
)
{
   *mode_index = 0;
   if( tdist[1] < tdist[0] ) *mode_index = 1;
}
/*----------------------------------------------------------------------------
 * get_wegt - compute lsp weights
 *----------------------------------------------------------------------------
 */
static void get_wegt(
 FLOAT  flsp[],         /* input : M LSP parameters */
 FLOAT  wegt[]          /* output: M weighting coefficients */
)
{
   int  i;
   FLOAT        tmp;

   tmp = flsp[1] - PI04 - (F)1.0;
   if (tmp > (F)0.0)       wegt[0] = (F)1.0;
   else         wegt[0] = tmp * tmp * (F)10. + (F)1.0;

   for ( i=1; i<M-1; i++ ) {
      tmp = flsp[i+1] - flsp[i-1] - (F)1.0;
      if (tmp > (F)0.0)    wegt[i] = (F)1.0;
      else              wegt[i] = tmp * tmp * (F)10. + (F)1.0;
   }

   tmp = PI92 - flsp[M-2] - (F)1.0;
   if (tmp > (F)0.0)       wegt[M-1] = (F)1.0;
   else         wegt[M-1] = tmp * tmp * (F)10. + (F)1.0;

   wegt[4] *= CONST12;
   wegt[5] *= CONST12;
}

