# Second review: re-cite 4 CM flight specs after #515

Second certificate reviewer, 2026-10-15. Lane G's #515 (merged as 8d5cb281) added `trusted_image`, `index_carry16` and `image_w` to cm-entry-dap (CM_ENTRY_DIGITAL_AUTOPILOT.agc), cm-rcs-dap (RCS-CSM_DIGITAL_AUTOPILOT.agc), cm-servicer (SERVICER207.agc) and cm-body-attitude (CM_BODY_ATTITUDE.agc). The first two were complete on docs/audits/lane-g-258.md, the last two on lane-g-107.md. **Verdict: all four re-cited.**

## Code

- **Flags.** The same changes accepted for cm-p30-p37 (rev2-recite-p30-p33.md): the image scans in `sane()` / `basic_sane()` sit under `!RK_FIXED_IMAGE`, `ixw` has an exact end-around carry, and `W` comes from agc_image_w.h.
- **Certificates.** Only hashes change, plus cm-servicer's vice_targets 123 to 122: #515 regrouped its C64 rows (tools/mk/cm-servicer.groups), and the cover check below still takes every host arc.
- **Case sets.** Unchanged: 7,168 / 4,352 / 20,480 / 17,920 cases, the same as the parent.

## Evidence at 8d5cb281 (reproduced)

| Check | entry-dap | rcs-dap | servicer | body-attitude |
|---|---|---|---|---|
| Host + sanitizer checks | 224 | 136 | 160 | 560 |
| VICE checks (= vice_targets, 0 failures) | 79 | 43 | 122 | 97 |
| `--cover-check` arcs, 0 missing | 913 | 719 | 989 | 811 |

`cert_selfcheck` OK on all four. `cert_precheck` 0 FAIL; the NEWJOB WARNs at entry/rcs line 46 predate #515.

**NOTE (PRG drift).** 0 of 27 PRGs match the logs at the merge and 27 of 27 at the PR commit 5da987f25. The merge brought in agc_transfer.c from 96d58f7e1 (agc_transfer_next), a shared runtime change. The merged PRGs pass VICE with identical counts, so under the steward's PRG-drift ruling this is a NOTE.
