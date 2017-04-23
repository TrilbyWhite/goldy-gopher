
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>


static void die(const char *where) {
	perror(where);
	exit(1);
}

const char *msg = "iWelcome to gopher!\n.\n";
static void handle(int fd) {
	int n;
	char buf[1024];
	memset(buf, 0, 1024);
	if ((n=read(fd,buf,1023)) < 0) die("read");
	if ((n=write(fd,msg,strlen(msg))) < 0) die("write");
}

int main(int argc, const char **argv) {
	int fd, client, pid;
	struct hostent *server;
	struct sockaddr_in serv_addr;
	if ((fd=socket(AF_INET, SOCK_STREAM, 0)) < 0) die("socket");
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(70);
	if (bind(fd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
		perror("bind");
		exit(1);
	}
	listen(fd, 5); // TODO why 5?

	for (;;) {
		if ((client=accept(fd,NULL,NULL)) < 0) die("accept");
		if ((pid=fork()) < 0) die("fork");
		if (pid == 0) {
			close(fd);
			handle(client);
			exit(0);
		}
		else {
			close(client);
		}
	}
	close(fd);
	return 0;
}

