#include <stdio.h>
#include <stdlib.h>

#include "App.h"
#include <Window.h>
#include <Button.h>
#include <View.h>
#include <String.h>
#include <Directory.h>
#include <NodeMonitor.h>

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
run_script(char *cmd)
{
  char buf[BUFSIZ];
  FILE *ptr;

  if ((ptr = popen(cmd, "r")) != NULL)
    while (fgets(buf, BUFSIZ, ptr) != NULL)
      (void) printf("RAWR%s", buf);
  (void) pclose(ptr);
  return 0;
}

int
main(void)
{
  //Haiku make window code
  App *app = new App();

  //run some Dropbox code
  run_script("python db_ls.py");
  app->Run();
  delete app;
  return 0;
}
