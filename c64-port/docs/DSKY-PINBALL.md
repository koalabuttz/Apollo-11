# Port original DSKY relay character and blanking services

Native DSPIN,11DSPIN and2BLANK match all 4,096 complete 2,048-word original-source images plus SUPER/IRQ, SHA 65097a2bedb65f7a5ebf81f42b101107b2e51ced4ae5aed81fd510e05bef0180. The entry domain includes all eight retained erasable banks, ordinary caller returns and source character/sign/blank counts; original clean-positive-zero CCS holes remain explicitly excluded. Strict and sanitizer checks and 64 actual C64 source images pass. The 9,859-byte PRG uses zero additional zero page and 10 hardware / 9 software stack bytes. Earlier EB2-only proof artifacts are retained.

These bounded services do not certify complete program coverage. The original
Luminary and Comanche source trees remain unchanged. Public headers state each
entry domain and producer exclusions; evidence includes original-source states,
host and sanitizer results, target PRGs, maps, mailboxes and stack measurements.

Reproduce the source comparison with `python3 tools/dsky_pinball_differential.py deps/fixture`.
