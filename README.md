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

`ZMGR.EXE run` opens a startup menu:

- `1`: Manager Forwarding, the original continuous upload/download/forwarding loop.
- `2`: Auto Download, repeatedly launches PDZM receive mode and skips upload forwarding checks.
- `3`: Serial Pass-through, bridges two selected COM ports byte-for-byte and watches for lightweight ZModem signatures.
- `4`: Port Diagnostics, reports BIOS-listed COM ports, basic UART response, and mouse-driver presence.
- `X`: exit.

During the manager or auto-download loop pause:

- `X`: exit ZMGR
- `P`: pause
- `C`: continue from pause

During serial pass-through:

- `X`: exit pass-through mode and return to the menu.

After upload handling, ZMGR now returns to this keyboard window before scanning the upload folder again.

## Serial Diagnostics

Port diagnostics target real DOS hardware first. ZMGR reads the BIOS Data Area COM table for COM1-COM4 base addresses, then performs a small UART scratch-register check when a base address is present. Mouse diagnostics query INT 33h for driver presence and use the extended AX=0024h call when available to report mouse type and IRQ. Serial mouse IRQs are mapped to likely COM ports when only one BIOS-listed candidate exists. If the result is ambiguous and DOS 6.0 or newer is detected, ZMGR can run `MSD.EXE /P REPORT.TXT` and parse the report's `Mouse` section as a fallback.

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
