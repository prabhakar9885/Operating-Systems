/*	Question 1 : Fork [ Marks : 20 ] 
	 
	Consider a 5*3 array ­  
	int arr[5][3] = { 
	 
	 {1, 2, 3}, 
	 
	 {4, 5, 6}, 
	 
	 {7, 8, 9}, 
	 
	 {10, 11, 12}, 
	 
	 {13, 14, 15} 
	 
	 }; 
	 
	 
	It has 5 rows. You need to find summation of all the numbers in a given row. 
	However for doing so, you will create 5 children using fork().  
	 
	Each child will find the summation of a row & print 3 things ­  
	a. its own PID 
	b. the row number 
	c. the summation of the row.  
	 
	This is the required output ­  
	>./a.out 
	Child pid : 13488, Sum of row : 0 is : 6 
	Child pid : 13489, Sum of row : 1 is : 15 
	Child pid : 13491, Sum of row : 3 is : 33 
	Child pid : 13490, Sum of row : 2 is : 24 
	Child pid : 13492, Sum of row : 4 is : 42 
	 
	NOTE#1 The row number will not necessarily come in order. 
	NOTE#2 Each child pid has to be unique. 
	 
	You have to solve this problem for any given matrix of any size. 
	You need to create row­count number of children, each of them printing every row’s sum. 
	 
	Input Format :  
	First line contains 2 numbers : row­count & column­count. 
	This will be followed by row­count number of lines, each containing column­count number of 
	numbers, separated by single space. 
	 
	Example :  
	5 3 
	1 2 3 
	4 5 6 
	7 8 9 
	10 11 12 
	13 14 15 */


#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/stat.h>

int main(int argc, char const *argv[])
{
	int rowCount, colCount;
	pid_t pid;
	scanf("%d%d", &rowCount, &colCount);

	int matrix[rowCount][colCount];
	
	for (int rowIndex = 0; rowIndex < rowCount; ++rowIndex)
	{
		// matrix[rowIndex] = (int*)malloc(sizeof(int)*colCount);
		for (int colIndex = 0; colIndex < colCount; ++colIndex)
			scanf("%d", &matrix[rowIndex][colIndex]);
	}

	for (int rowIndex = 0; rowIndex < rowCount; ++rowIndex)
	{
		pid = fork();
		if(pid == -1)
		{
			printf("Fork error\n");
			exit(1);
		}
		if(pid == 0)
		{
			int sum = 0;
			for (int colIndex = 0; colIndex < colCount; ++colIndex)
				sum += matrix[rowIndex][colIndex];
			printf("Child pid : %u, Sum of row : %d is : %d \n", getpid(), rowIndex+1, sum);
			exit(0);
		}
//		wait(0);

	}

	wait(0);
	return 0;
}
