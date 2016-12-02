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

// Task class

#include "FPE_Task.hpp" 

// Process command line arguments

#include "FPE_ProcCmdLine.hpp"

// BOOST program options processing

#include "boost/program_options.hpp" 

namespace po = boost::program_options;

// Handbrake command and default command if --command not specified

const std::string kHandbrakeCommand = "/usr/local/bin/HandBrakeCLI -i %1% -o %2% --preset=\"Normal\" >> /home/pi/FPE_handbrake.log 2>&1";
const std::string kCommandToRun = "echo %1%";

//
// Read in and process command line arguments using boost.
//

void procCmdLine (int argc, char** argv, ParamArgData& argData) {
    
        // Default values
    
        argData.bDeleteSource = false;
        argData.bFileCopy = false;
        argData.bVideoConversion = false;
        argData.bRunCommand = false;
        argData.maxWatchDepth = -1;
        argData.bDeleteSource = false;

        // Define and parse the program options

        po::options_description desc("Options");
        desc.add_options()
                ("help", "Print help messages")
                ("watch,w", po::value<fs::path>(&argData.watchFolder)->required(), "Watch Folder")
                ("destination,d", po::value<fs::path>(&argData.destinationFolder)->required(), "Destination Folder")
                ("maxdepth", po::value<int>(&argData.maxWatchDepth), "Maximum Watch Depth")
                ("copy", "File Copy Watcher")
                ("video", "Video Conversion Watcher")
                ("command", po::value<std::string>(&argData.commandToRun), "Command Watcher")
                ("delete", "Delete Source File");

        po::variables_map vm;

        try {
            
            // Process arguments

            po::store(po::parse_command_line(argc, argv, desc), vm);

            // Display options and exit with success
            
            if (vm.count("help")) {
                std::cout << "File Processing Engine Application" << std::endl
                        << desc << std::endl;
                exit(SUCCESS);
            }

            // Copy watched files.
            
            if (vm.count("copy")) {
                argData.bFileCopy=true;
            }
            
            // Convert watched video files
            
            if (vm.count("video")) {
                argData.bVideoConversion=true;
            }

            // Run command on watched files
            
            if (vm.count("command")) {
                argData.bRunCommand=true;
             }
            
            // Delete source file
            
            if (vm.count("delete")) {
                argData.bDeleteSource=true;
             }
      
            // Default to command
            
            if (!argData.bFileCopy && !argData.bVideoConversion) {
                argData.bRunCommand=true;
            }
   
            po::notify(vm);

            // --video with --command override Handbrake video conversion for passed command
            
            if (!argData.bRunCommand) {
                if (argData.bVideoConversion) {
                    argData.commandToRun = kHandbrakeCommand;
                } else {
                    argData.commandToRun = kCommandToRun;
                }
            }

            // Make watch/destination paths absolute
        
            argData.watchFolder = fs::absolute(argData.watchFolder);
            argData.destinationFolder = fs::absolute(argData.destinationFolder);
  
        } catch (po::error& e) {
            std::cerr << "FPE Error: " << e.what() << std::endl << std::endl;
            std::cerr << desc << std::endl;
            exit(ERROR_IN_COMMAND_LINE);
        }
    
}

