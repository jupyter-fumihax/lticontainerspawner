

#include "ltictr_ws.h"
#include "https_tool.h"


void  init_rest_header(void);
void  setup_rest_params(void);

void  post_rest_server(struct ws_info* info);
void  send_rest_data(char* host, int port, int tls, tList* header, int response);

