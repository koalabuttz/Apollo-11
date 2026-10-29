# DISPLAY_INTERFACE_ROUTINES full-file certificate (Luminary099, Comanche055)

Lane C. Milestone `lanec-di-cert`.

This drafts the `pending_complete_certificate` of both
`DISPLAY_INTERFACE_ROUTINES.agc` files in `docs/port-coverage.json` for the
auditor's review. The files stay `partial` until the auditor flips them.

## What it claims

Every assembled word of each file is owned by exactly one Lane C display
milestone, or by the constant table at the end of the file:

| Words (LM / CM) | Owner | Native module |
|---|---|---|
| 81 / 81 | `lanec-di-mark`: BLANKET..CLEARMRK +7 (fixed-fixed), MARKEND..COPYPACS +7 | `src/lanec_di_mark.c` |
| 143 / 140 | `lanec-di-dsp`: CHKPRIO..PRIODSP +2, EXDSPRET..SKIPADD +3 | `src/lanec_di_dsp.c` |
| 105 / 105 | `lanec-di-play`: MAKEPRIO..PRIOPLAY +1, WITCHONE..RELINTQ +1, LINUSCHR | `src/lanec_di_play.c` |
| 60 / 60 | `lanec-di-sleep`: GOSLEEPS..INITDSP +8 | `src/lanec_di_sleep.c` |
| 93 / 93 | `lanec-di-nvdsp`: PINBRNCH..V50PASTE +1 | `src/lanec_di_nvdsp.c` |
| 149 / 149 | `lanec-di-idle`: CLEANEND..TERMATE +1, PROCEED..ERASER +3 | `src/lanec_di_idle.c` |
| 39 / 39 | constant table, PERFMASK to the end of the file | compiled `symbols.txt` WORD rows |

Of the 39 constant words, 35 are read by the native code as compiled
constants. The other four (V06N07, BUSYMASK, OCT67777, OCT40420) are not
named by any source line of the program and stay in the rope as data.

## How it is checked

`tools/lanec/wholefile.py` (`make test-lanec-di-cert`) rebuilds the map from
the pinned listings and the unchanged original files. It fails unless:

- every word has exactly one owner;
- every instruction word was started by the unchanged original program in
  Virtual AGC under a case of the six display suites. This holds for all
  612 LM and 609 CM instruction words, measured with the oracle's
  `LANEC_TRACE`. The native modules reproduce those cases' complete
  results;
- each inline data word (an ADRES, OCT, 2CADR or GENADR after a call) sits
  inside an owner's interval as the argument of the call before it.

The map records, for every word, its original line number, address,
assembled word, owner, the C function that carries it (the function named
after its label, else the module's dispatch) and whether the oracle
executed it. Each entry word also names the transfer-ABI routine it is
registered as (`abi_routine`). The `aliases` rows resolve the file's EQUALS
names for code labels (GOAGIN, GOXDSP, GOXDSPR, GOXDSPF, GOXDSPFR, ENDEXT,
GOPLAY, LINUS) to those rows, since other files call through them.

## Transfer ABI

Every labelled entry and every return into the file from a callee is an
`exec_status` routine in `lanec_di_lm_routines` / `lanec_di_cm_routines`
(165 / 164), and 996 round trips through `agc_transfer_run` match the
original program; see `docs/LANEC-DI-TRANSFER.md` (milestone
`lanec-di-transfer`). `lanec_di_*` is the canonical translation of the file.

Comanche055 still needs Lane A's `cm_exec_sites` for its three NOVAC/FINDVAC
2CADR operands (MAKEPLAY, JAMTERM, PLAYJUM1); its round trips stop at the
Executive until then.

## Evidence

Hashes: `evidence/lanec-di-cert/SHA256.json`.

- Source-to-native map: `lanec-di-lm-source-map.json` and
  `lanec-di-cm-source-map.json`.
- Independent assembly comparison: `lanec-di-comparison.json`. It holds
  the `diff.py` summaries of the six module suites, the `di-transfer`,
  `di-transfer-c64` and `di-transfer-c64m` round trips and the `di-refuse`
  refusals: 22,364 cases, 21,678 compared with zero differing and 686
  refused with the state unchanged (122 di-idle and 114 di-nvdsp index
  cases, 28 di-transfer-c64 and 38 di-transfer-c64m chains that end where
  only an unregistered routine could go on, 384 di-refuse), with accepted cases
  counted per mission (`tools/lanec/cert_evidence.py`).
- Host and sanitizer results: `lanec-di-checks.log`. It has the strict
  and ASan/UBSan runs of all ten suites and the whole-file map run.
- C64 execution results: `lanec-di-c64-results.json`. It has fresh VICE
  PAL runs of the six frozen module subsets (each picked by
  `tools/lanec/cover.py` to take every host branch arc), the transfer round
  trips into the display-start and MAKEPLAY modules (`di-transfer-c64`, one
  program per mission) and into the mark, sleep, idle and nvdsp modules
  (`di-transfer-c64m`, one program per module group and mission): 397
  checks, 0 failures, with the PRG hashes.
- C64 branch outcomes: `lanec-di-c64-cover.json` (`cover.py record --pool`).
  Each suite's C64 rows take every branch arc of its host cases. The host
  suite `di-transfer` has no C64 program (with the Luminary099 Executive
  linked the round trip does not fit one): 705 of its 934 arcs are taken by
  the other suites' C64 rows and the other 229 are marked host-only with
  their reason. They are the Executive's job services (`executive.c`,
  `lm_exec_*`, `lm_basic_*`, `lm_admission_image.c`, `p66_native.c`), which
  run on the C64 in Lane A's Executive milestones (`vice-lm-exec-jobs`,
  `vice-lm-exec-services`, `vice-lm-exec-idle`), the host runner's merged
  LM site and table set-up, and `agc_classify`'s +0/-0 returns, reached on
  these chains only inside the Executive.

The supported domain and the external continuations are written into the
certificate itself. The continuations are the EXECUTIVE (with the
FINDVAC job JAMTERM), INTER-BANK_COMMUNICATION, SERVICE_ROUTINES,
ALARM_AND_ABORT and PINBALL services that the file calls. The only channel
writes are WITCHONE's WAND on channel 011; ROR SUPERBNK only reads channel 07.

Regenerate the evidence with:

    python3 tools/lanec/cert_evidence.py lanec-di build/di-cert-new di-dsp di-mark di-play \
      di-sleep di-idle di-nvdsp di-transfer di-transfer-c64 di-transfer-c64m \
      --refusals di-idle di-nvdsp di-transfer-c64 di-transfer-c64m \
      --refusals-only di-refuse --wholefile tools/lanec/wholefile.py \
      --c64 di-dsp=lanec-di-dsp di-mark=lanec-di-mark di-play=lanec-di-play \
      di-sleep=lanec-di-sleep di-idle=lanec-di-idle di-nvdsp=lanec-di-nvdsp \
      di-transfer-c64=lanec-di-transfer di-transfer-c64m=lanec-di-transfer-c64m

and the C64 branch-outcome record with:

    python3 tools/lanec/cover.py record evidence/lanec-di-cert/lanec-di-c64-cover.json \
      di-dsp di-mark di-play di-sleep di-idle di-nvdsp di-transfer-c64 di-transfer-c64m \
      di-transfer --pool --host-only=FILE:LINE=REASON ...

with one `--host-only` argument per line of the record's `host_only_lines`
(91 lines carrying the 229 arcs).
