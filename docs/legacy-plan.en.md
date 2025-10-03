# Klish Project Refactoring

## Problems

* Poor navigation principle. Each VIEW/COMMAND must know its nesting level in advance. I.e., static nesting level.
* Inability for third-party programs to get command execution status. Hinders automation of management from external graphical utilities or remotely over the network.
* No batch command execution mode.
* XML configuration lacks sufficient flexibility (macros, substitutions), leading to a lot of copy-paste in configuration files.
* Inconvenient type definition (PTYPE). Unreadable regular expressions when defining PTYPE. Code duplication within regular expressions. No ability to execute arbitrary code to validate entered values.
* Inability to press Enter when an incorrect command or invalid parameter is entered. Practice has shown this is not always convenient.
* To inherit commands from a parent VIEW, inheritance must be explicitly specified.
* Dynamic VARs lack arguments. Many duplicated variables with very similar, but not identical, behavior.
* Strong coupling between program modules. Excessive code complexity. Some class confusion.
* Poorly documented code and external function interfaces.
* In some cases, poor error handling leads to crashes.
* No localization capability.

## Requirements

### Fast Command Search

Currently, a splay-tree structure is used to store and search commands. Profiling shows low search speed. A more efficient data structure and algorithm for this task need to be developed or found.

### Navigation

New navigation principle. The VIEW level is determined dynamically, not statically. The current path tree is built in memory. The zero level of the tree is considered the global command space "__view_global". The first level is the initial VIEW specified in STARTUP (or by other means). If the user enters nested VIEWs, they become levels 2, 3, etc., respectively. It is impossible to move up the tree beyond level 1. Attempting to do so exits the program.

Compatibility mode is not provided.

The "depth" attribute for the VIEW tag is declared deprecated. The "restore=view" attribute for COMMAND and VIEW tags is declared deprecated. The "view" attribute is declared deprecated.

Inside the COMMAND tag, a new "nav" attribute appears - navigation. New navigation commands (the "nav" attribute):

* `down:<nested_view>` - enter the nested VIEW with the specified name. Increases the nesting level by one.
* `up[:<number>]` - exit the nested VIEW. Decreases the nesting level by one. If <number> is specified, moves up the current path tree by the specified number of levels.
* `replace:<view>[@<level>]` - remain at the current nesting level, replacing the current VIEW with the one specified in the command. If the tree level <level> is specified, <view> replaces not the current VIEW, but the VIEW at the specified level. The level might be invalid. For example, being above the current level or < 1. In such a case, a navigation error is issued.
* `exit` - exit the program.

### Schema Definition Method

Currently, the schema and command set are defined using XML files. A mechanism needs to be developed that allows describing the schema and commands in other ways. Each method can have its strengths and applications. Possible methods:

* XML. Simplicity, compatibility (full or partial) with existing developments. Ability to make changes without recompilation.
* Lua. Flexibility. Ability to make changes without recompilation.
* Embedding in C code. Speed, independence from external libraries, ability to compile a single static binary without external library and configuration files. The C code for embedding should be generated from sources in other languages (XML, Lua, ...).

The mechanism must be modular and allow adding new ways to describe the schema and commands beyond those listed.

The main requirement for the klish schema mechanism is the ability to describe the schema using XML. Other description methods can extend, improve, or simplify the description, but the internal schema representation in klish must be covered, even if suboptimally, using XML.

At the first stage, implementing the XML option is sufficient. The implementation must consider the possibility of implementing other options in the future.

### Command Separator

A command separator, as a basic functionality, is not required. If necessary, a command separator can be implemented on the client side (see architecture description).

The ability to enter multiple commands in one line can be implemented. Commands are separated by the ";" character. This can be useful when the command source is a file or a shell command line.

### Error Pointer

If an incorrect command is entered, the CLI should show where in the command the error was detected. By indicating the character number and an arrow "^". The character number alone is insufficient because it's hard to count. The arrow alone is insufficient because there are multi-line commands.

### Hot-keys

Improvement of the Hot-keys mechanism. As part of the general refactoring of the internal structure.

### NAMESPACE Inheritance

Automatic inheritance of NAMESPACE from VIEWs at lower levels. (VIEWs with smaller depth values are considered lower levels, i.e., the root of the tree).

Higher-level VIEWs no longer need to explicitly include command inheritance from lower-level VIEWs using NAMESPACE. Inheritance happens automatically. NAMESPACE should only be specified for service VIEWs used to aggregate commands. For example, when these commands need to be included in multiple VIEWs simultaneously. Using service VIEWs as independent ones is not recommended. I.e., such VIEWs should not appear in the current path tree.

