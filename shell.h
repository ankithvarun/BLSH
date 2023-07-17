#ifndef __SHELL_H__
#define __SHELL_H__

char dir_name[500], prevdir_name[500];
char dir_path[500], prevdir_path[500];
int dirlevel, prevdirlevel;
char username[500], hostname[500];

void init();

char* getwdir();
void pwd();
void cd(char* args[], int argno);
void echo(char* args[], int argno);

int tokenize(char string[],char* delim, char* tokens[]);
char* concat(char* args[],int argno,int bg_check);
int check_flag(char* args[], int argno, char* flag);
void implement_pipe(char* args[], int argno);
void execute_command(char* args[], int argno);

void ls_driver(char* args[], int argno);

#endif