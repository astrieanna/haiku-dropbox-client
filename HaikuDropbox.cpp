#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "App.h"
#include <NodeMonitor.h>
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
      output->Append(buf);
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
  one_path_arg("db_rm.py",local_to_db_filepath(filepath));
}

/*
* Given the local file path of a new file,
* run the script to upload it to Dropbox
*/
void
add_file_to_dropbox(const char * filepath)
{
  get_or_put("db_put.py",filepath, local_to_db_filepath(filepath));
}

/*
* Given the local file path of a new folder,
* run the script to mkdir on Dropbox
*/
void
add_folder_to_dropbox(const char * filepath)
{
  one_path_arg("db_mkdir.py",local_to_db_filepath(filepath));
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
  printf("Local file moved. TODO: sync to remote");
}

/*
* Given a local file path,
* update the corresponding file on Dropbox
*/
void
update_file_in_dropbox(const char * filepath)
{
  printf("File edited locally. TODO: properly sync %s to dropbox.\n", filepath);
  add_file_to_dropbox(filepath); //need to utilize parent_rev
}

//Local filesystem stuff

//TODO: pick better default permissions...
void
create_local_directory(BString *dropbox_path)
{
    BString *local_path = new BString(local_path_string);
    local_path->Append(*dropbox_path);
    status_t err = create_directory(local_path->String(), 0x0777);
    printf("Create local dir %s: %s\n",local_path->String(),strerror(err));
}

void
watch_entry(const BEntry *entry, int flag)
{
  node_ref nref;
  status_t err;

  err = entry->GetNodeRef(&nref);
  if(err == B_OK)
  {
    err = watch_node(&nref, flag, be_app_messenger);
    if(err != B_OK)
      printf("watch_entry: Not Ok.\n");
  }
}

/*
* Given a BEntry* representing a file (or folder)
* add the relevant BFile and BPath to the global tracking lists
* (tracked_files and tracked_filepaths)
*/
void
App::track_file(BEntry *new_file)
{
  BFile *file = new BFile(new_file, B_READ_ONLY);
  this->tracked_files.AddItem((void*)file);
  BPath *path = new BPath;
  new_file->GetPath(path);
  this->tracked_filepaths.AddItem((void*)path);
}

void
App::recursive_watch(BDirectory *dir)
{
  status_t err;

  BEntry entry;
  err = dir->GetNextEntry(&entry);

  //for each file in the current directory
  while(err == B_OK)
  {
    //put this file in global list
    this->track_file(&entry);
    BFile file = BFile(&entry,B_READ_ONLY);
    if(file.IsDirectory())
    {
      watch_entry(&entry,B_WATCH_DIRECTORY);
      BDirectory *ndir = new BDirectory(&entry);
      this->recursive_watch(ndir);
      delete ndir;
    }
    else
    {
      watch_entry(&entry,B_WATCH_STAT);
    }

    err = dir->GetNextEntry(&entry);
  }
}

int32
App::find_nref_in_tracked_files(node_ref target)
{
  node_ref current_nref;
  BFile * current_file;
  int32 ktr = 0;
  int32 limit = this->tracked_files.CountItems();

  while((current_file = (BFile *)this->tracked_files.ItemAt(ktr++)) && ktr<=limit)
  {
    current_file->GetNodeRef(&current_nref);
    if(target == current_nref)
    {
      return --ktr; //account for ++ in while
    }
  }
  return -1;
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
    BString *str = new BString;
    create_local_directory(str);
    delete str;
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

  printf("Done pulling changes, now to start tracking\n");

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

  //watch all the child files for edits and the folders for create/delete/move
  this->recursive_watch(&dir);
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
            entry_ref ref;
            BPath path;
            const char * name;

            // unpack the message
            msg->FindInt32("device",&ref.device);
            msg->FindInt64("directory",&ref.directory);
            msg->FindString("name",&name);
            ref.set_name(name);

            BEntry new_file = BEntry(&ref);
            new_file.GetPath(&path);
            this->track_file(&new_file);

            if(new_file.IsDirectory())
            {
               add_folder_to_dropbox(path.Path());
               this->recursive_watch(&BDirectory(&new_file));
            }
            else
            {
              add_file_to_dropbox(path.Path());
              watch_entry(&new_file,B_WATCH_STAT);
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
            node_ref nref;
            msg->FindInt32("device", &nref.device);
            msg->FindInt64("node", &nref.node);

            int32 index = this->find_nref_in_tracked_files(nref);
            if(index >= 0)
            {
              BPath *path = (BPath*)this->tracked_filepaths.ItemAt(index);
              printf("local file %s deleted\n",path->Path());

              delete_file_on_dropbox(path->Path());
              this->tracked_files.RemoveItem(index);
              this->tracked_filepaths.RemoveItem(index);
            }
            else
            {
              printf("could not find deleted file\n");
            }

            break;
          }
          case B_STAT_CHANGED:
          {
            printf("EDITED FILE\n");
            node_ref nref;
            msg->FindInt32("device", &nref.device);
            msg->FindInt64("node", &nref.node);

            int32 index = this->find_nref_in_tracked_files(nref);
            if(index >= 0)
            {
              BPath *path = (BPath*)this->tracked_filepaths.ItemAt(index);
              update_file_in_dropbox(path->Path());
            }
            else
            {
              printf("Could not find edited file\n");
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
