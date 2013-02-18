# A Dropbox Client for Haiku

Currently:
While running, will push new and modified files in the /boot/home/Dropbox folder to Dropbox and delete files that are rm'd locally. (Only changes made while the client is running will be sync'd to Dropbox)

# Dependencies and Compilation

You will need to be running Haiku to compile and run this program. For the C++ part, just run `make` in the source directory and it should build an executable named `hdbclient.exe` in a directory whose name starts with 'object'. For the Python part to work you'll need to install the Python SDK (which requires install setuptools).

# Dropbox Authorization

Once you have the Dropbox Python SDK installed, you'll need to authorize this Dropbox client to access your Dropbox. It is setup to only have access to its own Apps folder (python_cli), so none of your existing files are in danger.

To give the scripts the proper credentials:

1. In the terminal, run: `python db_login1.py`
2. Copy-paste that link into Web+ and approve the access.
3. Run `python db_login2.py`.

You should only have to do this once. The authorization token is written to a local file, and then the Python scripts read it in every time they want to talk to Dropbox.

# Have Fun :)

To play with the client, just run `hdbclient.exe` and then create/remove/edit files in the `~/Dropbox` folder. You Dropbox files are in no danger because it only has access to Apps/python_cli. There will be a lot of output to the terminal because I'm still debugging things.
You'll be able to see the files/edits from other devices with Dropbox clients or from the Dropbox web interface.