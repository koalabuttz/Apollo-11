# PINBALL_GAME_BUTTONS_AND_LIGHTS full-file certificates

Lane C. Milestone `lanec-pin-cert`.

This documents the accepted `complete_native_coverage` certificates of
`Luminary099/PINBALL_GAME_BUTTONS_AND_LIGHTS.agc` and of
`Comanche055/PINBALL_GAME_BUTTONS_AND_LIGHTS.agc` in
`docs/port-coverage.json`. The reviewer accepted them
(`docs/audits/pinball-674.md`, #674 merged as b56219b3) and both files are
`complete` (flip #844, merged as d38c7704).

## Transfer ABI

Every labelled entry, every return into the file from a callee outside it,
every BANKCALL return point and every word the file reaches through
POSTJUMP or BANKJUMP (GOVNUPDT, UPDAT1 +2, DSPOCTIN, OCTBACK; registered by
`lanec-pin-gaps`) is a routine of the native control-transfer
ABI (`docs/NATIVE_TRANSFER_ABI.md`): 287 Luminary099 routines in
`lanec_pin_lm_routines` and 286 Comanche055 routines in
`lanec_pin_cm_routines` (`src/lanec_pin_routines.inc`), and 13 per mission
at CHARIN and its key labels in the keypress tables
(`docs/LANEC-PIN-KEYS.md`), generated and
checked against the listings by `tools/lanec/routines.py pin`. The
internal subroutines it lists are reached only by a TC inside the file and
are compiled into their callers. Each routine runs the earlier Pinball
modules that claim its point, with the head families
(`docs/LANEC-PIN-HEADS.md`) in place of the composite modules at the points
they claim. See `docs/LANEC-PIN-TRANSFER.md` (milestone
`lanec-pin-transfer`): 2,332 round trips from the genuine external callers
and from every routine's own entry give 1,265 complete states identical to
the original and 1,067 chains refused unchanged; `pin-refuse` refuses all
1,530 entry states with a broken register contract unchanged; and on the C64
one program per module and mission runs the round trips in VICE, and the
Pinball flight modules run the rest from the REU store.

## What it claims

Every assembled word of each file has exactly one owner:

| Words (LM / CM) | Owner | Native code |
|---|---|---|
| 120 / 121 | `lanec-pin-gaps`: DSP2DEC/END2DEC, NVSUBB, LOADLV1, DSPDCWD1, GOVNUPDT, DSPOCTIN, the octal display's WDAGAIN/OCTBACK loop and the other words no earlier module carried | `src/lanec_pin_gaps.c` |
| 101 / 84 | `lanec-pin-heads`: the head families, each run by its `pin-head-*` suite | `src/lanec_pin_head_*.c` |
| 49 / 46 | `lanec-pin-keys`: CHARIN and the key labels | `src/lanec_pin_keys*.c` |
| 1,898 / 1,915 | `lanec-pw`: the earlier Pinball modules, each run by its `pw-*` suite | `src/dsky_*.c`, `src/cm_*.c` through `src/lanec_pw*_runs.c` |
| 210 / 213 | constants | compiled `symbols.txt` WORD rows or literals |

Both module counts include each call's inline arguments (65 words per
mission: CADR, ADRES and the like after a TC), which the native call
compiles in. The constant words are 199 / 202 data words and 11 instruction
words that other code reads as data and that never execute: INRELTAB + 15,
MIXAD, the three DISPLACE words, NVSBENDL, ENDINST, TCNOVAC, TCWAIT,
TCTSKOVR and TCFINDVC (`tools/lanec/wholefile_pin.py` gives each one's
readers).

Two Luminary099 and three Comanche055 instruction words are unreachable:

- LEGALTST `TC CCSHOLE`: `AD NEG2`, `CCS A` reaches it only for a sum of
  +0, which the one's-complement adder never gives when one operand (NEG2)
  is nonzero.
- Comanche055 NEGOPT `TC CCSHOLE`: the same, after `AD 20BIAS`.
- ALL3DEC's first `TC FORCEV25`: `CS OCT34BAR`, `MASK DECBRNCH` leaves at
  most 34 octal, and `AD OCT34BAR` (-34) gives a negative sum or -0, so
  `CCS A` never takes the positive branch to it.

## Flight domain

The inputs a flight brings to Pinball are the verb and noun codes and the
addresses the noun tables give. Both ropes' tables (NNADTAB, IDADDTAB,
NNTYPTAB in bank 42, read from `evidence/fixed-data-*-original.rope.bin`)
give these addresses:

