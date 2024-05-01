#ifndef CWATCH_H
#define CWATCH_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/wait.h>
#include <libgen.h>

#define MAXFILEWATCH 50
#define MAXDIRWATCH 5
#define MAXEXTWATCH 5
#define MSGMAX 2*1024
/*
extern char *files[MAXFILEWATCH];
extern int filecount;
*/

struct DATA {
  char *file;
  char *cmd;
};

#endif
