# P66 display sleep and resume control

`p66_makeplay_sleep_enter` follows actual GOSLEEPS2754 through UPENT2 to
JOBWAKE5137. It sets normal wait bit01000 and searches for the original
PLAYJUM1 waiting CADR020615. `p66_makeplay_sleep_return` follows wake caller2770
through XCHTOEND, replacing an older found waiting display's return with
ENDINST, then reaches actual JOBSLEEP5133 for the current display.
`p66_makeplay_sleep_resume` follows real PLAYJUM1 wake2615 to GOPLAY3070.
`p66_makeplay_busy` follows busy NVSUB caller3121 through REST/RESTSLEP to
NVSUBUSY4442 or the actual ENDOFJOB5155 entrance.

The supported original normal P66 V06N60 context is defined in the header.
The canonical image and explicit SUPER/IRQ are authoritative. Unsupported
inputs leave both objects unchanged; the caller owns exclusion. Wake/sleep,
Pinball admission and actual job completion remain separate native service
edges. The control bodies preserve those edges rather than pretending a
sleep finishes synchronously. This milestone does not certify all display
interface entries.

`tools/makeplay_sleep_differential.py` compares 16,384 complete original
Luminary099 2,050-word states across all four boundaries. Original display,
service and bank source instructions are checked against the fixture; only
an unused PRIOSAVE alias declaration is omitted. Snapshot SHA256:
`e6a0591338811835b2788e312d8caf39aa2e0bb028a5494b00235c63ed2b5f1b`.
Strict tests cover 64 frozen complete images and52 invalid boundaries; all
source cases agree under ASAN/UBSAN. C64 PAL VICE passes all64 images:
mailbox `a500004000`, PRG4,812 bytes, no extra zero page, measured hardware
stack14 bytes (plus14 ROM bytes) and software stack15 bytes.

Standalone recipes are in `tools/makeplay_sleep.mk`. Native code executes
fixed translated source paths and contains no AGC instruction decoder.
