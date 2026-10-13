# Certificate review: Lane D1 PR #208 (fixes for #188)

Reviewed 2026-10-13 by the certificate reviewer at `port/c64-native-c` 41fd045b,
against the certificates for milestones `sband-antenna`, `gimbal-lock` and
`cm-imu-calibration`. This is the second review; the rejections are in #188.

Verdict: **accepted** for all five files:

- Luminary099/S-BAND_ANTENNA_FOR_LM.agc
- Comanche055/S-BAND_ANTENNA_FOR_CM.agc
- Luminary099/GIMBAL_LOCK_AVOIDANCE.agc
- Comanche055/GIMBAL_LOCK_AVOIDANCE.agc
- Comanche055/IMU_CALIBRATION_AND_ALIGNMENT.agc

## What was fixed

- **Per-site S-band outcomes (#188 item 3):**
  - The CM BPL positive-argument rows now run on the C64.
  - The LM COVCNV `UNIT BOV` site at 42,3677 takes all 236 of its host arcs on the C64, overflow and DVOVF included.
  - The per-site edges come from `tools/irc_edges.c` (spec `site_edges`). 13,320 of 13,320 edges are taken on the C64.
- **cm-imu negative DIM (item 5) and the CHECKG1 spin (item 6):**
  - Joint states seed PIPINDEX, PIPAX and ZERONDX.
  - Both paths are taken on host and on C64 rows, and compared with yaAGC.
- **Gimbal lock:** the proof re-derives, with 636/636 edges.

## Reproduced

- Test, sanitize and every yaAGC differential case pass.
- VICE gives sband 224, gimbal 36 and cm-imu 507 checks, equal to `vice_targets`, with 0 failures.
- All 29 freshly built PRGs match the hashes in their logs.
- All three covers re-derive with nothing missing.
- An independent `gcc -O0 --coverage` run, per object with the LM and CM objects measured separately, finds no host outcome missing on the C64 rows.

## Notes for the lane (not blocking)

- At this tip `make sband-antenna-differential` repacks the same 224 rows (same order and digests) into 9 PRGs instead of the committed 10. Both groupings pass VICE. Refreeze at the next touch, and consider making the packing deterministic.
- A few operand-class residues remain inside shared helpers. Each is either invisible at the AGC level or not plausible in flight:
  - CM `:320` BZF on −0, where +0 takes the same AGC path;
  - CM `:429` CCS on a +0 word, where −0 takes the same path;
  - LM `:702` VSU and `:297` MXV, the sign of the overflow ±1.
- The cm-imu freeze takes longer than two hours. Its cases, stops, cover-check and VICE were reproduced; the freeze itself was not rerun to completion.
