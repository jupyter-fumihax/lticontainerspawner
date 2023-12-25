# vi: set tabstop=4 noautoindent:

#
all: install


install: 
	[ -d /var/lib/jupyterhub ] || mkdir /var/lib/jupyterhub
	[ -f /usr/local/etc/jupyterhub_lticontainer_config.py ] || install -m 0644 etc/jupyterhub_lticontainer_config.py /usr/local/etc
	[ -f /usr/local/etc/lticontainerspawner_config.py ]     || install -m 0644 etc/lticontainerspawner_config.py     /usr/local/etc
	[ -f /usr/lib/systemd/system/jupyterhub.service ]       || install -m 0644 etc/jupyterhub.service                /usr/lib/systemd/system
	[ -f /usr/lib/systemd/system/ltictr_proxy.service ]     || install -m 0644 etc/ltictr_proxy.service              /usr/lib/systemd/system
	[ -f /usr/local/etc/ltictr_proxy.conf ]                 || install -m 0640 etc/ltictr_proxy.conf                 /usr/local/etc
	install -m 0644 etc/podman.socket            /usr/lib/systemd/system
	install -m 0755 etc/lticontainerspawner.py   /usr/local/etc
	install -m 0755 bin/ltictr_proxy_server      /usr/local/bin
	install -m 0755 bin/ltictr_api_server   	 /usr/local/bin
	install -m 0755 sh/chgrppodman.sh            /usr/local/bin
	install -m 0755 sh/dockerpull.sh             /usr/local/bin
	install -m 0755 sh/dockerpush.sh             /usr/local/bin
	install -m 0755 sh/dockerrmi.sh              /usr/local/bin
	systemctl enable jupyterhub   || true
	systemctl enable ltictr_proxy || true
	systemctl daemon-reload
	install -m 0750 bin/unlock_podman_containers /usr/local/bin
	chgrp podman /usr/local/bin/unlock_podman_containers
	chmod u+s    /usr/local/bin/unlock_podman_containers
	chgrp podman /var/lib/containers/storage/overlay-containers
	chmod g+rx   /var/lib/containers/storage/overlay-containers


clean:
	rm -f /var/lib/jupyterhub/*


uninstall: clean
	systemctl stop jupyterhub      || true
	systemctl disable jupyterhub   || true
	systemctl stop ltictr_proxy    || true
	systemctl disable ltictr_proxy || true
	rm -f /var/lib/jupyterhub/*
	rm -f /usr/local/etc/jupyterhub_lticontainer_config.py
	rm -f /usr/local/etc/lticontainerspawner.py
	rm -f /usr/local/etc/lticontainerspawner_config.py
	rm -f /usr/lib/systemd/system/jupyterhub.service
	rm -f /usr/lib/systemd/system/ltictr_proxy.service
	rm -f /usr/local/etc/ltictr_proxy.conf
	rm -f /usr/local/bin/ltictr_proxy_server
	rm -f /usr/local/bin/ltictr_api_server
	rm -f /usr/local/bin/unlock_podman_containers
	rm -f /usr/local/bin/chgrppodman.sh
	rm -f /usr/local/bin/dockerpull.sh
	rm -f /usr/local/bin/dockerpush.sh
	rm -f /usr/local/bin/dockerrmi.sh
	install -m 0755 bin/chgrppodman.sh          /usr/local/bin
	install -m 0755 bin/dockerpull.sh           /usr/local/bin
	install -m 0755 bin/dockerpush.sh           /usr/local/bin
	install -m 0755 bin/dockerrmi.sh            /usr/local/bin
	systemctl daemon-reload

