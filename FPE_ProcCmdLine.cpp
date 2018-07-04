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
// C11++        : Use of C11++ features.
// Antik Classes: CFile, CPath.
// Linux        : Target platform
// Boost        : Program options.
//

// =============
// INCLUDE FILES
// =============

//
// C++ STL
//

#include <iostream>

//
// Program components.
//

#include "FPE.hpp"
#include "FPE_ProcCmdLine.hpp"

//
// Antik Classes
//

#include "CFile.hpp"
#include "CPath.hpp"

//
// Boost  program options processing
//

#include "boost/program_options.hpp" 

// =========
// NAMESPACE
// =========

namespace FPE_ProcCmdLine {

    // =======
    // IMPORTS
    // =======

    using namespace std;

    using namespace FPE;
    using namespace FPE_TaskActions;
    
    using namespace Antik::File;

    namespace po = boost::program_options;

    // ===============
    // LOCAL FUNCTIONS
    // ===============

    //
    // Add options common to both command line and config file
    //

    static void addCommonOptions(po::options_description& commonOptions, FPEOptions& options) {

        commonOptions.add_options()
                ("watch,w", po::value<string>(&options.map[kWatchOption])->required(), "Watch folder")
                ("destination,d", po::value<string>(&options.map[kDestinationOption]), "Destination folder")
                ("task,t", po::value<string>(&options.map[kTaskOption])->required(), "Task number")
                ("command", po::value<string>(&options.map[kCommandOption]), "Shell command to run")
                ("maxdepth", po::value<string>(&options.map[kMaxDepthOption])->default_value("-1"), "Maximum watch depth")
                ("extension,e", po::value<string>(&options.map[kExtensionOption]), "Override destination file extension")
                ("quiet,q", "Quiet mode (no trace output)")
                ("delete", "Delete source file")
                ("log,l", po::value<string>(&options.map[kLogOption]), "Log file")
                ("single,s", "Run task in main thread")
                ("killcount,k", po::value<string>(&options.map[kKillCountOption])->default_value("0"), "Files to process before closedown")
                ("server,s", po::value<string>(&options.map[kServerOption]), "SMTP/IMAP/MongoDB server URL and port")
                ("user,u", po::value<string>(&options.map[kUserOption]), "Account username")
                ("password,p", po::value<string>(&options.map[kPasswordOption]), "Account username password")
                ("recipient,r", po::value<string>(&options.map[kRecipientOption]), "Recipients(s) for email with attached file")
                ("mailbox,m", po::value<string>(&options.map[kMailBoxOption]), "IMAP Mailbox name for drop box")
                ("archive,a", po::value<string>(&options.map[kArchiveOption]), "ZIP destination archive")
                ("database,b", po::value<string>(&options.map[kDatabaseOption]), "Database name")
                ("collection,c", po::value<string>(&options.map[kCollectionOption]), "Collection/Table name")
                ("list", "Display a list of supported tasks.");
                

    }
    
    //
    // If a task option is not present throw an exception.
    //

    static void checkTaskOptions(const vector<string>& options, const po::variables_map& configVarMap) {

        for (auto opt : options) {
            if (!configVarMap.count(opt) || configVarMap[opt].as<string>().empty()) {
                throw po::error("Task option '" + opt + "' missing.");
            }
        }

    }
   
    //
    // If an option is not a valid int throw an exception.For the moment just try to convert to an
    // integer with stoi() (throws an error if the conversion fails). Note stoi() will convert up and to
    // the first non-numeric character so a string like "89ttt" will be converted to 89. 
    //
    
    static void checkIntegerOptions(const vector<string>& options, const po::variables_map& configVarMap) {

        for (auto opt : options) {
            if (configVarMap.count(opt)) {
                try {
                    stoi(configVarMap[opt].as<string>());
                } catch (const exception& e) {
                    throw po::error(opt + " is not a valid integer.");
                }
            }
        }

    }

    //
    // Preprocess program option data and display run options.
    //

