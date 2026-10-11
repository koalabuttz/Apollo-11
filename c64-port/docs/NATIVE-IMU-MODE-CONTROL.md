# Native IMU initialization and PIPA stages

`imu_mode_control` translates 142 original local Luminary words and 141
Comanche words from `IMU_MODE_SWITCHING_ROUTINES.agc`. These compiled stages
cover ZEROICDU/NOATTOFF, IMUZERO/IMUFINE/IMUCOARS admissions, zero/fine task
continuations, IFAILOK/IPIPAOK/IMUPULSE/PIPASR and their local warning/alarm
returns. This is a partial containing-file milestone.

The canonical 2048-word image remains authoritative. Required inputs are
ordinary 15-bit RAM and raw 16-bit A/L, fixed Q02000..07777, coherent bank07/EB3,
ordinary channel11/12/14, IRQ0/1 and source SUPER bits. OPT counters035/036 must
be +0; gyro047 and CDU050..052 admit either signed zero. Channel14 gyro/coarse
activity071000 must be clear. The caller excludes all relevant producers and
owns these source states throughout each call. Invalid inputs return unchanged.
The API selects a compiled source stage; it never interprets an opcode stream.

| Stage | LM entry | CM entry | Actual selected boundary |
| --- | --- | --- | --- |
| ZERO_CDU | 05457 | 05521 | TC Q, Q3 |
| NO_ATT | 03266 | 03151 | TC Q, Q3 |
| ZERO2 | 02757 | 02642 | VARDELAY or IMUBAD |
| ZERO3 | 02766 | 02651 | IBNKCALL SETISSW or IMUBAD |
| FAIL_OK | 03234 | 03117 | IBNKCALL SETISSW or TASKOVER |
| PIPA_OK | 03254 | 03137 | IBNKCALL SETISSW or TASKOVER |
| PIPA_USE | 03273 | 03156 | IBNKCALL SETISSW or SWRETURN |
| PIPA_FREE | 03310 | 03173 | MODEEXIT or ALARM0212 |
| FINE | 03210 | 03073 | WAITLIST or MODEEXIT |
| ZERO | 02714 | 02577 | WAITLIST, MODEEXIT or ALARM0206 |
| ZERO_REGISTERED | 02747 | 02632 | MODEEXIT or ALARM0210 |
| FINE_REGISTERED | 03225 | 03110 | WAITLIST |
| FINE_DONE | 03231 | 03114 | MODEEXIT |
| WARNING_ZERO_DONE | 02777 | 02662 | ENDIMU |
| WARNING_FAIL_DONE | 03253 | 03136 | TASKOVER |
| WARNING_PIPA_DONE | 03307 | 03172 | MODEEXIT |
| PIPA_ALARM_DONE | 03321 | 03204 | IBNKCALL SETISSW |
| COARSE_START | 03000 | 02663 | WAITLIST or MODEEXIT |
| COARSE_REGISTERED | 03007 | 02672 | MODEEXIT |
| ZERO_ALARM_DONE | 02723 | 02606 | MODEEXIT |

Each return retains the actual source PC/Q, banks, A/L, IRQ and shared-memory
changes. MODEEXIT itself, external delay/WAITLIST/alarm/warning bodies and
scheduled jobs are not executed by this packet. COARSE_START invokes the
unchanged, independently proved native SETCOARS body under its already valid
quiet-counter domain before selecting the original WAITLIST entry. Internal C
returns introduce no scheduler checks.

ZERO_CDU zeros only CDUX/Y/Z. NO_ATT clears the source lamp bits and adds the
sign-extended BIT15 through the original AD/ADS arithmetic. Caged branches keep
the source CAGETSTQ/CAGETSTJ distinctions, including IMUCADR negative zero.
PIPA_USE stores negative zero into all three PIPA counters before testing the
cage latch. PIPA_FREE follows the literal BIT10-set branch to MODEEXIT; the
BIT10-clear branch enters ALARM0212 regardless of the source comment wording.
ZERO_REGISTERED retains its literal BIT9 alarm selection. Warning continuations
remain source frontiers with their inline CADR or 2CADR words intact.

`imu_mode_literal` exposes 15 LM and 14 CM original local passive fixed words:
three alarm numbers, four WAITLIST 2CADR pairs, three warning CADRs, and the LM
BITS4-5 word. The CM BITS4-5 symbol is an external alias and emits no local word.
All 65,536 accessor addresses per mission are compared with the original ledger;
unowned addresses and invalid mission/null output leave the output unchanged.
The private table is reproducible from `tools/imu_mode_control_source.py`.

The unchanged original fixtures, declarations and disclosed CM indentation-only
repairs remain pinned by `evidence/imu-pulse-source-audit.json`. The independent
host-only GPL source engine executes original instruction words and is absent
from the target binaries. No native expected values are supplied to that oracle.

Verification covers 65,536 constructor states per mission/stage, 2,621,440 total.
Each row includes identity, IRQ/SUPER, channel11/12/14, status and all 2048 RAM
words. Complete original, strict native and full ASAN/UBSAN streams match byte
for byte. Actual mode/latch inputs, raw A/L, signed-zero idle counters and
nonzero unowned-word patterns vary; targeted cage, alarm, warning and admission
branches are retained. This is a constructor-cohort result, not an exhaustive
Cartesian claim. Each original stream is retained once as deterministic XZ;
native and sanitized outputs are compared directly to its decoded bytes.

Forty stock PAL C64 diagnostics check 1,280 complete source-frozen states and
480 unchanged-state guard calls. Their actual PRGs, maps, mailbox and measured
hardware/software stack dumps are retained. Compiler ZP allocation is zero and
all allocated sections end below C000. Constant coverage is the independent host
accessor sweep, not a claim that the target diagnostics exhaust its arguments.
Reproduction uses `tools/imu_mode_control.mk` all/test/sanitize/check-host and
mission/stage vice-test targets. A clean source proof regenerates separate build
archives; retained evidence objects are not overwritten.

The additive gap ledger retains prior frozen packet ownership and adds this
word map. SENDPULS/CA+ECE transport, stall/attitude admission, R02 and standby
local groups remain open. These stage proofs do not establish asynchronous
coarse pulse delivery, arbitrary active-counter operation or either complete
mission application.
