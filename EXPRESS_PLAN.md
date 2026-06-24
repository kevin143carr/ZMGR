# Express Forwarding Plan

Saved before reverting the working tree back to the `NEXTGEN1` v2.2.0 baseline.

## Intent

Keep normal STORE forwarding exactly as the v2.2.0 baseline behaves. EXPRESS should be added later as an isolated feature, with no changes to the normal `.FIL` send/receive path unless a test proves the change is required.

## Proposed Express Model

- `FORWARDMODE = STORE` remains the default.
- EXPRESS is opt-in from the Forward File wizard for a single recipient only.
- Normal `.FIL` files remain PDZM send-list files and continue to use the existing path.
- `.XFL` files are Express metadata files, not PDZM send-list files.

Example `.XFL`:

```text
SOURCE=JEAN
TARGET=FRANKY
PAYLOAD=C:\PATH\FILE.ZIP
```

## PDZM Send Rule

Because `@file` means "read filenames from this list file", do not call:

```text
sz @C:\UPLOAD\FRANKY.XFL
```

Instead, if sending the `.XFL` control file through PDZM's list-file syntax is needed, create:

```text
C:\UPLOAD\ZMSEND.LST
```

containing:

```text
C:\UPLOAD\FRANKY.XFL
```

then call:

```text
sz @C:\UPLOAD\ZMSEND.LST
```

## Guardrails For Next Attempt

- First prove the checked-in STORE path still works unchanged.
- Add Express behind `FORWARDMODE = EXPRESS` only.
- Do not alter the baseline upload/download loop order.
- Do not let Express scratch files such as `ZMSEND.LST` or `XPASS.CTL` count as user upload work.
- Test direct STORE send first: JEAN sends `MELBA.FIL` and payload to MELBA; MELBA must print the normal "Forward Files found for ME!" message and move the payload to `C:\DOWNLOAD\KEEP`.
- Test relay STORE send second: JEAN to FRANKY through MELBA and MIDWAY.
- Only then test `.XFL` transmission.
