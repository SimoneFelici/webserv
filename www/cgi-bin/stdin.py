#!/usr/bin/python3
import sys

data = sys.stdin.read()
print("Content-Type: text/plain")
print()
print("Read" + str(len(data)) + " bytes")