For the VIEW tag, attributes similar to NAMESPACE attributes appear but relate to lower levels of the current path tree.

* `inherit="<true/false>"` - whether commands from lower-level VIEWs of the current path tree are available in the current VIEW. Default is `inherit="true"`.
* `completion="<true/false>"` - same as in NAMESPACE, but relative to lower levels.
* `context_help="<true/false>"` - same as in NAMESPACE, but relative to lower levels.

The search for an entered command occurs as follows: from higher nesting levels of the path tree to lower levels. At each level, the search is performed in the current VIEW and all its NAMESPACEs. If the command is not found, the search moves to a lower level of the tree.

When a command is found, its "restore" field is analyzed. If the field is not defined, the command is executed at the current nesting level. If defined, the command's level is considered the one where it was found. That level is restored (or it matches the current one) and the command is executed.

### Handlers for PTYPE

Currently, PTYPE types only have predefined methods for validating entered arguments. Validation of PTYPE using arbitrary code is required. An ACTION tag is added. Argument validity is checked only using ACTION. Built-in checks are eliminated. Uniformity is achieved. Former built-in checks are implemented in a plugin.

### PTYPE Scope

Currently, all PTYPE types are global. Since a global command space exists, technically a zero-level VIEW also exists to implement this global namespace. I.e., in the general case, the VIEW structure contains both a set of commands belonging to this scope and a set of PTYPEs, as they should be in the global VIEW. There is no point in making the global VIEW (level 0) special. This would only add complexity to the code. Therefore, local PTYPEs are obtained almost automatically. More local PTYPEs mask more global PTYPEs with the same name (just like in programming languages).

### Arguments and VAR Scope

VAR variables must have PARAM parameters, just like commands.

VAR can be not only global but also belong to a specific VIEW. Global VARs are in the global VIEW. Variables are resolved from more local to more global.

### (?) Multi-parameters

Currently, one formal PARAM corresponds to exactly one actual entered parameter. The ability to enter multiple actual parameters for one formal parameter needs to be implemented. Consider how to declare such parameters. Possible options:

* `<PARAM ... multi="true">`
* `<PARAM ... number="1..4">` The numbers indicate the minimum and maximum number of actual parameters of this type, respectively.

**If there is a field indicating the possible number of parameters, then `optional="true"` implies `multi="0..1"`. How to access multi-parameters later? Is args needed for this? If needed, should it be typed?**

The second implementation option for multi-parameters:
Introduce a `MULTI` tag that can repeat the parameters nested within it. The advantage of this method is that it can repeat not just one parameter but a whole sequence. This includes using nested `SWITCH`. The disadvantage of this approach is that it is more cumbersome. The number of possible repetitions is set by the `number="1..4"` field. The tag instance can have an optional name `name="my_multi"`. Using a variable with this name, the actual number of entered repetitions can be known.

### Automation Mode

A special klish operation mode intended for automation management is introduced. In this mode, an external program communicating with klish should be able to get the return code of each executed command, as well as its output and errors. This is achieved because the communication protocol between the client and the klish core is binary. One client is used for interactive mode, another for automation mode. The exchange protocol implies that the client will be sent a return code upon command execution. How the client uses this code is its business.

### Enter on Erroneous Command

Allow entering a command even if it fails syntactic validation. For automatic management, it's necessary to accept the command and say it's incorrect, not block input.

### Space on Erroneous Command

Allow entering a space even if parameters fail syntactic validation.

### Session Settings

Currently, in startup.xml, there is a hack that implements directories with session settings /tmp/clish-session.<ssid>. This is done, for example, for the pager off/on command.

The new klish should have the ability to change variable values. This is sufficient to implement session settings. The client should be able to get variable values from the klish core.

### (?) Pipeline

Ability to pass command output through '|'.

In klish, where the command set is predefined and fixed, it doesn't make sense to allow using an arbitrary command to the right of "|". All commands allowed to the right of "|" can be called output filters. These are, for example, grep, head, etc. Using an arbitrary command to the right of "|" can lead to command chain hangs. For output filters, a new FILTER tag is provided. This tag completely replicates the COMMAND tag format. Internally, a filter is a command with an additional structure field `filter=true;`. A non-filter command cannot appear to the right of "|", and a filter cannot appear before "|". Accordingly, filters do not appear in autocompletion when entering the main (first) command. And ordinary commands do not appear in autocompletion after entering "|".

