#!/usr/bin/env bash
# get_labextensions_path.sh
# JupyterLab の「システム」labextensions ディレクトリを 1 行だけ出力．

set -euo pipefail

python3 - <<'PY'
import os, sys
from pathlib import Path
import warnings

# 新方式のパス解決を有効化し，警告を抑止
os.environ.setdefault("JUPYTER_PLATFORM_DIRS", "1")
warnings.filterwarnings("ignore", category=DeprecationWarning)

# 1) 明示指定があればそれを優先
env = os.environ.get("JUPYTER_LABEXTENSIONS_DIR")
if env:
    print(env)
    sys.exit(0)

# 2) jupyter_core の system パスから labextensions を候補化
try:
    from jupyter_core.paths import SYSTEM_JUPYTER_PATH
    candidates = [Path(p)/"labextensions" for p in SYSTEM_JUPYTER_PATH]
except Exception:
    candidates = []

# 3) sys.prefix 配下の一般的な場所も候補
candidates.append(Path(sys.prefix)/"share"/"jupyter"/"labextensions")

# 4) 既存ディレクトリを優先して出力
for c in candidates:
    try:
        if c.is_dir():
            print(str(c))
            sys.exit(0)
    except Exception:
        pass

# 5) 存在未確認でも最有力候補を出力
if candidates:
    print(str(candidates[0]))
else:
    print("/usr/local/share/jupyter/labextensions")
PY
