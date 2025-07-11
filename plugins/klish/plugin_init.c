/*
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include <faux/faux.h>
#include <klish/kplugin.h>
#include <klish/kcontext.h>

#include "private.h"


const uint8_t kplugin_klish_major = KPLUGIN_MAJOR;
const uint8_t kplugin_klish_minor = KPLUGIN_MINOR;


int kplugin_klish_init(kcontext_t *context)
{
	kplugin_t *plugin = NULL;

	assert(context);
	plugin = kcontext_plugin(context);
	assert(plugin);

	// Misc
	kplugin_add_syms(plugin, ksym_new_fast("nop", klish_nop));
	kplugin_add_syms(plugin, ksym_new_fast("tsym", klish_tsym));
	kplugin_add_syms(plugin, ksym_new_fast("print", klish_print));
	kplugin_add_syms(plugin, ksym_new_fast("printl", klish_printl));
	kplugin_add_syms(plugin, ksym_new_fast("prompt", klish_prompt));

	// Log
	kplugin_add_syms(plugin, ksym_new_fast("syslog", klish_syslog));

	// Navigation
	// Navigation must be permanent (no dry-run) and sync. Because unsync
	// actions will be fork()-ed so it can't change current path.
	kplugin_add_syms(plugin, ksym_new_ext("nav", klish_nav,
		KSYM_PERMANENT, KSYM_SYNC, KSYM_SILENT));
	kplugin_add_syms(plugin, ksym_new_ext("pwd", klish_pwd,
		KSYM_PERMANENT, KSYM_SYNC, KSYM_SILENT));

	// PTYPEs
	// These PTYPEs are simple and fast so set SYNC flag
	kplugin_add_syms(plugin, ksym_new_fast("COMMAND", klish_ptype_COMMAND));
	kplugin_add_syms(plugin, ksym_new_fast("completion_COMMAND",
		klish_completion_COMMAND));
	kplugin_add_syms(plugin, ksym_new_fast("help_COMMAND",
		klish_help_COMMAND));
	kplugin_add_syms(plugin, ksym_new_fast("COMMAND_CASE",
		klish_ptype_COMMAND_CASE));
	kplugin_add_syms(plugin, ksym_new_fast("INT", klish_ptype_INT));
	kplugin_add_syms(plugin, ksym_new_fast("UINT", klish_ptype_UINT));
	kplugin_add_syms(plugin, ksym_new_fast("STRING", klish_ptype_STRING));

	return 0;
}


int kplugin_klish_fini(kcontext_t *context)
{
//	fprintf(stderr, "Plugin 'klish' fini\n");
	context = context;

	return 0;
}
