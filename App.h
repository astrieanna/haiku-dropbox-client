#ifndef APP_H
#define APP_H

#include <Application.h>
#include <List.h>

class App: public BApplication
{
public:
  App(void);
  void MessageReceived(BMessage *msg);
  BList tracked_files; // BFile*
  BList tracked_filepaths; //BPath*
};

#endif

