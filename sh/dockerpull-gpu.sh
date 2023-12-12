#!/bin/bash
#
#  ./dockerpull-gpu <tag> [category_of_images] 
#
#  ex.) ./dockerpull-gpu latest jupyterhub-ltictr
#

LST="base-notebook-gpu tensorflow-notebook-gpu scipy-notebook-gpu"

#
TAGSTR="latest"
IDSTR="jupyterhub-ltictr"
DKRREP="www.nsl.tuis.ac.jp:5000"

if [ "$1" != "" ]; then
    TAGSTR=$1
else
    echo "usage... $0 <tag_of_pulled>  [category_of_images]"
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

