# Geometry (CSM_GEOMETRY + LEM_GEOMETRY) after #585

Second certificate reviewer, 2026-10-17, reviewed head e98e2450.

**Verdict: accepted and re-cited.** The previous record is
[rev2-recite-1014b.md](rev2-recite-1014b.md).

## Code

#585 changes only the geometry spec in `tools/interp_specs.py`: the SXTLOGIC
pin for 0115 (INTBIT15) is now 040000, the value flight holds. Before, the pin
excluded a value flight can hold, even though it had no effect at runtime. The
pins for the one SXTLOGIC caller are LOC 23,2004, EBANK 046000 plus 0..7, and
EDOP 0171. No C source changed.

## Evidence

- Differential: PASS, 76,800 geometry cases equal the original programs
  (150 routines).
- Host log: 665 checks pass.
- VICE log: 8 PRGs, 196 checks, 0 failures. sha256 5a3926c7…2ed6 matches
  SHA256.json and the release asset.
- Source cover 1,594/1,594. `ledgers.py --validate` and `cert_precheck.py`
  pass for both files (0 FAIL; vice-manifest WARN only because the proxy
  blocks the manifest fetch).

## Notes

The refusal-site NOTEs (67 LM, 115 CM) are unchanged from the previous record.
