/*
 * File:   FPECPP.cpp
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

#include "FPETask.hpp" 

// Task Action functions

#include "FPE_ActionFuncs.hpp"

// BOOST program options processing

#include "boost/program_options.hpp" 

namespace po = boost::program_options;

// Globals

fs::path gWatchFolder;           // Watch Folder
fs::path gDestinationFolder;     // Destination Folder for copies.

// Handbrake command and default command if --command not specified

std::string gHandbrakeCommand = "/usr/local/bin/HandBrakeCLI -i %1% -o %2% --preset=\"Normal\" >> /home/pi/FPE_handbrake.log 2>&1";
std::string gCommandToRun = "echo %1%";

bool gFileCopy=false;            // Task file copy
bool gVideoConversion=false;     // Task video conversion
bool gRunCommand=false;          // Task perform command
int  gMaxWatchDepth=-1;          // Watch depth -1=all,0=just watch folder,1=next level down etc.
bool gDeleteSource=false;        // Delete source file

// Command line exit status

namespace { 
    const size_t ERROR_IN_COMMAND_LINE = 1;
    const size_t SUCCESS = 0;
    const size_t ERROR_UNHANDLED_EXCEPTION = 2;
}

//
// === FPE MAIN ENTRY POINT ===
//

int main(int argc, char** argv) {

    try {

        // Define and parse the program options

        po::options_description desc("Options");
        desc.add_options()
                ("help", "Print help messages")
                ("watch,w", po::value<fs::path>(&gWatchFolder)->required(), "Watch Folder")
                ("destination,d", po::value<fs::path>(&gDestinationFolder)->required(), "Destination Folder")
                ("maxdepth", po::value<int>(&gMaxWatchDepth), "Maximum Watch Depth")
                ("copy", "File Copy Watcher")
                ("video", "Video Conversion Watcher")
                ("command", po::value<std::string>(&gCommandToRun), "Command Watcher")
                ("delete", "Delete Source File");

        po::variables_map vm;

        try {
            
            // Process arguments

            po::store(po::parse_command_line(argc, argv, desc), vm);

            // Display options and exit with success
            
            if (vm.count("help")) {
                std::cout << "File Processing Engine Application" << std::endl
                        << desc << std::endl;
                return SUCCESS;
            }

            // Copy watched files.
            
            if (vm.count("copy")) {
                gFileCopy=true;
            }
            
            // Convert watched video files
            
            if (vm.count("video")) {
                gVideoConversion=true;
            }

            // Run command on watched files
            
            if (vm.count("command")) {
                gRunCommand=true;
             }
            
            // Delete source file
            
            if (vm.count("delete")) {
                gDeleteSource=true;
             }
      
            // Default to command
            
            if (!gFileCopy && !gVideoConversion) {
                gRunCommand=true;
            }
   
            po::notify(vm);

        } catch (po::error& e) {
            std::cerr << "FPE Error: " << e.what() << std::endl << std::endl;
            std::cerr << desc << std::endl;
            return ERROR_IN_COMMAND_LINE;
        }

        // FPE up and running
        
        std::cout << "FPE Running..." << std::endl;
        
        // Display BOOST version
        
        std::cout << "Using Boost " << BOOST_VERSION / 100000 << "." // major version
                << BOOST_VERSION / 100 % 1000 << "." // minor version
                << BOOST_VERSION % 100 // patch level
                << std::endl;

       // Make watch/destination paths absolute
        
        gWatchFolder = fs::absolute(gWatchFolder);
        gDestinationFolder = fs::absolute(gDestinationFolder);

        // --video with --command override Handbrake video conversion for passed command
        
        if (gVideoConversion && gRunCommand) {
             gHandbrakeCommand = gCommandToRun;
        }
  
        // Create destination folder for task
        
        if (!fs::exists(gDestinationFolder)) {
            std::cout << "Destination Folder " << gDestinationFolder << " DOES NOT EXIST." << std::endl;

            if (fs::create_directory(gDestinationFolder)) {
                std::cout << "Creating Destination Folder " << gDestinationFolder << std::endl;
            }
        }
   
        // Create task object
        
        std::shared_ptr<FPETask> task;
        
        if (gFileCopy) {
            task.reset(new FPETask(std::string("File Copy"), gWatchFolder.string(), gMaxWatchDepth, copyFile));
        } else if (gVideoConversion) {
            task.reset(new FPETask(std::string("Video Conversion"), gWatchFolder.string(), gMaxWatchDepth, handBrake));
        } else {
            task.reset(new FPETask(std::string("Run Command"), gWatchFolder.string(), gMaxWatchDepth, runCommand));
        }
        
        // Create task object thread and wait
  
        std::unique_ptr<std::thread> taskThread;
        
        taskThread.reset(new std::thread(&FPETask::monitor, task)); 
 
        taskThread->join();     
    
    //
    // Catch any errors
    //    

    } catch (const fs::filesystem_error & e) {
        std::cerr << "BOOST file system exception occured: " << e.what() << std::endl;
        return ERROR_UNHANDLED_EXCEPTION;
   } catch (std::runtime_error &e) {
        std::cerr << "Caught a runtime_error exception: " << e.what() << std::endl;
    } catch (std::exception & e) {
        std::cerr << "STL exception occured: " << e.what() << std::endl;
        return ERROR_UNHANDLED_EXCEPTION;
    } catch (...) {
        std::cerr << "unknown exception occured" << std::endl;
        return ERROR_UNHANDLED_EXCEPTION;
    }

    return SUCCESS;

} 