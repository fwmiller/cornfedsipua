INCDIRS = -I../include

LIBDIRS = -L../sub -L../hist -L../src -L../stun -L../codec
ifeq ($(CODEC_G729),-D_CODEC_G729)
LIBDIRS += -L../codec/codec_g729
endif

CC = gcc
STRIP = strip

CFLAGS = -c -Wall
ifeq ($(DEBUG),-D_DEBUG)
CFLAGS += -g -pg
endif
ifeq ($(CODEC_G729),-D_CODEC_G729)
CFLAGS += $(CODEC_G729)
endif

OBJS = cli_about.o cli_answer.o cli.o cli_cmd_history.o cli_debug.o cli_dial.o cli_dialog.o cli_dnd.o cli_dns.o cli_dtmf.o cli_hangup.o cli_help.o cli_history.o cli_local.o cli_log.o cli_nat.o cli_outbound_proxy.o cli_play_wav.o cli_record_wav.o cli_refuse.o cli_register.o cli_remote.o cli_reset.o cli_ringtone.o cli_rtp.o cli_soundcard.o cli_wav.o cli_volume.o log.o main.o readline.o reg_get_auth_user.o reg_get_expires.o reg_get_interval.o reg_set_expires.o reg_thread_func.o ua_get_rtt.o ua_set_rtt.o ua_history_clear.o ua_history_update.o uac_canceled.o uac_completed.o uac_connect.o uac_register_failed.o uac_register_prompt_for_user.o uac_timeout.o uas_cancel.o uas_completed.o uas_connect.o uas_hangup.o uas_request_uri_lookup.o uas_ringback.o
LIBS = -lhistory -lsip -lcodec -lstun -lpthread -lresolv -lexpat
ifeq ($(CODEC_G729),-D_CODEC_G729)
LIBS += -lg729a -lm
endif
TGT = sip

all: $(OBJS)
ifeq ($(DEBUG),-D_DEBUG)
	$(CC) $(LIBDIRS) -g -pg -o $(TGT) $(OBJS) $(LIBS)
else
	$(CC) $(LIBDIRS) -o $(TGT) $(OBJS) $(LIBS)
	$(STRIP) $(TGT)
endif

clean:
	$(RM) gmon.out $(TGT) $(OBJS)

.c.o: $<
	$(CC) $(CFLAGS) $(INCDIRS) -o $@ $<
