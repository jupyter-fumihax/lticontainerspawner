/*
    Proxy Server for JupyterHub and LTIContainerSpawner 
        
                by Fumi.Iseki '22 02/12   BSD License.
                              '22 06/20   v1.1.0
                              '22 07/07   v1.2.0
                              '23 04/25   v1.2.1  fix ltictr_xmlrpc.c
                              '23 08/27   v1.3.0  xmlrpc -> rest
*/

#define  LTICTR_PROXY_VERSION   "1.3.0"


#include "ltictr_proxy.h"
#include "ltictr_proxy_server.h"


#define  LTICTR_PID_FILE        "LTICTR_PID_File"
#define  LTICTR_SERVER_CERT     "LTICTR_Server_Cert"
#define  LTICTR_SERVER_CHAIN    "LTICTR_Server_Chain"
#define  LTICTR_PRIVATE_KEY     "LTICTR_Private_Key"
#define  LTICTR_API_TOKEN       "LTICTR_API_Token"

#define  REST_PATH_KEY          "Rest_Path"
#define  REST_SERVICE_KEY       "Rest_Service"
#define  REST_HTTPVER_KEY       "Rest_HTTPver"
#define  REST_RESPONSE_KEY      "Rest_Response"

#define  API_SERVER_NAME        "ltictr_api_server"


pid_t    RootPID;
pid_t    APIChildPID    = 0;

int      APIServerExec  = ON;
int      ChunkedMode    = TRUE;

int      Nofd = 0, Sofd = 0;
int      ServerSSL      = OFF;     // クライアント側（自身はサーバ）とのSSL 接続
SSL_CTX* ServerCTX      = NULL;
SSL_CTX* ClientCTX      = NULL;

tList*   ProxyList      = NULL;
tList*   PIDList        = NULL;

char**   PTR_argv       = NULL;

int      NoSigchld      = OFF;
int      PendingSigchld = 0;

//int      Logtype        = LOG_ERR;

//default config value
char*    PIDFile       = "/var/run/ltictr_proxy.pid";
char*    TLS_CertPem   = "/etc/pki/tls/certs/server.pem";
char*    TLS_ChainPem  = NULL;
char*    TLS_KeyPem    = "/etc/pki/tls/private/key.pem";
char*    API_Token     = "default_token";

char*    Rest_Path     = "/webservice/rest/server.php";
char*    Rest_Service  = "mod_lticontainer_write_nblogs";
char*    Rest_HTTPver  = "1.1";
int      Rest_Response = FALSE;



