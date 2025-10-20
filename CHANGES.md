# The "klish" project changelog

## 3.2.0

* Don't search for completion/help in upper levels when line is empty.
* Don't show help for upper levels when single word is unfinished.
* Fixed klish pipe hanging when filter exits before main command.
* Log message when scheme is illegal or not defined.
* New ksym_new_fast() function added for plugins.
* Fixed memory leak when arg validation returns non-empty string.
* Fixed possible memory leak while parameters validation.
* The "transparent=true/false" attribute that determines whether the upper path level elements are available while command line parsing.
* The COND tag is implemented that allows to enable or disable parent elements.
* Fix ACTION's retval processing.
* doc: Fixed documentation.
* doc: Added English documentation.
* examples: Updated XML-config examples.
* plugin-script: New environment variable KLISH_LINE.
* plugin-script: Environment variable for the multi-value parameters.
* plugin-script: Fixed memory leak.
* plugin-klish: Prompt allows escape character '\e'.
* plugin-klish: The "prompt" symbol understands hex codes.
* plugin-klish: Speed up builtin ptype syms.
* plugin-klish: Support for the short commands with '|' delimeter syntax.
* plugin-klish: Update COMMAND_CASE PTYPE.
* plugin-klish: STRING PTYPE supports regular expressions.
* plugin-lua: Script can set a return code by "return <num>".

Thanks to Andrey Eremin <korbezolympus@gmail.com> and Peter Kosyh <p.kosyh@gmail.com> for the patches and bugfix.
