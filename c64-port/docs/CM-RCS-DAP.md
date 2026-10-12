# Translate Comanche RCS-CSM_DIGITAL_AUTOPILOT (CM RCS DAP)

Lane G. Milestone `cm-rcs-dap`.

## Scope

`Comanche055/RCS-CSM_DIGITAL_AUTOPILOT.agc`, all of it: bank 21
`2105`-`3247`, 611 assembled words. That includes the RCSLOC and T6ADDR
2CADRs, the two AMBGUPDT 2CADRs after TC NOVAC, the IBNKCALL CADRs, and
the DELTATT, ONESEK, DELAY200, DACLIMIT, NO.T5VAR, =.24, RCSINIT(B),
T5WAIT60, OCT0xxxx, MANROT, MANTABLE, =+14MS, FREETAU and GAIN1/GAIN2
constants. Nothing is left in the file.

`cm_rcs_dap(image, io)` runs one entry to its exit. `io` holds channels 12
(RAND, WAND, WOR of the needles and error-counter bits), 13 (WOR of BIT15,
the T6 enable), 14 (WOR of 13,14,15, the IMU CDU drive) and 31 (read only:
DAP enable, attitude-hold and hand-controller bits).

| Entry | Reached from |
|---|---|
| `2105` REDORCS | restart (FRESH_START_AND_RESTART 2CADR) |
| `2122` RCSATT | T5RUPT via T5LOC (RCSLOC, RCSADDR, RCS2CADR) |
| `2236`, `2401` | returns from SMALLMP, DPADD (MYSUBS) |
| `2360`, `2616` | returns from NOVAC |
| `2550`, `2552` | returns from IBNKCALL S41.2, S40.14 |
| `2534` REINIT, `3067` T6PROGM | TCF from AUTOMATIC_MANEUVERS |
| `2420` NEEDLER, `2443` NEEDLE11, `2476` NEEDLES, `2632` ZEROJET, `3130` STICKCHK | IBNKCALL or TC; end in TC Q |

Admitted state: FB bank 21 (`042000`), EB E6, BB `042006`, any raw A/L,
erasable contents and channels, and the INDEX operands within the tables
the file indexes:

- ATTKALMN -1..11 and RATEINDX 0..6 at REDORCS/RCSATT. These index GAIN1,
  GAIN2 and MANTABLE. The file itself only stores 11 down to -1 in
  ATTKALMN, and RATEINDX holds 0, 2, 4 or 6.
- SPNDX 0..2 and DPNDX = 2 SPNDX at the SMALLMP and DPADD returns (the loop
  state of DRHOLOOP and DELOOP), plus ATTKALMN -1..11 at the SMALLMP return.
- For the five TC Q entries, Q a caller's return in `3250`-`3764` or
  `4000`-`7777`.

`CM_RCS_DAP_OK` leaves `Z` at RESUME, IBNKCALL, NOVAC, SMALLMP or DPADD
(with Q the TC's return), at JETS, T6PROG or AHFNOROT (AUTOMATIC_MANEUVERS),
or at the caller's return with `Q = 3`. Anything else is
`CM_RCS_DAP_OUT_OF_SCOPE`, image and io unchanged.

### How it is translated

As in `cm-tvc-daps`: one qualified `agc_image_ops` primitive per original
word, under C labels named for the original addresses. TC/TCR to a label in
the file sets Q and jumps; every TC Q goes through one dispatch on the
return address. TC to another file is a frontier. READ, RAND, WAND, WOR,
RXOR LCHAN (L as channel 1) and MSU are written from the yaAGC definitions.

- INDEX words add SPNDX/DPNDX (the file's own loop counters), ATTKALMN,
  STICKCHK's 0..3 manual-rate indices or RATEINDX to the next word. INDEX
  reads of fixed memory come from native tables equal to the rope: GAIN1-3
  through GAIN2+11, MANTABLE-1 through =+14MS, FREETAU, DACLIMIT±1.
- `TS T5TEMP` at `2512` overflows when a needle command passes the DAC
  limit. The skip runs `INDEX A; CAF DACLIMIT` with A = ±1.
- `TS TIME5` at `3151` overflows with a large T5TIME. The skip runs
  `CAF DELTATT2`.
- `TS T5TEMP` at `3130` overflows when a caller enters STICKCHK with an
  overflowed A. The skip runs `TS PMANNDX` with A = ±1.
- The other TS words cannot overflow: they store loads, constants, masks,
  MP or MSU results, or small indices.
- `CCS A` after a MASK sees +0 or a positive value, so only two slots occur.
  `CCS SPNDX` sees the loop counter.

## Source map

`evidence/cm-rcs-dap/cm-rcs-dap-source-map.json` (`tools/cm_rcs_dap_source.py`)
records every word with its source line, rope check and native label. It
also matches against the rope:

- the 87 fixed-constant reads, to the native literals;
- the 2CADRs and the four INDEX tables;
- the TC Q dispatch, to the listing's internal TC/TCR returns;
- the frontier TCs, to their returns.

Independent assembly comparison: `evidence/cm-rcs-dap/cm-rcs-dap-assembly-comparison.json`
(`tools/cm_flight_assembly_comparison.py`).

## Evidence

Hashes: `evidence/cm-rcs-dap/SHA256.json`; bulk files in release
`evidence-cm-rcs-dap`.

- Oracle: `deps/cm-flight-fixture` (original CM rope `6e3150ae...`).
  - The runner stops at the frontiers above or at the caller's return.
  - Channel 13 starts with its trap and device-only bits clear. Channel 14
    starts with the gyro, optics and IMU CDU drive bits (`01000`-`070000`)
    clear.
  - Once NEEDLES sets the CDU drive bits, yaAGC's coarse-align model
    empties CDUXCMD-CDUZCMD (`050`-`052`) on the next step. That is the
    simulator standing in for the CDU hardware, so the runner keeps those
    words as the program left them.
  - The scaler is held, so TIME5/TIME6 do not count.
- Host and sanitizer results: `make test-cm-rcs-dap sanitize-cm-rcs-dap`.
  - 65536 cases (`tests/cm_rcs_dap_cases.c`), covering every entry. A
    quarter of the RCSATT cases each are steered into T5PHASE2 (hand
    controller, manual rate, free drift) and into the rate filter.
  - Every case reaches an exit, and the two sides agree on all of them.
  - Native line coverage is complete apart from the refusal default.
  - Stream identity `cm-rcs-dap-source-stream.json`, deltas
    `cm-rcs-dap-reference.deltas.xz`, log `cm-rcs-dap-checks.log`.
- C64 execution results: `make vice-cm-rcs-dap`, 128 frozen rows.
  `cm-rcs-dap-results.json`.

## Open

ABI registration of the entries follows in the Lane G routine-table step,
after Lane A's ISWRETRN fix. AUTOMATIC_MANEUVERS (JETS, T6PROG, AHFNOROT)
is the next Lane G file in this bank.
