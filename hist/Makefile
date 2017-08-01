include ../MakeConfig

INCDIRS = -I../include

CC = gcc
AR = ar -r
RANLIB = ranlib

CFLAGS = -c -Wall
ifeq ($(CODEC_G729),-D_CODEC_G729)
CFLAGS += $(CODEC_G729)
endif
ifeq ($(DEBUG),-D_DEBUG)
CFLAGS += -g -pg
endif

OBJS = history.o
TGT = libhistory.a

all: $(OBJS)
	$(AR) $(TGT) $(OBJS)
	$(RANLIB) $(TGT)

clean:
	$(RM) $(TGT) $(OBJS)

.c.o: $<
	$(CC) $(CFLAGS) $(INCDIRS) -o $@ $<
