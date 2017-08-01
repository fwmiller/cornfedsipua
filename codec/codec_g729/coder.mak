#-----------------------------------------------------------------------
# Makefile for compiler the encoder of the 
# G.729 Annex C Floating point implementation of 
# the 8 kbit/s G.729 Annex A algorithm
# This makefile was prepared for Unix systems
#-----------------------------------------------------------------------

# Define compiler options
#CFLAGS=-std # For Digital cc compiler

# Targets
OBJETS = \
 coder.o cod_ld8a.o acelp_ca.o pitch_a.o lpca.o\
 lpcfunca.o tab_ld8a.o bits.o cor_func.o filter.o\
 gainpred.o lspgetq.o p_parity.o pre_proc.o pred_lt3.o\
 qua_gain.o qua_lsp.o taming.o util.o

# Generation of the executable
coder : $(OBJETS)
	$(CC) -o coder $(OBJETS) -lm

# Compilations if necessary

coder.o : coder.c typedef.h ld8a.h
	$(CC) $(CFLAGS) -c coder.c

cod_ld8a.o : cod_ld8a.c typedef.h ld8a.h
	$(CC) $(CFLAGS) -c cod_ld8a.c

acelp_ca.o : acelp_ca.c typedef.h ld8a.h
	$(CC) $(CFLAGS) -c acelp_ca.c

pitch_a.o : pitch_a.c typedef.h ld8a.h
	$(CC) $(CFLAGS) -c pitch_a.c

lpca.o : lpca.c typedef.h ld8a.h tab_ld8a.h
	$(CC) $(CFLAGS) -c lpca.c

lpcfunca.o : lpcfunca.c typedef.h ld8a.h tab_ld8a.h
	$(CC) $(CFLAGS) -c lpcfunca.c

tab_ld8a.o : tab_ld8a.c typedef.h ld8a.h
	$(CC) $(CFLAGS) -c tab_ld8a.c

bits.o : bits.c typedef.h ld8a.h\
    tab_ld8a.h
	$(CC) $(CFLAGS) -c bits.c

cor_func.o : cor_func.c typedef.h ld8a.h
	$(CC) $(CFLAGS) -c cor_func.c

filter.o : filter.c typedef.h ld8a.h
	$(CC) $(CFLAGS) -c filter.c

gainpred.o : gainpred.c typedef.h ld8a.h\
    tab_ld8a.h
	$(CC) $(CFLAGS) -c gainpred.c

lspgetq.o : lspgetq.c typedef.h ld8a.h
	$(CC) $(CFLAGS) -c lspgetq.c

p_parity.o : p_parity.c typedef.h ld8a.h
	$(CC) $(CFLAGS) -c p_parity.c

pre_proc.o : pre_proc.c typedef.h ld8a.h\
    tab_ld8a.h 
	$(CC) $(CFLAGS) -c pre_proc.c

pred_lt3.o : pred_lt3.c typedef.h ld8a.h\
    tab_ld8a.h 
	$(CC) $(CFLAGS) -c pred_lt3.c

qua_gain.o : qua_gain.c typedef.h ld8a.h\
    tab_ld8a.h 
	$(CC) $(CFLAGS) -c qua_gain.c

qua_lsp.o : qua_lsp.c typedef.h ld8a.h\
    tab_ld8a.h 
	$(CC) $(CFLAGS) -c qua_lsp.c

taming.o : taming.c typedef.h ld8a.h
	$(CC) $(CFLAGS) -c taming.c

util.o : util.c typedef.h ld8a.h
	$(CC) $(CFLAGS) -c util.c

