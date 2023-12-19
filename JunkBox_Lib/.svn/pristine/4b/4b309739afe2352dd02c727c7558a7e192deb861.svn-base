/** 
@brief   ASN1 NODE用ライブラリ
@file    asn1_node.c
@author  Fumi.Iseki (C)
*/

//#include "tlist.h"
#include "asn1_node.h"


const char* _ASN1_NODE_CONST[] = 
{
    "PRIMITIVE",            // 0x00
    "CONSTRUCTED"           // 0x20
};


const char* _ASN1_NODE_TYPE[] = 
{
    "UNIVERSAL",            // 0x00
    "APPLICATION",          // 0x40
    "CONTEXT-DEFINED",      // 0x80
    "PRIVATE"               // 0xc0
};


const char* _ASN1_NODE_NAME[] = 
{
    "DATA END",             // 0x00
    "BOOLEAN",              // 0x01
    "INTEGER",              // 0x02
    "BIT STRING",           // 0x03
    "OCTET STRING",         // 0x04
    "NULL",                 // 0x05
    "OBJECT IDENTIFIER",    // 0x06
    "OBJECT DESCRITOR",     // 0x07
    "EXTERNAL",             // 0x08
    "REAL",                 // 0x09
    "ENUMERATED",           // 0x0a
    "EMBEDDED PDV",         // 0x0b
    "UTF8 STRING",          // 0x0c
    "RELATIVE-OID",         // 0x0d
    "?",                    // 0x0e
    "?",                    // 0x0f
    "SEQUENCE",             // 0x10
    "SET",                  // 0x11
    "NUMERIC STRING",       // 0x12
    "PRINTABLE STRING",     // 0x13
    "TELETEX STRING",       // 0x14
    "VIDEOTEX STRING",      // 0x15
    "IA5 STRING",           // 0x16
    "UTC TIME",             // 0x17
    "GENERALIZED TIME",     // 0x18
    "GRAPHIC STRING",       // 0x19
    "VISIBLE STRING",       // 0x1a
    "GENERAL STRING",       // 0x1b
    "UNIVERSAL STRING",     // 0x1c
    "CHARACTER STRING",     // 0x1d
    "BMP STRING",           // 0x1e
    "?",                    // 0x1f
};



void  asn1_node_type(int node, int* cnst, int* type, int* name)
{
    if (cnst!=NULL) {
        if (node & JBXL_ASN1_CNSTRCTD) *cnst = 1;
        else *cnst = 0;
    }
    if (type!=NULL) *type = (node & 0xc0) >> 6;
    if (name!=NULL) *name = node & 0x1f;

    return;
}


void  asn1_print_node(FILE* fp, int node)
{
    int cnst, type, name;

    asn1_node_type(node, &cnst, &type, &name);

    if (cnst==1) fprintf(fp, "%s ", _ASN1_NODE_CONST[cnst]);
    fprintf(fp, "%s ", _ASN1_NODE_TYPE[type]);
    if (type==0x00) fprintf(fp, "%s ", _ASN1_NODE_NAME[name]);
    else fprintf(fp, "%d ", name);
    fprintf(fp, "(%02x) ", node);

    return;
}
    


void  asn1_print_node_value(FILE* fp, int node, Buffer buf)
{
    fprintf(fp, ": ");

    if (node==JBXL_ASN1_INT) {
        long int n = bin2int_DER(buf);
        fprintf(fp, "INT = %ld ", n);
    }
    else if (node==JBXL_ASN1_OCT) {
        if (buf.buf!=NULL) fprintf(fp, "\"%s\" ", buf.buf);
        else               fprintf(fp, "\"\" ");
    }
    else if (buf.buf!=NULL) {               // その他
        fprintf(fp, "....... ");
    }

    return;
}

