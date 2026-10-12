# Translate Luminary Q_R-AXIS_RCS_AUTOPILOT as transfer-ABI routines

Lane F. Milestone `lm-q-r-axis`. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

## Scope

`Luminary099/Q_R-AXIS_RCS_AUTOPILOT.agc`, all of it: bank 17 `02072`-`03251`
and MAKERUPT at fixed-fixed `07754`-`07755`, 626 assembled words. Nothing is
left in the file.

`src/lm_q_r_axis.c` registers seventeen routines in `lm_q_r_axis_routines`.
Each runs from its entry until the original program leaves the file, and
every source instruction is one `agc_image_ops` call (register effects checked
against yaAGC by `tools/agc_image_ops_differential.py`), named with its
erasable address or the source's fixed constant. MSU, DTCB, EDRUPT and the
channel orders (READ, RAND, WOR) are small helpers with yaAGC's effect.

| Entry | Body |
|---|---|
| CALLQERR `17,2072` | from P-AXIS's DTCB: the Q,R errors (QERRCALC), the control system choice (GTSCADR's DTCB, TRYGTS, RCS), X-translation (SENSEGET), the minimum impulse, rate command and attitude hold modes (QRCONTRL, CHEKSTIK, RHCACTIV, ENTERUV), ATTSTEER and TJLAW, and the U,V jet selection (AFTERTJ, DOROTAT, DOSKIP, FEEDBACK, LOOPER, XTRANS), to the first call that leaves the file or CLOSEOUT's `TCF NOQRSM` |
| `17,2172` `17,2214` `17,2454` `17,2620` `17,2714` `17,2724` `17,2775` `17,3004` `17,3042` `17,3074` `17,3103` `17,3224` | the returns from those calls (ALARM in X-translation, IBNKCALL to ZATTEROR twice, IBNKCALL to TIMEGMBL, `TC TJETLAW`, IBNKCALL to SPSRCS, `TC WRITEP`, `TC WRITEP -1`, `TC WRITEP`, `TC WRITEU -1`, `TC WRITEV -1`, ALARM in SELCTSUB), continuing the same flow |
| CLOSEOUT `17,3236` | from GTS's DTCB: `TC MAKERUPT`, whose EDRUPT runs A as `TC ENDJASK`; ENDJASK restores the job's registers; `TCF NOQRSM` |
| QERRCALC `17,2626` | from DAPIDLER's IBNKCALL: the Q,R attitude errors; `TC Q` |
| JTLST `17,3114` | from P-AXIS's IBNKCALL: the T6 jet list; `TC Q` |
| ROT-TOUV `17,3146` | from AOSTASK's IBNKCALL: Q,R to U,V; `TC Q` |

The calls that leave the file end a routine with Z at the destination and
Q (or the IBNKCALL operand) as the original leaves it: `TC ALARM`,
`TC IBNKCALL`, `TC TJETLAW`, `TC WRITEP` indexed by INDEXES (`+4`, `+13`),
`TC WRITEP -1` indexed by INDEXES (which is `WRITEU -1` or `WRITEV -1`),
`TC WRITEU -1`, `TC WRITEV -1`, the DTCB to GTS (TRIM_GIMBAL_CONTROL_SYSTEM),
`TCF NOQRSM` and the subroutines' `TC Q`. QERRCALC, ROT-TOUV, SELCTSUB and
JTLST are entered by TC from inside the file as well and are compiled as C
calls that leave Q as the original does (3 after each `TC Q`). The jumps
written as `TC` (`TC +3`, `TC +2`, `TC +1`, `TC +4`) set Q as well.

The original's indirect transfers are followed:

- LOOPER's `TC RETJADR` runs the word in RETJADR. The file stores GENADR
  MINRTN, BACKHAND or TJLAW +3 there, each a TC with Q = RETJADR +1. Only
  this file stores RETJADR, so a loop return entry (which reaches LOOPER with
  AXISCTR 1) holding any other word is refused with the image unchanged.
  CALLQERR, 17,2172, 17,2214, 17,2454 and 17,2620 store RETJADR before
  LOOPER reads it.
- `INDEX AXISCTR; INDEX SKIPU; TCF +1` goes to the word after the TCF or the
  one four past it (SKIPU is 0 or 4).
