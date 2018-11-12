#!/usr/bin/env python

import os
import urllib2
import subprocess

print("Downloading clang+llvm...")
response = urllib2.urlopen("http://releases.llvm.org/7.0.0/clang+llvm-7.0.0-x86_64-apple-darwin.tar.xz")
data = response.read()
filename = "clang+llvm.tar.xz"
file_ = open(filename, 'w')
file_.write(data)
file_.close()

print("Unpacking tarfile...")
command = ["tar", "xf"] + [filename]
subprocess.call(command, stderr=subprocess.STDOUT)
os.rename("clang+llvm-7.0.0-x86_64-apple-darwin", "clang+llvm")

print("Remove tarfile..")
os.remove(filename)

print("Done")
