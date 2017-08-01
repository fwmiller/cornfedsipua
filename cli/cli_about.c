#include <stdio.h>
#include "cornfedsipua.h"

void
about()
{
	printf("\nCornfed SIP User Agent\n");
	printf("Version %d.%d.%d\n", CORNFEDSIPUA_VERSION, MAJOR_RELEASE,
	       MINOR_RELEASE);
	printf("Copyright (C) 2004-2008 Cornfed Systems LLC\n");
	printf("Written by Frank W. Miller\n\n");
}

void
cli_about(char *cmdline)
{
	about();
}
