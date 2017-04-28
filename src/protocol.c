
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <error.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include "protocol.h"

extern Color colors[];

Gopher *history(const char *host, int port, const char *path) {
	static char *hosts[MAX_HIST];
	static char *paths[MAX_HIST];
	static int ports[MAX_HIST];
	static int n = -1;
	if (port == -1) { /* free all history */
		for (n = 0; n < MAX_HIST; ++n) {
			free(hosts[n]);
			free(paths[n]);
		}
		return NULL;
	}
	if (host && path) { /* push */
		if (hosts[n] && !strcmp(host,hosts[n]) && port==ports[n] &&
				paths[n] && !strcmp(path,paths[n]))
			return NULL;
		free(hosts[++n]);
		free(paths[n]);
		hosts[n] = strdup(host);
		paths[n] = strdup(path);
		ports[n] = port;
		return NULL;
	}
	/* pop */
	if ((--n) < 0) {
		if (hosts[MAX_HIST-1]) n = MAX_HIST-1;
		else n = 0;
	}
	return gopherGet(hosts[n], ports[n], paths[n]);
}

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
	history(host, port, path);
	int fd = gopherOpen(host, port, path);
	if (fd < 0) return NULL;
	FILE *fp = fdopen(fd, "r");
	if (!fp) return NULL;
	int n = 1;
	char line[1024], *ptr, *rpath;
	Gopher *gopher = malloc(sizeof(Gopher));
	Gopher *gp = gopher;
	gp->item = GOPHER_START;
	gp->host = strdup(host);
	gp->port = port;
	if (path[0] == '/') gp->path = strdup(path + 1);
	else gp->path = strdup(path);
	if (strncmp(host,"file",5) == 0) {
		rpath = realpath(path, NULL);
		asprintf(&gp->string, "file://%s", rpath);
		free(rpath);
	}
	else {
		asprintf(&gp->string, "gopher://%s:%d/%s", gp->host, gp->port, gp->path);
	}
	while (fgets(line, 1024, fp) && line[0] != '.') {
		if ((ptr=strpbrk(line, "\r\n"))) *ptr = 0;
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

int gopherOpen(const char *host, int port, const char *path) {
	int fd;
	/* this is cool ... local files without a server! */
	if (strncmp(host,"file",5) == 0)
		return open(path, O_RDONLY);
	struct hostent *server;
	struct sockaddr_in serv_addr;
	if ((fd=socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("gopherOpen");
		return -1;
	}
	if (!(server=gethostbyname(host))) {
		printf("gopherOpen: %s: Name not known\n", host);
		return -1;
	}
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
	serv_addr.sin_port = htons(port);
	if (connect(fd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("gopherOpen");
		return -1;
	}
	dprintf(fd, "%s\n",path);
	return fd;
}

int gopherShowMenu(Gopher *gopher, int start, int end) {
	if (!gopher) return 1;
	Gopher *gp = gopher;
	int n = 1;
	struct Color *col;
	printf("\n \033[38;5;46;1m%s\033[0m\n\n", gp->string);
	for (++gp; gp && gp->item; ++gp, ++n) {
		if (start && n < start) continue;
		if (end && n > end) break;
		col = &colors[gp->item];
		if (!col || !col->item || !col->str) col = &colors[127];
		printf("\033[0;%sm%4d  \033[0;%sm%s\033[0m\n", col->item, n, col->str, gp->string);
	}
	printf("\n");
	return 0;
}

int gopherTransfer(const char *host, int port, const char *path, int wfd) {
	int rfd = gopherOpen(host, port, path);
	if (rfd < 0) return -1;
	int n;
	char buf[256];
	while ((n=read(rfd,buf,255)) > 0)
		if (write(wfd, buf, n) < 0) error(0, errno, "gopherTransfer");
	return 0;
}

Gopher *gopherFollowLink(Gopher *gopher, int n, const char *args) {
	if (n < 1) return gopher;
	Gopher *gp;
	for (gp = gopher; n && gp && gp->item; ++gp, --n);
	if (!gp || gp->item == 'i' || gp->item == '3') return gopher;
	if (!gp->host || !gp->path) return gopher;
	char *host = strdup(gp->host);
	int port = gp->port;
	char *path, item = gp->item;
	if (args && args[0]) asprintf(&path, "%s\t%s", gp->path, args);
	else path = strdup(gp->path);
	switch (item) {
		case '0':
			gopherTransfer(host, port, path, fileno(stdout));
			break;
		case '1': case '7':
			gopherFree(&gopher);
			gopher = gopherGet(host, port, path);
			break;
		case '2':
			// TODO: CSO
			break;
		case '8': case '+': case 'T':
			/* ignore these ... for now */
			break;
		case '3': case 'i':
			/* ignore these ... forever */
			break;
		default:
			/* all others, download */
			// TODO check args for filename, or make-temp file
			// open file for writing
			// gopherTransfer(host, port, path, wfd);
			// write filename to stdout w/ message
			break;
	}
	free(host);
	free(path);
	return gopher;
}

