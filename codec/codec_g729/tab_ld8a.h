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

extern const FLOAT hamwindow[L_WINDOW];
extern const FLOAT lwindow[MP1];
extern const FLOAT lspcb1[NC0][M];
extern const FLOAT lspcb2[NC1][M];
extern const FLOAT fg[2][MA_NP][M];
extern const FLOAT fg_sum[2][M];
extern const FLOAT fg_sum_inv[2][M];
extern const FLOAT grid[GRID_POINTS+1];
extern const FLOAT inter_3l[FIR_SIZE_SYN];
extern const FLOAT pred[4];
extern const FLOAT gbk1[NCODE1][2];
extern const FLOAT gbk2[NCODE2][2];
extern const int map1[NCODE1];
extern const int map2[NCODE2];
extern const FLOAT coef[2][2];
extern const FLOAT thr1[NCODE1-NCAN1];
extern const FLOAT thr2[NCODE2-NCAN2];
extern const int imap1[NCODE1];
extern const int imap2[NCODE2];
extern const FLOAT b100[3];
extern const FLOAT a100[3];
extern const FLOAT b140[3];
extern const FLOAT a140[3];
extern const int  bitsno[PRM_SIZE];
extern const int bitsno2[4]; 

extern const FLOAT past_qua_en_reset[4];
extern const FLOAT lsp_reset[M];

const FLOAT freq_prev_reset[M];
