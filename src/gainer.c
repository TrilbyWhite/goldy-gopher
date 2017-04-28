
// TODO: organize these: I'm getting a migraine header-ache!
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <limits.h>
#include <errno.h>
#include <error.h>
#include <netdb.h>
#include <dirent.h>
#include <pwd.h>
#include <magic.h>
#include <grp.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

static int serve_dir(int, const char *);
static int serve_file(int, const char *);
static int service(int);
//static int daemonize();
static int init_socket();
static int drop_privileges();

static const char denied[] = "3Permission denied for requested resource\n.\n";
static const char notfound[] = "3Requested resource was not found\n.\n";
static const char srverror[] = "3Server error: failed to access requested resource\n.\n";
static const char _docroot[] = "/srv/gopher/";
static const char *docroot = _docroot;
static const char _host[] = "localhost";
static const char *host = _host;

static const char *type_map[] = {
	"h text/html",
	"0 text/",
	"g image/gif",
	"I image/",
	"s audio/",
	"d application/pdf",
	/* use 9="binary" as a catch all for anything else */
	"9 "
};

int serve_dir(int wfd, const char *path) {
	/* if "gophermap" exits, serve that instead of the dir listing: */
	if (chdir(path) < 0) {
		write(wfd, srverror, sizeof(srverror));
		error(0, errno, "chdir [path=\"%s\"", path);
		return 1;
	}
	if (access("gophermap", R_OK) == 0)
		return serve_file(wfd, "gophermap");
	/* no "gophermap", create a dir listing: */
	DIR *dir;
	struct dirent *de;
	struct stat info;
	char type;
	const char *mime, *match;
	magic_t m;
	int i;
	/* open dir, init libmagic, read magic db - return error on any failure: */
	if (!(dir=opendir(path)) || !(m=magic_open(MAGIC_MIME)) || magic_load(m, NULL)) {
		if (m) magic_close(m);
		write(wfd, srverror, sizeof(srverror));
		return 1;
	}
	// TODO include a ".." item?
	//  -> append "/.." to path and put through realpath?
	//  or strrchr for last "/" and trim?
	while ((de=readdir(dir))) {
		if (de->d_name[0] == '.') continue;
		if (stat(de->d_name, &info) < 0) continue;
		if (S_ISDIR(info.st_mode)) type = '1';
		else if (S_ISREG(info.st_mode)) {
			/* not a subdir, so get type from libmagic and type_map: */
			mime = magic_file(m, de->d_name);
			for (i = 0; i < sizeof(type_map)/sizeof(type_map[0]); ++i) {
				match = &type_map[i][2];
				type = type_map[i][0];
				if (strncmp(match, mime, strlen(match)) == 0) break;
			}
		}
		else continue;
		/* print gopher line: */
		dprintf(wfd, "%c%s\t%s/%s\t%s\t70\n", type, de->d_name, path, de->d_name, host);
	}
	magic_close(m);
	return 0;
}

int serve_file(int wfd, const char *path) {
	int rfd = open(path, O_RDONLY);
	if (!rfd) {
		write(wfd, srverror, sizeof(srverror));
		close(wfd);
		error(1, errno, "serve_file");
	}
	int n;
	char buf[256];
	while ((n=read(rfd,buf,255)) > 0)
		if (write(wfd, buf, n) < 0) error(1, errno, "serve_file");
	if (n < 0)
		error(0, errno, "serve_file");
	return 0;
}

int service(int fd) {
	int n;
	char path[256], *ptr;
	memset(path, 0, 256);
	path[0] = '/';
	if ((n=read(fd,path+1,254)) < 0)
		error(1, errno, "read");
	if ((ptr=strpbrk(path,"\t\r\n"))) *ptr = 0;
	struct stat info;
	if (stat(path, &info) != 0) {
		write(fd, notfound, sizeof(notfound));
		error(0, errno, "service");
	}
	else if (S_ISDIR(info.st_mode)) serve_dir(fd, path);
	else if (S_ISREG(info.st_mode)) serve_file(fd, path);
	else write(fd, denied, sizeof(denied));
	close(fd);
	return 0;
}


/*
 * Old habits die hard ... but this shouldn't be needed
int daemonize() {
	int pid;
	if ((pid=fork()) < 0) error(1, errno, "daemonize");
	if (pid) exit(0);
	if (setsid() < 0) error(1, errno, "daemonize");
	signal(SIGCHLD, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	if ((pid=fork()) < 0) error(1, errno, "daemonize");
	if (pid) exit(0);
	chdir(docroot);
	return 0;
}
*/

int init_socket() {
	int fd;
	struct hostent *server;
	struct sockaddr_in serv_addr;
	if ((fd=socket(AF_INET, SOCK_STREAM, 0)) < 0)
		error(1, errno, "socket");
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(70);
	if (bind(fd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
		error(1, errno, "bind");
	listen(fd, 5); // TODO why 5?
	return fd;
}

int drop_privileges() {
	// TODO create gopher user + group
#ifdef UNSAFE
#warning This program is NOT SAFE to use as it does not drop root privileges! You are testing unsafe code, at your own risk.
#else
#error This program is not yet safe to use as it does not drop root privileges! To test - at your own risk - define UNSAFE (e.g. gcc -DUNSAFE ...)
#endif
	/* the following code post-dates the above preprocessor error, but until this
	 * receives 3rd party review the error and warning will remain in place.
	 */
	struct passwd *pw;
	if (!(pw=getpwnam("http"))) error(1, errno, "drop 1");
	if (chdir(docroot) != 0) error(1, errno, "drop 5");
	if (chroot(docroot)) error(1, errno, "drop 6");
	if (setgid(pw->pw_gid) != 0) error(1, errno, "drop 2");
	if (initgroups(pw->pw_name, pw->pw_gid) != 0) error(1, errno, "drop 4");
	if (setuid(pw->pw_uid) != 0) error(1, errno, "drop 3");
	/*
	if (	!(pw=getpwnam("http")) ||
			(setgid(pw->pw_gid) != 0) ||
			(setuid(pw->pw_uid) != 0) ||
			(initgroups(pw->pw_name, pw->pw_gid) != 0) ||
			(chdir(docroot) != 0) ||
			(chroot(docroot)) )
		error(1, errno, "drop_privieleges");
	*/
}


static int running = 1;
static void signal_handler(int sig) {
	running = 0;
}

int main(int argc, const char **argv) {
	/* TODO make args[] more flexible ... */
//	if (argc != 3) return 1;
//	host = argv[1];
//	docroot = argv[2];
	if (getuid() != 0)
		error(1, 0, "must be run as root");
	//daemonize();
	int fd = init_socket();
	drop_privileges();
	/* load signal handler */
	struct sigaction sa;
	sa.sa_handler = signal_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	//if (sigaction(SIGINT, &sa, NULL) < 0) error(1, errno, "signal_handler");
	//if (sigaction(SIGTERM, &sa, NULL) < 0) error(1, errno, "signal_handler");
	/* listen for connections */
	int client, pid;
	while (running) {
		/* fork to service each incoming connection: */
		if ((client=accept(fd,NULL,NULL)) < 0) error(1, errno, "accept");
		if ((pid=fork()) < 0) error(1, errno, "fork");
		if (pid == 0) {
			close(fd);
			service(client);
			exit(0);
		}
		else {
			close(client);
		}
	}
	close(fd);
	return 0;
}

