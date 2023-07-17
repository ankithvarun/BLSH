#include "process.h"
#include "shell.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

void IGN(int sig)       // Equivalent of SIG_IGN macro for the shell
{
    printf("\n<%s@%s:%s> ",username,hostname,dir_name);
    fflush(stdout);
}

void INThandler(int sig)        // Handler for SIGINT signal
{
    kill(fgproc_pid,SIGINT);
    printf("\n");
}

void TSTPhandler(int sig)       // Handler for SIGTSTP signal
{
    setpgid(fgproc_pid,0);
    add_proc(fgproc_pid,fgproc_name);
    kill(fgproc_pid,SIGSTOP);
    printf("\nStopped %s [%d]\n",fgproc_name,fgproc_pid);
    fgproc_pid = shellpid;
}

void ChildHandler(int sig)      // Handler for SIGCHLD signal for parent process to detect exited child process
{
    int status;
    waitpid(-1, &status, WNOHANG);
}

void initSignals()      // Initialise handlers for the respective signals
{
    signal(SIGINT,SIG_IGN);
    signal(SIGTSTP,SIG_IGN);
    signal(SIGCHLD,ChildHandler);
}
