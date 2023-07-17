#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <time.h>

char* l_fields(struct stat sb,const char* name)         // Functionality for -l flag in ls command
{
    char *dets = (char*) malloc(200*sizeof(char));

    sprintf(dets,(sb.st_mode < 17000) ? "d" : "-");
    sprintf(dets + strlen(dets),(sb.st_mode & S_IRUSR) ? "r" : "-");
    sprintf(dets + strlen(dets),(sb.st_mode & S_IWUSR) ? "w" : "-");
    sprintf(dets + strlen(dets),(sb.st_mode & S_IXUSR) ? "x" : "-");
    sprintf(dets + strlen(dets),(sb.st_mode & S_IRGRP) ? "r" : "-");
    sprintf(dets + strlen(dets),(sb.st_mode & S_IWGRP) ? "w" : "-");
    sprintf(dets + strlen(dets),(sb.st_mode & S_IXGRP) ? "x" : "-");
    sprintf(dets + strlen(dets),(sb.st_mode & S_IROTH) ? "r" : "-");
    sprintf(dets + strlen(dets),(sb.st_mode & S_IWOTH) ? "w" : "-");
    sprintf(dets + strlen(dets),(sb.st_mode & S_IXOTH) ? "x " : "- ");
    sprintf(dets + strlen(dets),"%d ",(int)sb.st_nlink);
    sprintf(dets + strlen(dets),"%s %s ",getpwuid(sb.st_uid)->pw_name,getpwuid(sb.st_gid)->pw_name);
    sprintf(dets + strlen(dets),"%10d ",(int)sb.st_size);

    struct tm *datetime = localtime(&sb.st_mtim.tv_sec);
    struct timespec current_time;
    char m_time[15];
    clock_gettime(CLOCK_REALTIME,&current_time);
    if((long int)current_time.tv_sec - (long int)sb.st_mtim.tv_sec < 15552000)
        strftime(m_time,sizeof(m_time),"%b %e %H:%M",datetime);
    else
        strftime(m_time,sizeof(m_time),"%b %e  %G",datetime);

    sprintf(dets + strlen(dets),"%s %s",m_time,name);

    return dets;
}

void ls(const char *_dir, int flag_a, int flag_l)       // Functionality for single directory ls command with flags
{
    struct dirent *d;
    struct stat sb;
	DIR *dh = opendir(_dir);
	if (!dh)
	{
		if(errno == ENOTDIR)
        {
            if(flag_l)
            {
                if(stat(_dir,&sb) == -1)
                {
                    perror("stat");
                    return;
                }
                printf("%s\n",l_fields(sb,_dir));
            }
            else
                printf("%s\n",_dir);
            return;
        }
        
        char error[120] = "ls: cannot access ";
        perror(strcat(error,_dir));
        return;
	}
        
    char* deets[1000];
    int sz = 0;
    long int file_sz = 0, block_sz;

    if(stat(_dir,&sb) == -1)
    {
        perror("ls");
        return;
    }
    block_sz = sb.st_blksize;

    if(strcmp(_dir,"."))
        if(chdir(_dir) == -1)
        {   
            perror("ls");
            return;
        }

	while ((d = readdir(dh)) != NULL)
	{
		if (!flag_a && d->d_name[0] == '.')
			continue;

		if(flag_l)
        {
            if(stat(d->d_name,&sb) == -1)
            {
                perror("ls");
                return;
            }

            deets[sz++] = l_fields(sb,d->d_name);
            if(sb.st_mode > 17000)
            {
                if((long int)sb.st_size % block_sz == 0)
                    file_sz += sb.st_size;
                else
                    file_sz += (((long int)sb.st_size / block_sz) + 1)*block_sz;
            }
        }
        else
            printf("%s\n", d->d_name);
	}

    if(flag_l)
    {
        printf("total %ld\n",file_sz/1024);
        for(int i = 0 ; i < sz ; i++)
            printf("%s\n",deets[i]);
    }

    if(chdir(dir_path) == -1)
    {
        perror("ls");
        return;
    }

    if(closedir(dh) == -1)
    {
        perror("ls");
        return;
    }
}

void ls_driver(char* args[],int argno)      // Driver function for ls command
{
    char* path;
    int flag_a = 0, flag_l = 0, dir_no = 0;

    if(check_flag(args,argno,"-a") >= 0)
        flag_a = 1;

    if(check_flag(args,argno,"-l") >= 0)
        flag_l = 1;

    if(check_flag(args,argno,"-la") >= 0 || check_flag(args,argno,"-al") >= 0)
    {
        flag_a = 1;
        flag_l = 1;
    }

    for(int i = 1 ; i < argno ; i++)
    {
        if(args[i][0] != '-')
        {
            dir_no++;      
            if(!strcmp(args[i],"~"))
            {
                char home[106] = "/home/";
                strcat(home,username);
                path = home;
            }
            else          
                path = args[i];
        }
    }

    if(dir_no == 0)
        ls(".",flag_a,flag_l);
    else if(dir_no == 1)        
        ls(path,flag_a,flag_l);
    else
    {
        for(int i = 1 ; i < argno ; i++)
        {
            if(args[i][0] != '-')
            {
                printf("%s:\n",args[i]);
                if(!strcmp(args[i],"~"))
                {
                    char home[106] = "/home/";
                    strcat(home,username);
                    ls(home,flag_a,flag_l);
                }
                else
                    ls(args[i],flag_a,flag_l);
                printf("\n");
            }
        }
    }
}