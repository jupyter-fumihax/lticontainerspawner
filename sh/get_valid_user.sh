#!/bin/bash

while read LINE ;
do
    USERID=`echo $LINE| awk -F":" '{print $3}'`
    LSHELL=`echo $LINE| awk -F":" '{print $7}'`
     
    if [ "$USERID" -gt "1000" -a "$LSHELL" = "/bin/bash" ] ; then
         echo $LINE | sed -e "s/\/Linux:/:/" 
    fi
done
