# Native major-mode operator requests

`dsky_major_request` and `cm_major_request` implement MMCHANG41,3430,
its operator return at3431, and REQMM41,3452 against each mission's
original source. Request entries blank the noun digits with real BANKCALL,
2BLANK and SWRETURN, turn on the source channel11 flash event, set decimal
input, and reach actual ENDINST, NVSUBEND or PASTEVB selected by ENTRET.
MMCHANG owns return3431; REQMM preserves its ordinary source caller in REQRET.

Return3431 performs the source DSPCOUNT test, including raw register signed
zeros and overflow. An incomplete entry reaches actual ALMCYCLE with its
source Q and A. A complete entry captures the full ordinary NOUNREG in MPAC,
clears the noun, blanks it, resets DSPCOUNT and performs literal POSTJUMP to
actual V37. The exact source handoffs are LM physicalbank04, PC02040,
FB010000, Q040 and CM physicalbank04, PC02011, FB010000, Q011. V37 and
Fresh-start are independently owned program continuations; this milestone
preserves their actual entry states and does not claim their bodies.

The domain is canonical ordinary image words, raw16 A/L/Q, physicalbank41,
any retained EB, SUPER100/120/140/160 and IRQ0/1. REQMM takes ordinary
Q0100..7777; initial MMCHANG Q is unused and accepts raw register values.
ENTRET must be the source ENDINST/NVSBENDL/MONBACK value only on request
paths where it is used. Return3431 accepts unrelated opaque ENTRET.
Both selected 2BLANK entries accept clean positive-zero relay words and
preserve original NOUT INCR and source signed-zero behavior. Unsupported
inputs preserve the image, external I/O and route. The caller holds restoring
producer exclusion; no scheduler poll or native placeholder callback occurs.

8192 complete2053-field original/native/fullASan+UBSan checkpoints agree:
all2048 source words, SUPER, IRQ, channel11, actual channel11 event and route.
Full SHA256:
`65785759df26bb88ec3b0493d9f89146349f25ecd976257a9cf22b428eb97d53`.
Each original program is independently assembled from eight audited source
files and a byte-exact pinned rope. The original oracle never links the
native body. Its banked checkpoints select the real physical program bank.
Coverage includes all entry/route pairs, all retained EB/SUPER/IRQ values,
both zeros and raw CPU overflow forms, full ordinary NOUNREG signs,
source DSPCOUNT completion boundaries, NOUT edges, selected clean-zero
relay and unused opaque return metadata.

Sixty-four frozen complete checkpoints pass strict host, sanitizers and
stock-ROM PAL C64 VICE. Mailbox a500004002; PRG8352, loaded8350 bytes;
hardware/software stack sentinel extents16/50. The diagnostic holds two
canonical images and frozen expected changes; production owns one image.
Portable host aliases and the explicit target are in tools/dsky_major_request.mk.

The existing committed additive audit
`evidence/phase-alarm-request-raw-cleanup-20261012.json` records verified
inactive proof reclamation. Active major-request inputs and every retained
source/evidence object remain intact.
