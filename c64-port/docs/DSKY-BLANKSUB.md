# Complete native ordinary blanking call

`dsky_blanksub` and `cm_blanksub` execute actual ordinary BLANKSUB at
Luminary4255 and Comanche4271, including its caller setup, the independently
proved BLNKSUB1 and SUPDXCHZ+1 bodies, and the real selected caller return.
The original caller supplies blanking bits in raw A and an ordinary Q0100..07775.
Any aligned FB/EB and legal SUPER/interrupt state is accepted; DSPLOCK is its
source boolean0/1. The source mask keeps only A bits0..2.

A busy monitor or display lock returns to original Q without blanking.
Available zero mask returns to Q+1. Available nonzero mask captures the actual
caller bank/SUPER and return Q+1 into BUF, switches to original bank40/EB1/
SUPER100, executes each selected component blanking operation, restores the
original DSPCOUNT and returns through the source bank switch. The final
selected return is a C frame boundary; no caller callback or scheduler poll is
inserted. Selected isolated relay words must be nonzero only when the actual
available nonzero branch uses them. Busy/zero paths accept arbitrary relay
words, including clean +0. Unsupported inputs preserve all state.

This completes ordinary BLANKSUB's return-pair producer. It does not assume
that the separate KEYREL CADRSTOR=-0 fallthrough had executed that producer;
KEYREL stale BUF closure remains explicitly separate. The canonical physical
2048-word image is authoritative; only SUPER/interrupt enable are external
metadata, and the caller owns producer exclusion. Native execution contains
no AGC instruction fetch or decoder.

The independent original LM+CM ropes are rebuilt with unchanged Pinball,
T4, Interpreter, Executive, Display Interface and Fresh Start sources; CM
indentation-only changes are recorded and whole rope equality required.
8192 complete2050-field original checkpoints cover all masks, both busy bits,
all FB/EB/SUPER banks, raw A/L values, both count/NOUT signed zeros, overflow
edges and opaque previous BUF. The stopping condition checks actual restored
FB and source Q2/Q3 as well as PC to avoid mistaking a Pinball subroutine's
same numeric PC for the actual caller return. Native and full ASan/UBSan
streams match original bytes, SHA256:
`9d5edb7bd0dc942abb25aa72bcec3d551a4c0b331c200cf2dc88749dbf1b6ed1`.

Sixty-four frozen complete original states run in strict host, sanitizer and
actual native C64 diagnostics. `tools/dsky_blanksub.mk` supplies portable host
all/test/sanitize aliases and an explicit C64 build; the stock-ROM VICE proof
checks mailbox, RAM limit, zero-page and hardware/software stack sentinels.
Generic erase, ENTER and remaining Pinball application fanout are separate
services still needed for the whole-program certificate.
