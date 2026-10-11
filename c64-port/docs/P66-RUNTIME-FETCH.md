# P66 fixed throttle EXIT fetch after the job poll

`p66_runtime_throttle_fetch` translates the fixed source lead-in after the
last completed DANZIG NEWJOB test. It accepts actual PC06067/LOC03507 with
CYR020006, or NEWOPS06070/LOC03510 with any ordinary CYR word. It performs
the source INCR LOC where applicable and the original fixed EXIT fetch,
clearing CYR before the source EXIT/DXCH/BANKCALL bridge. It ends at the
actual THROTTLE+3 entrance02221 and preserves ordinary NEWJOB without polling.

A compiled dispatcher must distinguish its resumed metadata from the actual
source lead-in. This adapter gives that distinction an independently checked
boundary. The header states the supported source state; unsupported inputs
leave the canonical image and explicit SUPER/IRQ unchanged. The caller owns
exclusion. Existing bridge APIs and published evidence remain unchanged.

`tools/runtime_fetch_differential.py` compares 8,192 complete original
Luminary099 2,050-word states, covering both entrances, raw caller registers,
pending signed/zero job words and ordinary CYR values. Snapshot SHA256:
`5e221fa7faa96b304b5654a513a09c839137949a16840cf37c680c7d449f4de4`.
Strict tests cover 64 frozen complete images and 26 invalid boundaries; all
source cases agree under ASAN/UBSAN. C64 PAL VICE passes all64 frozen images:
mailbox `a500004000`, PRG5,201 bytes, no extra zero page, measured hardware
stack14 bytes (plus14 ROM bytes) and software stack13 bytes.

Standalone recipes are in `tools/runtime_fetch.mk`. Native code executes a
fixed translated source path and contains no AGC instruction decoder.
