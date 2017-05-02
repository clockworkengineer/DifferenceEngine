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

    static void checkArgumentPresent(const vector<string>& arguments, po::variables_map& configVarMap) {

        for (auto arg : arguments) {
            if (!configVarMap.count(arg) || configVarMap[arg].as<string>().empty()) {
                throw po::error("Required argument '" + arg + "' missing.");
            }
        }

    }

    //
    // Display string argument
    //

    static void displayArgument(const string& label, const string argumentStr) {
        if (!argumentStr.empty()) {
            CLogger::coutstr({"*** ", label, " = [", argumentStr, "] ***"});
        }

    }

     //
    // Process program argument data and display run options. Note: Any
    // arguments not needed for task are set to empty.
    //

    static void processArgumentData(ParamArgData& argumentData) {

        // Do not require destination for tasks

        if ((argumentData.taskFunc.name == kEmailFileStr) ||
                (argumentData.taskFunc.name == kZipFileStr) ||
                (argumentData.taskFunc.name == kRunCommandStr)) {
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
            argumentData.mailBoxNameStr = "";
        }
   
        // Make watch/destination paths absolute

        argumentData.watchFolderStr = fs::absolute(argumentData.watchFolderStr).string();
        argumentData.destinationFolderStr = fs::absolute(argumentData.destinationFolderStr).string();
 
        // Display arguments

        displayArgument("CONFIG FILE", argumentData.configFileNameStr);
        displayArgument("WATCH FOLDER", argumentData.watchFolderStr);
        displayArgument("DESTINATION FOLDER", argumentData.destinationFolderStr);
        displayArgument("SHELL COMMAND", argumentData.commandToRunStr);
        displayArgument("SERVER URL", argumentData.serverURLStr);
        displayArgument("MAILBOX", argumentData.mailBoxNameStr);
        displayArgument("ZIP ARCHIVE", argumentData.zipArchiveStr);
        displayArgument("TASK", argumentData.taskFunc.name);
        displayArgument("LOG FILE", argumentData.logFileNameStr);

        if (argumentData.bQuiet) {
            CLogger::coutstr({"*** QUIET MODE ***"});
        }

        if (argumentData.bDeleteSource) {
            CLogger::coutstr({"*** DELETE SOURCE FILE ON SUCESSFUL PROCESSING ***"});
        }

        if (argumentData.bSingleThread) {
            CLogger::coutstr({"*** SINGLE THREAD ***"});
        }

        if (argumentData.killCount) {
            CLogger::coutstr({"*** KILL COUNT = ", to_string(argumentData.killCount), " ***"});
        }
    
        // Create watch folder for task if necessary 

        if (!fs::exists(argumentData.watchFolderStr)) {
            fs::create_directory(argumentData.watchFolderStr);
        }

        // Create destination folder for task if necessary 

        if (!argumentData.destinationFolderStr.empty() && !fs::exists(argumentData.destinationFolderStr)) {
            fs::create_directory(argumentData.destinationFolderStr);
        }

    }

    // ================
    // PUBLIC FUNCTIONS
    // ================

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

        po::variables_map configVarMap;

        try {

            // Process command line arguments

            po::store(po::parse_command_line(argc, argv, commandLine), configVarMap);

            // Display options and exit with success

            if (configVarMap.count("help")) {
                cout << "File Processing Engine Application" << endl << commandLine << endl;
                exit(EXIT_SUCCESS);
            }

            // Load config file specified

            if (configVarMap.count("config")) {
                if (fs::exists(configVarMap["config"].as<string>().c_str())) {
                    ifstream configFileStream{configVarMap["config"].as<string>().c_str()};
                    if (configFileStream) {
                        po::store(po::parse_config_file(configFileStream, configFile), configVarMap);
                    } else {
                        throw po::error("Error opening config file.");
                    }
                } else {
                    throw po::error("Specified config file does not exist.");
                }
            }

            // Task argument validation. Parameters  valid to the task being
            // run are checked for and if not present an exception is thrown to
            // produce a relevant error message.Any extra arguments not required 
            // for a task are just ignored.

            if (configVarMap.count("task")) {
                argumentData.taskFunc = getTaskDetails(configVarMap["task"].as<int>());
                if (argumentData.taskFunc.name == "") {
                    throw po::error("Invalid Task Number.");
                } else if (argumentData.taskFunc.name == kVideoConversionStr) {
                    argumentData.commandToRunStr = kHandbrakeCommandStr;
                } else if (argumentData.taskFunc.name == kRunCommandStr) {
                    checkArgumentPresent({"command"}, configVarMap);
                } else if (argumentData.taskFunc.name == kZipFileStr) {
                    checkArgumentPresent({"archive"}, configVarMap);
                } else if (argumentData.taskFunc.name == kEmailFileStr) {
                    checkArgumentPresent({"server", "user", "password", "recipient", "mailbox"}, configVarMap);
                }
            }

            // Delete source file

            if (configVarMap.count("delete")) {
                argumentData.bDeleteSource = true;
            }

            // No trace output

            if (configVarMap.count("quiet")) {
                argumentData.bQuiet = true;
            }

            // Use main thread for task.

            if (configVarMap.count("single")) {
                argumentData.bSingleThread = true;
            }

            po::notify(configVarMap);

        } catch (po::error& e) {
            cerr << "FPE Error: " << e.what() << "\n" << endl;
            cerr << commandLine << endl;
            exit(EXIT_FAILURE);
        }

        // Process program argument data

        processArgumentData(argumentData);

        return (argumentData);

    }

} // FPE_ProcCmdLine
