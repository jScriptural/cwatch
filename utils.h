#ifndef UTILS_H
#define UTILS_H

extern char *f;
extern char *d;
extern char *e;
extern char *c;

/*[get_options]- a wrapper function over the 
 * standard 'getopt' function.
 */



void get_options(int argc,char *argv[],const char *optstring);



#endif
