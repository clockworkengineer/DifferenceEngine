#include "FPE_HOST.hpp"
/*
 * File:   FPE_ProcCmdLine.cpp
 * 
 * Author: Robert Tizzard
 *
 * Created on October 24, 2016, 2:34 PM
 *
 * The MIT License
 *
 * Copyright 2016.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <iostream>

// Process command line arguments

#include "FPE_ProcCmdLine.hpp"

// BOOST program options processing

#include "boost/program_options.hpp" 

namespace po = boost::program_options;

// Boost file system and format libraries definitions

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

//
// Read in and process command line arguments using boost.
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

        // Define and parse the program options

        po::options_description desc("Options");
        desc.add_options()
                ("help", "Print help messages")
                ("watch,w", po::value<std::string>(&argData.watchFolder)->required(), "Watch Folder")
                ("destination,d", po::value<std::string>(&argData.destinationFolder)->required(), "Destination Folder")
                ("maxdepth", po::value<int>(&argData.maxWatchDepth), "Maximum Watch Depth")
                ("copy", "Task = File Copy Watcher")
                ("video", "Task = Video Conversion Watcher")
                ("command", po::value<std::string>(&argData.commandToRun), "Task = Run Shell Command")
                ("extension,e", po::value<std::string>(&argData.extension), "Overrde destination file extension")
                ("delete", "Delete Source File");

        po::variables_map vm;

        try {
            
            int taskCount=0;
            
            // Process arguments

            po::store(po::parse_command_line(argc, argv, desc), vm);

            // Display options and exit with success
            
            if (vm.count("help")) {
                std::cout << "File Processing Engine Application" << std::endl << desc << std::endl;
                exit(SUCCESS);
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
            exit(ERROR_IN_COMMAND_LINE);
        }
    
}

