
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "protocol.h"
#include "config.h"

#ifndef PREFIX
#define PREFIX "/usr"
#endif
#ifndef PROG
#define PROG "goldy"
#endif

static Gopher *cmdBack(Gopher *, int, char *const *);
static Gopher *cmdFind(Gopher *, int, char *const *);
static Gopher *cmdGo(Gopher *, int, char *const *);
static Gopher *cmdHelp(Gopher *, int, char *const *);
static Gopher *cmdLink(Gopher *, int, char *const *);
static Gopher *cmdOpen(Gopher *, int, char *const *);
static Gopher *cmdQuit(Gopher *, int, char *const *);
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
	['F'] = cmdFind,
	['G'] = cmdGo,
	['H'] = cmdHelp,
	['O'] = cmdOpen,
	['Q'] = cmdQuit,
	['S'] = cmdShow
};

Gopher *cmdBack(Gopher *gopher, int argc, char *const *argv) {
	Gopher *goph = history(NULL, 0, NULL);
	if (!goph) return gopher;
	gopherFree(&gopher);
	gopherShowMenu(goph, 0, 0);
	return goph;
}

Gopher *cmdFind(Gopher *gopher, int argc, char *const *argv) {
	// TODO direct query to Veronica...
	printf("Find is not yet implemented\n");
	return gopher;
}

Gopher *cmdGo(Gopher *gopher, int argc, char *const *argv) {
	gopher = cmdOpen(gopher, argc, argv);
	gopherShowMenu(gopher, 0, 0);
	return gopher;
}

Gopher *cmdHelp(Gopher *gopher, int argc, char *const *argv) {
	gopherFree(&gopher);
	gopher = gopherGet("file", 0, PREFIX "/share/" PROG "/help");
	gopherShowMenu(gopher, 0, 0);
	return gopher;
}

Gopher *cmdLink(Gopher *gopher, int argc, char *const *argv) {
	// TODO paste  argv[1] + argv[2] + ... for search engines ...
	if (!gopher) return gopher;
	char *title = strdup(gopher->string);
	gopher = gopherFollowLink(gopher, atoi(argv[0]), (argc > 1 ? argv[1] : NULL));
	/* only show the menu if it's a new location */
	if (auto_show_on_follow && strcmp(title, gopher->string))
		gopherShowMenu(gopher, 0, 0); // TODO get rid of auto_show and use max...
	free(title);
	return gopher;
}

Gopher *cmdOpen(Gopher *gopher, int argc, char *const *argv) {
	if (argc < 2) return gopher;
	gopherFree(&gopher);
	if (argc == 2) gopher = gopherGet(argv[1], 70, "");
	else if (argc == 3) gopher = gopherGet(argv[1], 70, argv[2]);
	else if (argc == 4) gopher = gopherGet(argv[1], atoi(argv[3]), argv[2]);
	return gopher;
}

Gopher *cmdQuit(Gopher *gopher, int argc, char *const *argv) {
	/* if needed, create an empty gopher to carry the QUIT message back */
	if (!gopher) gopher = calloc(2, sizeof(Gopher));
	gopher->item = GOPHER_QUIT;
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


char *rl_generator(const char *text, int state) {
	static const char *cmd[] = { "back", "find", "go", "help", "open", "quit", "show", NULL };
	static int n, len;
	if (!state) { n = -1; len = strlen(text); }
	while (cmd[++n])
		if (!strncmp(cmd[n], text, len))
			return strdup(cmd[n]);
	return NULL;
}

Gopher *parseCommand(Gopher *gopher, char *cmd) {
	if (cmd[0] == 0) return gopher;
	char *dup = strdup(cmd);
	int argc = 0;
	char **argv = NULL, *argp;
	argp = strtok(dup, " \t\n");
	while (argp) {
		argv = realloc(argv, (++argc) * sizeof(char *));
		argv[argc-1] = strdup(argp);
		argp = strtok(NULL, " \t\n");
	}
	free(dup);
	if (handler[toupper(**argv)]) {
		gopher = handler[toupper(**argv)](gopher, argc, argv);
		add_history(cmd);
	}
	else {
		printf("Command not found: %s\n", cmd);
	}
	while (argc) free(argv[--argc]);
	free(argv);
	return gopher;
}

int main(int argc, char *const *argv) {
	Gopher *gopher = NULL;
	int i;
	char *cmd = NULL;
	size_t n = 0;
	for (cmd = argv[i=1]; i < argc; cmd = argv[++i])
		gopher = parseCommand(gopher, cmd);
	rl_completion_entry_function = rl_generator;
	while ((!gopher || gopher->item != GOPHER_QUIT) && (cmd=readline(prompt))) {
		gopher = parseCommand(gopher, cmd);
		free(cmd);
	}
	if (gopher) gopherFree(&gopher);
	history(NULL, -1, NULL);
	return 0;
}
