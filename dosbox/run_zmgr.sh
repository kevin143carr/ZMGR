#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
HELPER="${DOSBOX_HELPER:-/Users/kevincarr/projects/DOSBox-Dev-Support/dosbox_helper.py}"
CONFIG="$SCRIPT_DIR/run_zmgr.conf"

if [[ -f "$HELPER" ]]; then
  exec python3 "$HELPER" run --launch-mode binary --fastlaunch --config "$CONFIG"
fi

DOSBOX="${DOSBOX_BIN:-/Applications/dosbox-x.app/Contents/MacOS/dosbox-x}"
exec "$DOSBOX" -conf "$CONFIG" -fastlaunch
