# LM basic priority change

`lm_basic_priority_change` translates the original PRIOCHNG entry at 05146
through the actual basic DTCB dispatch. Its input is the canonical 2,048-word
image, explicit SUPER/IRQ state and a caller-owned next-address result. It
uses the native Executive priority service and preserves source CPU registers,
scan scratch, banks and selected context. The header states the supported
active-core and basic-context domain; unsupported inputs leave all objects
unchanged. The caller owns exclusion.

`tools/lm_priority_differential.py` compares 8,192 complete original Luminary099
2,050-word states, including raw A/L/Q, all erasable words, SUPER and IRQ.
Coverage includes all initial EB/FB/SUPER values, dormant/free/active slots,
priority ties and caller register signed zeros. Snapshot SHA256:
`122aad0dbd56103883bb071825718a19f3a25b54e373616bee474e55b96a58d3`.
Strict tests cover 64 frozen complete images and 16 invalid boundaries; all
8,192 source states also agree under ASAN/UBSAN.

C64 PAL VICE passes all 64 frozen images: mailbox `a500004000`, PRG 14,278
bytes, no extra zero page, measured hardware stack 24 bytes (plus 14 ROM
bytes) and software stack 698 bytes. An earlier target fixture used an
unsigned-int generator expression whose multiplication overflowed on the
16-bit target. The generator now explicitly uses uint32_t; the host/source
corpus is unchanged. The earlier fixture, debug program and difference
records remain under `build/lm-priority-original-input*` and
`build/lm-priority-debug*`.

Standalone recipes are in `tools/lm_priority.mk`. Native code contains no
AGC instruction decoder or emulated CPU. Original assembly is used only by
the host reference runner.
