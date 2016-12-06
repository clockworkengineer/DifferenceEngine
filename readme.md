## C++ File Processing Engine ##

# Introduction #

This is a C++/Linux variant of the JavaScript/Node file processing engine. In its current form it has support for 3 tasks 

1. The copying of files from a watched folder to a specified destination (keeping any source directory structure intact
1. The conversion of any video files copied to the watch folder to .mp4 format using HandbrakeCLI and its normal preset. 
1. The running of a shell script command on each file added to the watch folder.

It is run from the command line and typing FPE --help gives its options

    File Processing Engine Application
    Options:
      --help                          Print help messages
      -w [ --watch ] arg              Watch Folder
      -d [ --destination ] arg        Destination Folder
      --maxdepth arg                  Maximum Watch Depth
      --copy                          Task = File Copy Watcher
      --video                         Task = Video Conversion Watcher
      --command arg                   Task = Run Shell Command
      --delete                        Delete Source File
 
*watch* - Folder to watch for files created or moved into.

*destination* - Destination folder for any processed source files.

*maxdepth* - The maximum depth is how far down  the directory hierarchy that will be watched (-1 the whole tree, 0 just the watcher folder, 1 the next level down etc).

*copy* - Copy file task.

*video* - Video file conversion task.

*command* - Run command task (With this option it takes any file passed through and runs the specified shell script command substituting %1% in the command for the source file and %2% for any destination file).

*delete* - delete any source file after successful processing.

**Note I tend to use the term folder/directory interchangeably coming from a mixed development environment.**

# Building #

At present this repository does not contain any build/make scripts but just the raw C++ source files. It will compile/link under Linux, is written in standard C++11, uses the [Boost library APis](http://www.boost.org/) and also the Linux kernel [inotify](https://en.wikipedia.org/wiki/Inotify) file event library. Make-files and build scripts might be deployed in a later time frame especially if the engine gets adapted for other platforms.

# Boost #

So that as much of the engine as possible is portable across platforms any functionality that cannot be provided by the C++ STL uses the boost set of library APis. The two main areas that this is used in are the [file-system](http://www.boost.org/doc/libs/1_62_0/libs/filesystem/doc/index.htm) and [parameter parsing](http://www.boost.org/doc/libs/1_62_0/libs/parameter/doc/html/index.html).Unfortunately the boost file-system does not provide any file watching functionality so for the inaugural version as mentioned earlier  inotify is used.

# Task Class #

The core for the file processing engine is provided by the FPE_Task class whose constructor takes five arguments, the task name (std::string), the folder to be watched (std::string), an integer specifying the watch depth (-1=all,0=just watch folder,1=next level down etc.) a pointer to a task action function that is called for each file that is copied/moved into the watch folder hierarchy and a pointer to data that may be needed by the action function.

The engine comes with three built in variants of this function, a file copy, file handbrake encoder an a run shell script (any new function should adhere to these functions template). To start watching/processing files call this classes monitor function; the code within FPE.cpp creates a separate thread for this but it can be run in the main programs thread by just calling task.monitor() without any thread creation wrappper code.

At the center of the class is an event loop which waits for inotify events and performs processing according to what type of event. If its a new folder that has appeared then a new watch is added for it, if a folder has disappeared then the watch for it is removed. Note that it is possible for a watch to be removed for a directory only to be recreated if it is moved within the hierarchy. The last type of event is for normal files that have been created; these are just sent to be processed by the user defined function passed in on creation.

Here is a good time to mention that the FPE_Task class has a separate worker thread which is created to handle files to be processed. The reason for this is that if the processing required is intensive as in handbrake video conversion then having this done on the same thread can cause the inotify event queue to stall. Any files to be processed are added to a queue which the worker thread then takes off to process. Access to this queue is controlled by a lock guard mutex so that no race type conditions should arise.

It should be noted that a basic shutdown protocol is provided to close down any threads that the task class uses by calling task.stop(). This just sets an internal atomic boolean called bDoWork to false so that both the internal loops stop and the functions exit gracefully and so do the threads. This is just to give some control over thread termination which the C++ STL doesn't really provide well in a subtle manner anyways.

# File Copy Task Function #

This function takes the file name and path passed as parameters and copies the combination file source  to  the the specified destination (--destination). It does this with the aid of boost file system API's. Note that any directories that need to be created in the destination tree for the source path specified are done by BOOST function create_directories().

# Handbrake Video Conversion Task Function #

This function takes the file name and path passed as parameters and creates a command to process the file into an ".mp4" file using handbrake. Please note that this command has a hard encoded path to my installation of handbrake and should be changed according to the target. The command is passed to the OS using the function call system(), which waits for completion and a returned status (nothing fancy is done with this except report success or failure). Note all output from handbrake (stdout/stderr) is  also redirected to a hard encoded log file.

# To Do #

1. The ability to drive program from tasksToRunDetails.json
1. Add more task options such a torrent file download
2. Add more google tests.



