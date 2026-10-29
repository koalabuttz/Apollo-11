#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""tools/flight_vice_load.py's host reference (#426) on made-up wrappers and
results: the FLIGHT_POINTS lines a module wrapper gives, the fl_point fields
the check records, and what makes a point differ from the host."""
import pathlib
import struct
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import flight_vice_load as fvl  # noqa: E402


def wrapper(text):
    with tempfile.TemporaryDirectory() as d:
        p = pathlib.Path(d) / 'module-0101.c'
        p.write_text(text + '#include "flight_table_module.c"\n')
        return fvl.wrapper(p)


class Wrapper(unittest.TestCase):
    def test_basic_tables_and_sites(self):
        fixed, lines, _ = wrapper('#define FS_DIRECT_ONLY 1\n'
                               '#define FS_SITES {14375u,040153u},{14448u,036626u}\n'
                               '#define FS_BASIC(X) X(lm_a_routines) X(lm_b_routines)\n#define FS_INTERP(X) \n')
        self.assertEqual(fixed, 0)
        self.assertFalse(wrapper('#define FS_MCT 15u\n#define FS_BASIC(X) X(t)\n#define FS_INTERP(X) \n')[2])
        self.assertTrue(wrapper('#define FS_MCT 0u\n#define FS_BASIC(X) X(t)\n#define FS_INTERP(X) \n')[2])
        self.assertEqual(lines, ['B lm_a_routines', 'B lm_b_routines', 'S 14375 40153', 'S 14448 36626'])

    def test_pick_with_keys(self):
        fixed, lines, _ = wrapper('#define FS_PICK(X) X(lm_interpreter_basic,31) X(lm_other,2)\n'
                               '#define FS_KEYS 36534u,02000u\n#define FS_BASIC(X) \n#define FS_INTERP(X) \n')
        self.assertEqual(lines, ['K lm_interpreter_basic 31 36534', 'K lm_other 2 1024'])

    def test_chunk_and_self_check_hook(self):
        fixed, lines, _ = wrapper('#define FS_CHUNK_TABLE lm_self_check_routines\n#define FS_CHUNK(X) X(0) X(1)\n'
                               '#define FS_FIXED_DATA lm_self_check_rope_data\n#define FS_BASIC(X) \n#define FS_INTERP(X) \n')
        self.assertEqual((fixed, lines), (1, ['K lm_self_check_routines 0', 'K lm_self_check_routines 1']))

    def test_interpretive_and_rope(self):
        fixed, lines, _ = wrapper('#define FS_BASIC(X) X(lm_m_basic)\n#define FS_INTERP(X) X(lm_m_interpretive)\n'
                               '#define FS_HAS_INTERP 1\n#define FS_DANZIG agc_lm_danzig_marker\n#define FS_ROPE_DATA 1\n')
        self.assertEqual((fixed, lines), (2, ['B lm_m_basic', 'H', 'I lm_m_interpretive']))

    def test_io_record_is_not_direct(self):
        self.assertIsNone(wrapper('#define FS_IO_KIND FLIGHT_IO_LM_DNTM\n#define FS_IO_CM 0\n'
                                  '#define FS_BASIC(X) X(lm_t)\n#define FS_INTERP(X) \n'))

    def test_keys_must_name_every_entry(self):
        with self.assertRaises(SystemExit):
            wrapper('#define FS_PICK(X) X(lm_t,1) X(lm_t,2)\n#define FS_KEYS 5u\n'
                    '#define FS_BASIC(X) \n#define FS_INTERP(X) \n')


class PointTable(unittest.TestCase):
    def test_layout(self):
        reu = bytearray(fvl.POINTS_END + 16)
        total = fvl.write_points(reu, [[(0o4000, 0o5567)], [], [(0, 0o2353), (0o20000, 0o3010)]])
        self.assertEqual(total, 3)
        self.assertEqual(struct.unpack_from('<4H', reu, fvl.POINTS_REU), (3, 1, 0, 2))
        self.assertEqual(struct.unpack_from('<6H', reu, fvl.POINTS_REU + 8),
                         (0o4000, 0o5567, 0, 0o2353, 0o20000, 0o3010))

    def test_refuses_a_used_area_and_overflow(self):
        reu = bytearray(fvl.POINTS_END + 16)
        reu[fvl.POINTS_END - 1] = 1
        with self.assertRaises(SystemExit):
            fvl.write_points(reu, [[(0, 1)]])
        with self.assertRaises(SystemExit):
            fvl.write_points(bytearray(fvl.POINTS_END), [[(0, i) for i in range(fvl.JOB_BYTES // 4 + 1)]])


class Point(unittest.TestCase):
    def test_fields(self):
        self.assertEqual(fvl.point(0x84, 0x2c), (4, 0, 0x12c))   # unmapped, 300 steps
        self.assertEqual(fvl.point(fvl.POINT_MAIN | 9, 0), (9, fvl.POINT_MAIN, 0))
        self.assertEqual(fvl.point(fvl.POINT_NONE, 0)[1], fvl.POINT_NONE)
        self.assertEqual(fvl.point(fvl.POINT_FULL | 0x80, 0), (0, fvl.POINT_FULL, 0x100))


class Compare(unittest.TestCase):
    MODULES = [dict(id='0x0101'), dict(id='0x0102'), dict(id='0x0103')]

    def test_match(self):
        self.assertEqual(fvl.compare({0: (0x1234, 7, 0), 2: (5, 0, 4)}, '0 4660 7 0\n2 5 0 4\n', self.MODULES), [])

    def test_each_difference_named(self):
        out = fvl.compare({0: (0x1234, 7, 0), 1: (1, 2, 0), 2: (3, 3, 0)}, '0 4661 7 0\n1 1 3 8\n', self.MODULES)
        self.assertEqual(out, ['module 0x0101: sum 1234/1235 (C64/host)',
                               'module 0x0102: steps 2/3, status ok/1201 (C64/host)',
                               'module 0x0103: no host run'])


if __name__ == '__main__':
    unittest.main()
