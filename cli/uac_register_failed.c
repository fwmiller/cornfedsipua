#include "cli.h"

void
uac_register_failed(char *host)
{
	reg_set_expires(NULL, -1);
}
