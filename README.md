# ZMGR
ZModem Manager - A PDZM (Public Domain ZModem) wrapper for continuous transfer and other cool stuff.

ZMGR is a wrapper for the PDZM (Public Domain ZModem) created by Peter Mandrella, 1994-98.  PDZM is a command line utility
that allows for zmodem transfer via null modem cable or dial up modem.

ZMGR as a wrapper managers what zmodem can do, and can run continuously as well as execute commands once an item is downloaded.

## DOSBox-X support

Local DOSBox-X support lives in `dosbox/` and follows the same pattern as `/Users/kevincarr/projects/DOSBox-Dev-Support`:

```bash
./dosbox/build_zmgr.sh
./dosbox/run_zmgr.sh
```

The scripts mount `/Users/kevincarr/projects/C` as DOS drive `D:`, enter `D:\ZMGR`, and call the existing `build.bat` or `ZMGR.EXE`. They use `/Users/kevincarr/projects/DOSBox-Dev-Support/dosbox_helper.py` in macOS app-bundle launch mode when it is available. Override paths with `DOSBOX_HELPER=/path/to/dosbox_helper.py` or `DOSBOX_BIN=/path/to/dosbox-x`.
