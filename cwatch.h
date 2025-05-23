#ifndef CWATCH_H
#define CWATCH_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/wait.h>
#include <libgen.h>
#include <signal.h>


#define MAXFILEWATCH 50
#define MAXDIRWATCH 5
#define MAXEXTWATCH 5
#define MSGMAX 2*1024

static inline const char *get_extension(const char *filename, char ext[], size_t n)
{

  const char *dot;
  char *tmp = strdup(filename);
  char *p = basename(tmp);

  if(( dot = strrchr(p,'.')) == NULL)
    return NULL;

  ++dot;
  memset(ext,0,n);
  strncpy(ext,dot,n);
  free(tmp);
  return ext;
}

static inline void err_msg(const char *msg,int quit)
{
  char msgbuf[MSGMAX];
  strcpy(msgbuf,"\033[1;35m[cwatch]\n\033[1;31m");
  strcat(msgbuf,msg);
  strcat(msgbuf,"\033[0m\n");
  write(STDERR_FILENO,msgbuf,strlen(msgbuf));
  if(quit)
    exit(EXIT_FAILURE);   
}


#endif
