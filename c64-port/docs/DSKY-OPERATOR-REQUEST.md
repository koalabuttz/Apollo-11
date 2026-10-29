# Uninterrupted operator scheduler request setup

`dsky_operator_request` and `cm_operator_request` compose actual Verb30
VBRQEXEC41,3466 or Verb31 VBRQWAIT41,3512 through real RELDSP, selected
JOBWAKE and source REQUESTC return to actual NOVAC/FINDVAC/WAITLIST entry.
The source bit1 selects executive allocation type; source priority/delay,
literal TC service word, caller inline pair and literal TC ENDOFJOB word
retain their exact canonical stores. Q0155 is the original erasable service
caller, not an invented fixed-memory continuation.

The source release clears the external-monitor bit, wakes the selected
DSPLIST continuation when present, clears DSPLIST and DSPLOCK, executes the
real channel11 key-release event and enables IRQ. The immediate REQUESTC
continuation then inhibits IRQ before calling the selected scheduler.
The actual channel event remains recorded even when its bit was already
clear. There is no intervening scheduler poll. Every producer owns one
canonical image; imported Executive views are transient native C work.

The source entry domain is canonical ordinary image words/rawCPU, bank41,
all retained EB, SUPER100/120/140/160 and IRQ0/1. Nonempty DSPLIST selects
the immutable JOBWAKE domain: positive active core0, remaining priorities
positive/dormant/minuszero, canonical NEWJOB displacement or minuszero,
and matching dormant saved banks within the source wake mask. CM selected
wake requires no pending native Executive alarm. Empty-list paths accept
opaque unrelated Executive words, including cleanzero priorities and
invalid NEWJOB, because the source never uses them. Unsupported inputs
preserve image, all external metadata, event and route.

This finishes the uninterrupted display-release and request-setup body.
Admission, WAITLIST, TC ENDOFJOB and downstream recovery remain genuine
separate source continuations. Pending service arguments retain full
ordinary priorities/delays, even when outside a bounded scheduler API;
this module does not impose a fictitious successful allocation.

8192 complete2053-field original/native/fullASan+UBSan states match all
2048 words, SUPER, IRQ, channel11, its actual event and selected service.
Full SHA256:
`69134e04b9bc420fbd340d06b43000cae0959d87a58f135d1bb1e62c82901a2f`.
Both original missions independently assemble eight audited files into
byte-exact pinned ropes. The original oracle never links the native body.
Coverage includes both entries/all three service routes, actual matching,
not-found and no-list wakes, both zero CADRs, every EB/SUPER/IRQ value,
raw CPU forms, canonical pending-job displacements, varied dormant slots,
source scratch exchanges and unused invalid Executive fields.

Sixty-four frozen complete states pass strict host, sanitizers and stock-ROM
PAL C64 VICE. Mailbox a500004002; PRG20822, loaded20820 bytes;
hardware/software stack sentinel extents21/753. Eight atomic guard checks
also pass host tests. Portable host and explicit C64 aliases are in
tools/dsky_operator_request.mk. The diagnostic's two comparison images and
frozen patches are test storage; production owns one canonical image.
