# ZMGR v0.1.0

Initial local release package for GitHub Releases.

## Changes

- Renamed the generated Borland compiler config from `ZMGR.CFG` to `BUILD.CFG`.
- Preserved `ZMMGR.CFG` as the runtime configuration file.
- Fixed the main loop so upload handling no longer skips the `X`, `P`, and `C` keyboard window.
- Initialized the process executor used by once-command handling.
- Updated DOSBox-X helper launch mode to use direct binary execution.

## Build

Built with Borland C++ 3.1 through DOSBox-X using `./dosbox/build_zmgr.sh`.
