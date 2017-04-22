
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "protocol.h"

static Gopher *cmdBack(Gopher *, int, char *const *);
static Gopher *cmdLink(Gopher *, int, char *const *);
static Gopher *cmdOpen(Gopher *, int, char *const *);
static Gopher *cmdShow(Gopher *, int, char *const *);

static Gopher *(*handler[]) (Gopher *, int, char *const *) = {
	['!'] = NULL,
	['1'] = cmdLink,
	['2'] = cmdLink,
	['3'] = cmdLink,
	['4'] = cmdLink,
	['5'] = cmdLink,
	['6'] = cmdLink,
	['7'] = cmdLink,
	['8'] = cmdLink,
	['9'] = cmdLink,
	['B'] = cmdBack,
	['O'] = cmdOpen,
	['S'] = cmdShow
};
static const struct { char *cmd, *args, *desription; } commands[] = {
	{ "back", "", "TODO" },
	{ "open", "host [path [port]]", "TODO" },
	{ "show", "[start [end]]", "TODO" },
	{ NULL, NULL, NULL }
};


Gopher *cmdBack(Gopher *gopher, int argc, char *const *argv) {
	// TODO implement history stack (here or in protocol??)
	return gopher;
}

Gopher *cmdLink(Gopher *gopher, int argc, char *const *argv) {
	// TODO paste  argv[1] + argv[2] + ...
	gopher = gopherFollowLink(gopher, atoi(argv[0]), argv[1]);
	return gopher;
}

Gopher *cmdOpen(Gopher *gopher, int argc, char *const *argv) {
	if (argc < 2) return gopher;
	gopherFree(&gopher);
	if (argc == 2) gopher = gopherGet(argv[1], 70, "");
	else if (argc == 3) gopher = gopherGet(argv[1], 70, argv[2]);
	else if (argc == 4) gopher = gopherGet(argv[1], atoi(argv[3]), argv[2]);
	gopherShowMenu(gopher, 0, 0);
	return gopher;
}

Gopher *cmdShow(Gopher *gopher, int argc, char *const *argv) {
	// TODO in protocol: allow negative numbers as offset from end
	int start = 0, end = 0;
	if (argc > 1) start = atoi(argv[1]);
	if (argc > 2) end = atoi(argv[2]);
	gopherShowMenu(gopher, start, end);
	return gopher;
}


char *generator(const char *text, int state) {
	static int n, len;
	if (!state) { n = -1; len = strlen(text); }
	while (commands[++n].cmd)
		if (!strncmp(commands[n].cmd, text, len))
			return strdup(commands[n].cmd);
	return NULL;
}

Gopher *parseCommand(Gopher *gopher, char *cmd) {
	if (cmd[0] == 0) return gopher;
	char *c, *dup = strdup(cmd);
	for (c = dup; *c; ++c) *c = toupper(*c);
	int argc = 0;
	char **argv = NULL, *argp;
	argp = strtok(dup, " \t\n");
	while (argp) {
		argv = realloc(argv, (++argc) * sizeof(char *));
		argv[argc-1] = strdup(argp);
		argp = strtok(NULL, " \t\n");
	}
	free(dup);
	if (handler[**argv]) {
		gopher = handler[**argv](gopher, argc, argv);
		add_history(cmd);
	}
	else {
		printf("Command not found: %s\n", cmd);
	}
	while (argc) free(argv[--argc]);
	return gopher;
}

int main(int argc, char *const *argv) {
	Gopher *gopher = NULL;
	int i;
	char *cmd;
	for (cmd = argv[i=1]; i < argc; cmd = argv[++i])
		gopher = parseCommand(gopher, cmd);
	rl_completion_entry_function = generator;
	while ((cmd=readline(">> "))) {
		gopher = parseCommand(gopher, cmd);
		free(cmd);
	}
	return 0;
}
