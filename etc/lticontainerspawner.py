#
# Copyright (c) Jupyter Development Team.
# Distributed under the terms of the Modified BSD License.

#
# LTIContainerSpawner v1.2.0 for LTI by Fumi.Iseki
#
#                                      BSD License.
#

from dockerspawner import DockerSpawner

from traitlets import (
    Bool,
    Dict,
    List,
    Int,
    Unicode,
)

from urllib.parse import urlparse

import pwd, grp, os, sys, re


class LTIContainerSpawner(DockerSpawner):
    #
    use_group      = Bool(True, config = True)
    default_group  = Unicode('users', config = True)
    group_home_dir = Unicode('/home/{groupname}', config = True)
    user_home_dir  = Unicode('/home/{groupname}/{username}', config = True)
    projects_dir   = Unicode('jupyter', config = True)
    works_dir      = Unicode('works', config = True)
    volumes_dir    = Unicode('.volumes', config = True)
    teacher_gname  = Unicode('TEACHER', config = True)
    teacher_gid    = Int(7000,  config = True)
    base_id        = Int(30000, config = True)

    # extension command
    ext_user_id_cmd     = 'user_userid'
    ext_group_id_cmd    = 'user_groupid'
    ext_grp_name_cmd    = 'user_groupname'

    # custom command
    custom_image_cmd    = 'lms_image'
    custom_cpulimit_cmd = 'lms_cpulimit'
    custom_memlimit_cmd = 'lms_memlimit'
    custom_cpugrnt_cmd  = 'lms_cpugrnt'
    custom_memgrnt_cmd  = 'lms_memgrnt'
    custom_defurl_cmd   = 'lms_defurl'
    custom_users_cmd    = 'lms_users'
    custom_teachers_cmd = 'lms_teachers'
    custom_volumes_cmd  = 'lms_vol_'
    custom_submits_cmd  = 'lms_sub_'
    custom_prsnals_cmd  = 'lms_prs_'
    custom_iframe_cmd   = 'lms_iframe'
    custom_ssninfo_cmd  = 'lms_sessioninfo'
    custom_srvrurl_cmd  = 'lms_serverurl'
    custom_srvrpath_cmd = 'lms_serverpath'
    custom_course_cmd   = 'lms_course'
    custom_ltiname_cmd  = 'lms_ltiname'
    custom_options_cmd  = 'lms_options'

    #
    user_id          = -1
    group_id         = -1
    grp_name         = ''
    lms_user_id      = -1    # LMS USER ID
    course_id        = '0'
    host_name        = ''
    host_url         = ''
    host_port        = 80
    userdata         = {}
    #
    ext_user_id      = -1
    ext_group_id     = -1
    ext_grp_name     = ''
    #
    custom_image     = ''
    custom_cpulimit  = '0.0'
    custom_memlimit  = '0'
    custom_cpugrnt   = '0.0'
    custom_memgrnt   = '0'
    custom_defurl    = '/lab'
    custom_users     = []
    custom_teachers  = []
    custom_volumes   = {}
    custom_submits   = {}
    custom_prsnals   = {}
    custom_iframe    = False
    custom_ltictr_id = 0
    custom_lti_id    = 0
    custom_srvrurl   = ''
    custom_srvrpath  = ''
    custom_course    = ''
    custom_ltiname   = ''
    custom_options   = ''


    def init_parameters(self):
        #print('=== init_parameters() ===')
        self.user_id          = -1
        self.group_id         = -1
        self.grp_name         = ''
        self.lms_user_id      = -1
        self.course_id        = '0'
        self.host_name        = 'localhost'
        self.host_url         = 'http://localhost'
        self.host_port        = 80
        self.userdara         = {}
        #
        self.ext_user_id      = -1
        self.ext_group_id     = -1
        self.ext_grp_name     = ''
        #
        self.custom_image     = ''
        self.custom_cpulimit  = '0.0'
        self.custom_memlimit  = '0'
        self.custom_cpugrnt   = '0.0'
        self.custom_memgrnt   = '0'
        self.custom_defurl    = '/lab'
        self.custom_users     = []
        self.custom_teachers  = []
        self.custom_volumes   = {}
        self.custom_submits   = {}
        self.custom_prsnals   = {}
        self.custom_iframe    = False
        self.custom_ltictr_id = 0
        self.custom_lti_id    = 0
        self.custom_srvrurl   = ''
        self.custom_srvrpath  = ''
        self.custom_course    = ''
        self.custom_ltiname   = ''
        self.custom_options   = ''
        #
        return


    def get_lms_userinfo(self):
        grp_name = self.default_group
        userinfo = {}
        #
        userinfo['uid']   = self.base_id + self.lms_user_id
        userinfo['gname'] = grp_name
        try :
            userinfo['gid'] = grp.getgrnam(grp_name).gr_gid
        except :
            userinfo['gid'] = self.base_id

        return userinfo


    def get_userid(self):
        if self.user_id < 0:
            try :
                self.user_id = pwd.getpwnam(self.user.name).pw_uid  # from system user account
            except :
                if self.ext_user_id>=0 :
                    self.user_id = self.ext_user_id                 # from extension command
                else :
                    self.user_id = self.get_lms_userinfo()['uid']   # form LMS user accound
        #
        return self.user_id


    def get_groupname(self):
        if self.group_id < 0:
            try :
                self.group_id = pwd.getpwnam(self.user.name).pw_gid # from system user account
            except :
                if self.ext_group_id>=0 :
                    self.group_id = self.ext_group_id               # from extension command
                else :
                    self.group_id = self.get_lms_userinfo()['gid']  # form LMS user accound
        #
        if self.use_group and self.group_id >= 0 :
            if self.grp_name == '' :
                try :
                    self.grp_name = grp.getgrgid(self.group_id).gr_name     # from system user account
                except :
                    if self.ext_grp_name != '' :
                        self.grp_name = self.ext_grp_name                   # from extension command
                    else :
                        self.grp_name = self.get_lms_userinfo()['gname']    # form LMS user accound
        #
        return self.grp_name


    def template_namespace(self):
        d = super(LTIContainerSpawner, self).template_namespace()
        d['groupname'] = self.get_groupname()
        return d


    @property
    def homedir(self):
        return self.user_home_dir.format(username=self.user.name, groupname=self.get_groupname())

    @property
    def groupdir(self):
        return self.group_home_dir.format(groupname=self.get_groupname())


    def get_args(self):
        #print('=== get_args() ===')
        args = super(LTIContainerSpawner, self).get_args()

        if self.custom_iframe :
            if sys.version_info >= (3, 8) : cookie_options = ', "cookie_options": { "SameSite": "None", "Secure": True }'
            else :                          cookie_options = ''
            #
            frame_ancestors = "frame-ancestors 'self' " + self.host_url
            args.append('--NotebookApp.tornado_settings={ "headers":{"Content-Security-Policy": "'+ frame_ancestors + '" }' + cookie_options + '}')
            #get_config().NotebookApp.disable_check_xsrf = True
        #
        args.append('--SingleUserNotebookApp.default_url=' + self.default_url)   # for jupyterhub (<2.00) in images
        return args


    def create_dir(self, directory, uid, gid, mode) :
        if not os.path.isdir(directory) :
            os.makedirs(directory)
            os.chown(directory, uid, gid)
            os.chmod(directory, mode)


    #def auth_hook(authenticator, handler, authentication):
    #    print('=== auth_hook() ===')
    #    return authentication


    #def spawn_hook(self):
    #    print('=== spawn_hook() ===')


    #
    # for custom/ext data
    # パラメータから情報を得る
    #
    def userdata_hook(self, auth_state):
        #print('=== userdata_hook() ===')
        if auth_state is None : return
        if not hasattr(self, 'init_parameters') : return

        self.userdata = auth_state              # raw data
        self.init_parameters()

        for key, value in self.userdata.items():

            if key == 'context_id' : self.course_id = value         # LMS Course ID (string)

            elif key == 'user_id'  : self.lms_user_id = int(value)  # LMS USER ID

            elif key == 'lis_outcome_service_url' :
                parsed = urlparse(value)
                netloc = parsed.netloc
                self.host_name = parsed.hostname                    # Host Name
                scheme = parsed.scheme                              # HTTP Scheme
                self.host_url = scheme + '://' + self.netloc        # Host URL
                portnm = parsed.port
                if portnm is None :
                    if   scheme == 'https' : portnm = 443
                    elif scheme == 'http'  : portnm = 80
                self.host_port = portnm
                #
            elif key.startswith('ext_'):                            # Extension Command
                ext_cmd = key.replace('ext_', '')
                #
                if ext_cmd == self.ext_user_id_cmd:                                             # User ID Command
                    value = re.sub('[^0-9]', '', value)
                    if value != '' : self.ext_user_id = int(value)
                #
                elif ext_cmd == self.ext_group_id_cmd:                                          # User Group ID Command
                    value = re.sub('[^0-9]', '', value)
                    if value != '' : self.ext_group_id = int(value)
                #
                elif ext_cmd == self.ext_grp_name_cmd:                                          # User Group Name Command
                    value = re.sub('[;$\!\"\'&|\\<>?^%\(\)\{\}\n\r~\/ ]', '', value)
                    self.ext_grp_name = value
                #
            elif key.startswith('custom_'):                         # Custom Command
                costom_cmd = key.replace('custom_', '')
                #
                if costom_cmd == self.custom_image_cmd:                                         # Container Image Command
                    value = re.sub('[;$\!\"\'&|\\<>?^%\(\)\{\}\n\r~ ]', '', value)
                    self.custom_image = value
                #
                elif costom_cmd == self.custom_users_cmd:                                       # Users Command
                    value = re.sub('[;$\!\"\'&|\\<>?^%\(\)\{\}\n\r~\/ ]', '', value)
                    self.custom_users = value.replace(',',' ').split()
                #
                elif costom_cmd[0:len(self.custom_teachers_cmd)] == self.custom_teachers_cmd:   # Teachers Command
                    value = re.sub('[;$\!\"\'&|\\<>?^%\(\)\{\}\n\r~\/ ]', '', value)
                    self.custom_teachers = value.replace(',',' ').split()
                #
                elif costom_cmd[0:len(self.custom_cpugrnt_cmd)] == self.custom_cpugrnt_cmd:     # CPU Limit Guarantee Command
                    value = re.sub('[^0-9\.]', '', value)
                    self.custom_cpugrnt = value
                #
                elif costom_cmd[0:len(self.custom_memgrnt_cmd)] == self.custom_memgrnt_cmd:     # Memory Guarantee Command
                    value = re.sub('[^0-9]', '', value)
                    self.custom_memgrnt = value
                #
                elif costom_cmd[0:len(self.custom_cpulimit_cmd)] == self.custom_cpulimit_cmd:   # CPU Limit Command
                    value = re.sub('[^0-9\.]', '', value)
                    self.custom_cpulimit = value
                #
                elif costom_cmd[0:len(self.custom_memlimit_cmd)] == self.custom_memlimit_cmd:   # Memory Limit Command
                    value = re.sub('[^0-9]', '', value)
                    self.custom_memlimit = value
                #
                elif costom_cmd == self.custom_defurl_cmd:                                      # Default URL Command
                    value = re.sub('[;$\!\"\'&|\\<>?^%\(\)\{\}\n\r~ ]', '', value)
                    self.custom_defurl = value
                #
                elif costom_cmd[0:len(self.custom_options_cmd)] == self.custom_options_cmd:     # Option Command
                    value = re.sub('[;$\!\"\'&|\\<>?^%\(\)\{\}\n\r~\/ ]', '', value)
                    self.custom_options = value
                #
                elif costom_cmd[0:len(self.custom_volumes_cmd)] == self.custom_volumes_cmd:     # Volumes Command
                    value = re.sub('[;$\!\"\'&|\\<>?^%\(\)\{\}\n\r~\/ ]', '', value)
                    self.custom_volumes[costom_cmd] = value
                #
                elif costom_cmd[0:len(self.custom_submits_cmd)] == self.custom_submits_cmd:     # Submits Volume Command
                    value = re.sub('[;$\!\"\'&|\\<>?^%\(\)\{\}\n\r~\/ ]', '', value)
                    self.custom_submits[costom_cmd] = value
                #
                elif costom_cmd[0:len(self.custom_prsnals_cmd)] == self.custom_prsnals_cmd:     # Personals Volume Command
                    value = re.sub('[;$\!\"\'&|\\<>?^%\(\)\{\}\n\r~\/ ]', '', value)
                    self.custom_prsnals[costom_cmd] = value
                #
                elif costom_cmd[0:len(self.custom_iframe_cmd)] == self.custom_iframe_cmd:       # iframe Command
                    if value == '1' :
                        self.custom_iframe = True
                #
                elif costom_cmd[0:len(self.custom_ssninfo_cmd)] == self.custom_ssninfo_cmd:     # sessioninfo Command
                    value = re.sub('[^0-9]', ' ', value)
                    self.custom_ltictr_id = value.split()[0]
                    self.custom_lti_id    = value.split()[1]
                #
                elif costom_cmd[0:len(self.custom_srvrurl_cmd)] == self.custom_srvrurl_cmd:     # serverurl Command
                    value = re.sub('[;$\!\"\'&|\\<>?^%\(\)\{\}\n\r~ ]', '', value)
                    self.custom_srvrurl = value
                    if self.host_name == 'localhost' or self.host_name == '':
                        self.host_url = value
                        parsed = urlparse(value)
                        self.host_name = parsed.hostname
                #
                elif costom_cmd[0:len(self.custom_srvrpath_cmd)] == self.custom_srvrpath_cmd:   # serverpath Command
                    value = re.sub('[;$\!\"\'&|\\<>?^%\(\)\{\}\n\r~ ]', '', value)
                    self.custom_srvrpath = value
                #
                elif costom_cmd[0:len(self.custom_course_cmd)] == self.custom_course_cmd:       # course name Command
                    value = re.sub('[\\\n\r]', '', value)
                    self.custom_course = value
                #
                elif costom_cmd[0:len(self.custom_ltiname_cmd)] == self.custom_ltiname_cmd:     # LTI name Command
                    value = re.sub('[\\\n\r]', '', value)
                    self.custom_ltiname = value
                #
        return


    #
    # ユーザのアクセス情報をチェックし，マウントする課題ボリュームのパスの配列を返す．
    #
    def get_volumes_info(self, assoc):
        #print('=== get_volumes_info() ===')
        vols = []
        for key, value in assoc.items():
            usrs = []
            disp = ''
            lst  = value.split(':')
            num  = len(lst)

            if num > 0 : disp = lst[0]
            if num > 1 : usrs = lst[1].replace(',',' ').split()

            if disp != '' :
                mnt = False
                if len(usrs) != 0 :                                                         # : によるアクセス制限の指定あり
                    if ('*' in usrs) or (self.user.name in usrs) :
                        mnt = True
                elif ('*' in self.custom_users) or (self.user.name in self.custom_users) :  # : によるアクセス制限の指定なし
                    mnt = True
                elif (self.user.name in self.custom_teachers) :                             # 教師
                    mnt = True

                if mnt:
                    dirname = key + '_' + self.course_id + '_' + self.custom_lti_id + '_' + self.host_name
                    vols.append(self.volumes_dir + '/' + dirname + ':' + disp)
        #
        return vols


    #
    # コンテナに渡す環境変数を設定する．
    # NB_UID, NB_GID, NB_USER, NB_GROUP, NB_UMASK, NB_VOLUMES, NB_SUBMITS, NB_PRSNAL,
    # NB_TEACHER, NB_THRGROUP, NB_THRGID, ...
    #
    def get_env(self):
        #print('=== get_env() ===')
        env = super(LTIContainerSpawner, self).get_env()

        userid    = self.get_userid()
        username  = self.user.name
        groupname = self.get_groupname()
        groupid   = self.group_id

        env.update(NB_UID       = userid)
        env.update(NB_USER      = username)
        env.update(NB_GID       = groupid)
        env.update(NB_GROUP     = groupname)
        env.update(NB_DIR       = self.notebook_dir.format(username=username, groupname=groupname))

        env.update(NB_THRGID    = self.teacher_gid)
        env.update(NB_THRGROUP  = self.teacher_gname)
        env.update(NB_OPTION    = self.custom_options)
        env.update(NB_HOSTNAME  = self.host_name)
        if (self.user.name in self.custom_teachers) :
            env.update(NB_UMASK = '0033')
            env.update(NB_TEACHER = self.user.name)
        else:
            env.update(NB_TEACHER = '')

        # volumes
        volumes = ' '.join(self.get_volumes_info(self.custom_volumes))
        env.update(NB_VOLUMES = volumes)

        submits = ' '.join(self.get_volumes_info(self.custom_submits))
        env.update(NB_SUBMITS = submits)

        prsnals = ' '.join(self.get_volumes_info(self.custom_prsnals))
        env.update(NB_PRSNALS = prsnals)

        # for LTI
        env.update(LTI_CTR_ID = self.custom_ltictr_id)
        env.update(LTI_ID     = self.custom_lti_id)
        env.update(LTI_COURSE = self.custom_course)
        env.update(LTI_NAME   = self.custom_ltiname)

        return env


    #
    # START LTIContainerSpawner
    #
    def start(self):
        #print('=== start() ===')
        username  = self.user.name
        groupname = self.get_groupname()    # get self.group_id, too
        hosthome  = self.homedir
        grouphome = self.groupdir
        self.notebook_dir = hosthome
        self.volumes = {}

        course_id = self.course_id
        lti_id    = self.custom_lti_id
        host_name = self.host_name
        self.object_name = f'jupyterhub-{username}-{course_id}-{lti_id}-{host_name}'

        # cpu and memory
        if self.custom_cpugrnt != '':
            self.cpu_guarantee = float(self.custom_cpugrnt)
        #
        if self.custom_memgrnt != '':
            self.mem_guarantee = int(self.custom_memgrnt)
        #
        if self.custom_cpulimit != '':
            self.cpu_limit     = float(self.custom_cpulimit)
        #
        if self.custom_memlimit != '':
            self.mem_limit     = int(self.custom_memlimit)

        # image
        if self.custom_image != '':
            self.image = self.custom_image

        # default url
        if self.custom_defurl != '':
            self.default_url = self.custom_defurl

        # volume
        self.volumes[hosthome] = hosthome

        self.create_dir(grouphome, 0, self.group_id, 0o0755)
        self.create_dir(hosthome,  self.user_id, self.group_id, 0o0700)
        self.create_dir(hosthome + '/' + self.projects_dir,  self.user_id, self.group_id, 0o0700)
        self.create_dir(hosthome + '/' + self.projects_dir + '/' + self.works_dir, self.user_id, self.group_id, 0o0700)

        fullpath_dir  = hosthome + '/' + self.projects_dir + '/' + self.works_dir
        mount_volumes = self.get_volumes_info(self.custom_volumes)
        mount_submits = self.get_volumes_info(self.custom_submits)

        for volume in mount_volumes:
            mountp  = volume.rsplit(':')[0]
            dirname = mountp.split('/')[-1]
            self.volumes[dirname] = fullpath_dir + '/' + mountp

        for submit in mount_submits:
            mountp  = submit.rsplit(':')[0]
            dirname = mountp.split('/')[-1]
            self.volumes[dirname] = fullpath_dir + '/' + mountp

        #
        self.remove = True

        #print('=== START LTIContainerSpawner ===')
        return super(LTIContainerSpawner, self).start()


    #def stop(self, now=True):
    #    return super(LTIContainerSpawner, self).stop(now)


    #def get_cmdmand(self):
    #    cmd = super(LTIContainerSpawner, self).get_cmdmand()
    #    return cmd
    #
    #    '''
    #    if self._user_set_cmd:
    #        cmd = self.cmd
    #    else:
    #        image_info = yield self.docker("inspect_image", self.image)
    #        cmd = image_info["Config"]["Cmd"]
    #    return cmd + self.get_args()
    #    '''


    #def docker(self, method, *args, **kwargs):
    #    #return self.executor.submit(self._docker, method, *args, **kwargs)
    #    return super(LTIContainerSpawner, self).docker(method, *args, **kwargs)



