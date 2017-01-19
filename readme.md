## C++ File Processing Engine ##

# Introduction #

This is a C++/Linux variant of the JavaScript/Node file processing engine. In its current form it has support for 4 tasks 

1. The copying of files from a watched folder to a specified destination (keeping any source directory structure intact).
2. The conversion of any video files copied to the watch folder to .mp4 (which can now be changed by use of the --extension option) format using HandbrakeCLI and its normal preset. 
3. The running of a shell script command on each file added to the watch folder.
4. The attaching of source file to an email and sending to a given recipient(s).

It is run from the command line and typing FPE --help gives a list of its options

    File Processing Engine Application

    Command Line Options:
      --help   						Display help message
      --config arg 					Configuration file name
      --email  						Task = Email File Attachment
      --copy   						Task = File Copy Watcher
      --video  						Task = Video Conversion Watcher
      --command arg					Task = Run Shell Command
      -w [ --watch ] arg   			Watch Folder
      -d [ --destination ] arg 		Destination Folder
      --maxdepth arg   				Maximum Watch Depth
      -e [ --extension ] arg   		Override destination file extension
      -q [ --quiet ]   				Quiet mode (no trace output)
      --delete 						Delete Source File
      -l [ --log ] arg 				Log file
      -s [ --single ]  				Run task in main thread
      -k [ --killcount ] arg   		Files to process before closedown
      -s [ --server ] arg  			SMTP Server URL and port
      -u [ --user ] arg				Account username
      -p [ --password ] arg			Account username password
      -r [ --recipient ] arg   		Recipients(s) for email with attached file

- **config;** Read commands from configuration file. Any values set on the command line but also specified in the configuration will override the file value.
- **watch:** Folder to watch for files created or moved into.
- **destination:** Destination folder for any processed source files.
- **maxdepth:** The maximum depth is how far down  the directory hierarchy that will be watched (-1 the whole tree, 0 just the watcher folder, 1 the next level down etc).
- **email:** Email file as attachment task
- **copy:** Copy file task.
- **video:** Video file conversion task.
- **command:** Run command task (With this option it takes any file passed through and runs the specified shell script command substituting %1% in the command for the source file and %2% for any destination file).
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

**Note I tend to use the term folder/directory interchangeably coming from a mixed development environment.**

# Building #

