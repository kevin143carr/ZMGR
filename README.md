# ZMGR

Version: v0.1.0

ZModem Manager is a Borland C++ 3.1 MS-DOS wrapper for PDZM (Public Domain ZModem), created by Peter Mandrella, 1994-1998. PDZM handles ZModem transfers over null modem cable or dial-up modem; ZMGR adds continuous upload/download handling, forwarding files, and optional post-transfer commands.

## Runtime Config

Runtime behavior is controlled by `ZMMGR.CFG`.

Important keys:

- `COMPUTERNAME`: local machine name, used to identify `.FIL` files intended for this machine.
- `DOWNLOADFOLDER`: folder where PDZM receives files.
- `UPLOADFOLDER`: folder scanned before download mode for outgoing files.
- `KEEPFOLDER`: destination for files forwarded to this machine.
- `EXECPATH`: path to `ZM.EXE`.
- `DOWNLOADPARAMS`, `UPLOADPARAMS`, `FORWARDPARAMS`: PDZM argument templates.
- `PAUSETIME`: seconds to wait between loop iterations while allowing `X`, `P`, and `C` keyboard commands.

`BUILD.CFG` is the generated Borland compiler configuration. It is not the runtime configuration file.

## Controls

During the loop pause:

- `X`: exit ZMGR
- `P`: pause
- `C`: continue from pause

After upload handling, ZMGR now returns to this keyboard window before scanning the upload folder again.

## DOSBox-X Build And Run

Local DOSBox-X support lives in `dosbox/` and follows the same pattern as `/Users/kevincarr/projects/DOSBox-Dev-Support`:

```bash
./dosbox/build_zmgr.sh
./dosbox/run_zmgr.sh
```

The scripts mount `/Users/kevincarr/projects/C` as DOS drive `D:`, enter `D:\ZMGR`, and call the existing `build.bat` or `ZMGR.EXE`. They use `/Users/kevincarr/projects/DOSBox-Dev-Support/dosbox_helper.py` in binary launch mode when it is available. Override paths with `DOSBOX_HELPER=/path/to/dosbox_helper.py` or `DOSBOX_BIN=/path/to/dosbox-x`.

## Release Build

Build the executable first:

```bash
./dosbox/build_zmgr.sh
```

The local release package for v0.1.0 is assembled under `dist/v0.1.0/` and includes:

- `ZMGR.EXE`
- `ZMMGR.CFG`
- `README.md`
- `LICENSE`
- `VERSION`
- `RELEASE_NOTES.md`

Upload the ZIP from `dist/v0.1.0/` to the matching GitHub release.
