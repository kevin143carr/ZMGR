# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

ZMGR (ZModem Manager) is a **Borland C++ 3.1, 16-bit MS-DOS** application. It is a launcher/orchestrator
around PDZM (Public Domain ZModem, by Peter Mandrella), handling file transfer, forwarding, and
multi-hop routing over null-modem serial links or dial-up modems between DOS machines. There is no
modern/native build target — everything compiles and runs only inside DOSBox-X against a real
Borland C++ 3.1 install.

Five separate DOS `.EXE`s are produced from this one source tree:

| EXE | Source | Purpose |
|---|---|---|
| `ZMGR.EXE` | `ZMGR.CPP` | Startup menu; spawns the other EXEs via `spawnl` and returns when they exit. Self-contained, no shared classes. |
| `ZMFT.EXE` | `ZMFT.CPP` | The real engine: upload/forward/download loop (`TRANSFER` mode) or the Forward File wizard (`FORWARD` mode). Thin `main()` over `ProcessManager`. |
| `ZMSER.EXE` | `ZMSER.CPP` | Manual two-port serial passthrough/bridge (operator picks both COM ports). |
| `ZMDIAG.EXE` | `ZMDIAG.CPP` | COM port hardware diagnostics + the "Who Is There" route-discovery protocol. |
| `ZMCFG.EXE` | `ZMCFG.CPP` | Standalone TUI editor for `ZMMGR.CFG`. Has its own tiny INI parser; does **not** reuse `ConfigClass`. |

