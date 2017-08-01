#-----------------------------------------------------------------------
# Makefile for compiler the decoder of the 
# G.729 Annex C Floating point implementation of 
# the 8 kbit/s G.729 Annex A algorithm
# This makefile was prepared for Unix systems
#-----------------------------------------------------------------------

# Define compiler options
#CFLAGS=-std # For Digital cc compiler

# Targets
OBJETS = \
 decoder.o dec_ld8a.o lpcfunca.o postfila.o tab_ld8a.o\
 bits.o de_acelp.o dec_gain.o dec_lag3.o filter.o\
 gainpred.o lspdec.o lspgetq.o p_parity.o post_pro.o\
 pred_lt3.o util.o

# Generation of the executable
decoder : $(OBJETS)
	$(CC) -o decoder $(OBJETS) -lm

# Compilations if necessary
decoder.o : decoder.c typedef.h ld8a.h
	$(CC) $(CFLAGS) -c decoder.c

dec_ld8a.o : dec_ld8a.c typedef.h ld8a.h
	$(CC) $(CFLAGS) -c dec_ld8a.c

lpcfunca.o : lpcfunca.c typedef.h ld8a.h tab_ld8a.h
	$(CC) $(CFLAGS) -c lpcfunca.c

postfila.o : postfila.c typedef.h ld8a.h
	$(CC) $(CFLAGS) -c postfila.c

tab_ld8a.o : tab_ld8a.c typedef.h ld8a.h
	$(CC) $(CFLAGS) -c tab_ld8a.c

bits.o : bits.c typedef.h ld8a.h\
    tab_ld8a.h 
	$(CC) $(CFLAGS) -c bits.c

de_acelp.o : de_acelp.c typedef.h ld8a.h
	$(CC) $(CFLAGS) -c de_acelp.c

dec_gain.o : dec_gain.c typedef.h ld8a.h\
    tab_ld8a.h 
	$(CC) $(CFLAGS) -c dec_gain.c

dec_lag3.o : dec_lag3.c typedef.h ld8a.h
	$(CC) $(CFLAGS) -c dec_lag3.c

filter.o : filter.c typedef.h ld8a.h
	$(CC) $(CFLAGS) -c filter.c

gainpred.o : gainpred.c typedef.h ld8a.h\
    tab_ld8a.h 
	$(CC) $(CFLAGS) -c gainpred.c

lspdec.o : lspdec.c typedef.h ld8a.h\
    tab_ld8a.h 
	$(CC) $(CFLAGS) -c lspdec.c

lspgetq.o : lspgetq.c typedef.h ld8a.h
	$(CC) $(CFLAGS) -c lspgetq.c

p_parity.o : p_parity.c typedef.h ld8a.h
	$(CC) $(CFLAGS) -c p_parity.c

post_pro.o : post_pro.c typedef.h ld8a.h\
    tab_ld8a.h 
	$(CC) $(CFLAGS) -c post_pro.c

pred_lt3.o : pred_lt3.c typedef.h ld8a.h\
    tab_ld8a.h 
	$(CC) $(CFLAGS) -c pred_lt3.c

util.o : util.c typedef.h ld8a.h
	$(CC) $(CFLAGS) -c util.c
