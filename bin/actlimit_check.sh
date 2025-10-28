#!/bin/bash
#
# Script to exit after a span of the time $NB_ACTLIMIT (s) 
#

STTMFL="/tmp/.system_uptime"

if [ "$NB_ACTLIMIT" = "" ]; then
    exit 1
fi
# No Limit
if [ "$NB_ACTLIMIT" = "0" ]; then
    exit 0
fi

UPTM=`awk -F" " '{print $1}' /proc/uptime | awk -F"." '{print $1}'`
if [ "$UPTM" = "" ]; then
    exit 1
fi
if [ ! -f $STTMFL ]; then
    echo $UPTM > $STTMFL
    exit 0
fi

STTM=`cat $STTMFL`
if [ "$STTM" = "" ]; then
   exit 1
fi
UPTM=`expr $UPTM - $STTM`
if [ "$UPTM" -gt "$NB_ACTLIMIT" ]; then
    kill -9 -- -1
fi

exit 0
