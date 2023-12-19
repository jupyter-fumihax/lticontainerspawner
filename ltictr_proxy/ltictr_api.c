
#include "ltictr_api.h"
#include "ltictr_signal.h"
#include "tjson.h"


#define  LTICTR_API_TOKEN_PRE   "token "

extern char*  API_Token;



int  api_main_process(int sock, SSL* ssl, tList* lproxy)
{
    tList* hdr = NULL;              // 受信ヘッダ
    Buffer buf = init_Buffer();     // 受信ボディ

    //
    buf = make_Buffer(RECVBUFSZ);
    int ret = recv_https_Buffer(sock, ssl, &hdr, &buf, HTTP_TIMEOUT, NULL, NULL, FALSE);
    if (ret<=0) {                   // 0 は正常切断
        del_tList(&hdr);
        free_Buffer(&buf);
        if (ret<0) send_http_error(sock, ssl, 400, NULL);
        return -1;
    }

    /*
    DEBUG_MODE {
        print_message("[LTICTR_API] === API RECV HEADER ===\n");
        print_protocol_header(hdr, OFF);
        print_message("\n");
    }*/

    int com = get_http_header_method(hdr);
    if (com <= HTTP_UNKNOWN_METHOD) {
        del_tList(&hdr);
        free_Buffer(&buf);
        send_http_error(sock, ssl, 400, NULL);
        return -1;
    }

    // Check Token String
    int check_token = OFF;
    Buffer tbuf = search_protocol_header(hdr, "Authorization", 1);
    char* token = (char*)tbuf.buf;
    if (token!=NULL) {
        if (ex_strcmp(LTICTR_API_TOKEN_PRE, token)) {
            token += strlen(LTICTR_API_TOKEN_PRE);
            if (!strcmp(API_Token, token)) check_token = ON;
        }
    }
    free_Buffer(&tbuf);
    if (check_token==OFF) {
        del_tList(&hdr);
        free_Buffer(&buf);
        DEBUG_MODE print_message("[LTICTR_API] Missmatch Token string!\n");
        send_http_error(sock, ssl, 401, NULL);
        return -1;
    }

    // Get User Name
    char* uname = get_api_username(hdr);
    del_tList(&hdr);
    if (uname==NULL) {
        free_Buffer(&buf);
        if (com<=HTTP_UNKNOWN_METHOD) send_http_error(sock, ssl, 400, NULL);
        else                          send_http_error(sock, ssl, 404, NULL);
        return -1;
    }

    // HTTP Method
    int err = 0;
    // GET
    if (com==HTTP_GET_METHOD) {
        Buffer res = init_Buffer();
        ret = api_get_user(uname, &res, lproxy);
        if (ret==0) {
            send_http_response(sock, ssl, 200, &res);
        }
        else {
            free_Buffer(&res);
            send_http_error(sock, ssl, ret, NULL);
            err = -1;
        }
        free_Buffer(&res);
    }
    // POST
    else if (com==HTTP_POST_METHOD) {
        ret = api_add_user(uname, buf, lproxy);
        if (ret==0) {
            send_http_response(sock, ssl, 201, NULL);
        }
        else {
            err = -1;
            send_http_error(sock, ssl, ret, NULL);
        }
    }
    // DELETE
    else if (com==HTTP_DELETE_METHOD) {
        ret = api_del_user(uname, lproxy);
        if (ret==0) {
            send_http_response(sock, ssl, 204, NULL);
        }
        else {
            err = -1;
            send_http_error(sock, ssl, ret, NULL);
        }
    }
    else {
        print_message("[LTICTR_API] Not Supported Method : %d \n", com);
        err = -1;
        Buffer opt = make_Buffer_str("GET, POST, DELETE");
        send_http_error(sock, ssl, 405, &opt);          // Method Not Allowed
        free_Buffer(&opt);
    }

    //
    free(uname);
    free_Buffer(&buf);
    //
    return err;
}



