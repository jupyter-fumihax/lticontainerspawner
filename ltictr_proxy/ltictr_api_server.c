/*
    API Server for JupyterHub and LTIContainerSpawner
        
                by Fumi.Iseki '22 02/12   BSD License.
*/

#define  LTICTR_API_VERSION "1.0.1"


#include "ltictr_api_server.h"
#include "ltictr_api.h"


#define  LTICTR_PID_FILE     "LTICTR_API_PID_File"
#define  LTICTR_SERVER_CERT  "LTICTR_Server_Cert"
#define  LTICTR_SERVER_CHAIN "LTICTR_Server_Chain"
#define  LTICTR_PRIVATE_KEY  "LTICTR_Private_Key"
#define  LTICTR_API_TOKEN    "LTICTR_API_Token"


int      Nofd = 0, Aofd = 0;
int      APIPortSSL     = OFF;
SSL_CTX* APIPortCTX     = NULL;

tList*   ProxyList      = NULL;

//int      Logtype        = LOG_ERR;


// default config value
char*    PIDFile        = "/var/run/ltictr_api.pid";
char*    TLS_CertPem    = "/etc/pki/tls/certs/server.pem";
char*    TLS_ChainPem   = NULL;
char*    TLS_KeyPem     = "/etc/pki/tls/private/key.pem";
char*    API_Token      = "default_token";



//
int main(int argc, char** argv)
{
    int    version = OFF;
    int    aport = 0;
    struct passwd* pw;

    Buffer api_url;
    Buffer efctvuser;
    Buffer pidfile;
    Buffer certfile;
    Buffer chainfile;
    Buffer keyfile;
    Buffer configfile;

    // for arguments
    api_url    = init_Buffer();
    efctvuser  = init_Buffer();
    pidfile    = init_Buffer();
    certfile   = init_Buffer();
    chainfile  = init_Buffer();
    keyfile    = init_Buffer();
    configfile = init_Buffer();

    for (int i=1; i<argc; i++) {
        if      (!strcmp(argv[i],"-a")) {if (i!=argc-1) api_url   = make_Buffer_bystr(argv[i+1]);}
        else if (!strcmp(argv[i],"-u")) {if (i!=argc-1) efctvuser = make_Buffer_bystr(argv[i+1]);}
        else if (!strcmp(argv[i],"-d")) DebugMode = ON;
        else if (!strcmp(argv[i],"--version")) version = ON;
        //
        else if (!strcmp(argv[i],"--apid"))   {if (i!=argc-1) pidfile    = make_Buffer_bystr(argv[i+1]);}
        else if (!strcmp(argv[i],"--cert"))   {if (i!=argc-1) certfile   = make_Buffer_bystr(argv[i+1]);}
        else if (!strcmp(argv[i],"--chain"))  {if (i!=argc-1) chainfile  = make_Buffer_bystr(argv[i+1]);}
        else if (!strcmp(argv[i],"--key"))    {if (i!=argc-1) keyfile    = make_Buffer_bystr(argv[i+1]);}
        else if (!strcmp(argv[i],"--conf"))   {if (i!=argc-1) configfile = make_Buffer_bystr(argv[i+1]);}
        else if (!strcmp(argv[i],"--config")) {if (i!=argc-1) configfile = make_Buffer_bystr(argv[i+1]);}
        //
        //else if (*argv[i]=='-') {
        //    print_message("[LTICTR_API_SERVER] Unknown argument: %s\n", argv[i]);
        //    free_Buffer(&api_url);
        //    break;
        //}
    }
    if (version==ON) {
        printf("%s\n", LTICTR_API_VERSION);
        exit(0);
    }
    if (api_url.buf==NULL) {
        print_message("Usage... %s -a [api_url:]port [-u user] [-d] [--apid pid_file] [--conf config_file] [--cert cert_file] [--key key_file] [--chain chain_file]\n", argv[0]);
        exit(1);
    }

    // SYSLOG
    //openlog("ltictr_api_server", LOG_PERROR|LOG_PID, LOG_AUTH);

    if (strstr((char*)api_url.buf, ":")!=NULL) {
        Buffer protocol = init_Buffer();
        unsigned short port;
        decomp_url(api_url, NULL, &protocol, NULL, &port, NULL);
        if (ex_strcmp("https", (char*)protocol.buf)) APIPortSSL = ON;
        free_Buffer(&protocol);
        aport = (int)port;
    }
    else {
        aport = atoi((char*)api_url.buf);
    }
    if (aport<=0) {
        //syslog(Logtype, "API Server Port num = (%d). Can not open port.", aport);
        print_message("[LTICTR_API_SERVER] API Server Port num = (%d). Can not open port.\n", aport);
        sig_term(-1);
    }

    if (pidfile.buf  !=NULL) PIDFile      = (char*)pidfile.buf;
    if (certfile.buf !=NULL) TLS_CertPem  = (char*)certfile.buf;
    if (chainfile.buf!=NULL) TLS_ChainPem = (char*)chainfile.buf;
    if (keyfile.buf  !=NULL) TLS_KeyPem   = (char*)keyfile.buf;
    //
    ProxyList = add_tList_node_anchor();

    //
    // Initialization
    DEBUG_MODE print_message("[LTICTR_API_SERVER] Start initialization.\n");
    if (configfile.buf!=NULL) {
        if (!file_exist((char*)configfile.buf)) {
            //syslog(Logtype, "Failure to check configuration file (%s). Can not read the configuration file.", (char*)configfile.buf);
            print_message("[LTICTR_API_SERVER] Failure to check configuration file (%s). Can not read the configuration file.\n", (char*)configfile.buf);
            sig_term(-1);
        }
    }
    init_main(configfile);
    //
    free_Buffer(&configfile);
    DEBUG_MODE print_message("[LTICTR_API_SERVER] Initialization is finished.\n");

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

    set_sigsegv_handler(sig_segmen);

    //
    // PID file
    FILE* fp = fopen((char*)PIDFile, "w");
    if (fp!=NULL) {
        fprintf(fp, "%d", (int)getpid());
        fclose(fp);
    }

    //
    // Change effective user
    if (efctvuser.buf!=NULL) {
        int err = -1;
        DEBUG_MODE print_message("[LTICTR_API_SERVER] Change to effective user (%s).\n", efctvuser.buf);
        if (isdigit(efctvuser.buf[0]) || efctvuser.buf[0]=='-') {
            err = seteuid(atoi((char*)efctvuser.buf));
        }
        else {
            pw = getpwnam((char*)efctvuser.buf);
            if (pw!=NULL) err = seteuid(pw->pw_uid);
        }
        if (err==-1) {
            DEBUG_MODE print_message("[LTICTR_API_SERVER] Cannot change to effective user (%s).\n", efctvuser.buf);
        }
        free_Buffer(&efctvuser);
    }

    //
    // for SSL/TLS
    if (APIPortSSL==ON) {
        ssl_init();
        APIPortCTX = ssl_server_setup(TLS_CertPem, TLS_KeyPem, TLS_ChainPem);
    }

    // socket open for client
    Nofd = tcp_server_socket(aport);       // block socket
    if (Nofd<0) {
        //syslog(Logtype, "Failure to open the server port for client connection. (%d)", aport);
        print_message("[LTICTR_API_SERVER] Failure to open the server port for client connection. (%d)\n", aport);
        sig_term(-1);
    }
    DEBUG_MODE print_message("[LTICTR_API_SERVER] Server port was opened for client connection. (%d)\n", aport);

    struct sockaddr cl_addr;
    socklen_t cdlen = sizeof(cl_addr);
    SSL* assl = NULL;
    DEBUG_MODE print_message("[LTICTR_API_SERVER] Start Main Loop.\n");

    // main loop
    Loop {
        Aofd = accept_intr(Nofd, &cl_addr, &cdlen); 
        if (Aofd>0) {
            if (APIPortCTX!=NULL) assl = ssl_server_socket(Aofd, APIPortCTX);
            api_main_process(Aofd, assl, ProxyList);
            ssl_close(assl);
            close(Aofd);
            assl = NULL;
            Aofd = 0;
            //
            DEBUG_MODE {
                print_message("[LTICTR_API_SERVER] === LIST ===\n");
                print_tList(stderr, ProxyList);
                print_message("================================\n");
            }
        }
    }

    // Unreachable
    DEBUG_MODE print_message("[LTICTR_API_SERVER] Stop  Main Loop.\n");
    term_main(99999);
    //
    exit(0);
}



