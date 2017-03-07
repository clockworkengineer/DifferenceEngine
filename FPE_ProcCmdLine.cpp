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
// Module: Command Line Parameter Processing
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

// Boost  file system library & program options processing

#include "boost/program_options.hpp" 
#include <boost/filesystem.hpp>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

// ===============
// LOCAL FUNCTIONS
// ===============

// ================
// PUBLIC FUNCTIONS
// ================

//
// Add options common to both command line and config file
//

void addCommonOptions(po::options_description& commonOptions, ParamArgData& argData) {
    
    commonOptions.add_options()
            ("email", "Task = Email File Attachment")
            ("copy", "Task = File Copy Watcher")
            ("video", "Task = Video Conversion Watcher")
            ("command", po::value<std::string>(&argData.commandToRunStr), "Task = Run Shell Command")
            ("watch,w", po::value<std::string>(&argData.watchFolderStr)->required(), "Watch Folder")
            ("destination,d", po::value<std::string>(&argData.destinationFolderStr)->required(), "Destination Folder")
            ("maxdepth", po::value<int>(&argData.maxWatchDepth), "Maximum Watch Depth")
            ("extension,e", po::value<std::string>(&argData.extensionStr), "Override destination file extension")
            ("quiet,q", "Quiet mode (no trace output)")
            ("delete", "Delete Source File")
            ("log,l", po::value<std::string>(&argData.logFileNameStr), "Log file")
            ("single,s", "Run task in main thread")
            ("killcount,k", po::value<int>(&argData.killCount), "Files to process before closedown")
            ("server,s", po::value<std::string>(&argData.serverURLStr), "SMTP Server URL and port")
            ("user,u", po::value<std::string>(&argData.userNameStr), "Account username")
            ("password,p", po::value<std::string>(&argData.userPasswordStr), "Account username password")
            ("recipient,r", po::value<std::string>(&argData.emailRecipientStr), "Recipients(s) for email with attached file")
            ("mailbox,m", po::value<std::string>(&argData.mailBoxNameStr), "IMAP Mailbox name for drop box");


}

//
// Read in and process command line arguments using boost. Note this is the only 
// component that uses std::cout and std:cerr directly and not the thread safe 
// coutstr/cerrstr but that is not necessary as still in single thread mode when
// reading and processing parameters.
//

void procCmdLine(int argc, char** argv, ParamArgData& argData) {

    // Default values

    argData.bDeleteSource = false;
    argData.bFileCopy = false;
    argData.bVideoConversion = false;
    argData.bRunCommand = false;
    argData.maxWatchDepth = -1;
    argData.bDeleteSource = false;
    argData.extensionStr = "";
    argData.bQuiet = false;
    argData.killCount = 0;
    argData.bSingleThread = false;
    argData.logFileNameStr = "";
    argData.configFileNameStr = "";
    argData.bEmailFile = false;
    argData.userNameStr = "";
    argData.userPasswordStr = "";
    argData.serverURLStr = "";
    argData.emailRecipientStr = "";

    // Define and parse the program options

    po::options_description commandLine("Command Line Options");

    // Command line (first unique then add those shared with config file

    commandLine.add_options()
            ("help", "Display help message")
            ("config", po::value<std::string>(&argData.configFileNameStr), "Configuration file name");

    addCommonOptions(commandLine, argData);

    // Config file options

    po::options_description configFile("Configuration File Options");

    addCommonOptions(configFile, argData);

    po::variables_map vm;

    try {

        int taskCount = 0;

        // Process command line arguments

        po::store(po::parse_command_line(argc, argv, commandLine), vm);

        // Display options and exit with success

        if (vm.count("help")) {
            std::cout << "File Processing Engine Application" << std::endl << commandLine << std::endl;
            exit(EXIT_SUCCESS);
        }

        // Load config file specified

        if (vm.count("config")) {
            if (fs::exists(vm["config"].as<std::string>().c_str())) {
                std::ifstream ifs{vm["config"].as<std::string>().c_str()};
                if (ifs) {
                    po::store(po::parse_config_file(ifs, configFile), vm);
                }
            } else {
                throw po::error("Specified config file does not exist.");
            }
        }

        // Email watched files.

        if (vm.count("email")) {
            argData.bEmailFile = true;
            taskCount++;
        }

        // Copy watched files.

        if (vm.count("copy")) {
            argData.bFileCopy = true;
            taskCount++;
        }

        // Convert watched video files

        if (vm.count("video")) {
            argData.bVideoConversion = true;
            argData.commandToRunStr = kHandbrakeCommandStr;
            taskCount++;
        }

        // Run command on watched files

        if (vm.count("command")) {
            argData.bRunCommand = true;
            taskCount++;
        }

        // Delete source file

        if (vm.count("delete")) {
            argData.bDeleteSource = true;
        }

        // No trace output

        if (vm.count("quiet")) {
            argData.bQuiet = true;
        }

        // Use main thread for task.

        if (vm.count("single")) {
            argData.bSingleThread = true;
        }

        // Default task file copy. More than one task throw error.

        if (taskCount == 0) {
            argData.bFileCopy = true;
        } else if (taskCount > 1) {
            throw po::error("More than one task specified");
        }

        po::notify(vm);

        // Make watch/destination paths absolute

        argData.watchFolderStr = fs::absolute(argData.watchFolderStr).string();
        argData.destinationFolderStr = fs::absolute(argData.destinationFolderStr).string();

    } catch (po::error& e) {
        std::cerr << "FPE Error: " << e.what() << std::endl << std::endl;
        std::cerr << commandLine << std::endl;
        exit(EXIT_FAILURE);
    }

}

