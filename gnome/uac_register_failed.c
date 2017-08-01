#include "gui.h"

void
uac_register_failed(char *host)
{
	status("Registration failed");

	reg_set_expires(NULL, (-1));
}
