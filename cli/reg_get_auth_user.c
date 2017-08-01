#include <string.h>
#include "cli.h"

void
reg_get_auth_user(char *host, char *auth_user)
{
	strcpy(auth_user, ua.reg_uri.user);
}
