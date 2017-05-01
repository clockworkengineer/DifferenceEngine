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
// Description: Command line argument processing functionality.
// 
// Dependencies:
// 
// C11++              : Use of C11++ features.
// Antikythera Classes: CLogger
// Linux              : Target platform
// Boost              : File system, program options.
//

// =============
// INCLUDE FILES
// =============

//
// C++ STL definitions
//

#include <iostream>

//
// Program components.
//

#include "FPE_ProcCmdLine.hpp"

//
// Antikythera Classes
//

#include "CLogger.hpp"

//
// Boost  file system library & program options processing
//

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

    using namespace FPE_ActionFuncs;

    using namespace Antik::Util;

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
                ("watch,w", po::value<string>(&argumentData.watchFolderStr)->required(), "Watch folder")
                ("destination,d", po::value<string>(&argumentData.destinationFolderStr)->required(), "Destination folder")
                ("task,t", po::value<int>(&argumentData.taskFunc.number)->required(), "Task number")
                ("command", po::value<string>(&argumentData.commandToRunStr), "Shell command to run")
                ("maxdepth", po::value<int>(&argumentData.maxWatchDepth), "Maximum watch depth")
                ("extension,e", po::value<string>(&argumentData.extensionStr), "Override destination file extension")
                ("quiet,q", "Quiet mode (no trace output)")
                ("delete", "Delete source file")
                ("log,l", po::value<string>(&argumentData.logFileNameStr), "Log file")
                ("single,s", "Run task in main thread")
                ("killcount,k", po::value<int>(&argumentData.killCount), "Files to process before closedown")
                ("server,s", po::value<string>(&argumentData.serverURLStr), "SMTP server URL and port")
                ("user,u", po::value<string>(&argumentData.userNameStr), "Account username")
                ("password,p", po::value<string>(&argumentData.userPasswordStr), "Account username password")
                ("recipient,r", po::value<string>(&argumentData.emailRecipientStr), "Recipients(s) for email with attached file")
                ("mailbox,m", po::value<string>(&argumentData.mailBoxNameStr), "IMAP Mailbox name for drop box")
                ("archive,a", po::value<string>(&argumentData.zipArchiveStr), "ZIP destination archive");


    }

    //
    // If a argument is not present throw an exception.
    //
    
    static void checkArgumentPresent(const string& param, po::variables_map& vm) {
        
        if (!vm.count(param) || vm[param].as<string>().empty()) {
            throw po::error("Required argument '"+param+"' missing.");
        }

    }
    
    // ================
    // PUBLIC FUNCTIONS
    // ================

    //
    // Process program argument data and display run options
    //

    void processArgumentData(ParamArgData& argumentData) {


        //
        // Set any unneeded arguments for task to none
        //
        
        // Do not require destination for tasks

        if ((argumentData.taskFunc.name == kEmailFileStr) ||
            (argumentData.taskFunc.name == kZipFileStr) ||
            (argumentData.taskFunc.name != kRunCommandStr)) {
            argumentData.destinationFolderStr = "";
        }

        // Only have ZIP archive if ZIP archive task

        if (argumentData.taskFunc.name != kZipFileStr) {
            argumentData.zipArchiveStr = "";
        }
        
        // Only have shell command if run command task

        if (argumentData.taskFunc.name != kRunCommandStr) {
            argumentData.commandToRunStr = "";
        }

        // Only mail server details if email file task

        if (argumentData.taskFunc.name != kEmailFileStr) {
            argumentData.serverURLStr = "";
            argumentData.userNameStr = "";
            argumentData.userPasswordStr = "";
            argumentData.mailBoxNameStr= "";
        }

        // Display config file used

        if (!argumentData.configFileNameStr.empty()) {
            CLogger::coutstr({"*** CONFIG FILE = [", argumentData.configFileNameStr, "] ***"});
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

        // Display any destination folder.

        if (!argumentData.destinationFolderStr.empty()) {
            CLogger::coutstr({"*** DESTINATION FOLDER = [", argumentData.destinationFolderStr, "] ***"});
        }

        // Display shell command

        if (!argumentData.commandToRunStr.empty()) {
            CLogger::coutstr({"*** SHELL COMMAND = [", argumentData.commandToRunStr, "] ***"});
        }

        // Display server url

        if (!argumentData.serverURLStr.empty()) {
            CLogger::coutstr({"*** SERVER URL = [", argumentData.serverURLStr, "] ***"});
        }

        // Display mailbox name

        if (!argumentData.mailBoxNameStr.empty()) {
            CLogger::coutstr({"*** MEILBOX = [", argumentData.mailBoxNameStr, "] ***"});
        }

        // Display any archive

        if (!argumentData.zipArchiveStr.empty()) {
            CLogger::coutstr({"*** ZIP ARCHIVE = [", argumentData.zipArchiveStr, "] ***"});
        }


        // Display Task

        if (!argumentData.taskFunc.name.empty()) {
            CLogger::coutstr({"*** TASK = [", argumentData.taskFunc.name, "] ***"});
        }

        // Display quiet mode

        if (argumentData.bQuiet) {
            CLogger::coutstr({"*** QUIET MODE ***"});
        }

        // Display source will be deleted on success

        if (argumentData.bDeleteSource) {
            CLogger::coutstr({"*** DELETE SOURCE FILE ON SUCESSFUL PROCESSING ***"});
        }

        // Display using single thread

        if (argumentData.bSingleThread) {
            CLogger::coutstr({"*** SINGLE THREAD ***"});
        }

        // Display using killCount

        if (argumentData.killCount) {
            CLogger::coutstr({"*** KILL COUNT = ", to_string(argumentData.killCount), " ***"});
        }

        // Display log file

        if (!argumentData.logFileNameStr.empty()) {
            CLogger::coutstr({"*** LOG FILE = [", argumentData.logFileNameStr, "] ***"});
        }

    }

    //
    // Read in and process command line arguments using boost. Note this is the only 
    // component that uses cout and std:cerr directly and not the thread safe 
    // coutstr/cerrstr but that is not necessary as still in single thread mode when
    // reading and processing arguments.
    //

    ParamArgData fetchCommandLineArgumentData(int argc, char** argv) {

        ParamArgData argumentData{};

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

            // Task argument validation. Parameters  valid to the task being
            // run are checked for and if not present an exception is thrown to
            // produce a relevant error message.Any extra arguments not required 
            // for a task are just ignored.

            if (vm.count("task")) {
                argumentData.taskFunc = getTaskDetails(vm["task"].as<int>());
                if (argumentData.taskFunc.name == "") {
                    throw po::error("Invalid Task Number.");
                } else if (argumentData.taskFunc.name == kVideoConversionStr) {
                    argumentData.commandToRunStr = kHandbrakeCommandStr;
                } else if (argumentData.taskFunc.name == kRunCommandStr) {
                    checkArgumentPresent("command", vm);
                } else if (argumentData.taskFunc.name == kZipFileStr) {
                    checkArgumentPresent("archive", vm);
                } else if (argumentData.taskFunc.name == kEmailFileStr) {
                    checkArgumentPresent("server", vm);
                    checkArgumentPresent("user", vm);
                    checkArgumentPresent("password", vm);
                    checkArgumentPresent("recipient", vm);
                    checkArgumentPresent("mailbox", vm);
                }
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

            po::notify(vm);

            // Make watch/destination paths absolute

            argumentData.watchFolderStr = fs::absolute(argumentData.watchFolderStr).string();
            argumentData.destinationFolderStr = fs::absolute(argumentData.destinationFolderStr).string();

        } catch (po::error& e) {
            cerr << "FPE Error: " << e.what() << endl << endl;
            cerr << commandLine << endl;
            exit(EXIT_FAILURE);
        }

        return (argumentData);

    }

} // FPE_ProcCmdLine
