#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define DEBUG

typedef struct CommandStruct
{
	const char* cmdName;		//	Command Name
	char options[26*2];			//	Command-Options that are passed to the command
	const char* fileName[1024];	//	Stores the file-names that are passed to the command
	int fileCount;				//	Total number of file-names that are passed as an argument to the command
}CommandStruct;

typedef struct WcResult
{
	int wordCount;
	int charCount;
	int lineCount;
	int byteCount;
	long longestLineLength;
}WcResult;

/*
 *	returns non-zero number if the first "count" number of characters are identical for both "a" and "b"
 *							else returns 0.
 */
int matchBegining(const char* a, const char *b, int count){
	int i = 0;
	while(i < count && a[i] == b[i]) i++;
	return i == count;
}

/*	
 *		Parses the command int the data-structure CommandStruct		
 */
CommandStruct ParseCommand(int argc, const char* argv[]){
	
	CommandStruct cmd;
	cmd.cmdName = argv[0];
	int fileCount = 0;
	strcpy(cmd.options, "");
	
	// Obtain the options and command parameters.... options may contain duplicate letters.
	for (int i = 1; i < argc; ++i){
		if( matchBegining( argv[i], "-", 1 ) )
			strcat(cmd.options, argv[i]+1);
		else
			cmd.fileName[fileCount++] = argv[i];
	}
	cmd.fileCount = fileCount;

	// Remove the duplicate options from the Options[] by using Counting-sort
	int aux[26*2] = {0};
	for (unsigned int i = 0; i < strlen(cmd.options); ++i){
		if(cmd.options[i] != '-'){
			if(cmd.options[i] >='A' && cmd.options[i] <='Z')
				aux[cmd.options[i] +26 - 'A'] = 1;
			else
				aux[cmd.options[i]- 'a'] = 1;
		}
	}

	int k = 0, j=0;
	for ( ; k < 26; ++k){	
		if(aux[k])
			cmd.options[j++] = (char)(k +'a');
	}
	for ( ; k < 26*2; ++k){	
		if(aux[k])
			cmd.options[j++] = (char)(k-26 +'A');
	}
	cmd.options[j] = '\0';
	
	return cmd;
}


/*
 *	Encodes all the options recieved by the command into a integer variable and returns it.
 *		bit-position:	4 3 2 1 0
 *		option-type :	l w m c L
 */
int selectSwitches(const char* options, const char* cmdName){
	int optionsCode; 	// lwmcL -> bit number

	if(strlen(options)==0)
		optionsCode = 26;

	for (unsigned int i = 0; i < strlen(options); ++i) {
		switch(options[i]){
			case 'l':	optionsCode = optionsCode | (1<<4); break;
			case 'w':	optionsCode = optionsCode | (1<<3); break;
			case 'm':	optionsCode = optionsCode | (1<<2); break;
			case 'c':	optionsCode = optionsCode | (1<<1); break;
			case 'L':	optionsCode = optionsCode | 1; 		break;
			default :	printf("%s: invalid option -- \'%c\'\n",cmdName, options[i] );
						exit(2);
		}
	}

	return optionsCode;
}


/*
 *	Returns the encapsulated result of "wc" command as a data-structure of type WcResult
 */
WcResult getWcResultForFile(const char* fileName, const int optionsCode){

	int fd = open(fileName, O_RDONLY);
	char buff, previousChar = ' ';
	long currentLineLength = 0;

	WcResult wc;
	wc.wordCount = wc.lineCount = wc.charCount = wc.byteCount = wc.longestLineLength = 0;

	while(read(fd, &buff, 1)){
		
		if ( optionsCode & 1 ){
			if( buff=='\t' )
				currentLineLength += 8;
			else if(buff != '\n')
				currentLineLength++;

			if( currentLineLength > wc.longestLineLength )
				wc.longestLineLength = currentLineLength;
			if(buff=='\n')
				currentLineLength = 0;
		}

		if ( (optionsCode & (1<<4)) && buff=='\n' ){
			// Read new-lines count
			wc.lineCount++;
		}
		if ( optionsCode & (1<<3) && (buff==' ' || buff=='\n'|| buff=='\t') && previousChar!=' ' && previousChar!='\n' && previousChar!='\t'){
			wc.wordCount++;
		}
		if ( optionsCode & (1<<2) ){
			wc.charCount++;
		}
		if ( optionsCode & (1<<1) ) {
			wc.byteCount++;
		}

		previousChar = buff;
	}
	if ( previousChar!=' ' && previousChar!='\n' && previousChar!='\t' )
			wc.wordCount++;

	close(fd);

	return wc;
}


/*
 *		Computes the output of wc command and prints the ouput to the screen.
 */
void getWcResult(int argc, const char* argv[]){

	CommandStruct cmd = ParseCommand(argc, argv);
	int optionsCode = selectSwitches(cmd.options, argv[0]);
	int lines=0, words=0, chars=0, bytes=0, longest=0;

	//	Compute the optput for wc on each file-name that is passed to the command and display it.
	for (int i = 0; i < cmd.fileCount; ++i) {
		WcResult wc = getWcResultForFile(cmd.fileName[i], optionsCode);
		if( optionsCode & (1<<4) ){
			printf(" %5d", wc.lineCount);
			lines += wc.lineCount;
		}
		if( optionsCode & (1<<3) ){
			printf(" %5d", wc.wordCount);
			words += wc.wordCount;
		}
		if( optionsCode & (1<<2) ){
			printf(" %5d", wc.charCount);
			chars += wc.charCount;
		}
		if( optionsCode & (1<<1) )	{
			printf(" %5d", wc.byteCount);
			bytes += wc.byteCount;
		}
		if( optionsCode &  1	 )	{
			printf(" %5ld", wc.longestLineLength);
			longest += wc.longestLineLength;
		}
		printf(" %s\n", cmd.fileName[i]);
	}

	//	Display the "total" line, at the end.
	if(cmd.fileCount > 1) {
		if( optionsCode & (1<<4) )	printf(" %5d", lines);
		if( optionsCode & (1<<3) )	printf(" %5d", words);
		if( optionsCode & (1<<2) )	printf(" %5d", words);
		if( optionsCode & (1<<1) )	printf(" %5d", bytes);
		if( optionsCode &  1	 )	printf(" %5d", longest);
	
		printf(" total\n");	
	}
}


int main(int argc, char const *argv[]){

	if(argc == 1){
		printf("Error: ");
		exit(1);
	}
 	getWcResult(argc, argv);

	return 0;
}// r -a -ld -re -e asf -dg asfag

