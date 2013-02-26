#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "App.h"
#include <Directory.h>
#include <NodeMonitor.h>
#include <Entry.h>
#include <Path.h>
#include <String.h>
#include <File.h>

const char * local_path_string = "/boot/home/Dropbox/";
const char * local_path_string_noslash = "/boot/home/Dropbox";

// String modification helper functions

/*
* Convert a Dropbox path to a local absolute filepath
* by adding the <path to Dropbox> to the beginning
*/
BString db_to_local_filepath(const char * local_path)
{
  BString s;
  s << local_path_string << local_path;
  return s;
}

/*
* Convert a local absolute filepath to a Dropbox one
* by removing the <path to Dropbox> from the beginning
*/
BString local_to_db_filepath(const char * local_path)
{
  BString s;
  s = BString(local_path);
  s.RemoveFirst(local_path_string);
  return s;
}

/*
* Moves the first line in src to dest.
* Destructively alters src.
* newline is included in the move.
*/
int32
get_next_line(BString *src, BString *dest)
{
  const int32 eol = src->FindFirst('\n');
  if(eol == B_ERROR)
    return B_ERROR;

  src->MoveInto(*dest,0,eol+1); //TODO: does this ever return an error code?
  return B_OK;
}


// Run Python Scripts
//TODO: Merge the 0,1,2 argument cases into 1 function.

/*
* Runs a command in the terminal, given the string you'd type
*/
BString*
run_script(const char *cmd)
{
  char buf[BUFSIZ];
  FILE *ptr;
  BString *output = new BString;

  if ((ptr = popen(cmd, "r")) != NULL)
    while (fgets(buf, BUFSIZ, ptr) != NULL)
       output->Append(buf);

  (void) pclose(ptr); //TODO: what's the return type?
  return output;
}

/*
* Run a python script named cmd, with the arguments path1 and path2
* Uses execlp to avoid any escaping issues with the path arguments
*/
BString*
get_or_put(const char *cmd, const char *path1, const char *path2)
{
  pid_t pid = fork();
  BString *output = new BString;
  char buf[BUFSIZ];

  //open pipe
  int fd[2];
  pipe(fd);

  if(pid < 0)
  {
    return output; //error
  }
  if(pid == 0)
  {
    //child
    //make stdout the pipe
    dup2(fd[0],STDOUT_FILENO);
    execlp("python","python",cmd,path1,path2,(char*)0);
  }
  else //parent
  {
    dup2(fd[1],STDIN_FILENO);

    //wait for child process to finish
    int status;
    waitpid(pid, &status, 0);

    //use read-end of pipe to fill in BString return value.
    while(fgets(buf,BUFSIZ,stdin) !=NULL)
    {
      output->Append("RAWR");
      output->Append(buf);
    }
    printf("output:\n%s\n",output->String());

  }
  return output;

}

/*
* Runs a python script named cmd with one arg, path.
* uses execlp to avoid escaping issues with path.
*/
BString*
one_path_arg(const char *cmd, const char *path)
{
  pid_t pid = fork();
  BString *output = new BString;
  char buf[BUFSIZ];

  int fd[2];
  pipe(fd);

  if(pid < 0)
  {
    dup2(fd[0],STDOUT_FILENO);
    execlp("python","python",cmd,path);
  }
  else //parent
  {
    dup2(fd[1],STDIN_FILENO);
    while(fgets(buf,BUFSIZ,stdin) != NULL)
      output->Append(buf);
  }
  return output;
}

// Talk to Dropbox

/*
* Given a local file path,
* call the script to delete the corresponding Dropbox file
*/
void
delete_file_on_dropbox(const char * filepath)
{
  printf("Telling Dropbox to Delete\n");
  BString s, dbfp;
  s = BString(filepath);
  s.RemoveFirst(local_path_string_noslash);
  dbfp << "python db_rm.py " << s;
  printf("%s\n",dbfp.String());
  run_script(dbfp);
}

