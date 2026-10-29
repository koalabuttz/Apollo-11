# Comanche source alarm recording

`cm_alarm_record` translates actual ALARM (`05650`), VARALARM (`05764`),
CURTAINS (`05757`), and BORTENT (`05655`) through the source alarm-record body.
It writes ALMCADR/BBCON, LOCALARM/BANKALRM and the three FAILREG words,
updates the source DSPTAB lamp request, and commits the genuine return CPU
image. ALARM takes its inline alarm word as certified passive caller data;
BORTENT retains ALMCADR established by its earlier abort-entry producer.

Source CCS distinguishes +0, -0, positive and negative FAILREG values.
Negative FAILREG values can branch without replacing that word; these actual
branches are preserved. MULTFAIL adds BIT15 to the saved L alarm code.
An overflowing TS skips the return branch and reaches genuine POODOO
(`05721`) with interrupts still inhibited. This API commits that actual
continuation instead of reporting an ordinary caller return. The selected
abort recovery body executes in a later native source group.

The sole persistent state is the canonical 2,048-word image with explicit
SUPERBNK/IRQ latches. Ordinary compiled Q must be `0100..07775`, banks must be
consistent, and VARALARM/BORTENT require a nonoverflow raw A alarm code.
The caller owns producer exclusion and passive inline-word qualification.
Unsupported inputs preserve image and I/O. No saved source word is decoded
or executed as a native instruction.

The original-layout source rope SHA256 is
`6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061`.
All 16,384 original complete-image checkpoints match raw A/L/Q and external
SUPERBNK/IRQ with lossless stream SHA256
`fc35d6b6420ed71181814d05dd5d25ef9e8e9d8ee79ed95f1f907fec312abde2`.
The corpus covers all combinations of eight FAILREG corners for all four
entries, varying alarm codes, bank and display-lamp state. Of these states,
1,803 take the genuine overflow POODOO continuation; 14,581 return to their
actual ordinary callers. Full strict and ASAN/UBSAN validation pass.

The C64 packet loads actual native control 214 and proof-only input/checker
254/255 from D64 through the checked loader. Its 64 representative contexts
cover all four entries, return and overflow continuations, with alternating
incoming C64 interrupt state. Every complete state is fingerprinted; two
complete image/I/O samples and the final image are compared directly. The
retained report records the actual verdict, payload bytes and stack extent.

Run `make -f Makefile -f tools/cm_alarm_record.mk test-cm-alarm-record` and
`sanitize-cm-alarm-record`. The packet qualifies these alarm-record groups;
BAILOUT/CCSHOLE snapshot production, WHIMPER/ENEMA recovery and every original
alarm-file entry require separate source closure.
