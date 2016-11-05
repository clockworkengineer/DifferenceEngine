/*
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
/*
 * File:   FPECPP.cpp
 * Author: Robert Tizzard
 *
 * Created on October 24, 2016, 2:34 PM
 */

#include "FPETask.hpp"

#include "boost/program_options.hpp" 

namespace po = boost::program_options;

fs::path watchFolder;           // Watch Folder
fs::path destinationFolder;     // Destination Folder for copies.

// Command line exit status

namespace { 
    const size_t ERROR_IN_COMMAND_LINE = 1;
    const size_t SUCCESS = 0;
    const size_t ERROR_UNHANDLED_EXCEPTION = 2;
} // namespace 

// Copy file task action function. Copy passed file to destination folder/directory 
// keeping the sources directory structure.

void copyFile(std::string filenamePathStr, std::string filenameStr) {

    // Destination path += (filename path - watch folder path)
    
    std::string destinationPathStr(destinationFolder.string() + 
                filenamePathStr.substr((watchFolder.string()).length()));

    try {

        // Create destination folder if needed
        
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
    // Catch any errors
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
                ("destination,d", po::value<fs::path>(&destinationFolder)->required(), "Destination Folder");

        po::variables_map vm;

        try {

            po::store(po::parse_command_line(argc, argv, desc), vm);

            if (vm.count("help")) {
                std::cout << "File Processing Engine Application" << std::endl
                        << desc << std::endl;
                return SUCCESS;
            }

            po::notify(vm); // throws on error, so do after help in case there are any problems 

        } catch (po::error& e) {
            std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
            std::cerr << desc << std::endl;
            return ERROR_IN_COMMAND_LINE;
        }

        // FPE up and running
        
        std::cout << "FPE Running" << std::endl;
        
        // Display BOOST version
        
        std::cout << "Using Boost " << BOOST_VERSION / 100000 << "." // major version
                << BOOST_VERSION / 100 % 1000 << "." // minor version
                << BOOST_VERSION % 100 // patch level
                << std::endl;

        // Create destination folder for copy file
        
        if (!fs::exists(destinationFolder)) {
            std::cout << "Destination Folder " << destinationFolder << " DOES NOT EXIST." << std::endl;

            if (fs::create_directory(destinationFolder)) {
                std::cout << "Creating Destination Folder " << destinationFolder << std::endl;
            }
        }

        // Make watch/destination paths absolute
        
        watchFolder = fs::absolute(watchFolder);
        destinationFolder = fs::absolute(destinationFolder);

        // Create task object
        
        FPETask task(std::string("File Copy"), watchFolder.string(), copyFile);

        // Create task object thread and wait
        
        std::thread taskThread(&FPETask::monitor, &task);
        taskThread.join();

    //
    // Catch any errors
    //    

    } catch (const fs::filesystem_error & e) {
        std::cerr << "BOOST file system exception occured: " << e.what() << std::endl;
        return ERROR_UNHANDLED_EXCEPTION;
    } catch (std::exception & e) {
        std::cerr << "STL exception occured: " << e.what() << std::endl;
        return ERROR_UNHANDLED_EXCEPTION;
    } catch (...) {
        std::cerr << "unknown exception occured" << std::endl;
        return ERROR_UNHANDLED_EXCEPTION;
    }

    return SUCCESS;

} 