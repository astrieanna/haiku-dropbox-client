import sys
from cli_client import APP_KEY, APP_SECRET, DropboxTerm

def main(src,dest):
    if APP_KEY == '' or APP_SECRET == '':
        exit("You need to set your APP_KEY and APP_SECRET!")
    term = DropboxTerm(APP_KEY, APP_SECRET)

    term.do_mv(src,dest)

if __name__ == '__main__':
    #get cmdline args...
    if len(sys.argv) != 3:
        print "usage: python db_get.py <remote src> <local dest>"
    else:
      main(sys.argv[1],sys.argv[2])
