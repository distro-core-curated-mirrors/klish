/*
 *
 */

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/utsname.h>

#include <faux/conv.h>
#include <faux/str.h>
#include <faux/list.h>
#include <faux/sysdb.h>
#include <klish/kcontext.h>
#include <klish/ksession.h>
#include <klish/kpath.h>


int klish_nop(kcontext_t *context)
{
	context = context; // Happy compiler

	return 0;
}


// Symbol for testing purposes
int klish_tsym(kcontext_t *context)
{
	const char *script = NULL;

	script = kcontext_script(context);
	if (faux_str_is_empty(script)) {
		printf("[<empty>]\n");
		fprintf(stderr, "Empty item\n");
		return -1;
	}

	printf("[%s]\n", script);

	return 0;
}


// Print content of ACTION script
int klish_printl(kcontext_t *context)
{
	const char *script = NULL;

	script = kcontext_script(context);
	if (faux_str_is_empty(script))
		script = "";

	printf("%s\n", script);

	return 0;
}


// Print content of ACTION script. Without additional '/n'
int klish_print(kcontext_t *context)
{
	const char *script = NULL;

	script = kcontext_script(context);
	if (faux_str_is_empty(script))
		script = "";

	printf("%s", script);
	fflush(stdout);

	return 0;
}


// Symbol to show current path
int klish_pwd(kcontext_t *context)
{
	kpath_t *path = NULL;
	kpath_levels_node_t *iter = NULL;
	klevel_t *level = NULL;

	path = ksession_path(kcontext_session(context));
	iter = kpath_iter(path);
	while ((level = kpath_each(&iter))) {
		printf("/%s", kentry_name(klevel_entry(level)));
	}
	printf("\n");

	return 0;
}


// Template for easy prompt string generation
int klish_prompt(kcontext_t *context)
{
	const char *script = NULL;
	const char *start = NULL;
	const char *pos = NULL;
	char *prompt = NULL;
	bool_t is_macro = BOOL_FALSE;

	script = kcontext_script(context);
	if (faux_str_is_empty(script))
		return 0;
	pos = script;
	start = script;

	while (*pos != '\0') {

		if (is_macro) {
			switch (*pos) {
			// Percent symbol itself
			case '%': {
				faux_str_cat(&prompt, "%");
				break;
				}
			// Backslash symbol itself
			case '\\': {
				faux_str_cat(&prompt, "\\");
				break;
				}
			// Escape character
			case 'e': {
				faux_str_cat(&prompt, "\x1b");
				break;
				}
			// Hexadecimal byte
			case 'x': {
				char c;
				char hs[3] = {};
				if (*(pos + 1) == '\0')
					break;
				if (*(pos + 2) == '\0') {
					pos++;
					break;
				}
				hs[0] = *(pos + 1);
				hs[1] = *(pos + 2);
				pos += 2;
				if (!faux_conv_atouc(hs, &c, 16))
					break;
				faux_str_catn(&prompt, &c, 1);
				break;
				}
			// Hostname
			case 'h': {
				struct utsname buf;
				if (uname(&buf) == 0)
					faux_str_cat(&prompt, buf.nodename);
				break;
				}
			// Username
			case 'u': {
				const char *user = NULL;
				user = ksession_user(kcontext_session(context));
				if (user)
					faux_str_cat(&prompt, user);
				break;
				}
			}
			is_macro = BOOL_FALSE;
			start = pos + 1;

		} else if (*pos == '%' || *pos == '\\') {
			is_macro = BOOL_TRUE;
			if (pos > start)
				faux_str_catn(&prompt, start, pos - start);
		}
		pos++;
	}
	if (pos > start)
		faux_str_catn(&prompt, start, pos - start);

	kcontext_printf(context, "%s", prompt);
	faux_str_free(prompt);

	return 0;
}
