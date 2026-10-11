# Comanche relay-writing source qualification

`cm_pinball_write` implements actual Comanche055 DSPIN40,3227,
11DSPIN40,3311 and 2BLANK40,2502 through the original caller. It shares the
fixed relay store arithmetic with the existing native Pinball kernel only
after independent CM source execution verifies every physical word. The wrapper
checks the actual CM source entry, uses one canonical image, and gathers only
transient SUPER/IRQ metadata. Source hardware normalization of raw L is retained.
No display or Executive callback is invoked: relay words and NOUT are actual
source output for the device consumer to drain.

The domain is bank40, SUPER100, retained EB0..7, ordinary caller Q. DSPIN
COUNT0..21 optionallyBIT14 and CODE0..31, sign entry A0..10/CODE0 or BIT11, and
blank DSPCOUNT0..21 are supported. Original DSPIN CCSHOLE for clean+0 relay words
remains explicitly outside scope. 2BLANK accepts clean+0 and -0, skipping NOUT
for both as the original BZMF does. Ordinary NOUT edge values include037777,
where original INCR stores overflow-corrected +0. Unsupported input does not
mutate source words or I/O metadata.

8192 original CM checkpoints compare all2048 physical words plus SUPER/IRQ,
covering all entries/retained banks, digit masks/sign relays, dirty/clean/negative
zero relay words, every NOUT arithmetic corner and raw register normalization.
The private fixture retains unchanged original Pinball/T4 source bytes and
records only indentation normalization required by modern yaYUL. Its entire rope
must equal the independently assembled pinned CM rope; no source instructions
are changed. The independent oracle links no native source bodies.
SHA256 of matching native/original full streams:
`5a7eb68f798fe4e8c5269492b065de94a4bc63e8bde94d7cc6641430e95fe5dd`.
Full ASan/UBSan output equals the original stream byte for byte.

Frozen64 complete2050-field states pass strict host, sanitizer and native C64.
The frozen corpus explicitly retains clean+0 blank words and NOUT overflow
regressions. VICE PAL mailbox `a500004002`; PRG9799 bytes (loaded9797);
hardware/software stack10/14 bytes. PRG/section bounds below C000 and zero extra
compiler zero-page storage are checked. Evidence uses `build/cm-pinball-*` and
`build/cm_pinball-*`; source-byte hashes and indentation counts are recorded in
`build/cm-pinball-source-audit.json`, and full streams are retained compressed.
`tools/cm_pinball.mk` exposes portable host-only all/test/sanitize and explicit
c64-cm-pinball/differential-cm-pinball targets.

These source bodies implement relay writes and blanking. CM keyboard command
bodies, decimal formatter/control qualification and actual queued display/device
consumption remain separate application requirements.
