#
# Usage
# 1. change FROM line
# 2. execute docker build command
# ex.) cd ..
#      docker build . --format=docker -t jupyterhub/singleuser:temp
#

FROM quay.io/jupyterhub/singleuser
#FROM quay.io/jupyter/base-notebook
#FROM quay.io/jupyter/datascience-notebook
#FROM quay.io/jupyter/tensorflow-notebook
#FROM quay.io/jupyter/scipy-notebook


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
RUN set -eux \
 && $CONDA_HOME/bin/conda update -n base -c conda-forge conda -y \
 && $CONDA_HOME/bin/conda config --add channels conda-forge \
 && $CONDA_HOME/bin/conda config --add channels defaults \
 && $CONDA_HOME/bin/conda config --set channel_priority strict \
 && $CONDA_HOME/bin/conda config --remove-key pinned_packages || true \
 && rm -f $CONDA_HOME/conda-meta/pinned || true \
 && $CONDA_HOME/bin/conda install --prefix $CONDA_HOME -c defaults "blas=1.0=openblas" -y \
 # conda-forge
 && $CONDA_HOME/bin/conda install --prefix $CONDA_HOME -c conda-forge \
      pip setuptools wheel \
      jupyterlab-language-pack-ja-jp ipywidgets pillow pandas ipycanvas \
      numpy matplotlib ipympl seaborn -y \
 && $CONDA_HOME/bin/conda install --prefix $CONDA_HOME -c conda-forge -c plotly plotly -y \
 && $CONDA_HOME/bin/conda clean --all -y \
 && $CONDA_HOME/bin/pip install --prefix $CONDA_HOME --no-cache-dir japanize-matplotlib jbturtle \
 && true

