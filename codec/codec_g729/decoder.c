/*
   ITU-T G.729 Annex C - Reference C code for floating point
                         implementation of G.729 Annex A
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

/*-----------------------------------------------------------------*
 * Main program of the G.729a 8.0 kbit/s decoder.                  *
 *                                                                 *
 *    Usage : decoder  bitstream_file  synth_file                  *
 *                                                                 *
 *-----------------------------------------------------------------*/

#include "typedef.h"
#include "ld8a.h"
#include "cst_ld8a.h"

/*-----------------------------------------------------------------*
 *            Main decoder routine                                 *
 *-----------------------------------------------------------------*/

int main( int argc, char *argv[])
{
   static FLOAT  synth_buf[L_FRAME+M];     /* Synthesis                  */
   FLOAT  *synth;
   static int parm[PRM_SIZE+1];            /* Synthesis parameters + BFI */
   static INT16 serial[SERIAL_SIZE];       /* Serial stream              */
   static FLOAT  Az_dec[MP1*2];            /* Decoded Az for post-filter */
   static int T2[2];                       /* Decoded Pitch              */

   int frame;
   int i;
   FILE   *f_syn, *f_serial;

	/* New unstaticed states */
	struct preproc_state_t postproc_s;
	struct dec_state_t dec_s;
	struct postfilt_state_t postfilt_s;


   printf("\n");
   printf("**************    G.729A 8 KBIT/S SPEECH DECODER    ************\n");
   printf("\n");
   printf("----------------- Floating point C simulation ----------------\n");
   printf("\n");
   printf("-----------------          Version 1.01       ----------------\n");
   printf("\n");

   /* Passed arguments */

   if ( argc != 3)
     {
        printf("Usage :%s bitstream_file  outputspeech_file\n",argv[0]);
        printf("\n");
        printf("Format for bitstream_file:\n");
        printf("  One (2-byte) synchronization word \n");
        printf("  One (2-byte) size word,\n");
        printf("  80 words (2-byte) containing 80 bits.\n");
        printf("\n");
        printf("Format for outputspeech_file:\n");
        printf("  Synthesis is written to a binary file of 16 bits data.\n");
        exit( 1 );
     }

   /* Open file for synthesis and packed serial stream */

   if( (f_serial = fopen(argv[1],"rb") ) == NULL )
     {
        printf("%s - Error opening file  %s !!\n", argv[0], argv[1]);
        exit(0);
     }

   if( (f_syn = fopen(argv[2], "wb") ) == NULL )
     {
        printf("%s - Error opening file  %s !!\n", argv[0], argv[2]);
        exit(0);
     }

   printf("Input bitstream file  :   %s\n",argv[1]);
   printf("Synthesis speech file :   %s\n",argv[2]);

/*-----------------------------------------------------------------*
 *           Initialization of decoder                             *
 *-----------------------------------------------------------------*/

  for (i=0; i<M; i++) synth_buf[i] = (F)0.0;
  synth = synth_buf + M;

  init_decod_ld8a(&dec_s);
  init_post_filter(&postfilt_s);
  init_post_process(&postproc_s);

/*-----------------------------------------------------------------*
 *            Loop for each "L_FRAME" speech data                  *
 *-----------------------------------------------------------------*/

   frame =0;
   while( fread(serial, sizeof(INT16), SERIAL_SIZE, f_serial) == SERIAL_SIZE)
   {
      frame++;
      printf(" Frame: %d\r", frame);

      bits2prm_ld8k( &serial[2], &parm[1]);

      if (serial[0] == SYNC_WORD) {
         parm[0] = 0;           /* No frame erasure */
      } else {
         parm[0] = 1;           /* frame erased     */
      }

      parm[4] = check_parity_pitch(parm[3], parm[4] ); /* get parity check result */

      decod_ld8a(&dec_s, parm, synth, Az_dec, T2);             /* decoder */

      post_filter(&postfilt_s, synth, Az_dec, T2);                  /* Post-filter */

      post_process(&postproc_s, synth, L_FRAME);                    /* Highpass filter */

      fwrite16(synth, L_FRAME, f_syn);
   }

   return(0);
}
