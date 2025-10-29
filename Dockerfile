#
# Usage
# 1. change FROM line
# 2. execute docker build command
# ex.) cd ..
#      docker build . --format=docker -t jupyterhub-ltictr/singleuser:20251029
#

FROM docker.io/jupyter/base-notebook
#FROM docker.io/jupyterhub/singleuser
#FROM docker.io/jupyter/datascience-notebook
#FROM docker.io/jupyter/tensorflow-notebook
#FROM docker.io/jupyter/scipy-notebook


USER root

#
RUN apt-get update \
 && apt-get install -y --no-install-recommends \
#    nodejs npm \
    language-pack-ja-base language-pack-ja \
    fonts-ipafont-gothic fonts-ipafont-mincho \
    fonts-noto-cjk fonts-noto-cjk-extra \
    fonts-takao-pgothic fonts-takao-gothic fonts-takao-mincho \
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


############################################

# Jupyter Notice  (need nodejs, npm)
WORKDIR /tmp/jhub-notice
RUN mkdir -p src

COPY \
    labextension_jnotice/package.json \
    labextension_jnotice/tsconfig.json \
    labextension_jnotice/webpack.config.cjs \
    ./
COPY labextension_jnotice/src/index.ts src

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
    nodejs npm \
 && npm install --no-audit --no-fund  \
 && npx webpack --config webpack.config.cjs --mode=production \
 && apt-get -y remove nodejs npm \
 && apt-get -y clean \
 && rm -rf /var/lib/apt/lists/* \
 && true

RUN set -eux; \
    LABEXT="/opt/conda/share/jupyter/labextensions/jnotice"; \
    mkdir -p "$LABEXT/static"; \
    cp -a static/* "$LABEXT/static/"; \ 
    cp -a package.json "$LABEXT/"; \
    chmod -R a+rX "$LABEXT" 

COPY labextension_jnotice/90-jnotice.json  /opt/conda/etc/jupyter/jupyter_server_config.d/
RUN  chmod a+r /opt/conda/etc/jupyter/jupyter_server_config.d/90-jnotice.json

WORKDIR /
RUN rm -rf /tmp/jhub-notice


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
RUN chmod a+rx /usr/local/bin/* \
 && chmod a+r  /etc/skel/.vimrc /etc/skel/.bash* \
 && true