//
int main(int argc, char** argv)
{
    int    version = OFF;
    int    sport = 0;
    struct passwd* pw;

    ProxyList = add_tList_node_anchor();
    PIDList   = add_tList_node_anchor();

    PTR_argv  = argv;

    Buffer hosturl;
    Buffer apiurl;
    Buffer efctvuser;
    Buffer pidfile;
    Buffer certfile;
    Buffer chainfile;
    Buffer keyfile;
    Buffer configfile;

    // for arguments
    hosturl    = init_Buffer();
    apiurl     = init_Buffer();
    efctvuser  = init_Buffer();
    pidfile    = init_Buffer();
    certfile   = init_Buffer();
    chainfile  = init_Buffer();
    keyfile    = init_Buffer();
    configfile = init_Buffer();

    for (int i=1; i<argc; i++) {
        if      (!strcmp(argv[i],"-p")) {if (i!=argc-1) sport = atoi(argv[i+1]);}
        else if (!strcmp(argv[i],"-h")) {if (i!=argc-1) hosturl   = make_Buffer_bystr(argv[i+1]);}
        else if (!strcmp(argv[i],"-a")) {if (i!=argc-1) apiurl    = make_Buffer_bystr(argv[i+1]);}
        else if (!strcmp(argv[i],"-u")) {if (i!=argc-1) efctvuser = make_Buffer_bystr(argv[i+1]);}
        else if (!strcmp(argv[i],"-c")) ServerSSL = ON;
        else if (!strcmp(argv[i],"-d")) DebugMode = ON;
        else if (!strcmp(argv[i],"--version")) version = ON;

        else if (!strcmp(argv[i],"-n"))          APIServerExec = OFF;
        else if (!strcmp(argv[i],"--noexecapi")) APIServerExec = OFF; 
        else if (!strcmp(argv[i],"--chunked"))   ChunkedMode   = TRUE;

        else if (!strcmp(argv[i],"--pid"))    {if (i!=argc-1) pidfile    = make_Buffer_bystr(argv[i+1]);}
        else if (!strcmp(argv[i],"--cert"))   {if (i!=argc-1) certfile   = make_Buffer_bystr(argv[i+1]);}
        else if (!strcmp(argv[i],"--chain"))  {if (i!=argc-1) chainfile  = make_Buffer_bystr(argv[i+1]);}
        else if (!strcmp(argv[i],"--key"))    {if (i!=argc-1) keyfile    = make_Buffer_bystr(argv[i+1]);}
        else if (!strcmp(argv[i],"--conf"))   {if (i!=argc-1) configfile = make_Buffer_bystr(argv[i+1]);}
        else if (!strcmp(argv[i],"--config")) {if (i!=argc-1) configfile = make_Buffer_bystr(argv[i+1]);}
        //
        else if (*argv[i]=='-') {
            print_message("[LTICTR_PROXY_SERVER] Unknown argument: %s\n", argv[i]);
            sport = 0;
            break;
        }
    }
    if (version==ON) {
        printf("%s\n", LTICTR_PROXY_VERSION);
        exit(0);
    }
    if (sport==0) {
        print_message("Usage... %s -p client_side_port [-c] [-h host_url[:port]] [-a [api_url:]port] [-u user] [-d] \n", argv[0]);
        print_message("               [--noexecapi] [--chunked] [--conf config_file] [--cert cert_file] [--key key_file] [--chain chain_file] [--pid pid_file]\n");
        exit(1);
    }
    //
    //openlog("ltictr_proxy_server", LOG_PERROR|LOG_PID, LOG_AUTH);

    int cport = 0;
    if (hosturl.buf!=NULL) {
        int sz = (int)strlen((char*)hosturl.buf) - 1; 
        while(sz>=0 && hosturl.buf[sz]!='/' && hosturl.buf[sz]!=':') sz--;
        if (sz>=0 && hosturl.buf[sz]==':') {
            cport = atoi((char*)&(hosturl.buf[sz+1]));
            hosturl.buf[sz] = '\0';
            hosturl.vldsz = strlen((char*)hosturl.buf);
        }
    }
    if (cport==0) cport = sport;
    if (hosturl.buf!=NULL) {
        if (!ex_strcmp("http://", (char*)hosturl.buf) && !ex_strcmp("https://", (char*)hosturl.buf)) {
            ins_s2Buffer("http://", &hosturl);
        }
        add_tList_node_bystr(ProxyList, 0, cport, "/", (char*)hosturl.buf, NULL, 0);
        DEBUG_MODE print_message("[LTICTR_PROXY_SERVER] Target Server is %s:%d\n", (char*)hosturl.buf, cport);
    }

    //
    if (apiurl.buf!=NULL) {
        if (strstr((char*)apiurl.buf, ":")==NULL) ins_s2Buffer(":", &apiurl);
        if (!ex_strcmp("http://", (char*)apiurl.buf) && !ex_strcmp("https://", (char*)apiurl.buf)) {
            ins_s2Buffer("http://", &apiurl);
        }
        DEBUG_MODE print_message("[LTICTR_PROXY_SERVER] API Server is %s\n", (char*)apiurl.buf);
    }
    else {
        APIServerExec = OFF;
    }

    //
    if (pidfile.buf !=NULL)  PIDFile      = (char*)pidfile.buf;
    if (certfile.buf!=NULL)  TLS_CertPem  = (char*)certfile.buf;
    if (chainfile.buf!=NULL) TLS_ChainPem = (char*)chainfile.buf;
    if (keyfile.buf !=NULL)  TLS_KeyPem   = (char*)keyfile.buf;

    //
    // Initialization
    DEBUG_MODE print_message("[LTICTR_PROXY_SERVER] Start initialization.\n");
    if (configfile.buf!=NULL) {
        if (!file_exist((char*)configfile.buf)) {
            //syslog(Logtype, "Failure to check configuration file (%s). Can not read the configuration file.", (char*)configfile.buf);
            print_message("[LTICTR_PROXY_SERVER] Failure to check configuration file (%s). Can not read the configuration file.\n", (char*)configfile.buf);
            sig_term(-1);
        }
    }
    init_main(configfile);
    free_Buffer(&configfile);
    DEBUG_MODE print_message("[LTICTR_PROXY_SERVER] Initialization is finished.\n");

    //
    // Signal handling
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sig_term;
    sa.sa_flags   = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGHUP,  &sa, NULL);
    sigaction(SIGINT,  &sa, NULL);      // ^C
    sigaction(SIGTERM, &sa, NULL);
    #
    set_sigterm_child(sig_child);       // Setting of proxy process is terminated
    set_sigsegv_handler(sig_segmen);

    //
    // PID file
    RootPID = getpid();
    FILE* fp = fopen((char*)PIDFile, "w");
    if (fp!=NULL) {
        fprintf(fp, "%d", (int)RootPID);
        fclose(fp);
    }

    //
    // Change effective user
    if (efctvuser.buf!=NULL) {
        int err = -1;
        DEBUG_MODE print_message("[LTICTR_PROXY_SERVER] Change to effective user (%s).\n", efctvuser.buf);
        if (isdigit(efctvuser.buf[0]) || efctvuser.buf[0]=='-') {
            err = seteuid(atoi((char*)efctvuser.buf));
        }
        else {
            pw = getpwnam((char*)efctvuser.buf);
            if (pw!=NULL) err = seteuid(pw->pw_uid);
        }
        if (err==-1) {
            DEBUG_MODE print_message("[LTICTR_PROXY_SERVER] Cannot change to effective user (%s).\n", efctvuser.buf);
        }
        free_Buffer(&efctvuser);
    }

    DEBUG_MODE print_message("[LTICTR_PROXY_SERVER] Start LTICTR_PROXY_SERVER. (%d)\n", RootPID);

    //
    // Network
    // for SSL/TLS
    if (ServerSSL==ON) {
        ssl_init();
        ServerCTX = ssl_server_setup(TLS_CertPem, TLS_KeyPem, TLS_ChainPem);
    }
    ClientCTX = ssl_client_setup(NULL);

    // socket open for client
    Nofd = tcp_server_socket(sport);       // block socket
    if (Nofd<0) {
        //syslog(Logtype, "Failure to open the server port for client connection.(%d)", sport);
        print_message("[LTICTR_PROXY_SERVER] Failure to open the server port for client connection.(%d)\n", sport);
        sig_term(-1);
    }
    DEBUG_MODE print_message("[LTICTR_PROXY_SERVER] Server port was opened for client connection. (%d)\n", sport);

    //////////////////////////////////////////////////
    // API Server Process の起動
    if (APIServerExec==ON) {
        fork_api_server();
    }

    //
    struct sockaddr cl_addr;
    socklen_t cdlen = sizeof(cl_addr);

    // main loop
    Loop {
        //print_message("[LTICTR_PROXY_SERVER] Start process loop\n");
        Sofd = accept_intr(Nofd, &cl_addr, &cdlen);
        if (Sofd<0) {
            //syslog(Logtype, "Failure to connect from client. [%s]", strerror(errno));
            print_message("[LTICTR_PROXY_SERVER] Failure to connect from client. [%s]\n", strerror(errno));
            sig_term(-1);
        }
        //
        pid_t pid = fork();
        if (pid==0) receipt_proxy(Sofd, ServerCTX, ClientCTX, apiurl, ProxyList, ChunkedMode);
        close(Sofd);    // don't use socket_close() !
        Sofd = 0;

        NoSigchld = ON;
        tList* lp = find_tList_end(PIDList);
        add_tList_node_int(lp, (int)pid, 0);
        NoSigchld = OFF;
        if (PendingSigchld>0) {
            sig_child(PendingSigchld);
            PendingSigchld = 0;
        }

        //print_message("[LTICTR_PROXY_SERVER] Stop process loop\n");
    }

    // Unreachable
    DEBUG_MODE print_message("[LTICTR_PROXY_SERVER] Stop main loop.\n");
    term_main(99999);
    //
    exit(0);
}



