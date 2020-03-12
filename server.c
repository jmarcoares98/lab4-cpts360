#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <libgen.h>;

#define MAX				256
#define SERVER_HOST		"localhost"
#define SERVER_IP		"127.0.0.1"
#define SERVER_PORT		1234

char* getcwd(char* buf, size_t size);

typedef struct {
	char* name;
	char* value;
} ENTRY;

ENTRY entry[MAX];

char* t1 = "xwrxwrxwr-------";
char* t2 = "----------------";
char line[MAX];
int n, sfd, cfd, len, fp, serverPort, length;
struct sockaddr_in saddr, caddr, naddr;
struct hostent* hp;

char* filename, cwd[256], path[1024], ans[MAX];

int server_init(char* myname)
{
	printf("1. get host name\n");
	hp = gethostbyname(myname);
	if (hp == 0)
		printf("unknown host %s\n", myname);

	printf("2. create a TCP socket\n");
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd < 0) {
		printf("socket creation failed\n");
		exit(1);
	}

	printf("3. fill in [localhost IP port=1234] as server address\n");
	bzero(&saddr, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	saddr.sin_port = htons(SERVER_PORT);

	printf("4. bind socket with server address\n");
	if ((bind(sfd, (struct sockaddr*) & saddr, sizeof(saddr))) != 0) {
		printf("socket bind failed\n");
		exit(2);
	}

	printf("5. get socket addr to show port number assigned by kernel\n");
	length = sizeof(naddr);
	int r = getsockname(sfd, (struct sockaddr*) & naddr, &length);
	if (r < 0) {
		printf("did not get sock name\n");
		exit(3);
	}
		
	printf("6. server listens\n");
	if ((listen(sfd, 5)) != 0) {
		printf("Listen failed\n");
		exit(4);
	}

	char* IP;
	IP = inet_ntoa(*((struct in_addr*)hp->h_addr));
	printf("hostname= %s IP= %s port= %d\n", hp->h_name, IP, ntohs(naddr.sin_port));
}

int main(int argc, char* argv[])
{
	char* hostname;
	if (argc < 2) 
		hostname = SERVER_HOST;
	else
		hostname = argv[1];

	server_init(hostname);

	len = sizeof(caddr);

	while (1) {
		printf("server accepting connection\n");

		cfd = accept(sfd, (struct sockaddr*) & caddr, (socklen_t*)&len);

		if (cfd < 0) {
			printf("server acccept failed\n");
			exit(4);
		}

		printf("server acccepted a client connection\n");

		// server Processing loop
		while (1) {
			printf("server: ready for next request\n");
			n = read(cfd, line, MAX);
			char* test = "";
			test = strtok(line, " ");
			entry[0].value = test;

			printf("name: %s\n", entry[0].value);
            sprintf(ans, "BAD server cant stat %s", entry[0].value);

			test = strtok(NULL, "\0");
			printf("inputting into entry name\n");
			entry[1].value = test;

			//strcpy(test, entry[0].value);
			strtok(0, " ");
			printf("value: %s\n", entry[1].value);
			get_cmd(argc, *argv);

			if (n == 0) {
				printf("server: client died, server loops\n");
				close(cfd);
				break;
			}

			// show the line contents
			printf("server: read  n=%d bytes; command = %s Filename = %s\n", n, entry[0].value, entry[1].value);
			
			// send the echo line to client
			n = write(cfd, line, MAX);
			printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, entry[1].value);
		}
	}
}

