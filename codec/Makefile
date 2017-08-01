include ../MakeConfig

INCDIRS = -I../include
ifeq ($(CODEC_G729),-D_CODEC_G729)
INCDIRS += -I./codec_g729
endif

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

OBJS = codec.o codec_alaw.o codec_ulaw.o
ifeq ($(CODEC_G729),-D_CODEC_G729)
OBJS += codec_g729.o
endif
TGT = libcodec.a

all: $(OBJS)
	$(AR) $(TGT) $(OBJS)
	$(RANLIB) $(TGT)

clean:
	$(RM) $(TGT) $(OBJS)

.c.o: $<
	$(CC) $(CFLAGS) $(INCDIRS) -o $@ $<
