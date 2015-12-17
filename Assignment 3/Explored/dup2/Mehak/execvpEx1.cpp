#include "sys/stat.h"
#include "sys/types.h"
#include "fcntl.h"
#include "unistd.h"

int main(int argc, char const *argv[])
{
	char *args[] = {"ls", "-l"};
	int fd = open("./output.txt", O_CREAT | O_TRUNC | 0666 );

	dup2(fd, 1);
	execvp(args[0], args);

	return 0;
}