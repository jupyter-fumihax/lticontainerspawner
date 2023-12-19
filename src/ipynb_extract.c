/**
    ipynb_extract.c : code cell 抽出: v1.0.0  by Fumi.Iseki BSD License.
        ipynb ファイルから，評価対象の code cell を抜き出す．
        ベース名として example.ipynb が指定された場合は，input_dir の *_example.ipynb が読み込まれる．
        番号の最期に n を指定した場合は code_num の無いTAGも取り出す．

        1.0.1  2023/08/02  # 出力ファイル名の変更
        1.0.0  2023/04/19

   ex.) ipynb_extract ../src example.ipynb 1 2 3 n  　（../src/*_example.ipynb が読み込まれる）
*/

#include "xtools.h"
#include "tjson.h"


int    get_codenum(char* buf);
char*  make_guid(void);
tJson* make_user_head(char* username);
tJson* make_user_tail(char* username);


int main(int argc, char** argv)
{
    tJson* ipynb = NULL;
    tJson* tail  = NULL;
    tJson* js;
    int    count = 0;

    if (argc<4) {
        fprintf(stderr, "usage ... %s input_dir base_filename code_num [code_num] [code_num] ...\n", argv[0]);
        exit(1);
    }

    init_rand();

    int arg_num   = argc;
    int out_nonum = FALSE;
    if (!strcmp(argv[arg_num-1], "n")) {
        out_nonum = TRUE;
        arg_num--;
    }

    Buffer srcdir = make_Buffer_str(argv[1]);
    if (srcdir.buf[srcdir.vldsz-1]!='/') cat_s2Buffer("/", &srcdir);
    //
    Buffer basefn = make_Buffer_str("_");
    cat_s2Buffer(argv[2], &basefn);

    tList* ld = get_dir_files(srcdir.buf);
    tList* ll = ld;
    //
    while(ll!=NULL) {
        if (!strnrvscmp(ll->ldat.val.buf, basefn.buf, strlen(basefn.buf)) && !strnrvscmp(basefn.buf, ".ipynb", 6)) {
            ll->ldat.lv = ON;
        }
        else {
            ll->ldat.lv = OFF;
        }
        ll = ll->next;
    }
    free_Buffer(&basefn);
    
    //
    ll = ld;
    while(ll!=NULL) {
        // 拡張子が .ipynb のファイルを処理する
        if (ll->ldat.lv==ON) {
            char* in_file = (char*)ll->ldat.val.buf;
            print_message("read %s\n", ll->ldat.val.buf);

            tJson* pp = json_parse_file(in_file, 999);
            if (pp==NULL) {
                print_message("PARSE error! No json file (%s)\n", in_file);
                ll = ll->next;
                continue;
            }
            if (pp->state!=JBXL_JSON_PARSED) {
                print_message("PARSE error! (%s: %d)\n", in_file, pp->state);
                del_json(&pp);
                ll = ll->next;
                continue;
            }

            //
            if (ipynb==NULL) {
                ipynb = json_parse("{ \"cells\": []}", 1);
                ipynb = del_json_anchor_node(ipynb);
            }
            if (tail==NULL) {
                tail = new_json_anchor();
                js = search_key_json_obj(pp, "metadata", 1);
                js->ctrl = TREE_NOSIS_NODE;
                dup_merge_json(tail, js);
                js = search_key_json(pp, "nbformat", TRUE, 1);
                js->ctrl = TREE_NOSIS_NODE;
                dup_merge_json(tail, js);
                js = search_key_json(pp, "nbformat_minor", TRUE, 1);
                js->ctrl = TREE_NOSIS_NODE;
                dup_merge_json(tail, js);
                //
                tail = del_json_anchor_node(tail);
                clear_tTree_ctrl(tail);
            }

            char* fname = get_file_name(in_file);
            print_message("extracting ... %s\n", fname);
            //
            char* user = awk(fname, '_', 1);
            js = make_user_head(user);
            join_json(ipynb->next, &js);
            //
            tList* ls = search_all_node_strval_json(pp, (char*)"cell_type", (char*)"code");
            //
            while (ls!=NULL) {
                if (ls->altp!=NULL) {
                    tJson* meta = search_key_sister_json(ls->altp, "metadata", FALSE);
                    if (meta!=NULL) {
                        //
                        int find_codenum = FALSE;
                        tJson* tag = search_key_json(meta, "tags", FALSE, 1);
                        if (tag!=NULL && tag->next!=NULL) {
                            tJson* code = tag->next;
                            do {
                                if (code->ldat.id==JSON_ARRAY_VALUE_NODE && code->ldat.lv==JSON_VALUE_STR) {
                                    if (!strncmp("\"codenum:", code->ldat.val.buf, 9)) {
                                        int num = get_codenum((char*)code->ldat.val.buf);
                                        for (int i=3; i<arg_num; i++) {
                                            if (atoi(argv[i])==num) {
                                                js = new_json();
                                                js->ldat.id = JSON_BRACKET_NODE;
                                                js->ldat.lv = JSON_VALUE_OBJ;
                                                dup_merge_json(js, ls->altp);
                                                join_json(ipynb->next, &js);
                                                count++;
                                            }
                                        }
                                        find_codenum = TRUE;
                                    }
                                }
                                code = code->ysis;
                            } while(code!=NULL);
                        }

                        // 勝手に生成されたコードセル
                        if (find_codenum==FALSE && out_nonum==TRUE) {
                            js = new_json();
                            js->ldat.id = JSON_BRACKET_NODE;
                            js->ldat.lv = JSON_VALUE_OBJ;
                            dup_merge_json(js, ls->altp);
                            join_json(ipynb->next, &js);
                            count++;
                        }
                    }
                }
                ls = ls->next;
            }
            js = make_user_tail(user);
            join_json(ipynb->next, &js);

            del_all_tList(&ls);
            del_json(&pp);
        }
        //
        ll = ll->next;
    }
    del_all_tList(&ld);

    //
    int output_flg = OFF;
    if (count>0 && ipynb!=NULL) {
        join_json(ipynb, &tail);
        char out_file[SBUFSZ];
        //snprintf(out_file, SBUFSZ, "./%03d-%s", atoi(argv[arg_num-1]), argv[2]);
        snprintf(out_file, SBUFSZ, "./%03d-%s", atoi(argv[3]), argv[2]);
        FILE* fp = fopen(out_file, "w");
        if (fp!=NULL) {
            print_json_opt(fp, ipynb, "\r\n", " ");
            fclose(fp);
            print_message("%s is outputed.\n", out_file);
            output_flg = ON;
        }
    }
    if (output_flg==OFF) print_message("no output file.\n");

    //
    if (ipynb!=NULL) del_json(&ipynb);

    return 0;
}



