#!/bin/bash

if [ "$1" = "" ]; then
    echo "usage... $0 group_name"
    exit 1
else
    GRP=$1
fi

chgrp -R $GRP /var/run/podman
chmod g+rwxs  /var/run/podman
chmod g+rw    /var/run/podman/*

