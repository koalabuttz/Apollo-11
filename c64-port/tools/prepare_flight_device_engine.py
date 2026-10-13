#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Host-only GPL oracle variant for the CM flight control files: yaAGC with
DV always through its hardware model and without its IMU and optics
peripherals.

The two changes are exactly those of tools/prepare_hardware_dv.py (the fast
DV path mishandles a sign-extended -0 in L) and
tools/prepare_no_imu_device_engine.py (between instructions the pinned
engine drains GYROCTR, bursts CDUXCMD..CDUZCMD while channel 14's drive
bits are set and dumps OPTX/OPTY: device effects the native port keeps in
its device layer). The DAP, TVC and entry files load those counters and set
the drive bits themselves, so their comparison needs both. The pinned
checkout stays intact; the variant, its reviewable diff and its object go
to build/.
"""
import difflib
import hashlib
import pathlib
import subprocess

ROOT = pathlib.Path(__file__).resolve().parents[1]
ENGINE = ROOT / 'deps/virtualagc/yaAGC'
SOURCE = ENGINE / 'agc_engine.c'
PINNED = '9c59728cdd968503caaaafe50a18ab3ed841f6176da3299c137f3a6a158fcbef'
DV = 'if (AbsA > AbsK || (AbsA == AbsK && AbsL != AGC_P0) || ValueOverflowed(Div16) != AGC_P0)'
START = '  //----------------------------------------------------------------------\n  // Same principle as for the counter-timers (above), but for handling \n'
END = '  //----------------------------------------------------------------------  \n  // Okay, here\'s the stuff that actually has to do with decoding instructions.\n'
REMOVED = '91e947cca392b0b8537710c9ba43dbd7e0d2094c5424fbc71175f4eb02da3e0b'
OUT = ROOT / 'build/agc_engine_flight_device.c'


def main():
    original = SOURCE.read_text()
    assert hashlib.sha256(original.encode()).hexdigest() == PINNED, 'unexpected yaAGC engine revision'
    assert original.count(DV) == 1 and original.count(START) == 1 and original.count(END) == 1
    a, b = original.index(START), original.index(END)
    removed = original[a:b]
    digest = hashlib.sha256(removed.encode()).hexdigest()
    assert digest == REMOVED, f'peripheral block changed: {digest}'
    modified = (original[:a] + original[b:]).replace(DV, DV.replace('if (', 'if (1 || ', 1))
    OUT.parent.mkdir(exist_ok=True)
    OUT.write_text(modified)
    patch = ''.join(difflib.unified_diff(original.splitlines(True), modified.splitlines(True),
                                         fromfile='pinned/agc_engine.c', tofile='flight-device/agc_engine.c'))
    (ROOT / 'build/flight-device.patch').write_text(patch)
    subprocess.run(['gcc', '-O2', '-w', '-I' + str(ENGINE), '-c', str(OUT), '-o',
                    str(ROOT / 'build/agc_engine_flight_device.o')], check=True)
    print('Pinned engine SHA256:', PINNED)
    print('Removed peripheral block SHA256:', digest, f'({removed.count(chr(10))} lines)')
    print('Variant SHA256:', hashlib.sha256(modified.encode()).hexdigest())


if __name__ == '__main__':
    main()
