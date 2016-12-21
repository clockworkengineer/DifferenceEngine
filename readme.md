## C++ File Processing Engine ##

# Introduction #

This is a C++/Linux variant of the JavaScript/Node file processing engine. In its current form it has support for 3 tasks 

1. The copying of files from a watched folder to a specified destination (keeping any source directory structure intact).
1. The conversion of any video files copied to the watch folder to .mp4 (which can now be changed by use of the --extension option) format using HandbrakeCLI and its normal preset. 
1. The running of a shell script command on each file added to the watch folder.

It is run from the command line and typing FPE --help gives a list of its options

    File Processing Engine Application
    Options:
      --help 					 	Print help messages
      -w [ --watch ] arg 			Watch Folder
      -d [ --destination ] arg	 	Destination Folder
      --maxdepth arg  				Maximum Watch Depth
      --copy  						Task = File Copy Watcher
      --video 						Task = Video Conversion Watcher
      --command arg   				Task = Run Shell Command
      -e [ --extension ] arg  		Overrde destination file extension
      -q [ --quiet ]                Quiet mode (no trace output)
      --delete						Delete Source File
 
*watch* - Folder to watch for files created or moved into.

*destination* - Destination folder for any processed source files.

*maxdepth* - The maximum depth is how far down  the directory hierarchy that will be watched (-1 the whole tree, 0 just the watcher folder, 1 the next level down etc).

*copy* - Copy file task.

*video* - Video file conversion task.

*command* - Run command task (With this option it takes any file passed through and runs the specified shell script command substituting %1% in the command for the source file and %2% for any destination file).

*extension* Override the extension on the destination file ( only works with *video* at present).

*delete* - delete any source file after successful processing.

*quiet* - Run in quiet mode i.e. trace output only comes from the main program and not the task class ( thus significantly reducing the amount).

**Note I tend to use the term folder/directory interchangeably coming from a mixed development environment.**

# Building #

At present this repository does not contain any build/make scripts but just the raw C++ source files. It will compile/link under Linux, is written in standard C++11, uses the [Boost library APis](http://www.boost.org/) and also the Linux kernel [inotify](https://en.wikipedia.org/wiki/Inotify) file event library. Make-files and build scripts might be deployed in a later time frame especially if the engine gets adapted for other platforms. Note that the sources now contains 3 google unit tests in folder 'tests' that require [google test](https://github.com/google/googletest) to be installed on the target platform to build and run.

# Boost #

So that as much of the engine as possible is portable across platforms any functionality that cannot be provided by the C++ STL uses the boost set of library APis. The two main areas that this is used in are the [file-system](http://www.boost.org/doc/libs/1_62_0/libs/filesystem/doc/index.htm) and [parameter parsing](http://www.boost.org/doc/libs/1_62_0/libs/parameter/doc/html/index.html).Unfortunately the boost file-system does not provide any file watching functionality so for the inaugural version as mentioned earlier  inotify is used. Note: The task class itself no longer uses boost and the only reliance on it is in the main program and the task action functions.

# Task Class #

The core for the file processing engine is provided by the FPE_Task class whose constructor takes five arguments, the task name (std::string), the folder to be watched (std::string), an integer specifying the watch depth (-1=all,0=just watch folder,1=next level down etc.) a pointer to a task action function that is called for each file that is copied/moved into the watch folder hierarchy and a pointer to data that may be needed by the action function.

The engine comes with three built in variants of the task action function, a file copy, file handbrake encoder and a run shell script (any new function should adhere to these functions template). To start watching/processing files call this classes monitor function; the code within FPE.cpp creates a separate thread for this but it can be run in the main programs thread by just calling task.monitor() without any thread creation wrappper code.

At the center of the class is an event loop which waits for inotify events and performs processing according to what type of event. If its a new folder that has appeared then a new watch is added for it, if a folder has disappeared then the watch for it is removed. Note that it is possible for a watch to be removed for a directory only to be recreated if it is moved within the hierarchy. The last type of event is for normal files that have been created; these are just sent to be processed by the user defined function passed in on creation.

The FPE_Task class has a separate worker thread which is created to handle files to be processed. The reason for this being is that if the processing required is intensive as in Handbrake video conversion then having this done on the same thread can cause the inotify event queue to stall. Any files to be processed are added to a queue which the worker thread then takes off to process. Access to this queue is controlled by a mutex and a conditional variable which waits in the worker thread until a file is queued and a notify sent by the monitor.

It should be noted that a basic shutdown protocol is provided to close down any threads that the task class uses by calling task.stop(). This just sets an internal atomic boolean called bDoWork to false so that both the internal loops stop and the functions exit gracefully and so do the threads. This is just to give some control over thread termination which the C++ STL doesn't really provide; well in a subtle manner anyways. The shutdown can be actuated by either deleting the watch folder or by specifying a kill count in the optional task options structure parameter that can be passed in the classes constructor.

The task options structure parameter also has two other members which are pointers to functions that handle all cout/cerr output from the class. These take as a parameter a vector of strings to output and if the option parameter is omitted or the pointers are nullptr then no output occurs. The FPE provides these two functions in the form of coutstr/coutstr which are passed in if --quiet is not specified nullptrs otherwise. All output is modeled this way was it enables the two functions in the FPE to use a mutex to control access to the output streams which are not thread safe and also to provide a --quiet mode and when it is implemented a output to log file option.

# File Copy Task Function #

This function takes the file name  passed in as a parameter and copies it to  the the specified destination (--destination). It does this with the aid of boost file system API's. Note that any directories that need to be created in the destination tree for the source path specified are done by BOOST function create_directories().

# Handbrake Video Conversion Task Function #

This function takes takes the file name passed in as a parameter and creates a command to process the file into an ".mp4" file using Handbrake. Please note that this command has a hard encoded path to my installation of Handbrake and should be changed according to the target. The command is passed to a function called **runCommand** which packs the command string into an argv[] that is  passed onto execvp() for execution. Before this a fork is performed and a wait is done for the forked process to exit and its status returned.

# Shell command Task Function #

This executes a simple shell script command (--command) for each file name passed. It uses the same **runCommand** function used by the Handbrake Video Conversion Task.

# To Do #

1. Add more task options such a using curl.
2. Add more google tests.
3. Look into using std::async instead of raw threads with task class.
4. Separate out inotify functionality from task class.



