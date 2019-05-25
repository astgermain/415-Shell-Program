/****************************************************************
 * Name        : Andrew St Germain                              *
 * Class       : CSC 415                                        *
 * Date        : 3/22/19                                        *
 * Description :  Writting a simple bash shell program          *
 *                that will execute simple commands. The main   *
 *                goal of the assignment is working with        *
 *                fork, pipes and exec system calls.            *
 ****************************************************************/

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

/* CANNOT BE CHANGED */
#define BUFFERSIZE 256
/* --------------------*/
#define PROMPT "myShell >> "
#define PROMPTSIZE sizeof(PROMPT)
//Drew some inspiration from Geeks for geeks
//
void pipes(char *argv1[], char *argv2[]){
	
	int pipefd[2];
	int sts = pipe(pipefd);
	pid_t p1, p2;

	if(pipe(pipefd) < 0){
		perror("Pipe failed, not init");
		exit(0);
	}
	p1 = fork();
		if(p1 == -1){
			perror("Failed fork");
			exit(0);
		}
		if(p1 == 0){
			close(pipefd[1]);
			close(0);
			dup(pipefd[0]);
			if(execvp(argv2[0], argv2) < 0){
				perror("Command error\n");
				exit(0);
			}
			close(pipefd[0]);
		}
		p2 = fork();
		if(p2 == -1){
			perror("Failed fork");
			exit(-1);
		}
		if(p2 == 0){
			close(pipefd[0]);
			close(1);
			dup(pipefd[1]);
			if(execvp(argv1[0], argv1) < 0){
				perror("Command error\n");
				exit(0);
			}
			close(pipefd[1]);
		}
		
		close(pipefd[0]);
		close(pipefd[1]);
		while(wait(&sts) != p2);
		while(wait(&sts) != p1);

}

void execute(char *argv1[], int fileName, int inputFile, int startbckgrd){
	int sts;
	pid_t pd;
	pd = fork();
	if(pd < 0){
		perror("Failed fork");
		exit(0);
	}
	else if(pd == 0){
		
		if(execvp(argv1[0], argv1) < 0){
			perror("Error, invalid command\n");
			exit(1);
		}

		if(fileName){
			close(1);
			dup(fileName);
			//close(fileName);
		}

		if(inputFile){
			close(0);
			dup(inputFile);
			//close(inputFile);
		}
		if(fileName){
			close(fileName);
		}
		if(inputFile){
			close(inputFile);
		}
	}

	else{
		if(!startbckgrd){
			while(wait(&sts) != pd);
		}
	}
}



int main() {
while(1){
	//learned from geeksforgeeks making linux shell
	//displays current working dir
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	printf("%s: %s ", PROMPT, cwd);

	//vars used
	int arg1 = 0, arg2 = 0, fileName = 0, startbckgrd = 0, inputFile = 0, pipe = 0, tmp = 0;
	char *modifier; 
	char buffer[BUFFERSIZE], buffercp[BUFFERSIZE];
       	//Decided not to parse args TT
	
	if(fgets(buffer, BUFFERSIZE, stdin)){
		strncpy(buffercp, buffer, BUFFERSIZE);
	}

	else{
		continue;
	}

	modifier = strtok(buffer, " \n");
	
	while(modifier != NULL){
		//execute in background is true
		//check all modifiers
		
		if(strcmp(modifier, "&") == 0){
			startbckgrd = 1;
		}

		else if(strcmp(modifier, ">") == 0){
			modifier = strtok(NULL, " \n");
			
			fileName = open(modifier, O_WRONLY| O_CREAT| O_TRUNC, S_IRUSR| S_IWUSR| S_IRGRP| S_IROTH);
			
			if(fileName == -1){
				perror("Cannot open file\n");
				exit(0);
			}
			
		}

		else if(strcmp(modifier, ">>") == 0){
			modifier = strtok(NULL, " \n");
			fileName = open(modifier, O_WRONLY|O_APPEND|O_CREAT,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
			
			if(fileName == -1){
				perror("Cannot open file\n");
				exit(0);
			}

		}
		
		else if(strcmp(modifier, "<") == 0){
			modifier = strtok(NULL, " \n");
			inputFile = open(modifier, O_RDONLY, NULL);
			exit(0);
			
			if(fileName == -1){
				perror("Cannot open file\n");
				exit(0);
			}

		}

		
		else if(strcmp(modifier, "|") == 0){
			pipe = 1;
			tmp = arg1;
			arg1 = 0;
		}

		else{
			++arg1;
		}

		modifier = strtok(NULL, " \n");
	}
	
	if(arg1 != 0){
		
		if(pipe){
			arg2 = arg1;
			arg1 = tmp;
		}

		char *arg1Arr[arg1 + 1], *arg2Arr[arg2 + 1];
		arg1Arr[0] = strtok(buffercp, " \n");
		int i = 0, j = 0;
		
		do{
			//fprintf(stderr, "works");
			if(startbckgrd){
		
				if(strcmp(arg1Arr[i], "&") == 0){
					arg1Arr[i] = strtok(NULL, " \n");
					break;
				}

			}
			
			if(fileName){
				
				if(strcmp(arg1Arr[i], ">") == 0){
					
					strtok(NULL, " \n");
					arg1Arr[i] = strtok(NULL, " \n");		
					break;
				}

				else if(strcmp(arg1Arr[i], ">>") == 0){
					strtok(NULL, " \n");
					arg1Arr[i] = strtok(NULL, " \n");
					break;
				}

			}

			if(inputFile){

				if(strcmp(arg1Arr[i], "<") == 0){
					strtok(NULL, " \n");
					arg1Arr[i] = strtok(NULL, " \n");
					break;
				}

			}

			if(pipe){

				if(strcmp(arg1Arr[i], "|") == 0){
					
					do{
					
						arg2Arr[j] = strtok(NULL, " \n");
						

					}
					while(arg2Arr[++j] != NULL);
					
					arg1Arr[i] = arg2Arr[--j];
					break;
				}
			}

			arg1Arr[++i] = strtok(NULL, " \n");
						
		}
		while(arg1Arr[i] != NULL);
		
		//terminates
		if(strcmp(arg1Arr[0], "exit") == 0){
			exit(0);
		}
		//change dir
		else if(strcmp(arg1Arr[0], "cd") == 0){
			if(arg1Arr[1] == NULL){
				chdir(getenv("HOME"));
			}
			else{
				chdir(arg1Arr[1]);
			}
		}
		
		else{
			if(pipe){
				pipes(arg1Arr, arg2Arr);
			}
			else{
				execute(arg1Arr, fileName, inputFile, startbckgrd);
			}
		}
	}
}

    
return 0;
}
