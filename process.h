#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <unistd.h>
#include <signal.h>

typedef struct proc{
    int id;
    pid_t pid;
    char name[25],status[10];
    struct proc* next;
} Proc;

Proc *procs;
int proc_sz;
int max_id;
int shellpid;
volatile sig_atomic_t fgproc_pid;
char fgproc_name[25];

void init_procs();
void add_proc(pid_t pid, char* name);
int check_child();
void exec_process(char* args[],int argno);
void pinfo(char* args[],int argno);
void jobs(char* args[], int argno);
void sig(char* args[],int argno);
void bg(char* args[],int argno);
void fg(char* args[],int argno);
void initSignals();
void INThandler(int sig);
void TSTPhandler(int sig);
void IGN(int sig);

#endif