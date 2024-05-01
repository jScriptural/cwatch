/************************************************
 * @file cwatch.c
 * @brief watch for changes in file(s)
 * @synopsis:
 *           cwatch [-f "file1:file2:...:filen" [-d "dir1:dir2:...:dirn"] [-e "js:html"]] -c "command"
 *
 *  options:
 *  -f : a colon separated filenames to watch. Either -f or -d must be specify or both can be specify.if -f is specified,`file1` must be the entry point for the project/app. `command` will be pasaed the file1 whenever change is detected.
 *  -d : a colon separated directories to watch.
 *  Either -f or -d must be specify or both can be specify.
 *  -e : Used with -d to selectively watch files with the specify extensions in the directory given to -d.
 *  -c : specify the command to run if a file changes.
 *
 *
 ************************************************/


#include "cwatch.h"

char *files[MAXFILEWATCH];
int filecount = 0;
char *directories[MAXDIRWATCH];
int dircount = 0;
char *extensions[MAXEXTWATCH];
int extcount = 0;

pid_t pid = -1;
pthread_mutex_t mpid = PTHREAD_MUTEX_INITIALIZER;


char *f=NULL;
char *d =NULL;
char *e = NULL;
char *c = NULL;

const char *extname (const char *filename);
void *routine(void *arg);
void err_msg(const char *msg,int exit);

int main(int argc, char *argv[])
{
  setvbuf(stdout,NULL,0,_IONBF);
  setvbuf(stderr,NULL,0,_IONBF);
  setvbuf(stdin,NULL,0,_IONBF);

  int opt;
  opterr = 0;

  while((opt=getopt(argc,argv,"f:d:e:c:")) != -1)
  {
    switch(opt)
    {
      case 'f':
	f=optarg;
	break;
      case 'd':
	d=optarg;
	break;
      case 'e':
	e=optarg;
	break;
      case 'c':
	c=optarg;
	break;

    }
  }//while(getopt)

 // printf("f=%s,d=%s,e=%s,c=%s\n",f,d,e,c);

  if(f)
  {
    char *file = strtok(f,":");
    struct stat  fdata;
    if(stat(file,&fdata) < 0)
      err_msg(strerror(errno),1);
    if(S_ISREG(fdata.st_mode))
      files[filecount++] = strdup(file); 

    while((file = strtok(NULL,":")) != NULL && filecount < MAXFILEWATCH)
    {
      memset(&fdata,0,sizeof(fdata));
      if(stat(file,&fdata) < 0)
	err_msg(strerror(errno),1);
      if(S_ISREG(fdata.st_mode))
	files[filecount++] = strdup(file); 
    }//while(file);

  }//if(f)

  /*for(int i =0; i < filecount; ++i)
    printf("files[%d] = %s\n",i, files[i]);*/


  if(e)
  {
    extensions[extcount++] = strtok(e,":");
    while((extensions[extcount] = strtok(NULL,":")) != NULL && extcount < MAXEXTWATCH)
      ++extcount;

  }//if(e)


  if(d)
  {
    DIR *dir;
    struct dirent *entry;
    struct stat sts;
    char file[BUFSIZ];

    directories[dircount++] = strtok(d,":");
    while((directories[dircount] = strtok(NULL,":")) != NULL && dircount < MAXDIRWATCH)
      ++dircount;

    for(int i=0; i < dircount; ++i)
    {
      char *dp = directories[i];

      if((dir = opendir(dp)) == NULL)
	err_msg(strerror(errno),1);

      while((entry = readdir(dir)) != NULL && filecount < MAXFILEWATCH)
      {
	memset(file,0,sizeof(file));
	strncpy(file,dp,sizeof(file));
	strcat(file,"/");
	strncat(file,entry->d_name,BUFSIZ-strlen(file));


	//printf("file = %s\n", file);
	if(stat(file,&sts) < 0)
	  err_msg(strerror(errno),1);

	if(S_ISREG(sts.st_mode))
	{
	  if(!e)
	    files[filecount++] = strdup(file);

	  const char *ext = extname(file);
	  char **p = extensions;

	  for(;*p != NULL; ++p)
	  {
	    if(!strcmp(*p,ext))
	    {
	      files[filecount++] = strdup(file);
	      break;
	    }//if(!strcmp)

	  }//for(p!=null)

	}//if(S_ISREG)

      }//while(readdir)

    }//for(dircount)

  }//if(d)
  

  /*printf("\033[1;35m[cwatch]\nWatching\033[0m: ");
  for(int i =0; i < filecount; ++i)
    printf("%s, ",files[i]);
  printf("\n\n");*/
  
  if(filecount <= 0 || c == NULL)
    err_msg("NO file to watch",1);

  char buffer[MSGMAX]={0};
  strcpy(buffer,"\033[1;35m[cwatch]\nWatching:\033[0m ");
  for(int i =0; i < filecount; ++i)
    strcat(buffer,files[i]);
  strcat(buffer,"\n\n");

  write(STDOUT_FILENO,buffer,strlen(buffer));
  for(int i = 0; i < filecount; ++i)
  {
    pthread_t tid;
    pthread_create(&tid,NULL,routine,(void*)files[i]);
    pthread_detach(tid);
    if(i >= filecount )
      write(STDOUT_FILENO,buffer,strlen(buffer));
  }//for(filecount)


  //pause();
  pthread_exit(NULL);
  
}//main



const char *extname(const char *filename)
{
  const char *dot;
  char *tmp = strdup(filename);
  char *p = basename(tmp);

  if(( dot = strrchr(p,'.')) == NULL)
    return NULL;

  free(tmp);
  return ++dot;
}

void *routine(void *arg)
{
  char *file =  (char *) arg;
  char *entpt = files[0];
  //pid_t pid;
  struct stat fdata;
  struct timespec mtim;

  printf("tid: %ld, file: %s\n",(long)pthread_self(),file);
  if(stat(file,&fdata) < 0)
    err_msg(strerror(errno),1);

  mtim = fdata.st_mtim;
  while(true)
  {
    memset(&fdata,0,sizeof(fdata));
    if(stat(file,&fdata) < 0)
      continue;

    if(mtim.tv_sec != fdata.st_mtime || mtim.tv_nsec != fdata.st_mtime_nsec)
    {
      mtim =  fdata.st_mtim;

      printf("pid: %ld\n",(long)pid);

      pthread_mutex_lock(&mpid);
      pid > 0 &&  kill(pid,SIGKILL);
      if((pid = fork()) < 0)
      {
	perror("fork");
	continue;

      }//if(fork);
      else if(pid == 0)
      {
	printf("inside child\n");
	if(execl(c,basename(strdup(c)),entpt,(char *)0) < 0)
	  err_msg(strerror(errno),0);

	exit(EXIT_FAILURE);
      }//if(pid);
      pthread_mutex_unlock(&mpid);

      printf("%ld CONTINUES\n\n",(long)pthread_self());
    }//if(mtv_s);

  }//while(true); 

}


void err_msg(const char *msg,int quit)
{
  char msgbuf[MSGMAX];
  strcpy(msgbuf,"\033[1;35m[cwatch]\n\033[1;31m");
  strcat(msgbuf,msg);
  strcat(msgbuf,"\033[0m\n");
  write(STDERR_FILENO,msgbuf,strlen(msgbuf));
  if(quit)
    exit(EXIT_FAILURE);   
}
