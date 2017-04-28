#include "HOST.hpp"
/*
 * File:   FPE_ProcCmdLine.cpp
 * 
 * Author: Robert Tizzard
 *
 * Created on October 24, 2016, 2:34 PM
 * 
 * Copyright 2016.
 * 
 */

//
// Module: FPE_ProcCmdLine
//
// Description: Parse command line parameters and fill in structure ParamArgData.
// 
// Dependencies: C11++, Boost C++ Libraries.
//

// =============
// INCLUDE FILES
// =============

//
// C++ STL definitions
//

#include <iostream>

// Process command line arguments

#include "FPE_ProcCmdLine.hpp"

//
// Antikythera Classes
//

#include "CLogger.hpp"

// Boost  file system library & program options processing

#include "boost/program_options.hpp" 
#include <boost/filesystem.hpp>


// =========
// NAMESPACE
// =========

namespace FPE_ProcCmdLine {

    // =======
    // IMPORTS
    // =======

    using namespace std;

    namespace po = boost::program_options;
    namespace fs = boost::filesystem;

    // ===============
    // LOCAL FUNCTIONS
    // ===============

    //
    // Add options common to both command line and config file
    //

    static void addCommonOptions(po::options_description& commonOptions, ParamArgData& argumentData) {

        commonOptions.add_options()
                ("email", "Task = Email File Attachment")
                ("copy", "Task = File Copy Watcher")
                ("video", "Task = Video Conversion Watcher")
                ("zip", "Task = File ZIP Archive Watcher")
                ("command", po::value<string>(&argumentData.commandToRunStr), "Task = Run Shell Command")
                ("watch,w", po::value<string>(&argumentData.watchFolderStr)->required(), "Watch Folder")
                ("destination,d", po::value<string>(&argumentData.destinationFolderStr)->required(), "Destination Folder")
                ("maxdepth", po::value<int>(&argumentData.maxWatchDepth), "Maximum Watch Depth")
                ("extension,e", po::value<string>(&argumentData.extensionStr), "Override destination file extension")
                ("quiet,q", "Quiet mode (no trace output)")
                ("delete", "Delete Source File")
                ("log,l", po::value<string>(&argumentData.logFileNameStr), "Log file")
                ("single,s", "Run task in main thread")
                ("killcount,k", po::value<int>(&argumentData.killCount), "Files to process before closedown")
                ("server,s", po::value<string>(&argumentData.serverURLStr), "SMTP Server URL and port")
                ("user,u", po::value<string>(&argumentData.userNameStr), "Account username")
                ("password,p", po::value<string>(&argumentData.userPasswordStr), "Account username password")
                ("recipient,r", po::value<string>(&argumentData.emailRecipientStr), "Recipients(s) for email with attached file")
                ("mailbox,m", po::value<string>(&argumentData.mailBoxNameStr), "IMAP Mailbox name for drop box")
                ("archive,a", po::value<string>(&argumentData.zipArchiveStr), "ZIP destination archive");


    }

    // ================
    // PUBLIC FUNCTIONS
    // ================

        //
    // Preprocess program argument data and display run options
    //

    void processArgumentData(ParamArgData& argumentData, CRedirect& logFile) {


        // Email/archive does not require a destination folder

        if (argumentData.bEmailFile || argumentData.bZipArchive) {
            argumentData.destinationFolderStr = "";
        }

        // Only have ZIP archive if file add to ZIP archive task

        if (!argumentData.bZipArchive) {
            argumentData.zipArchiveStr = "";
        }

        // Create watch folder for task.

        if (!fs::exists(argumentData.watchFolderStr)) {
            CLogger::coutstr({"Watch folder [", argumentData.watchFolderStr, "] DOES NOT EXIST."});
            if (fs::create_directory(argumentData.watchFolderStr)) {
                CLogger::coutstr({"Creating watch folder [", argumentData.watchFolderStr, "]"});
            }
        }

        CLogger::coutstr({"*** WATCH FOLDER = [", argumentData.watchFolderStr, "] ***"});

        // Create destination folder for task

        if (!argumentData.destinationFolderStr.empty() && !fs::exists(argumentData.destinationFolderStr)) {
            CLogger::coutstr({"Destination folder ", argumentData.destinationFolderStr, " does not exist."});
            if (fs::create_directory(argumentData.destinationFolderStr)) {
                CLogger::coutstr({"Creating destination folder ", argumentData.destinationFolderStr});
            }
        }

        // Signal any destination folder.

        if (!argumentData.destinationFolderStr.empty()) {
            CLogger::coutstr({"*** DESTINATION FOLDER = [", argumentData.destinationFolderStr, "] ***"});
        }

        // Signal any archive

        if (!argumentData.zipArchiveStr.empty()) {
            CLogger::coutstr({"*** ZIP ARCHIVE = [", argumentData.zipArchiveStr, "] ***"});
        }

        // Signal config file used

        if (!argumentData.configFileNameStr.empty()) {
            CLogger::coutstr({"*** CONFIG FILE = [", argumentData.configFileNameStr, "] ***"});
        }

        // Signal email file task

        if (argumentData.bEmailFile) {
            CLogger::coutstr({"*** EMAIL FILE TASK ***"});
        }

        // Signal file copy task

        if (argumentData.bFileCopy) {
            CLogger::coutstr({"*** FILE COPY TASK ***"});
        }

        // Signal file ZIP archive task

        if (argumentData.bZipArchive) {
            CLogger::coutstr({"*** FILE ARCHIVE TASK ***"});
        }

        // Signal video conversion task

        if (argumentData.bVideoConversion) {
            CLogger::coutstr({"*** VIDEO CONVERSION TASK ***"});
        }

        // Signal run command task

        if (argumentData.bRunCommand) {
            CLogger::coutstr({"*** RUN COMMAND TASK ***"});
        }

        // Signal quiet mode

        if (argumentData.bQuiet) {
            CLogger::coutstr({"*** QUIET MODE ***"});
        }

        // Signal source will be deleted on success

        if (argumentData.bDeleteSource) {
            CLogger::coutstr({"*** DELETE SOURCE FILE ON SUCESSFUL PROCESSING ***"});
        }

        // Signal using single thread

        if (argumentData.bSingleThread) {
            CLogger::coutstr({"*** SINGLE THREAD ***"});
        }

        // Signal using killCount

        if (argumentData.killCount) {
            CLogger::coutstr({"*** KILL COUNT = ", to_string(argumentData.killCount), " ***"});
        }

        // Output to log file ( CRedirect(cout) is the simplest solution). Once the try is exited
        // CRedirect object will be destroyed and cout restored.

        if (!argumentData.logFileNameStr.empty()) {
            CLogger::coutstr({"*** LOG FILE = [", argumentData.logFileNameStr, "] ***"});
            logFile.change(argumentData.logFileNameStr, ios_base::out | ios_base::app);
            CLogger::coutstr({string(100, '=')});
        }

    }

