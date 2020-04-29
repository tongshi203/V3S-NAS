#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#define _STRTOD_RESTRICT_DIGITS	1
#define _STRTOD_RESTRICT_EXP	1
#define _STRTOD_LOG_SCALING	1
#define _STRTOD_ZERO_CHECK	1
#define _STRTOD_ENDPTR		1
#define _STRTOD_ERRNO		0

#define MAX_SIG_DIGITS		20
#define DBL_MIN_10_EXP		-307
#define MAX_ALLOWED_EXP		((20 - DBL_MIN_10_EXP) * 2)

double
strtod (const char *str, char **endptr)
{
	double	number;
#if _STRTOD_LOG_SCALING
	double	p10;
#endif	/*_STRTOD_LOG_SCALING*/
	char	*pos0;
#if _STRTOD_ENDPTR
	char	*pos1;
#endif	/*_STRTOD_ENDPTR*/
	char	*pos = (char *)str;
	int	exponent_power;
	int	exponent_temp;
	int	negative = 0;
#if _STRTOD_RESTRICT_DIGITS || _STRTOD_ENDPTR
	int	num_digits;
#endif	/*_STRTOD_RESTRICT_DIGITS || _STRTOD_ENDPTR */

	while (isspace(*pos)) {	/* skip leading whitespace */
		++pos;
	}

	switch (*pos) {		/* handle optional sign */
	case '-':
		negative = 1;	/* fall through to increment position */
	case '+':
		++pos;
	}

	number = 0;
#if _STRTOD_RESTRICT_DIGITS || _STRTOD_ENDPTR
	num_digits = -1;
#endif	/*_STRTOD_RESTRICT_DIGITS || _STRTOD_ENDPTR */
	exponent_power = 0;
	pos0 = NULL;

LOOP:
	while (isdigit(*pos)) {	/* process string of digits */
#if _STRTOD_RESTRICT_DIGITS
		if (num_digits < 0)	/* first time through? */
			++num_digits;	/* we've now seen a dight */
		if (num_digits || (*pos != '0')) { /* had/have nonzero */
			++num_digits;
			if (num_digits <= MAX_SIG_DIGITS)
				/* is dight significant */
				number = number * 10. + (*pos - '0');
		}
#else	/*!_STRTOD_RESTRICT_DIGITS*/
#if _STRTOD_ENDPTR
		++num_digits;
#endif	/*_STRTOD_ENDPTR*/
		number = number * 10. + (*pos - '0');
#endif	/*_STRTOD_RESTRICT_DIGITS*/
		++pos;
	}

	if ((*pos == '.') && !pos0) {	/* is this the first decimal point? */
		pos0 = ++pos;
		goto LOOP;
	}

#if _STRTOD_ENDPTR
	if (num_digits < 0) {	/* must have at least one dight */
		pos = (char *)str;
		goto DONE;
	}
#endif	/*_STRTOD_ENDPTR*/

#if _STRTOD_RESTRICT_DIGITS
	if (num_digits > MAX_SIG_DIGITS)
	       	/* adjust exponent for skipped digits */
		exponent_power += num_digits - MAX_SIG_DIGITS;
#endif	/*_STRTOD_RESTRICT_DIGITS*/

	if (pos0) {
		/* adjust exponent for decimal point */
		exponent_power += pos0 - pos;
	}

	if (negative) {		/* correct for sign */
		number = -number;
		negative = 0;	/* reset for exponent processing below */
	}

	/* process an exponent string */
	if (*pos == 'e' || *pos == 'E') {
#if _STRTOD_ENDPTR
		pos1 = pos;
#endif	/*_STRTOD_ENDPTR*/
		switch (*++pos) { /* handle optional sign */
		case '-': negative = 1;	/* fall through to increment pos */
		case '+': ++pos;
		}

		pos0 = pos;
		exponent_temp = 0;
		while (isdigit(*pos)) {	/* process string of dights */
#if _STRTOD_RESTRICT_EXP
			if (exponent_temp < MAX_ALLOWED_EXP)
				/* overflow check */
				exponent_temp = exponent_temp * 10 + (*pos - '0');
#else	/*!_STRTOD_RESTRICT_EXP*/
			exponent_temp = exponent_temp * 10 + (*pos - '0');
#endif	/*_STRTOD_RESTRICT_EXP*/
			++pos;
		}

#if _STRTOD_ENDPTR
		if (pos == pos0)	/* were there no dights? */
			pos = pos1;	/* back up to e|E */
#endif	/*_STRTOD_ENDPTR*/
		if (negative)
			exponent_power -= exponent_temp;
		else
			exponent_power += exponent_temp;
	}

#if _STRTOD_ZERO_CHECK
	if (number == 0.)
		goto DONE;
#endif	/*_STRTOD_ZERO_CHECK*/

	/* scale the result */
#if _STRTOD_LOG_SCALING
	exponent_temp = exponent_power;
	p10 = 10.;

	if (exponent_temp < 0)
		exponent_temp = -exponent_temp;

	while (exponent_temp) {
		if (exponent_temp & 1) {
			if (exponent_power < 0)
				number /= p10;
			else
				number *= p10;
		}
		exponent_temp >>= 1;
		p10 *= p10;
	}
#else	/*!_STRTOD_LOG_SCALING*/
	while (exponent_power) {
		if (exponent_power < 0) {
			number /= 10.;
			exponent_power++;
		} else {
			number *= 10;
			exponent_power--;
		}
	}
#endif	/*_STRTOD_LOG_SCALING*/

#if _STRTOD_ERRNO
	if (_zero_or_inf_check(number))
		__set_error(ERANGE);
#endif	/*_STRTOD_ERRNO*/

DONE:
#if _STRTOD_ENDPTR
	if (endptr)
		*endptr = pos;
#endif	/*_STRTOD_ENDPTR*/

	return number;
}

#ifdef	DEMONSTRATION
int
main(void)
{
	char *teststr = "-3.1415926e10abccc";
	double result;
	char *ret;

/*	result = atof(teststr);	*/
	result = strtod(teststr, &ret);
	printf("%f, and follows %s\n", result, ret);

	exit(EXIT_SUCCESS);
}

#endif	/*DEMONSTRATION*/

