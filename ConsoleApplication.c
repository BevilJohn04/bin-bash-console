/* Programmer: John A. Bevil
* Date start: 4/12/25
* Date End: 4/23/25
* Program: A simple shell meant to operate in UNIX systems
*/
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "minishell.h"



/*-----------------------------------------------------------|
| FUNCTIONS													 |
|-----------------------------------------------------------*/
char* lookupPath(char **, char **);
int parseCommand(char*, struct command_t*);
int parsePath(char **);
void printPrompt();
void readCommand(char*);
/*-----------------------------------------------------------|
| MAIN METHOD												 |
|-----------------------------------------------------------*/

int main() {
	//Shell initialization
	char* pathv[MAX_PATH_LEN];
	char commandLine[LINE_LEN + 1];
	struct command_t command = { NULL, 0, { NULL } };
	int CPID;
	parsePath(pathv);

	while (1) {
		//Print's hostname and ">"
		printPrompt();

		//Read the command line and parse it
		readCommand(commandLine);
		parseCommand(commandLine, &command);

		//Get the full pathname for the file
		command.name = lookupPath(command.argv, pathv);
		if (command.name == NULL) {
			//Error Report
			continue;
		}

		//Command to exit miniShell and break loop for termination.
		if (!strcmp(command.name, "exit")) {
			break;
		}

		//Create child and wait
		if (fork() != 0) {
			wait(NULL);
		}
		else {
			//Child executes command name with args
			execvp(command.name, command.argv);
		}

		
		free(command.name);
	}

	//Shell termination
	printf("Terminating. . .\n");
	return 0;

}

/*-----------------------------------------------------------|
| FUNCTION DEFINITIONS										 |
|-----------------------------------------------------------*/

//------------------------------------------------------------
char* lookupPath(char** argv, char** dir) {
	/*This function searches the directories identified by the dir
	argument to see if argv[0] (the file name) appears there.
	Allocates a new string, place the full path name in it, then returns the string.
	*/
	char* result;
	char pName[MAX_PATH_LEN];
	

	result = (char*)malloc(sizeof(char) * MAX_PATH_LEN);
	//Checks to see if the file name is already an absolute path name
	if (*argv[0] == '/') {
		strcpy(result, argv[0]);
		return result;
	}

	//Look in PATH directories
	//Use access() to see if the file is in a dir.
	if (!strcmp(argv[0], "exit")) {
		strcpy(result, argv[0]);
		return result;
	}
	for (int i = 0; i < MAX_PATHS; i++) {
		if (dir[i] == NULL) break;

		strcpy(pName, dir[i]);
		strcat(pName, "/");
		strcat(pName, argv[0]);

		if (access(pName, F_OK | X_OK) != -1) {
			strcpy(result, pName);
			return result;
		}
	}

	//File name not found in any path variable
	fprintf(stderr, "%s: command not found\n", argv[0]);
	return NULL;
}
//------------------------------------------------------------
int parseCommand(char* cmdLine, struct command_t* command) {
	/*Determines command name and constructs the parameter list.
	This function will build argv[] and set the argn value.
	argv[] will be the array of obtained "tokens" from user input, ending with NULL and
	argn being the number of tokens*/
	int argn = 0;
	char * buffer;
	while((buffer = strsep(&cmdLine,WHITESPACE))!=NULL){
		if(strlen(buffer) > 0){
			command->argv[argn++] = buffer;
		}
	}
	command->argv[argn--]=NULL;
	command->argc = argn;
	return 0;
}
//------------------------------------------------------------
int parsePath(char* dirs[]) {
	/*This function reads the PATH variable for this
	enviorment, then builds an array of the directories
	in dirs[].*/
	char* pathEnvVar;
	char* thePath;
	int parsedTokens;
	char* token = NULL;

	//Frees the current directory array.
	for (int i = 0; i < MAX_ARGS; i++) {
		dirs[i] = NULL;
	}

	//Gets the "PATH" variable and places it into thePath for parsing.
	pathEnvVar = (char*)getenv("PATH");
	thePath = (char*)malloc(strlen(pathEnvVar) + 1);
	strcpy(thePath, pathEnvVar);


	//Tokenizes each parsed string using ":" as a delimeter and inserts each token into dirs[i]
	for (parsedTokens = 0, token = strsep(&thePath, ":"); token; token = strsep(&thePath, ":")) {
		if (parsedTokens > MAX_ARGS) {
			printf("Too many ARGS.");
			return(1);
		}

		dirs[parsedTokens++] = token;
	}
	return 0;
}
//------------------------------------------------------------
void printPrompt() {
	//Fetches the machine name and displays it with ">" to prompt input.
	char promptString[100];
	gethostname(promptString, 100);
	printf("%s >", promptString);
	
}
//------------------------------------------------------------
void readCommand(char* buffer) {
	//Collects user input from STDIN and places it into the buffer.
	fgets(buffer,LINE_LEN,stdin);
}
//--------------------------------------------------------------
