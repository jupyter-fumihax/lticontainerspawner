#
# Usage
# 1. change FROM line
# 2. execute docker build command
# ex.) docker build . --format=docker -t jupyterhub-ltictr/singleuser:20251102
#

FROM localhost/jupyterhub/singleuser:temp
#FROM localhost/jupyterhub/base-notebook:temp
#FROM localhost/jupyterhub/datascience-notebook:temp
#FROM localhost/jupyterhub/tensorflow-notebook:temp
#FROM localhost/jupyterhub/scipy-notebook:temp
#FROM localhost/jupyterhub/jupyterlab-broccoli:temp
#FROM localhost/jupyterhub/jupyterlab-broccoli-tensorflow:temp
#FROM localhost/jupyterhub/java-notebook:temp
#FROM localhost/jupyterhub/php-notebook:temp
#FROM localhost/jupyterhub/xeus-cling:temp

#FROM localhost/jupyterhub/rust-notebook:temp
#FROM localhost/jupyterhub/swift-cli:temp
#FROM localhost/jupyterhub/xeus-ijs:temp
#FROM localhost/jupyterhub/devel-notebook:temp
#FROM localhost/jupyterhub/cuda-ubuntu20:temp


USER root

#
# Jupyter Notice 
ENV CONDA_HOME=/opt/conda
RUN $CONDA_HOME/bin/pip install --prefix $CONDA_HOME --no-cache-dir jnotice 

#
# Lticontainer
HEALTHCHECK CMD /usr/local/bin/health_check.sh

COPY \
    bin/start.sh \
    bin/start-notebook.sh \
    bin/start-singleuser.sh \
    bin/commit.sh \
    bin/ipynb_conv \
    bin/ipynb_conv_alt \
    bin/ipynb_conv_alt.py \
    bin/ipynb_deploy \
    bin/ipynb_setup \
    bin/ipynb_submit \
    bin/submit \
    bin/ipynb_extract \
    bin/extract \
    bin/ipynb_tocsv \
    bin/tocsv \
    bin/health_check.sh \
    /usr/local/bin/

COPY \
    etc/.bashrc \
    etc/.bash_profile \
    etc/.vimrc \
    /root/

COPY \
    etc/.bash_profile \
    etc/.bashrc \
    etc/.vimrc \
    /etc/skel/

COPY \
    etc/passwd.orig \
    etc/group.orig \
    /etc/

#
RUN set -eux \
 && chmod a+rx /usr/local/bin/* \
 && chmod a+r  /etc/skel/.vimrc /etc/skel/.bash* \
 && true


CMD ["start-notebook.sh"]

