/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char *rcsid = "$OpenBSD: htons.c,v 1.5 2004/11/28 07:23:41 mickey Exp $";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
//#include <machine/endian.h>

#undef htons

u16_t
htons(u16_t x)
{
#if BYTE_ORDER == LITTLE_ENDIAN
	u8_t *s = (u8_t *) &x;
	return (u16_t)(s[0] << 8 | s[1]);
#else
	return x;
#endif
}
