/*
 * File:   FPE.cpp
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

// Task Action functions

#include "FPE_ActionFuncs.hpp"

// Process command line arguments

#include "FPE_ProcCmdLine.hpp"

//
// Create task and run in thread.
//

void createTaskAndActivate( std::string taskName, std::string watchFolder, int watchDepth, void (*taskFcn)(std::string, std::string, std::shared_ptr<void> fnData), std::shared_ptr<void> fnData) {

    // Create task object

    FPETask task(taskName, watchFolder, watchDepth, taskFcn, fnData);
 
    // Create task object thread and wait

    std::unique_ptr<std::thread> taskThread;

    taskThread.reset(new std::thread(&FPETask::monitor, &task));

    taskThread->join();

}

//
// === FPE MAIN ENTRY POINT ===
//

int main(int argc, char** argv) {

    
    try {

        ParamArgData argData;
        ActFnData   *fcnData = new ActFnData();
        std::shared_ptr<void> fnData(fcnData);
     
        // Process FPE command line arguments.
    
        procCmdLine(argc, argv, argData);

        // Copy relevant data from command line structure to task function data
        
        fcnData->watchFolder = argData.watchFolder;
        fcnData->destinationFolder = argData.destinationFolder;
        fcnData->bDeleteSource = argData.bDeleteSource;
        fcnData->commandToRun = argData.commandToRun;
        
        // FPE up and running
        
        std::cout << "FPE Running..." << std::endl;
        
        // Display BOOST version
        
        std::cout << "Using Boost " << BOOST_VERSION / 100000 << "." // major version
                << BOOST_VERSION / 100 % 1000 << "." // minor version
                << BOOST_VERSION % 100 // patch level
                << std::endl;

       // Make watch/destination paths absolute
        
        fcnData->watchFolder = fs::absolute(fcnData->watchFolder);
        fcnData->destinationFolder = fs::absolute(fcnData->destinationFolder);
    
        // Create destination folder for task
        
        if (!fs::exists(fcnData->destinationFolder)) {
            std::cout << "Destination Folder " << fcnData->destinationFolder << " DOES NOT EXIST." << std::endl;

            if (fs::create_directory(fcnData->destinationFolder)) {
                std::cout << "Creating Destination Folder " << fcnData->destinationFolder << std::endl;
            }
        }
   
        // Create task object

        if (argData.bFileCopy) {
            createTaskAndActivate(std::string("File Copy"), fcnData->watchFolder.string(), argData.maxWatchDepth, copyFile, fnData);
        } else if (argData.bVideoConversion) {
            createTaskAndActivate(std::string("Video Conversion"), fcnData->watchFolder.string(), argData.maxWatchDepth, handBrake, fnData);
        } else {
            createTaskAndActivate(std::string("Run Command"), fcnData->watchFolder.string(), argData.maxWatchDepth, runCommand, fnData);
        }
        
        
    //
    // Catch any errors
    //    

    } catch (const fs::filesystem_error & e) {
        std::cerr << "BOOST file system exception occured: " << e.what() << std::endl;
        exit(ERROR_UNHANDLED_EXCEPTION);
   } catch (std::runtime_error &e) {
        std::cerr << "Caught a runtime_error exception: " << e.what() << std::endl;
    } catch (std::exception & e) {
        std::cerr << "STL exception occured: " << e.what() << std::endl;
        exit( ERROR_UNHANDLED_EXCEPTION);
    } catch (...) {
        std::cerr << "unknown exception occured" << std::endl;
        exit(ERROR_UNHANDLED_EXCEPTION);
    }

    exit(SUCCESS);

} 