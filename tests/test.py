from subprocess import Popen
import time
import os

os.system("rm -rf /boot/home/Dropbox/*")
os.system("rm log.txt lines_*")
os.system("touch log.txt")

# start dbclient
p = Popen(["../objects.x86-gcc2-release/hdbclient.exe"])
time.sleep(2)

putlines = open("lines_db_put.txt",'w+')
putlines.write("success")
putlines.close()

foo = open("/boot/home/Dropbox/foo",'w+')
foo.write("Hello,World")
foo.close()

deltalines = open("lines_db_delta.txt",'w+')
deltalines.write("FILE /foo")
deltalines.close()

time.sleep(10)

# kill dbclient
p.kill()

print "Checking Assertions:"
os.system("cat log.txt")

