import sys
from cli_client import APP_KEY, APP_SECRET, DropboxTerm

def main(cursor=None):
    if APP_KEY == '' or APP_SECRET == '':
        exit("You need to set your APP_KEY and APP_SECRET!")
    term = DropboxTerm(APP_KEY, APP_SECRET)

    term.do_delta(cursor)

    print "Asked for the Delta!"

if __name__ == '__main__':
    if len(sys.argv) == 1:
        main()
    elif len(sys.argv) != 2:
        print "usage: python db_delta.py [<cursor>]"
    else:
        main(sys.argv[1])
