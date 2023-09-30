
#include "https_tool.h"


/** 
@code
tList_data 構造体
    int                 id;      ソケットデータ．
    int                 lv;      ポート番号．負数の場合は SSL接続
    Buffer              key;     ユーザ名．
    Buffer              val;     接続先 FQDN または IPアドレス．
    void*               ptr;     日付
    int                 sz;      *ptr のサイズ．strlen(*ptr)
    struct _tList*      lst; 
@endcode
*/


int    api_server(int port, SSL_CTX* ctx,  tList* lproxy);
int    api_main_process(int sock, SSL* ssl, tList* lproxy);

int    api_get_user(char* uname, Buffer* res, tList* lproxy);
int    api_add_user(char* uname, Buffer  buf, tList* lproxy);
int    api_del_user(char* uname, tList*  lproxy);

int    send_http_response(int sock, SSL* ssl, int num, Buffer* buf);
int    send_http_error(int sock, SSL* ssl, int err, Buffer* opt);

char*  get_api_username(tList* hdr);
