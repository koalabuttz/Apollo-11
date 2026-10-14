# The earlier Pinball modules through the Lane C harness

Lane C. Milestone `lanec-pw`.

## What it adds

Pinball (`PINBALL_GAME_BUTTONS_AND_LIGHTS`) was translated over many earlier
milestones as separate modules (`src/dsky_*.c` and their Comanche twins),
each checked by its own runner against a source-map model. For the Pinball
full-file certificates and the transfer ABI (`lanec-pin-transfer`), every
one of them now also runs through the Lane C harness (`tools/lanec/`, see
`docs/LANEC-EV-FRONT.md`), so the Virtual AGC oracle compares its complete
results with the unchanged original program and records which original
words it covers.

- `tests/lanec_pw.h`, `tests/lanec_pw.c`: the shared pieces. Each `pw-*`
  suite draws its states from the module's own input generator, spread over
  the generator's whole domain, plus targeted cases of its own; the native
  module runs first and the oracle runs the original from the same entry
  until Z first reaches the point where the module stopped with the same Q
  (`lanec_pw_exit_stop`). The module claims nothing past it.
- `src/lanec_pw*_runs.c`: one `lanec_run` per module, the bridge from the
  harness's state and channels to the module's own interface.
- 93 suites in seven batches (`tools/mk/lanec-pw-a.mk` ... `-g.mk`,
  `tools/mk/lanec-pw.mk`): the CHARIN family, the decimal and octal
  displays, the scale-factor and load routines, the monitor, NVSUB and the
  verb/noun machinery, the lamp test and the operator-request scheduler,
  for both missions where both have the code.
  `pw-cm-scale` (batch f) runs Comanche DP1OUTSF, DP2OUTSF and DP3OUTSF
  (`cm_scale_output`), which DECDSP3 enters across banks.

The modules themselves are unchanged; a case outside a module's domain is
refused with the state unchanged (`diff.py --refusals`).

## Evidence

Hashes: `evidence/lanec-pw/SHA256.json`.

- Independent assembly comparison: `lanec-pw-diff.json`, the summary of
  every suite (cases, refused unchanged, native and oracle result hashes):
  108,063 cases, 108,043 identical and 20 refused unchanged, zero
  differing.
- Host and sanitizer results: `make test-lanec-pw sanitize-lanec-pw`
  (`lanec-pw-sanitize.txt`).
- C64 execution: `make vice-lanec-pw` builds one program per suite with
  the frozen rows `tools/lanec/cover.py pick` chose to take every host
  branch arc (`tests/lanec_pw_*_c64.h`) and runs them on a VICE PAL C64:
  85 programs, 917 checks, zero failures (`lanec-pw-vice.txt`). Seven
  suites' modules do not fit a program with the harness below $C000
  (`LANEC_PW_C64_TOO_BIG` in `tools/mk/lanec-pw.mk`); their modules run on
  the C64 in their own milestones' programs (`vice-legacy-*`). Every
  module also runs through the transfer ABI on the C64
  (`lanec-pin-transfer`).
