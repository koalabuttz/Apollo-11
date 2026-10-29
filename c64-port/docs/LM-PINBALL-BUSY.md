# LM Pinball busy display

`lm_pinball_busy` translates actual NVSUBUSY04442 through POSTJUMP to
NVSUBSY1. It preserves the original wake CADR and bank/SUPER effects, sets
DSPLIST and channel11 when free, and returns at actual JOBSLEEP05133. An
occupied CADRSTOR or DSPLIST returns at actual POODOO05652 with alarm01206;
it does not pretend that recovery has run. The canonical image and explicit
SUPER/IRQ/channel11 are the only state. The caller owns producer exclusion.
Unsupported inputs preserve all owned objects.

The header defines the ordinary source-domain conditions. DSPLIST uses +0
for free; its -0 CCS path reaches the inline OCT01206 word as an instruction
and is excluded. CADRSTOR accepts both signed zeros. This is a basic service,
so it does not introduce an interpretive pending-job poll.

`tools/lm_busy_differential.py` verifies 8,192 complete original Luminary099
2,051-word output states, including all EB/FB/SUPER values, signed zeros,
raw wake CADRs and both alarm branches. SHA256:
`3cb4e414f2622d2265a267605e2e5e8791a0851125a25865a29acc820b365bcc`.
Strict checks cover64 frozen states and13 unchanged invalid boundaries;
all source states also agree under ASAN/UBSAN. C64 PAL VICE passes all64:
mailbox `a500004000`, PRG4,410 bytes, hardware stack14 bytes plus14 ROM
bytes, software stack13 bytes and no extra zero page. Recipes are in
`tools/lm_busy.mk`. Native code contains no AGC instruction decoder or CPU
emulator. Full JOBSLEEP and alarm recovery are separate source services.
