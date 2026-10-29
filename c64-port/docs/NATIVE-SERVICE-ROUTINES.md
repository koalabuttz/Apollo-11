The native `service_routines` module maps the complete byte-original
Luminary099 and Comanche055 `SERVICE_ROUTINES.agc` bodies: 96 and 123 emitted
words respectively, including all three passive data words per mission and
the OCT7 alias and placement directives. Fresh original files assemble without
repairs into the unchanged original-layout LM `1f5326e0...` and CM `6e3150ae...`
ropes. The source audit records every noncomment source line and emitted word.
Canonical physical erasable storage is the only authoritative RAM image.

| Original body | Native entry or driver | Exact endpoint |
| --- | --- | --- |
| UPENT2 / DOWNENT2 | Mission-qualified selectors | Real caller+1; LM Q+1 indexed helper, CM INCR Q / TC Q |
| UPFLAG / DOWNFLAG, DEBIT / COMFLAG | Generic named-bit selectors | Real TC L return, PC=caller+1 / Q=2 |
| DELAYJOB / DELLOOP / OK2DELAY | Slot scan | Actual DLY2-1 WAITLIST entry or alarm/error continuation |
| TCGETCAD | Original MAKECADR and save | JOBSLEEP; overflowing save falls into actual WAKER and JOBWAKE |
| WAKER | Original clear/read exchange | JOBWAKE, with exact return descriptor |
| WAKER's returned continuation | WAKER_FINISH | Actual TASKOVER entry |
| GENTRAN / GENTRAN+1 | One source iteration or block driver | Genuine +1 back edge, true Q+2 return, or fixed B5OFF fallthrough |
| B5OFF | Selector | Actual ENDOFJOB entry |
| CM TRFAILOF / TRFAILON / REQ | CM-only selectors | Actual caller return after original DSPTAB/OPTMODES changes |
| OCT1104, TCSLEEP, WAKECAD | `service_routines_literal` | Original immutable words, with unsupported-output preservation |

CM UPENT2 initially stores A in L; raw overflow skips the low-three-bit MASK
and uses source signed one for the subsequent indexed flag address. LM instead
always changes FLAGWRD4 and uses the source Q+1 helper. Generic UPFLAG and
DOWNFLAG resolve the named descriptor through the real caller's inline data,
retain source DV-by-15 quotient/remainder in ITEMP1/2, exchange the previous
flag word into ITEMP3, enable interrupts, and return through the actual TC L
sequence. Fixed literals come from caller-owned immutable source data;
erasable literals are reread from the canonical image after the source stores.
This implementation leaves all older bounded flag and VAC5 transfer APIs
unchanged; callers explicitly select the new mission-qualified API.

DELAYJOB scans the original three LM or four CM slot locations, treating both
signed zeros as available. Positive-overflow TS Q skips CAF and starts from
source index one. Negative overflow starts from DELAYLOC-1; a zero there takes
the original success path. The nonzero LM path calls BAILOUT1 without the
preceding DXCH, and the CM path reaches the literal 01104 as its original fixed
TC descriptor, stopping at the external PC01104 before that caller body
executes. These unusual paths are retained rather than reported as ordinary
successful delays. Full scheduler, alarm, sleep and wake bodies are external
compiled continuations, not newly claimed by this file certificate.

GENTRAN performs the source's descending copy, not a host memmove. TS ITEMP1
precedes the source inline read, and the destination inline read follows the
source load. The final CCS rereads actual ITEMP1, including a word copied over
the count. Positive count takes the genuine GENTRAN+1 back edge; positive zero
executes the true Q+2 helper. A negative overwritten count falls into CS BIT5
and B5OFF; negative zero skips that CS and masks with the diminished-absolute
zero left by CCS. Source/destination overlap, scratch aliases, banked ordinary
RAM, and passive fixed words therefore retain the actual source behavior.

`service_routines_block` follows only this one compiled back edge. It can
complete a whole transfer or return LOOP at that exact boundary when a caller
budget ends. No Executive or IRQ poll is added, and the caller retains producer
exclusion until the original source returns or explicitly hands off. A missing
passive fixed-data resolver fails before that iteration changes RAM. BAD after
prior completed iterations leaves their real source state; it is not a
transactional rollback or a manufactured successful source return. The fixed
reader sees only a passive physical data-word address, never an instruction
fetch or an opcode to interpret. All banks, including real channel-7
superbank forms, use the existing original passive-storage address convention.

The supported typed domain uses ordinary 15-bit RAM and raw 16-bit A/L,
ZERO=0, coherent EB/FB/BB, channel-7 mask0160 and IRQ0/1. Compiled caller Q is
0100..07775. Named descriptors use nonnegative source DV input and an effective
ordinary erasable target; the declared 14,459 upper bound is the last encoded
bit whose logical register remains <=01777. GENTRAN's N-1 is ordinary
nonnegative <=01777, effective CA/LXCH address forms are valid, and its RAM data
and inline words map to ordinary physical memory >=030. CPU control and
editing-register operands are outside this ordinary data-transfer API.
TCGETCAD's RUPTREG1 is an original allocated slot index; WAKER's BB is in its
original bank0 and therefore 0..7. Unsupported entry inputs leave the whole
image and external latches unchanged. Context products and transfer sizes are
sampled, not an exhaustive enumeration of all possible mission images.

Fresh original/native execution and full ASAN/UBSAN agree on 106,496 complete
2,048-word states plus SUPER/IRQ. That includes 98,304 source-body/iteration
cases and 8,192 complete overlapping/banked GENTRAN calls of 1..64 words. Fixed
caller descriptors and fixed source words are independently checked against
the original ropes. The canonical ordered decoded SHA256 is
`860235bc19064c6d945c0afff223ce2e9cc9e5a2cfa4df273cdb6fc010877a8f`.
Host accessor tests check all 65,536 arguments for each mission and preserve
unsupported outputs. Invalid-input guards, missing passive-resolver preflight,
and whole versus budgeted transfer continuation also pass sanitizer checks.

Stock-ROM PAL C64 executes 416 selected original states and compares every
physical word and both latches individually. Lossless sparse fixtures preserve
all untouched constructor words; no hash replaces comparison. The standalone
diagnostic contains a separate expected comparison object, not another mission
image. Its retained PRG/map and actual RAM bounds describe that diagnostic,
not a production overlay allocation or a unified application loader. The
evidence ledger retains the complete compressed source stream, selected source
states, logs, target artifacts and independent reviews. Quiesced external
producers keep those source boundaries stable during proof.

Reproduce with `make -f tools/service_routines_port.mk test sanitize`,
`check-host`, and `vice-test` using the configured MOS compiler, VICE and ROM
paths. The GPL Virtual AGC engine is an independent host oracle only. Native
code and C64 targets contain no engine copy, instruction decoder or runtime AGC
emulator. Complete qualification of these source files does not assert that
both full flight applications or their external service bodies are finished.
