#
# Usage
# 1. change FROM line
# 2. execute docker build command
# ex.) cd ..
#      docker build . --format=docker -t jupyterhub-ltictr/singleuser:20240319
#

#FROM docker.io/jupyter/base-notebook
FROM docker.io/jupyterhub/singleuser
#FROM docker.io/jupyter/datascience-notebook
#FROM docker.io/jupyter/tensorflow-notebook
#FROM docker.io/jupyter/scipy-notebook


USER root

#
RUN apt-get update \
 && apt-get install -y --no-install-recommends \
    language-pack-ja-base \
    language-pack-ja \
    fonts-takao* \
    fonts-ipafont* \
    fonts-noto-cjk* \
 && apt-get -y clean \
 && rm -rf /var/lib/apt/lists/* \
 && true

ENV CONDA_HOME=/opt/conda
ENV PATH=$CONDA_HOME/bin:$PATH

# Language & Ext Library
#RUN $CONDA_HOME/bin/conda update -n base -c conda-forge conda -y
RUN $CONDA_HOME/bin/conda install --prefix $CONDA_HOME -c conda-forge jupyterlab-language-pack-ja-jp -y \
 && $CONDA_HOME/bin/conda install --prefix $CONDA_HOME -c conda-forge ipywidgets -y \
 && $CONDA_HOME/bin/conda install --prefix $CONDA_HOME -c conda-forge pillow -y \
 && $CONDA_HOME/bin/conda install --prefix $CONDA_HOME -c conda-forge pandas -y \
 && $CONDA_HOME/bin/conda install --prefix $CONDA_HOME -c conda-forge ipycanvas -y \
 && $CONDA_HOME/bin/conda install --prefix $CONDA_HOME -c conda-forge numpy -y \
 && $CONDA_HOME/bin/conda install --prefix $CONDA_HOME -c conda-forge matplotlib -y \
 && $CONDA_HOME/bin/conda install --prefix $CONDA_HOME -c conda-forge ipympl -y \
 && $CONDA_HOME/bin/conda install --prefix $CONDA_HOME -c conda-forge seaborn -y \
 && $CONDA_HOME/bin/conda install --prefix $CONDA_HOME -c plotly plotly -y \
 && $CONDA_HOME/bin/conda clean   --all -y \
 && $CONDA_HOME/bin/pip   install --prefix $CONDA_HOME japanize-matplotlib \
 && $CONDA_HOME/bin/pip   install --prefix $CONDA_HOME jbturtle \
 && $CONDA_HOME/bin/pip   install --prefix $CONDA_HOME nbgrader \
 && true

# Lticontainer
HEALTHCHECK CMD /usr/local/bin/actlimit_check.sh

ADD \
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
    bin/actlimit_check.sh \
    /usr/local/bin/

ADD \
    etc/.bashrc \
    etc/.bash_profile \
    etc/.vimrc \
    /root/

ADD \
    etc/.bash_profile \
    etc/.bashrc \
    etc/.vimrc \
    /etc/skel/

ADD \
    etc/passwd.orig \
    etc/group.orig \
    /etc/

#
RUN chmod a+rx /usr/local/bin/* \
 && chmod a+r  /etc/skel/.vimrc /etc/skel/.bash* \
 && true

