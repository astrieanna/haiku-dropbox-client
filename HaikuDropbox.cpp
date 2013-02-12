#include <stdio.h>
#include <stdlib.h>

#include "App.h"
#include <Window.h>
#include <Button.h>
#include <View.h>
#include <String.h>
#include <Directory.h>
#include <NodeMonitor.h>

enum
{
  M_BUTTON_CLICKED = 'btcl'
};

App::App(void)
  : BApplication("application/x-vnd.lh-MyDropboxClient")
{
  fCount = 0;

  BRect frame(100,100,500,400);
  myWindow = new BWindow(frame,"Dropbox FTW"
                  , B_TITLED_WINDOW
                  , B_ASYNCHRONOUS_CONTROLS
                   | B_QUIT_ON_WINDOW_CLOSE);

  BButton *button = new BButton(BRect(10,10,11,11),"button","Click me!"
                                , new BMessage(M_BUTTON_CLICKED));
  button->SetTarget(this);
  button->ResizeToPreferred();
  myWindow->AddChild(button);

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

  myWindow->Show();
}

void
App::MessageReceived(BMessage *msg)
{
  switch(msg->what)
  {
    case M_BUTTON_CLICKED:
    {
      fCount++;
      BString labelString("Clicks:");
      labelString << fCount;
      myWindow->SetTitle(labelString.String());
      break;
    }
    case B_NODE_MONITOR:
    {
      myWindow->SetTitle("ALLLERT!");
      printf("Received Node Monitor Alert\n");
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
