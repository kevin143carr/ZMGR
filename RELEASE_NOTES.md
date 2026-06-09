# ZMGR v2.2.0 NextGen1

Hardware-tested NextGen1 routing and forwarding release.

## Changes

- Added `Who Is There` multi-hop route learning with checksum-protected discovery packets.
- Added `ZMROUTES.TXT` next-hop routing for forwarded `.FIL` uploads.
- Fixed relay behavior so forwarded files do not bounce back to the inbound COM port.
- Added `MULTI.FIL` support for one payload delivered to multiple named receivers.
- Added first-run setup for blank `COMPUTERNAME` and optional blank `PORTS`.
- Updated default `ZMOPTIONS` with `-o1` so PDZM overwrites received files instead of auto-renaming.
- Added COM diagnostics prompt to update `PORTS` from detected non-mouse serial ports.
- Improved ZMDIAG, first-run setup, and Forward File screens with double-border UI.
- Removed startup debug output from the file-transfer path.

## Build

Built with Borland C++ 3.1 through DOSBox-X using `./dosbox/build_zmgr.sh`.
