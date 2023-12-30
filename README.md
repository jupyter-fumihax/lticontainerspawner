# LTIContainerSpawner

## Spawners for Integration System of Moodle(LMS) and JupyterHub

### Install
```
# git clone https://gitlab.nsl.tuis.ac.jp/iseki/lticontainerspawner.git
# cd lticontainerspawner
# make install
```
or
```
# wget https://github.com/jupyter-fumihax/lticontainerspawner/archive/refs/tags/v#.#.#.tar.gz
# zcat v#.#.#.tar.gz | tar xfv -
# cd lticontainerspawner-#.#.#
# make install
```

### Setup
##### for Ltictr_Proxy (if you use Ltictr_Proxy)
```
# vi /usr/local/etc/ltictr_proxy.conf
# vi /usr/lib/systemd/system/ltictr_proxy.service
# systemctl enable ltictr_proxy.service
# systemctl start  ltictr_proxy.service
```
##### for LTIContainerSpawner
```
# vi /usr/local/etc/lticontainerspawner_config.py
```

##### for JupyterHub 
``` 

# vi /usr/local/etc/jupyterhub_lticontainer_config.py
# vi /usr/lib/systemd/system/jupyterhub.service
# systemctl enable jupyterhub.service
# systemctl start  jupyterhub.service
```


### Wiki
- English:  https://github.com/jupyter-fumihax/lticontainerspawner/wiki/Moodle-&-JupyterHub
- Japanese: https://github.com/jupyter-fumihax/lticontainerspawner/wiki/Moodle-&-JupyterHub-(J)

### Please see also bellow Wiki (Moodle Module)
- English:  https://github.com/moodle-fumihax/mod_lticontainer/wiki
- Japanese: https://github.com/moodle-fumihax/mod_lticontainer/wiki/mod_lticontainer-(J)

### License

2021 Fumi.Iseki <iseki@rsch.tuis.ac.jp> and Masanori Urano 

This program is free software: you can redistribute it and/or modify it under
the terms of the BSD 3-Clause License as published by the Free Software
