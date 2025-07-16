/*
 * Implementation of standard PTYPEs
 */

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <syslog.h>

#include <faux/str.h>
#include <faux/list.h>
#include <faux/conv.h>
#include <faux/argv.h>
#include <klish/kcontext.h>
#include <klish/kentry.h>


typedef struct {
	const char *cmd;
	size_t len;
	size_t min_len;
} klish_ptype_COMMAND_t;


klish_ptype_COMMAND_t *klish_ptype_COMMAND_init(kentry_t *entry)
{
	klish_ptype_COMMAND_t *udata = NULL;
	const char *cmd = NULL;
	size_t len = 0;
	size_t min_len = 0;
	char *space = NULL;

	cmd = kentry_value(entry);
	if (cmd) {
		space = strchr(cmd, ' ');
		if (space) {
			unsigned char val = 0;
			if (!faux_conv_atouc((char *)(space + 1), &val, 10))
				return NULL;
			len = space - cmd;
			min_len = (val < len) ? val : len;
		} else {
			len = strlen(cmd);
			min_len = len;
		}
	} else {
		cmd = kentry_name(entry);
		if (!cmd)
			return NULL;
		len = strlen(cmd);
		min_len = len;
	}

	udata = faux_malloc(sizeof(*udata));
	assert(udata);
	if (!udata)
		return NULL;

	udata->cmd = cmd;
	udata->len = len;
	udata->min_len = min_len;

	kentry_set_udata(entry, udata, (kentry_udata_free_fn)faux_free);

	return udata;
}


/** @brief PTYPE: Consider ENTRY's name (or "value" field) as a command
 */
int klish_ptype_COMMAND(kcontext_t *context)
{
	kentry_t *entry = NULL;
	const char *value = NULL;
	klish_ptype_COMMAND_t *udata = NULL;
	size_t len = 0;

	entry = kcontext_candidate_entry(context);
	value = kcontext_candidate_value(context);

	udata = (klish_ptype_COMMAND_t *)kentry_udata(entry);
	if (!udata) {
		udata = klish_ptype_COMMAND_init(entry);
		if (!udata)
			return -1;
	}

	len = strlen(value);
	if (len < udata->min_len || len > udata->len)
		return -1;

	if (faux_str_casecmpn(value, udata->cmd, len) != 0)
		return -1;

	kcontext_fwrite(context, stdout, udata->cmd, udata->len);

	return 0;
}


/** @brief COMPLETION: Consider ENTRY's name (or "value" field) as a command
 *
 * This completion function has main ENTRY that is a child of COMMAND ptype
 * ENTRY. The PTYPE entry has specific ENTRY (with name and possible value)
 * as a parent.
 *
 * command (COMMON ENTRY) with name or value
 *     ptype (PTYPE ENTRY)
 *         completion (COMPLETION ENTRY) - start point
 */
int klish_completion_COMMAND(kcontext_t *context)
{
	kentry_t *entry = NULL;
	klish_ptype_COMMAND_t *udata = NULL;

	entry = kcontext_candidate_entry(context);
	udata = (klish_ptype_COMMAND_t *)kentry_udata(entry);
	if (!udata) {
		udata = klish_ptype_COMMAND_init(entry);
		if (!udata)
			return -1;
	}

	kcontext_fwrite(context, stdout, udata->cmd, udata->len);

	return 0;
}


/** @brief HELP: Consider ENTRY's name (or "value" field) as a command
 *
 * This help function has main ENTRY that is a child of COMMAND ptype
 * ENTRY. The PTYPE entry has specific ENTRY (with name and possible value)
 * as a parent.
 *
 * command (COMMON ENTRY) with name or value
 *     ptype (PTYPE ENTRY)
 *         help (HELP ENTRY) - start point
 */
int klish_help_COMMAND(kcontext_t *context)
{
	kentry_t *entry = NULL;
	const char *help_text = NULL;
	klish_ptype_COMMAND_t *udata = NULL;

	entry = kcontext_candidate_entry(context);
	udata = (klish_ptype_COMMAND_t *)kentry_udata(entry);
	if (!udata) {
		udata = klish_ptype_COMMAND_init(entry);
		if (!udata)
			return -1;
	}

	help_text = kentry_help(entry);
	if (!help_text)
		help_text = kentry_value(entry);
	if (!help_text)
		help_text = kentry_name(entry);
	assert(help_text);

	kcontext_fwrite(context, stdout, udata->cmd, udata->len);
	kcontext_printf(context, "\n%s\n", help_text);

	return 0;
}


/** @brief PTYPE: ENTRY's name (or "value" field) as a case sensitive command
 */
int klish_ptype_COMMAND_CASE(kcontext_t *context)
{
	const kentry_t *entry = NULL;
	const char *value = NULL;
	const char *command_name = NULL;

	entry = kcontext_candidate_entry(context);
	value = kcontext_candidate_value(context);

	command_name = kentry_value(entry);
	if (!command_name)
		command_name = kentry_name(entry);
	if (!command_name)
		return -1;

	return strcmp(value, command_name);
}
