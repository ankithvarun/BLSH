#include <stdio.h>
#include "process.h"
#include "shell.h"
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>

char getState(pid_t pid)
{
    char path[25];
    sprintf(path,"/proc/%d/status",pid);        // Path to file containing status and memory details

    FILE *f = fopen(path,"r"); 

    char State, word[30];
    while(fscanf(f,"%s: ",word) == 1)
    {
        if(!strcmp(word,"State:"))
            fscanf(f," %c ",&State);
    }
    fclose(f);
    return State;
}

void pinfo(char* args[], int argno)         // Displays the status of a process
{
    if(argno > 2)
    {
        printf("pinfo: too many arguments\n");
        return;
    }

    pid_t pid;
    if(argno == 1)
        pid = getpid();
    else
        pid = atoi(args[1]);

    char path[25];
    sprintf(path,"/proc/%d/status",pid);        // Path to file containing status and memory details

    FILE *f = fopen(path,"r"); 
    if(f == NULL)
    {
        perror("-shell: pinfo: process not found");
        return;
    }

    char State[2], word[30];
    long int VM;
    while(fscanf(f,"%s: ",word) == 1)
    {
        if(!strcmp(word,"State:"))
            fscanf(f," %s ",State);
        else if(!strcmp(word,"VmSize:"))
            fscanf(f," %ld ",&VM);
    }
    fclose(f);

    char execpath[500];
    sprintf(path,"/proc/%d/exe",pid);       // Executable path of the process
    int sz = readlink(path,execpath,500);
    execpath[sz] = '\0';

    printf("pid -- %d\n",pid);
    printf("Process Status -- %s",State);
    printf((pid == getpid()) ? "+\n" : "\n");
    printf("memory -- %ld kB\n",VM);
    printf("Executable path -- %s\n",execpath);
}

void init_procs()       // Initialise linked list for background processes
{
    procs = (Proc*) malloc(sizeof(Proc));
    procs->pid = 0;
    procs->next = NULL;
    proc_sz = 0;
    max_id = 0;

    fgproc_pid = getpid();
}

void add_proc(pid_t pid, char* name)        // Add a child background process to the linked list once created 
{
    Proc *p = (Proc *) malloc(sizeof(Proc));
    proc_sz++;
    max_id++;
    p->id = max_id;
    p->pid = pid;
    strcpy(p->name,name);

    p->next = procs->next;
    procs->next = p;
}

Proc* get_proc(pid_t pid, int id,int remove_flag)       // Retrieve background process details according to job id or pid and selectively delink it from the linked list
{
    Proc* p = procs;
    while(p->next != NULL)
    {
        if((p->next->pid == pid && id == 0) || (p->next->id == id && pid == 0))
        {
            Proc* temp = p->next;

            if(proc_sz == 1)
                max_id = 0;
            else if(temp->next != NULL)
                max_id = temp->next->id;
            else
                max_id = p->id;

            if(remove_flag)
            {
                p->next = temp->next;
                proc_sz--;
            }
            return temp;
        }
        p = p->next;
    }
    return NULL;
}

int check_child()       // Checks if a background process has finished and displays an appropriate prompt
{
    int wstatus;
    pid_t pid = waitpid(-1,&wstatus,WNOHANG);
    
    if(pid > 0)
    {
        Proc* p = get_proc(pid,0,1);
        if(WIFEXITED(wstatus))
            fprintf(stderr,"%s with pid %d exited normally\n",p->name,pid);
        else
            fprintf(stderr,"%s with pid %d exited abnormally\n",p->name,pid);
        free(p);
        return 1;
    }    
    return 0;
}

int cmp(const void * a, const void * b)     // Comparator function for qsort function in jobs
{
    return strcmp(((Proc*)a)->name,((Proc*)b)->name);
}