| | LM | CM |
|---|---|---|
| nouns with an address | 79 | 86 |
| address words (normal nouns' first word, mixed nouns' components) | 166 | 184 |
| lowest | 013 (N65, SAMPTIME) | 013 (N65) |
| other addresses below 0100 | 024 (N36, TIME2), 032 (N20, CDUX), 035-037 (N21, N72) | 024 (N36), 032 (N20), 035-037 (N21, N91) |
| words in 016-023 | none | none |
| past 01777 (as NOUNADD) | N13 (TCDH, 01776, three components): 02000-02001 | none |

So every noun reads unswitched erasable from 013 up or one switched bank
through NOUNADD 01400-01777, and only the LM's noun 13 runs on into fixed
memory: FB 31 under superbank 4 is bank 41, whose first words are
`TC NVSUB1` and `TC LOADLV1` (03544, 02775) in both missions. DECDSP, the
octal verbs and the loads take all of it natively. A host sweep through the
flight store (`flight-host-run-lm`/`-cm`, scaled time) of V01-V06, V11-V17
and V21-V25 on every noun of both missions, V32-V37, V82, V83 and V16 N36
followed by V32 runs every Pinball point natively with no refusal but the
cases below.

The guards that stay are argued here:

- **Operator-entered machine addresses (N01-N03, and N15 incrementing
  NOUNADD) in 0-07.** These are A, L, Q, EB, FB, Z, BB and the zero
  register. A display of one shows Pinball's own working registers at the
  read, which is not a flight quantity, and no noun names them. N15 from the
  NOUNADD a fresh start leaves (01401, EB 0) reads 1-3. Refused.
- **016-023 for the octal verbs, 017-023 for the decimal displays and the
  loads.** BANKRUPT and BRUPT (016-017) are rewritten by every interrupt;
  CYR, SR, CYL and EDOP (020-023) are edited on every access, so a read
  rewrites them. No noun names them. DECDSP models the editing on its read;
  the others refuse.
- **Logical addresses past 01777** other than N13's two words: fixed memory,
  which no noun reaches and a load cannot write. Refused.
- **2INTOUT negative values other than -0.** The nouns it shows are TREDES
  (clamped to 0-99), LOOKANGL (an arcsine in degrees) and on the CM VHFCNT
  and TRKMKCNT (counters): never below zero in flight. They can be -0: LM
  P65's display hands 2INTOUT MPAC = 077777 (lm-p65-autoland-nassp), and
  DSPDECVN shows it as 00 (the 0244 multiply keeps -0, and INDEX -0 picks
  digit 0). 2INTOUT and DSPDECVN take -0 natively (`pw-two-integer`,
  `pw-two-digit`). Values past 99 are shown modulo 100 natively, as DSPDECVN
  does.
- **ALMCYCLE with superbank 0-3.** ALMCYCLE (fixed-fixed 04145 / 04161) is
  called only from Pinball code in banks 40-43, which runs only under
  superbank 4 or higher.
- **A routine that changes input channel 030-033 through the trampoline.**
  The device record carries channels 030-033 as inputs the devices own (the
  switches, the IMU and radar status). A CHANLOAD onto them (V21 onto a
  channel) is off-nominal, and no flight procedure does it. The trampoline
  refuses such a result instead of storing it (src/lanec_pin_trampoline.c
  `swap_inputs`/`unrepresented`), the same way it refuses a write to a
  channel the record does not carry. No case takes this arm.

