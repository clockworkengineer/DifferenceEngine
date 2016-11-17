/*
 * File:   FPECPP.cpp
 * 
 * Author: Robert Tizzard
 *
 * Created on October 24, 2016, 2:34 PM
 *
 * The MIT License
 *
 * Copyright 2016 Robert Tizzard.
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

// BOOST program options processing

#include "boost/program_options.hpp" 

namespace po = boost::program_options;

// Globals

fs::path watchFolder;           // Watch Folder
fs::path destinationFolder;     // Destination Folder for copies.

bool fileCopy=false;            // Task file copy
bool videoConversion=false;     // Task video conversion

// Command line exit status

namespace { 
    const size_t ERROR_IN_COMMAND_LINE = 1;
    const size_t SUCCESS = 0;
    const size_t ERROR_UNHANDLED_EXCEPTION = 2;
}

//
// Convert video file action function. Convert passed in file to MP4
// using handbrakes "normal preset" to desination folder (--destination).
//

void handBrake(std::string filenamePathStr, std::string filenameStr) {
  
    fs::path sourceFile(filenamePathStr+filenameStr);
    fs::path destinationFile(destinationFolder.string());

    try {

        // Create destination file name
        
        destinationFile /= sourceFile.stem().string();
        destinationFile.replace_extension(".mp4");
 
        // Convert file
        
        std::string command = "/usr/local/bin/HandBrakeCLI -i " + sourceFile.string()+ " -o " + destinationFile.string() + " --preset=\"Normal\" >> /home/pi/FPE_handbrake.log 2>&1";
        
        std::cout << command << std::endl;

        auto  result=0;
        if ((result = std::system(command.c_str()))==0) {
            std::cout << "File conversion success." << std::endl;
        } else {
            std::cout << "File conversion error: " << result << std::endl;
        }
        
    //
    // Catch any errors locally and report so that thread keeps running.
    //   
        
   } catch (const fs::filesystem_error & e) {
        std::cerr << "BOOST file system exception occured: " << e.what() << std::endl;
    } catch (std::exception & e) {
        std::cerr << "STL exception occured: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "unknown exception occured" << std::endl;
    }

}

// Copy file action function. Copy passed file to destination folder/directory 
// keeping the sources directory structure.

void copyFile(std::string filenamePathStr, std::string filenameStr) {

    // Destination file path += ("filename path" - "watch folder path")
    
    std::string destinationPathStr(destinationFolder.string() + 
                filenamePathStr.substr((watchFolder.string()).length()));

    try {

        // Construct full destination path if needed
        
        if (!fs::exists(destinationPathStr)) {
            if (fs::create_directories(destinationPathStr)) {
                std::cout << "CREATED :" + destinationPathStr << std::endl;
            } else {
                std::cerr << "CREATED FAILED FOR :" + destinationPathStr << std::endl;
            }
        }

        // Add filename to source and destination paths
        
        filenamePathStr += filenameStr;
        destinationPathStr += filenameStr;

        // Currently only copy file if it doesn't already exist.
        
        if (!fs::exists(destinationPathStr)) {
           std::cout << "COPY FROM [" << filenamePathStr << "] TO [" << destinationPathStr << "]" << std::endl;
           fs::copy_file(filenamePathStr, destinationPathStr, fs::copy_option::none);
        } else {
            std::cout << "DESTINATION ALREADY EXISTS : " + destinationPathStr << std::endl;
        }
        
    //
    // Catch any errors locally and report so that thread keeps running.
    // 
        
   } catch (const fs::filesystem_error & e) {
        std::cerr << "BOOST file system exception occured: " << e.what() << std::endl;
    } catch (std::exception & e) {
        std::cerr << "STL exception occured: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "unknown exception occured" << std::endl;
    }

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
                ("watch,w", po::value<fs::path>(&watchFolder)->required(), "Watch Folder")
                ("destination,d", po::value<fs::path>(&destinationFolder)->required(), "Destination Folder")
                ("copy", "File Copy Watcher")
                ("video", "Video Conversion Watcher");

        po::variables_map vm;

        try {

            po::store(po::parse_command_line(argc, argv, desc), vm);

            // Display options and exit with success
            
            if (vm.count("help")) {
                std::cout << "File Processing Engine Application" << std::endl
                        << desc << std::endl;
                return SUCCESS;
            }

            // Copy watched files.
            
            if (vm.count("copy")) {
                fileCopy=true;
            }
            
            // Convert watched video files
            
            if (vm.count("video")) {
                videoConversion=true;
            }
            
            // Default to copy
            
            if (!fileCopy && !videoConversion) {
                fileCopy=true;
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
        
        watchFolder = fs::absolute(watchFolder);
        destinationFolder = fs::absolute(destinationFolder);

        // Create destination folder for task
        
        if (!fs::exists(destinationFolder)) {
            std::cout << "Destination Folder " << destinationFolder << " DOES NOT EXIST." << std::endl;

            if (fs::create_directory(destinationFolder)) {
                std::cout << "Creating Destination Folder " << destinationFolder << std::endl;
            }
        }
   
        // Create task object
        
        std::shared_ptr<FPETask> task;
        
        if (fileCopy) {
            task.reset(new FPETask(std::string("File Copy"), watchFolder.string(), copyFile));
        } else {
            task.reset(new FPETask(std::string("Video Conversion"), watchFolder.string(), handBrake));
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