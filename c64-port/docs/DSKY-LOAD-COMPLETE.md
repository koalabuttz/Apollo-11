# Original completed-load release and recall

This additive native service translates actual LOADLV41,2775 for both missions,
including genuine RELDSP, source POSTJUMP to RECALTST, selected JOBWAKE,
return-context updates, the second display release and actual ENDOFJOB.
It composes the unchanged original-qualified display-release/response-recall
and canonical LM/CM wake services. No extra Executive poll is introduced.

`dsky_load_complete` uses the canonical image and `dsky_lamp_io`;
`cm_load_complete` uses the canonical image, `cm_native_io`, retained channel
write events and restart-light word. The caller retains restoring producer
exclusion. Entry requires PC02775, FB062000, aligned retained EBANK0..7 with
coherent BBANK, ZERO+0, SUPER100/120/140/160, IRQ0/1, restart0/1 and ordinary
channel11. Incoming raw A/L/Q and all ordinary old DECBRNCH/LOADSTAT classes
are supported; LOADLV overwrites the latter fields before inspecting them.

Selected nonempty DSPLIST or CADRSTOR wakes require the frozen canonical wake
caller contract: active positive core0, no +0 candidate core, NEWJOB-0 or an
aligned valid core displacement, and a matching dormant saved bank<=0177.
CADRSTOR and DSPLIST themselves support all ordinary word classes including
-0. The read-only preflight accounts for the first DSPLIST wake: that newly
positive core is skipped when identifying a later CADRSTOR match. Empty
DSPLIST/CADR paths leave unrelated Executive words opaque and may have
unsupported scheduler markers because no wake reads them. CM pending
executive-alarm metadata is excluded only when a wake is selected. Unsupported
input preserves complete image and IO/events/restart atomically. These wake
preconditions preserve historical verified helpers; +0/corrupt Executive
exceptional paths remain their separately tracked owner work, not a successful
load certificate.

LOADLV stores DECBRNCH+0 and LOADSTAT-0, executes actual RELDSP with caller
03002, clears DSPLOCK/key-release ownership and monitor-external bit, and
retains the original channel11 write, restart-lamp behavior and wake debris.
It stores source -19 in DSPCOUNT and performs the actual POSTJUMP to
LM40,3547/CM40,3454. RECALTST supports both empty CADRSTOR and real matched/
unmatched wakes. The source -0 LOADSTAT selects a displacement of two; all
context stores and raw register effects are retained. An unmatched JOBWAKE
returns -1 and executes the genuine indexed LOC-1/MPAC-1 exchanges; no slot
is fabricated. The final stage ends at actual LM05155/CM05217 ENDOFJOB with
exact source CPU/banks/IRQ and display-device state. End-of-job scheduler
execution is a separate source service. Channel12/13 and SUPER are retained.

The independent fresh-assembly oracle checks both ropes against their pinned
original fixtures and links no tested native service. All8192 complete source
states match strict native C and the full ASan/UBSan stream: all2048 image
words plus SUPER, IRQ, channels11/12/13, events, restart and route.
Snapshot SHA256:
`473bff611cff793c8f1f419426db454631908c4b509e674220acc1bc20a885e7`.
The corpus samples all CADRSTOR/DSPLIST zero/sign classes, matched/unmatched
and successive wakes, all old load-state classes, every EBANK/selecting SUPER
form, raw register encodings, pending-source priorities, restart and device
values. Frozen64 whole source images and16 atomic guards pass strict host and
sanitizers. These are sampled combinations, not a Cartesian-product claim.

Actual C64 PAL VICE diagnostics pass32 complete source states per mission.
LM mailboxa500002000, PRG18135 bytes/loaded18133, hardware/software stack
extents18/746; CM mailboxa500002001, PRG18223 bytes/loaded18221, stacks19/725.
Both fit the below-C000 linked diagnostic RAM budget without extra zero-page
storage. Their linked Executive dependencies make each diagnostic exceed a
16KB overlay; this component proof does not claim a production disk payload
or uninterrupted application integration. Exact source/dependency hashes,
fixture audits/listings, complete compressed original/native/sanitizer streams,
logs, PRG/maps and raw mailbox/stack evidence are frozen.
`tools/dsky_load_complete.mk` supplies host-only all/test/sanitize aliases and
explicit C64/differential/freeze commands. Final load-verb caller/flag stores,
channel output and uninterrupted operator/application composition remain work.
