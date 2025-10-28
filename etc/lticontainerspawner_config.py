# Configuration file for LTIContainerSpawner

#
# LTIContainerSpawner Parameters
#

#
# Volumes are mounted at /user_home_dir/projects_dir/works_dir/volumes_dir
#
spawner_use_group = True
group_home_dir    = '/home/{groupname}'
user_home_dir     = group_home_dir + '/{username}'
projects_dir      = 'jupyter'
works_dir         = 'works'
volumes_dir       = '.volumes'
notebook_dir      = user_home_dir

default_group     = 'others'                    # A group of users (ID unknown) that does not exist on the host (to be created in advance).
teacher_gid       = 7000                        # 1000 or more, GID not used in the system.
base_id           = 30000                       # ID The underlying ID number in case of unknown. The part that is not used in the system.

act_limit         = 6000                        # 100m, Default Activity Limit Time (s)
cpu_limit         = 1                           # Default CPU Limit
mem_limit         = 1073741824                  # Default Memory Limit: 1GiB
default_url       = '/lab'
http_timeout      = 60                          # 60s


#
# PROXY
#
proxy_should_start = True                       # False means to use ltictr_proxy. True means to use configurable-http-proxy
proxy_cleanup      = True
proxy_api_url      = 'http://localhost:8001'
proxy_api_token    = "ABCDEFG"                  # Same value as LTICTR_API_Token in ltictr_proxy.conf


#
# System Environment
#
spawner_environment = {
    'GRANT_SUDO'      : 'no',                   # 'no' for normal use
    'PRJCT_DIR'       : projects_dir,
    'WORK_DIR'        : works_dir,
    'VOLUME_DIR'      : volumes_dir,
    'NB_UMASK'        : '0037',
    'CONDA_DIR'       : '/opt/conda',
    'CHOWN_HOME'      : 'yes',
    'CHOWN_HOME_OPTS' : '-R',
}
# CHOWN_EXTRA, CHOWN_EXTRA_OPTS

