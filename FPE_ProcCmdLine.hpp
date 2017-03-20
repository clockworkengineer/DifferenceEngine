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

//
// C++ STL definitions
//

#include <string>

// Command line parameter data

struct ParamArgData {
    std::string  watchFolderStr;        // Watch Folder
    std::string  destinationFolderStr;  // Destination Folder for copies.
    std::string  commandToRunStr;       // Command to run
    bool         bFileCopy;             // Task file copy
    bool         bVideoConversion;      // Task video conversion
    bool         bRunCommand;           // Task perform command
    bool         bZipArchive;           // Task add to ZIP archive
    int          maxWatchDepth;         // Watch depth -1=all,0=just watch folder,1=next level down etc.
    bool         bDeleteSource;         // Delete source file
    std::string  extensionStr;          // Override destination file extension
    bool         bQuiet;                // Quiet mode no tracing output.
    int          killCount;             // Kill Count
    bool         bSingleThread;         // Run task in the main thread
    std::string  logFileNameStr;        // Log file name
    std::string  configFileNameStr;     // Config file name
    bool         bEmailFile;            // Email or save file to Inbox.
    std::string  userNameStr;           // Email account user name
    std::string  userPasswordStr;       // Email account user name password
    std::string  serverURLStr;          // Email SMTP/IMAP server URL
    std::string  emailRecipientStr;     // Email recipient
    std::string  mailBoxNameStr;        // IMAP Mailbox name
    std::string  zipArchiveStr;         // Destination ZIP Archive

};

// Handbrake command

const std::string kHandbrakeCommandStr = "/usr/local/bin/HandBrakeCLI -i %1% -o %2% --preset=\"Normal\"";

// Process command line parameters

void procCmdLine (int argc, char** argv, ParamArgData& argData);

#endif /* FPE_PROCCMDLINE_HPP */

