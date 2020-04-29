/*
 * Reconfigurable error handler
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include "device.h"

/* output error message and return -1*/
int
GdError(const char *format, ...)
{
	va_list args;
	char 	buf[512];

	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);
	write(2, buf, strlen(buf));
	return -1;
}

/* null routine to consume messages */
int
GdErrorNull(const char *format, ...)
{
	return -1;
}