int  get_codenum(char* buf)
{
    char* pp = buf + 9;     // strlen("\"codenum:") 
    char* pt = pp;
    while (*pt!='\"' && *pt!='\0') pt++;
    char bk = *pt;
    *pt = '\0';
    int num = atoi(pp);
    *pt = bk;

    return num;
}


char*  make_guid(void)
{
    char* uuid = randbit(128);
    char* guid = uuid2guid(uuid);
    free(uuid);

    return guid;
}


tJson*  make_user_head(char* username)
{
    char  buf[SBUFSZ];
    char  headmark[] = "{ \"cell_type\": \"markdown\", \"id\": \"%s\", \"metadata\": {}, \"source\": [ \"## %s\" ] }";
    char* guid = make_guid();

    snprintf(buf, SBUFSZ, headmark, guid, username);
    free_null(guid);

    tJson* js = json_parse(buf, 99);
    js = del_json_anchor_node(js);

    return js;
}


tJson*  make_user_tail(char* username)
{
    char  buf[SBUFSZ];
    char  tailmark[] = "{ \"cell_type\": \"markdown\", \"id\": \"%s\", \"metadata\": {}, \"source\": [ \"%s\" ] }";
    char  codecell[] = "{ \"cell_type\": \"code\", \"execution_count\": null, \"id\": \"%s\", \
                                         \"metadata\": { \"tags\": [ \"user: %s\", \"codenum:999\" ] }, \"source\": [\"0\"] }";
    int  len;
    char* guid;

    guid = make_guid();
    snprintf(buf, SBUFSZ, tailmark, guid, "#### Score:");
    free_null(guid);

    len = strlen(buf);
    guid = make_guid();
    snprintf(buf+len, SBUFSZ-len, codecell, guid, username);
    free_null(guid);

    len = strlen(buf);
    guid = make_guid();
    snprintf(buf+len, SBUFSZ-len, tailmark, guid, "<hr size=\\\"2\\\">");
    free_null(guid);

    tJson* js = json_parse(buf, 99);
    js = del_json_anchor_node(js);

    return js;
}


