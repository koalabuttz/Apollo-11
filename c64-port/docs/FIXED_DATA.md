# Original immutable data on the native C64

fixed_data.c supplies **1,524 LM words and 1,347 CM words**, as immutable native
uint16_t arrays with historical fixed-word addresses. Each word is the original
15-bit data value, including negative zero. No source instruction decoder,
AGC engine, heap allocation, or mutable mission storage is involved.
The original public-domain declarations generate the arrays; the MIT runtime
only looks up immutable data.

Complete constant-data files:

| Program | Original file | Native words |
|---|---|---:|
| LM | CONTROLLED_CONSTANTS.agc | 463 |
| LM | FIXED_FIXED_CONSTANT_POOL.agc | 96 |
| LM | INTERPRETIVE_CONSTANT.agc | 35 |
| LM | DOWNLINK_LISTS.agc | 242 |
| LM | RESTART_TABLES.agc | 132 |
| CM | FIXED_FIXED_CONSTANT_POOL.agc | 94 |
| CM | INTERPRETIVE_CONSTANTS.agc | 35 |
| CM | STAR_TABLES.agc | 223 |
| CM | DOWNLINK_LISTS.agc | 226 |

Both programs additionally provide all six Pinball table sections. NNADTAB
and NNTYPTAB contain 100 words each; IDADDTAB contains 180; RUTMXTAB contains
60. SFINTAB and **SFOUTAB** contain 58 words each for LM and 40 for CM.
SFOUTAB is the actual source spelling. The CM RESTART_TABLES prefix adds 249
data words before executable NEWPHASE. Reader code in the Pinball file and
NEWPHASE in the CM restart file are outside this data milestone.

SIZETAB uses twelve TC assembler pseudo-ops to encode numeric offset
expressions. These are data entries in the documented restart-table format.
The native arrays store their resulting numbers and never execute those
source words. Other restart descriptor words preserve original priorities,
times, and historical address pairs. Calling the corresponding native job
still requires the separately reviewed native continuation registry.

include/fixed_data.h exposes mission-selectable fixed-word and noun-table
reads. A fixed address is historical_bank * 1024 + within_bank_offset, with
the fixed-fixed banks naturally at addresses 04000..07777. Unsupported
missions, tables, addresses, indices, or null output pointers return false;
valid output objects remain unchanged on failure. Reads access only certified
data spans, so adjacent executable reader words are unavailable.

~~~c
uint16_t word;
if (apollo_noun_table_read(APOLLO_FIXED_LM, APOLLO_NNADTAB, 60, &word)) {
    /* word == 024074; interpretation and fresh erasable reads belong to Pinball. */
}
~~~

fixed_data_symbols.h supplies compile-time original label and alias bindings.
FIXED_WORD and ERASABLE_WORD names are physical source-word indices;
ASSEMBLER_VALUE names are original assembler integers, including register
aliases, and do not imply one's-complement data encoding. Punctuation in source
names is encoded with its hexadecimal character value to prevent collisions.
The generated bindings allocate no memory. Declaration/layout records retain
their source lines and resolved symbol kinds in the audit.

Compile with APOLLO_FIXED_LM_ONLY or APOLLO_FIXED_CM_ONLY to include one
program. The immutable arrays and lookup function can be linked into a native
overlay; callers must finish before unloading that module. They do not create
a second copy of authoritative erasable RAM. Native consumers continue to
read current inputs from the canonical shared source-word image.

The assembly audit records the exact originals overlaid into existing pinned
support fixtures, filename aliases, indentation-only repairs, every data
source line, emitted word, historical address, resolved label, and source
fingerprint. Every declared data-emitting line in each complete file is mapped.
Placement, alias, comment, and mixed executable files are inventoried separately.
fixed-data-certificates.json recommends the nine complete data-file mappings
for the parent's shared inventory; it is not a whole-application certificate.

Two CM original-versus-pinned layout distinctions remain explicit:

* ERASABLE_ASSIGNMENTS line2094 contains a stray uncommented -16. The
  assembler emits an erasable word there and advances allocation. Original
  ALTVAR is physical **01357**, versus pinned 01356. Its interpretive operand
  at bank34,3215 is encoded as address+1: original **01360**, pinned 01357.
  The support reference is P20-P25's REND15 operand, listing line1104.
  Bank34's checksum changes accordingly. The stray word is retained in the
  allocation audit and excluded from fixed data; this does not certify the
  entire erasable declaration file as translated.
* Original R53FLBIT aliases BIT8 at fixed address05010, whose word is00200.
  Pinned support aliases BIT6 at05012, whose word is00040. No assembled support
  operand refers to R53FLBIT. Other added/missing symbol spellings are recorded.

ALTVAR is the only changed common erasable address in the complete symbol
comparison; R53FLBIT is the additional changed fixed alias. LM uses one
documented zero-word compatibility alias, TTPI0 EQUALS TTPIO, so pinned
support callers resolve while the original spelling remains untouched.
The resulting LM rope equals its established fixture byte for byte. The CM
overlay rope differs only in the ALTVAR support operand and its bank checksum.
That support instruction is not claimed as a native translation here.

Verification:

~~~sh
python3 tools/fixed_data_generate.py
python3 tests/test_fixed_data.py
cc -std=c99 -O2 -Wall -Wextra -Werror -Wconversion -pedantic \
  -Iinclude src/fixed_data.c tests/test_fixed_data.c -o build/fixed-data-test
./build/fixed-data-test
python3 tools/fixed_data_vice_test.py --cc mos-c64-clang --vice x64sc \
  --rom-dir /path/to/C64/roms --data-dir /path/to/vice/data
~~~

All 2,871 words match independently decoded original binary ropes on the host.
All 131,072 possible mission/address probes enforce exact coverage and failure
preservation. Separate one-mission builds and ASan/UBSan pass. Stock-ROM PAL
VICE executes the native API and dumps every returned word for exact comparison:
mailbox a5370b, PRG6,447 bytes, loaded end $212e. This target proof links both
datasets directly; native disk-overlay execution is qualified separately by
the overlay framework milestone.
