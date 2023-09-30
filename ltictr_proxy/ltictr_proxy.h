
#include "ipaddr_tool.h"
#include "https_tool.h"



void   receipt_proxy(int ssock, SSL_CTX* s_ctx, SSL_CTX* c_ctx, Buffer api_host, tList* lp, int chunked);
//
int    get_proxy_socket(tList* lst);
SSL*   get_proxy_ssl(int sock, SSL_CTX* ctx, tList* lst);
char*  get_proxy_username(tList* hdr);
Buffer get_proxy_target(char* apiserver, int port, SSL_CTX* ctx, char* uname, char* token);

int    proc_chunked(int fromfd, SSL* fromssl, int tofd, SSL* tossl, Buffer buf, char* proto, int tm, int relay);

