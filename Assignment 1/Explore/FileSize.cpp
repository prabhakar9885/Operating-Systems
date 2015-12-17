#include "stdio.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
	struct stat fileInfo;
	stat(argv[1], &fileInfo);

	printf("Size: %ld", fileInfo.st_size);
	return 0;
}