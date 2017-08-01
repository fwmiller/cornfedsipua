include ../MakeConfig

INCDIRS = -I../include

LIBDIRS = -L../hist -L../src -L../stun -L../codec
ifeq ($(CODEC_G729),-D_CODEC_G729)
LIBDIRS += -L../codec/codec_g729
endif

CC = gcc
STRIP = strip

CFLAGS = -c -Wall
ifeq ($(CODEC_G729),-D_CODEC_G729)
CFLAGS += $(CODEC_G729)
endif
ifeq ($(DEBUG),-D_DEBUG)
CFLAGS += -g -pg
endif
CFLAGS += `pkg-config libgnome-2.0 libgnomeui-2.0 gthread-2.0 --cflags`

OBJS = callbacks.o callbacks_media.o callbacks_outbound_proxy.o callbacks_provider.o callbacks_ringtone.o callbacks_stun_server.o gui.o gui_dialog.o gui_message.o gui_settings.o log.o main.o reg_get_auth_user.o reg_get_expires.o reg_get_interval.o reg_set_expires.o reg_thread_func.o status.o uac_canceled.o uac_completed.o uac_connect.o uac_register_failed.o uac_register_prompt_for_user.o uac_timeout.o ua_get_rtt.o ua_history_clear.o ua_history_update.o uas_cancel.o uas_completed.o uas_connect.o ua_set_rtt.o uas_hangup.o uas_request_uri_lookup.o uas_ringback.o
LIBS = -lhistory -lsip -lcodec -lstun -lpthread -lresolv -lexpat
ifeq ($(CODEC_G729),-D_CODEC_G729)
LIBS += -lg729a -lm
endif
LIBS += `pkg-config libgnome-2.0 libgnomeui-2.0 gthread-2.0 --libs`
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

wc: clean
	wc -l *.h *.c

.c.o: $<
	$(CC) $(CFLAGS) $(INCDIRS) -o $@ $<
