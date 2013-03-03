import sys
from cli_client import APP_KEY, APP_SECRET, DropboxTerm

def main(src,dest,parent_rev=None):
    if APP_KEY == '' or APP_SECRET == '':
        exit("You need to set your APP_KEY and APP_SECRET!")
    term = DropboxTerm(APP_KEY, APP_SECRET)

    if(parent_rev != None):
        metadata = term.do_put(src,dest,parent_rev)
    else:
        metadata = term.do_put(src,dest,None)
    print "%s %s" % (metadata['path'],metadata['rev'])
    print >> sys.stderr, "%s %s" % (metadata['path'],metadata['rev'])

if __name__ == '__main__':
    if len(sys.argv) == 3:
        main(sys.argv[1],sys.argv[2])
    elif len(sys.argv) == 4:
        main(sys.argv[1],sys.argv[2],parent_rev=sys.argv[3])
    else:
        print "usage: python db_put.py <local src> <dropbox dest>"
