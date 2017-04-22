
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "protocol.h"

/*
RFC 1436
RFC 4266
*/

char *opts[] = {
	"one",
	"two",
	"three",
	NULL
};

char *generator(const char *text, int state) {
	static int n, len;
	if (!state) { n = -1; len = strlen(text); }
	while (opts[++n])
		if (!strncmp(opts[n], text, len))
			return strdup(opts[n]);
	return NULL;
}

Gopher *command(Gopher *gopher, const char *cmd) {
	if (cmd[0] == 0) return gopher;
	int link = 0;
	if ((link=atoi(cmd)))
		gopher = gopherFollowLink(gopher, link, cmd);
	else if (cmd[0] == 'o') {
		gopherFree(&gopher);
		gopher = gopherGet("sdf.org", 70, "");
		gopherShowMenu(gopher, 0, 0);
	}
	else
		printf("%s\n");
	add_history(cmd);
	return gopher;
}

int main(int argc, char *const *argv) {
	Gopher *gopher;
	/*
	if (argc == 4) gopher = gopherGet(argv[1], atoi(argv[2]), argv[3]);
	else if (argc == 3) gopher = gopherGet(argv[1], atoi(argv[2]), "/");
	else if (argc == 2) gopher = gopherGet(argv[1], 70, "/");
	else gopher = gopherGet("sdf.org", 70, "/");
	gopherShowMenu(gopher, 0, 0);
	*/

	int i;
	char *cmd;
	for (cmd = argv[i=1]; i < argc; cmd = argv[++i])
		command(NULL, cmd);

	rl_completion_entry_function = generator;
	while ((cmd=readline("> "))) {
		gopher = command(gopher, cmd);
		free(cmd);
	}
	return 0;
}
