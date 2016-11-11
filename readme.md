## C++ File Processing Engine ##

# Introduction #

This is a C++/Linux variant of the JavaScript/Node file processing engine. In its current form it has support for the copying of files from a watched folder to a specified destination (keeping any source directory structure intact) and also the conversion of any video files copied to the watch folder to .mp4 format using HandbrakeCLI and its normal preset. It as run from the command line and typing FPECPP --help gives its options

    Options:
      --help   Print help messages
      -w [ --watch ] arg   Watch Folder
      -d [ --destination ] arg Destination Folder
      --copy   File Copy Watcher
      --video  Video Conversion Watcher

Both watch and destination Folders are mandatory but if copy and  video parameters are missing it defaults to file. Note I tend to use the term folder/directory interchangeably coming from a mixed development environment.

# Building #

At present this repository does not contain any build/make scripts but just the raw C++ source files. It will compile/link under Linux, is written in standard C++11, uses the [Boost library APis](http://www.boost.org/) and also a third party API called [inotify-cxx](https://github.com/wankdanker/svn.aiken.cz/tree/master/inotify-cxx/tags/inotify-cxx-0.7.1) that provides a wrapper to Linux kernel [inotify](https://en.wikipedia.org/wiki/Inotify) file event library. Make-files and build scripts might be deployed in a later time frame especially if the engine gets adapted for other platforms.

# Boost #

So that as much of the engine as possible is portable across platforms any functionality that cannot be provided by the C++ STL uses the boost set of library APis. The two main areas that this is used in is the [file-system](http://www.boost.org/doc/libs/1_62_0/libs/filesystem/doc/index.htm) and [parameter parsing](http://www.boost.org/doc/libs/1_62_0/libs/parameter/doc/html/index.html).Unfortunately the boost file-system does not provide any file watching functionality so for the inaugural version as mentioned earlier  inotify is used.

# Task Class #

The core for the file processing engine is provided by the FPETask class which constructor takes three arguments the task name (std::string), the folder to be watch (std::string) and a pointer to a function that is called for each file that is copied/moved into the watch folder hierarchy. This is virtually identical to how the class is created in the JavaScript variant except that a process is provided. The engine comes with two variants of this function, file copy and file handbrake encoding (any new function should adhere to these functions template). To start watching/processing files call this class monitor function; the code within FPECPP.cpp creates a separate thread for this but it can be run in the main programs thread by just calling task.monitor().

At the center of the class is an event loop which waits for inotify events and performs processing according to what type of event. If its a new folder that has appeared then a new watch is added for it, if a folder has disappeared then the watch for it is removed. Note that it is possible for a watch to be removed for a directory only to be recreated if it is moved within the hierarchy. The last type of event is for normal files that have been created; these are just sent to be processed by the user defined function passed in on creation.

Here is a good time to mention that the FPETask class has a separate worker thread which is created to handle files to be processed. The reason for this is that if the processing required is intensive as in handbrake video conversion then having this done on the same thread can cause inotify event queue to stall. Any files to be processed are added to a queue which the worker thread takes off to process each file. Access to this queue is controlled by a lock guard mutex so that no race type conditions should arise.

It should be noted that a basic shutdown protocol is provided to close down any threads that the task class uses by calling task.stop(). This just sets an internal atomic boolean called doWork to false so that both the internal loops stop and the functions exit gracefully and so do the threads. This is just to give some control over thread termination which the C++ STL doesn't really provide well in a subtle manner anyways.

# File Copy Function #

This function takes the file name and path passed as parameters and copies the combination file source  to  the the specified destination (--destination). It does this with the aid of boost file system API's. Note that any directories that need to be created in destination tree for the source path specified are done by function create_directories().

# Handbrake video conversion function #

This function takes the file name and path passed as parameters and creates a command to process the file into an ".mp4" using handbrake. Please note that this command has a hard encoded path to my installation of handbrake and should be changed according to the target. The command is passed to the OS using the call system() and waits for completion and a returned status (nothing fancy is done with this except report sucess or failure). Note all output from handbrake (stdout/stderr) is  also redirected to a hard encoded log file.

