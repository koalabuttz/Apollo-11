# Original TSTLTS4 release, both missions

`dsky_lamp_release` and `cm_lamp_release` translate actual TSTLTS4,
Luminary09940,3471 / Comanche05540,3376, through real RELDSP and its selected
JOBWAKE path. They reach original ENDOFJOB for empty CADRSTOR, POSTJUMP to
actual PINBRNCH for nonzero occupied CADRSTOR, or BLNKSUB1 for minus zero.
No native poll is added between original calls.

CADRSTOR minus zero is a real reachable source branch. CCS's three-word skip
executes inline PINBRNCH's literal CADR as DAS: LM updates physical01047/01050,
CM01271/01272, including original L effects. It falls into UNSUSPEN, clears
DSPLOCK, then reaches actual BLNKSUB1 LM40,3505 / CM40,3412. The compiled
native branch reproduces these exact stores; it does not decode instructions
or claim that this path reaches job completion.

The source image remains the sole authoritative flight state. Any EBANK,
four selecting SUPER latches100/120/140/160, raw registers and IRQ0/1 are
qualified. CADRSTOR includes both zeros and positive/negative extremes.
Nonempty DSPLIST uses the unchanged wake kernel's positive active-core0,
nonzero remaining priorities, canonical NEWJOB and matching dormant saved
bank domain; +0 core-slot wake closure belongs to Executive work. Empty
DSPLIST leaves unrelated Executive words opaque, including +0 priorities or
noncanonical NEWJOB. CM's Executive alarm guard applies only to a selected
wake. Unsupported inputs preserve all words/latches/events/route.

Channel11's actual RELDSP write clears key-release, and an output retaining
01000 clears the hardware RESTART flip-flop. Channel12/13 remain untouched;
unused channel13 trap bits are not rejected.

8192 independent unchanged-original full checkpoints compare every2048
state words plus SUPER/IRQ/channel11/channel12/channel13/event-mask/
RESTART-flip-flop/route (2056fields). Native and full ASan/UBSan streams are
byte-identical to original:

`787eaf80de5967dcf41cbd0935a7f3e3ea972df95b8aa97041ca037fca6dcb4e`.

Frozen64 source cases cover both missions, all EB/SUPER/IRQ choices, raw
registers, CADRSTOR classes, empty/nonempty signed-zero lists, matching/
unmatched dormant wakes, wake slots, and RESTART states. Host/sanitizer
include16 atomic guard cases. Actual stock-ROM PAL C64 runs compare32 complete
source states per mission: LM a500002000, PRG13475loaded13473, stack17/723;
CM a500002001, PRG13931loaded13929, stack16/722. Both fit belowC000 and add
no zero-page storage. Source fixture ropes remain byte-identical to originals.

`tools/dsky_lamp_release.mk` provides isolated host/test/sanitize/differential/
C64 targets; `tools/dsky_lamp_release_vice_test.py --mission lm|cm` checks
actual target execution with explicit ROM/data paths.

This closes TSTLTS4 to genuine selected entries. PINBRNCH, BLNKSUB1 and
canonical job completion remain explicit source continuations to compose;
queued lamp-test scheduling and the visible display path are separate proofs.
