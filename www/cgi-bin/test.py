#!/usr/bin/python3
import os

print("Content-Type: text/html")
print()
print("<h1>CGI env:</h1>")
print("<ul>")
for key in os.environ:
    print("<li>" + key + " = " + os.environ[key] + "</li>")
print("</ul>")