/*
* Given the local file path of a new file,
* run the script to upload it to Dropbox
*/
void
add_file_to_dropbox(const char * filepath)
{
  get_or_put("db_put.py",filepath, local_to_db_filepath(filepath));
  printf("local filepath:%s\n",filepath);
  printf("dropbox filepath:%s\n",local_to_db_filepath(filepath).String());
}

/*
* Given the local file path of a new folder,
* run the script to mkdir on Dropbox
*/
void
add_folder_to_dropbox(const char * filepath)
{
  one_path_arg("db_mkdir.py",local_to_db_filepath(filepath));
  printf("local filepath: %s\n", filepath);
  printf("db filepath: %s\n", local_to_db_filepath(filepath).String());
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
  printf("Putting %s to Dropbox.\n", filepath);
  add_file_to_dropbox(filepath); //just put it?
}

create_local_directory(BString *dropbox_path)
{
    create_directory(BString(local_path_string) << path, 0x0777);
}

// Act on Deltas
// TODO: consider moving some cases into separate functions
int
parse_command(BString command)
{
  if(command.Compare("RESET\n") == 0)
  {
    printf("Burn Everything. 8D\n");
    run_script("rm -rf ~/Dropbox"); //TODO: use native API, not shell command
    create_local_directory(BString(""));
  }
  else if(command.Compare("FILE ",5) == 0)
  {
    BString path, dirpath;
    command.CopyInto(path,5,command.FindLast(" ") - 5);

    path.CopyInto(dirpath,0,path.FindLast("/"));
    create_local_directory(&dirpath);

    printf("create a file at |%s|\n",path.String());
    get_or_put("db_get.py",path.String(), db_to_local_filepath(path.String()));
  }
  else if(command.Compare("FOLDER ",7) == 0)
  {
    BString path;
    command.CopyInto(path,7,command.FindLast(" ") - 7);

    printf("create a folder at |%s|\n", path.String());
    create_local_directory(&path);
  }
  else if(command.Compare("REMOVE ",7) == 0)
  {
    BString path;
    command.CopyInto(path,7,command.FindLast(" ") - 7);
    printf("Remove whatever is at |%s|\n", path.String());
    //TODO: actually remove it...
  }
  else
  {
    printf("Did not recognize command.\n");
    return B_ERROR;
  }
  return B_OK;
}

