# Native internal display release

`dsky_display_release` and `cm_display_release` execute actual internal RELDSP
at Luminary04457 or Comanche04473 through the ordinary source caller stored
in Q. They clear the external monitor flag under INHINT, inspect DSPLIST,
compose independently verified actual JOBWAKE when selected, then execute
RELDSP2, channel11 key-release light clearing, DSPLOCK clearing and source
RELRET return. Final Q0145, A, L, scratch, banks, Executive/VAC and IRQ effects
are preserved. This is the internal callable body used by extended verbs,
not a synthetic CHARIN invocation.

Inputs support all aligned FB/EB and SUPER0/20/40/60/100/120/140/160, IRQ0/1,
ordinary caller Q0100..7777 and ordinary erasable words. DSPLIST+0 bypasses
Executive validation entirely; the corpus deliberately uses invalid unrelated
Executive words there. Selected nonempty list branches require the actual
qualified wake domain: positive active core0, positive/dormant/-0 candidates
without +0 priority holes, NEWJOB-0 or valid12-word core displacements, and
ordinary saved bank bits for the first matching dormant context. CM selected
wake additionally requires no preexisting external Executive alarm. Both
negative-zero list and signed list CADRs are qualified. DSPLOCK itself is
unconditionally cleared and has no artificial0/1 restriction.

The canonical image is sole authority. Caller owns restoring producer
exclusion. Rejected domains preserve image and complete I/O; the optional CM
channel-event output is untouched on rejection. The generic source return is
qualified to ordinary Q callers; register/erasable executable request words
outside that domain remain explicit later continuations.

8192 complete2052-field original/native/fullASan/UBSan states match unchanged
original assemblies, all2048 canonical words plus SUPER, IRQ, channel11 and
channel event. Full SHA256:
`8723a623a2480ba28a3998701f19da01bb0bb4d6f9c141441b599446556b3004`.
Separate mission originals and seven-file source audits must produce complete
pinned ropes unchanged. The oracle excludes the tested native service and
uses only source-agnostic inputs and original assembly execution.

Sixty-four frozen complete states pass strict host, sanitizer and native C64
checks. Stock-ROM VICE mailbox is a500004002, PRG16405 loaded16403 bytes,
hardware/software stack sentinel extents20/748. The diagnostic includes both
complete input/output canonical images and source wake fixtures; production
code does not store a duplicated authoritative image. Portable host aliases
and the explicit C64 recipe are tools/dsky_display_release.mk. Extended-verb
POSTJUMP, fanout and verb bodies remain actual separate continuations.
