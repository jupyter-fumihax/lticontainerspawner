/* vi: set tabstop=4 nocindent noautoindent: */

#ifndef _LTICTR_PROXY_SERVER_H
#define _LTICTR_PROXY_SERVER_H


#include "tools.h"
#include "buffer.h"
#include "tlist.h"
#include "network.h"
#include "ssl_tool.h"
#include "ipaddr_tool.h"
#include "protocol.h"
#include "ltictr_signal.h"

#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <dlfcn.h>
#include <errno.h>



///////////////////////////////////////////////////////////////////////////////////

int   main(int argc, char** argv);
void  fork_api_server(void);
int   init_main(Buffer file);
void  term_main(int code);

//void  close_all_socket(tList* lp);


#endif

