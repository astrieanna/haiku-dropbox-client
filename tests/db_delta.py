import os

file = open("lines_db_delta.txt")
lines = file.read()
os.remove("lines_db_delta.txt")
print "%s" % lines  
