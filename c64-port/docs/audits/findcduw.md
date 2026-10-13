# Certificate review: Luminary099/FINDCDUW--GUIDAP_INTERFACE.agc

Prepared 2026-10-13 by Lane A2 for the certificate reviewer, against the certificate
drafted by milestone `findcduw` (branch a2-fcw2, merged with `port/c64-native-c`
fb3fd46f). It records what the lane checked and how to reproduce it. It is not an
independent verdict: the reviewer decides acceptance.

Verdict: **pending review**.

## What was checked

- Source paths. All 441 words of bank 30 (30,3104-3774) compile into the 75
  routines of `src/lm_findcduw.c`. `findcduw-source-map.json` names the routine
  for each word and lists no untranslated word.
  - The three labels reached from other files are registered entries at their
    original addresses: INITCDUW 30,3104, FINDCDUW -2 30,3111 and SPARCSIN -1
    30,3672.
  - Every word that can follow a DANZIG is a CHANG2 resume routine (`resume_keys`).
- Outcomes per stop in the 19,200 host cases. All of them compare equal to yaAGC
  in Z, all 2,048 erasable words and the interrupt-enable state.

  | Stop | Meaning | host cases |
  |---|---|---|
  | NEWOPS 06070, LOC outside the file | interpretive return to the caller (RVQ, the callers' CALLs) | 9,400 |
  | IBNKCALL 04674 | NOATTCNT +2's call to STOPRATE (no attitude control without the alarm) | 4,544 |
  | CHANG2 05126 | DANZIG with NEWJOB positive | 2,708 |
  | ALARM 05567 | NOATTCNT and ALARMMGA | 2,189 |
  | Q (TC Q, the caller's word) | SPARCSIN -1 entered from basic | 256 |
  | POODOO1 05726 | SQRT of a negative MPAC at 30,3542 | 103 |

- Native pieces. RTB NORMUNIT (`src/lm_findcduw_rtb.c`) is `rtb_vector`'s NORMUNIT
  front followed by the three `rtb_unit_phase` phases. An overflow phase ends at
  DANZIG as UNIT's DVOVF does, and UNIT's alarm 1302 (a zero vector) is refused.
  RTB QUICTRIG is `lm_powered_quick`, then its return, then `lmi_swreturn`. Neither
  piece changes a shared library.
- Channels. PAUTNO's RAND CHAN30 and RAND CHAN31 read the inputs latched for the
  run through the base's `channel_inputs`. Each case draws them from its seed, and
  the oracle sets yaAGC's input channels to the same values. The PNGCS control bit
  (channel 30 bit 10) and the auto bit (channel 31 bit 14) are both set and clear
  across the cases.
- Refusals. `src/lm_findcduw.c` has 251 `EXEC_OUT_OF_SCOPE` refusal sites (the count `cert_precheck.py` reports; the ABI wrapper's null-pointer check is not counted). The 19,200
  cases reach none of them (the differential prints no "refused at" line):
  - 75 are entry guards (Z or LOC not at the routine);
  - 145 are indexed-instruction guards (the index changes the opcode or reaches a
    central register). TEM2, TEM4, NDXCDUW and SPARCSIN's A take only the values
    their callers leave;
  - 21 are TS overflow skips whose skipped-to word the compiled path does not
    continue (TS CDUSPOTZ, UNFVZ/2, EBANK and MPAC). No case overflows there;
  - 3 are RTB results outside the domain (the zero-vector alarm);
  - the rest are the defensive TC Q/TC K checks, the BOV/BOVB default arms and the
    unextended -0 arm after each RAND (`refuse_unextended`). A positive bit
    ANDed with a channel cannot give -0.
- C64 index carry. On the C64 `unsigned` is 16 bits, so INDEX's end-around carry
  was lost there: SPARCSIN's `INDEX A` with A = -1 refused on VICE and passed on
  the host. `index_carry16` fixes it for this spec only. Other specs keep the old
  code, so their generated output is unchanged.

## Reproduced

- `interp_compile.py findcduw --check` passes.
- `make findcduw-differential` (256 cases per routine, `--freeze`): 19,200 cases
  equal yaAGC across 75 routines. It froze 1,200 vectors, 146 C64 rows and 14
  groups.
- `make test-findcduw sanitize-findcduw`: 1,200 checks, 0 failures each.
- `make vice-findcduw`: 14 PRGs, 146 checks, 0 failures (12, 7, 2, 1, 1, 9, 6, 9,
  18, 13, 1, 3, 31, 33). The log names each PRG's sha256.
- C64 cover: gcov over all 21 linked sources gives 1,974 host arcs and 1,974 C64
  row arcs, with 0 host arcs missing on C64.

## Notes for the reviewer

- The PRGs build with `-fno-inline -fstatic-stack -mlto-zp=110 -DIRC_INPUTS`, and
  the image sits in the RAM under KERNAL at $e000. Without the static stack the
  groups did not fit.
- MODE is pinned to -1 where the RTBs need a vector. The resume keys otherwise
  enter PAUTNO's RTB NORMUNIT with a scalar MODE, which cannot happen in flight:
  the preceding VXV/UNIT leave a vector.
