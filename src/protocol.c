
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <err.h>
#include <unistd.h>
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
	gopherHistory(host, port, path);
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
		gp->string = malloc(snprintf(0,0,"file://%sx",rpath));
		if (gp->string) sprintf(gp->string, "file://%s", rpath);
		free(rpath);
	}
	else {
		gp->string = malloc(snprintf(0,0,"gopher://%s:%d/%sx",gp->host,gp->port,gp->path));
		if (gp->string) sprintf(gp->string,"gopher://%s:%d/%s",gp->host,gp->port,gp->path);
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

Gopher *gopherHistory(const char *host, int port, const char *path) {
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

int gopherOpen(const char *host, int port, const char *path) {
	int fd;
	/* this is cool ... local files without a server! */
	if (strncmp(host,"file",5) == 0)
		return open(path, O_RDONLY);
	struct hostent *server;
	struct sockaddr_in serv_addr;
	if ((fd=socket(AF_INET, SOCK_STREAM, 0)) < 0)
		err(1, "gopherOpen");
	if (!(server=gethostbyname(host)))
		err(2, "gopherOpen (%s not known)", host);
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
	serv_addr.sin_port = htons(port);
	if (connect(fd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		err(3, "gopherOpen");
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
		if (write(wfd, buf, n) < 0) warn("gopherTransfer");
	return 0;
}

static char *implode(int argc, char *const *argv, char *sep) {
	static char ret[256];
	ret[0] = '\0';
	int i, len = 255, decrement = 0;
	for (i = 0; i < argc; ++i) {
		decrement = strlen(sep) + strlen(argv[i]);
		if (decrement > len) break; // TODO print error msg
		strcat(ret, sep);
		strcat(ret, argv[i]);
		len -= decrement;
	}
	return ret;
}

Gopher *gopherFollowLink(Gopher *gopher, int n, int argc, char *const *argv) {
	if (n < 1) return gopher;
	Gopher *gp;
	for (gp = gopher; n && gp && gp->item; ++gp, --n);
	if (!gp || gp->item == 'i' || gp->item == '3') return gopher;
	if (!gp->host || !gp->path) return gopher;
	char *host = strdup(gp->host);
	int port = gp->port, wfd = 0;
	char *path, *ptr, item = gp->item;
	char temp[] = "/tmp/goperXXXXXX";
	if (item == '7' && argv && argv[0]) {
		ptr = implode(argc, argv, " ");
		path = malloc(snprintf(0,0,"%s\t%sx",gp->path,ptr));
		if (path) sprintf(path, "%s\t%s", gp->path, ptr);
	}
	else {
		path = strdup(gp->path);
	}
	switch (item) {
		case '0':
			// TODO options for stdout?  Pagers, etc?
			gopherTransfer(host, port, path, fileno(stdout));
			break;
		case '1': case '7':
			gopherFree(&gopher);
			gopher = gopherGet(host, port, path);
			break;
		case '2':
			printf("Handler for CSO item-type is not complete\n");
			// TODO: CSO
			break;
		case '8': case '+': case 'T':
			printf("Handler for telnet item-types is not complete\n");
			// TODO: Telnet
			break;
		case '3': case 'i':
			/* ignore these ... forever */
			break;
		default:
			/* all others, download */
			if (argc && argv && argv[0]) { /* user supplied file name */
				if ((wfd=open(argv[0],O_WRONLY|O_CREAT|O_EXCL,0644)) < 0)
					warn("download");
				printf("download: saving to \"%s\" ... ", argv[0]);
			}
			else if ((ptr=strrchr(path,'/')) && ++ptr) { /* else take filename from server path */
				if ((wfd=open(ptr,O_WRONLY|O_CREAT|O_EXCL,0644)) < 0)
					warn("download");
				printf("download: saving to \"%s\" ... ", ptr);
			}
			else { /* else fallback to temp file */
				if ((wfd=mkstemp(temp)) < 0)
					warn("download");
				printf("download: saving to temporary file \"%s\" ... ", temp);
			}
			if (wfd > 0) {
				gopherTransfer(host, port, path, wfd);
				close(wfd);
				printf("done\n");
			}
			else {
				printf("failed\n");
			}
			break;
	}
	free(host);
	free(path);
	return gopher;
}

