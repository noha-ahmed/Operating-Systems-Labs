#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#define MAX_LIMIT 100

char command[MAX_LIMIT]; // string is an array of character OR  pointer to the first character
 char *parsedCommand[MAX_LIMIT];  //array of strings ( pointers )
pid_t pid;
pid_t endid;
bool waitFlag;
int status;
FILE *logFile;

// a function to read command and store it in a string
void readCommand()
{
    memset(parsedCommand, 0, sizeof parsedCommand);
    printf("command >> ");
    fgets(command, MAX_LIMIT, stdin);
    command[strlen(command)-1]='\0';
    printf("\n");
}

// a function to split command into separate command and arguments
void parseCommand()
{
    if( command[strlen(command)-1] == '&' )
    {
        command[strlen(command)-1]='\0';
        waitFlag = false;
    }
    parsedCommand[0] = strtok(command," "); //returns pointer to the first String
    int i = 0;
    while(parsedCommand[i]!= NULL)
    {
        parsedCommand[++i]=strtok(NULL," "); //return pointer to rest of strings
    }

}


// a function to execute command and fork child processes
void executeCommand()
{
    // exit shell
    if( strcmp(parsedCommand[0],"exit") == 0 )
    {
        kill(0,SIGKILL);
    }
    // fork child process
    pid = fork();
    if( pid == -1 )  //failed to fork child process
    {
        perror("-- Failed to fork child --\n");
        exit(0);
    }
    else if ( pid == 0 )  //return child process
    {
        // if process is not successful execvp returns -1
        // if successful it does not return;
        int x =execvp(parsedCommand[0],parsedCommand);
        if( x < 0 )
        {
            perror("-- Failed to execute command -- \n");
            exit(x);
        }
        exit(0);

    }
    else  //return from parent process if command without "&'
    {
        if( waitFlag )
        {
            endid = waitpid(pid,&status,WUNTRACED);
            printf("-- Wait is finished successfully , parent will proceed -- \n");

        }
    }
}

//a function to log terminated child processes in the logfile
void logTermination()
{

    logFile = fopen("logFile.txt","a");
    fprintf(logFile, "- Child process was terminated Successfully \n");
    fclose(logFile);
}

//a signal handler for the terminated child processes
void signal_handler()
{

    int wstat;
    pid_t pid;
    pid = wait3 (&wstat, WNOHANG, (struct rusage *)NULL );
    logTermination();
    if (pid == 0)
        return;
    else if (pid == -1)
        return;

}



int main()
{
    //handling the terminating of child processes
    signal (SIGCHLD, signal_handler);
    //reset the log file
    logFile = fopen("logFile.txt","w");
    fclose(logFile);
    //running the shell
    while(1)
    {
        waitFlag = true;
        readCommand();
        parseCommand();
        executeCommand();
    }

    return 0;
}
