/*
 * Implementation of standard PTYPEs
 */

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <faux/str.h>
#include <faux/list.h>
#include <faux/conv.h>
#include <faux/argv.h>
#include <klish/kcontext.h>
#include <klish/kentry.h>


/** @brief PTYPE: Signed int with optional range
 *
 * Use long long int for conversion from text.
 *
 * <ACTION sym="INT">-30 80</ACTION>
 * Means range from "-30" to "80"
 */
int klish_ptype_INT(kcontext_t *context)
{
	const char *script = NULL;
	const char *value_str = NULL;
	long long int value = 0;

	script = kcontext_script(context);
	value_str = kcontext_candidate_value(context);

	if (!faux_conv_atoll(value_str, &value, 0))
		return -1;

	// Range is specified
	if (!faux_str_is_empty(script)) {
		faux_argv_t *argv = faux_argv_new();
		const char *str = NULL;
		faux_argv_parse(argv, script);

		// Min
		str = faux_argv_index(argv, 0);
		if (str) {
			long long int min = 0;
			if (!faux_conv_atoll(str, &min, 0) || (value < min)) {
				faux_argv_free(argv);
				return -1;
			}
		}

		// Max
		str = faux_argv_index(argv, 1);
		if (str) {
			long long int max = 0;
			if (!faux_conv_atoll(str, &max, 0) || (value > max)) {
				faux_argv_free(argv);
				return -1;
			}
		}

		faux_argv_free(argv);
	}

	return 0;
}


/** @brief PTYPE: Unsigned int with optional range
 *
 * Use unsigned long long int for conversion from text.
 *
 * <ACTION sym="UINT">30 80</ACTION>
 * Means range from "30" to "80"
 */
int klish_ptype_UINT(kcontext_t *context)
{
	const char *script = NULL;
	const char *value_str = NULL;
	unsigned long long int value = 0;

	script = kcontext_script(context);
	value_str = kcontext_candidate_value(context);

	if (!faux_conv_atoull(value_str, &value, 0))
		return -1;

	// Range is specified
	if (!faux_str_is_empty(script)) {
		faux_argv_t *argv = faux_argv_new();
		const char *str = NULL;
		faux_argv_parse(argv, script);

		// Min
		str = faux_argv_index(argv, 0);
		if (str) {
			unsigned long long int min = 0;
			if (!faux_conv_atoull(str, &min, 0) || (value < min)) {
				faux_argv_free(argv);
				return -1;
			}
		}

		// Max
		str = faux_argv_index(argv, 1);
		if (str) {
			unsigned long long int max = 0;
			if (!faux_conv_atoull(str, &max, 0) || (value > max)) {
				faux_argv_free(argv);
				return -1;
			}
		}

		faux_argv_free(argv);
	}

	return 0;
}


/** @brief PTYPE: Arbitrary string
 */
int klish_ptype_STRING(kcontext_t *context)
{
	// Really any string is a ... (surprise!) string

	context = context; // Happy compiler

	return 0;
}
