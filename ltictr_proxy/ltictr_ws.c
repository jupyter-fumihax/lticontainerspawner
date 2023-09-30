/* vi: set tabstop=4 nocindent noautoindent: */

#include "ltictr_ws.h"



///////////////////////////////////////////////////////////////////////
// Web Socket Data

tJson*  ws_json_parse(ringBuffer* ring)
{
    // 非断片化のテキストのみサポート
    if (ring->datasz<=0) {
        ring->state = JBXL_NODATA;
        return NULL;
    }

    unsigned char knd = *ref_ringBuffer(ring, 0);
    if (knd!=0x81 && knd!=0x82) {
        ring->state = JBXL_INCOMPLETE;
        return NULL;
    }

    // ヘッダデータ
    tJson* json = NULL;
    unsigned char* pp = NULL;
    unsigned char* pm = NULL;
    int pos = 0, mask, i;
    long int len = 0;

    mask = (int)((*ref_ringBuffer(ring, 1)) >> 7);
    len  = (long int)((*ref_ringBuffer(ring, 1)) & 0x7f);
    pos  = 2;
    if (len==126) {
        len = (long int)(*ref_ringBuffer(ring, 2))*256 + (long int)(*ref_ringBuffer(ring, 3));
        pos = 4;
    }
    else if (len==127) {
        len = (long int)(*ref_ringBuffer(ring, 2));
        for(i=3; i<10; i++) {
            len = len*256 + (long int)(*ref_ringBuffer(ring, i));
            pos = 10;
        }
    }
    // data size = pos + mask*4 + len

    // ボディデータ処理
    if (ring->datasz >= pos + mask*4 + len) {
        //
        seek_ringBuffer(ring, pos);
        if (mask==1) pm = get_ringBuffer(ring, 4);
        pp = get_ringBuffer(ring, len);   // len + 1 Byte
        pp[len] = '\0';
        //
        if (mask==1) {
            for (i=0; i<len; i++) {
                pp[i] = pp[i] ^ pm[i%4];
            }
        }

        unsigned char* px = pp;
        if (knd==0x82) {          // for JupyterLab v4.x
            px += px[8];          // 推定処理．要確認
            if (strncasecmp((const char*)px, "shell", 5)) {
                free(pp);
                if (pm!=NULL) free(pm);
                ring->state = JBXL_DATA_REMAINS;
                return NULL;
            }
            //fprintf(stderr, "\n++%c%c%c%c%c+++++\n", px[0], px[1], px[2], px[3], px[4]);
            //fprintf(stderr, "%s\n", px);
            px += 5;    // + strlen("shell");
        }

        if (*px=='{') json = json_parse((char*)px, 0);

        // 解析失敗
        if (knd==0x82 && json==NULL) {
            FILE* fp = fopen("/var/log/ltictr_ws.log", "aw");
            fprintf(fp, "Parse Error [%ld]\n", time(0));
            fdump(fp, pp, 128);
            fclose(fp);
        } 
        //
        free(pp);
        if (pm!=NULL) free(pm);
        ring->state = JBXL_NORMAL;
    }
    else {
        ring->state = JBXL_INCOMPLETE;
    }

    return json;
}



tJson*  ws_json_client(unsigned char* mesg, int cc)
{
    tJson* json = NULL;
    tJson* jtmp = NULL;

    static ringBuffer* cring = NULL;
    if (cring==NULL) cring =  new_ringBuffer(BUFSZ2M);

    if ((mesg[0]!=0x81 && mesg[0]!=0x82) && cring->state==JBXL_NORMAL) return NULL;
    if ((mesg[0]==0x81 || mesg[0]==0x82) && cring->state==JBXL_INCOMPLETE) {
        clear_ringBuffer(cring);
    }
    put_ringBuffer(cring, mesg, cc);

    jtmp = ws_json_parse(cring);
    while (jtmp!=NULL || cring->state==JBXL_DATA_REMAINS) {
        if (jtmp!=NULL) json = join_json(json, &jtmp);
        jtmp = ws_json_parse(cring);
    }

    return json;
}



tJson*  ws_json_server(unsigned char* mesg, int cc)
{
    tJson* json = NULL;
    tJson* jtmp = NULL;

    static ringBuffer* sring = NULL;
    if (sring==NULL) sring = new_ringBuffer(BUFSZ2M);

    if ((mesg[0]!=0x81 && mesg[0]!=0x82) && sring->state==JBXL_NORMAL) return NULL;
    if ((mesg[0]==0x81 || mesg[0]==0x82) && sring->state==JBXL_INCOMPLETE) {
        clear_ringBuffer(sring);
    }
    put_ringBuffer(sring, mesg, cc);

    jtmp = ws_json_parse(sring);
    while (jtmp!=NULL || sring->state==JBXL_DATA_REMAINS) {
        if (jtmp!=NULL) json = join_json(json, &jtmp);
        jtmp = ws_json_parse(sring);
    }

    return json;
}



