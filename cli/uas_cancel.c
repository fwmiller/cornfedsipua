#include "cli.h"
#include "log.h"

void
uas_cancel(sip_dialog_t dialog)
{
	log_msg(LOG_CONNECTION, "Canceling Call-ID [%s]", dialog->call_id);

	sip_dialog_init(dialog);
	rtp_endpoint_init(&(ua.rtp.remote));
	ua.rtp.codec = (-1);

	soundcard_flush(&(ua.soundcard));
}
