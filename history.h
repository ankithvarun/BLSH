#ifndef __HISTORY_H__
#define __HISTORY_H__

int log_sz;
char commands_log[20][1000];
char log_path[1000];

void init_history();
int get_commands();
void add_command(char* command);
void history(char* args[],int argno);

#endif