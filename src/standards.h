/*****************************************************************************
*****************************************************************************/

#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/io.h>
#include	<fcntl.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<signal.h>
#include	<sys/types.h>
#include	<sys/ipc.h>
#include	<sys/shm.h>
#include	<dlfcn.h>
#include 	<sys/types.h>
#include 	<sys/socket.h>
#include	<netdb.h>
#include	<netinet/in.h>
#include	<sys/time.h>

/* Testing Values */
#define	TRUE	1
#define FALSE	0
#define OK	0
#define ERROR	-1