void fork_api_server()
{
    DEBUG_MODE print_message("[LTICTR_PROXY_SERVER] Start LTICTR_API_SERVER.\n");
    APIChildPID = fork();
    if (APIChildPID==0) {
        Buffer compath = make_Buffer(LPATH);
        char* path = get_file_path(PTR_argv[0]);
        if (path!=NULL) {
            copy_s2Buffer(path, &compath);
            free(path);
        }
        cat_s2Buffer(API_SERVER_NAME, &compath);
        //
        PTR_argv[0] = dup_str(API_SERVER_NAME);
        execv((char*)compath.buf, PTR_argv);
        free_Buffer(&compath);
        _exit(0);
    }
    add_tList_node_int(PIDList, (int)APIChildPID, 0);

    return;
}



int  init_main(Buffer configfile)
{
    // config file
    tList* filelist = NULL;
    if (configfile.buf!=NULL) {
        filelist = read_index_tList_file((char*)configfile.buf, '=');
        //
        if (filelist!=NULL) {
            PIDFile       = get_str_param_tList (filelist, LTICTR_PID_FILE,     PIDFile);
            TLS_CertPem   = get_str_param_tList (filelist, LTICTR_SERVER_CERT,  TLS_CertPem);
            TLS_ChainPem  = get_str_param_tList (filelist, LTICTR_SERVER_CHAIN, TLS_ChainPem);
            TLS_KeyPem    = get_str_param_tList (filelist, LTICTR_PRIVATE_KEY,  TLS_KeyPem);
            API_Token     = get_str_param_tList (filelist, LTICTR_API_TOKEN,    API_Token);
            //
            Rest_Path     = get_str_param_tList (filelist, REST_PATH_KEY ,      Rest_Path);
            Rest_Service  = get_str_param_tList (filelist, REST_SERVICE_KEY,    Rest_Service);
            Rest_HTTPver  = get_str_param_tList (filelist, REST_HTTPVER_KEY,    Rest_HTTPver);
            Rest_Response = get_bool_param_tList(filelist, REST_RESPONSE_KEY,   Rest_Response);

            del_tList(&filelist);
        }
    }

    return  0;
}