int  init_main(Buffer configfile)
{
    // config file
    tList* filelist = NULL;
    if (configfile.buf!=NULL) {
        filelist = read_index_tList_file((char*)configfile.buf, '=');
        //
        if (filelist!=NULL) {
            PIDFile      = get_str_param_tList(filelist, LTICTR_PID_FILE,     PIDFile);
            TLS_CertPem  = get_str_param_tList(filelist, LTICTR_SERVER_CERT,  TLS_CertPem);
            TLS_ChainPem = get_str_param_tList(filelist, LTICTR_SERVER_CHAIN, TLS_ChainPem);
            TLS_KeyPem   = get_str_param_tList(filelist, LTICTR_PRIVATE_KEY,  TLS_KeyPem);
            API_Token    = get_str_param_tList(filelist, LTICTR_API_TOKEN,    API_Token);
            //
            del_tList(&filelist);
        }
    }
    return 0;
}


//
void  term_main(int code)
{
    UNUSED(code);

    if (Aofd>0) socket_close(Aofd);
    if (Nofd>0) socket_close(Nofd);
    if (PIDFile!=NULL) remove(PIDFile);
    //
    //DEBUG_MODE print_message("[LTICTR_API_SERVER] Shutdown root LTICTR_API_SERVER process with code = (%d)\n", code);

    //closelog();

    return;
}


//
// Termination of program
//
void  sig_term(int signal)
{
    term_main(signal);
    
    //pid_t pid = getpid();
    //DEBUG_MODE print_message("[LTICTR_API_SERVER] sig_term: Exit program with signal = %d (%d)\n", signal, pid);

    if (signal<0) signal = -signal;
    if (signal==SIGTERM) signal = 0;    // by systemctl stop ....

    exit(signal);
}


//
// Segmentation Falt  
//
void  sig_segmen(int signal)
{
    pid_t pid = getpid();

    print_message("[LTICTR_API_SERVER] **********************************************************\n");
    print_message("[LTICTR_API_SERVER] Segmentation Falt in Main Process [%d] !!!!!\n", pid);
    print_message("[LTICTR_API_SERVER] **********************************************************\n");

    sig_term(signal);
}

