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


typedef struct {
	bool_t is_range;
	long long int min;
	long long int max;
} klish_ptype_INT_t;

typedef struct {
	bool_t is_range;
	unsigned long long int min;
	unsigned long long int max;
} klish_ptype_UINT_t;


klish_ptype_INT_t *klish_ptype_INT_init(kaction_t *action)
{
	klish_ptype_INT_t *udata = NULL;
	const char *line = NULL;
	faux_argv_t *argv = NULL;

	udata = faux_malloc(sizeof(*udata));
	assert(udata);
	if (!udata)
		return NULL;

	line = kaction_script(action);

	if (faux_str_is_empty(line)) {
		udata->is_range = BOOL_FALSE;

	} else {
		const char *str = NULL;

		udata->is_range = BOOL_TRUE;

		argv = faux_argv_new();
		faux_argv_parse(argv, line);
		if (faux_argv_len(argv) < 2)
			goto err;

		// Min
		str = faux_argv_index(argv, 0);
		if (!faux_conv_atoll(str, &udata->min, 0))
			goto err;

		// Max
		str = faux_argv_index(argv, 1);
		if (!faux_conv_atoll(str, &udata->max, 0))
			goto err;

		faux_argv_free(argv);
	}

	kaction_set_udata(action, udata, faux_free);

	return udata;

err:
	faux_argv_free(argv);
	faux_free(udata);

	return NULL;
}


/** @brief PTYPE: Signed int with optional range
 *
 * Use long long int for conversion from text.
 *
 * <ACTION sym="INT">-30 80</ACTION>
 * Means range from "-30" to "80"
 */
int klish_ptype_INT(kcontext_t *context)
{
	kaction_t *action = NULL;
	klish_ptype_INT_t *udata = NULL;
	const char *value_str = NULL;
	long long int value = 0;

	value_str = kcontext_candidate_value(context);

	if (!faux_conv_atoll(value_str, &value, 0))
		return -1;

	action = kcontext_action(context);
	udata = (klish_ptype_INT_t *)kaction_udata(action);
	if (!udata) {
		udata = klish_ptype_INT_init(action);
		if (!udata)
			return -1;
	}

	if (!udata->is_range)
		return 0;

	if (value < udata->min || value > udata->max)
		return -1;

	return 0;
}


klish_ptype_UINT_t *klish_ptype_UINT_init(kaction_t *action)
{
	klish_ptype_UINT_t *udata = NULL;
	const char *line = NULL;
	faux_argv_t *argv = NULL;

	udata = faux_malloc(sizeof(*udata));
	assert(udata);
	if (!udata)
		return NULL;

	line = kaction_script(action);

	if (faux_str_is_empty(line)) {
		udata->is_range = BOOL_FALSE;

	} else {
		const char *str = NULL;

		udata->is_range = BOOL_TRUE;

		argv = faux_argv_new();
		faux_argv_parse(argv, line);
		if (faux_argv_len(argv) < 2)
			goto err;

		// Min
		str = faux_argv_index(argv, 0);
		if (!faux_conv_atoull(str, &udata->min, 0))
			goto err;

		// Max
		str = faux_argv_index(argv, 1);
		if (!faux_conv_atoull(str, &udata->max, 0))
			goto err;

		faux_argv_free(argv);
	}

	kaction_set_udata(action, udata, faux_free);

	return udata;

err:
	faux_argv_free(argv);
	faux_free(udata);

	return NULL;
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
	kaction_t *action = NULL;
	klish_ptype_UINT_t *udata = NULL;
	const char *value_str = NULL;
	unsigned long long int value = 0;

	value_str = kcontext_candidate_value(context);

	if (!faux_conv_atoull(value_str, &value, 0))
		return -1;

	action = kcontext_action(context);
	udata = (klish_ptype_UINT_t *)kaction_udata(action);
	if (!udata) {
		udata = klish_ptype_UINT_init(action);
		if (!udata)
			return -1;
	}

	if (!udata->is_range)
		return 0;

	if (value < udata->min || value > udata->max)
		return -1;

	return 0;
}
