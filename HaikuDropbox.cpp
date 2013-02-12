#include <stdio.h>
#include <stdlib.h>

#include "App.h"
#include <Window.h>
#include <Button.h>
#include <View.h>
#include <String.h>

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
    default:
    {
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
  App *app = new App();
  app->Run();
  delete app;
  run_script("python db_ls.py");
  return 0;
}
