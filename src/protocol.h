
#ifndef __GOPHER_H__
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

#define GOPHER_NULL	0
#define GOPHER_START	2
#define GOPHER_QUIT	4

#define MAX_HIST		25

typedef unsigned char bool;

enum { false = 0, true = 1 };

typedef struct Gopher { char item, *string, *host, *path; int port; } Gopher;
typedef struct Color { char *item, *str; } Color;

int gopherFree(Gopher **);
Gopher *gopherFollowLink(Gopher *, int, int, char *const *);
Gopher *gopherGet(const char *, int, const char *);
Gopher *gopherHistory(const char *, int, const char *);
int gopherOpen(const char *, int, const char *);
int gopherShowMenu(Gopher *, int, int);

#endif /* __GOPHER_H__ */
