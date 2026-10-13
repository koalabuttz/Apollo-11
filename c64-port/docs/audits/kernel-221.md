# Certificate review: Luminary099/SERVICE_ROUTINES.agc after Kernel PR #221

Reviewed 2026-10-13 by the certificate reviewer at `port/c64-native-c` 3e9e4e04,
against the `service-transfer` certificate. The earlier review
(docs/audits/kernel-212.md) accepted Comanche055/SERVICE_ROUTINES.agc and sent the
Luminary file back for text only: the certificate and docs/SERVICE-TRANSFER.md
said 171 site words while the table holds 175, and the two `TC GENTRAN +1`
callers were not described. Every behavioural finding (B2, B3, B4) was already
fixed and reproduced in that review.

Verdict: **accepted**.

## What was checked

- docs/SERVICE-TRANSFER.md now gives 175 words for `src/lm_service_sites.inc`
  (152 CM) and describes the two `TC GENTRAN +1` sites at 20,3574 and 20,3615.
  `tools/agc_sites_generate.py lm-service --check` reports 175 sites.
- The certificate's supported_domain names GENTRAN +1 as an entry and its
  external continuations name GENTRAN's `TCF Q+2` return.
- The reproduction recorded in kernel-212.md stands: service-transfer
  1,646 cases and 1,658 checks with 0 failures on host and sanitizer, the
  differential identical to yaAGC, VICE 569 checks equal to vice_targets, and
  no host-only arc in an independent `gcc -O0 --coverage` run. `src/service_transfer.c`
  and the site tables have not changed since.
- `cert_precheck.py` reports 0 FAIL at this tip.

## Note for the lane (not blocking)

- The text says the two GENTRAN +1 callers run "AOSTASK running inside T3RUPT"
  and skip GENTRAN's `INHINT`. The calls sit in 1/ACCS after its own `INHINT`;
  the effect is the same, but reword it at the next touch.