//
void  term_main(int code)
{
    pid_t pid = getpid();
    if (pid==RootPID) {
        if (Sofd>0) socket_close(Sofd);
        if (Nofd>0) socket_close(Nofd);
        if (PIDFile!=NULL) remove(PIDFile);
        //
        tList* lpid = PIDList;
        if (lpid!=NULL && lpid->ldat.id==TLIST_ANCHOR_NODE) lpid = lpid->next;
        while (lpid!=NULL) {
            if (lpid->ldat.id>0) kill((pid_t)lpid->ldat.id, SIGTERM);   
            lpid = lpid->next;   
        }
        //
        DEBUG_MODE print_message("[LTICTR_PROXY_SERVER] Shutdown root LTICTR_PROXY process with code = (%d)\n", code);
        //
        //closelog();
    }
    else {
        DEBUG_MODE print_message("[LTICTR_PROXY_SERVER] Shutdown proxy LTICTR_PROXY process with code = (%d)\n", code);
    }
    return;
}


//
// Termination of program
//
void  sig_term(int signal)
{
    term_main(signal);
    
    pid_t pid = getpid();
    //DEBUG_MODE print_message("[LTICTR_PROXY_SERVER] SIGTERM : signal = %d (%d)\n", signal, pid);

    if (signal<0) signal = -signal;
    if (signal==SIGTERM) signal = 0;    // by systemctl stop ....

    if (pid==RootPID)  exit(signal);
    else              _exit(signal);
}



//
// Termination of child proxy process
//
void  sig_child(int signal)
{
    //print_message("[LTICTR_PROXY_SERVER] SIGCHILD: Start sig_child\n");
    if (NoSigchld==ON) {
        PendingSigchld = signal;
        return;
    }

    pid_t pid = 0;

    int ret;
    pid = waitpid(-1, &ret, WNOHANG);
    while(pid>0) {
        //print_message("[LTICTR_PROXY_SERVER] SIGCHILD: Exited child is %d\n", pid);
        tList* lst = search_id_tList(PIDList, pid, 1);
        if (lst!=NULL) del_tList_node(&lst);
        if (pid==APIChildPID) {
            //print_message("[LTICTR_PROXY_SERVER] SIGCHILD: API Server is down!!\n");
            sig_term(signal);
            //fork_api_server();
        }
        //DEBUG_MODE print_message("[LTICTR_PROXY_SERVER] SIGCHILD: signal = %d (%d)\n", signal, pid);
        //
        pid = waitpid(-1, &ret, WNOHANG);
    }

    //print_message("[LTICTR_PROXY_SERVER] SIGCHILD: Stop sig_child\n");
    return;
}



//
// Segmentation Falt
//
void  sig_segmen(int signal)
{
    pid_t pid = getpid();

    print_message("[LTICTR_PROXT_SERVER] **********************************************************\n");
    if (pid==RootPID) print_message("[LTICTR_PROXY_SERVER] Segmentation Falt in Main Process  [%d] !!!!!\n", pid);
    else              print_message("[LTICTR_PROXY_SERVER] Segmentation Falt in Child Process [%d] !!!!!\n", pid);
    print_message("[LTICTR_PROXT_SERVER] **********************************************************\n");

    sig_term(signal);
}