//
// GET よる ユーザ uname の問い合わせの返答データを buf に格納する．
//
int  api_get_user(char* uname, Buffer* buf, tList* lproxy)
{
    char json_data[LDATA]; 
    char json_root_fmt[] = "{\"/\":{\"hub\":true,\"target\":\"%s:%d\",\"jupyterhub\":true,\"last_activity\":\"%s\"}}";
    char json_user_fmt[] = "{\"/user/%s\":{\"user\":\"%s\",\"server_name\":\"\",\"target\":\"%s:%d\",\"jupyterhub\":true,\"last_activity\":\"%s\"}}";

    *buf = init_Buffer();
    tList* pp  = NULL;
    tJson* res = json_parse_prop(NULL, "{}", 0);

    if (lproxy!=NULL && lproxy->ldat.id==TLIST_ANCHOR_NODE) lproxy = lproxy->next;

    if (!strcmp("/", uname)) {
        pp = lproxy;
        while (pp!=NULL) {
            char* user = (char*)pp->ldat.key.buf;
            if (user!=NULL) {
                char* url  = (char*)pp->ldat.val.buf;
                char* date = (char*)pp->ldat.ptr;
                int   port = pp->ldat.lv;
                memset(json_data, 0, LDATA);

                if (!strcmp("/", user)) {
                    snprintf(json_data, LDATA-1, json_root_fmt, url, port, date);
                }
                else {
                    snprintf(json_data, LDATA-1, json_user_fmt, user, user, url, port, date);
                }
                tJson* json = json_parse_prop(NULL, json_data, 2);
                insert_json_nodes(res, json);
            }
            pp = pp->next;
        }
    }
    else {
        pp = strncasecmp_tList(lproxy, uname, 0, 1);
        if (pp!=NULL) {
            //
            char* user = (char*)pp->ldat.key.buf;
            char* url  = (char*)pp->ldat.val.buf;
            char* date = (char*)pp->ldat.ptr;
            int   port = pp->ldat.lv;
            memset(json_data, 0, LDATA);
            //
            snprintf(json_data, LDATA-1, json_user_fmt, user, user, url, port, date);
            tJson* json = json_parse_prop(NULL, json_data, 2);
            insert_json_nodes(res, json);
        }
    }

    *buf = json_inverse_parse(res, JSON_ONELINE_FORMAT);
    del_json(&res);

    int ret = 0;
    if (buf->buf==NULL) ret = 500;
    return ret;
}



//
// ユーザ uname を lproxy リストに追加．
// buf : POST データ
//
/**
{
  "user": "bob", 
  "server_name": "", 
  "target": "http://127.0.0.1:49168", 
  "jupyterhub": true
}
*/
int  api_add_user(char* uname, Buffer buf, tList* lproxy)
{
    Buffer target = init_Buffer();
    Buffer protocol, srvfqdn;
    unsigned short int port;

    tJson* json = json_parse_prop(NULL, (char*)buf.buf, 2);
    if (json==NULL) return 400;    // analyze error

    // user name check
    if (strcmp("/", uname)) {
        Buffer user = get_key_json_val(json, "user", 1);
        if (user.buf==NULL || strcmp((char*)uname, (char *)user.buf)) {
            del_json(&json);
            free_Buffer(&user);
            return 400;                // not match
        }
        free_Buffer(&user);
    }
 
    //
    target = get_key_json_val(json, "target", 1);
    del_json(&json);
    if (target.buf==NULL) return 400;

    decomp_url(target, NULL, &protocol, &srvfqdn, &port, NULL);
    free_Buffer(&target);
    cat_s2Buffer("://",  &protocol);
    cat_Buffer(&srvfqdn, &protocol);
    free_Buffer(&srvfqdn);

    // check of user exist
    tList* exist = strncasecmp_tList(lproxy, uname, 0, 1);
    if (exist!=NULL) del_tList_node(&exist);
    //
    lproxy = find_tList_end(lproxy);
    char* lasttime = get_local_timestamp(time(0), "%Y-%b-%dT%H:%M:%SZ");
    add_tList_node_bystr(lproxy, 0, (int)port, uname, (char*)protocol.buf, lasttime, strlen(lasttime)+1);
    free(lasttime);
    free_Buffer(&protocol);

    //
    DEBUG_MODE {
        print_message("[LTICTR_API] === ADD User === (%s)\n", uname);
    //    print_tList(stderr, lproxy);
    //    print_message("\n");
    }
    return 0;
}


//
// ユーザ uname を lproxy リストから削除．
//
int  api_del_user(char* uname, tList* lproxy)
{
    tList* pp = strncasecmp_tList(lproxy, uname, 0, 1);
    if (pp==NULL) return 404;      // user does not exist

    del_tList_node(&pp);
    
    //
    DEBUG_MODE {
        print_message("[LTICTR_API] === DEL User === (%s)\n", uname);
    //    print_tList(stderr, lproxy);
    //    print_message("\n");
    }
    return 0;
}



