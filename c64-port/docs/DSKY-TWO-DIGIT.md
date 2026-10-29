# Port original DSKY two-digit noun-verb formatter

Native DSPDECVN matches all 4,096 complete 2,048-word source images plus SUPER/IRQ, SHA 284719c9194b0c106ff4c0e79ed0000adbb28450d928f9297c606afb51fdc924. Original SHORTMP consumes retained MPAC+2, formats values0..99 into adjacent source display characters and returns through the actual caller, with all eight retained erasable banks covered. Strict and sanitizer checks and 64 actual C64 source images pass. The 13,289-byte PRG uses zero additional zero page and 13 hardware / 40 software stack bytes.

These bounded services do not certify complete program coverage. The original
Luminary and Comanche source trees remain unchanged. Public headers state each
entry domain and producer exclusions; evidence includes original-source states,
host and sanitizer results, target PRGs, maps, mailboxes and stack measurements.

Reproduce the source comparison with `python3 tools/dsky_two_digit_differential.py deps/fixture`.
