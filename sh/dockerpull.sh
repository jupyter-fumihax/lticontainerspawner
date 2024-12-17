#!/bin/bash
#
#  ./dockerpull.sh <tag> [category_of_images] 
#
#  ex.) ./dockerpull.sh latest jupyterhub-ltictr
#

LST="base-notebook singleuser datascience-notebook tensorflow-notebook scipy-notebook vhtec-notebook \
     xeus-cling java-notebook php-notebook swift-tensorflow jupyterlab-broccoli"

#
TAGSTR="latest"
IDSTR="jupyterhub-ltictr"
DKRREP="www.nsl.tuis.ac.jp:5000"

if [ "$1" != "" ]; then
    TAGSTR=$1
else
    echo "usage... $0 <tag_of_pulled>  [category_of_images]"
    echo "    ex.) $0 latest jupyterhub-ltictr"
    exit 1
fi
if [ "$2" != "" ]; then
    IDSTR=$2
fi

#
for IMG in $LST ; do
    echo 
    echo Pull $DKRREP/$IDSTR/${IMG}:${TAGSTR}
    IMGID=`docker images | grep "$DKRREP/$IDSTR/$IMG" | grep $TAGSTR | awk -F" " '{print $3}'`
    docker pull $DKRREP/$IDSTR/${IMG}:${TAGSTR} 
    #
    if [ "$IMGID" != "" ]; then
        NONE=`docker images | grep $IMGID | grep "\<none\>"`
        if [ "$NONE" != "" ]; then
            TMPN="$DKRREP/$IDSTR/$IMG:del"
            docker tag $IMGID $TMPN
            docker rmi $TMPN
        fi
    fi   
done

