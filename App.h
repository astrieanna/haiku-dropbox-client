#ifndef APP_H
#define APP_H

#include <Application.h>
#include <List.h>

class App: public BApplication
{
public:
  App(void);
  void MessageReceived(BMessage *msg);
private:
  BList tracked_files; // BFile*
};

#endif

