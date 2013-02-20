import sys
from cli_client import APP_KEY, APP_SECRET, DropboxTerm

def main():
    if APP_KEY == '' or APP_SECRET == '':
        exit("You need to set your APP_KEY and APP_SECRET!")
    term = DropboxTerm(APP_KEY, APP_SECRET)

    try:
      with open('delta.txt', 'r') as f:
        cursor = f.read()
    except IOError as e:
      cursor = None
    
    new_cursor = term.do_delta(cursor)

    with open('delta.txt','w') as f:
       f.write(new_cursor)
    print new_cursor

if __name__ == '__main__':
    if len(sys.argv) == 1:
        main()
    elif len(sys.argv) != 2:
        print "usage: python db_delta.py [<cursor>]"
    else:
        main(sys.argv[1])
