#ifndef _ECHO_H_
#define _ECHO_H_


#include "csapp.h"
#include <string.h>
#define N 3
#define MAX 500

int isCmd (char *buf);
int decoupe (char *buf,char ** args);
int tailF (int fd);
void telechargeFile (int clientfd,char *cmd);

#endif
