/*
 * Implementation of STRING PTYPE.
 *
 * It gets any string by default. The regular expression can be defined within
 * PTYPE body. In this case input string will be validated using this regexp.
 */

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <regex.h>

#include <faux/str.h>
#include <faux/list.h>
#include <faux/conv.h>
#include <faux/argv.h>
#include <klish/kcontext.h>
#include <klish/kentry.h>


typedef struct {
	bool_t is_regex;
	regex_t compiled_regex;
} klish_ptype_STRING_t;


static void klish_ptype_STRING_free(void *data)
{
	klish_ptype_STRING_t *udata = (klish_ptype_STRING_t *)data;

	if (udata->is_regex)
		regfree(&udata->compiled_regex);

	faux_free(udata);
}


klish_ptype_STRING_t *klish_ptype_STRING_init(kaction_t *action)
{
	klish_ptype_STRING_t *udata = NULL;
	const char *pattern = NULL;

	udata = faux_malloc(sizeof(*udata));
	assert(udata);
	if (!udata)
		return NULL;

	pattern = kaction_script(action);

	if (faux_str_is_empty(pattern)) {
		udata->is_regex = BOOL_FALSE;
	} else {
		udata->is_regex = BOOL_TRUE;
		if (regcomp(&udata->compiled_regex, pattern,
			REG_NOSUB | REG_EXTENDED))
		{
			faux_free(udata);
			return NULL;
		}
	}

	kaction_set_udata(action, udata, klish_ptype_STRING_free);

	return udata;
}


/** @brief PTYPE: String
 */
int klish_ptype_STRING(kcontext_t *context)
{
	kaction_t *action = NULL;
	const char *value = NULL;
	klish_ptype_STRING_t *udata = NULL;
	size_t len = 0;

	action = kcontext_action(context);
	value = kcontext_candidate_value(context);

	udata = (klish_ptype_STRING_t *)kaction_udata(action);
	if (!udata) {
		udata = klish_ptype_STRING_init(action);
		if (!udata)
			return -1;
	}

	if (!udata->is_regex)
		return 0;

	if (regexec(&udata->compiled_regex, value, 0, NULL, 0))
		return -1;

	return 0;
}
