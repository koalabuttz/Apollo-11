# INTER-BANK_COMMUNICATION against yaAGC (both missions)

Lane A, milestone `interbank-oracle`. Re-certification evidence for the
INTER-BANK items of #129: every genuine inline-CADR call site and every
registered entry of the file in both missions, run through `agc_transfer_run`
and compared with the original program in yaAGC.

## Cases

`tests/interbank_oracle_cases.c` builds 2,162 cases:

- 818 site cases (454 LM, 364 CM): one per genuine call-site operand word of
  the site tables (`TC` and its alias `TCR`), with Q at the operand and the
  banks of the calling word;
- 1,344 entry cases: 48 for each of the 14 entries of each mission
  (BANKCALL, SWCALL, SWRETURN, POSTJUMP, BANKJUMP, QJUMP, MAKECADR, SUPDACAL,
  IBNKCALL, ISWCALL, ISWRETRN, USPRCADR, SUPERSW and SWCALL -1), with a
  random image, A sign-extended (a 15-bit CADR of bank 020 and up would
  otherwise read as overflow), and Q, L and the banks in each routine's
  domain. SUPDACAL reads 32 rope words, every fourth in banks 040-043 (`tests/interbank_oracle_data.h`,
  written by the oracle with `--data` in a first pass).

A random Q inside the file's own code (04600-04777) moves up by 01000, since
the oracle stops at the first Z equal to the native continuation. Each entry
case draws its inputs from its index among the entry cases, so a site added
to a table leaves the entry cases unchanged. 2,161 cases reach a continuation
outside the file; one LM ISWRETRN case is a native refusal, printed as such
by both sides.

## Oracle and checks

`tools/interbank_oracle_reference.c` runs each case in yaAGC with the
original rope until Z and FB first equal the native continuation.
`tools/interbank_oracle_freeze.py` requires byte-identical output (the reach
flag, the interrupt inhibit, SUPERBNK and all 2048 erasable words) and
freezes one digest per case in `tests/interbank_oracle_vectors.h`.
`make test-interbank-oracle` and `make sanitize-interbank-oracle` pass 2,166
checks.

The C64 diagnostic runs every third case plus 6 cover rows, taking all 504
branch arcs the host corpus takes
(`evidence/interbank-oracle/interbank-oracle-c64-cover.json`);
`make vice-interbank-oracle` passes 729 checks in VICE.

## Source map

`tools/lane_a_source_map.py interbank-oracle --check` maps all 76 assembled
lines of each mission's file to the registered native routine
(`src/lm_interbank_routines.c`, `src/cm_interbank_routines.c`); the oracle
executed every instruction in both missions.