Two native files are in no try-list: `src/dsky_decimal_consumer.c` and
`src/dsky_operator_schedule.c`. Their `pw-*` suites still compare them with
the original, but no Pinball point's try-list runs them, and the
certificate claims nothing for them.

## How it is checked

`tools/lanec/wholefile_pin.py` (`make test-lanec-pin-cert`) rebuilds both
maps from the pinned listings and the unchanged original files. It runs
`pin-gaps`, the ten `pin-head-*` suites, the keypress modules and the
`pw-*` suites whose module still runs at some Pinball point (76; the
composites the heads replace or `dropped` removes stay host tests) through
the oracle with `LANEC_TRACE` and fails unless:

- every word has exactly one owner;
- every instruction word but the constants and the unreachable ones was
  started by the unchanged original program in Virtual AGC under one of the
  suites' cases (2,101 LM and 2,098 CM words), and the native modules
  reproduce those cases' complete results;
- each data word after a call is that call's argument, and every other data
  word is a constant;
- the constant and unreachable instruction words match the listing.

The earlier modules overlap, so a word executed under several suites is
given to the first that runs it (`pin-gaps`, then the heads, the keypress
modules and the `pw-*` suites in name order); every suite that runs it
reproduces the original there.

## Evidence

Hashes: `evidence/lanec-pin-cert/SHA256.json`.

- Source-to-native maps: `lanec-pin-lm-source-map.json` and
  `lanec-pin-cm-source-map.json`. They give every word's original line,
  address, assembled word, owner, native module, transfer-ABI routine and
  how the oracle covered it.
- Independent assembly comparison: `lanec-pin-comparison.json`: 91
  suites (`pin-gaps`, the ten `pin-head-*` suites, the 76 `pw-*` suites,
  `pin-transfer`, `pin-transfer-c64`, `pin-decdsp3` and `pin-refuse`),
  150,956 cases, zero differing and 4,907 refused with the image and I/O
  unchanged.
- Host and sanitizer results: `lanec-pin-checks.log`. It has the strict and
  ASan/UBSan runs of every suite and the whole-file map run.
- C64 execution results: `lanec-pin-c64-results.json`, with every PRG's
  hash: `vice-lanec-pin-gaps` (63 checks), `vice-lanec-pw-1` to `-5` (86
  programs, 947 checks) and `vice-lanec-pin-transfer-1` to `-4` (85
  programs, 216 checks), plus three runs through the Pinball flight modules
  loaded from the REU store (`docs/MODULE-TEST.md`), one routine per case:
  `vice-lanec-pin-keys-1` to `-4` (every `pin-keys` and `pin-key-labels`
  case, 1,408 per mission each, one part per suite and mission), `vice-lanec-pin-chains` (the frozen C64 rows of
  `pin-transfer`, `pin-decdsp3` and `pin-refuse`: 240 LM and 151 CM, 11 CM,
  239 LM and 111 CM) and `vice-lanec-pin-flight-1` and `-2` (the frozen rows of the ten
  `pin-head-*` suites and `pw-fixed-display`, LM then CM). All ran on a
  VICE PAL C64 with no failures; the store runs name the store's sha256.
