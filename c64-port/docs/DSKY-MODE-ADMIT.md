# Original DSPMM admission, both missions

`dsky_mode_admit` and `cm_mode_admit` implement actual DSPMM, Luminary099
04,2636 and Comanche05504,2566, through the original NOVAC admission, RELINT
and TC MPAC caller return. DSPMM requests a deferred mode-display job; it
does not draw directly. The exact literal job is LM{03534,060101} or
CM{03441,060101}. Its EB1 descriptor differs from NEWMODEX's EB2 request.

The caller's original Q is saved in MPAC. Core exhaustion retains every
source CPU/scratch/resource word at actual BAILOUT LM05716 / CM07755, including
CM's explicit01202 alarm metadata. Successful admission returns to that
actual saved ordinary caller with Q0155 and IRQ enabled, without an added
scheduler poll. Any incoming raw A/L, all eight aligned EBANKs, all eight
SUPER latch forms and IRQ0/1 are qualified. Fixed bank04 is unchanged.

The domain is an active positive core0, remaining nonzero source priorities,
canonical NEWJOB and VAC markers, ordinary callerQ0100..7777 and canonical
ordinary image words. Both successful admissions and no-core exhaustion are
included. The unchanged admission kernels' +0 core-slot exclusion remains
explicit; its genuine exceptional continuation belongs to Executive closure.
Unsupported inputs preserve image, device metadata and route.

8192 complete independent unchanged-original checkpoints compare all2048
image words plus SUPER/IRQ/CM alarm/route (2052fields). Fresh fixture ropes
remain byte-identical to pinned originals. Native, ASan/UBSan and original
streams have identical SHA256:

`0a88efd4d370676aba5d2313e909eb5b7f26a8cc1badc9b2eb7fab70754fbe16`.

Frozen64 cases cover both missions, allEB/SUPER/IRQ/raw register classes,
caller classes, NEWJOB values, each first available core and exhaustion.
Host and sanitizer include16 atomic rejection cases. Actual stock-ROM PAL
C64 checks32 complete original states per mission: LM mailboxa500002000,
PRG10293bytes (loaded10291), stack16/691; CM mailboxa500002001,
PRG11439bytes (loaded11437), stack14/694. Both use no extra zero page and
fit all image/BSS storage belowC000.

Host/test/sanitize/differential/C64 targets are isolated in
`tools/dsky_mode_admit.mk`; actual target runner
`tools/dsky_mode_admit_vice_test.py --mission lm|cm` requires ROM/data paths.

This closes DSPMM's actual admission/return body. Deferred DSPMMJB formatting,
the lamp-test caller's SWRETURN and release chain, and integrated queued
application scheduling are required subsequent composition milestones.