The FILTER tag has a special field `auto="true"`. Such filters are run automatically for any command. The main purpose of such a command is the pager. If multiple filters with the `auto="true"` field are defined, they will run in alphabetical order by filter name.

The pager is interactive, i.e., it waits for user input. Interactive commands can only be last in the "|" call chain. Regular commands can also be interactive. For example, a command launching a text editor. Such commands cannot be passed through a pager. An `interactive="true/false"` attribute is introduced for the ACTION tag. The attribute indicates that no filters should be called after this command; in other words, this command is the last in the chain. Default is `interactive="false"`.

A command in klish can be executed in the context of the current process or in a separate process (fork()). By default, all commands will be executed in a separate process. In the previous version, the task of spawning processes was assigned to the builtin command (e.g., code that ran a shell script). Now klish itself will handle this. Commands executed in the current context cannot be filters because it's impossible to build a chain of functions connected via pipe in a single-threaded application. To understand which commands need to be run in the current context and which need to be fork()-ed, a `fork=true/false` field appears in the clish_sym_t structure.

There should be a command to globally disable the pager, i.e., automatic filters.

**(?) Is this really needed? Is it necessary to introduce a new FILTER tag?**

### Localization

It should be possible to write help for parameters and commands in any language. The output language for hints should depend on the locale.

### (?) Documentation

Consider a mechanism for embedding documentation in the XML file. This refers to documentation describing commands and parameters. For example, introduce a DOC tag for each element requiring description. Based on such embedded documentation, a description of all commands can be automatically generated. A separate utility can generate the output document, and klish can ignore the DOC tags.

**(?) In what language should this documentation be? gettext? Wouldn't it be too cumbersome for PARAM, for example?**

### Plugin

The plugin object should contain a reference to userdata inside itself.

### Shebang Field

For the ACTION tag, the shabang field is no longer required. Everything will be specified by the builtin field or its equivalent.

### Builtin Field

The name `builtin` no longer reflects the essence of the attribute. Instead of `builtin`, the name `sym` will be used. The attribute is a reference to a symbol from the plugin.

### Konfd Service

The konfd mechanism is no longer supported. It is recommended to implement similar functionality through other mechanisms. Possibly making access to the configuration storage just part of an `ACTION`.

### COMMAND Visibility

