"""Check that malformed LPUnit input cannot silently discard tests."""
import io
import runpy
import sys
import unittest

TestSuite = runpy.run_path(sys.argv.pop(1))["TestSuite"]


class GeneratorTests(unittest.TestCase):
    def test_unclosed_test_before_next_test(self):
        suite = TestSuite()
        with self.assertRaisesRegex(SyntaxError, "@end"):
            suite.ReadFile(io.StringIO("@testsuite example\n@test first\n{}\n@test second\n{}\n@end\n"))

    def test_unclosed_test_at_eof(self):
        with self.assertRaisesRegex(SyntaxError, "@end"):
            TestSuite().ReadFile(io.StringIO("@testsuite example\n@test first\n{}\n"))

    def test_unmatched_end(self):
        with self.assertRaisesRegex(SyntaxError, "@end"):
            TestSuite().ReadFile(io.StringIO("@testsuite example\n@end\n"))

    def test_complete_tests_are_preserved(self):
        suite = TestSuite()
        suite.ReadFile(io.StringIO("@testsuite example\n@test first\n{}\n@end\n@test second\n{}\n@end\n"))
        self.assertEqual([name for name, _ in suite.tests], ["first", "second"])


if __name__ == "__main__":
    unittest.main()
