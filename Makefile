# vi: set tabstop=4 noautoindent:

#
_LNG ?= C

ifeq ($(_LNG), JP)
NOTICE_SUFFIX = _jp
else
NOTICE_SUFFIX =
endif


PDMN = $(shell grep "^podman:" /etc/group | sed -e 's/:.*//')


all: install


jp:
	$(MAKE) _LNG=JP all


install: 
	[ -d /var/lib/jupyterhub ]   || mkdir /var/lib/jupyterhub
	[ -d /usr/local/etc/ltictr ] || mkdir -p /usr/local/etc/ltictr
	chmod 755 /usr/local/etc/ltictr
	rmdir /usr/local/etc/ltictr/notice_* 2> /dev/null || true
	[ -f /usr/local/etc/ltictr/jupyterhub_lticontainer_config.py ] \
	  && install -m 0644 etc/jupyterhub_lticontainer_config.py /usr/local/etc/ltictr/jupyterhub_lticontainer_config.py.new \
	  || install -m 0644 etc/jupyterhub_lticontainer_config.py /usr/local/etc/ltictr/
	[ -f /usr/local/etc/ltictr/lticontainerspawner_config.py ] \
	  && install -m 0644 etc/lticontainerspawner_config.py     /usr/local/etc/ltictr/lticontainerspawner_config.py.new \
	  || install -m 0644 etc/lticontainerspawner_config.py     /usr/local/etc/ltictr/
	[ -f /usr/local/etc/ltictr/ltictr_proxy.conf ] \
	  && install -m 0640 etc/ltictr_proxy.conf                 /usr/local/etc/ltictr/ltictr_proxy.conf.new \
	  || install -m 0640 etc/ltictr_proxy.conf                 /usr/local/etc/ltictr/
	[ -f /usr/lib/systemd/system/ltictr_proxy.service ] \
	  && install -m 0644 etc/ltictr_proxy.service              /usr/lib/systemd/system/ltictr_proxy.service.new \
	  || install -m 0644 etc/ltictr_proxy.service              /usr/lib/systemd/system/
	#
	[ -f /usr/local/etc/ltictr/notice_memory.txt ] || install -m 0644 etc/notice_memory$(NOTICE_SUFFIX).txt  /usr/local/etc/ltictr/notice_memory.txt
	[ -f /usr/local/etc/ltictr/notice_active.txt ] || install -m 0644 etc/notice_active$(NOTICE_SUFFIX).txt  /usr/local/etc/ltictr/notice_active.txt
	[ -f /usr/local/etc/ltictr/notice_sticky.txt ] || install -m 0644 etc/notice_sticky$(NOTICE_SUFFIX).txt  /usr/local/etc/ltictr/notice_sticky.txt
	#
	install -m 0755 etc/lticontainerspawner.py /usr/local/etc/ltictr/
	install -m 0755 sh/dockerpull.sh           /usr/local/bin/
	install -m 0755 sh/dockerpush.sh           /usr/local/bin/
	install -m 0755 sh/dockerrmi.sh            /usr/local/bin/
	#
	(cd src && make) || true
	(cd ltictr_proxy && make) || true
	install -m 0755 bin/ltictr_proxy_server    /usr/local/bin/ || true
	install -m 0755 bin/ltictr_api_server      /usr/local/bin/ || true
	#systemctl enable ltictr_proxy || true
	#
ifeq ($(PDMN), podman)
	[ -f /usr/lib/systemd/system/jupyterhub.service ] \
	  && install -m 0644 etc/jupyterhub.service  /usr/lib/systemd/system/jupyterhub.service.new \
	  || install -m 0644 etc/jupyterhub.service  /usr/lib/systemd/system/
	install -m 0644 etc/podman.socket            /usr/lib/systemd/system/
	install -m 0755 sh/chgrppodman.sh            /usr/local/bin/
	install -m 0750 bin/unlock_podman_containers /usr/local/bin/
	chgrp podman /usr/local/bin/unlock_podman_containers
	chmod u+s    /usr/local/bin/unlock_podman_containers
	chgrp podman /var/lib/containers/storage/overlay-containers/
	chmod g+rx   /var/lib/containers/storage/overlay-containers/
else
	[ -f /usr/lib/systemd/system/jupyterhub.service ] \
	  && install -m 0644 etc/jupyterhub.docker.service  /usr/lib/systemd/system/jupyterhub.service.new \
	  || install -m 0644 etc/jupyterhub.docker.service  /usr/lib/systemd/system/jupyterhub.service
endif
	#
	systemctl daemon-reload       || true
	systemctl enable jupyterhub   || true


clean:
	rm -f /var/lib/jupyterhub/*


uninstall: clean
	systemctl stop    jupyterhub   || true
	systemctl disable jupyterhub   || true
	systemctl stop    ltictr_proxy || true
	systemctl disable ltictr_proxy || true
	#
	rm -f  /var/lib/jupyterhub/*
	rm -f  /usr/local/etc/jupyterhub_lticontainer_config.py
	rm -f  /usr/local/etc/lticontainerspawner.py
	rm -f  /usr/local/etc/lticontainerspawner_config.py
	rm -f  /usr/local/etc/ltictr_proxy.conf
	#
	rm -rf /usr/local/etc/ltictr
	rm -f  /usr/local/bin/ltictr_proxy_server
	rm -f  /usr/local/bin/ltictr_api_server
	rm -f  /usr/local/bin/dockerpull.sh
	rm -f  /usr/local/bin/dockerpush.sh
	rm -f  /usr/local/bin/dockerrmi.sh
	rm -f  /usr/lib/systemd/system/jupyterhub.service
	rm -f  /usr/lib/systemd/system/ltictr_proxy.service
	#
	rm -f  /usr/local/bin/unlock_podman_containers
	rm -f  /usr/local/bin/chgrppodman.sh

