#include "cli.h"
#include "log.h"

void
uas_hangup(sip_dialog_t dialog)
{
	log_msg(LOG_CONNECTION, "UAC Disconnecting Call-ID [%s]",
		dialog->call_id);

	sip_dialog_init(dialog);
	wav_rec_list_flush(&ua);
	rtp_endpoint_init(&(ua.rtp.remote));
	ua.rtp.codec = (-1);

	soundcard_flush(&(ua.soundcard));

}
