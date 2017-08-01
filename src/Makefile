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

OBJS = config.o dns.o dtmf.o http.o ipendpoint.o lws2sws.o md5.o msglines.o net.o nextarg.o ringtone.o rtp.o rtp_lock.o rtp_stats.o sdp.o sip.o sip_dialog.o sip_gen.o sip_parse.o sip_recv.o sip_recv_request.o sip_recv_response.o sip_ringing.o sip_route_set.o sip_timers.o sip_uac_authenticate.o sip_uac.o sip_uac_cancel.o sip_uac_completed.o sip_uac_connect.o sip_uac_hangup.o sip_uac_invite.o sip_uac_proceeding.o sip_uac_register.o sip_uac_registered.o sip_uas_answer.o sip_uas.o sip_uas_cancel.o sip_uas_hangup.o sip_uas_invite.o sip_uas_options.o sip_uas_refuse.o sip_uas_response.o sip_uri.o sip_user_agent.o sip_via.o soundcard.o wav.o
TGT = libsip.a

all: $(OBJS)
	$(AR) $(TGT) $(OBJS)
	$(RANLIB) $(TGT)

clean:
	$(RM) $(TGT) $(OBJS)

.c.o: $<
	$(CC) $(CFLAGS) $(INCDIRS) -o $@ $<