- `INDEX A; CS 1/ANET2 +1` uses AXISDIFF (TJET_LAW's table, 0 or 16) plus 0 or
  1; `INDEX A; CS LIMITS` limits an overflowed sum to +-POSMAX (each sign
  has its own branch counter).
- MAKERUPT's EDRUPT, with no interrupt pending, saves its address +1 in
  ZRUPT and its word in BRUPT, sets the interrupt in progress and vectors to
  address 0, where A (ADRRUPT) runs as `TC ENDJASK` with Q = 1.
- JTLST's LASTCHG `TS T6FURTHA` overflow skip enters ROT-TOUV, which returns
  through the same Q.

The CCS exits that land past an `EXTEND` or on a data word run that word as
the original does: DOSKIP's CCS SENSETYP negative exit runs `READ LOSCALAR`'s
word as INHINT; the docked TJLAW's CCS -0 exit runs `CADR SPSRCS` as
`CS 17,3727`; X-translation's CCS POLYTEMP negative exit runs `OCT 02002` as
`TC 02002`. TS overflow skips and the negative and -0 exits of CCS land where
the original's do, even where no admitted input reaches them. INHINT and
RELINT set the interrupt-enable bit of the device record.

Channels 4 (LOSCALAR), 5, 13 and 31, the interrupt-enable bit and the
interrupt-in-progress bit are in the device record `lm_flight_io`
(`include/lm_flight_io.h`, which gains channel 4 and the interrupt-in-progress
bit). WOR CHAN13 keeps channel 13's readable bits (`043777`), as yaAGC does.

The two `TC ALARM` codes (`OCT 02002` in X-translation, `OCT 02004` in
SELCTSUB) are compiled site operands in `src/lm_q_r_axis_sites.inc`
(`lm_q_r_axis_sites`). The four IBNKCALL CADRs are in
`src/lm_interbank_sites.inc`. The words no code reaches are answered by
`lm_q_r_axis_data` for passive fixed-data reads: 1/10S, BIT8,9 and SCLNORM,
which have no reference in Luminary099, and ZEROTJ's `CA ZERO; TCF SETTIME`,
which nothing branches to.

Refused (`EXEC_OUT_OF_SCOPE`, image and io unchanged):

- an entry under an FB other than bank 17, or an EB other than EBANK6
  (`EBANK= CDUXD`);
- a missing io record;
- a QERRCALC, JTLST or ROT-TOUV return in Q that is not a TC into fixed
  memory;
- CALLQERR or a return entry with SKIPU or SKIPV other than 0 or 4. The
  double INDEX would jump anywhere; every producer (this file, DAPIDLER, GTS)
  stores 0 or 4;
- CALLQERR with ALLOWGTS -0 or channel 5 negative. CCS would enter
  `READ CHAN5` or `DCA GTSCADR` without its EXTEND; ALLOWGTS is stored as
  +0 or 1 (AOSTASK, DAPIDLER, TIMEGMBL), and channel 5 holds the eight jet bits;
- a loop return entry (17,2714, 17,2724, 17,2775, 17,3004, 17,3042,
  17,3074, 17,3103, 17,3224) with AXISCTR other than 0 or 1, NUMBERT outside
  0-4 or SENSETYP above 5, or with AXISCTR 1 and RETJADR not one of the three
  return addresses (except 17,3074 and 17,3103, which go to CLOSEOUT). They
  index INDEXES, AXISDIFF, TJU and TYPEPOLY; the file's own loop and TJETLAW
  store these values before the calls these entries return from;
- 17,2172, 17,2214 and 17,2454 with SENSETYP above 5 (no producer stores
  one).

CALLQERR, 17,2172 (the ALARM 02002 return), 17,2214 (CHKBIT10's ZATTEROR
return), 17,2454 (ENTERUV's ZATTEROR return) and 17,2620 (the TIMEGMBL
return) accept any AXISCTR and NUMBERT: they store both (MINRTN, MIMRET,
BACKHAND, TJLAW, TSNUMBRT) before reading them. P-AXIS's PURGENCY leaves
AXISCTR -1, GTS keeps SR in AXISCTR (SAVESR) and PJETSLEC leaves NUMBERT 5 or
6. SENSEGET's SELCTSUB, which would index TJU by AXISCTR on its alarm, never
alarms: TYPEPOLY 0 and 1 share no jet with ALLJETS 4 or 5.

## Verification

`tests/lm_q_r_axis_cases.c` builds 16,384 cases, 512 for each of 32 entry
slots: the seventeen entries, with CALLQERR in sixteen slots. Each case starts
from a random image with FB bank 17, EB 6, random channels 4, 13 (its readable
bits) and 31, an eight-bit channel 5 and a random interrupt-enable bit.

- In three quarters of the cases, every word above the counters has a random
  bit width, with +0 and -0 frequent.
- In one cohort in eight, three quarters of those words are +0 or -0.
- SKIPU and SKIPV are 0 or 4. RETJADR holds one of the file's three return
  addresses in fifteen cases of sixteen. At CALLQERR, 17,2172, 17,2214,
  17,2454 and 17,2620, AXISCTR is 0 or 1 in half the cases, -1 in a quarter,
  -0 or a random word in the rest, and NUMBERT is 0-6; at the other entries
  AXISCTR is 0 or 1 and NUMBERT 0-4. SENSETYP is 0-2 in three quarters. One cohort in 32 keeps all of these
  random, and half of that cohort has a random 15-bit channel 5.
- Q holds a caller return for the subroutine entries: SWRETURN, a fixed-fixed
  word, or a bank 17 word outside the file.

The trampoline runs each case with this file's routines until the original
program would leave the file.

- `make lm-q-r-axis-differential` runs `tools/lm_q_r_axis_freeze.py`.
  - It builds the yaAGC oracle `tools/lm_q_r_axis_reference.c`, which runs
    every case in the unmodified rope until Z leaves the file at an
    instruction boundary. Z in the central registers (a `TC Q` on its way to
    the word in Q, or EDRUPT's vector to A), at MAKERUPT, or at RETJADR
    holding one of the three return addresses is in the file.
  - It requires the native snapshot to be byte-identical for every case the
    native routines accept: 15,547 cases. The comparison covers the outcome,
    channel 13, the interrupt-enable and interrupt-in-progress bits and all
    2,048 erasable words.
  - It leaves out the 837 refused cases. Their digest is of the unchanged
    input image, which the host test checks.
  - The digests are frozen into `tests/lm_q_r_axis_vectors.h`.
  - `tools/lane_f_c64_rows.py` chooses the C64 rows
    (`tests/lm_q_r_axis_c64_vectors.h`, 128 cases). It runs every host case
    with branch counting (`tools/lane_f_path_trace.c`) and takes, greedily,
    cases until every entry slot, outcome, counted branch and basic block
    (`-fsanitize-coverage=trace-pc`) the host cases take is taken (54
    cases), then spreads the rest with a stride of 97. The
    per-branch host and C64 counts are in
    `evidence/lm-q-r-axis/lm-q-r-axis-c64-cover.json`.
  - The proof (`evidence/lm-q-r-axis/lm-q-r-axis-proof.json`) counts where
    the cases leave the file: every exit above is taken except `TC 02002`,
    which no admitted input reaches.
- `make test-lm-q-r-axis` and `make sanitize-lm-q-r-axis` check every digest
  (strict and ASAN/UBSAN). They require every case to leave the file or be
  refused with the image unchanged, and require every entry and every
  reachable branch to be taken. They also check the refusals, the IBNKCALL
  CADRs against the compiled interbank table, and `lm_q_r_axis_data`.
- `make vice-lm-q-r-axis` runs the 128 C64 rows in VICE: every entry, every
  outcome, branch and basic block the host cases take. The VICE log records the
  PRG's SHA-256.

Branches no admitted input reaches are documented in the host test:

- the negative and -0 exits of a CCS of a MASK result (CCS A after a MASK of
  a positive constant or a channel bit, CCS POLYTEMP), which is never
  negative;
- FAILOOP's CCS NUMBERT negative and -0 exits: NUMBERT counts down from
  THREE;
- LOOPER's CCS AXISCTR negative and -0 exits: AXISCTR is 0 or 1;
- LOOPER's `TC RETJADR` to another word: refused at the entry;
- DOSKIP's CCS TJU +0 and -0 exits: DOSKIP follows DOROTAT, whose CCS TJU
  sends +-0 to NOROTAT, and nothing between stores TJU.

## Source map

Every assembled word has one native owner. This is checked by
`tools/lm_q_r_axis_source.py`, using the shared checker
`tools/lm_basic_source.py` (which now reads a fixed-fixed listing address
without a bank); the map is `evidence/lm-q-r-axis/lm-q-r-axis-source-map.json`.

After the C preprocessor, the owner and the helpers it calls hold:

- each erasable address (for an INDEX order, the indexed base);
- the rope word of each fixed operand, including the INDEXES, AXISDIFF,
  LIMITS, ALLJETS and TYPEPOLY words read under INDEX;
- each branch target, or the inline composition named in the tool (the
  transfers among this file's own words);
- channels 4, 5, 13 and 31;
- each constant, including GTSCADR, ADRRUPT and the GENADR return addresses.

The ALARM codes and IBNKCALL CADRs are checked in their site tables.

| Address | Native owner |
|---|---|
| 17,2626-2660 | `qerrcalc_body` |
| 17,3114-3145 | `jtlst_body` |
| 17,3146-3172 | `rot_touv_body` |
| 17,3173-3222, 17,3225-3235 | `selctsub` |
| 17,2343, 17,2571-2572, 17,3111-3112 | `qr_data` |
| 17,2171, 17,3223 | `src/lm_q_r_axis_sites.inc` |
| 17,2213, 17,2453, 17,2617, 17,2723 | `src/lm_interbank_sites.inc` |
| the rest of 17,2072-3251, and 07754-07755 | `flow` |

## Evidence

Hashes: `evidence/lm-q-r-axis/SHA256.json`; all five files are kept in the
tree.