Currently, there is an `access` field for commands, but it is static, i.e., checked at klish startup. Commands filtered at the loading stage do not enter the command list (in the program's memory) at all. For PARAM parameters, there is a `test` field that allows dynamically hiding individual parameters based on a given condition. A similar dynamic mechanism for commands is needed. Only the `test` field is replaced by a `<COND>` tag.

### Substitutions

In the current klish version, substitutions are widely used. Substitutions are text strings that include references to klish variables. When using such strings, the values of klish variables are expanded directly into the text where the variable reference previously stood. Currently, substitutions are used in the following constructs:

  * ACTION:script
  * CONFIG:attributes
  * PARAM:completion
  * PARAM:test
  * VIEW:prompt
  * VIEW:viewid
  * VAR:value
  * COMMAND:view
  * HOTKEY:cmd

In many cases, using substitutions is unsafe, especially in the case of a script for execution by a shell interpreter. Additionally, it's impossible to implement substitution (i.e., string value evaluation) in an arbitrary language. There is no way to specify the substitution interpreter. In the new klish, substitutions as a basic functionality should not exist. In most cases, substitutions are used in XML tag attributes. Attributes are replaced by nested tags. These nested tags can contain an ACTION tag. Thus, string generation, as in completion, or condition checking, as in test, can be written in any supported language (see ACTION tag).

Examples of replacing attributes with nested tags:

```
<VIEW .... >
  <PROMPT>
    <ACTION>
    echo "my_prompt"
    </ACTION>
  </PROMPT>
```

```
<PARAM ...>
  <COND>
    <ACTION>
      test $env_var -eq 0
    </ACTION>
  </COND>
</PARAM>
```

In cases where substitutions are still useful, they can be implemented inside the executable function that will process the ACTION content, i.e., the script.

### Escaping

This section concerns cases where it's necessary to enter a complex string (with spaces and special characters) on the command line. It's desirable to avoid excessive escaping that makes the string unreadable. For this, the new klish should use 3 different characters for quoting:

- `"` (double quote)
- `'` (single quote)
- &#96; (backtick)

Additionally, repeated quote characters can be used as opening and closing quotes. For example, the character sequence `"""` can be an opening quote. In this case, the closing quote must be the same character sequence. This approach allows using the same character inside the string as the opening/closing quote but with a smaller number of consecutive characters. For example:

```
""This is a "long" string""
```

Here, the nested quotes around the word `long` are part of the string. The opening and closing quotes are the character sequence `""`. The number of characters in the opening and closing sequence can vary depending on the situation and string content. For definiteness, let's assume the number of characters in the sequence should not exceed three.

### Autocompletion (completion)

Currently, the code generating autocompletion options separates these options with a space. Sometimes the options themselves contain spaces. Then the scheme stops working. In the new klish, the code generating autocompletion options should separate them with a newline character.

## Architecture

### Client - Server

A client-server model is used for the klish system. The server is the klish core. The core runs under a specific user, and all actions performed by the core are executed under that same user. Clients running under this user establish a connection with the core using network means (sockets). Accordingly, in the general case, multiple clients can connect to one core simultaneously. For each client, the core creates its own session.

Upon startup, the core processes schema files, i.e., XML files defining the set of available commands. Then the core waits for connections from clients and responds to their requests. Clients essentially implement the user interface and do not execute any commands themselves. Instead, they send command execution requests to the core, the core executes the command, and then returns the result. The exchange protocol between the core and clients is standardized and allows transmitting service information. Based on this protocol, one can implement both a text client aimed at end-user work, a text client aimed at automated management, and also a graphical client.

The behavior of the core and clients is regulated by special configuration files (not to be confused with schema files).

At least two standard usage schemes for klish are possible.

The first scheme implies that for a specific user, only one running instance of the core exists. Multiple clients connect to this core. In this scheme, two strategies for launching the core can exist. First - the core is launched once by external means and then remains in memory constantly. It is unloaded also by external means. The second strategy - the core is launched automatically when the first (local) client starts. It is unloaded when the last client disconnects. Thus, the core does not occupy RAM when it has no clients.

The second scheme implies that a separate core instance is launched for each client. This scheme corresponds to klish branch number 2, where there is no client-server separation, and the user interface is combined with the execution core.

An option was considered where the system contains only one core instance designed to work with all users simultaneously, as well as an option where clients can be remote. For these cases, complex authentication built into the core would be needed. As well as logic in the core executing requests from different clients under different users. This in turn would necessitate launching the core as root.

### Executable Functions

Here, executable functions refer to those functions called when executing klish commands. All executable functions are implemented in plugins. Plugins can be external or internal. The strange phrase "internal plugin" is justified because the implementation of structures for internal and external plugins is very similar. There is no point in making separate mechanisms for standard functions that should always be present in the system and for user functions.

The basic unit of information in a plugin is a "symbol". By analogy with symbols in shared libraries. A symbol represents a reference to an executable function. Besides the function reference, the symbol contains additional service information. An example of such service information is the symbol type.

Symbols (functions) can be of two main kinds:

  * asynchronous (async)
  * synchronous (sync)

The main difference between synchronous and asynchronous symbols is that synchronous functions are executed within the klish core, while asynchronous functions are executed in a process forked from the core. The core handles its own business until it receives a signal that the forked process has completed. Thus, safe execution of third-party functions is achieved. Errors in asynchronous functions cannot affect the klish core. Also, long execution time of asynchronous functions does not affect the core's responsiveness.

Using asynchronous functions is recommended over synchronous ones. Using synchronous functions is only acceptable when the function's execution duration is fixed and small. An example of a synchronous function is the prompt formation function. This function doesn't take much time and is called very often. Therefore, making it asynchronous is too resource-intensive.

Due to the differences between synchronous and asynchronous functions, their API and the way they access klish variables (VAR) are different.

#### Synchronous Functions

The klish core passes to the synchronous function a reference to the internal variable storage, a pointer to the stdout and stderr strings.

Using the reference to the variable storage and the corresponding API for accessing this storage, the function can get and set klish variable values. Importantly, a synchronous function can only access those variables whose ACTIONs are implemented using synchronous functions. If an ACTION is implemented by an asynchronous function, accessing such a variable will cause an error. Accessing an asynchronous function from a synchronous function is not allowed.

The result of a synchronous function's work is a return code and the formed stdout and stderr strings.

The return code can be successful (0) or unsuccessful (any other number). The specific number, in case of unsuccessful completion, can indicate the error type.

Using the stdout, stderr pointers, the function can return strings to the core for output to the respective streams. The core, in turn, will forward these strings to the client. When the function is executed as an ACTION for a klish variable, the stdout string will be the variable's value.

#### Asynchronous Functions

An identifier of the klish core is passed to the asynchronous function. Here, the core identifier implies an object allowing communication with the core, since when the asynchronous function is launched, it will be executed in a forked process, and the core process is external to this forked process. The object can be a network socket, pipe, etc.

Using the core identifier, the function can send a request to the core to get or set a klish variable's value. For such requests, a special API, different from the API for working with klish variables from synchronous functions, should be used. For asynchronous functions, there are no restrictions on the type of variable whose value is requested from the core. Variable ACTIONs can be implemented by both asynchronous and synchronous functions.

An asynchronous function returns a return code in the same format as a synchronous function.

For interactive (or non-interactive) exchange of strings with the client, before forking the process to launch the asynchronous function, the core creates a pseudo-terminal to provide the executable function with stdin, stdout and a pipe to provide stderr. The stderr stream is separated from stdout so that the core and client can distinguish these two streams. After launching the function, the core reads its side of the pseudo-terminal and pipe to forward data to the client. When the asynchronous function implements a klish variable's ACTION, all output to stdout is the value of that variable.

#### Input and Output Streams for Asynchronous Functions

Asynchronous functions can be interactive and non-interactive. Interactive functions are provided with a pseudo-terminal for interaction with the user. Thus, the executed code won't see the difference between working directly with the user and working with the user through the "remote" client-server channel of the klish system. Non-interactive functions are provided with input/output streams via pipe.

## Design

### Stream File Operations

Where possible, avoid using stream file operations (fopen(), fgets(), etc.), because during fork() and subsequent operations with file objects, including simply atexit(), fflush() on the stream and ultimately lseek() on the file descriptor can occur. This all leads to problems with file positioning.

## Formatting and Style

### Automatic Formatting

The project adopts a formatting style similar to that used in Linux kernel development. There is a utility `indent` that formats the source code to the required style. In the project's source tree root, there is a script `indent.sh` that runs the `indent` utility with the necessary options. The set of used options can be seen in the source code of the `indent.sh` script. Example usage:

```
$ ./indent src/prog.c
```

Using `indent.sh` solves most formatting problems, but there are additional source code styling rules.

Besides the `indent` utility, there is a program `clang-format` that also handles source code formatting. For this program, a `.clang-format` file is placed in the source tree root, which sets the formatting rules. To use `clang-format`, the following command is used:

```
$ clang-format -i filname.c
```

Where `filename.c` is the name of the file to be processed.

### Blank Lines

Use two blank lines to separate functions from each other.

A blank line separates variable declarations from other function or block commands.

```
int fn1(void)
{
        int i = 1;
        int j = 0;
        float b = 3.5;

        i += 3;

        return i;
}


int fn2(void)
{
        int i = 1;

        i += 4;
        i = 5 + i * 9;

        return i;
}

```

### Variables

Variables should have clear names. Variable names can contain Latin letters in lower case, digits, and underscore `_`.

Each variable is declared on a separate line.

Each variable must be initialized.

Pointers, after freeing the memory they point to, must be set to NULL.

Avoid using global variables.

Assigning a value to each variable is done on a separate line. Do not assign values to multiple variables in one line like `a = b = foo();`.

### Structure Size

Suppose memory needs to be allocated to store a structure. Usually, two different styles are used to determine the size of the allocated memory.

First style:

```
struct mytype *a = NULL;
a = malloc(sizeof(struct mytype));

```

Second style:

```
struct mytype *a = NULL;
a = malloc(sizeof(*a));

```

This project uses the second style. This style is safer because when the variable type changes, the memory allocation line does not need to be changed.

### Null Check

For shorter and clearer notation, the pointer null check is performed according to the following style:

```
if (ptr) ...
if (!ptr) ...
```

The null byte check, in cases like end of string or a separate null character, is performed according to the following style:

```
if ('\0' == *ptr) ...
if ('\0' == c) ...
```

The reverse order of operands for comparison is explained by protection against accidental replacement of the comparison operation `==` with the assignment operation '='. Such errors are difficult to debug.

The null check when processing return values from functions (usually int type) is performed as follows:

```
if (foo() == 0) ...
a = foo();
if (0 == a) ...
if (a < 0) ...
if (a != 0) ... 
```

### memset() or bzero()

The bzero() function is more illustrative but less portable. In the modern POSIX standard, bzero() is marked as legacy. Programs should use the memset() function. To combine the advantages of both functions, the auxiliary library `faux` has a function `faux_bzero()`, which has the interface of the `bzero()` function but internally uses the more portable `memset()` function.
```
