#include "utils.h"
#include <unistd.h>
#include <getopt.h>



void get_options(int argc, char *argv[],const char *optstr)
{
  opterr = 0;
  int opt;

  while((opt = getopt(argc,argv,optstr)) != -1)
  {
    switch(opt)
    {
      case 'f': f = optarg; break;
      case 'd': d = optarg; break;
      case 'e': e = optarg; break;
      case 'c': c = optarg; break;
    }
  }
  
}
