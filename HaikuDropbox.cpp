#include <stdio.h>
#include <stdlib.h>

#include "App.h"
#include <Directory.h>
#include <NodeMonitor.h>
#include <Entry.h>
#include <Path.h>
#include <String.h>

App::App(void)
  : BApplication("application/x-vnd.lh-MyDropboxClient")
{
  //start watching ~/Dropbox folder
  BDirectory dir("/boot/home/Dropbox");
  node_ref nref;
  status_t err;
  if(dir.InitCheck() == B_OK){
    dir.GetNodeRef(&nref);
    err = watch_node(&nref, B_WATCH_DIRECTORY, BMessenger(this));
    if(err != B_OK)
      printf("Watch Node: Not OK\n");
  }
}

int
run_script(const char *cmd)
{
  char buf[BUFSIZ];
  FILE *ptr;

  if ((ptr = popen(cmd, "r")) != NULL)
    while (fgets(buf, BUFSIZ, ptr) != NULL)
      (void) printf("RAWR%s", buf);
  (void) pclose(ptr);
  return 0;
}

void
delete_file_on_dropbox(char * filepath)
{
  run_script("python db_ls.py");
}

void
add_file_to_dropbox(const char * filepath)
{
  BString s, dbfp;
  dbfp = BString(filepath);
  dbfp.RemoveFirst("/boot/home/Dropbox/");
  s << "python db_put.py " << BString(filepath) << " " << dbfp;
  run_script(s.String());
}

void
moved_file(BMessage *msg) 
{
  //is this file being move into or out of ~/Dropbox?
  run_script("python db_ls.py");
}

void
App::MessageReceived(BMessage *msg)
{
  switch(msg->what)
  {
    case B_NODE_MONITOR:
    {
      printf("Received Node Monitor Alert\n");
      status_t err;
      int32 opcode;
      err = msg->FindInt32("opcode",&opcode);
      if(err == B_OK)
      {
        printf("what:%d\topcode:%d\n",msg->what, opcode);
        switch(opcode)
        {
          case B_ENTRY_CREATED:
          {
            printf("NEW FILE\n");
            entry_ref ref;
            BPath path; 
            const char * name;
            msg->FindInt32("device",&ref.device);
            msg->FindInt64("directory",&ref.directory);
            msg->FindString("name",&name);
            ref.set_name(name);
            BEntry new_file = BEntry(&ref);
            new_file.GetPath(&path);
            add_file_to_dropbox(path.Path());
            break;
          }
          case B_ENTRY_MOVED:
          {
            printf("MOVED FILE\n");
            moved_file(msg);
            break;
          }
          case B_ENTRY_REMOVED:
          {
            printf("DELETED FILE\n");
            delete_file_on_dropbox("hi");
            break;
          }
          default:
          {
            printf("default case opcode...\n");
          }
        }
      }
      break;
    }
    default:
    {
      printf("default msg\n");
      BApplication::MessageReceived(msg);
      break;
    }
  }
}

int
main(void)
{
  //Haiku make window code
  App *app = new App();

  app->Run();
  delete app;
  return 0;
}
