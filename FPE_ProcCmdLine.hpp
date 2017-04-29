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

// =========
// NAMESPACE
// =========

namespace FPE_ProcCmdLine {
    
    //
    // Command line parameter data
    //
    
    struct ParamArgData {
        std::string watchFolderStr;       // Watch Folder
        std::string destinationFolderStr; // Destination Folder for copies.
        std::string commandToRunStr;      // Command to run
        bool bFileCopy { false };         // Task file copy
        bool bVideoConversion { false };  // Task video conversion
        bool bRunCommand { false };       // Task perform command
        bool bZipArchive { false };       // Task add to ZIP archive
        int maxWatchDepth { -1 };         // Watch depth -1=all,0=just watch folder,1=next level down etc.
        bool bDeleteSource { false };     // Delete source file
        std::string extensionStr;         // Override destination file extension
        bool bQuiet { false };            // Quiet mode no tracing output.
        int killCount { 0 };              // Kill Count
        bool bSingleThread { false };     // Run task in the main thread
        std::string logFileNameStr;       // Log file name
        std::string configFileNameStr;    // Config file name
        bool bEmailFile { false };        // Email or save file to Inbox.
        std::string userNameStr;          // Email account user name
        std::string userPasswordStr;      // Email account user name password
        std::string serverURLStr;         // Email SMTP/IMAP server URL
        std::string emailRecipientStr;    // Email recipient
        std::string mailBoxNameStr;       // IMAP Mailbox name
        std::string zipArchiveStr;        // Destination ZIP Archive
    };

    //
    // Handbrake command
    //
    
    constexpr const char *kHandbrakeCommandStr = "/usr/local/bin/HandBrakeCLI -i %1% -o %2% --preset=\"Normal\"";

    //
    // Process command argument data
    //
    
    void processArgumentData(ParamArgData& argumentData);
     
    // Get command line parameters

    ParamArgData fetchCommandLineArgumentData(int argc, char** argv);
    

} // namespace FPE_ProcCmdLine
#endif /* FPE_PROCCMDLINE_HPP */

