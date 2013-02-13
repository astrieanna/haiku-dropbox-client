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
  BString s, dbfp;
  s = BString(filepath);
  s.RemoveFirst("/boot/home/Dropbox");
  dbfp << "python db_rm.py " << s;
  run_script(dbfp);
}

BString local_to_db_filepath(const char * local_path)
{
  BString s;
  s = BString(local_path);
  s.RemoveFirst("/boot/home/Dropbox/");
  return s;
}

void
add_file_to_dropbox(const char * filepath)
{
  BString s, dbfp;
  dbfp = local_to_db_filepath(filepath);
  s << "python db_put.py " << BString(filepath) << " " << dbfp;
  printf("local filepath:%s\n",filepath);
  printf("dropbox filepath:%s\n",dbfp.String());
  run_script(s.String());
}

void
add_folder_to_dropbox(const char * filepath)
{
  BString s;
  s << "python db_mkdir.py " << local_to_db_filepath(filepath);
  printf("local filepath: %s\n", filepath);
  printf("db filepath: %s\n", local_to_db_filepath(filepath).String());
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
            printf("name:%s\n",name);
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
            //node_ref nref;
            //msg->FindInt32("device",&nref.device);
            //msg->FindInt64("node",&nref.node);
            //BNode node = BNode(&nref);
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
