#ifndef APP_H
#define APP_H

#include <Application.h>
#include <List.h>
#include <Directory.h>
#include <Entry.h>
#include <Node.h>
#include <MessageRunner.h>

class App: public BApplication
{
public:
  App(void);
  void MessageReceived(BMessage *msg);
private:
  BList tracked_files; // BFile*
  BList tracked_filepaths; //BPath*
  BMessageRunner *msg_runner;
  int32 find_nref_in_tracked_files(node_ref target);
  void recursive_watch(BDirectory *dir);
  void track_file(BEntry *new_file);
  int parse_command(BString command);
  void pull_and_apply_deltas();
};

#endif

