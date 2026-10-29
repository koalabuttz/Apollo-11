# LM Executive superbank transfers

`lm_super_dispatch` implements the complete fixed SUPDXCHZ body at 05165 and its alternate +1 entry at 05166. The first entry exchanges A/L; the second accepts bank/location already in A/L. Both perform the original SUPERBNK write, BBANK store and TC L, returning the actual selected source pair as explicit native metadata. Source Q becomes 2. They preserve interrupt enable and every other canonical word.

The native arguments are normal sign-extended AGC registers with a valid BBCON and source location 0100..07777. There is no source-opcode decoder, native callback, scheduler poll or invented lifecycle step. The transferred program remains a separate source continuation. NEG100 is unchanged original constant data in the source; these two entries do not read it.

The independent unchanged assembly engine verifies 16,384 whole 2048-word images plus SUPER/IRQ, covering both entries, every fixed bank/erasable bank/SUPER combination, interrupts enabled and masked, and arbitrary incoming Q. Strict and full ASAN/UBSAN native outputs match the complete original streams. Metadata checks independently verify all 16,384 selected pairs. Thirteen mutation guards and three null interfaces preserve all canonical words, I/O and output metadata on rejection.

The stock PAL ROM/D81 target checks 64 frozen complete original-state fingerprints through 32 input/checker modules. Frames return before each subsequent disk load. The image stays at $7000, fixture metadata is bounded to $c000..$c03f, and native I/$01 is restored. The exact measured stack/mailbox results are retained in `evidence/lm-super-dispatch-proof.json`.

Run `lm-super-dispatch-port-test` or `lm-super-dispatch-port-sanitize` through `tools/lm_super_dispatch_port_integration.mk` for a fresh-checkout replay using retained originals, private host builds and checked target descriptors. These source entries are complete within their explicit normal-address contract; the packet does not certify the remaining Executive file or execute target programs.
