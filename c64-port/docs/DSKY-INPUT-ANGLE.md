# Original degree and optics input scaling

This additive native service translates actual DEGINSF, DEGINSF2, SIGNFIX,
NEG180, SGNTO1 and ENDSCALE, plus LM360-CDU/360-CDUE and CMOPTDEGIN/OPDEGIN2.
It composes unchanged original-qualified DMP, TPSL1, 2ROUND+, TESTOFUF and
SFRUT source services. Native C executes compiled source control and arithmetic;
no instruction engine is linked to the native implementation.

| Entry | LM bank40 PC | CM bank40 PC |
|---|---:|---:|
| DEGINSF | 03010 | 02716 |
| DEGINSF2 | 03021 | 02727 |
| SIGNFIX | 03031 | 02737 |
| NEG180 | 03066 | 02753 |
| SGNTO1 | 03070 | 02755 |
| ENDSCALE | 03043 | 02751 |
| 360-CDU / OPTDEGIN | 02625 | 03004 |
| 360-CDUE / OPDEGIN2 | 02634 | 03011 |

`dsky_input_angle` accepts the canonical image and `dsky_lamp_io`;
`cm_input_angle` accepts the canonical image, `cm_native_io`, retained channel
write events and restart light. Callers retain restoring producer exclusion.
Entries require FB060000, SUPER100/120/140/160, any aligned retained EBANK
with coherent BBANK, ZERO+0, IRQ0/1 and restart0/1. Raw A/L, all ordinary
TP limbs and overflow flags, including either zero, are supported.

LM scale entries require MIXBR1/2, with DECOUNT0..2 only when MIXBR2 selects
mixed indexing. This bounded scale caller contract also applies when the
selected arithmetic branch reaches an alarm before reading those fields.
LM standalone 360-CDU entries instead require an ordinary incoming caller
Q0100..7777 and retain unused MIXBR/DECOUNT opaquely. CM entries never read
MIXBR/DECOUNT and accept arbitrary retained words there. Unsupported input
preserves the complete image, IO, events, restart and route atomically.
This is a source-bounded service, not a whole Pinball arbitrary-caller claim.

The degree body uses the literal original constant pair026161/030707 and
preserves source ADDRWD, MPTEMP, A/L and MPAC exchanges. The middle-word
rounding bias distinguishes positive, negative and both zero classes. All
three actual TPSL1 shifts, 2ROUND+2 conditional TS skips and TESTOFUF
frontiers are retained. The source CCS magnitude adjustment and SIGNFIX
paths preserve sign correction, signed zero, NEG180 and force-sign stores.
A positive magnitude under a negative flag, or flag-0, reaches actual
CCSHOLE (LM05705/CM05747) with the source Q and CPU debris. It is not
converted to a fabricated successful return.

LM ENDSCALE executes the actual BANKCALL/SFRUTNOR-or-MIX/SWRETURN exchanges,
including BUF2/EXITEM and editing-register debris. Its routine-code comparison
supports all32 five-bit fields: code2 skips the actual 360-CDU correction;
other fields execute it. Standalone 360-CDU preserves its ordinary caller
return and Q3. CM ENDSCALE performs the genuine direct POSTJUMP and retains
incoming A. CM optics subtracts the literal06250 bias, multiplies by
021616/07071 and rounds with BIT12. OPTDEGIN rejects negative major words,
including -0, through the actual ALMCYCLE frontier; direct OPDEGIN2 starts
after that check and supports all ordinary major words.

Completed scale entries reach actual PUTCOM241,3155 with FB062000, Q01155
and the real retained result in A. RECYCLE stops at ALMCYCLE04145/04161.
TESTOFUF flag-0 reaches the real LM03200/CM03105 fallthrough; it is reported
as FALLTHROUGH. All channel values, events, SUPER, IRQ and restart light are
retained. MODE is untouched because the basic DMP source never reads it.

The independent oracle freshly assembles both original programs, checks each
rope against the pinned source fixture and links no tested native body.
All8192 complete original LM+CM states match strict native C and the full
ASan/UBSan stream: all2048 image words plus SUPER, IRQ, channels11/12/13,
events, restart and route. Full snapshot SHA256:
`e0f03b64f369d03041be2d9d5026e4e5c35ab976c8eca1fde315f969690c8ac9`.
The corpus covers all eight source entries, both zero/sign/extrema classes,
all32 routine codes, raw register overflow encodings, normal/mixed selection,
all EBANKs/selecting SUPER forms and retained unused fields. Combinations
are sampled rather than asserted as a Cartesian product. Frozen64 complete
source states and16 atomic guards pass strict host and sanitizer checks.

Separate actual C64 PAL VICE diagnostics pass32 full source states per mission.
LM mailbox is a500002000, PRG10685 bytes, loaded10683 bytes, hardware/software
stack extents20/152. CM mailbox is a500002001, PRG9829 bytes, loaded9827 bytes,
stack extents20/145. Both use below-C000 diagnostic memory and no added
zero-page storage. These linked component checks do not establish application
disk-overlay integration. Exact source/dependency hashes, original fixture
listings/audits, complete compressed streams, logs, PRG/maps and raw stack/
mailbox evidence are retained. `tools/dsky_input_angle.mk` provides host-only
all/test/sanitize aliases and explicit target/differential/freeze commands.
Time input scaling, final load-verb stores and deferred lifecycle, channel
output and uninterrupted keyboard/application composition remain tracked work.