`ZMGR.CPP` and `ZMCFG.CPP` each implement their own small `read_config_value`/`update_config_value`
INI read/write pair directly against `ZMMGR.CFG` instead of going through `ConfigClass` — this is
intentional duplication (those two EXEs don't link the rest of the class hierarchy), not a bug to
"fix" by unifying them.

## Build / run / package

All building and running happens inside DOSBox-X; there is no host-native toolchain.

```bash
./dosbox/build_zmgr.sh        # boots DOSBox-X, runs build.bat (bcc/tlink via ZMGR.MAK), writes BUILD.LOG
./dosbox/run_zmgr.sh          # boots DOSBox-X, runs ZMGR.EXE run
./dosbox/run_client.sh <host> # boots DOSBox-X as a nullmodem TCP client against <host>:5000 (for two-machine testing)
./package-test.sh             # builds, then zips a hardware-test package into dist/test/<branch>/
```

The DOSBox scripts mount `/Users/kevincarr/projects/C` as DOS drive `D:` (this repo lives at
`D:\ZMGR`) and `/Volumes/WindowsShare/MSDOS/C_DRIVE` as `C:` (this is where the actual Borland C++ 3.1
install — `C:\BC`, `bcc`, `tlink`, `tlib` — lives, *outside* this repo). Scripts prefer
`/Users/kevincarr/projects/DOSBox-Dev-Support/dosbox_helper.py` when present (override with
`DOSBOX_HELPER`/`DOSBOX_BIN`), falling back to `dosbox-x` directly.

Inside DOS, `build.bat` does `del *.exe & del *.obj & make -B -f zmgr.mak`. `ZMGR.MAK` is a Borland
`make` file with explicit per-EXE `tlink`/`tlib` rules (not a generic pattern build) — when adding a
new `.cpp`/object to one of the EXEs, both the `*_dependencies` list **and** the explicit `tlink @&&|`
linker response block for that EXE need updating, since `tlink` needs the object names listed
in-order. `build.cfg` (lowercase target generated from `BUILD.CFG`) is the bcc compiler-flags file
generated from `ZMGR.MAK`'s `build.cfg:` rule — distinct from the runtime `ZMMGR.CFG`.

`uptest.bat` / `downtest.bat` are quick manual scenario scripts (used from the DOSBox `C:` test
environment, not from this repo's root) that seed `c:\upload`/`c:\download` and run
`zmgr -f`/`zmgr -ff` to exercise forwarding by hand.

Built artifacts (`*.OBJ`, `*.EXE`, `dist/`) are gitignored but normally present in the working tree;
don't treat their presence/absence as meaningful when looking at `git status`.

## Runtime configuration (`ZMMGR.CFG`)

Every machine has its own `ZMMGR.CFG` (plain `KEY = VALUE` lines, see the file's own header comment
for the exact spacing convention it expects). Key fields: `COMPUTERNAME` (must be unique per machine;
`ZMGR.EXE run` prompts for it on first run if blank), `PORTS` (comma-separated COM ports, may be
blank and filled in later via Port Diagnostics), `DOWNLOADFOLDER`/`UPLOADFOLDER`/`KEEPFOLDER`,
`EXECPATH` (path to PDZM's `ZM.EXE`), `ZMOPTIONS` (shared PDZM flags), `ONCEEXEC`/`ONCEDOSCOMMAND`
(one-shot post-transfer hooks, erased after running), and `ZMGRSECRET` (hidden control commands —
`CONTINUE`, `QUIT`, `SKIPONCE`, `WAITnn`). `ZMMGR.BAK` is the auto-backup written before any
programmatic rewrite of `ZMMGR.CFG`.

## Core architecture (ZMFT's class hierarchy)

`ProcessManager` (`PROCMAN.CPP/H`) is the hub that `ZMFT.CPP` drives. It owns one instance each of:

- **`ConfigClass`** (`CONFIGCL`) — singleton; typed (`enum Category`) access to `ZMMGR.CFG` via the
  generic line-scanning helpers in `PARSER.H`.
- **`UploadManager`** / **`DownloadManager`** (`UPLDMGR`/`DLDMGR`) — handle the outbound and inbound
  halves of the transfer loop; delegate file movement to `ForwardManager` and actual PDZM invocation
  to `Transfer`.
- **`ForwardManager`** (`FWDMGR`) — singleton; scans/validates/copies/moves forward files between
  folders.
- **`Transfer`** (`TRANSFER`) — wraps the actual PDZM (`ZM.EXE`) send/receive invocations via `ExecPgm`.
- **`ExecPgm`** (`EXECPGM`) — `spawnl`/DOS-command wrapper used for PDZM, `ONCEEXEC`, `ONCEDOSCOMMAND`.
- **`SecretThings`** (`SECRETS`) — interprets `ZMGRSECRET` hidden commands.
- **`MemoryManager`** (`MEMMGR`) — manual heap-allocation tracking (`allocate_string_array` /
  `free_string_array` etc.); used anywhere file-list arrays are built, since this is 16-bit DOS C++
  with no RAII containers available.

`UTILS.H` is a header-only grab-bag of `static` helpers (file/dir listing, string trimming, `println`,
`monitormemory`, etc.) included by nearly every `.cpp` — check there before adding a new low-level
helper, it may already exist.

## Serial layer (three pieces, not one)

- **`DOSSER.C/H`** → `DOSSER.LIB` — vendored third-party "Serial Library 1.4" (Karl Stenerud),
  low-level buffered UART I/O (`serial_open/read/write/...`). Treat as a fixed external dependency,
  not project code to refactor.
- **`SERIAL.CPP/H`** (`SerialManager`) — hardware diagnostics and the "Who Is There" discovery
  protocol, implemented via **direct UART register I/O** (`inportb`/`outportb`) and BIOS data-area /
  INT 33h mouse probing — it does not go through `dosser.lib`. Linked only into `ZMDIAG.EXE`.
- **`SERBRDG.CPP`** (also implements `SerialManager`, the other half of the same class) —
  `passthrough()` / `passthrough_until_idle_after_traffic()`, the byte-bridging implementation built
  on `dosser.lib`. Linked into `ZMSER.EXE` (manual bridge) and `ZMFT.EXE` (Express auto-bridge), but
  **not** into `ZMDIAG.EXE`.

  `SerialManager`'s methods are split across `SERIAL.CPP` and `SERBRDG.CPP` by which EXE needs them —
  don't assume both diagnostics methods and passthrough methods are linkable from the same EXE; check
  `ZMGR.MAK`'s per-EXE `_dependencies` list before calling a `SerialManager` method from new code.

## Multi-hop routing & forwarding model

This is the non-obvious part of the system and spans `PROCMAN.CPP`, `UPLDMGR.CPP`, `DLDMGR.CPP`,
`SERIAL.CPP`, and `ZMROUTES.TXT`. See `notes.txt` for the full design history; summary:

- Outbound work is expressed as files dropped into `UPLOADFOLDER`:
  - `NAME.FIL` — single recipient, a PDZM send-list file (first line payload path, second line itself)
    for machine `NAME`.
  - `MULTI.FIL` — multicast: first line is a comma-separated recipient list, second line the payload
    path. Each hop strips its own name and forwards the remainder if names remain.
  - `NAME.XFL` — **Express** metadata (`SOURCE=`/`TARGET=`/`PAYLOAD=` keys), *not* a PDZM list file.
    Per `EXPRESS_PLAN.md`, an `.XFL` is never passed to PDZM via `@file` directly (that syntax means
    "read filenames from this list file") — it goes through a `ZMSEND.LST` wrapper instead.
- `ZMROUTES.TXT` holds `MACHINE=COMx` (or `MACHINE=LOCAL`) next-hop entries, looked up by
  `ProcessManager::get_route_port_for_machine`. ZMFT prefers the routed port; falls back to configured
  `PORTS` order with "NO ROUTE" if no entry matches, and avoids immediately bouncing a just-relayed
  file back out the port it arrived on.
- Routes are learned by the "Who Is There" discovery protocol (`SerialManager::run_who_is_there`,
  `ZMDIAG.EXE` option 2): machines exchange checksummed `IAM::<sender>::CS=xxxx` and
  `KNOW::<sender>::<machine,machine,...>::CS=xxxx` frames; a receiver records every advertised
  machine as reachable through the COM port the frame arrived on (split-horizon: a route is never
  re-advertised back out the port it was learned from), so routes propagate across a serial chain of
  more than two machines.
- Express (`.XFL`) relaying is different from normal store-and-forward: when the current machine is
  neither the `SOURCE` nor `TARGET` of a pending `.XFL`, `ProcessManager` opens a live
  `SerialManager::passthrough_until_idle_after_traffic` bridge between the inbound and outbound COM
  ports instead of storing the payload locally.

## Other files worth knowing about

- `EXPRESS_PLAN.md` — design notes for the Express (`.XFL`) feature, written before a deliberate
  revert back to the `NEXTGEN1` v2.2.0 STORE-forwarding baseline; read it before changing Express
  behavior, it lists explicit guardrails (don't change baseline STORE behavior, test STORE before
  `.XFL`, etc.).
- `notes.txt` — running dev log of routing/discovery design decisions and hardware test topology
  (`JEAN`/`MELBA`/`MIDWAY`/`FRANKY`); the closest thing to a design doc for the routing protocol.
- `FWDFILES.PY` / `MKFWDFILE.py` — standalone Tkinter GUI scratch scripts for listing/writing
  directory contents to a file; unrelated to the DOS build, not imported by anything.
- `ZMGR.PRJ` — legacy Borland IDE project file; `ZMGR.MAK` is the actual build entry point used by
  `build.bat`.
