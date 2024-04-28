/**
    ipynb_conv.c : ipnb ファイル 変換 v1.2.0  by Fumi.Iseki BSD License.
        filename: と codenum: を metadata の tags の配列に追加 

        1.2.0  2023/05/22  LTIContainerUser 等の追加
        1.1.0  2023/05/08  再処理の禁止
        1.0.0  2022/07/29
        0.9.3  2021/08/19

   ex.) ipynb_conv  example.ipynb
        ipynb_conv  example.ipynb  ../../mon1/test1.ipynb  10
        ipynb_conv  example.ipynb  -  10  >  ./example_tag.ipynb    /// "-" meens stdout
*/

#include "tjson.h"


int main(int argc, char** argv)
{
    if (argc<2) {
        fprintf(stderr, "usage ... %s in_filename [out_filename] [num_fac]\n", argv[0]);
        exit(1);
    }

    char  meta_title[]     = "{\"LTIContainer\": \"ipynb_conv\"}";
    char  format_course[]  = "{\"LTIContainerCourse\": \"%s\"}";
    char  format_ltiname[] = "{\"LTIContainerLtiName\": \"%s\"}";
    char  format_teacher[] = "{\"LTIContainerTeacher\": \"%s\"}";
    char  format_user[]    = "{\"LTIContainerUser\": \"%s\"}";

    char* name_course      = NULL;
    char* name_ltiname     = NULL;
    char* name_user        = NULL;
    char* name_teacher     = NULL;
    char  name_buf[LNAME];
    //
    char* in_file  = argv[1];
    char* out_file = NULL;
    char* filename = NULL;
    int   num_fac  = 1;
    char  buf[LBUF];

    //
    if (argc>2) {
        out_file = argv[2];
        if (out_file[0]=='-' && out_file[1]=='\0') out_file = NULL;     // out_file == "-"
        if (argc>3) num_fac  = atoi(argv[3]);
    }
    if (out_file!=NULL) filename = get_file_name((const char*)out_file);
    else                filename = get_file_name((const char*)in_file);

    // パース Json
    tJson* pp = json_parse_file(in_file, 999);
    if (pp==NULL) exit(1);
    if (pp->state!=JBXL_JSON_PARSED) {
        print_message("PARSE Error (%s: %d)\n", in_file, pp->state);
        del_json(&pp);
        exit(1);
    }

    // 再処理の禁止
    tJson* mt = search_key_child_json(pp, (char*)"metadata", FALSE);
    tJson* lt = search_key_child_json(mt, (char*)"LTIContainer", FALSE);
    //if (lt!=NULL) return 0;
    //
    tJson* js = json_parse_prop(NULL, meta_title, 2);
    js->ldat.id = JSON_TEMP_NODE;       // 結合部分で { がダブるので
    join_json(mt, &js);

    // add infomations
    name_course = getenv("LTI_COURSE");
    if (name_course!=NULL) {
        snprintf(name_buf, LNAME, format_course, name_course);
        js = json_parse_prop(NULL, name_buf, 2);
        js->ldat.id = JSON_TEMP_NODE;
        join_json(mt, &js);
    }
    name_ltiname = getenv("LTI_NAME");
    if (name_ltiname!=NULL) {
        snprintf(name_buf, LNAME, format_ltiname, name_ltiname);
        js = json_parse_prop(NULL, name_buf, 2);
        js->ldat.id = JSON_TEMP_NODE;
        join_json(mt, &js);
    }
    name_user = getenv("NB_USER");
    if (name_user!=NULL) {
        snprintf(name_buf, LNAME, format_user, name_user);
        js = json_parse_prop(NULL, name_buf, 2);
        js->ldat.id = JSON_TEMP_NODE;
        join_json(mt, &js);
    }
    name_teacher = getenv("NB_TEACHER");
    if (name_teacher!=NULL) {
        snprintf(name_buf, LNAME, format_teacher, name_teacher);
        js = json_parse_prop(NULL, name_buf, 2);
        js->ldat.id = JSON_TEMP_NODE;
        join_json(mt, &js);
    }

    //
    tList* ls = search_all_node_strval_json(pp, (char*)"cell_type", (char*)"code");
    //
    while (ls!=NULL) {
        if (ls->altp!=NULL) {
            tJson* meta = search_key_sister_json(ls->altp, "metadata", FALSE);
            //
            int num = (ls->ldat.id + 1) * num_fac;
            if (meta!=NULL) {
                tJson* tags = search_key_json(meta, "tags", FALSE, 1);
                if (tags!=NULL) {
                    // tags ノードが存在する．
                    if (tags->next!=NULL) {
                        tJson* array = tags->next;
                        while (array->esis!=NULL) array = array->esis;
                        do {
                            tJson* temp = array->ysis;
                            if      (!strncmp("\"filename:", array->ldat.val.buf, 10)) del_json_node(&array);
                            else if (!strncmp("\"codenum:",  array->ldat.val.buf,  9)) del_json_node(&array);
                            array = temp;
                        } while (array!=NULL);
                    }
                    //
                    memset(buf, 0, LBUF);
                    snprintf(buf, LBUF-1, "[ \"filename:%s\", \"codenum:%d\" ]", filename, num);
                    tJson* js = json_array_parse(NULL, buf, 2);
                    join_json(tags, &js);
                }
                else {
                    // tags ノードが存在しない．
                    memset(buf, 0, LBUF);
                    snprintf(buf, LBUF-1, "{ \"tags\": [ \"filename:%s\", \"codenum:%d\" ] }", filename, num);
                    tJson* js = json_parse_prop(NULL, buf, 2);
                    js->ldat.id = JSON_TEMP_NODE;       // 結合部分で { がダブるので
                    join_json(meta, &js);
                }
            }
            else {
                // metadata ノードが存在しない．
                memset(buf, 0, LBUF);
                snprintf(buf, LBUF-1, "{ \"metadata\": { \"tags\": [ \"filename:%s\", \"codenum:%d\" ] } }", filename, num);
                tJson* js = json_parse_prop(NULL, buf, 2);
                js->ldat.id = JSON_TEMP_NODE;       // 結合部分で { がダブるので
                join_json(ls->altp->prev, &js);    
            }
        }
        ls = ls->next;
    }

    if (ls!=NULL) del_all_tList(&ls);

    if (out_file!=NULL) {
        FILE* fp = fopen(out_file, "w");
        print_json_opt(fp, pp, "\n", " ");
        fclose(fp);
    }
    else {
        //print_tTree(stdout, pp);
        print_json_opt(stdout, pp, "\n", " ");
    }
    del_json(&pp);

    return 0;
}

