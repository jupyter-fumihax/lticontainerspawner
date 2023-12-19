
#include "ltictr_rest.h"
#include "ipaddr_tool.h"
#include "https_tool.h"


int    relay_to_client(int sock, SSL* ssl, tList* hdr, Buffer buf);
int    relay_to_server(int sock, SSL* ssl, tList* hdr, Buffer buf, char* proto);
//

char*  get_info_from_header(tList* hdr,  char* key);
char*  get_info_from_cookie(tList* hdr,  char* key);
char*  get_info_from_ltidata(char* mesg, char* key);

void   rest_client_ws_info(tJson* pp);
void   rest_server_ws_info(tJson* pp);
void   rest_client_ws_info_v4(tJson* pp);
void   rest_server_ws_info_v4(tJson* pp);
