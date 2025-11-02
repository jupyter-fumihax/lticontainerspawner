#!/bin/bash
#
#  ./dockerpull-gpu.sh [tag] [category_of_images] 
#
#  ex.) ./dockerpull-gpu.sh latest jupyterhub-ltictr
#

LST="base-notebook-gpu tensorflow-notebook-gpu scipy-notebook-gpu cuda-ubuntu20"

#
TAGSTR="latest"
IDSTR="jupyterhub-ltictr"
#DKRREP="docker.io/fumihax"
DKRREP="ghcr.io/jupyter-fumihax"

if [ "$1" != "" ]; then
    TAGSTR=$1
    if [ "$2" != "" ]; then
        IDSTR=$2
    fi
fi

#
for IMG in $LST ; do
    echo 
    echo Pull $DKRREP/$IDSTR.${IMG}:${TAGSTR}
    IMGID=`docker images | grep "$DKRREP/$IDSTR.$IMG" | grep $TAGSTR | awk -F" " '{print $3}'`
    docker pull $DKRREP/$IDSTR.${IMG}:${TAGSTR} 
    #
    if [ "$IMGID" != "" ]; then
        NONE=`docker images | grep $IMGID | grep "\<none\>"`
        if [ "$NONE" != "" ]; then
            TMPN="$DKRREP/$IDSTR.$IMG:del"
            docker tag $IMGID $TMPN
            docker rmi $TMPN
        fi
    fi   
done

