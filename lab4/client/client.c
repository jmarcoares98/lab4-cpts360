// Travis Cripe
// Marco Ares

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


#define MAX			256
#define PORT		1234

typedef struct {
	char* name;
	char* value;
} ENTRY;

ENTRY entry[MAX];

char line[MAX], ans[MAX];
int n;
char buf[MAX];

int cfd, sport;
char* sip;
struct sockaddr_in saddr;
struct hostent* hp;

char* t1 = "xwrxwrxwr-------";
char* t2 = "----------------";

char* filename, path[1024];

int client_init(char* argv[])
{
	printf("1. get server info\n");
	hp = gethostbyname(argv[1]);
	sip = inet_ntoa(*((struct in_addr*)hp->h_addr));
	sport = PORT;

	printf("hostname= %s IP= %s port= %d\n", hp->h_name, sip, sport);

	printf("2. create a TCP socket\n");
	cfd = socket(AF_INET, SOCK_STREAM, 0);
	if (cfd < 0) {
		printf("socket creation failed\n");
		exit(1);
	}
	printf("3. fill in [localhost IP port=1234] as server address\n");
	//bzero(&saddr, sizeof(saddr));
	saddr.sin_family = AF_INET;
	//saddr.sin_addr.s_addr = sip;
	inet_pton(AF_INET, sip, &saddr.sin_addr);
	saddr.sin_port = htons(sport);


	printf("3. connect client socket to server\n");
	int r = connect(cfd, (struct sockaddr*) & saddr, sizeof(saddr));
	if (r < 0) {
		printf("connection to server failed\n");
		exit(2);
	}

	printf("hostname= %s IP= %s port= %d\n", hp->h_name, sip, sport);
}

int main(int argc, char* argv[])
{
	if (argc < 3) {
		printf("Usage : client ServerName SeverPort\n");
		exit(1);
	}

	client_init(argv);

	printf("*************  processing loop  **************\n");
	while (1) {
		printf("**********************************************\n");
		printf("menu:\tget put ls  cd  pwd  mkdir  rmdir  rm\n");
		printf("\tlcat lls lcd lpwd lmkdir lrmdir lrm\n");
		printf("**********************************************\n");
		printf("input a line : ");
		bzero(line, MAX);                // zero out line[ ]
		fgets(line, MAX, stdin);         // get a line (end with \n) from stdin

		line[strlen(line) - 1] = 0;        // kill \n at end
		if (line[0] == 0)                  // exit if NULL line
			exit(0);

		// Send ENTIRE line to server
		n = write(cfd, line, MAX);
		printf("client: wrote n=%d bytes; line=(%s)\n", n, line);

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
		get_localcmd(argc, *argv);

		// Read a line from sock and show it
		n = read(cfd, ans, MAX);
		printf("client: read  n=%d bytes; echo=(%s)\n", n, ans);

	}
}

int get_localcmd(int argc, char* argv[])
{
	int r;
	struct stat mystat, * sp;
	char path[1024], cwd[256];
	char* pathname;

	if (strcmp(entry[0].value, "lmkdir") == 0)
	{
		r = mkdir(entry[1].value, 0755);
	}

	else if (strcmp(entry[0].value, "lrmdir") == 0)
	{
		r = rmdir(entry[1].value);
	}

	else if (strcmp(entry[0].value, "lrm") == 0)
	{
		r = unlink(entry[1].value);
	}

	else if (strcmp(entry[0].value, "lls") == 0)
	{
		sp = &mystat;
		int r;
		pathname = "./";

		if (r = lstat(pathname, sp) < 0)
		{
			printf("no such file %s \n", pathname);
			exit(1);
		}

		printf("test\n");

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

	else if (strcmp(entry[0].value, "lcd") == 0)
	{
		sp = &mystat;
		int r;
		pathname = "./";

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

	else if (strcmp(entry[0].value, "lpwd") == 0)
	{
		char cwd[PATH_MAX];
		if (getcwd(cwd, sizeof(cwd)) != NULL)
		{
			printf("Current working dir: %s\n", cwd);
		}
	}

	else if (strcmp(entry[0].value, "lcat") == 0)
	{
		FILE* fp;
		int c;
		fp = fopen(entry[1].value, "r");
		if (fp == NULL)
			printf("cannot cat file\n");

		/*while ((c = fgetc(fp) != EOF)) {
			putchar(c);
		}*/
		printf("%s\n", buf);
		fclose(fp);
		printf("\n");

	}

	if (strcmp(entry[0].value, "get") == 0)
	{
		printf("testing for when size get thrown\n");
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

	if (strcmp(entry[0].value, "put") == 0)
	{
		int total, n;
		char buf[MAX];

		pathname = entry[1].value;

		printf("1. stat %s \n", pathname);
		sp = &mystat;
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

		int fp = open(pathname, O_RDONLY);
		if (fp < 0) {
			printf("cant open %s for READ\n", pathname);
			sprintf(ans, "BAD server cant open %s for READ", pathname);
			write(cfd, ans, MAX);
		}

		total = 0;
		while (n = read(fp, buf, MAX)) {
			n = write(cfd, buf, MAX);
			total += n;
			printf("n= %d total= %d\n", n, total);
		}
		close(fp);
	}

	if (strcmp(entry[0].value, "lquit") == 0)
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
