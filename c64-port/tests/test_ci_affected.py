#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""tools/ci_affected.py: a comment-only change to a C or Python file is not a code change."""
import pathlib
import sys
import unittest

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[1] / 'tools'))
import ci_affected  # noqa: E402

code_of = ci_affected.code_of


class CodeOf(unittest.TestCase):
    def test_c_comment_rewrap_is_same_code(self):
        a = 'int x; /* one\n two */\nint y = 1; // c\n'
        b = 'int x; /* one two\n */\n\n   int y = 1;   // other\n'
        self.assertEqual(code_of('a.h', a), code_of('a.h', b))

    def test_c_code_change(self):
        self.assertNotEqual(code_of('a.c', 'int y = 1;'), code_of('a.c', 'int y = 2;'))

    def test_comment_marks_inside_strings_are_code(self):
        self.assertNotEqual(code_of('a.c', 'char *s = "/* x */";'), code_of('a.c', 'char *s = "/* y */";'))

    def test_line_sensitive_files_are_not_compared(self):
        self.assertIsNone(code_of('a.c', 'assert(x); /* a */'))
        self.assertIsNone(code_of('a.c', 'int l = __LINE__;'))
        self.assertIsNone(code_of('a.c', 'char *s = "a\\\n  b";'))

    def test_python(self):
        self.assertEqual(code_of('a.py', 'x=1  # a\n'), code_of('a.py', 'x = 1\n# b\n'))
        self.assertNotEqual(code_of('a.py', 'x = 1'), code_of('a.py', 'x = 2'))

    def test_other_kinds_are_not_compared(self):
        self.assertIsNone(code_of('a.mk', 'x'))


if __name__ == '__main__':
    unittest.main()
