#!/bin/bash

SSH_HOST=202.26.144.41

#
if [ -n "$PASSWORD" ]; then
  echo "$PASSWORD"
  exit 0
fi

read -p  "userid: " SSH_USER
read -sp "passwd: " SSH_PASS

#SSH_USER=
#SSH_PASS=

if [ "$SSH_USER" = "" -o "$SSH_PASS" = "" ]; then
    exit 1
fi

CMD="getent passwd"

export PASSWORD=$SSH_PASS
export SSH_ASKPASS=$0
export DISPLAY=:0.0

#setsid ssh $SSH_USER@$SSH_HOST $CMD 
exec setsid ssh $SSH_USER@$SSH_HOST $CMD | ./get_valid_user.sh
