/********************************************************************
 * G.729 A/C codec interface and line packing
 *
 * (C) 2003-2005, Alex Volkov <codepro@usa.net>
 * This code should have come with LICENSE file, please read it.
 * 
 ********************************************************************/

#ifndef _G729AB_H_INCL_
#define _G729AB_H_INCL_

#include "typedef.h"
#include "cst_ld8a.h"
#include "g729frm.h"

struct cod_state
{
	struct cod_state_t cod_s;
	struct preproc_state_t preproc_s;

	int frame;                  /* frame counter */
};

struct dec_state
{
	struct dec_state_t dec_s;
	struct preproc_state_t postproc_s;
	struct postfilt_state_t postfilt_s;

	FLOAT  synth_buf[M+L_FRAME];        /* Synthesis  */
	FLOAT  *synth;
};


void g729_init_coder(struct cod_state *);
int  g729_coder(struct cod_state *, INT16 *DataBuff, char *Vout, int* poutlen);

void g729_init_decoder(struct dec_state *);
int  g729_decoder(struct dec_state *, INT16 *DataBuff, char *Vinp, int inplen);

void g729_line_pack(int* prm, unsigned char *Vout, int* poutlen);
void g729_line_unpack(int* prm, unsigned char *Vinp, int Ftyp);

#endif /* _G729AB_H_INCL_ */
