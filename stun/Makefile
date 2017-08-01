include ../MakeConfig

INCDIRS = -I../include

CC = gcc
AR = ar -r
RANLIB = ranlib

CFLAGS = -c -Wall
ifeq ($(DEBUG),-D_DEBUG)
CFLAGS += -g -pg
endif

OBJS = bufdump.o stun.o stun_transactions.o
TGT = libstun.a

all: $(OBJS)
	$(AR) $(TGT) $(OBJS)
	$(RANLIB) $(TGT)

clean:
	$(RM) $(TGT) $(OBJS)

.c.o: $<
	$(CC) $(CFLAGS) $(INCDIRS) -o $@ $<