////////////////////////////////////////////////////////////////////////
// HTTP
//

//
int  send_http_response(int sock, SSL* ssl, int num, Buffer* buf)
{
    tList* hdr = NULL;
    tList* lst = NULL;

    if (num==200) {
        lst = hdr = add_tList_node_str(NULL, HDLIST_FIRST_LINE_KEY, "HTTP/1.1 200 OK");
        lst = add_tList_node_str(lst, "Content-Type", "application/json");
        lst = add_tList_node_str(lst, "Content-Length", "0");
    }
    else if (num==201) {
        lst = hdr = add_tList_node_str(NULL, HDLIST_FIRST_LINE_KEY, "HTTP/1.1 201 Created");
        lst = add_tList_node_str(lst, "Content-Length", "0");
    }
    else if (num==204) {
        lst = hdr = add_tList_node_str(NULL, HDLIST_FIRST_LINE_KEY, "HTTP/1.1 204 Not Content");
    }

    lst = add_tList_node_str(lst, "Connection", "close");
    char* date = get_http_header_date(time(0));
    if (date!=NULL) {
        lst = add_tList_node_str(lst, "Date", date);
        free(date);
    }

    int cc = send_https_Buffer(sock, ssl, hdr, buf);

    /*
    DEBUG_MODE {
        print_message("[LTICTR_API] === SEND Data ===\n");
        print_protocol_header(hdr, OFF);
        print_message("\n");
    }*/

    del_tList(&hdr);
    return cc;
}



int  send_http_error(int sock, SSL* ssl, int err, Buffer* opt)
{
    tList* hdr = NULL;
    tList* lst = NULL;

    if      (err==400) {
        lst = hdr = add_tList_node_str(NULL, HDLIST_FIRST_LINE_KEY, "HTTP/1.1 400 Bad Request");
    }
    else if (err==401) {
        lst = hdr = add_tList_node_str(NULL, HDLIST_FIRST_LINE_KEY, "HTTP/1.1 401 Unauthorized");
    }
    else if (err==404) {
        lst = hdr = add_tList_node_str(NULL, HDLIST_FIRST_LINE_KEY, "HTTP/1.1 404 Not Found");
    }
    else if (err==405) {
        lst = hdr = add_tList_node_str(NULL, HDLIST_FIRST_LINE_KEY, "HTTP/1.1 405 Method Not Allowed");
        if (opt!=NULL) {
            lst = add_tList_node_str(lst, "Allow",  (char*)opt->buf);
        }
    }
    else if (err==500) {
        lst = hdr = add_tList_node_str(NULL, HDLIST_FIRST_LINE_KEY, "HTTP/1.1 500 Internal Server Error");
    }
    else {
        lst = hdr = add_tList_node_str(NULL, HDLIST_FIRST_LINE_KEY, "HTTP/1.1 400 Bad Request");
    }
    //
    lst = add_tList_node_str(lst, "Connection", "close");
 
    int cc = send_https_header(sock, ssl, hdr, OFF);
    del_tList(&hdr);

    return cc;
}



#define  LTICTR_API_ROUTES  "/api/routes"
#define  LTICTR_API_USER    "/api/routes/user/"


char*  get_api_username(tList* hdr)
{
    if (hdr==NULL) return NULL;

    char*  path = NULL;
    Buffer hbuf = search_protocol_header(hdr, (char*)HDLIST_FIRST_LINE_KEY, 1);

    if (hbuf.buf!=NULL) {
        path = cawk((char*)hbuf.buf, ' ', 2);
        if (path!=NULL && *path!='/') {
            free(path);
            path = NULL;
        }
        free_Buffer(&hbuf);
    }
    if (path==NULL) return NULL;

    char* uname = NULL;

    if (ex_strcmp(LTICTR_API_ROUTES, path)) {
        int len = strlen(LTICTR_API_ROUTES);
        if ((path[len]=='\0') ||  (path[len]=='/' && path[len+1]=='\0')) {
            uname = dup_str("/");
        }
        else if (ex_strcmp(LTICTR_API_USER, path)) {
            len = strlen(LTICTR_API_USER);
            uname = dup_str(path + len);
        }
    }
    free(path);

    return uname;
}