int get_cmd(int argc, char* argv[])
{
	int r;
	struct stat mystat, * sp;
	char* pathname;

	if (strcmp(entry[0].value, "mkdir") == 0)
	{
		r = mkdir(entry[1].value, 0755);
		printf("hola\n");
	}

	else if (strcmp(entry[0].value, "rmdir") == 0)
	{
		r = rmdir(entry[1].value);
	}

	else if (strcmp(entry[0].value, "rm") == 0)
	{
		r = unlink(entry[1].value);
	}

	else if (strcmp(entry[0].value, "ls") == 0)
	{
		sp = &mystat;
		int r;
		pathname = "./";

		if (argc > 1)
		{
			pathname = argv[1];
		}

		if (r = lstat(pathname, sp) < 0)
		{
			printf("no such file %s \n", pathname);
			exit(1);
		}

		strcpy(path, pathname);
		if (path[0] != '/')
		{
			getcwd(cwd, 256);
			strcpy(path, cwd);
			strcat(path, "/");
			strcat(path, pathname);

			if (entry[1].value != NULL)
			{
				strcat(path, "/");
				strcat(path, entry[1].value);
			}
		}

		if (S_ISDIR(sp->st_mode))
		{
			ls_dir(path);
		}

		else
		{
			ls_file(path);
		}
	}

	else if (strcmp(entry[0].value, "cd") == 0)
	{
		sp = &mystat;
		int r;
		pathname = "./";

		if (argc > 1)
		{
			pathname = argv[1];
		}

		if (r = lstat(pathname, sp) < 0)
		{
			printf("no such file %s \n", pathname);
			exit(1);
		}

		strcpy(path, pathname);

		if (path[0] != '/')
		{
			getcwd(cwd, 256);
			strcpy(path, cwd);
			strcat(path, "/");
			strcat(path, pathname);
			strcat(path, "/");
			strcat(path, entry[1].value);
		}

		chdir(path);
	}

	else if (strcmp(entry[0].value, "pwd") == 0)
	{
		char cwd[PATH_MAX];
		if (getcwd(cwd, sizeof(cwd)) != NULL)
		{
			printf("Current working dir: %s\n", cwd);
		}
	}

	else if (strcmp(entry[0].value, "get") == 0) 
	{
		int total, n;
		char buf[MAX];
		
		pathname = entry[1].value;

		printf("server get %s\n", pathname);
		
		printf("1. stat %s \n", pathname);
		sp = &mystat;
		stat(pathname, sp);

		r = lstat(pathname, sp);
		if (r < 0) {
			printf("server: cant stat %s\n", pathname);
			sprintf(ans, "BAD server cant stat %s", pathname);
			write(cfd, ans, MAX);
		}

		//stat ok
		if (!S_ISREG(sp->st_mode)) {
			printf("%s is not REG file\n", pathname);
			sprintf(ans, "BAD %s is nor REG file", pathname);
			write(cfd, ans, MAX);
		}

		fp = open(pathname, O_RDONLY);
		if (fp < 0) {
			printf("cant open %s for READ\n", pathname);
			sprintf(ans, "BAD server cant open %s for READ", pathname);
			write(cfd, ans, MAX);
		}

		// reply to client to OK size
		printf("2. send OK  %d to client\n", sp->st_size);
		sprintf(ans, "%d", sp->st_size);
		write(cfd, ans, MAX);

		total = 0;

		printf("3. open %s for READ\n", pathname);
		while (n = read(fp, buf, MAX)) {
			n = write(cfd, buf, MAX);
			total += n;
			printf("n= %d total= %d\n", n, total);
		}
		close(fp);

		printf("sent %d bytes\n", total);
	}

	else if (strcmp(entry[0].value, "put") == 0)
	{
		int count = 0;
		char gbuf[MAX];
		int size = atoi(ans);
		int fp = open(entry[1].value, O_WRONLY | O_CREAT);
		read(cfd, gbuf, MAX);

		while (count < size)
		{
			n = read(cfd, gbuf, MAX);
			count += n;
			write(fp, gbuf, n);
		}

		close(fp);
	}

    else if (strcmp(entry[0].value, "quit") == 0)
    {
        exit(1);
    }
}

int ls_file(char* fname)
{
	struct stat fstat, * sp;
	int r, i;
	char ftime[64];
	sp = &fstat;

	if ((r = lstat(fname, &fstat)) < 0)
	{
		printf("can’t stat %s\n", fname);
		exit(1);
	}

	else if ((sp->st_mode & 0xF000) == 0x8000) // if (S_ISREG())
	{
		printf("%c", '-');
	}

	else if ((sp->st_mode & 0xF000) == 0x4000) // if (S_ISDIR())
	{
		printf("%c", 'd');
	}

	else if ((sp->st_mode & 0xF000) == 0xA000) // if (S_ISLNK())
	{
		printf("%c", 'l');
	}

	for (i = 8; i >= 0; i--)
	{
		if (sp->st_mode & (1 << i)) // print r|w|x
		{
			printf("%c", t1[i]);
		}

		else
		{
			printf("%c", t2[i]); // or print -
		}
	}
	printf("%4d ", sp->st_nlink); // link count
	printf("%4d ", sp->st_gid);  // print time
	printf("%4d ", sp->st_uid);  // gid
	printf("%8d ", sp->st_size); // uid

	strcpy(ftime, ctime(&sp->st_ctime)); // print time in calendar form
	ftime[strlen(ftime) - 1] = 0; // kill \n at end

	printf("%s ", ftime); // print name
	printf("%s", basename(fname)); // print file basename // print -> linkname if symbolic file

	if ((sp->st_mode & 0xF000) == 0xA000)
	{
		char* linkname = "";
		r = readlink(fname, linkname, 16);
		printf(" -> %s", linkname);
	}
	printf("\n");
}

int ls_dir(char* dname)
{
	struct dirent* ep;
	char test[128];
	DIR* dp = opendir(dname);

	while (ep = readdir(dp))
	{
		strcpy(test, dname);
		strcat(test, "/");
		strcat(test, ep->d_name);
		ls_file(test);
		strcpy(test, "/0");
	
	}
}
