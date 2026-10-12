#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Host-only GPL oracle variant: yaAGC without its IMU and optics peripherals.

Between instructions the pinned engine models the hardware that channel 14
drives: it drains GYROCTR into fictitious channel 0177 while GYRO ACTIVITY is
set, bursts CDUXCMD..CDUZCMD while the CDU drive bits are set, and dumps
OPTX/OPTY. Those are device effects, not program semantics, and the native
port keeps them in its device layer. The imu-mode-switching oracle removes
exactly that block (the gyro, IMU CDU drive and optics CDU sections before
instruction decoding); no instruction, timing, counter or interrupt code
changes. The pinned checkout stays intact; the variant, its reviewable diff
and its object go to build/.
"""
import difflib
import hashlib
import pathlib
import subprocess

ROOT = pathlib.Path(__file__).resolve().parents[1]
ENGINE = ROOT / 'deps/virtualagc/yaAGC'
SOURCE = ENGINE / 'agc_engine.c'
PINNED = '9c59728cdd968503caaaafe50a18ab3ed841f6176da3299c137f3a6a158fcbef'
START = '  //----------------------------------------------------------------------\n  // Same principle as for the counter-timers (above), but for handling \n'
END = '  //----------------------------------------------------------------------  \n  // Okay, here\'s the stuff that actually has to do with decoding instructions.\n'
REMOVED = '91e947cca392b0b8537710c9ba43dbd7e0d2094c5424fbc71175f4eb02da3e0b'
OUT = ROOT / 'build/agc_engine_no_imu_device.c'


def main():
    original = SOURCE.read_text()
    assert hashlib.sha256(original.encode()).hexdigest() == PINNED, 'unexpected yaAGC engine revision'
    assert original.count(START) == 1 and original.count(END) == 1
    a, b = original.index(START), original.index(END)
    removed = original[a:b]
    digest = hashlib.sha256(removed.encode()).hexdigest()
    assert digest == REMOVED, f'peripheral block changed: {digest}'
    for name in ('RegGYROCTR', 'RegCDUXCMD', 'RegOPTX', 'RegOPTY'):
        assert name in removed
    modified = original[:a] + original[b:]
    OUT.parent.mkdir(exist_ok=True)
    OUT.write_text(modified)
    patch = ''.join(difflib.unified_diff(original.splitlines(True), modified.splitlines(True),
                                         fromfile='pinned/agc_engine.c', tofile='no-imu-device/agc_engine.c'))
    (ROOT / 'build/no-imu-device.patch').write_text(patch)
    subprocess.run(['gcc', '-O2', '-w', '-I' + str(ENGINE), '-c', str(OUT), '-o',
                    str(ROOT / 'build/agc_engine_no_imu_device.o')], check=True)
    print('Pinned engine SHA256:', PINNED)
    print('Removed peripheral block SHA256:', digest, f'({removed.count(chr(10))} lines)')


if __name__ == '__main__':
    main()
