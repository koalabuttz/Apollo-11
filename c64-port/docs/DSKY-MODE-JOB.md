# Original deferred DSPMMJB, both missions

`dsky_mode_job` and `cm_mode_job` execute actual DSPMMJB, Luminary09940,3534
and Comanche05540,3441, through original ENDOFJOB or the genuine CCSHOLE
entry selected by clean-plus-zero display data. They write the real relay
words consumed by DSPOUTSB rather than a display callback.

MODREG0..99 formats two decimal mode digits through the unchanged original
DSPDECVN/DSPIN arithmetic kernels. Minus zero uses actual2BLANK; negative
nonzero leaves the displayed mode untouched. Original DSPCOUNT is restored
and DSPMMTEM retains the precise arithmetic debris on normal completion.
Any aligned EBANK, four selecting SUPER latches100/120/140/160 and IRQ0/1
are supported; raw incoming A/L/Q and arbitrary ordinary DSPCOUNT/MPAC words
are retained or changed exactly as the source specifies. Formatter arithmetic
has no SUPER write, and an independently qualified transient arithmetic view
allows reuse of the frozen helper while retaining the actual physical latch.
Existing released modules are unchanged.

Clean-plus-zero DSPTAB mode word is accepted. Its decimal path executes the
real first SHORTMP/TRACE1/DSPIN prelude and reaches CCSHOLE LM05705 / CM05747
with exact incoming Q/banks/scratch/IRQ. No recovery or job completion is
invented. Minus-zero blanking accepts either zero relay representation.
NOUT includes zero, minus zero, and ordinary overflow edges: original INCR
uses register addition with source overflow correction.

8192 complete unchanged-original checkpoints compare every2048 image words
plus SUPER/IRQ/route (2051fields). Fresh fixture ropes remain byte-identical
to pinned originals. Full native, ASan/UBSan and source streams share SHA256:

`7e946d90991c6fdda4fe96153a8540653ec60b41adc3c4ef3effdf8801d58333`.

The full corpus exercises all100 positive/zero modes, negative/blank modes,
zero/signed relay classes and NOUT edges. Frozen64 original cases cover both
missions, allEB/SUPER/IRQ/raw register classes, ordinary relay/NOUT/saved-count
classes and END/CCSHOLE branches. Host/sanitizer add16 atomic rejection cases.
Actual stock-ROM PAL C64 checks32 complete source cases per mission: LM
mailboxa500002000, PRG11912loaded11910, stack25/63; CM a500002001,
PRG12264loaded12262, stack22/97. All image/BSS storage fits belowC000 and
no extra zero page is allocated.

`tools/dsky_mode_job.mk` supplies isolated host/test/sanitize/differential/C64
targets. `tools/dsky_mode_job_vice_test.py --mission lm|cm` checks actual target
execution with explicit ROM/data paths. Unsupported inputs preserve image,
IO and route.

This is the complete stated DSPMMJB body. Real Executive job completion,
CCSHOLE/recovery and the asynchronous relay-to-device lifecycle are separate
compiled continuations to compose, not included in this body certificate.