At present this repository does not contain any build/make scripts but just the raw C++ source files. It will compile/link under Linux, is written in standard C++11, uses the [Boost library APis](http://www.boost.org/) and also the Linux kernel [inotify](https://en.wikipedia.org/wiki/Inotify) file event library. Make-files and build scripts might be deployed in a later time frame especially if the engine gets adapted for other platforms. Note that the sources now contains 3 google unit tests in folder 'tests' that require [google test](https://github.com/google/googletest) to be installed on the target platform to build and run.

# Boost #

So that as much of the engine as possible is portable across platforms any functionality that cannot be provided by the C++ STL uses the boost set of library APis. The three main areas that this is used in are the [file-system](http://www.boost.org/doc/libs/1_62_0/libs/filesystem/doc/index.htm) and [parameter parsing](http://www.boost.org/doc/libs/1_62_0/libs/parameter/doc/html/index.html).  Unfortunately the boost file-system does not provide any file watching functionality so for the inaugural version as mentioned earlier  inotify is used. Note: The CFileTask/CFileApprise classes no longer uses boost and the only reliance on it is in the main program and the task action functions.

# [Task Class](https://github.com/clockworkengineer/difference_engine/blob/master/classes/CFileTask.cpp) #

The core for the file processing engine is provided by the CFileTask class whose constructor takes five arguments, 

- **taskName:** The task name (std::string).
- **watchFolder:** The folder to be watched (std::string).
- **taskActFcn:** A pointer to a task action function that is called for each file that is copied/moved into the watch folder hierarchy.
- **fnData:** A pointer to data that may be needed by the action function.
- **watchDepth:** An integer specifying the watch depth (-1=all,0=just watch folder,1=next level down etc.)
- **options:**(optional) This structure passes in values used in any low level functionality (ie. killCount) and can be implementation specific such as providing a trace routine for low level inotify events or  pointers to the generic coutsr/coutstr trace functions.

The engine comes with three built in variants of the task action function, a file copy, file handbrake encoder and a run shell script (any new function should adhere to these functions template). To start watching/processing files call this classes monitor function; the code within FPE.cpp creates a separate thread for this but it can be run in the main programs thread by just calling task.monitor() without any thread creation wrappper code (--single  option).

At the center of the class is an event loop which waits for CFileApprise events and calls the passed action function to process any file name passed through as part of an add event. The CFileApprise class is new and is basically an encapsulation of all of the inotify file event handling  code that used to reside in the Task class with an abstraction layer to hide any platform specifics. In the future new platform specific versions of this class may be implemented for say MacOS or Windows; in creating this class the one last non portable component of the FPE has been isolated thus aiding porting in future. As a result of this new class the task class is a lot simpler and smaller with all of the functionality being offloaded. The idea of making Task a child of the CFileApprise base class had been thought about but for present a task just creates and uses an private CFileApprise watcher object.

It should be noted that a basic shutdown protocol is provided to close down any threads that the task class uses by calling task.stop(). This now in turn calls the CFileApprise objects stop method which stops its internal event reading loop and performs any closedown of the CFileApprise object and thread.  This is just to give some control over thread termination which the C++ STL doesn't really provide; well in a subtle manner anyways. The shutdown can be actuated as well by either deleting the watch folder or by specifying a kill count in the optional task options structure parameter that can be passed in the classes constructor.

The task options structure parameter also has two other members which are pointers to functions that handle all cout/cerr output from the class. These take as a parameter a vector of strings to output and if the option parameter is omitted or the pointers are nullptr then no output occurs. The FPE provides these two functions in the form of coutstr/coutstr which are passed in if --quiet is not specified nullptrs otherwise. All output is modeled this way was it enables the two functions in the FPE to use a mutex to control access to the output streams which are not thread safe and also to provide a --quiet mode and when it is implemented a output to log file option.

# [CFileApprise Class](https://github.com/clockworkengineer/difference_engine/blob/master/classes/CFileApprise.cpp) #

This is class was created to be a standalone class / abstraction of the inotify file event handling code that used to be contained in CFileTask. 

Its constructor has 3 parameters:

- **watchFolder:** Folder to watch for files created or moved into.
- **watchDepth:**  The watch depth is how far down the directory hierarchy that will be watched (-1 the whole tree, 0 just the watcher folder, 1 the next level down etc).
- **options:**(optional) This structure passes in values used in any low level functionality and can be implementation specific such as providing a trace routine for low level inotify events or  pointers to the generic coutsr/coutstr trace functions.

Once the object is created then its core method CFileApprise::watch() is run on a separate thread that is used to generate events from actions on files using inotify. While this is happening the main application loops  waiting on events returned by method CFileApprise::getEvent().

The current supported event types being

    enum EventId { 
    	Event_none=0,   	// None
    	Event_add,  		// File added to watched folder hierachy
    	Event_change,   	// File changed
    	Event_unlink,   	// File deleted from watched folder hierachy
    	Event_addir,		// Directory added to watched folder hierachy
    	Event_unlinkdir,	// Directory deleted from watched folder hierachy
    	Event_error 		// Exception error
    };

and they are contained within a structure of form

    struct Event {
    	EventId id;				// Event id
    	std::string message;   	// Event file name / error message string
    };
    
Notes: 

- Events *addir*/unlinkdir will result in new watch folders being added/removed from the internal watch table maps (depending on the value of watchDepth).
- The change event is currently unsupported and not required by CFileTask but is penciled in to be added in future.

# [Redirect Class](https://github.com/clockworkengineer/difference_engine/blob/master/classes/CRedirect.cpp) #

This is a small self contained utility class designed for FPE logging output. Its prime functionality is to provide a wrapper for pretty generic code that saves away an output streams read buffer, creates a file stream and redirects the output stream to it. The code to restore the original output streams is called from the objects destructor thus providing convenient for restoring the original stream. Its primary use within the FPE is to redirect std::cout to a log file.

# Exceptions #

Both the CFileTask and CFileApprise classes are designed to run in a separate thread although the former can run in the main thread quite happily. As such any exceptions thrown by them could be lost and so that they are not a copy is taken inside each objects main catch clause and stored away in a std::exception_ptr. This value can then by retrieved with method getThrownException() and either re-thrown if the and of the chain has been reached or stored away again to retrieved by another getThrownException() when the enclosing object closes down (as in the case CFileTask and CFileApprise class having thrown the exception).

# File Copy Task Function #

This function takes the file name  passed in as a parameter and copies it to  the the specified destination (--destination). It does this with the aid of boost file system API's. Note that any directories that need to be created in the destination tree for the source path specified are done by BOOST function create_directories().

# Handbrake Video Conversion Task Function #

This function takes takes the file name passed in as a parameter and creates a command to process the file into an ".mp4" file using Handbrake. Please note that this command has a hard encoded path to my installation of Handbrake and should be changed according to the target. The command is passed to a function called **runCommand** which packs the command string into an argv[] that is  passed onto execvp() for execution. Before this a fork is performed and a wait is done for the forked process to exit and its status returned.

# Shell command Task Function #

This executes a simple shell script command (--command) for each file name passed. It uses the same **runCommand** function used by the Handbrake Video Conversion Task.

# Email Task Function #

Take the source file name passed in and attach it to an email that is then sent to recipient(s) using a specified server and account. This function utilizes the CMailSend class to create an email,
attach a file and send it to an SMTP server. The class CMailSend in turn uses libcurl when talking the internet which is written in highly portable 'C' and so available on a multitude of platforms.


# To Do #

1. Look into using std::async instead of raw threads with task class.



