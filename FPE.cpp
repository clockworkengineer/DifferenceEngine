#include "FPE_HOST.hpp"
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

// STL definitions

#include <iostream>
#include <mutex>
#include <system_error>

// Task Action functions

#include "FPE_ActionFuncs.hpp"

// Task class

#include "FPE_Task.hpp" 

// Process command line arguments

#include "FPE_ProcCmdLine.hpp"

// Boost file system and format libraries definitions

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;


//
// Standard cout for string of vectors. All calls to this function from different
// threads are guarded by mutex mOutput (this is static just to keep it local to the
// function).
//

void coutstr(const std::vector<std::string>& outstr) {

    static std::mutex mOutput;
    std::unique_lock<std::mutex> locker(mOutput);
    
    if (!outstr.empty()) {
        
        for (auto str : outstr)
            std::cout << str;

        std::cout << std::endl;

    }

}

//
// Standard cerr for string of vectors. All calls to this function from different
// threads are guarded by mutex mError (this is static just to keep it local to the
// function).
//

void cerrstr(const std::vector<std::string>& errstr) {

    static std::mutex mError;
    std::unique_lock<std::mutex> locker(mError);
   
    if (!errstr.empty()) {

        for (auto str : errstr)
            std::cerr << str;

        std::cerr << std::endl;

    }

}

//
// Create task and run in thread.
//

void createTaskAndActivate( const std::string &taskName, const std::string &watchFolder, int watchDepth, TaskActionFcn taskActFcn, std::shared_ptr<void> fnData) {

    // ASSERT if strings length 0 , pointer parameters NULL
    
    assert(taskName.length() != 0);
    assert(watchFolder.length() != 0);
    assert(taskActFcn!=nullptr);
    assert(fnData!=nullptr);

    // Use function data to access set coutstr/cerrstr
    
    ActFnData *funcData = static_cast<ActFnData *> (fnData.get());
    
    // Set task options ( no kill count and all output to local coutstr/cerrstr.
    
    std::shared_ptr<TaskOptions> taskOpt;
    
    taskOpt.reset(new TaskOptions { 0, funcData->coutstr, funcData->cerrstr} ) ;
             
    // Create task object

    FPE_Task task(taskName, watchFolder, taskActFcn, fnData,  watchDepth, taskOpt);
 
    // Create task object thread and start to watch

    std::unique_ptr<std::thread> taskThread;
    taskThread.reset(new std::thread(&FPE_Task::monitor, &task));
    taskThread->join();
    
    //
    // For Non thread variant just uncomment below and comment out thread creation block
    // above. (May make this a run time control parameter).
    //
    // task.monitor();
    //
   

}

//
// === FPE MAIN ENTRY POINT ===
//

int main(int argc, char** argv) {
    
    try {
           
        // Process FPE command line arguments.
  
        ParamArgData argData;
   
        procCmdLine(argc, argv, argData);  
  
        // FPE up and running
        
        coutstr({"FPE Running..."});
        
        // Display BOOST version
        
        coutstr({"Using Boost ", 
                std::to_string(BOOST_VERSION / 100000), ".",      // major version
                std::to_string(BOOST_VERSION / 100 % 1000), ".",  // minor version
                std::to_string(BOOST_VERSION % 100)});            // patch level

        // Create watch folder for task.

        if (!fs::exists(argData.watchFolder)) {
            coutstr({"Watch folder [", argData.watchFolder, "] DOES NOT EXIST."});
            if (fs::create_directory(argData.watchFolder)) {
                coutstr({"Creating watch folder [", argData.watchFolder, "]"});
            }
        }
                
        // Create destination folder for task
        
        if (!fs::exists(argData.destinationFolder)) {
            coutstr({"Destination folder ", argData.destinationFolder, " does not exist." });
            if (fs::create_directory(argData.destinationFolder)) {
                coutstr({"Creating destination folder ", argData.destinationFolder});
            }
        }
   
        // Signal file copy task
        
        if (argData.bFileCopy) {
            coutstr({ "*** FILE COPY TASK ***"});
        }
                
        // Signal video conversion task
        
        if (argData.bVideoConversion) {
            coutstr({ "*** VIDEO CONVERSION TASK ***"});
        }
  
         // Signal run command task
        
        if (argData.bRunCommand) {
            coutstr({ "*** RUN COMMAND TASK ***"});
        }
  
        // Signal quiet mode
        
        if (argData.bQuiet) {
            coutstr({ "*** QUIET MODE ***"});
        }
    
        // Signal source will be deleted on success
        
        if (argData.bDeleteSource) {
            coutstr({ "*** DELETE SOURCE FILE ON SUCESSFUL PROCESSING ***"});
        }
    
       // Create function data (wrap in void shared pointer for passing to task).
        
        std::shared_ptr<void> fnData(new ActFnData {argData.watchFolder, 
        argData.destinationFolder, argData.commandToRun, argData.bDeleteSource, 
        argData.extension, ((argData.bQuiet)? nullptr : coutstr), ((argData.bQuiet)? nullptr :cerrstr)});
 
        // Create task object

        if (argData.bFileCopy) {
            createTaskAndActivate(std::string("File Copy"), argData.watchFolder, argData.maxWatchDepth, copyFile, fnData);
        } else if (argData.bVideoConversion) {
            createTaskAndActivate(std::string("Video Conversion"), argData.watchFolder, argData.maxWatchDepth, handBrake, fnData);
        } else {
            createTaskAndActivate(std::string("Run Command"), argData.watchFolder, argData.maxWatchDepth, runCommand, fnData);
        }
        
        coutstr({"FPE Exiting."});
        
    //
    // Catch any errors
    //    

    } catch (const fs::filesystem_error & e) {
        cerrstr({"BOOST file system exception occured: [", e.what(), "]"});
        exit(1);
   } catch (std::system_error &e) {
        cerrstr({"Caught a runtime_error exception: [", e.what(), "]"});
        exit(1);
     } catch (std::exception & e) {
        cerrstr({"Standard exception occured: [", e.what(), "]"});
        exit(1);
    }

    exit(0);

} 