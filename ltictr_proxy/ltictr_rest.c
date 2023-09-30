/* vi: set tabstop=4 nocindent noautoindent: */


#include "ltictr_rest.h"


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

extern char*    Rest_Path;
extern char*    Rest_Service;
extern char*    Rest_HTTPver;
extern int      Rest_Response;



///////////////////////////////////////////////////////////////////////
// send data to Moodle Web Service

void  send_rest_data(char* host, int port, int tls, tList* header, int response)
{
    int sock = tcp_client_socket(host, port);
    if (sock<0) {
        DEBUG_MODE {
            print_message("[LTICTR_REST] Failure to connect to server (%s:%d) : ", host, port);
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
        print_message("[LTICTR_REST] === ANS of REST ===\n");
        print_message("%s\n\n", ans);
    }

    if (tls) {
        if (ssl!=NULL) ssl_close(ssl);
        SSL_CTX_free(ctx);
    }
    close(sock);

    return;
}



void  post_rest_server(struct ws_info* info)
{
    setup_rest_params();
    if (ServerPath==NULL || ServerName==NULL) return;

    // data -> HTTP_Header, HTTP_Host
    char* path = ServerPath;
    if (*path=='%') path += 3;
    if (*path=='/') path++;
    //
    char url[LMESG];
    //snprintf(url, LMESG-1, "POST /%s/webservice/rest/server.php?wsfunction=mod_lticontainer_write_nblogs&moodlewsrestformat=json&wstoken=%s HTTP/%s", path, ServerToken, Rest_HTTPver);
    snprintf(url, LMESG-1, "POST /%s%s?wsfunction=%s&wstoken=%s HTTP/%s", path, Rest_Path, Rest_Service, ServerToken, Rest_HTTPver);
    copy_s2Buffer(url, &(HTTP_Header->ldat.val));
    copy_s2Buffer(ServerName, &(HTTP_Host->ldat.val));

    Buffer buf = make_form_urlenc("nb_logs[0][host]", info->host);
    add_form_urlenc(&buf, "nb_logs[0][inst_id]",  info->inst_id);
    add_form_urlenc(&buf, "nb_logs[0][lti_id]",   info->lti_id);
    add_form_urlenc(&buf, "nb_logs[0][session]",  info->session);
    add_form_urlenc(&buf, "nb_logs[0][message]",  info->message);
    add_form_urlenc(&buf, "nb_logs[0][status]",   info->status);
    add_form_urlenc(&buf, "nb_logs[0][username]", info->username);
    add_form_urlenc(&buf, "nb_logs[0][cell_id]",  info->cell_id);
    add_form_urlenc(&buf, "nb_logs[0][tags]",     info->tags);
    add_form_urlenc(&buf, "nb_logs[0][date]",     info->date);
    //
    copy_i2Buffer((int)buf.vldsz, &(HTTP_Length->ldat.val));
    copy_Buffer(&buf, &(HTTP_Data->ldat.val));

    send_rest_data(ServerName, ServerPort, ServerTLS, HTTP_Header, Rest_Response);

    freenull(ServerName);
    free_Buffer(&buf);
}



void  init_rest_header(void)
{
    tList* pp = NULL;
    pp = HTTP_Header = add_tList_node_str(pp, HDLIST_FIRST_LINE_KEY, "");
    pp = HTTP_Host   = add_tList_node_str(pp, "Host", "");
    //pp               = add_tList_node_str(pp, "Content-Type", "text/html");
    pp               = add_tList_node_str(pp, "Content-Type", "application/x-www-form-urlencoded");
    pp = HTTP_Length = add_tList_node_str(pp, "Content-Length", "");
    pp               = add_tList_node_str(pp, "Connection", "close");
    pp               = add_tList_node_str(pp, HDLIST_END_KEY, "");
    pp = HTTP_Data   = add_tList_node_str(pp, HDLIST_CONTENTS_KEY,  "");

    return;
}


void  setup_rest_params(void)
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

