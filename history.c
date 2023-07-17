#include "history.h"
#include "shell.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void init_history()     // Initialise path of history.data and retrieve command entries
{
    sprintf(log_path,"/home/%s/history.data",username);      // Path of the file which stores entries of previous commands (change if necessary)
    log_sz = get_commands();
}

int get_commands()      // Retrieve command entries from history.data
{
    FILE* f = fopen(log_path,"rb");
    if(f == NULL)
        return 0;    
    int i = 0;
    fread(commands_log,sizeof(commands_log),sizeof(char),f);
    while(strcmp(commands_log[i],"") && i < 20)
        i++;
    fclose(f);
    return i;
}

void put_commands()     // Load command entries into history.data
{
    FILE* f = fopen(log_path,"wb");
    fwrite(commands_log,sizeof(commands_log),sizeof(char),f);
    fclose(f);
}

void add_command(char* command)     // Adds a command entered in shell prompt to commands_log
{
    if(log_sz > 0 && !strcmp(commands_log[log_sz - 1],command))
        return;
    
    if(log_sz < 20)
    {
        strcpy(commands_log[log_sz],command);
        log_sz++;
    }
    else
    {
        for(int i = 0 ; i < 19 ; i++)
            strcpy(commands_log[i],commands_log[i+1]);
        strcpy(commands_log[19],command);
    }
    put_commands();
}

void history(char* args[],int argno)        // Implementation of history command
{
    if(argno > 2)
    {
        printf("history: too many arguments\n");
        return;
    }

    int n;
    if(argno == 1)
        n = 10;
    else
        n = atoi(args[1]);

    if(log_sz < n)
        n = log_sz;

    for(int i = log_sz - n ; i < log_sz ; i++)
        printf("%s\n",commands_log[i]);
}