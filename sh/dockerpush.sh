#!/bin/bash
#
#  docker login -u <username> -p <password>  <repository_host>
#  ./dockerpush.sh <tag_of_updated>  [tag_of_reposirory]
#
#  ex.) docker login -u alice -p password_of_alice  www.nsl.tuis.ac.jp:5000
#       ./dockerpush.sh 20230503 latest
#

DKRREP="www.nsl.tuis.ac.jp:5000"

#
if [ "$1" != "" ]; then
    TAG=$1
else
    echo "usage... $0 <tag_of_updated>  [tag_of_repository]"
    echo "    ex.) docker login -u alice -p password_of_alice  www.nsl.tuis.ac.jp:5000"
    echo "         $0 20230503 latest"
    exit 1
fi

REPTAG=$TAG
if [ "$2" != "" ]; then
    REPTAG=$2
fi
#
LST=`docker images | awk -F" " '{print $1":"$2}' | grep -e ":${TAG}$" | grep -v "$DKRREP" | sed -e "s/localhost\///" `

echo $LST
for IMG in $LST ; do
    REP=`echo $IMG | awk -F":" '{print $1}'`
    REP=$DKRREP"/"$REP":"$REPTAG 
    echo ----------------------------------------------------------
    echo "Push $IMG to $REP"
    #
    docker tag  $IMG  $REP
    docker push $REP || (echo "did you login? ...  docker login -u <username> -p <password> $DKRREP" && exit 1)
    docker rmi  $REP
done

