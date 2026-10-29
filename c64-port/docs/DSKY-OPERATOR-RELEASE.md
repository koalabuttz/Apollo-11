# Original operator display release, both missions

The native `dsky_operator_release` and `cm_operator_release` bodies cover
VBRELDSP (LM 40,3461 / CM 40,3366) and direct UNSUSPEN (LM 40,3477 /
CM 40,3404). VBRELDSP clears UPACT through the actual channel 11 output,
then follows the original saved-lock and monitor branch. Its normal path
composes the unchanged TSTLTS4/RELDSP/JOBWAKE service. Monitor unsuspension
clears DSPLOCK and follows the actual CADRSTOR/list controls without an
extra Executive poll. Completion stops at genuine ENDOFJOB, PINBRNCH, or
BLNKSUB1 entries.

All ordinary saved-lock values are modeled, including both zeros, minus
one, and minus two. Source CCS skips differ for these values: minus zero
and negative values other than minus one select unsuspension; positive
locks select it only when MONSAVE1 bit 14 is set. Direct UNSUSPEN preserves
incoming Q on the minus-zero CADRSTOR branch. Its plus-zero CADRSTOR path
executes RELDSP1, and only an empty DSPLIST selects the RELDSP2 channel
write and interrupt enable.

The canonical image is the only flight state. The caller owns producer
exclusion. Any EBANK, selecting SUPER 100/120/140/160, IRQ 0/1, and raw
registers are qualified. Normal release with nonempty DSPLIST requires
the frozen wake kernel's positive active core 0, nonzero other priorities,
canonical NEWJOB, and legal matching dormant bank. Empty-list and
unsuspension paths leave unrelated Executive words opaque, including
plus-zero priorities and noncanonical NEWJOB. CM's Executive alarm guard
applies only to a selected wake. Rejected inputs preserve the full image,
latches, event mask, restart flip-flop, and route.

Channel 11 writes retain their actual event and restart flip-flop effects;
channels 12/13 are untouched. No arbitrary opcode execution or emulator
is linked into either native body.

The independent unchanged-original oracle compares 8,192 full checkpoints:
all 2,048 source words plus SUPER, IRQ, channels 11/12/13, event mask,
restart flip-flop, and selected route. Original, native, and full
ASan/UBSan streams are byte-identical, SHA-256:

`b33a7d06f9cfa0d298155284dd03d42525fccdf56f4561cd3a96e1b9af848bca`.

Frozen 64 cases and 16 atomic guards pass host and sanitizer tests. Actual
stock-ROM PAL C64 diagnostics compare 32 full source states per mission:
LM mailbox `a500002000`, PRG 13,540 bytes (13,538 loaded), stack extents
16/724; CM mailbox `a500002001`, PRG 13,935 bytes (13,933 loaded), stack
extents 16/722. Both stay below C000 and allocate no additional zero page.
Fresh assembled fixture ropes match their pinned originals byte for byte.

`tools/dsky_operator_release.mk` provides isolated host, test, sanitizer,
differential, and C64 targets. The VICE runner accepts explicit ROM and
data paths. The selected END/PIN/BLANK continuations, device rendering,
and live job/task composition remain separate source-qualified stages.
