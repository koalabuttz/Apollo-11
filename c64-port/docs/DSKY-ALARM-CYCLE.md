# Native alarm recycle call and return stages

`dsky_alarm_cycle` and `cm_alarm_cycle` implement actual ALMCYCLE
(LM04145/CM04161) through FALTON and BANKCALL to UPDATVB-1 at41,2337,
and actual SWRETURN(LM04631/CM04675) or after-call04152/04166 through
POSTJUMP to ENTER41,2002. These are source call/return stages; the middle
UPDATVB formatter is a separate service. The initial stage executes the
real channel11 operator-error event, complements VERBSAVE into REQRET,
forms source-owned BUF2 call frame and presents the actual A/L/Q/bank
state to the updater. It imposes no formatter guard on the pending callee.

The full ordinary VERBSAVE domain, including both zeros and sign boundaries,
is preserved. In particular VERBSAVE+0 produces raw A-0 and REQRET-0 at the
updater entry. The source return executes the original bank restore and
literal POSTJUMP, preserving full register/scratch/source image effects.
SWRETURN corrects incoming raw A through ordinary BUF2 RAM; direct after-call
POSTJUMP preserves raw register overflow bits. This distinction is proved,
not approximated by converting every register to an ordinary word.

Inputs accept canonical ordinary image words, raw16 CPU, any aligned FB/EB,
SUPER100/120/140/160 and IRQ0/1. Actual SWRETURN requires FB062000, the
literal source BUF2 return04152/04166 and aligned saved caller FB; direct
after-call entry accepts unused opaque BUF2. Selected source stages make
no scheduler poll/callback. Unsupported inputs preserve complete image,
external I/O and route; caller owns restoring producer exclusion. The
original verb formatter, ENTER fanout and actual alarm/recovery downstream
are separate continuations and are not reported as completed here.

8192 complete2053-field unchanged original/native/fullASan+UBSan checkpoints
match all2048 words plus SUPER, IRQ, channel11, its event and route.
Full SHA256:
`a8ac851dc2249fb04aff61efcfa02c532f0a6e11809deb80b5cacd00e4546336`.
Both missions independently rebuild with eight-file original audits including
actual interbank source and byte-exact pinned ropes. The original oracle
links no native body; updater/ENTER stops select actual physical bank41.
Coverage includes all three stages, all aligned caller FB, all EB/SUPER/IRQ,
both signed zeros, all raw CPU sign/overflow forms, deliberately clean-zero
pending relay and unused opaque source scratch.

Sixty-four frozen full checkpoints pass strict host, sanitizers and native
stock-ROM C64 VICE. Mailbox a500004002; PRG5729 loaded5727 bytes;
hardware/software stack extents12/35. The diagnostic holds two canonical
images and frozen expected changes; production owns one image. Portable
host aliases and explicit target are in tools/dsky_alarm_cycle.mk.
The next complete recycle composition needs the separately owned widened
UPDATVB qualification for signed-zero and selected-clean+0 branches.

The additive cleanup audit evidence/phase-octal-cadr-blank-raw-cleanup-
20261012.json records nine inactive committed raw streams reclaimed after
independent HEAD archive/blob, decoded SHA and byte-size verification.
The exact retained evidence objects remain unchanged; this record is part
of the current milestone allowlist and does not overwrite earlier audits.
