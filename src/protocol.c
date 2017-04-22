
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include "protocol.h"

int gopherFree(Gopher **gopher) {
	Gopher *gp;
	if (!gopher || !*gopher) return 0;
	for (gp = *gopher; gp && gp->item; ++gp) {
		free(gp->string);
		free(gp->host);
		free(gp->path);
	}
	free(*gopher);
	*gopher = NULL;
	return 0;
}

Gopher *gopherGet(const char *host, int port, const char *path) {
	FILE *fp = gopherOpen(host, port, path);
	if (!fp) return NULL;
	int n = 1;
	char line[1024], *ptr;
	Gopher *gopher = malloc(sizeof(Gopher));
	Gopher *gp = gopher;
	gp->item = 1;
	gp->host = strdup(host);
	gp->port = port;
	gp->path = strdup(path);
	asprintf(&gp->string, "%s:%d%s", gp->host, gp->port, gp->path);
	while (fgets(line, 1024, fp) && line[0] != '.') {
		gopher = realloc(gopher, (++n) * sizeof(Gopher));
		gp = &gopher[n-1];
		gp->item = line[0];
		ptr = strtok(line, "\t");
		if (ptr[1] != 0) gp->string = strdup(ptr+1);
		else gp->string = strdup("");
		gp->path = NULL;
		gp->host = NULL;
		if ((ptr=strtok(NULL, "\t"))) gp->path = strdup(ptr);
		if ((ptr=strtok(NULL, "\t"))) gp->host = strdup(ptr);
		if ((ptr=strtok(NULL, "\t"))) gp->port = atoi(ptr);
	}
	gopher = realloc(gopher, (++n) * sizeof(Gopher));
	gopher[n - 1].item = 0;
	fclose(fp);
	return gopher;
}

FILE *gopherOpen(const char *host, int port, const char *path) {
	int fd;
	struct hostent *server;
	struct sockaddr_in serv_addr;
	if ((fd=socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("gopherOpen");
		return NULL;
	}
	if (!(server=gethostbyname(host))) {
		printf("gopherOpen: %s: Name not known\n", host);
		return NULL;
	}
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
	serv_addr.sin_port = htons(port);
	if (connect(fd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("gopherOpen");
		return NULL;
	}
	FILE *fp = fdopen(fd, "r+");
	fprintf(fp, "%s\n",path);
	return fp;
}

static void gopherDrawLine(int col, int n, int bold, const char *str) {
	if (bold) printf("\033[38;5;%d;1m%4d \033[0;1m %s\033[0m\n", col, n, str);
	else printf("\033[38;5;%d;1m%4d \033[0m %s\033[0m\n", col, n, str);
}

int gopherShowMenu(Gopher *gopher, int start, int end) {
	Gopher *gp = gopher;
	int n = 1;
	printf("\n \033[38;5;46;1mgopher://%s\033[0m\n\n", gp->string);
	for (++gp; gp && gp->item; ++gp, ++n) {
		if (start && n < start) continue;
		if (end && n > end) break;
		switch(gp->item) {
			case '0': break;
			case '1': gopherDrawLine(33, n, 1, gp->string); break;
			case '2': break;
			case '3': break;
			case '4': break;
			case '5': break;
			case '6': break;
			case '7': gopherDrawLine(48, n, 1, gp->string); break;
			case '8': break;
			case '9': break;
			case 'g': break;
			case 'h': break;
			case 'i': gopherDrawLine(239, n, 0, gp->string); break;
			case 's': break;
			case 'I': break;
			case 'T': break;
			default: printf("\033[31m%c%s\033[0m\n", gp->item, gp->string);
		}
	}
	printf("\n");
	return 0;
}

Gopher *gopherFollowLink(Gopher *gopher, int n, const char *args) {
	if (n < 1) return gopher;
	Gopher *gp;
	for (gp = gopher; n && gp && gp->item; ++gp, --n);
	if (!gp->item || gp->item == 'i' || gp->item == '3') return gopher;
	if (!gp->host || !gp->port || !gp->path) return gopher;
	// TODO: check item type ...
	char *host = strdup(gp->host);
	int port = gp->port;
	char *path;
	if (args) asprintf(&path, "%s\t%s", gp->path, (++args));
	else path = strdup(gp->path);
	gopherFree(&gopher);
	gopher = gopherGet(host, port, path); // TODO don't auto get, check type...
	free(host);
	free(path);
	gopherShowMenu(gopher, 0, 0);
	return gopher;
}

