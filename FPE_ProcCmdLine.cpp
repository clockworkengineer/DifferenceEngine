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
// Module: Command Line Parameter Proess
//
// Description: Parse command line parameters and fill in structure ParamArgData.
// 
// Dependencies: C11++, Boost C++ Libraries.
//

// =============
// INCLUDE FILES
// =============

// STL definitions

#include <iostream>

// Process command line arguments

#include "FPE_ProcCmdLine.hpp"

// Boost program options processing

#include "boost/program_options.hpp" 

namespace po = boost::program_options;

// Boost file system library definitions

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

// ===============
// LOCAL FUNCTIONS
// ===============

// ================
// PUBLIC FUNCTIONS
// ================

//
// Read in and process command line arguments using boost. Note this is the only 
// component that uses std::cout and std:cerr directly and not the thread safe 
// coutstr/cerrstr but that is not necessary as still in single thread mode when
// reading and processing parameters.
//

void procCmdLine (int argc, char** argv, ParamArgData &argData) {
    
        // Default values
    
        argData.bDeleteSource = false;
        argData.bFileCopy = false;
        argData.bVideoConversion = false;
        argData.bRunCommand = false;
        argData.maxWatchDepth = -1;
        argData.bDeleteSource = false;
        argData.extension  = "";
        argData.bQuiet = false;
        argData.killCount = 0;
        argData.bSingleThread = false;
        argData.logFileName = "";

        // Define and parse the program options

        po::options_description desc("Options");
        desc.add_options()
                ("help", "Print help messages")
                ("copy", "Task = File Copy Watcher")
                ("video", "Task = Video Conversion Watcher")
                ("command", po::value<std::string>(&argData.commandToRun), "Task = Run Shell Command")
                ("watch,w", po::value<std::string>(&argData.watchFolder)->required(), "Watch Folder")
                ("destination,d", po::value<std::string>(&argData.destinationFolder)->required(), "Destination Folder")
                ("maxdepth", po::value<int>(&argData.maxWatchDepth), "Maximum Watch Depth")
                 ("extension,e", po::value<std::string>(&argData.extension), "Override destination file extension")
                ("quiet,q","Quiet mode (no trace output)")
                ("delete", "Delete Source File")
                ("log,l",po::value<std::string>(&argData.logFileName), "Log file" )
                ("single,s","Run task in main thread")
                ("killcount,k",po::value<int>(&argData.killCount), "Files to process before closedown");

        po::variables_map vm;

        try {
            
            int taskCount=0;
            
            // Process arguments

            po::store(po::parse_command_line(argc, argv, desc), vm);

            // Display options and exit with success
            
            if (vm.count("help")) {
                std::cout << "File Processing Engine Application" << std::endl << desc << std::endl;
                exit(EXIT_SUCCESS);
            }

            // Copy watched files.
            
            if (vm.count("copy")) {
                argData.bFileCopy=true;
                taskCount++;
            }
            
            // Convert watched video files
            
            if (vm.count("video")) {
                argData.bVideoConversion=true;
                argData.commandToRun = kHandbrakeCommand;
                taskCount++;
            }

            // Run command on watched files
            
            if (vm.count("command")) {
                argData.bRunCommand=true;
                taskCount++;
             }
            
            // Delete source file
            
            if (vm.count("delete")) {
                argData.bDeleteSource=true;
             }
      
            // No trace output
            
            if (vm.count("quiet")) {
                argData.bQuiet=true;
             }
 
            // Use main thread for task.
            
            if (vm.count("single")) {
                argData.bSingleThread=true;
             }
 
            // Default task file copy. More than one task throw error.
            
            if (taskCount==0) {
                argData.bFileCopy=true;
            } else if (taskCount>1) {
                 throw po::error("More than one task specified");
            }

            po::notify(vm);

            // Make watch/destination paths absolute
        
            argData.watchFolder = fs::absolute(argData.watchFolder).string();
            argData.destinationFolder = fs::absolute(argData.destinationFolder).string();
  
        } catch (po::error& e) {
            std::cerr << "FPE Error: " << e.what() << std::endl << std::endl;
            std::cerr << desc << std::endl;
            exit(EXIT_FAILURE);
        }
    
}

