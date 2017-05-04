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
// Description: Command line option processing functionality.
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

    static void addCommonOptions(po::options_description& commonOptions, FPEOptions& optionData) {

        commonOptions.add_options()
                ("watch,w", po::value<string>(&optionData.optionsMap["watch"])->required(), "Watch folder")
                ("destination,d", po::value<string>(&optionData.optionsMap["destination"])->required(), "Destination folder")
                ("task,t", po::value<int>(&optionData.taskFunc.number)->required(), "Task number")
                ("command", po::value<string>(&optionData.optionsMap["command"]), "Shell command to run")
                ("maxdepth", po::value<string>(&optionData.optionsMap["maxdepth"])->default_value("-1"), "Maximum watch depth")
                ("extension,e", po::value<string>(&optionData.optionsMap["extension"]), "Override destination file extension")
                ("quiet,q", "Quiet mode (no trace output)")
                ("delete", "Delete source file")
                ("log,l", po::value<string>(&optionData.optionsMap["log"]), "Log file")
                ("single,s", "Run task in main thread")
                ("killcount,k", po::value<string>(&optionData.optionsMap["killcount"])->default_value("0"), "Files to process before closedown")
                ("server,s", po::value<string>(&optionData.optionsMap["server"]), "SMTP server URL and port")
                ("user,u", po::value<string>(&optionData.optionsMap["user"]), "Account username")
                ("password,p", po::value<string>(&optionData.optionsMap["password"]), "Account username password")
                ("recipient,r", po::value<string>(&optionData.optionsMap["recipient"]), "Recipients(s) for email with attached file")
                ("mailbox,m", po::value<string>(&optionData.optionsMap["mailbox"]), "IMAP Mailbox name for drop box")
                ("archive,a", po::value<string>(&optionData.optionsMap["archive"]), "ZIP destination archive");

    }
    
    //
    // If a option is not present throw an exception.
    //

    static void checkOptionPresent(const vector<string>& options, po::variables_map& configVarMap) {

        for (auto opt : options) {
            if (!configVarMap.count(opt) || configVarMap[opt].as<string>().empty()) {
                throw po::error("Task option '" + opt + "' missing.");
            }
        }

    }
    
    //
    // Display option name and its string value
    //

    static void displayOption(const string& nameStr, const string& valueStr) {
        if (!valueStr.empty()) {
            CLogger::coutstr({"*** ", nameStr, " = [", valueStr, "] ***"});
        }
    }
    
    //
    // Display description string if boolean option true
    //
    
    static void displayOption(bool bOption, const string& descStr) {
        if (bOption) {
            CLogger::coutstr({"*** ", descStr, " ***"});
        }
    }
    
    //
    // Process program option data and display run options. Note: Any
    // options not needed for task are set to empty.
    //

    static void processOptionData(FPEOptions& optionData) {

        // Do not require destination for tasks

        if ((optionData.taskFunc.name == kEmailFileStr) ||
                (optionData.taskFunc.name == kZipFileStr) ||
                (optionData.taskFunc.name == kRunCommandStr)) {
            optionData.optionsMap.erase("destination");
        }

        // Only have ZIP archive if ZIP archive task

        if (optionData.taskFunc.name != kZipFileStr) {
            optionData.optionsMap.erase("archive");
        }

        // Only have shell command if run command task

        if (optionData.taskFunc.name != kRunCommandStr) {
            optionData.optionsMap.erase("command");
        }

        // Only mail server details if email file task

        if (optionData.taskFunc.name != kEmailFileStr) {
            optionData.optionsMap.erase("server");
            optionData.optionsMap.erase("user");
            optionData.optionsMap.erase("password");
            optionData.optionsMap.erase("mailbox");
        }
        
        // Make watch/destination paths absolute

        optionData.optionsMap["watch"] = fs::absolute(optionData.optionsMap["watch"]).string();
        optionData.optionsMap["destination"] = fs::absolute(optionData.optionsMap["destination"]).string();
 
        // Display options

        displayOption(static_cast<string>("CONFIG FILE"), optionData.optionsMap["config"]);
        displayOption(static_cast<string>("WATCH FOLDER"), optionData.optionsMap["watch"]);
        displayOption(static_cast<string>("DESTINATION FOLDER"), optionData.optionsMap["destination"]);
        displayOption(static_cast<string>("SHELL COMMAND"), optionData.optionsMap["command"]);
        displayOption(static_cast<string>("SERVER URL"), optionData.optionsMap["server"]);
        displayOption(static_cast<string>("MAILBOX"), optionData.optionsMap["mailbox"]);
        displayOption(static_cast<string>("ZIP ARCHIVE"), optionData.optionsMap["archive"]);
        displayOption(static_cast<string>("TASK"), optionData.taskFunc.name);
        displayOption(static_cast<string>("LOG FILE"), optionData.optionsMap["log"]);
        displayOption((getOption<int>(optionData, "killcount") > 0),"KILL COUNT = ["+optionData.optionsMap["killcount"]+"]");
        displayOption(getOption<bool>(optionData,"quiet"), "QUIET MODE");
        displayOption(getOption<bool>(optionData,"delete"), "DELETE SOURCE FILE");
        displayOption(getOption<bool>(optionData,"single"),"SINGLE THREAD");
     
        // Create watch folder for task if necessary 

        if (!fs::exists(optionData.optionsMap["watch"])) {
            fs::create_directory(optionData.optionsMap["watch"]);
        }

        // Create destination folder for task if necessary 

        if (!optionData.optionsMap["destination"].empty() && !fs::exists(optionData.optionsMap["destination"])) {
            fs::create_directory(optionData.optionsMap["destination"]);
        }

    }

    // ================
    // PUBLIC FUNCTIONS
    // ================
    
    //
    // Read in and process command line options using boost. Note this is the only 
    // component that uses cout and std:cerr directly and not the thread safe 
    // coutstr/cerrstr but that is not necessary as still in single thread mode when
    // reading and processing options.
    //

    FPEOptions fetchCommandLineOptionData(int argc, char** argv) {

        FPEOptions optionData{};

        // Define and parse the program options

        po::options_description commandLine("Command Line Options");

        // Command line (first unique then add those shared with config file

        commandLine.add_options()
                ("help", "Display help message")
                ("config", po::value<string>(&optionData.optionsMap["config"]), "Configuration file name");

        addCommonOptions(commandLine, optionData);

        // Config file options

        po::options_description configFile("Configuration File Options");

        addCommonOptions(configFile, optionData);

        po::variables_map configVarMap;

        try {

            // Process command line options

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

            // Task option validation. Parameters  valid to the task being
            // run are checked for and if not present an exception is thrown to
            // produce a relevant error message.Any extra options not required 
            // for a task are just ignored.

            if (configVarMap.count("task")) {
                optionData.taskFunc = getTaskDetails(configVarMap["task"].as<int>());
                if (optionData.taskFunc.name == "") {
                    throw po::error("Invalid Task Number.");
                } else if (optionData.taskFunc.name == kVideoConversionStr) {
                    optionData.optionsMap["command"] = kHandbrakeCommandStr;
                } else if (optionData.taskFunc.name == kRunCommandStr) {
                    checkOptionPresent({"command"}, configVarMap);
                } else if (optionData.taskFunc.name == kZipFileStr) {
                    checkOptionPresent({"archive"}, configVarMap);
                } else if (optionData.taskFunc.name == kEmailFileStr) {
                    checkOptionPresent({"server", "user", "password", "recipient", "mailbox"}, configVarMap);
                }
            }

            // Check killcount is a valid int

            if (configVarMap.count("killcount")) {
                try {
                    stoi(configVarMap["killcount"].as<string>());
                } catch (const std::exception& e) {
                    throw po::error("killcount is not a valid integer.");
                }
            }
   
           // Check maxdepth is a valid int

            if (configVarMap.count("maxdepth")) {
                try {
                    stoi(configVarMap["maxdepth"].as<string>());
                } catch (const std::exception& e) {
                    throw po::error("maxdepth is not a valid integer.");
                }
            }
  
            // Delete source file

            if (configVarMap.count("delete")) {
                optionData.optionsMap["delete"] = "1";
            }

            // No trace output

            if (configVarMap.count("quiet")) {
                optionData.optionsMap["quiet"] = "1";
            }

            // Use main thread for task.

            if (configVarMap.count("single")) {
                optionData.optionsMap["single"] = "1";
            }

            po::notify(configVarMap);

        } catch (po::error& e) {
            cerr << "FPE Error: " << e.what() << endl;
            exit(EXIT_FAILURE);
        }

        // Process program option data

        processOptionData(optionData);

        return (optionData);

    }

} // FPE_ProcCmdLine