    //
    // Read in and process command line arguments using boost. Note this is the only 
    // component that uses cout and std:cerr directly and not the thread safe 
    // coutstr/cerrstr but that is not necessary as still in single thread mode when
    // reading and processing parameters.
    //

    ParamArgData fetchCommandLineArgumentData(int argc, char** argv) {

        ParamArgData argumentData { };
        
        // Define and parse the program options

        po::options_description commandLine("Command Line Options");

        // Command line (first unique then add those shared with config file

        commandLine.add_options()
                ("help", "Display help message")
                ("config", po::value<string>(&argumentData.configFileNameStr), "Configuration file name");

        addCommonOptions(commandLine, argumentData);

        // Config file options

        po::options_description configFile("Configuration File Options");

        addCommonOptions(configFile, argumentData);

        po::variables_map vm;

        try {

            int taskCount = 0;

            // Process command line arguments

            po::store(po::parse_command_line(argc, argv, commandLine), vm);

            // Display options and exit with success

            if (vm.count("help")) {
                cout << "File Processing Engine Application" << endl << commandLine << endl;
                exit(EXIT_SUCCESS);
            }

            // Load config file specified

            if (vm.count("config")) {
                if (fs::exists(vm["config"].as<string>().c_str())) {
                    ifstream ifs{vm["config"].as<string>().c_str()};
                    if (ifs) {
                        po::store(po::parse_config_file(ifs, configFile), vm);
                    }
                } else {
                    throw po::error("Specified config file does not exist.");
                }
            }

            // Email watched files.

            if (vm.count("email")) {
                argumentData.bEmailFile = true;
                taskCount++;
            }

            // Copy watched files.

            if (vm.count("copy")) {
                argumentData.bFileCopy = true;
                taskCount++;
            }

            // Convert watched video files

            if (vm.count("video")) {
                argumentData.bVideoConversion = true;
                argumentData.commandToRunStr = kHandbrakeCommandStr;
                taskCount++;
            }

            // Add file to ZIP Archive

            if (vm.count("zip")) {
                argumentData.bZipArchive = true;
                taskCount++;
            }

            // Run command on watched files

            if (vm.count("command")) {
                argumentData.bRunCommand = true;
                taskCount++;
            }

            // Delete source file

            if (vm.count("delete")) {
                argumentData.bDeleteSource = true;
            }

            // No trace output

            if (vm.count("quiet")) {
                argumentData.bQuiet = true;
            }

            // Use main thread for task.

            if (vm.count("single")) {
                argumentData.bSingleThread = true;
            }

            // Default task file copy. More than one task throw error.

            if (taskCount == 0) {
                argumentData.bFileCopy = true;
            } else if (taskCount > 1) {
                throw po::error("More than one task specified");
            }

            po::notify(vm);

            // Make watch/destination paths absolute

            argumentData.watchFolderStr = fs::absolute(argumentData.watchFolderStr).string();
            argumentData.destinationFolderStr = fs::absolute(argumentData.destinationFolderStr).string();

        } catch (po::error& e) {
            cerr << "FPE Error: " << e.what() << endl << endl;
            cerr << commandLine << endl;
            exit(EXIT_FAILURE);
        }
        
        return(argumentData);

    }

} // FPE_ProcCmdLine
