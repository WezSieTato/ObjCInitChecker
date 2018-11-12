#!/usr/bin/env python

import os
import sys
import subprocess

SRCROOT = os.getenv("SRCROOT")
CONFIGURATION_BUILD_DIR = os.getenv("CONFIGURATION_BUILD_DIR")

compiler = "{root}/clang+llvm/bin/clang".format(root=SRCROOT)
plugin_path = "{build_dir}/libObjCInitChecker.dylib".format(build_dir=CONFIGURATION_BUILD_DIR)
test_file_path = "{root}/Tests/ObjCSuperInitObjectTest.m".format(root=SRCROOT)
compiler_flags = ["-Xclang", "-load", "-Xclang", plugin_path,
                  "-Xclang", "-plugin", "-Xclang", "ObjCSuperInitPlugin",
                  "-fsyntax-only"]

command = [compiler] + compiler_flags + [test_file_path]
print command
try:
    actual_result = subprocess.check_output(command, stderr=subprocess.STDOUT)
except Exception, e:
    actual_result = str(e.output)

expected_result = """{file}:49:1: error: Missing init call
}}
^
{file}:53:1: error: Missing init call
}}
^
2 errors generated.
""".format(file=test_file_path)
if actual_result != expected_result:
    print("Test failed:")
    print("  expected")
    print(expected_result)
    print("  received")
    print(actual_result)
    sys.exit(1)
print("Test succeeded")
sys.exit(0)
