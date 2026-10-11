# Canonical LM delayed tasks and WAIT2

`lm_delay_image` executes literal FIXDELAY05221 or VARDELAY05224 through insertion and the actual TASKOVER05261 entry, or the original1203/1204 alarm frontier. FIXDELAY reads its inline time from canonical erasable INDEXQ storage, or explicit compiled fixed metadata; VARDELAY receives time in A. The scheduled continuation is the real caller+2 or caller+1 instruction, with the original BBANK/SUPER. The native function does not finish TASKOVER, execute a task callback, or poll NEWJOB.

`lm_waitlist_internal` exposes the genuine shared WAIT203231 entry for already-prepared callers. Canonical WAITEXIT holds the source return word, WAITADR the task location, A the caller BBANK, L the task BBCON and Q the delta. The source return word may be DELAYEX015257: original AD TWO and DTCB produce literal TASKOVER05261 while retaining the correct raw A/L/Q and scratch. Direct ordinary caller returns and alarm cases use the same source body. No instruction word is decoded.

The body preserves the real TIME3/pending-overflow arithmetic, ordered list insertion, split intervals, dropped final task, caller-bank/scratch debris and reservation before a full-list alarm. Shared fields061..064 remain sole canonical storage. Negative and both zero delay encodings reach POODOO1 rather than a fabricated native correction; source IRQ remains inhibited.

All16,384 complete2048-word plus SUPER/IRQ original assembly states match strict C and full ASAN/UBSAN across all three entries, free/full queues, valid delays and1203/1204 routes. A separate16,384-state equivalence test prepares literal WAIT2 from the unchanged frozen WAITLIST/TWIDDLE caller body and compares every resulting canonical/I/O word. Thirty-one mutation guards and four null interfaces preserve all rejected objects.

Stock PAL C64 ROM/D81 checks64 frozen original complete-state fingerprints through32 returned fixture modules, mailbox a50000400002. Software/hardware stack extents120/29 bytes, extra zero page zero; native I/$01 is restored. The sole AGC image is$7000 and no active C frame spans replacement of an overlay.

The original WAITLIST documented positive maximum16250cs, coherent source banks, normal compiled return descriptors and owned task producer exclusion remain explicit contracts. Oversize positive delays, malformed hardware/code descriptors and arbitrary interruption are excluded before mutation. Complete WAITLIST/SVCT3/LONGCALL application acceptance remains a separate milestone.

Fresh-checkout targets are `lm-delay-image-port-test` and `lm-delay-image-port-sanitize` in tools/lm_delay_image_port_integration.mk. They use private host builds, retained originals and checked target fixture descriptors; no historical source/helper/evidence is modified.
