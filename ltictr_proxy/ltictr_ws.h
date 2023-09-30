

#include "buffer.h"
#include "tjson.h"


struct  ws_info {
    char*  host;
    char*  inst_id;
    char*  lti_id;
    //
    char*  session;
    char*  message;
    char*  status;
    char*  username;
    char*  cell_id;
    char*  tags;
    char*  date;
};


//tJson* ws_json(Buffer* buf, char* mesg, int cc);
tJson* ws_json_parse(ringBuffer* ring);
tJson* ws_json_client(unsigned char* mesg, int cc);
tJson* ws_json_server(unsigned char* mesg, int cc);

//void  print_buf(FILE* fp, unsigned char* buf, int size);
