from subprocess import Popen
import time
import os

#setup
os.system("rm -rf /boot/home/Dropbox/*")
os.system("rm log.txt lines_*")
os.system("touch log.txt")

# start dbclient
p = Popen(["../objects.x86-gcc2-release/hdbclient.exe"])
time.sleep(2)

#tell db_put what to say
putlines = open("lines_db_put.txt",'w+')
putlines.write("success AAAAA")
putlines.close()

#create file
foo = open("/boot/home/Dropbox/foo",'w+')
foo.write("Hello,World")
foo.close()

#tell db_delta what to say
deltalines = open("lines_db_delta.txt",'w+')
deltalines.write("FILE /foo")
deltalines.close()

#wait for pull-deltas to finish
time.sleep(10)

# kill dbclient
p.kill()

# produce result
print "Checking Assertions:"
os.system("cat log.txt")

