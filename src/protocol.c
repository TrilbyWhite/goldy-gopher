
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include "protocol.h"

extern Color colors[];

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
	gp->item = GOPHER_START;
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
	gopher[n - 1].item = GOPHER_NULL;
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

int gopherShowMenu(Gopher *gopher, int start, int end) {
	if (!gopher) return 1;
	Gopher *gp = gopher;
	int n = 1;
	struct Color *col;
	printf("\n \033[38;5;46;1mgopher://%s\033[0m\n\n", gp->string);
	for (++gp; gp && gp->item; ++gp, ++n) {
		if (start && n < start) continue;
		if (end && n > end) break;
		col = &colors[gp->item];
		if (!col) col = &colors[127];
		printf("\033[0;%sm%4d  \033[0;%sm%s\033[0m\n", col->item, n, col->str, gp->string);
	}
	printf("\n");
	return 0;
}

Gopher *gopherFollowLink(Gopher *gopher, int n, const char *args) {
	if (n < 1) return gopher;
	Gopher *gp;
	for (gp = gopher; n && gp && gp->item; ++gp, --n);
	if (!gp || gp->item == 'i' || gp->item == '3') return gopher;
	if (!gp->host || !gp->port || !gp->path) return gopher;
	char *host = strdup(gp->host);
	int port = gp->port;
	char *path, item = gp->item;
	if (args && args[0]) asprintf(&path, "%s\t%s", gp->path, args);
	else path = strdup(gp->path);
	switch (item) {
		case '1': case '7':
			gopherFree(&gopher);
			gopher = gopherGet(host, port, path);
			break;
		case '0':
			break;
	//TODO
	}
	free(host);
	free(path);
	return gopher;
}

