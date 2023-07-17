#include "shell.h"
#include "process.h"
#include "history.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

void execute_command(char* args[], int argno)        // Driver code to execute a command
{
    if(check_flag(args,argno,">") >= 0)     // Implementation of output redirection
    {
        int pos = check_flag(args,argno,">");
        if(freopen(args[pos+1],"w",stdout) == NULL)
        {
            perror(args[pos+1]);
            return;
        }
        execute_command(args,argno - 2);
        freopen("/dev/tty", "w", stdout);
    }

    else if(check_flag(args,argno,">>") >= 0)       // Implementation of output redirection in append mode
    {
        int pos = check_flag(args,argno,">>");
        if(freopen(args[pos+1],"a",stdout) == NULL)
        {
            perror(args[pos+1]);
            return;
        }
        execute_command(args,argno - 2);
        freopen("/dev/tty", "w", stdout);
    }

    else if(check_flag(args,argno,"<") >= 0)        // Implementation of input redirection
    {
        int pos = check_flag(args,argno,"<");
        if(freopen(args[pos+1],"r",stdin) == NULL)
        {
            perror(args[pos+1]);
            return;
        }
        execute_command(args,argno - 2);
        freopen("/dev/tty", "r", stdin);
    }

    else if(!strcmp("echo",args[0]))
        echo(args,argno);
    else if(!strcmp("pwd",args[0]))
        pwd();
    else if(!strcmp("cd",args[0]))
        cd(args,argno);
    else if(!strcmp("ls",args[0]))
        ls_driver(args,argno);
    else if(!strcmp("pinfo",args[0]))
        pinfo(args,argno);
    else if(!strcmp("repeat",args[0]))      // Implementation of repeat command
    {
        int i,n = atoi(args[1]);
        char temp[100],_command[100];
        for(i = 2 ; i < argno ; i++)
            args[i-2] = args[i];

        for(i = 0 ; i < n ; i++)
            execute_command(args,argno - 2);
        
    }
    else if(!strcmp("history",args[0]))
        history(args,argno);
    else if(!strcmp("jobs",args[0]))
        jobs(args,argno);
    else if(!strcmp("sig",args[0]))
        sig(args,argno);
    else if(!strcmp("bg",args[0]))
        bg(args,argno);
    else if(!strcmp("fg",args[0]))
        fg(args,argno);
    else if(!strcmp("replay",args[0]))      // implementation of replay command
    {
        int interval_pos = check_flag(args,argno,"-interval"); 
        float setTime = atof(args[interval_pos + 1]);

        int period_pos = check_flag(args,argno,"-period");
        float n = atof(args[period_pos + 1]) / setTime; 
        int command_pos = check_flag(args,argno,"-command");  

        char* command = (char*) calloc(50,sizeof(char));
        
        for(int i = command_pos + 1 ; i < interval_pos ; i++)
        {
            strcat(command,args[i]);
            if(i < argno - 1)
                strcat(command," ");
        }

        char* _args[50];
        int _argno = tokenize(command," \t",_args);
        
        for(float i = 1 ; i <= n ; i++)
        {
            time_t startTime;
            float elapsedTime;
            time(&startTime);
            while (difftime(time(NULL), startTime) < setTime);
            execute_command(_args,_argno); 
        }

        int x = atoi(args[period_pos + 1]) / atoi(args[interval_pos + 1]);
        if(x != 0)
        {
            x = atoi(args[period_pos + 1]) - x*atoi(args[interval_pos + 1]);
            sleep(x);
        }
    }
    else
        exec_process(args,argno);
}

void implement_pipe(char* commands[],int commandno)     // Implementation of one or many command pipelines
{
    int pipefd[2],status,tempfd,i = 0;
    pid_t pid;
    
    while(i < commandno)
    {
        if(pipe(pipefd) < 0)
        {
            perror("-shell: pipe: ");
            return;
        }
        
        if((pid = fork()) < 0)
        {
            perror("-shell: fork: ");
            return;
        }

        else if(pid == 0)
        {
            dup2(tempfd,STDIN_FILENO);
            if(i < commandno - 1)
                dup2(pipefd[1],STDOUT_FILENO);
            close(pipefd[0]);

            char* args[50];
            int argno = tokenize(commands[i]," \t",args);
            execute_command(args,argno);
            exit(1);            
        }

        else
        {
            wait(NULL);
            close(pipefd[1]);
            tempfd = pipefd[0];
            i++;
        }
    }
}