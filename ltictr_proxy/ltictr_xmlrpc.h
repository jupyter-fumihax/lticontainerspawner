
/**
この機能が現在使用されていません．

ltictr_rest.h を参照してください．
*/


#include "ltictr_ws.h"
#include "https_tool.h"


void  init_xmlrpc_header(void);
void  setup_xmlrpc_params(void);

void  post_xmlrpc_server(struct ws_info* info);
void  send_xmlrpc_data(char* host, int port, int tls, tList* header, int response);

