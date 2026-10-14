# Certificate withdrawn: Comanche055/SYSTEM_TEST_STANDARD_LEAD_INS.agc

Withdrawn 2026-10-14 by the certificate reviewer, at `port/c64-native-c`
871702f1. The file was accepted in docs/audits/kernel-212.md.

The port auditor's issue #318 shows that CM E/JOBWAK refuses a normal flight
state. The certificate is therefore returned to pending until the Kernel lane
fixes it.

## The defect

`cm_basic_wake` (`src/cm_basic_wake.c:6-7`), which `standard_lead_wake.c:13-14`
calls, refuses whenever core set 0's PRIORITY is negative or −0. Its search
also starts at core set 1.

Core set 0 sleeping is the ordinary idle state: JOBSLP1 leaves PRIORITY
negative, and ENDJOB1 leaves it −0 (EXECUTIVE.agc:277-280 and 368-371).
JOBWAKE scans from LOCCTR=0 (EXECUTIVE.agc:304-351).

With a fresh seed (0x7e93, SLO_PER_KIND 256):
- CM E/JOBWAK reaches only 66 of 256 cases, against 254 for LM.
- All 190 refusals have core set 0 asleep.

Under PORT_SCOPE, refusing a reachable flight state blocks a certificate.

The LM form (`lm_basic_wake`) already loops from 0 and ports SPECTEST, so
Luminary099/SYSTEM_TEST_STANDARD_LEAD_INS.agc stays complete.

## What the lane must do

- Port the LM fix to CM. Remove the guard, start the search at core set 0, and
  port SPECTEST, which six cases need when core set 0 itself matches.
- Refreeze standard-lead-oracle.
- Check the other `cm_basic_wake` callers: `imu_pulse_end.c`,
  `dsky_load_control.c` and `cm_clock_display_builtin.c`.
- Ping the certificate reviewer for re-review.
