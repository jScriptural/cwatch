/************************************************
 * @file cwatch.c
 * @brief watch for changes in file(s)
 * @author Isonguyo John <isonguyojohndeveloper@gmail.com
 * @docs <https://gitub.com/jscripture/cwatch/README.md>
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

void *routine(void *arg);
void loadfilesfromdirs(char *dir[],int *dirc,char *fil[],int *filc);
void loadfiles(char *fil[],int *filc);


int main(int argc, char *argv[])
{
  setvbuf(stdout,NULL,_IONBF,0);
  setvbuf(stderr,NULL,_IONBF,0);
  setvbuf(stdin,NULL,_IONBF,0);

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

  if(f)
    loadfiles(files,&filecount);


  if(e)
  {
    extensions[extcount++] = strtok(e,":");
    while((extensions[extcount] = strtok(NULL,":")) != NULL && extcount < MAXEXTWATCH)
      ++extcount;
  }//if(e)

  if(d)
    loadfilesfromdirs(directories,&dircount,files,&filecount);
  
  if(filecount <= 0 || c == NULL)
    err_msg("NO file to watch",1);

  char buffer[MSGMAX]={0};
  strcpy(buffer,"\033[1;35m[cwatch]\nWatching:\033[0m ");
  for(int i =0; i < filecount; ++i)
  {
    i != 0 &&  strcat(buffer,",");
    strcat(buffer,files[i]);
  }
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
  //
  
}//main




void *routine(void *arg)
{
  char *file =  (char *) arg;
  char *entpt = files[0];
  struct stat fdata;
  struct timespec mtim;

  //get current file modification time
  if(stat(file,&fdata) < 0)
    err_msg(strerror(errno),1);
  mtim = fdata.st_mtim;

  //check for changes
  while(true)
  {
    memset(&fdata,0,sizeof(fdata));
    //continue if the inode changes 
    if(stat(file,&fdata) < 0)
      continue;

    if(mtim.tv_sec != fdata.st_mtim.tv_sec || mtim.tv_nsec != fdata.st_mtim.tv_nsec)
    {
      //update the modification time
      mtim =  fdata.st_mtim;

      pthread_mutex_lock(&mpid);
      /*if process was already running,
       * kill it before restart to avoid 
       * conflict especially with server 
       * programs
       */
      pid > 0 &&  kill(pid,SIGKILL);
      if((pid = fork()) < 0)
      {
	perror("fork");
	//Try again on next chnage
	continue;

      }//if(fork);
      else if(pid == 0)
      {
	if(execl(c,basename(strdup(c)),entpt,(char *)0) < 0)
	  err_msg(strerror(errno),1);

      }//if(pid);

      pthread_mutex_unlock(&mpid);

    }//if(mtv_s);

  }//while(true); 

}


void loadfilesfromdirs(char *dr[],int *dc,char *fl[],int *fc)
{
    DIR *dir;
    struct dirent *entry;
    struct stat sts;
    char file[FILENAME_MAX];

   // printf("d: %s\n",d);
    dr[(*dc)++] = strtok(d,":");
    while((dr[*dc] = strtok(NULL,":")) != NULL && *dc < MAXDIRWATCH)
      ++(*dc);

   /* printf("dc: %d\n",*dc);
    for(int i=0; i < *dc; ++i)
      printf("dir[%d]: %s\n",i,dr[i]);*/

    for(char **dp = dr; *dp != NULL; ++dp)
    {
      //char *dp = dr[i];
      //printf("dir: %s\n",*dp);

      if((dir = opendir(*dp)) == NULL)
	err_msg(strerror(errno),1);

      while((entry = readdir(dir)) != NULL && *fc < MAXFILEWATCH)
      {
	memset(file,0,sizeof(file));
	strncpy(file,*dp,sizeof(file));
	strcat(file,"/");
	strncat(file,entry->d_name,FILENAME_MAX-strlen(file));

	//printf("file: %s\n",file);

	if(stat(file,&sts) < 0)
	  err_msg(strerror(errno),1);
	
	//printf("filecount: %d\n",*fc);

	if(S_ISREG(sts.st_mode))
	{
	  if(!e)
	    fl[(*fc)++] = strdup(file);

	  const char *ext = extname(file);
	  char **p = extensions;

	  for(;*p != NULL; ++p)
	  {
	    if(!strcmp(*p,ext))
	    {
	      fl[(*fc)++] = strdup(file);
	      break;
	    }//if(!strcmp)

	  }//for(p!=null)

	}//if(S_ISREG)

      }//while(readdir)

    }//for(dc)
}

void loadfiles(char *fl[],int *fc)
{
    char *file = strtok(f,":");
    struct stat  fdata;
    if(stat(file,&fdata) < 0)
      err_msg(strerror(errno),1);
    if(S_ISREG(fdata.st_mode))
      fl[(*fc)++] = strdup(file); 

    while((file = strtok(NULL,":")) != NULL && *fc < MAXFILEWATCH)
    {
      memset(&fdata,0,sizeof(fdata));
      if(stat(file,&fdata) < 0)
	err_msg(strerror(errno),1);
      if(S_ISREG(fdata.st_mode))
	fl[(*fc)++] = strdup(file); 
    }//while(file);

}
