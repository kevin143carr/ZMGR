#!/bin/bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
BRANCH_NAME="$(git -C "$ROOT_DIR" rev-parse --abbrev-ref HEAD)"
COMMIT_SHA="$(git -C "$ROOT_DIR" rev-parse --short HEAD)"
BUILD_DATE="$(date '+%Y-%m-%d %H:%M:%S %Z')"
PACKAGE_NAME="$BRANCH_NAME"
DIST_ROOT="$ROOT_DIR/dist/test"
PACKAGE_DIR="$DIST_ROOT/$PACKAGE_NAME"
ZIP_PATH="$DIST_ROOT/$PACKAGE_NAME.zip"

cd "$ROOT_DIR"

./dosbox/build_zmgr.sh

rm -rf "$DIST_ROOT"
mkdir -p "$PACKAGE_DIR"

cp ZMGR.EXE "$PACKAGE_DIR/"
cp ZMMGR.CFG "$PACKAGE_DIR/"
cp README.md "$PACKAGE_DIR/"
cp LICENSE "$PACKAGE_DIR/"
cp VERSION "$PACKAGE_DIR/"
cp RELEASE_NOTES.md "$PACKAGE_DIR/"

if [[ -f FORWARDTO.txt ]]; then
  cp FORWARDTO.txt "$PACKAGE_DIR/"
fi

if [[ -f uptest.bat ]]; then
  cp uptest.bat "$PACKAGE_DIR/"
fi

if [[ -f downtest.bat ]]; then
  cp downtest.bat "$PACKAGE_DIR/"
fi

cat > "$PACKAGE_DIR/BUILDINFO.TXT" <<EOF
Package: $PACKAGE_NAME
Branch: $BRANCH_NAME
Commit: $COMMIT_SHA
Built: $BUILD_DATE
Purpose: Hardware test package, not a formal release.
EOF

rm -f "$ZIP_PATH"
(
  cd "$DIST_ROOT"
  zip -qr "$PACKAGE_NAME.zip" "$PACKAGE_NAME"
)

echo "Created $PACKAGE_DIR"
echo "Created $ZIP_PATH"
