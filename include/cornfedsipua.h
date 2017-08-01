#ifndef __CORNFEDSIPUA_H
#define __CORNFEDSIPUA_H

#define CORNFEDSIPUA_VERSION	1
#define MAJOR_RELEASE		1
#define MINOR_RELEASE		7

#define BUFSIZE			2048

#define SIP_DEFAULT_PORT	5060
#define RTP_DEFAULT_PORT	5004

#define CORNFED_DIR		"/.cornfed"
#define CONFIG_FILE		"/config"
#define HISTORY_FILE		"/history"
#define SOUNDCARD_DEVICE	"/dev/dsp"
#define RINGTONE_FILE		"/usr/share/cornfed/ring.wav"
#define RINGTONE_DEVICE		"/dev/dsp"
#define IF_NAME			"eth0"
#define IF_NAME_LOCAL		"lo"
#define STUN_SERVER		"stun.counterpath.com"

#define REGISTER_INTERVAL	300		/* 5 minutes */
#define REGISTER_BUFFER		20		/* seconds */

/* Command line client prompt */
#define PROMPT			"sip> "

/* Gnome GUI client configuration */
#define ICON_SMALL		"/usr/share/pixmaps/sip_logo_32.png"
#define ICON_LARGE		"/usr/share/pixmaps/sip_logo_48.png"
#define WINDOW_WIDTH		256
#define WINDOW_HEIGHT		336

typedef unsigned char u8_t;
typedef unsigned short u16_t;
typedef unsigned long u32_t;

#endif
