/*
 * File:   FPE_ProcCmdLine.hpp
 * 
 * Author: Robert Tizzard
 *
 * Created on October 24, 2016, 2:34 PM
 *
 * Copyright 2016.
 * 
 */

#ifndef FPE_PROCCMDLINE_HPP
#define FPE_PROCCMDLINE_HPP

// C++ STL definitions

#include <string>

// Command line parameter data

struct ParamArgData {
    std::string  watchFolder;        // Watch Folder
    std::string  destinationFolder;  // Destination Folder for copies.
    std::string  commandToRun;       // Command to run
    bool         bFileCopy;          // Task file copy
    bool         bVideoConversion;   // Task video conversion
    bool         bRunCommand;        // Task perform command
    int          maxWatchDepth;      // Watch depth -1=all,0=just watch folder,1=next level down etc.
    bool         bDeleteSource;      // Delete source file
    std::string  extension;          // Override destination file extension
    bool         bQuiet;             // Quiet mode no tracing output.
    int          killCount;          // Kill Count
    bool         bSingleThread;      // Run task in the main thread
    std::string  logFileName;        // Log file name
    std::string  configFileName;     // Config file name
};

// Handbrake command

const std::string kHandbrakeCommand = "/usr/local/bin/HandBrakeCLI -i %1% -o %2% --preset=\"Normal\"";

void procCmdLine (int argc, char** argv, ParamArgData& argData);

#endif /* FPE_PROCCMDLINE_HPP */

