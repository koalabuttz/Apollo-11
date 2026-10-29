# Original input-factor reader and scaling dispatch

This additive native service composes PUTDECSF41,3174, PUTSFNOR41,3206 and
PUTDCSF241,3207 from both missions through the real original SFCONUM/GTSFIN/
SFRUT calls and BANKJUMP. It stops at the selected actual source input-scale
entry, retaining the whole canonical CPU/image and external metadata. The
route value is the original routine index0..13. Scaling is separate compiled
source work, not a fabricated callback result or runtime instruction decoder.

`dsky_input_factor` uses `dsky_lamp_io`; `cm_input_factor` uses `cm_native_io`
plus channel-event and restart-light words. Callers own restoring producer
exclusion. The canonical image requires physical bank41, FB062000, selecting
SUPER100/120/140/160, any retained aligned EBANK, coherent BBANK, ZERO+0,
IRQ0/1, ordinary memory words and raw A/L registers. These entries do not read
incoming Q as an external return descriptor. Unsupported contexts preserve
image, IO, event, restart and route atomically.

PUTDECSF requires MIXBR1 normal or2 mixed; only selected mixed input requires
DECOUNT0..2. The selected constant number is0..28 LM or0..19 CM, covering every
pair in the actual mission SFINTAB. The selected routine is0..13. PUTSFNOR
requires only the normal NNTYPTEM middle5 routine, and ignores unused MIXBR,
DECOUNT and constant-number fields. PUTDCSF2 uses incoming A0..13 directly,
retaining all otherwise unused table scratch. It supports both zeros in packed
fields and the genuine zero-valued scale factors. The constructor includes
opaque unused words and raw register extrema.

PUTDECSF executes the exact saved-call and GTSFIN DXCH Z convention: it
selects temporary bank42/EB1, loads the immutable source factor into SFTEMP1/2,
and restores the original retained caller bank. It then selects the normal or
mixed source routine, preserving EXITEM, L dispatch instruction, editing
registers and reader/return CPU debris. The final BANKJUMP preserves the actual
TC Q3212 input in A, sets Q to the chosen CADR offset, sets FBANK/BBANK and
actual logical source PC, retains EBANK/SUPER/IRQ and never polls the scheduler.
The literal source SFINTABR dispatch differs between missions: LM routine6 is
DSPALARM and13 DEGINSF, while CM6 is OPTDEGIN and13 DPFRACIN. HMSIN selects
bank42; other ordinary scales select bank40; routine0 selects GOALMCYC in bank41.
All actual CADRs are frozen from the original source, not guessed from the LM
address layout.

The immutable noun_reader/fixed_data APIs supply native original table data
and exact source reader exchanges. Those APIs are shared unchanged. Target
builds select APOLLO_FIXED_LM_ONLY or CM_ONLY, with no runtime AGC instruction
engine or opaque full-rope execution on the C64.

All8192 complete source/native/full ASan/UBSan states match: every2048 image
word plus SUPER, IRQ, channels11/12/13, events, restart and selected routine.
The independent oracle freshly assembles both original programs, verifies each
rope against its pinned fixture, and links no native service. Snapshot SHA256:
`c600a8b6584350fb6c382943d8e0319e553c189816fa0a918b377142b4023917`.
Coverage spans all routine and scale-constant indices, components, bank/latch
selections, raw register classes, source zero fields and opaque unused inputs;
combinations are sampled rather than asserted as a Cartesian product.
Frozen64 whole states plus16 meaningful atomic guards pass strict host and
sanitizers. Separate native LM/CM C64 diagnostics each execute32 full frozen
states, checking actual PAL VICE mailbox, no extra zero-page storage,
below-C000 memory budget and hardware/software stack sentinels. These are
component linked diagnostics, not a production disk-overlay application proof.

`tools/dsky_input_factor.mk` supplies host-only all/test/sanitize aliases,
explicit C64 and fresh differential/freeze targets. The frozen packet retains
exact source and dependency identities, fixture/listing audits, complete
compressed source/native/sanitizer streams, host logs, target PRG/maps and raw
mailbox/stack bytes. Whole scale bodies, final loaded-value caller stores,
channel output and application keyboard lifecycle remain explicitly tracked.
