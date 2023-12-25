/* 
  unlock overlay containers for podman

  example 
    unlock_podman_containers /var/lib/containers/storage/overlay-containers/volatile-containers.json j20000aa
*/

#include "tjson.h"


int main(int argc, char** argv)
{
    if (argc<3) {
        fprintf(stderr, "usage ... %s filename unlock_userid\n", argv[0]);
        exit(1);
    }

    tJson* json = json_parse_file(argv[1], 999);
    if (json==NULL) return 1;
    //print_tTree(stdout, json);
    //print_json(stdout, json, JSON_INDENT_FORMAT);

    Buffer buf = make_Buffer_str("jupyterhub-");
    cat_s2Buffer(argv[2], &buf);
    tJson* lst = search_all_node_strval_json(json, "ARRAY_VALUE", buf.buf);
    tJson* tgt = lst;

    print_tTree(stdout, lst);
    int chngflg = OFF;
    while (tgt!=NULL && tgt->altp!=NULL) {
        if (tgt->altp->prev!=NULL) {
            if (!strncmp("names", tgt->altp->prev->ldat.key.buf, 5)) {
                del_json(&(tgt->altp->prev->prev));
                tgt->altp = NULL;
                chngflg = ON;
            }
        }
        tgt = tgt->next;
    }
    print_json(stdout, json, JSON_INDENT_FORMAT);

    if (chngflg==ON) {
        FILE* fp = fopen(argv[1], "w");
        if (fp!=NULL) {
	        print_json(fp, json, 0);
            fclose(fp);
        }
    }

    free_Buffer(&buf);
    del_json(&lst);
    del_json(&json);

    return 0;
}

