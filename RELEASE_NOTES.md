# ZMGR v2.1.0 NextGen1

First NextGen1 text-mode release.

## Changes

- Split the application into focused DOS executables:
  - `ZMGR.EXE`: text-mode launcher menu.
  - `ZMFT.EXE`: file transfer and forwarding.
  - `ZMSER.EXE`: serial pass-through.
  - `ZMDIAG.EXE`: serial diagnostics and route discovery.
  - `ZMCFG.EXE`: text-mode configuration editor.
- Added a centered double-border launcher menu with configurable startup countdown.
- Added `STARTUPSTATE` and `MENUCOUNTDOWN` runtime settings.
- Replaced per-command transfer params with `PORTS` and shared `ZMOPTIONS`.
- Added a text-mode configuration editor that preserves comments and creates `ZMMGR.BAK`.
- Added improved COM diagnostics with base address, standard IRQ, UART type, and baud guidance.
- Added `Who Is There` route discovery, writing learned next-hop routes to `ZMROUTES.TXT`.
- Added full-path forwarding wizard while preserving `.FIL` forwarding files.

## Build

Built with Borland C++ 3.1 through DOSBox-X using `./dosbox/build_zmgr.sh`.
