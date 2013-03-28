import os

try:
  file = open("lines_db_get.txt",'r')
  lines = file.read()
  os.remove("lines_db_get.txt")
  print "%s" % lines
except:
  file = open("log.txt",'a')
  file.write("db_get got called\n")
  file.close()

