#include <stdio.h>
#include "cli.h"

void
cli_local(char *cmdline)
{
	printf("\n");
	printf("local host   : [%s]\n", ua.local_endpoint.host);
	printf("local port   : %d\n", ua.local_endpoint.port);
	printf("visible host : [%s]\n", ua.visible_endpoint.host);
	printf("visible port : %d\n", ua.visible_endpoint.port);
	printf("\n");
}