    static void preprocessOptions(FPEOptions& options) {
        
        // Make watch/destination paths absolute and create directories
        
        CPath watchPath {options.map[kWatchOption]};
        options.map[kWatchOption] = watchPath.absolutePath();
        if (!CFile::exists(watchPath)) {
            CFile::createDirectory(watchPath);
        }
        
        if (!options.map[kDestinationOption].empty()) {
            CPath destinationPath { options.map[kDestinationOption] };
            options.map[kDestinationOption] = destinationPath.absolutePath();
            if (!CFile::exists(destinationPath)) {
                CFile::createDirectory(destinationPath);
            }
        }
        
        // Display options

        for (auto &option : options.map) {
            if (!option.second.empty()) {
                cout << "*** " << option.first << " = [" << option.second << "] ***" << endl;
            }
        }

    }

    // ================
    // PUBLIC FUNCTIONS
    // ================
    
    //
    // Read in and process command line options using boost.
    //

    FPEOptions fetchCommandLineOptions(int argc, char** argv) {

        FPEOptions options{};
        
        // Set boost version 
        
        options.map["boost-version"] = 
                to_string(BOOST_VERSION / 100000)+"."+
                to_string(BOOST_VERSION / 100 % 1000)+"."+
                to_string(BOOST_VERSION % 100);

        // Define and parse the program options

        po::options_description commandLine("Command Line Options");

        // Command line (first unique then add those shared with config file

        commandLine.add_options()
                ("help", "Display help message")
                (kConfigOption, po::value<string>(&options.map[kConfigOption]), "Configuration file name");

        addCommonOptions(commandLine, options);

        // Config file options

        po::options_description configFile("Configuration File Options");

        addCommonOptions(configFile, options);

        po::variables_map configVariablesMap;

        try {

            // Process command line options

            po::store(po::parse_command_line(argc, argv, commandLine), configVariablesMap);

            // Display options and exit with success

            if (configVariablesMap.count("help")) {
                cout << "File Processing Engine Application" << endl << commandLine << endl;
                exit(EXIT_SUCCESS);
            }
            
            // Display list of available tasks
            
            if (configVariablesMap.count("list")) {
                cout << "File Processing Engine Application Tasks\n\n";
                int taskNo=0;
                shared_ptr<TaskAction> taskFunc;
                taskFunc = TaskAction::create(taskNo);
                while (!taskFunc->getName().empty()){
                    cout << taskNo << "\t" << taskFunc->getName() << "\n";
                    taskFunc =  TaskAction::create(++taskNo);
                }
                exit(EXIT_SUCCESS);
            }

            // Load config file specified

            if (configVariablesMap.count(kConfigOption)) {
                if (CFile::exists(CPath(configVariablesMap[kConfigOption].as<string>().c_str()))) {
                    ifstream configFileStream{configVariablesMap[kConfigOption].as<string>()};
                    if (configFileStream) {
                        po::store(po::parse_config_file(configFileStream, configFile), configVariablesMap);
                    } else {
                        throw po::error("Error opening config file.");
                    }
                } else {
                    throw po::error("Specified config file [" + configVariablesMap[kConfigOption].as<string>() + "] does not exist.");
                }
            }
            
            // Check common integer options
            
            checkIntegerOptions({kTaskOption, kKillCountOption, kMaxDepthOption}, configVariablesMap);
                 
            // Task option validation. Options  valid to the task being
            // run are checked for and if not present an exception is thrown to
            // produce a relevant error message.Any extra options not required 
            // for a task are just ignored.

            if (configVariablesMap.count(kTaskOption)) {
                options.action = TaskAction::create(stoi(configVariablesMap[kTaskOption].as<string>()));
                if (options.action) {
                    checkTaskOptions(options.action->getParameters(), configVariablesMap);
                } else {
                    throw po::error("Error invalid task number.");                 
                }
            }
  
            //
            // Set any boolean flags
            //
            
            // Delete source file

            if (configVariablesMap.count(kDeleteOption)) {
                options.map[kDeleteOption] = "1";  // true
            }

            // No trace output

            if (configVariablesMap.count(kQuietOption)) {
                options.map[kQuietOption] = "1"; // true
            }

            // Use main thread for task.

            if (configVariablesMap.count(kSingleOption)) {
                options.map[kSingleOption] = "1"; // true
            }

            po::notify(configVariablesMap);

        } catch (po::error& e) {
            cerr << "FPE Error: " << e.what() << endl;
            exit(EXIT_FAILURE);
        }

        // Preprocess program option data

        preprocessOptions(options);

        return (options);

    }

} // namespace FPE_ProcCmdLine
