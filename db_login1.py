from cli_client import APP_KEY, APP_SECRET, DropboxTerm

def main():
    if APP_KEY == '' or APP_SECRET == '':
        exit("You need to set your APP_KEY and APP_SECRET!")
    term = DropboxTerm(APP_KEY, APP_SECRET)
    #term.cmdloop()
    term.do_login()

if __name__ == '__main__':
    main()
