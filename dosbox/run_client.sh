#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
HELPER="${DOSBOX_HELPER:-/Users/kevincarr/projects/DOSBox-Dev-Support/dosbox_helper.py}"
CONFIG="$SCRIPT_DIR/run_client.conf"
SERVER_HOST="${1:-${DOSBOX_SERVER:-}}"
TMP_CONFIG=""

if [[ -z "$SERVER_HOST" ]]; then
  echo "Usage: $0 <server-host-or-ip>" >&2
  echo "Or set DOSBOX_SERVER=<server-host-or-ip>" >&2
  exit 1
fi

TMP_CONFIG="$(mktemp "${TMPDIR:-/tmp}/zmgr-client.XXXXXX")"
sed "s/server:127.0.0.1/server:${SERVER_HOST}/" "$CONFIG" > "$TMP_CONFIG"

if [[ -f "$HELPER" ]]; then
  exec python3 "$HELPER" run --launch-mode binary --fastlaunch --config "$TMP_CONFIG"
fi

DOSBOX="${DOSBOX_BIN:-/Applications/dosbox-x.app/Contents/MacOS/dosbox-x}"
exec "$DOSBOX" -conf "$TMP_CONFIG" -fastlaunch
