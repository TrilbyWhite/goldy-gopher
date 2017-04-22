
#ifndef __GOPHER_H__
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

typedef struct Gopher { char item, *string, *host, *path; int port; } Gopher;

int gopherFree(Gopher **);
Gopher *gopherFollowLink(Gopher *, int, const char *);
Gopher *gopherGet(const char *, int, const char *);
FILE *gopherOpen(const char *, int, const char *);
int gopherShowMenu(Gopher *, int, int);

#endif /* __GOPHER_H__ */
