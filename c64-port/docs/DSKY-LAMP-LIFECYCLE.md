# Uninterrupted original lamp-test lifecycle, both missions

`dsky_lamp_lifecycle` and `cm_lamp_lifecycle` compose the actual original
controls and scheduler callers without an added job or interrupt poll:

| Source entry | Actual continuation |
| --- | --- |
| VBTSTLTS / TSTLTS1 | WAITLIST, literal task `{03662,062102}`, 500cs, then ENDOFJOB |
| TSTLTS2 | NOVAC, literal job `{03667,062102}`, priority030000, then TASKOVER |
| TSTLTS3 | BANKCALL DSPMM, real NOVAC admission, TC MPAC, SWRETURN, KILMONON, FLASHOFF, POSTJUMP TSTLTS4, RELDSP/JOBWAKE, then END/PINBRNCH/BLNKSUB1 |

Core exhaustion and a full timer queue retain their genuine source BAILOUT
entries and every CPU/scratch/resource word. CM non-P00 VBTSTLTS executes the
actual CHKPOOH and ALM/END to PINBRNCH. The selected admissions preserve their
real caller frames: DSPMM's deferred job is LM `{03534,060101}` or CM
`{03441,060101}`, while the lamp timer/task descriptors use EB2. TSTLTS2
includes original idle-core0 admission as well as admission while another
job is active. No displayed values or queue effects are substituted.

The canonical image is the sole flight authority. Any aligned EBANK, all
four selecting SUPER latches, IRQ0/1 and raw incoming registers are source
qualified. Scheduler preconditions apply only when that source path selects
the scheduler. On/wait paths leave unrelated Executive state opaque; CM's
non-P00 check also leaves the timer state opaque. The full corpus explicitly
includes +0 core priorities/noncanonical NEWJOB on unused paths and negative
TIME3 on the bypassed CM check. Selected admissions retain the frozen kernels'
nonzero core-slot/canonical VAC marker/NEWJOB domain; actual +0 slot closure
belongs to Executive work. Selected display wakes retain their matching
dormant saved-bank domain. Unsupported inputs preserve image, IO, event mask
and selected route.

CADRSTOR minus zero remains its actual inline-CADR/DAS path and reaches
BLNKSUB1. Positive/negative occupied CADRSTOR reaches real PINBRNCH; empty
CADRSTOR reaches ENDOFJOB. Channel11/13 outputs and hardware RESTART-flip-flop
effects are retained through the whole chain; channel12 supplies NO ATT.

8192 independent unchanged-original checkpoints compare all 2048 image
words plus SUPER/IRQ/channel11/channel12/channel13/events/RESTART/CM admission
and wait metadata/route (2058 fields). Fresh fixture ropes remain byte-identical
to pinned originals, and CM alarm metadata comes from the actual original
inline alarm data. Complete native, ASan/UBSan and source streams share SHA256:

`3eb787091f386111fcba58b660c0a0b4c326d00b8a1d2b45a1d2cae9341f5d0e`.

Frozen 64 cases cover both missions, all entry and final route classes,
EB/SUPER/IRQ/raw registers, CADRSTOR/list/mode classes, queue occupancies and
active/idle/unused core0 state. Host and sanitizer add 16 atomic rejection
cases. Actual stock-ROM PAL C64 tests 32 complete source states per mission:
LM mailbox `a500002000`, PRG 32205 bytes (loaded 32203), stack 19/798;
CM mailbox `a500002001`, PRG 37029 bytes (loaded 37027), stack 26/799.
All image/BSS storage fits below C000 and no extra zero page is allocated.

`tools/dsky_lamp_lifecycle.mk` provides isolated host, test, sanitizer,
differential and C64 targets. `tools/dsky_lamp_lifecycle_vice_test.py
--mission lm|cm` checks actual target execution with explicit ROM/data paths.

This qualifies uninterrupted source caller composition, including actual
queue writes. It does not claim to advance a hardware timer by 500cs or run
the queued job automatically. TASKOVER/RESUME, ENDOFJOB/idle dispatch,
PINBRNCH and BLNKSUB1 are genuine compiled continuation entries. Deferred
DSPMMJB has its separate complete body packet; real relay draining/device
output and production disk-overlay scheduling need application composition.
The linked C64 diagnostic is evidence for this body, not a claim that its
combined code is already packaged into the production overlay lease.
