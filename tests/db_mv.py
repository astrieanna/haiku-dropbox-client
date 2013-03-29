import os

try:
  file = open("lines_db_mv.txt",'r')
  lines = file.read()
  os.remove("lines_db_mv.txt")
  print "%s" % lines
except:
  file = open("log.txt",'a')
  file.write("db_mv got called\n")
  file.close()

