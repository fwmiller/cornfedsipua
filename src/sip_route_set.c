#include <stdlib.h>
#include <string.h>
#include "sip.h"

void
sip_route_set_init(sip_route_set_t route_set)
{
	if (route_set == NULL)
		return;

	memset(route_set, 0, sizeof(struct sip_route_set));
}

void
sip_route_set_append(char *route, sip_route_set_t route_set)
{
	int i, len, pos;

	if (route == NULL || route_set == NULL)
		return;

	len = strlen(route);
	for (pos = 0, i = 0; i < route_set->routes; i++)
		pos += route_set->len[i];

	strcpy(route_set->buf + pos, route);
	route_set->pos[route_set->routes] = pos;
	route_set->len[route_set->routes] = len;
	route_set->routes++;
}

int
sip_route_set_first(sip_route_set_t route_set, char *route)
{
	if (route_set == NULL || route == NULL || route_set->routes == 0)
		return (-1);

	strncpy(route, route_set->buf, route_set->len[0]);
	route_set->current_route = 1;
	return 0;
}

int
sip_route_set_next(sip_route_set_t route_set, char *route)
{
	int current_route;

	if (route_set == NULL || route == NULL ||
	    route_set->current_route >= route_set->routes)
		return (-1);

	current_route = route_set->current_route;
	strncpy(route, route_set->buf + route_set->pos[current_route],
		route_set->len[current_route]);
	route_set->current_route++;
	return 0;
}
