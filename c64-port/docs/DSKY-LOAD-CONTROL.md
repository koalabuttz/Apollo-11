# Native load-response and recall control

`dsky_load_control` and `cm_load_control` execute actual VBPROC, VBTERM,
VBRESEQ and PROCKEY through RECALTST to genuine ENDOFJOB, or enter directly
at RECALTST. They set the original LOADSTAT response, kill the monitor,
execute internal RELDSP with actual queued-job JOBWAKE, clear the flash lamp,
then wake the actual ENDIDLE request and store its response continuation and
verb/noun pair. The final original RELDSP and ENDOFJOB source transfers are
preserved; no scheduler poll or fabricated job callback is added.

All ordinary LOADSTAT words are supported. Positive response advances the
recalled basic location by1; negative nonzero advances by0; negative zero
advances by2. Positive zero returns at the actual pathological ENDOFJOB path
with IRQ inhibited after the original wake, without executing the final
RELDSP. CADRSTOR positive zero bypasses waking and unrelated Executive
validation. CADRSTOR negative zero executes the original CCS skip and real
JOBWAKE negative-zero request rather than an assumed invalid caller.

A source no-match wake sets LOCCTR to negative1. The original fixed INDEX
instructions then select LOC-1 and the adjacent MPAC pair. Native code
explicitly performs those known source data operations, including register
addition, TS overflow correction/skipping the noun load, and DXCH debris.
It does not decode modified AGC instructions or pretend a sleeping job was
found. The complete source comparisons qualify this real branch.

Inputs are actual bank40, all EB, SUPER100/120/140/160 and IRQ0/1, ordinary
canonical memory and raw A/L. Only selected wake paths require positive
active core0, positive/dormant/-0 candidates without +0 holes, legal NEWJOB
core displacements/-0, and ordinary saved bank bits for matching dormant
contexts. CM selected wake requires no preexisting Executive alarm. A direct
RECALTST LOADSTAT+0 path ignores DSPLIST: the corpus deliberately gives its
unused list matching dormant invalid bank data. No-wake paths deliberately
contain unusable unrelated Executive data. Unsupported inputs preserve the
image, complete I/O and optional CM event output. Caller holds restoring
producer exclusion; the canonical image is sole authority.

8192 complete2052-field unchanged original/native/fullASan/UBSan states match
all2048 words plus SUPER, IRQ, channel11 and channel event. Full SHA256:
`95cf7221ab900ae8de8a99853bbf9e06437788c62e45a6e4ad97f25c671946fa`.
The independent reference links only original assembly and source-agnostic
inputs. Seven original source files are audited and whole rebuilt ropes must
match pinned fixtures. Existing independently qualified LM/CM JOBWAKE and
internal RELDSP services are composed unchanged.

Sixty-four complete frozen cases pass strict host, sanitizers and actual
stock-ROM C64 VICE. Mailbox a500004002; PRG21628 loaded21626 bytes; hardware/
software stack extents30/773. The diagnostic includes two full canonical
images and the frozen source cases; production service code keeps one owned
canonical image. tools/dsky_load_control.mk provides portable host aliases
and the explicit C64 target. Actual Executive ENDOFJOB execution remains its
separately translated continuation; no application completion is implied.
