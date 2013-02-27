# A Dropbox Client for Haiku

A native C++ program for Haiku, that uses the offical Python SDK to talk to Dropbox.
The goal is a program the runs in the background to sync files in the way the official clients do.

The most irritating thing about really trying a new operating system is not having your files.
Without your files, you can't do work.
I am writing a Dropbox client for Haiku so that I'll be able to do more work from Haiku.

##Current Behavior.

This program uses the Dropbox API with permissions to only access it's own App folder.
This makes it safe because it cannot mess up anything in your main Dropbox;
it does not endanger your files.
Until it is thoroughly tested, I'm going to leave it in it's sandbox.
However, the eventual goal is to give it full access so that it can be truly useful.

On startup, the program will pull changes from Dropbox.
The first time you run it, it will delete the ~/Dropbox folder if it exists,
and make a new one, which it will then add all your Dropbox files and folders to.
On subsequent starts, it will pull new changes from Dropbox
-- creating/removing files and folders as instructed by Dropbox.

Changes made locally without the client running will not be detected or sync'd.

Local file/folder creation/deletion will be sync'd.
File editing locally will cause a new file to be added to Dropbox.
Moving/renaming local files will be sync'd to Dropbox.
This includes moving files into or out of the ~/Dropbox folder.

# Dependencies and Compilation

You will need to be running Haiku to compile and run this program.

For the C++ part, just run `make` in the source directory
and it should build an executable named `hdbclient.exe`
in a directory whose name starts with 'object'.

For the Python part to work you'll need to install the Dropbox Python SDK
(which requires installing setuptools).

# Dropbox Authorization

Once you have the Dropbox Python SDK installed,
you'll need to authorize this Dropbox client to access your Dropbox.
It is setup to only have access to its own Apps folder (python_cli),
so none of your existing files are in danger.

##To give the scripts the proper credentials for Dropbox:

1. In the terminal, run: `python db_login1.py`
2. Copy-paste that link into Web+ and approve the access.
3. Run `python db_login2.py`.

You should only have to do this once.
The authorization token is written to a local file,
and then the Python scripts read it in every time they want to talk to Dropbox.

Obviously, this should at some point be a real part of the client.
However, you only need to do it once during setup, so it's not yet a priority.

# Have Fun :)

To play with the client, just run `hdbclient.exe`
and then create/remove/edit/move files in the `~/Dropbox` folder.
You Dropbox files are in no danger because it only has access to Apps/python_cli.
There will be a lot of output to the terminal because I'm still debugging things.
You'll be able to see the files/edits from other devices
with Dropbox clients or from the Dropbox web interface.

# License

This code is under the MIT license.
See the file LICENSE for the full text of it.
