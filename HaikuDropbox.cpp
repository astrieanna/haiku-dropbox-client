#include <stdio.h>
#include <stdlib.h>

#include "App.h"
#include <Directory.h>
#include <NodeMonitor.h>
#include <Entry.h>
#include <Path.h>
#include <String.h>


/*
* Sets up the Node Monitoring for Dropbox folder and contents
* and creates data structure for determining which files are deleted or edited
*/
App::App(void)
  : BApplication("application/x-vnd.lh-MyDropboxClient")
{
  //start watching ~/Dropbox folder contents (create, delete, move)
  BDirectory dir("/boot/home/Dropbox"); //don't use ~ here
  node_ref nref;
  status_t err;
  if(dir.InitCheck() == B_OK){
    dir.GetNodeRef(&nref);
    err = watch_node(&nref, B_WATCH_DIRECTORY, BMessenger(this));
    if(err != B_OK)
      printf("Watch Node: Not OK\n");
  }

  // record each file in the folder so that we know the name on deletion
  BEntry entry;
  status_t err2;
  err = dir.GetNextEntry(&entry);
  while(err == B_OK) //loop over files
  {
    err = dir.GetNextEntry(&entry);
    this->tracked_files.AddItem((void*)&entry); //add file to my list
    err2 = entry.GetNodeRef(&nref);
    if(err2 == B_OK)
    {
      err2 = watch_node(&nref, B_WATCH_STAT, be_app_messenger); //watch for edits
      if(err2 != B_OK)
        printf("Watch file Node: Not OK\n");
    }
  }

}

/*
* Runs a command in the terminal, given the string you'd type
*/
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

/*
* Given a local file path,
* call the script to delete the corresponding Dropbox file
*/
void
delete_file_on_dropbox(const char * filepath)
{
  BString s, dbfp;
  s = BString(filepath);
  s.RemoveFirst("/boot/home/Dropbox");
  dbfp << "python db_rm.py " << s;
  run_script(dbfp);
}

/*
* Convert a local absolute filepath to a Dropbox one
* by removing the <path to Dropbox> from the beginning
*/
BString local_to_db_filepath(const char * local_path)
{
  BString s;
  s = BString(local_path);
  s.RemoveFirst("/boot/home/Dropbox/");
  return s;
}

/*
* Given the local file path of a new file,
* run the script to upload it to Dropbox
*/
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

/*
* Given the local file path of a new folder,
* run the script to mkdir on Dropbox
*/
void
add_folder_to_dropbox(const char * filepath)
{
  BString s;
  s << "python db_mkdir.py " << local_to_db_filepath(filepath);
  printf("local filepath: %s\n", filepath);
  printf("db filepath: %s\n", local_to_db_filepath(filepath).String());
  run_script(s.String());
}

/*
* TODO
* Given a "file/folder moved" message,
* figure out whether to call add or delete
* and with what file path
* and then call add/delete.
*/
void
moved_file(BMessage *msg)
{
  //is this file being move into or out of ~/Dropbox?
  run_script("python db_ls.py");
}

/*
* Given a local file path,
* update the corresponding file on Dropbox
*/
void
update_file_in_dropbox(const char * filepath)
{
  add_file_to_dropbox(filepath); //just put it?
}

/*
* Message Handling Function
* If it's a node monitor message,
* then figure out what to do based on it.
* Otherwise, let BApplication handle it.
*/
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
            node_ref nref, cref;
            msg->FindInt32("device",&nref.device);
            msg->FindInt64("node",&nref.node);
            BEntry *entryPtr;
            int32 ktr = 0;
            int32 limit = this->tracked_files.CountItems();
            while((entryPtr = (BEntry *)this->tracked_files.ItemAt(ktr++))&&(ktr<limit))
            {
              if(entryPtr->InitCheck() == B_OK)
              {
                entryPtr->GetNodeRef(&cref);
              }
              else
              {
                printf("OH NO!\t%lu\n",entryPtr->InitCheck());
                printf("OK = %lu\n", B_OK);
                printf("Entry Not Found = %lu\n",B_ENTRY_NOT_FOUND);
                printf("Bad Value = %lu\n",B_BAD_VALUE);
                printf("No Init = %lu\n", B_NO_INIT);
                return;
              }

              if(nref == cref)
              {
                BPath path;
                entryPtr->GetPath(&path);
                delete_file_on_dropbox(path.Path());
                break; //break out of loop
              }
            }
            break;
          }
          case B_STAT_CHANGED:
          {
            printf("EDITED FILE\n");
            node_ref nref1,nref2;
            msg->FindInt32("device",&nref1.device);
            msg->FindInt64("node",&nref1.node);
            BEntry * entryPtr;
            int32 ktr = 0;
            while((entryPtr = (BEntry *)this->tracked_files.ItemAt(ktr++)))
            {
              entryPtr->GetNodeRef(&nref2);
              if(nref1 == nref2)
              {
                BPath path;
                entryPtr->GetPath(&path);
                update_file_in_dropbox(path.Path());
                break;
              }
            }
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
  //set up application (watch Dropbox folder & contents)
  App *app = new App();

  //start the application
  app->Run();

  //clean up now that we're shutting down
  delete app;
  return 0;
}
