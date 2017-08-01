/*
**
** File:        "cst_ld8a.h"
**
** Description:  This file contains global definition of the
**    CS-ACELP Coder for 8 kbps.
**
*/

#ifndef _CST_LD8A_H_INCL_
#define _CST_LD8A_H_INCL_

#include "ld8a.h"

/*
   Used structures
*/
struct cod_state_t
{
	/* Speech vector */
	FLOAT old_speech[L_TOTAL];
	FLOAT *speech, *p_window;
	FLOAT *new_speech;                    /* Global variable */

	/* Weighted speech vector */
	FLOAT old_wsp[L_FRAME+PIT_MAX];
	FLOAT *wsp;

	/* Excitation vector */
	FLOAT old_exc[L_FRAME+PIT_MAX+L_INTERPOL];
	FLOAT *exc;

	/* LSP Line spectral frequencies */
	FLOAT lsp_old[M];
	FLOAT lsp_old_q[M];

	/* Filter's memory */
	FLOAT  mem_w0[M], mem_w[M], mem_zero[M];
	FLOAT  sharp;

	FLOAT exc_err[4]; /* taming state */

	struct lsp_cod_state_t lsp_s;
	//struct cod_cng_state_t cng_s;
	//struct vad_state_t vad_s;
	struct gain_state_t gain_s;
	//struct taming_state_t taming_s;
};

struct dec_state_t
{
	/* Excitation vector */
	FLOAT old_exc[L_FRAME+PIT_MAX+L_INTERPOL];
	FLOAT *exc;

	/* Lsp (Line spectral pairs) */
	FLOAT lsp_old[M];

	FLOAT mem_syn[M];       /* Synthesis filter's memory          */
	FLOAT sharp;            /* pitch sharpening of previous frame */
	int old_t0;             /* integer delay of previous frame    */
	FLOAT gain_code;        /* Code gain                          */
	FLOAT gain_pitch;       /* Pitch gain                         */

	int bad_lsf;
		/*
		   This variable should be always set to zero unless transmission errors
		   in LSP indices are detected.
		   This variable is useful if the channel coding designer decides to
		   perform error checking on these important parameters. If an error is
		   detected on the  LSP indices, the corresponding flag is
		   set to 1 signalling to the decoder to perform parameter substitution.
		   (The flags should be set back to 0 for correct transmission).
		*/

	struct lsp_dec_state_t lsp_s;
	//struct dec_cng_state_t cng_s;
	struct gain_state_t gain_s;
};

#endif /* _CST_LD8A_H_INCL_ */
