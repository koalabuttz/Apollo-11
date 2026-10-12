# Auditor review: Luminary099/TIME_OF_FREE_FALL.agc

Reviewed 2026-10-12 by the port auditor against the certificate drafted by
milestone `lm-time-free-fall` (PR #36, 5b8c434). The first review (issue #28)
was not accepted; this re-review finds all three blockers fixed.

Verdict: **accepted**.

## Blockers from #28, now fixed

- `independent_assembly_comparison` cites the yaAGC run record
  (`lm-time-free-fall-source-stream.json`, 65536 states, decoded sha256
  `4ac7d7e5…`, 64825 returned and 711 SQRT aborts) and its snapshot archive.
  The differential rerun reproduces the archive byte for byte, and
  `oracle_sampling` lists the input classes.
- TCDANZIG, the BOVB TCDANZIG order, NEARONE and R300K are exported
  (`include/lm_time_free_fall.h:49-58`). NEARONE equals the listing at
  27,3770; the BOVB operand is the same word (57753) at all three callers,
  and both overflow outcomes run on the host and on the C64 rows.
- The C64 rows take every branch outcome the host stream takes (74 of 109,
  measured by gcov over the frozen C64 rows themselves). The other 35 are
  unreachable from admitted state: 28 failure guards behind certified table
  reads, compiled literal operands and `admitted()` scope checks, and 7 abort
  exits preceded by ABS, an untaken BMN, scalar-mode ABVAL or a VSQ that has
  not overflowed. The one reachable SQRT abort (ELLIPSE) is taken by the 711
  aborted oracle chains.

## Reproduced

- `make test-lm-time-free-fall sanitize-lm-time-free-fall`: 65536 oracle call
  chains equal native rows `4ac7d7e5…` (strict and ASan/UBSan); 266 words on
  253 lines map to 28 functions; 23600 body images equal yaAGC; C64 cover
  check passes.
- `make vice-lm-time-free-fall` (VICE x64sc 3.7.1, PAL): 4 PRGs, 63 checks,
  0 failures; PRGs and maps rebuild byte-identical to the evidence.
- No rope reads (bank-23 constants from the certified fixed-data table,
  bank-27 constants as literals); the only switch is over 25 native phases.

## Notes for future callers

MEASUREMENT_INCORPORATION:370, ORBITAL_INTEGRATION:94 and P20-P25:3216 use
BOVB TCDANZIG as the left order of a pair; the oracle sampled only TFF's own
right-hand uses. `bovb_tcdanzig` is position-independent, but a caller with
a pending right-hand order must continue with `lmi_danzig_right`.
TFFCONIC is qualified from an EDOP +0 entry no current caller produces; a
future caller must re-qualify it.
