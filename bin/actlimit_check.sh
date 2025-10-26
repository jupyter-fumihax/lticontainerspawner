#!/bin/bash
#
# Script to exit after a span of the time $NB_ACTLIMIT (s) 
#
#    v2.0 20251024
#

set -euo pipefail

STTMFL="/tmp/.system_uptime"
GRACE=15   # TERM -> KILL 猶予秒

case "${NB_ACTLIMIT:-}" in
  ""|"0") exit 0 ;;
esac

LIMIT=$NB_ACTLIMIT
[ "$LIMIT" -le 0 ] && exit 0

UPTM=$(awk '{print int($1)}' /proc/uptime 2> /dev/null || echo 0)
[ "$UPTM" -le 0 ] && exit 0

if [ ! -f "$STTMFL" ]; then
  echo "$UPTM" > "$STTMFL"
  exit 0
fi

STTM=$(cat "$STTMFL" 2>/dev/null || echo "$UPTM")
DIFF=$(( UPTM - STTM ))
[ "$DIFF" -lt 0 ] && { echo "$UPTM" > "$STTMFL"; exit 0; }

if [ "$DIFF" -ge $(( LIMIT * 9 / 10 )) ] && [ "$DIFF" -lt "$LIMIT" ]; then
  echo "[actlimit_check] warning: ${DIFF}s/${LIMIT}s elapsed" >&2
fi

if [ "$DIFF" -gt "$LIMIT" ]; then
  echo "[actlimit_check] limit exceeded: ${DIFF}s > ${LIMIT}s → stopping" >&2
  kill -TERM -- -1 || true
  sleep "$GRACE"
  kill -KILL -- -1 || true
  exit 1
fi

exit 0

