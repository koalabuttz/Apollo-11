# Native operator scheduler request call stages

`dsky_request_control` and `cm_request_control` translate VBRQEXEC41,3466,
REQEX1 at3473, priority store3476, REQUESTC3477, SETVAC3510,
VBRQWAIT3512, ENDRQWT3515 and source return3500 in both original programs.
The prefixes preserve the actual TC NOVAC/FINDVAC/WAITLIST word in MPAC,
priority or delay in MPAC+4, and actual RELDSP call state with Q03500.
The source bit1 selects FINDVAC versus NOVAC; executive requests clear it
from the source priority and wait requests retain the full ordinary delay.

After the independently owned RELDSP body returns at3500, the continuation
stores literal TC ENDOFJOB in MPAC+3, performs the source double-word load
and exchange from DSPTEM1+1 into MPAC+1/+2, inhibits IRQ and reaches the
actual service with Q0155. LM NOVAC/FINDVAC/WAITLIST entries are05072,
05105 and05203; CM entries05134,05147 and05245. Full incoming MPAC+2
becomes source L debris. The canonical source words are preserved; native
compiled service selection never fetches or decodes AGC instructions.

These are genuine call-stage boundaries. RELDSP, admission, WAITLIST,
source TC ENDOFJOB and exceptional recovery are separate continuations.
Invalid priorities/delays are preserved at the actual pending service entry,
without imposing the separately bounded scheduler's input domain or
inventing a successful return. Caller exclusion remains held throughout.

Inputs accept canonical ordinary image words, raw16 CPU, bank41, all eight
retained EB values, selecting SUPER100/120/140/160 and IRQ0/1. REQEX1
accepts source-produced literal NOVAC/FINDVAC words in A; direct priority
stores3476/3515 accept sign-extended ordinary A, matching their actual
producers and excluding overflow skip into the unrelated continuation.
Source3500 accepts the three actual literal service words in MPAC. Other
prefix entries accept unrelated opaque MPAC/A where unused. Unsupported
inputs leave complete image, external I/O and route unchanged.

8192 original/native/fullASan+UBSan complete2053-field checkpoints match:
all2048 words, SUPER, IRQ, channel11, its source event and selected route.
Full SHA256:
`646ed265dfd397ae3dbebcad3499e0643eb7fa57e99058e8cf0af1a4514efd20`.
Each mission independently assembles eight unchanged audited source files
with byte-exact pinned rope; the original oracle links no native body.
The corpus covers every actual entry and service route, all EB/SUPER/IRQ,
raw CPU overflow and signed-zero forms, full ordinary request words,
both FINDVAC-selector bits and actual double-word exchange debris.

Sixty-four frozen complete checkpoints pass strict host, sanitizers and
stock-ROM PAL C64 VICE. Mailbox a500004002; PRG6144, loaded6142 bytes;
hardware/software stack sentinel extents14/43. Eight atomic guard cases
also pass host checks.
Portable host aliases and explicit target are in tools/dsky_request_control.mk.
The frozen diagnostic uses two images solely for complete comparison;
production uses one canonical source image.
