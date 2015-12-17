#include <fcntl.h>
#include "stdio.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "string.h"

int main(int argc, char const *argv[])
{
	int fd = creat("testWrite", 0644 );
	write(fd, "Hello Wordl", strlen("Hello Wordl"));
	write(fd, "Hey", strlen("Hey"));
	close(fd);
	return 0;
}