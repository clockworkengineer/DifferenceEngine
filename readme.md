## C++ File Processing Engine ##

# Introduction #

This is a C++/Linux variant of the JavaScript/Node file processing engine. In its current form it has support for 5 tasks 

1. The copying of files from a watched folder to a specified destination (keeping any source directory structure intact).
2. The conversion of any video files copied to the watch folder to .mp4 (which can now be changed by use of the --extension option) format using HandbrakeCLI and its normal preset. 
3. The running of a shell script command on each file added to the watch folder.
4. The attaching of source file to an email and sending to a given recipient(s). If the URL specifies an IMAP server then the email is appended to a named mailbox on the server.
5. The adding of a file to a specified ZIP archive.

It is run from the command line and typing FPE --help gives a list of its options

    File Processing Engine Application
    Command Line Options:
      --help                       Display help message
      --config arg                 Configuration file name
      -w [ --watch ] arg           Watch folder
      -d [ --destination ] arg     Destination folder
      -t [ --task ] arg            Task number
      --command arg                Shell command to run
      --maxdepth arg (=-1)         Maximum watch depth
      -e [ --extension ] arg       Override destination file extension
      -q [ --quiet ]               Quiet mode (no trace output)
      --delete                     Delete source file
      -l [ --log ] arg             Log file
      -s [ --single ]              Run task in main thread
      -k [ --killcount ] arg (=0)  Files to process before closedown
      -s [ --server ] arg          SMTP server URL and port
      -u [ --user ] arg            Account username
      -p [ --password ] arg        Account username password
      -r [ --recipient ] arg       Recipients(s) for email with attached file
      -m [ --mailbox ] arg         IMAP Mailbox name for drop box
      -a [ --archive ] arg         ZIP destination archive
      --list                       Display a list of supported tasks.

- **config:** Read commands from configuration file. Any values set on the command line but also specified in the configuration will override the file value.
- **Task**: Task number to run (for a list of values see --list).
- **watch:** Folder to watch for files created or moved into.
- **destination:** Destination folder for any processed source files.
- **maxdepth:** The maximum depth is how far down  the directory hierarchy that will be watched (-1 the whole tree, 0 just the watcher folder, 1 the next level down etc).
- **command:** Shell script command to run (task run command) substituting %1% in the command for the source file and %2% for any destination file).
- **extension:** Override the extension on the destination file ( only works with *video* at present).
- **delete:** Delete any source file after successful processing.
- **quiet:** Run in quiet mode i.e. trace output only comes from the main program and not the task class ( thus significantly reducing the amount).
- **log:** Send output to a log file.
- **single:** Run task in main thread instead of creating a separate one.
- **killcount:** Process N files before stopping task.
- **server:** URL address and port number of SMTP mail server for email task.
- **user:** User account name on server for email task
- **password:** User account password  on server for email task
- **recipient:** Recipient(s) for email task email.
- **mailbox**: IMAP mailbox which to append file.
- **archive:** Path to ZIP file archive to which file is added.
- **list:** List available tasks.

**Note I tend to use the term folder/directory interchangeably coming from a mixed development environment.**

# Building #

At present this repository does not contain any build/make scripts but just the raw C++ source files. It will compile/link under Linux, is written in standard C++11, uses the [Boost library APis](http://www.boost.org/) and also the Linux kernel [inotify](https://en.wikipedia.org/wiki/Inotify) file event library. Make-files and build scripts might be deployed in a later time frame especially if the engine gets adapted for other platforms. Note that the sources contains 2 google unit tests under folder 'tests' that require [google test](https://github.com/google/googletest) to be installed on the target platform to build and run.  **Note: The program now uses the master version of the Antikythera classes and no longer relies on a repository copy.**


# Boost #

So that as much of the engine as possible is portable across platforms any functionality that cannot be provided by the C++ STL uses the boost set of library APis. The two main areas that this is used in are the [file-system](http://www.boost.org/doc/libs/1_62_0/libs/filesystem/doc/index.htm) and [parameter parsing](http://www.boost.org/doc/libs/1_62_0/libs/parameter/doc/html/index.html).  Unfortunately the boost file-system does not provide any file watching functionality so for the inaugural version as mentioned earlier  inotify is used.

# File Copy Task Function #

This function takes the file name  passed in as a parameter and copies it to  the the specified destination (--destination). It does this with the aid of boost file system API's. Note that any directories that need to be created in the destination tree for the source path specified are done by BOOST function create_directories().

# Handbrake Video Conversion Task Function #

This function takes takes the file name passed in as a parameter and creates a command to process the file into an ".mp4" file using Handbrake. Please note that this command has a hard encoded path to my installation of Handbrake and should be changed according to the target. The command is passed to a function called **runCommand** which packs the command string into an argv[] that is  passed onto execvp() for execution. Before this a fork is performed and a wait is done for the forked process to exit and its status returned.

# Shell command Task Function #

This executes a simple shell script command (--command) for each file name passed. It uses the same **runCommand** function used by the Handbrake Video Conversion Task.

# Email Task Function #

Take the source file name passed in and attach it to an email that is then sent to recipient(s) using a specified server and account. This function utilizes the CSMTP class to create an email,
attach a file and send it to an SMTP server. The class CSMTP in turn uses libcurl when talking the internet which is written in highly portable 'C' and so available on a multitude of platforms. If the server URL specifies an IMAP server then this function uses class CIMAP to connect to the server and append the created email to a specified mailbox instead of posting the email to any recipients.

# ZIP Archive Task Function #

Take the source file name passed in and add the file to a specified ZIP archive. If the archive does not already exist it is created.

# To Do #

1. Use libcurl to create an ftp copy task action function.