/*
* Sets up the Node Monitoring for Dropbox folder and contents
* and creates data structure for determining which files are deleted or edited
*/
App::App(void)
  : BApplication("application/x-vnd.lh-MyDropboxClient")
{
  //ask Dropbox for deltas!
  BString *delta_commands = run_script("python db_delta.py");
  BString line;
  while(get_next_line(delta_commands,&line) == B_OK)
  {
    (void) parse_command(line); //TODO: do something more appropriate with return
  }

  //start watching ~/Dropbox folder contents (create, delete, move)
  BDirectory dir(local_path_string_noslash); //don't use ~ here
  node_ref nref;
  status_t err;
  if(dir.InitCheck() == B_OK){
    dir.GetNodeRef(&nref);
    err = watch_node(&nref, B_WATCH_DIRECTORY, BMessenger(this));
    if(err != B_OK)
      printf("Watch Node: Not OK\n");
  }

  // record each file in the folder so that we know the name on deletion
  BEntry *entry = new BEntry;
  status_t err2;
  err = dir.GetNextEntry(entry);
  BPath *path;
  BFile *file;
  //TODO: refactor this part out into a new function
  while(err == B_OK) //loop over files
  {
    //put this file in global list
    file = new BFile(entry, B_READ_ONLY);
    this->tracked_files.AddItem((void*)(file));

    //add filepath to global list
    path = new BPath;
    entry->GetPath(path);
    this->tracked_filepaths.AddItem((void*)path);

    printf("tracking: %s\n",path->Path());

    err2 = entry->GetNodeRef(&nref);
    if(err2 == B_OK)
    {
      if(file->IsDirectory())
      {
        err2 = watch_node(&nref, B_WATCH_STAT|B_WATCH_DIRECTORY, be_app_messenger);
        if(err2 != B_OK)
          printf("Watch folder Node %s: Not OK\n", path->Path());

        //TODO: recurse to track this folder's contents
      }
      else
      {
        err2 = watch_node(&nref, B_WATCH_STAT, be_app_messenger); //watch for edits
        if(err2 != B_OK)
          printf("Watch file Node %s: Not OK\n", path->Path());
      }
    }

    //increment loop variables
    entry = new BEntry; //TODO: don't make an new entry each time. They don't go in the list anyway.
    err = dir.GetNextEntry(entry);
  }
  delete entry;
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
    case B_NODE_MONITOR: //TODO: Refactor cases out into own functions
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

            // unpack the message
            msg->FindInt32("device",&ref.device);
            msg->FindInt64("directory",&ref.directory);
            msg->FindString("name",&name);
            printf("name:%s\n",name);
            ref.set_name(name);
            BEntry new_file = BEntry(&ref);
            if(new_file.IsDirectory())
            {
               printf("Actually, it's a directory!\n");
               //add to Dropbox
               new_file.GetPath(&path);
               add_folder_to_dropbox(path.Path());

               //do I need a global data structure for BDirectories?

               //track folder
               node_ref nref;
               err = new_file.GetNodeRef(&nref);
               if(err == B_OK)
               {
                 err = watch_node(&nref, B_WATCH_STAT | B_WATCH_DIRECTORY, be_app_messenger);
                 if(err != B_OK)
                   printf("Watch new folder %s: Not Ok.\n", path.Path());
               }

            }
            else //it's a file (or sym link)
            {
              // add the new file to Dropbox
              new_file.GetPath(&path);
              add_file_to_dropbox(path.Path());

              // add the new file to global tracking lists
              BFile *file = new BFile(&new_file, B_READ_ONLY);
              this->tracked_files.AddItem((void*)file);
              BPath *path2 = new BPath;
              new_file.GetPath(path2);
              this->tracked_filepaths.AddItem((void*)path2);

              // listen for EDIT alerts on the new file
              node_ref nref;
              err = new_file.GetNodeRef(&nref);
              if(err == B_OK)
              {
                err = watch_node(&nref, B_WATCH_STAT, be_app_messenger);
                if(err != B_OK)
                  printf("Watch new file %s: Not Ok.\n", path2->Path());
              }
            }
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
            BFile *filePtr;
            int32 ktr = 0;
            int32 limit = this->tracked_files.CountItems();
            printf("About to loop %d times\n", limit);
            while((filePtr = (BFile*)this->tracked_files.ItemAt(ktr))&&(ktr<limit))
            {
              printf("In loop.\n");
              filePtr->GetNodeRef(&cref);
              printf("GotNodeRef\n");
              if(nref == cref)
              {
                printf("Deleting it\n");
                BPath *path = (BPath*)this->tracked_filepaths.ItemAt(ktr);
                printf("%s\n",path->Path());
                delete_file_on_dropbox(path->Path());

                this->tracked_files.RemoveItem(ktr);
                this->tracked_filepaths.RemoveItem(ktr);
                break; //break out of loop
              }
              ktr++;
            }
            break;
          }
          case B_STAT_CHANGED:
          {
            printf("EDITED FILE\n");
            node_ref nref1,nref2;
            msg->FindInt32("device",&nref1.device);
            msg->FindInt64("node",&nref1.node);
            BFile * filePtr;
            int32 ktr = 0;
            while((filePtr = (BFile *)this->tracked_files.ItemAt(ktr++)))
            {
              filePtr->GetNodeRef(&nref2);
              if(nref1 == nref2)
              {
                BPath *path;
                path = (BPath*)this->tracked_filepaths.ItemAt(ktr-1);
                update_file_in_dropbox(path->Path());
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
