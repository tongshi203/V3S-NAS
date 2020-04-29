#include <stdlib.h>

extern double strtod (const char *str, char **endptr);

double
atof(const char *nptr)
{
	return strtod(nptr, (char **)NULL);
}