void jobs(char* args[], int argno)      // Function that lists all background processes (running or stopped or both)
{
    if(proc_sz == 0)
        return;
    
    int n = 0;
    char state_flag[3], status[10];
    Proc* p = procs;
    Proc proc_arr[30];
    if(check_flag(args,argno,"-r") >= 0 && check_flag(args,argno,"-s") == -1)
        strcpy(state_flag,"RS");
    else if(check_flag(args,argno,"-s") >= 0 && check_flag(args,argno,"-r") == -1)
        strcpy(state_flag,"TZ");
    else
        strcpy(state_flag,"A");

    while(p->next != NULL)
    {
        char state = getState(p->next->pid);
        if(!strcmp(state_flag,"A") || state == state_flag[0] || state == state_flag[1])
        {
            if(state == 'R' || state == 'S')
                strcpy(p->next->status,"Running");
            else    
                strcpy(p->next->status,"Stopped");
            proc_arr[n++] = *(p->next);
        }
        p = p->next;
    }

    qsort(proc_arr,n,sizeof(Proc),cmp);

    for(int i = 0 ; i < n ; i++)
    {
        printf("[%d] %s %s [%d]\n",proc_arr[i].id,proc_arr[i].status,proc_arr[i].name,proc_arr[i].pid);
    }
}

void sig(char* args[],int argno)        // Function to send a signal to a particular background process based on job number
{
    int id = atoi(args[1]),signal = atoi(args[2]);
    Proc* p = get_proc(0,id,0);
    if(p == NULL)
    {
        printf("-shell: sig: %d: no such job\n",id);
        return;
    }
    
    if(kill(p->pid,signal) < 0)
    {
        perror("-shell: sig");
        return;
    }
}

void bg(char* args[],int argno)         // Changes the status of the required background process from Stopped to Running (if needed)
{
    int id;
    if(argno == 1)
        id = max_id;
    else
        id = atoi(args[1]);
    
    Proc* p = get_proc(0,id,0);
    if(p == NULL)
    {
        printf("-shell: fg: %d: no such job\n",id);
        return;
    }

    if (kill(p->pid, SIGCONT) < 0) {
        perror("-shell: bg: could not run background process");
        return;
    }

    printf("[%d] %s running in background\n",p->id,p->name);
}

void fg(char* args[],int argno)     // Brings the required background process to the foreground
{
    int id;
    if(argno == 1)
        id = max_id;
    else
        id = atoi(args[1]);
    
    Proc* p = get_proc(0,id,1);
    if(p == NULL)
    {
        printf("-shell: fg: %d: no such job\n",id);
        return;
    }
    
    pid_t shellpgid = getpgid(0);    

    signal(SIGTTOU, SIG_IGN);
    tcsetpgrp(STDIN_FILENO, getpgid(p->pid));
    signal(SIGTTOU, SIG_DFL);

    kill(p->pid,SIGCONT);
    fgproc_pid = p->pid;
    strcpy(fgproc_name,p->name);
    waitpid(p->pid,NULL,WUNTRACED);

    signal(SIGTTOU, SIG_IGN);
    tcsetpgrp(STDIN_FILENO, shellpgid);
    signal(SIGTTOU, SIG_DFL);

    fgproc_pid = shellpid;
    strcpy(fgproc_name,"Shell");

    printf("[%d] %s\n",p->id,p->name);  
}

void exec_process(char* args[],int argno)       // Executes a system command as foreground or background process
{
    if(!strcmp(args[argno-1],"&"))
    {
        args[argno-1] = NULL;
        pid_t forkReturn = fork();

        if(forkReturn == 0)
        {
            setpgid(0,0);
            execvp(args[0],args);
        }
        else
        {
            add_proc(forkReturn,concat(args,argno,1));
            printf("%d\n",forkReturn);
            setpgid(forkReturn, 0);
            tcsetpgrp(STDIN_FILENO, getpgrp());
        }
    }

    else
    {
        args[argno] = NULL;
        int status;
        pid_t forkReturn = fork();

        fgproc_pid = forkReturn;
        strcpy(fgproc_name,concat(args,argno,0));

        if(forkReturn == 0)
        {
            execvp(args[0],args);
        }
        else
            pause();
    }
}
