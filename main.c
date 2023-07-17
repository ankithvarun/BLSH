#include <stdio.h>
#include <string.h>
#include "shell.h"
#include "history.h"
#include "process.h"
#include <signal.h>
#include <stdlib.h>

void signalHandler(int sig)
{
    signal(sig, SIG_IGN);

    printf("Okay exiting...\n");
    exit(0);
}

int main()
{
    char input[1000],*commands[100];
    init();
    initSignals();

    do{
        if(check_child())
        {
            sleep(1);
            continue;
        }
        printf("<%s@%s:%s> ",username,hostname,dir_name);       // Displays the prompt of the shell

        if(fgets(input,1000,stdin) == NULL)
        {
            printf("\nlogout");
            exit(0);
        }
        input[strcspn(input, "\n")] = 0;

        signal(SIGINT,INThandler);
        signal(SIGTSTP,TSTPhandler);
        int no_of_commands = tokenize(input,";",commands);      // Tokenises multiple commands separated by semicolon
        if(no_of_commands > 0)
            add_command(input);     // Log the input to history.txt
        for(int i = 0 ; i < no_of_commands ; i++)
        {
            char *args[50];
            int argno;
            argno = tokenize(commands[i],"|",args);
            if(argno > 1)
                implement_pipe(args,argno);
            else
            {
                argno = tokenize(commands[i]," \t",args);
                execute_command(args,argno);
            }
        }
        signal(SIGINT,IGN);
        signal(SIGTSTP,IGN);
    }while(strcmp(input,"exit"));       // Exit the shell when prompted
}