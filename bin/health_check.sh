#!/bin/bash
#
# health_check.sh
#    Script to exit after a span of the active time and over the memory
#
#    v1.0 20251025
#

set -euo pipefail

#
NOTICE_FILE="${NB_DIR}/jupyter/works/.jnotice.txt"

ACT_ALERT=90    # % 時間警告
MEM_ALERT=90    # % メモリ警告
ACT_GRACE=15    # TERM → KILL 猶予時間


NB_WRKDIR="${NB_WRKDIR:-$NB_DIR}" 
ACTIVE_NOTICE_PATH="${NB_WRKDIR}/.active_notice.txt"
MEMORY_NOTICE_PATH="${NB_WRKDIR}/.memory_notice.txt"
SYS_TM_FILE="/tmp/.system_uptime"

ACT_LIMIT="${NB_ACTLIMIT//[^0-9]/}"
ACT_LIMIT="${ACT_LIMIT:-0}"


# ========== 関数 ===========

# 指定ファイルの内容を1行で返す．無ければ空文字．
read_notice_file() {
  local p="$1"
  [ -f "$p" ] || { echo ""; return; }
  # 末尾改行を除去して取り出す
  head -c 10000 "$p" 2>/dev/null | tr -d '\r' | sed -e 's/[[:space:]]*$//'
}


# NOTICE に書き，stderr にも同文を出す．
emit_notice() {
  local msg="$1"
  mkdir -p "$(dirname "$NOTICE_FILE")" 2>/dev/null || true
  if [ -f "$NOTICE_FILE" ]; then
    local cur; cur="$(cat "$NOTICE_FILE" 2>/dev/null || true)"
    [[ "$cur" == "$msg" ]] && { echo "[healthcheck] $msg" >&2; return 0; }
  fi
  echo "$msg" > "$NOTICE_FILE" 2>/dev/null || true
  echo "[healthcheck] $msg" >&2
}


# コンテナの停止
graceful_stop() {
  rm -f $NOTICE_FILE
  kill -TERM -- -1 2>/dev/null || true
  sleep $ACT_GRACE
  kill -KILL -- -1 2>/dev/null || true
}


# 使用メモリ率の獲得
get_mem_pct() {
  local mem="/sys/fs/cgroup/memory.current" max="/sys/fs/cgroup/memory.max"
  if [ ! -f "$mem" ] && [ -f "/sys/fs/cgroup/memory/memory.usage_in_bytes" ]; then
    mem="/sys/fs/cgroup/memory/memory.usage_in_bytes"
    max="/sys/fs/cgroup/memory/memory.limit_in_bytes"
  fi
  [ -f "$mem" ] || { echo -1; return; }
  [ -f "$max" ] || { echo -1; return; }
  local cur maxv; cur="$(cat "$mem" 2>/dev/null || echo 0)"; maxv="$(cat "$max" 2>/dev/null || echo 0)"
  [[ "$maxv" == "max" || "$maxv" -eq 0 ]] && { echo -1; return; }
  echo $(( cur * 100 / maxv ))
}


# ========== チェック ===========

# ===== 稼働時間 =====
UP_TIME="$(awk '{print int($1)}' /proc/uptime 2>/dev/null || echo 0)"

if (( ACT_LIMIT > 0 && UP_TIME > 0 )); then
  [ -f $SYS_TM_FILE ] || (echo $UP_TIME > $SYS_TM_FILE; rm -f "$NOTICE_FILE")
  sttm=$(cat $SYS_TM_FILE 2>/dev/null || echo $UP_TIME)
  diff=$(( UP_TIME - sttm ))

  # 警告 .active_notice の内容を表示
  alert_tm=$(( ACT_LIMIT * ACT_ALERT / 100 ))
  if (( diff >= alert_tm )); then
    act_msg="$(read_notice_file "$ACTIVE_NOTICE_PATH")"
    if [ ! -z "$act_msg" ]; then
      emit_notice "$act_msg"
    fi
    # 超過時は終了．
    if (( diff > ACT_LIMIT )); then
      graceful_stop
      exit 1
    fi
  fi
fi


# ===== メモリ =====
mem_pct="$(get_mem_pct)"
if (( mem_pct >= 0 )); then
  mem_msg="$(read_notice_file "$MEMORY_NOTICE_PATH")"
  if [ ! -z "$mem_msg" ]; then
    if (( mem_pct >= MEM_ALERT )); then
      emit_notice "$mem_msg"
    fi
  fi
fi

exit 0

