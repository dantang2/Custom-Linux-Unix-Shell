// Shell starter file
// You may make any changes to any part of this file.

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define COMMAND_LENGTH 1024
#define NUM_TOKENS (COMMAND_LENGTH / 2 + 1)
#define HISTORY_DEPTH 1000


//static char buffer[BUFFER_SIZE];

_Bool signalInterrupted = false;

_Bool skip_loop = false;



/**
 * Command Input and Processing
 */

/*
 * Tokenize the string in 'buff' into 'tokens'.
 * buff: Character array containing string to tokenize.
 *       Will be modified: all whitespace replaced with '\0'
 * tokens: array of pointers of size at least COMMAND_LENGTH/2 + 1.
 *       Will be modified so tokens[i] points to the i'th token
 *       in the string buff. All returned tokens will be non-empty.
 *       NOTE: pointers in tokens[] will all point into buff!
 *       Ends with a null pointer.
 * returns: number of tokens.
 */

void handle_SIGINT()
{
	signalInterrupted = true;
}


void addToHistoryList(char *history[], char*tokens[], int *historyCounter)
{
	for(int i=0; tokens[i]!=NULL;i++)
	{	
		if(i==0)
		{	history[*historyCounter] = (char*)malloc(COMMAND_LENGTH*sizeof(char));
			strcpy(history[*historyCounter],tokens[0]);
		}

		else
		{	
			strcat(history[*historyCounter], " ");
			strcat(history[*historyCounter], tokens[i]);

		}
	}

	*historyCounter = *historyCounter + 1;
}

void printHistoryList(char *history[], int historyCounter)
{
	int j = 0;
	if(historyCounter<10)
		j = 0;
	else
		j = historyCounter - 10;

	char tmp[sizeof(int)];
	for (int i=j; i<historyCounter; i++)
	{	
		sprintf(tmp, "%d", (i+1));
		
		write(STDOUT_FILENO, tmp, strlen(tmp));
		write(STDOUT_FILENO, "\t", strlen("\t"));
		write(STDOUT_FILENO, history[i], strlen(history[i]));
		write(STDOUT_FILENO, "\n", strlen("\n"));
	}
}

int tokenize_command(char *buff, char *tokens[])
{
	int token_count = 0;
	_Bool in_token = false;
	int num_chars = strnlen(buff, COMMAND_LENGTH);
	for (int i = 0; i < num_chars; i++) {
		switch (buff[i]) {
		// Handle token delimiters (ends):
		case ' ':
		case '\t':
		case '\n':
			buff[i] = '\0';
			in_token = false;
			break;

		// Handle other characters (may be start)
		default:
			if (!in_token) {
				tokens[token_count] = &buff[i];
				token_count++;
				in_token = true;
			}
		}
	}
	tokens[token_count] = NULL;
	return token_count;
}

/**
 * Read a command from the keyboard into the buffer 'buff' and tokenize it
 * such that 'tokens[i]' points into 'buff' to the i'th token in the command.
 * buff: Buffer allocated by the calling code. Must be at least
 *       COMMAND_LENGTH bytes long.
 * tokens[]: Array of character pointers which point into 'buff'. Must be at
 *       least NUM_TOKENS long. Will strip out up to one final '&' token.
 *       tokens will be NULL terminated (a NULL pointer indicates end of tokens).
 * in_background: pointer to a boolean variable. Set to true if user entered
 *       an & as their last token; otherwise set to false.
 */

void read_command(char *buff, char *tokens[], _Bool *in_background)
{
	*in_background = false;

	// Read input
	int length = read(STDIN_FILENO, buff, COMMAND_LENGTH-1);


	if ((length < 0) && (errno !=EINTR ))
	 {
		perror("Unable to read command from keyboard. Terminating.\n");
		exit(-1);
	}

	if(errno==EINTR)
	{
		length=0;
	}


	// Null terminate and strip \n.
	buff[length] = '\0';
	if (buff[strlen(buff) - 1] == '\n') {
		buff[strlen(buff) - 1] = '\0';
	}


	// Tokenize (saving original command string)
	int token_count = tokenize_command(buff, tokens);
	if (token_count == 0) {
		return;
	}

	// Extract if running in background:
	if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
		*in_background = true;
		
	}
}

void previousCommand(char *history[], char *tokens[], int historyCounter)
{
	if(historyCounter>0)
	{	
		tokenize_command(history[historyCounter-1],tokens);

		for(int i = 0 ; tokens[i]!=NULL; i++)
		{
			if(i==0)
			{
				history[historyCounter-1] = (char*)malloc(COMMAND_LENGTH*sizeof(char));
				strcpy(history[historyCounter-1], tokens[0]);
			}
			else
			{
				strcat(history[historyCounter-1]," ");
				strcat(history[historyCounter-1],tokens[i]);
			}
		}
		write(STDOUT_FILENO, history[historyCounter-1], strlen(history[historyCounter-1]));
		write(STDOUT_FILENO, "\n", strlen("\n"));

		//free(history[historyCounter-1]);
	}

	else
	{
		write(STDOUT_FILENO, "ERROR: No entries to be found\n",strlen("ERROR: No entries to be found\n"));
		skip_loop = true;
	}
}

