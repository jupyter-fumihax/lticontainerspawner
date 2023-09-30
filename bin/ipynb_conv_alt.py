#!python
#
#

import sys
import json
import hashlib
import os

if len(sys.argv) < 2:
    print('usage ... ' + sys.argv[0] + " in_filename [out_filename]")
    sys.exit(1)

in_file = sys.argv[1]
out_file = ""

if len(sys.argv) > 2:
    if sys.argv[2] != "-":
        out_file = sys.argv[2]

with open(in_file, 'r') as fp:
    contents = fp.read()

json_data = json.loads(contents)

#
if 'metadata' in json_data:
    if 'lectureName' in json_data['metadata']:
        exit(0)


def make_uuid(str):
    hash = hashlib.md5(str.encode()).hexdigest()
    ret = '-'.join([hash[:8], hash[8:12], hash[12:16], hash[16:20], hash[20:]])
    return ret


def chek_code_cell(p_val, lti, cell_map):

    if 'cell_type' in p_val and p_val['cell_type'] == 'code':
        if not 'metadata' in p_val:
            p_val['metadata'] = {}
        if not 'tags' in p_val['metadata']:
            p_val['metadata']['tags'] = []
        meta = p_val['metadata']
        tags = p_val['metadata']['tags']
        filename = ''
        codenum = '0'
        for i, tag in enumerate(tags):
            if tag.startswith('filename:'):
                filename = tag[9:]
            elif tag.startswith('codenum:'):
                codenum = tag[8:]

        uniq_str = lti['course'] + lti['name'] + str(lti['id']) + filename + codenum
        rtl_uuid = make_uuid(uniq_str)
        cell_map[rtl_uuid] = {}
        cell_map[rtl_uuid]['displayName'] = codenum
        cell_map[rtl_uuid]['displayId'] = codenum

        meta['RTL'] = {}
        meta['RTL']['uuid'] = rtl_uuid
        meta['RTL']['displayId'] = codenum
        meta['RTL']['displayName'] = codenum
        meta['RTL']['judge'] = {}
        meta['RTL']['judge']['data'] = ""
        meta['RTL']['judge']['type'] = "execResult"
        meta['RTL']['targetCell'] = True
    else:
        for val in p_val.values():
            if isinstance(val, dict):
                chek_code_cell(val, lti, cell_map)


env = os.environ

lti = {}
lti['ctr_id'] = 0
lti['id'] = 0
lti['course'] = ''
lti['name'] = ''
lti['teacher'] = ''

if 'LTICTR_ID' in env:
    lti['ctr_id'] = env['LTICTR_ID']
if 'LTI_ID' in env:
    lti['id'] = env['LTI_ID']
if 'LTI_COURSE' in env:
    lti['course'] = env['LTI_COURSE']
if 'LTI_NAME' in env:
    lti['name'] = env['LTI_NAME']
if 'NB_TEACHER' in env:
    lti['teacher'] = env['NB_TEACHER']


if isinstance(json_data, dict):
    cell_map = {}
    if 'cells' in json_data:
        for val in json_data['cells']:
            if isinstance(val, dict):
                chek_code_cell(val, lti, cell_map)

    if 'metadata' in json_data:
        meta = json_data['metadata']

        meta['RTL'] = {}
        meta['RTL']['cellNameMap'] = {}

        for key, val in cell_map.items():
            meta['RTL']['cellNameMap'][key] = val

        lectureName = lti['course'] + ' : ' + lti['name']
        meta['RTL']['lectureName'] = lectureName
        meta['RTL']['lectureSignature'] = make_uuid(lectureName + ' : ' + str(lti['id']))
        meta['RTL']['lecturerName'] = lti['teacher']


text = json.dumps(json_data, indent=4, ensure_ascii=False)

if out_file != "":
    with open(out_file, 'w') as fp:
        fp.write(text)
else:
    print(text)
