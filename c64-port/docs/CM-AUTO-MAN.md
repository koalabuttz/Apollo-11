# Translate Comanche AUTOMATIC_MANEUVERS (CM RCS DAP hold and jet timing)

Lane G. Milestone `cm-auto-man`.

## Scope

`Comanche055/AUTOMATIC_MANEUVERS.agc`, all of it: bank 21 `3250`-`3764`,
333 assembled words. That includes the MINTAU table, the WLH/SLOP, WL-H/SLP,
WLH, WLMH, WL and SLOPE2 constants, and the JETADDR 2CADR. Nothing is left
in the file.

`cm_auto_man(image, io)` runs one entry to its exit. `io` holds channels 31
and 32, which the file only reads (hold/free switches and the
translation-controller bits).

| Entry | Reached from |
|---|---|
| `3250` AHFNOROT | RCSATT T5PHASE2, `BZMF AHFNOROT` (no hand-controller rotation) |
| `3441` JETS | RCSATT SETWBODY loop, `TCF JETS` |
| `3757` T6PROG | RCSATT T6PROGM, `TCF T6PROG` |
| `3304` | STICKCHK return (FREECONT) |
| `3732`, `3734` | SMALLMP returns (JTIME) |

Admitted state: FB bank 21, EB E6, BB `042006`, any raw A/L/Q, erasable
contents and channels. RMANNDX/PMANNDX/YMANNDX must be 0..3 at `3304`
(STICKCHK leaves them so), and SPNDX 0..2 at `3732`/`3734` (the JLOOP
counter). `CM_AUTO_MAN_OK` leaves `Z` at RESUME, at REINIT or T6PROGM
(RCS-CSM_DIGITAL_AUTOPILOT), or at STICKCHK/SMALLMP with Q the TC's return.

### How it is translated

As in `cm-rcs-dap`: one qualified `agc_image_ops` primitive per original
word, under C labels named for the original addresses. READ and MSU are
written from the yaAGC definitions. The SIGNCK sequences (a CCS of the high
word whose +0/-0 slots fall to a CCS of the low word) are compiled as that
pair of CCS words.

- `TS T5TEMP` at `3443` (ADB + 4) overflows when ADB is within 3 of
  POSMAX. The skip runs JLOOP with SPNDX = +1.
- ADB + 4 cannot overflow negatively, so SPNDX never goes negative. The CCS
  SPNDX negative slots (ZEROCMDS) are translated but unreachable.
- The INDEX tables (MINTAU, OCT01400 and its neighbours, BIT13 and its
  neighbours) are native arrays equal to the rope.

## Source map

`evidence/cm-auto-man/cm-auto-man-source-map.json` (`tools/cm_auto_man_source.py`)
records every word with its source line, rope check and native label. It
also matches against the rope:

- the 19 fixed-constant reads, to the native literals;
- the double words, the 2CADR and the INDEX tables;
- the internal TC J6. and the frontier TCs, which must set Q to their
  returns.

Independent assembly comparison: `evidence/cm-auto-man/cm-auto-man-assembly-comparison.json`.

## Evidence

Hashes: `evidence/cm-auto-man/SHA256.json`; bulk files in release
`evidence-cm-auto-man`.

- Oracle: `deps/cm-flight-fixture` (original CM rope `6e3150ae...`). The
  runner stops at RESUME, REINIT, T6PROGM, STICKCHK or SMALLMP.
- Host and sanitizer results: `make test-cm-auto-man sanitize-cm-auto-man`.
  - 65536 cases (`tests/cm_auto_man_cases.c`).
  - Every phase-plane region and the TAU limits occur. That includes
    the rates on the WLH/WLMH boundaries for the SIGNCK low-word tests.
  - Every case reaches an exit, and the two sides agree on all of them.
  - Untaken native lines: the refusal default and ZEROCMDS (unreachable,
    above).
- C64 execution results: `make vice-cm-auto-man`, 128 frozen rows.
  `cm-auto-man-results.json`.

## Open

ABI registration of the entries follows in the Lane G routine-table step.