void chooseOldCommand(char *history[], char *tokens[], int historyCounter, int number, char tmp[])
{
	if(historyCounter>0)
	{	
		number = 0;
		for(int i = 1; (tokens[0][i])!= '\0'; i++)
		{	
			tmp[i] = tokens[0][i];
			if(i==1)
			{
				number = number + atoi(tmp+i);
			}

			else
			{
				number = number*10 + atoi(tmp+i); 
			}

			tmp[i]='\0';
		}

		if((number-1)<historyCounter&&(number-1)>=0)
		{	
			tokenize_command(history[number-1],tokens);

			for(int i = 0 ; tokens[i]!=NULL; i++)
			{
				if(i==0)
				{
					history[number-1] = (char*)malloc(COMMAND_LENGTH*sizeof(char));
					strcpy(history[number-1], tokens[0]);
				}

				else
				{
					strcat(history[number-1]," ");
					strcat(history[number-1],tokens[i]);
				}


			}

			write(STDOUT_FILENO, history[number-1], strlen(history[number-1]));
			write(STDOUT_FILENO, "\n", strlen("\n"));

			//free(history[number-1]);
				
		}

		else if((number-1)<0)
		{
			write(STDOUT_FILENO, "SHELL: Unknown history command. \n",strlen("SHELL: Unknown history command. \n"));
			skip_loop = true;
		}

		else
		{
			write(STDOUT_FILENO, "ERROR: This entry does not exist.\n",strlen("ERROR: This entry does not exist.\n"));
			skip_loop = true;
		}

	}

	else
	{
		write(STDOUT_FILENO, "ERROR: No entries to be found.\n",strlen("ERROR: No entries to be found.\n"));
		skip_loop = true;
	}

	
}




/**
 * Main and Execute Commands
 */
int main(int argc, char* argv[])
{
	system("clear");
	printf("------ WELCOME TO MY SHELL ------\n");

	char input_buffer[COMMAND_LENGTH];
	char *tokens[NUM_TOKENS];

	_Bool loop = true;


	char **history = (char**)malloc(HISTORY_DEPTH*sizeof(char*));
	int historyCounter = 0;

	char tmp[COMMAND_LENGTH]; //temporary character array

	struct sigaction handler;
	handler.sa_handler = handle_SIGINT;
	sigaction(SIGINT, &handler, NULL);


	while (loop)
	{

		// Get command
		// Use write because we need to use read() to work with
		// signals, and read() is incompatible with printf().

		skip_loop = false; // used to skip loop 

		if(signalInterrupted)
		{
			write(STDOUT_FILENO, "\n", strlen("\n"));
			printHistoryList(history, historyCounter);
			signalInterrupted = false;
		}


		
		else
		{
			char cwd[COMMAND_LENGTH]; 
			getcwd(cwd, sizeof(cwd)); //gets current working directory

			write(STDOUT_FILENO, cwd, strlen(cwd)); //print command-prompt with current working directory
			write(STDOUT_FILENO, "> ", strlen("> "));

			_Bool in_background = false;
			read_command(input_buffer, tokens, &in_background); //backspace should delete previous entries


			while(waitpid(-1, NULL, WNOHANG)>0) // wait for zombie processes
			{
				//do nothing
			}
			if(tokens[0]==NULL)
			{
				//do nothing if user presses enter on an empty line
			}

			else
			{	


				if(strcmp(tokens[0],"!!")==0)
				{
					previousCommand(history,tokens, historyCounter); 
					if(skip_loop)
						continue;
				}


				int number = 0; //this variable will be used to calculate position of command in the history list
				tmp[0] = tokens[0][0];
				if(tmp[0]=='!'&&tokens[0][1]!='!')
				{
					chooseOldCommand(history,tokens,historyCounter,number, tmp);
					if(skip_loop)
						continue;	
				}





				addToHistoryList(history, tokens, &historyCounter); //adds command to history of commands

				for(int i = 0; tokens[i]!=NULL;i++) //check if tokens contain "&" 
				{
					if(strcmp(tokens[i],"&")==0)
					{
						tokens[i] = 0;
						in_background = true;
					}
				}


				if(strcmp(tokens[0], "exit")==0) 
				{
					write(STDOUT_FILENO,"------ CLOSING SHELL ------\n", strlen("------ CLOSING SHELL ------\n"));
					for(int i = 0; i<historyCounter; i++)
					{
						free(history[i]);
					}
					free(history);
					exit(0);
					
				}

				else if(strcmp(tokens[0], "history")==0)
				{
					if(historyCounter>0)
						printHistoryList(history, historyCounter);
					else
						write(STDOUT_FILENO, "ERROR: No entries found\n",strlen("ERROR: No entries found\n"));
				}

				else if(strcmp(tokens[0], "pwd")==0)
				{
					write(STDOUT_FILENO, "Current working directory: ", strlen("Current working directory: "));
		       		write(STDOUT_FILENO, cwd, strlen(cwd));
		       		write(STDOUT_FILENO, "\n", strlen("\n"));
		       		// check to see if it ignore any extra arguments
				}


				else if(strcmp(tokens[0], "cd")==0)
				{
					int change = 0;

					if(tokens[1]==NULL)
					{
						change = chdir(getenv("HOME"));
					}

					else
						change = chdir(tokens[1]);

					if(change!=0&&tokens[2]==NULL)
						write(STDOUT_FILENO, "ERROR: Invalid directory.\n", strlen("ERROR: Invalid directory.\n"));
				
				}

				else //create child process
				{
					if (in_background)
					{
						write(STDOUT_FILENO, "Run in background.\n", strlen("Run in background.\n"));
					}

					pid_t pid;
					pid = fork();
					if(pid==0)
					{
						if(execvp(tokens[0],tokens)== -1)
						{
							write(STDOUT_FILENO, tokens[0], strlen(tokens[0]));
							write(STDOUT_FILENO, ": Unknown command.\n",strlen(": Unknown command.\n"));
							break;
						}
							
					}

					else if(pid<0)
					{
						printf("ERROR: Fork Failed!\n");
					}

					if(in_background!=true)
					{
						wait(0);
					}
				}
			}

		}
	}

	


	return 0;
}