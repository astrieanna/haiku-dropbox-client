import sys
from cli_client import APP_KEY, APP_SECRET, DropboxTerm

def main(path):
    if APP_KEY == '' or APP_SECRET == '':
        exit("You need to set your APP_KEY and APP_SECRET!")
    term = DropboxTerm(APP_KEY, APP_SECRET)

    term.do_mkdir(path)

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print "usage: python db_rm.py <dropbox path>"
    main(sys.argv[1])
