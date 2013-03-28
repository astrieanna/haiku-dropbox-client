import os

try:
  file = open("lines_db_put.txt",'r')
  lines = file.read()
  os.remove("lines_db_put.txt")
  print "%s" % lines
except:
  file = open("log.txt",'a')
  file.write("db_put got called\n")
  file.close()

