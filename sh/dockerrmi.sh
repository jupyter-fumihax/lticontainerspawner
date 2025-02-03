#!/bin/bash
#
# ex.) ./dockerrmi.sh 20240617 jupyterhub
#

if [ "$1" != "" ]; then
    TAG=$1
else
    echo "usage... $0 <tag_of_images> [key_word]"
    echo "    ex.) $0 20240617 jupyterhub"
    exit 1
fi
#
if [ "$2" != "" ]; then
    LST=`docker images | awk -F" " '{print $1":"$2}' | grep -e ":${TAG}$" | grep $2`
else
    LST=`docker images | awk -F" " '{print $1":"$2}' | grep -e ":${TAG}$"`
fi

for IMG in $LST ; do
    echo "delete .... $IMG"
    docker rmi  $IMG
done