- C64 branch outcomes: `lanec-pin-c64-cover.json` (`tools/lanec/cover.py
  record --pool`). For each suite it lists the host cases' branch arcs over
  every linked source and the frozen C64 rows. Every host arc is taken by
  some C64 program's rows, with these groups listed apart:
  - the 163 `pin-transfer-c64` rows of the 40 programs that do not fit
    below $C000 (`LANEC_PIN_C64_TOO_BIG`, `pw_load_store_cm` among them) do
    not count (`c64_rows_not_run`); the DI round trips
    (`di-transfer-c64`), the keypress pair (`pin-keys`, `pin-key-labels`)
    and the flight-module chains (`pin-transfer-flight`,
    `pin-decdsp3-flight`, `pin-refuse-flight`) are pooled in, since they run
    the same routines in the same build;
  - `pin-transfer`, `pin-decdsp3` and `pin-refuse` have no C64 rows of their
    own (`pooled_only`): their `-flight` twins run the same cases the way the
    C64 module test does, and their arcs count only through the pool;
  - 117 arcs on 43 lines that no C64 row can take (`host_only_lines`,
    `host_branches_not_on_c64`, each line with its reason): the transfer
    harness itself (the pin-head scan cache in `tests/lanec_pin_head.c`, the
    range, stop and accept tests in `tests/lanec_pin_transfer_cases.c`), and
    other files' Executive, waitlist, interpreter-change and DI MARK
    routines that only the pooled host suites reach, where a host chain runs
    on past the call the C64 flight run stops at. Those routines are
    certified with their own files.
- The full DSKY sweep (`evidence/dsky-sweep/dsky-sweep.json`,
  `tools/dsky_sweep_vice.py --cases all`, acceptance item 6): every key
  sequence of `tools/dsky_sweep_cases.py` on both missions, PRO cases
  included, ends in the host state on the C64.

The supported domain and the external continuations are written into the
certificates themselves.

Regenerate the evidence with (the suites of `tools/lanec/wholefile_pin.py`
but the keypress pair, whose evidence is `vice-lanec-pin-keys-1` to `-4`):

    S=$(python3 -c "import sys; sys.path.insert(0, 'tools/lanec'); import wholefile_pin as w; \
      print(' '.join(s[0] for s in w.SUITES if not s[0].startswith('pin-key')))")
    python3 tools/lanec/cert_evidence.py lanec-pin evidence/lanec-pin-cert $S pin-transfer pin-transfer-c64 \
      pin-decdsp3 --refusals $S pin-transfer pin-transfer-c64 pin-decdsp3 --refusals-only pin-refuse \
      --wholefile tools/lanec/wholefile_pin.py \
      --c64 pin-gaps=lanec-pin-gaps $(for n in 1 2 3 4 5; do echo pw=lanec-pw-$n; done) \
      $(for n in 1 2 3 4; do echo pin-transfer-c64=lanec-pin-transfer-$n; done)
    make flight-store
    make lanec-pin-keys-vice; make vice-lanec-pin-chains; make lanec-pin-flight-vice
    python3 tools/lanec/cover.py record evidence/lanec-pin-cert/lanec-pin-c64-cover.json $S pin-keys pin-key-labels \
      pin-transfer-c64 di-transfer-c64 pin-transfer-flight pin-decdsp3-flight pin-refuse-flight \
      pin-transfer pin-decdsp3 pin-refuse --pool \
      --pooled-only pin-transfer=pin-transfer-flight --pooled-only pin-decdsp3=pin-decdsp3-flight \
      --pooled-only pin-refuse=pin-refuse-flight \
      --not-run pin-transfer-c64=<the rows of LANEC_PIN_C64_TOO_BIG's programs> \
      --host-only 'FILE:LINE=<reason>' (each line of host_only_lines, with its reason)
    python3 tools/dsky_sweep_vice.py --cases all --evidence evidence/dsky-sweep

The three store runs use the built store and run one after another. The
cover record traces every host case of every suite (several hours; it keeps
each suite's arcs in `build/lanec-cover` and resumes from them), and the
full sweep takes about two hours a mission on four cores. The sweep for this
certificate ran once per mission (`--missions lm`, then `--missions cm`,
each with `--known tools/dsky_sweep_known.json --jobs 4`), and the two
reports were joined into one `evidence/dsky-sweep/dsky-sweep.json` (both
missions, mode `all`, results and stores side by side)), with its tree summary
`evidence/dsky-sweep/dsky-sweep-summary.json` (docs/DSKY-SWEEP.md).
