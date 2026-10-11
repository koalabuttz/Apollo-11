# Source startup into the live P66 application

The resident application now starts at the actual Luminary STARTP66 entry,
runs FASTCHNG/NEWMODEX, the complete bias initialization and vertical startup,
then follows the translated P66 descent, throttle and horizontal computations.
The actual deferred DSPMMJB and MAKEPLAY jobs run through canonical ENDOFJOB
to the source ADVAN idle entry. This replaces manually positioning the resident
at RODCOMP for this composition. Upstream navigation and sensor producers remain
explicit owned inputs; this is a qualified application path, not completion of
all Luminary or Comanche source domains.

`p66_application_binding` places the existing display/descent/basic/arithmetic
binding at offset zero. Its compiled resolver names original source entries;
it never reads an AGC opcode. Startup uses modules50 (setup),83 (bias),52
(vertical). Each returns its normal C frame before another image replaces the
single slot. Ownership remains with the same job across these internal service
seams. Phase's source-qualified direct-launch callback adopts RODCOMP without
an additional NEWJOB or asynchronous request check. Initial STARTP66 retains
the qualified NEWJOB+0 domain: its real bias interpreter has scheduling checks.
The separate final VRTSTART entry supports its verified pending-job domain.

Deferred mode-display and CHARIN VERB/NOUN/digit callbacks run their actual
source bodies, then invoke the canonical source END helper directly. The source
selected basic/interpreter/idle result is adopted once. WDRET, raw A/L/Q,
editing registers, source interrupt enable and all other erasable words remain
part of the comparison. Channel11 is explicit hardware metadata; the native
peripheral latch is synchronized after END, including its activity lamp clear.
Other CHARIN keys remain outside this resolver body's qualified service domain.
The keyboard interrupt producer and100-centisecond task recurrence are separate
extensions; their modules are packaged but this target run does not invoke them.

Validation retained in `evidence/p66-application-startup-proof.json`:

- 2,048 complete startup/final-vertical states through the actual application
  service router match the original2050-word corpus under strict and full
  ASAN/UBSAN builds.
- 13,312 mode-display/VERB/NOUN/digit jobs match all2052 raw source words through
  canonical ENDOFJOB and idle, including SUPERBNK, source IRQ, channels11/14.
- 16 actual startup/application paths match1,226 complete raw2052-word original
  checkpoints. No output word is normalized or omitted.
- Stock PAL C64 ROMs and the actual104-module D81 execute two frozen startup
  inputs through156 raw-source fingerprints, with both SUPER forms and restoring
  enabled/masked caller IRQ state. Mailbox `a500009c0002`; software stack777 bytes,
  measured hardware stack70 bytes including the14-byte ROM margin; no extra ZP.

The source audit compares every statement in nine complete original service
files. It records the existing transcription label spellings DSPOCTWO/DSPOCTWD
and ISWCALLL/ISWCALL and one redundant declaration. This reconciliation affects
only source-text auditing; every compared machine word is retained unchanged.
Historical original files remain untouched. The host-only oracle uses the
pinned engine and established hardware-DV variant; neither is a native runtime.
Source scaler advancement is excluded and channel14 drive-enable bits are clear,
so hardware counter producers cannot mutate unrelated source words during proof.

The unchanged physical layout is canonical words `$7000..$7fff`, slot
`$8000..$bfff`, software stack `$c800..$cfff`. Executive view is648 transient bytes
at `$c100..$c387`; the76-byte task view lease `$c388..$c3d3` remains reserved.
Application metadata is163 bytes at `$c400..$c4a2`, within its explicit
`$c400..$c5ff` reservation. Frozen fixture metadata uses `$c000..$c03f`, mailbox
`$c080..$c085`. The resident ends at `$6810`, leaving2,032 bytes below the
canonical image. No overlay function pointer or active overlay C frame survives
a load. Existing scheduler service images150..153 handle the same locked seams.

Reproduce with `tools/p66_application.mk`. Target inputs, source checkpoints,
resident, disk, all loaded images and dependency identities are retained. This
milestone does not broaden the older modeled-word pending-job corpus into a
raw-source multi-job claim. Canonical CHANG2/higher-priority composition, recurring
RODTASK, the remaining keyboard bodies and other landing programs remain work.
