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
// Linux        : Target platform
// Boost        : File system, program options.
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

    using namespace FPE;
    using namespace FPE_TaskActions;

    namespace po = boost::program_options;
    namespace fs = boost::filesystem;

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
    // Process program option data and display run options. All options specified 
    // are displayed even though they may be ignored for the task (this is the 
    // simplest solution rather displaying dependant upon task).
    //

    static void processOptions(FPEOptions& options) {
        
        // Make watch/destination paths absolute

        options.map[kWatchOption] = fs::absolute(options.map[kWatchOption]).lexically_normal().string();
        if (options.map[kWatchOption].back() == '.') options.map[kWatchOption].pop_back();
        if (!options.map[kDestinationOption].empty()) {
            options.map[kDestinationOption] = fs::absolute(options.map[kDestinationOption]).lexically_normal().string();
            if (options.map[kDestinationOption].back() == '.') options.map[kDestinationOption].pop_back();
        }
        
        // Display options

        for (auto &option : options.map) {
            if (!option.second.empty()) {
                cout << "*** " << option.first << " = [" << option.second << "] ***" << endl;
            }
        }
  
        // Create watch folder for task if necessary 

        if (!fs::exists(options.map[kWatchOption])) {
            fs::create_directories(options.map[kWatchOption]);
        }

        // Create destination folder for task if necessary 

        if (!options.map[kDestinationOption].empty() && !fs::exists(options.map[kDestinationOption])) {
            fs::create_directories(options.map[kDestinationOption]);
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
        
        // Set bost version 
        
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

        po::variables_map configVarMap;

        try {

            // Process command line options

            po::store(po::parse_command_line(argc, argv, commandLine), configVarMap);

            // Display options and exit with success

            if (configVarMap.count("help")) {
                cout << "File Processing Engine Application" << endl << commandLine << endl;
                exit(EXIT_SUCCESS);
            }
            
            // Display list of available tasks
            
            if (configVarMap.count("list")) {
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

            if (configVarMap.count(kConfigOption)) {
                if (fs::exists(configVarMap[kConfigOption].as<string>().c_str())) {
                    ifstream configFileStream{configVarMap[kConfigOption].as<string>().c_str()};
                    if (configFileStream) {
                        po::store(po::parse_config_file(configFileStream, configFile), configVarMap);
                    } else {
                        throw po::error("Error opening config file.");
                    }
                } else {
                    throw po::error("Specified config file [" + configVarMap[kConfigOption].as<string>() + "] does not exist.");
                }
            }
            
            // Check common integer options
            
            checkIntegerOptions({kTaskOption, kKillCountOption, kMaxDepthOption}, configVarMap);
                 
            // Task option validation. Options  valid to the task being
            // run are checked for and if not present an exception is thrown to
            // produce a relevant error message.Any extra options not required 
            // for a task are just ignored.

            if (configVarMap.count(kTaskOption)) {
                options.action = TaskAction::create(stoi(configVarMap[kTaskOption].as<string>()));
                if (options.action) {
                    checkTaskOptions(options.action->getParameters(), configVarMap);
                } else {
                    throw po::error("Error invalid task number.");                 
                }
            }
  
            //
            // Set any boolean flags
            //
            
            // Delete source file

            if (configVarMap.count(kDeleteOption)) {
                options.map[kDeleteOption] = "1";  // true
            }

            // No trace output

            if (configVarMap.count(kQuietOption)) {
                options.map[kQuietOption] = "1"; // true
            }

            // Use main thread for task.

            if (configVarMap.count(kSingleOption)) {
                options.map[kSingleOption] = "1"; // true
            }

            po::notify(configVarMap);

        } catch (po::error& e) {
            cerr << "FPE Error: " << e.what() << endl;
            exit(EXIT_FAILURE);
        }

        // Process program option data

        processOptions(options);

        return (options);

    }

} // FPE_ProcCmdLine
