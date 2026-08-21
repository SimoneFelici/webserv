import sys, os

n = int(os.environ.get("CONTENT_LENGTH", "0"))
data = sys.stdin.read(n)
print("Content-Type: text/plain")
print()
print("ricevuti " + str(len(data)) + " byte: " + data)
