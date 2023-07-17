#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/wait.h>
#include "shell.h"
#include "history.h"
#include "process.h"

void initdir()      // Function to initialise the home directory name and path
{
    strcpy(dir_name,"~");
    strcpy(prevdir_name,"-");
    strcpy(dir_path,getwdir());
    dirlevel = 0;
}

void Getusername()      // Function to fetch the name of the current user 
{
    struct passwd *pwent = getpwuid(getuid());
    strcpy(username,pwent->pw_name);
}

void Gethostname()      // Function to fetch the name of the system
{
    if(gethostname(hostname,100))
        perror("gethostname");
}

void init()     // Initialises environment variables of the shell
{
    initdir();
    Getusername();
    Gethostname();
    init_history();
    init_procs();
    shellpid = getpid();
    fgproc_pid = shellpid;
    strcpy(fgproc_name,"Shell");
}

char* getwdir()     // Function to get the current working directory of the shell
{
    char* cwd = (char*) malloc(100*sizeof(char));
    if(getcwd(cwd,100) == NULL)
        perror("pwd");
    return cwd;
}

int check_flag(char* args[], int argno, char* flag)     // Check if a flag exists in a command
{
    for(int i = 0 ; i < argno; i++)
        if(!strcmp(args[i],flag))
            return i;
    return -1;
}

int tokenize(char string[],char* delim, char* tokens[])     // Parses input and commands into arguments based on a delimiter
{
    int i = 0;
    char* token = strtok(string,delim);
    tokens[i++] = token;

    while(token != NULL)
    {
        token = strtok(NULL,delim);
        tokens[i++] = token;
    }
    return --i;
}

char* concat(char* args[],int argno,int bg_check)       // Function to concatenate command from tokenised arguments
{
    char* string = (char*) calloc(50,sizeof(char));
    int n;
    if(bg_check == 1)
        n = argno - 1;
    else
        n = argno;
    for(int i = 0 ; i < n ; i++)
    {
        strcat(string,args[i]);
        if(i < n - 1)
            strcat(string," ");
    }
    return string;
}

void echo(char* args[], int argno)      // Implementation of the built-in command echo
{
    if(argno == 1)
    {
        printf("\n");
        return;
    }
    
    for(int i = 1 ; i < argno - 1 ; i++)
        printf("%s ",args[i]);
    printf("%s\n",args[argno-1]);
}

void pwd()      // Implementation of the built-in command pwd
{
    if(getwdir() != NULL)
        printf("%s\n",getwdir());
}

void cd(char* args[], int argno)        // Implementation of the built-in command cd with various flags
{
    if(argno > 2)
    {
        printf("cd: too many arguments\n");
        return;
    }
    
    if(argno == 1 || !strcmp(args[1],"~"))
    {
        char home[106] = "/home/";
        if(chdir(strcat(home,username)) == -1)
        {
            perror("cd");
            return;
        }
        
        strcpy(prevdir_name,dir_name);
        strcpy(prevdir_path,dir_path);
        strcpy(dir_name,home);
        strcpy(dir_path,home);
        prevdirlevel = dirlevel;
        dirlevel = -1;
        return;
    }

    else if(!strcmp(args[1],"."))
        return;

    else if(!strcmp(args[1],"-"))
    {
        if(!strcmp(prevdir_name,"-"))
        {
            printf("-shell: cd: OLDPWD not set\n");
            return;
        }
        
        printf("%s\n",prevdir_path);
        if(chdir(prevdir_path) == -1)
        {
            perror("cd");
            return;
        }
        strcpy(prevdir_path,dir_path);
        
        char temp[100];
        strcpy(temp,dir_name);
        strcpy(dir_name,prevdir_name);
        strcpy(prevdir_name,temp);

        int _temp = dirlevel;
        dirlevel = prevdirlevel;
        prevdirlevel = _temp;

        strcpy(dir_path,getwdir());
        return;
    }

    else
    {
        if(chdir(args[1]) == -1)
        {
            perror("cd");
            return;
        }
    }

    strcpy(prevdir_name,dir_name);
    strcpy(prevdir_path,dir_path);
    prevdirlevel = dirlevel;
    if(args[1][0] == '/')
    {
        strcpy(dir_name,args[1]);
        dirlevel = -1;
    }
    else if(!strcmp(args[1],".."))
    {
        dirlevel--;
        if(dirlevel < 0)
            strcpy(dir_name,getwdir());
        else
        {
            int i = strlen(dir_name) - 1;
            while(dir_name[i] != '/')
                i--;
            dir_name[i] = '\0';
        }
    }
    else
    {
        if(dirlevel < 0)
            strcpy(dir_name,getwdir());
        else
        {
            int count = 0;
            for(int i = 0 ; i < strlen(args[1]) ; i++)
                if(args[1][i] == '/')
                    count++;
            dirlevel += count + 1;
            strcat(dir_name,"/");
            strcat(dir_name,args[1]);
        }
    }
    strcpy(dir_path,getwdir());
}