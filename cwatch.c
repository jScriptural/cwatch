/************************************************
 * @file cwatch.c
 * @brief watch for changes in file(s)
 * @author Isonguyo John <isonguyojohndeveloper@gmail.com
 * @docs <https://gitub.com/jscripture/cwatch/README.md>
 *
 ************************************************/


#include "cwatch.h"
#include "utils.h"

char *files[MAXFILEWATCH] = {0};
int filecount = 0;

char *directories[MAXDIRWATCH] = {0};
int dircount = 0;

char *extensions[MAXEXTWATCH] = {0};
int extcount = 0;

pid_t pid = -1;
pthread_mutex_t mpid = PTHREAD_MUTEX_INITIALIZER;


char *f = NULL;
char *d = NULL;
char *e = NULL;
char *c = NULL;

void *thread_routine(void *arg);

void load_files_from_dirs(char *dir[],int *dirc,char *fil[],int *filc);
void load_files(char *fil[],int *filc);
void sigchld_handler(int signo);

int main(int argc, char *argv[])
{
  char buffer[MSGMAX]={0};

  //Disable buffering for standard I/O 
  setvbuf(stdout,NULL,_IONBF,0);
  setvbuf(stderr,NULL,_IONBF,0);
  setvbuf(stdin,NULL,_IONBF,0);


  //NO Zombies allowed 
  if(signal(SIGCHLD,sigchld_handler)==SIG_ERR)
  {
    perror("signal");
    exit(EXIT_FAILURE);
  }

  //process commandline options
  get_options(argc,argv,"f:d:e:c:");

  if(f != NULL)
    load_files(files,&filecount);


  if(e != NULL)
  {
    extensions[extcount++] = strtok(e,":");
    while((extensions[extcount] = strtok(NULL,":")) != NULL && extcount < MAXEXTWATCH)
      ++extcount;
  }//if(e)
   

  if(d != NULL)
    load_files_from_dirs(directories,&dircount,files,&filecount);
  

  if(filecount <= 0)
    err_msg("NO file to watch",1);

  if(c == NULL)
    err_msg("cwatch: No command to run",1);




  //write to stdout the files being watched
  strcpy(buffer,"\033[1;35m[cwatch]\nWatching:\033[0m ");
  for(int i =0; i < filecount; ++i)
  {
    if(i != 0)
      strcat(buffer,",");

    strcat(buffer,files[i]);
  }
  strcat(buffer,"\n\n");
  write(STDOUT_FILENO,buffer,strlen(buffer));

  //spawn a thread to watch each file 
  //for modification.
  for(int i = 0; i < filecount; ++i)
  {
    pthread_t tid;
    pthread_create(&tid,NULL,thread_routine,(void*)files[i]);
    pthread_detach(tid);
  }//for(filecount)


  //pause();
  pthread_exit(NULL);
  //
  
}//main




void *thread_routine(void *arg)
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
    if(lstat(file,&fdata) < 0)
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
      if(pid > 0)
	kill(pid,SIGKILL);

      if((pid = fork()) < 0)
      {
	perror("fork");
	//Try again on next chnage
	continue;

      }//if(fork);
      else if(pid == 0)
      {
	if(execlp(c,basename(strdup(c)),entpt,(char *)0) < 0)
	  err_msg(strerror(errno),1);

      }//if(pid);

      pthread_mutex_unlock(&mpid);

    }//if(mtv_s);

  }//while(true); 

}


void load_files_from_dirs(char *dr[],int *dc,char *fl[],int *fc)
{
    DIR *dir;
    struct dirent *entry;
    struct stat sts;
    char file[FILENAME_MAX];

    dr[(*dc)++] = strtok(d,":");

    while(*dc < MAXDIRWATCH && (dr[*dc] = strtok(NULL,":")) != NULL)
      ++(*dc);


    for(char **dp = dr; *dp != NULL; ++dp)
    {

      if((dir = opendir(*dp)) == NULL)
	err_msg(strerror(errno),1);

      while(*fc < MAXFILEWATCH && (entry = readdir(dir)) != NULL)
      {
	memset(file,0,sizeof(file));
	strncpy(file,*dp,sizeof(file));
	strcat(file,"/");
	strncat(file,entry->d_name,FILENAME_MAX-strlen(file));

	//printf("file: %s\n",file);

	if(lstat(file,&sts) < 0)
	  err_msg(strerror(errno),1);
	
	//printf("filecount: %d\n",*fc);

	if(S_ISREG(sts.st_mode))
	{

	  char **p = extensions;
	  char ext[FILENAME_MAX] = {0};
	  get_extension(file,ext,FILENAME_MAX);

	  //if no -e option was provided
	  //watch all regular files from the
	  //directories specified
	  if(e == NULL)
	    fl[(*fc)++] = strdup(file);

	  //else only watch files with the 
	  //provided extension
	  for(;*p != NULL && ext[0] != '\0'; ++p)
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

void load_files(char *fl[],int *fc)
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


void sigchld_handler(int signo)
{
  //Wait for terminated child process
  //so they do not become zombies.
  waitpid(-1, NULL, 0);
}
