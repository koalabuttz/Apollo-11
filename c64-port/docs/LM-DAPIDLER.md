# Translate Luminary DAPIDLER_PROGRAM as transfer-ABI routines

Lane F. Milestone `lm-dapidler`. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

## Scope

`Luminary099/DAPIDLER_PROGRAM.agc`, all of it: bank 16 `02000`-`02212` and bank
20 `02226`-`02446`, 284 assembled words. Nothing is left in the file.

`src/lm_dapidler.c` registers nine routines in `lm_dapidler_routines`. Each runs
from its entry to the next transfer, and every source instruction is one
`agc_image_ops` call (register effects checked against yaAGC by
`tools/agc_image_ops_differential.py`), named with its erasable address or the
source's fixed constant. MSU, DTCB and the channel orders (READ, RAND, WAND,
WOR, WRITE, and RXOR on channel 1) are small helpers with yaAGC's effect.

| Entry | Body |
|---|---|
| CHEKBITS `16,2000` | mode select (channel 31 bits 13-14) and IMU usability; JUMPDSP's DTCB to ALTDSPLY, SHUTDOWN or MOREIDLE |
| CHEKMORE `16,2016` | PGNCS control (channel 30 bit 10); MOREIDLE or RETURN to CHEKBITS's caller |
| DAPIDLER `16,2024` | the T5RUPT idler: BANKRUPT and QRUPT, NOVAC 1/ACCSET once per fresh start, else CHECKUP |
| CHECKUP `16,2041` (NOVAC's return) | `TC CHEKBITS` |
| `16,2042` (CHEKBITS's return) | the ACCSOKAY test; STARTDAP's `TC IBNKCALL` to ZATTEROR, or MOREIDLE |
| `16,2050` (IBNKCALL's return) | the DAP initialization (rates, offsets, timers, RCSFLAGS, T5ADR = PAXIS), SETTIME5, `TCF RESUME` |
| `16,2161` (QERRCALC's return) | `TC IBNKCALL` to CALCPERR |
| SHUTDOWN `16,2163` (CALCPERR's return) | jets off (channels 5 and 6), trim gimbal off (channel 12), T5ADR = DAPIDLER, SETTIME5, `TCF RESUME` |
| ALTDSPLY `20,2226` | the needle alternation, DSPLYTOT with OVERSUB2, the NEEDLER initialization passes on channel 12, the DAC loop and channel 14 bits 13-15; RETNMORE's DTCB to CHEKMORE |

MOREIDLE, SHUTDOWN, SETTIME5 and OVERSUB2 are compiled where they are reached.
`TC NOVAC`, `TC IBNKCALL` and `TCF RESUME` are transfers. The negative and -0
exits of CCS land where the original's do, even where no admitted input
reaches them.

Channels are the device record `lm_flight_io` (`include/lm_flight_io.h`):
channels 5, 6, 12 and 14 are outputs, and 30 and 31 are inputs.

Refused (`EXEC_OUT_OF_SCOPE`, image and io unchanged), with no original
producer:

- an entry outside its bank, or under an EB other than EBANK6 (`EBANK= AOSQ`
  and `AK`);
- a missing io record;
- a CHEKBITS or CHEKMORE return in Q that is not a TC into fixed memory.

## Verification

`tests/lm_dapidler_cases.c` builds 9,216 cases, 1,024 per entry. Each starts
from a random image with FB bank 16 (bank 20 for ALTDSPLY) and EB 6. Channels
5, 6, 12, 30 and 31 are random, and channel 14 holds bits 1-5 only. The CDU
angles and command counters are random, the Executive is canonical (all cores
busy in an eighth of the cases), and Q holds a caller return.

The trampoline runs each case with the native Executive and inter-bank
routines until the original program leaves the file for good:

- `TCF RESUME`;
- the 1202 alarm from NOVAC;
- the IBNKCALL to ZATTEROR, QERRCALC or CALCPERR;
- CHEKBITS's return to its caller.

ZATTEROR is already native (`lm-dap-interface`). The cases stop at it because
the native ISWRETRN does not yet admit the Q = 3 that ZATTEROR's `TC Q` leaves.
A quarter of the ALTDSPLY cases put a small attitude-error input in AK, so the
DAC loop's 0.1 scaling gives +0 and -0.

The case program carries this file's five compiled site operands (the NOVAC
2CADR and the three IBNKCALL CADRs). The host test checks each against
`lm_exec_site_table` and `lm_interbank_site_table`; the whole tables do not fit
the C64 diagnostic.

- `make lm-dapidler-differential` runs `tools/lm_dapidler_freeze.py`. It
  builds the yaAGC oracle `tools/lm_dapidler_reference.c`, runs every case in
  the unmodified rope to the same stop, and requires the native snapshot to be
  byte-identical in the outcome, channels 5, 6, 12 and 14, and all 2,048
  erasable words.
  - When ALTDSPLY sets the channel 14 CDU drive bits, yaAGC's coarse-alignment
    pulse output empties CDUXCMD-CDUZCMD into the IMU outside the program. The
    oracle therefore reports those three counters as the program left them at
    `CAF 13,14,15` (`20,2425`), just before `WOR CHAN14`.
  - The digests are frozen into `tests/lm_dapidler_vectors.h`.
  - `tools/lane_f_c64_rows.py` chooses the C64 rows
    (`tests/lm_dapidler_c64_vectors.h`, 192 cases). It runs every host case
    with its path counters on and every linked source built with clang
    `-fsanitize-coverage=edge,trace-pc-guard` (`tools/lane_f_path_trace.c`),
    and takes, greedily, cases until every entry, outcome, counted path and
    control-flow edge the host cases take is taken (16 cases), then spreads
    the rest with a stride of 97. The per-path and per-edge host and C64
    counts are in `evidence/lm-dapidler/lm-dapidler-c64-cover.json`.
- `make test-lm-dapidler` and `make sanitize-lm-dapidler` check every digest
  (strict and ASAN/UBSAN), require every case to reach its stop, require every
  routine and every reachable branch to be taken, and check the refusals.
- `make vice-lm-dapidler` runs the 192 C64 rows in VICE: every entry, every
  outcome, path and edge the host cases take. The VICE log records the
  PRG's SHA-256.

Branches no admitted input reaches, documented in the host test: the negative
and -0 exits of each CCS on a MASK or RAND result (in CHEKBITS, DAPIDLER,
CHEKMORE, ALTDSPLY and NEEDLES3), which is never negative, and those of
`CCS DINDX` (DINDX is 2, 1 or 0).

## Source map

Every assembled word has one native owner (`tools/lm_dapidler_source.py`,
using the shared checker `tools/lm_basic_source.py`, which now takes a file
that spans banks; `evidence/lm-dapidler/lm-dapidler-source-map.json`). After
the C preprocessor, the owner and the helpers it calls hold several things:

- each erasable address (for an INDEX order, the indexed base);
- the rope word of each fixed operand;
- each branch target, or the inline composition named in the tool;
- each channel;
- each constant, including the 2CADRs IDLERADR, PAXADIDL, DSPCADR and
  MORECADR.

The NOVAC 2CADR and the IBNKCALL CADRs are checked in the compiled site tables.

| Address | Native owner |
|---|---|
| 16,2000-2015, 16,2206-2212 | `chekbits_body` |
| 16,2016-2023 | `chekmore` |
| 16,2024-2036 | `dapidler_body` |
| 16,2037-2040 | `src/lm_exec_sites.inc` |
| 16,2041 | `checkup_body` |
| 16,2042-2046 | `accsokay` |
| 16,2047, 16,2160, 16,2162 | `src/lm_interbank_sites.inc` |
| 16,2050-2151, 16,2202, 16,2204-2205 | `startdap` |
| 16,2152-2154 | `settime5` |
| 16,2155-2156, 16,2163-2201, 16,2203 | `shutdown` |
| 16,2157 | `moreidle` |
| 16,2161 | `calcperr` |
| 20,2226-2434 | `altdsply` |
| 20,2435-2441 | `oversub2` |
| 20,2442-2446 | `retnmore` |

## Evidence

Hashes: `evidence/lm-dapidler/SHA256.json`; all four files are kept in the tree.
