/* vi: set tabstop=4 nocindent noautoindent: */

/**
この機能が現在使用されていません．

ltictr_rest.c を参照してください．
*/

#include "ltictr_xmlrpc.h"


tList*   HTTP_Header = NULL;
tList*   HTTP_Host   = NULL;
tList*   HTTP_Length = NULL;
tList*   HTTP_Data   = NULL;



char*    ServerName = NULL;
int      ServerPort = 443;
int      ServerTLS  = TRUE;


extern char*    ServerURL;
extern char*    ServerPath;
extern char*    ServerToken;

extern char*    XmlRpc_Path;
extern char*    XmlRpc_Service;
extern char*    XmlRpc_HTTPver;
extern int      XmlRpc_Response;



///////////////////////////////////////////////////////////////////////
// send data to Moodle Web Service

void  send_xmlrpc_data(char* host, int port, int tls, tList* header, int response)
{
    int sock = tcp_client_socket(host, port);
    if (sock<0) {
        DEBUG_MODE {
            print_message("[LTICTR_XMLRPC] Failure to connect to server (%s:%d) : ", host, port);
            jbxl_fprint_state(stderr, sock);
        }
        return;
    }
    SSL_CTX* ctx = NULL;
    SSL* ssl     = NULL;

    if (tls) {
        ctx = ssl_client_setup(NULL);
        ssl = ssl_client_socket(sock, ctx, OFF);
    }

    DEBUG_MODE print_tList(stderr, header);
    send_https_header(sock, ssl, header, ON);
    if (response) { 
        char ans[RECVBUFSZ];
        ssl_tcp_recv(sock, ssl, ans, RECVBUFSZ-1);
        print_message("[LTICTR_XMLRPC] === ANS of XMLRPC ===\n");
        print_message("%s\n\n", ans);
    }

    if (tls) {
        if (ssl!=NULL) ssl_close(ssl);
        SSL_CTX_free(ctx);
    }
    close(sock);

    return;
}



void  post_xmlrpc_server(struct ws_info* info)
{
    setup_xmlrpc_params();
    if (ServerPath==NULL || ServerName==NULL) return;

    // data -> HTTP_Header, HTTP_Host
    char* path = ServerPath;
    if (*path=='%') path += 3;
    if (*path=='/') path++;
    //
    char url[LMESG];
    snprintf(url, LMESG-1, "POST /%s%s?wstoken=%s HTTP/%s", path, XmlRpc_Path, ServerToken, XmlRpc_HTTPver);
    copy_s2Buffer(url, &(HTTP_Header->ldat.val));
    copy_s2Buffer(ServerName, &(HTTP_Host->ldat.val));

    tXML* xml = NULL;
    xml = xml_rpc_add_member(xml, "host",     info->host, "");
    xml = xml_rpc_add_member(xml, "inst_id",  info->inst_id, "");
    xml = xml_rpc_add_member(xml, "lti_id",   info->lti_id, "");
    xml = xml_rpc_add_member(xml, "session",  info->session, "");
    xml = xml_rpc_add_member(xml, "message",  info->message, "");
    xml = xml_rpc_add_member(xml, "status",   info->status, "");
    xml = xml_rpc_add_member(xml, "username", info->username, "");
    xml = xml_rpc_add_member(xml, "cell_id",  info->cell_id, "");
    xml = xml_rpc_add_member(xml, "tags",     info->tags, "");
    xml = xml_rpc_add_member(xml, "date",     info->date, "");
    xml = xml_rpc_end_member(xml);
    
    // data -> HTTP_Length, HTTP_Data
    Buffer buf = xml_rpc_request_pack(XmlRpc_Service, xml);
    copy_i2Buffer((int)buf.vldsz, &(HTTP_Length->ldat.val));
    copy_Buffer(&buf, &(HTTP_Data->ldat.val));

    send_xmlrpc_data(ServerName, ServerPort, ServerTLS, HTTP_Header, XmlRpc_Response);

    freenull(ServerName);
    free_Buffer(&buf);
    del_xml(&xml);
}



void  init_xmlrpc_header(void)
{
    tList* pp = NULL;
    pp = HTTP_Header = add_tList_node_str(pp, HDLIST_FIRST_LINE_KEY, "");
    pp = HTTP_Host   = add_tList_node_str(pp, "Host", "");
    pp               = add_tList_node_str(pp, "Content-Type", "text/html");
    pp = HTTP_Length = add_tList_node_str(pp, "Content-Length", "");
    pp               = add_tList_node_str(pp, "Connection", "close");
    pp               = add_tList_node_str(pp, HDLIST_END_KEY, "");
    pp = HTTP_Data   = add_tList_node_str(pp, HDLIST_CONTENTS_KEY,  "");

    return;
}


void  setup_xmlrpc_params(void)
{
    char* p;
    char* s;
    char  b;

    if (ServerURL==NULL) return;

    p = ServerURL;
    s = p;
    while (*p!='%' && *p!=':' && *p!='\0') p++;
    b  = *p;
    *p = '\0';
    if (!strncmp("https", s, 5)) ServerTLS = TRUE;
    else                         ServerTLS = FALSE;
    *p = b;
    if      (*p=='%') p += 3;        // :
    else if (*p==':') p++;
    //
    if      (*p=='%') p += 3;        // /
    else if (*p=='/') p++;
    if      (*p=='%') p += 3;        // /
    else if (*p=='/') p++;

    freenull(ServerName);
    s = p;
    while (*p!='%' && *p!=':' && *p!='\0') p++;
    b  = *p;
    *p = '\0';
    ServerName = dup_str(s);
    *p = b;
    if      (*p=='%') p += 3;
    else if (*p==':') p++;

    s = p;
    ServerPort = atoi(s);

    return;
}

