#include "HOST.hpp"
/*
 * File: FPE.cpp
 *
 * Author: Robert Tizzard
 *
 * Created on October 24, 2016, 2:34 PM
 *
 * Copyright 2016.
 *
 */

//
// Program: File Processing Engine
//
// Description: This is a generic file processing engine that sets up a
// watch folder and waits for files/directories to be copied to it. Any 
// added directories are also watched (this is recursive) but any added files
// are be processed using one of its built in task action functions
// 
// 1) File copy
// 2) Video file conversion (using Handbrake)
// 3) Run shell command
// 4) Email file as a attachment
// 
// All of this can be setup by using parameters  passed to the program from
// command line (FPE --help for a full list).
// 
// Dependencies: C11++, Classes (CFileTask, CRedirect, CIFileApprise, 
//               CLogger, CMailSend), Linux, Boost C++ Libraries.
//
 
// =============
// INCLUDE FILES
// =============

// File Processing Engine Definitions

#include "FPE.hpp"

// ===============
// LOCAL FUNCTIONS
// ===============

//
// Exit with error message/status
//

void exitWithError(std::string errmsg) {

    // Closedown email, display error and exit.
    
    CMailSend::closedown();
    
    CLogger::cerrstr({errmsg});

    exit(EXIT_FAILURE);

}

//
// Create task and run in thread.
//

void createTaskAndRun(const std::string& taskName, ParamArgData& argData, CFileTask::TaskActionFcn taskActFcn) {

    // ASSERT if strings length 0 , pointer parameters NULL

    assert(taskName.length() != 0);
    assert(taskActFcn != nullptr);

    // Date and Time stamp output
    
    CLogger::setDateTimeStamped(true);
        
    // Create function data (wrap in void shared pointer for passing to task).

    std::shared_ptr<void> fnData(new ActFnData{argData.watchFolder,
        argData.destinationFolder, argData.commandToRun, argData.bDeleteSource,
        argData.extension, argData.userName, argData.userPassword, argData.serverURL,
        argData.emailRecipient, ((argData.bQuiet) ? CLogger::noOp : CLogger::coutstr), 
       ((argData.bQuiet) ? CLogger::noOp : CLogger::cerrstr)});

    // Use function data to access set coutstr/cerrstr

    ActFnData *funcData = static_cast<ActFnData *> (fnData.get());

    // Set task options ( kill count and all output to locally defined  coutstr/cerrstr.

    std::shared_ptr<CFileTask::TaskOptions> options;

    options.reset(new CFileTask::TaskOptions{argData.killCount, funcData->coutstr, funcData->cerrstr});

    // Create task object

    CFileTask task(taskName, argData.watchFolder, taskActFcn, fnData, argData.maxWatchDepth, options);

    // Create task object thread and start to watch else use FPE thread.

    if (!argData.bSingleThread) {
        std::unique_ptr<std::thread> taskThread;
        taskThread.reset(new std::thread(&CFileTask::monitor, &task));
        taskThread->join();
    } else {
        task.monitor();
    
    }

    // If an exception occurred rethrow (end of chain)

    if (task.getThrownException()) {
        std::rethrow_exception(task.getThrownException());
    }

}

// ============================
// ===== MAIN ENTRY POINT =====
// ============================

int main(int argc, char** argv) {

    ParamArgData argData;   // Command lien arguments  
    
    try {
        
        // Initialise CMailSend internals
        
        CMailSend::init();
        
        // std::cout to logfile if parameter specified.
        
        CRedirect logFile{std::cout};

        // Process FPE command line arguments.
     
        procCmdLine(argc, argv, argData);

        // FPE up and running

        CLogger::coutstr({"FPE Running..."});

        // Display BOOST version

        CLogger::coutstr({"Using Boost ",
            std::to_string(BOOST_VERSION / 100000), ".", // major version
            std::to_string(BOOST_VERSION / 100 % 1000), ".", // minor version
            std::to_string(BOOST_VERSION % 100)}); // patch level

        // Email does not require a destination folder
            
        if (argData.bEmailFile) {
            argData.destinationFolder = "";
        }
            
        // Create watch folder for task.

        if (!fs::exists(argData.watchFolder)) {
            CLogger::coutstr({"Watch folder [", argData.watchFolder, "] DOES NOT EXIST."});
            if (fs::create_directory(argData.watchFolder)) {
                CLogger::coutstr({"Creating watch folder [", argData.watchFolder, "]"});
            }
        }
            
        CLogger::coutstr({"*** WATCH FOLDER = [",argData.watchFolder , "] ***"});

        // Create destination folder for task

        if (!argData.destinationFolder.empty() && !fs::exists(argData.destinationFolder)) {
            CLogger::coutstr({"Destination folder ", argData.destinationFolder, " does not exist."});
            if (fs::create_directory(argData.destinationFolder)) {
                CLogger::coutstr({"Creating destination folder ", argData.destinationFolder});
            }
        }

        // Run does not require a destination
        
        if (!argData.destinationFolder.empty()) {
            CLogger::coutstr({"*** DESTINATION FOLDER = [", argData.destinationFolder, "] ***"});
        }
        
        // Signal config file used
        
        if (!argData.configFileName.empty()) {
            CLogger::coutstr({"*** CONFIG FILE = [", argData.configFileName, "] ***"});
        }
    
        // Signal email file task

        if (argData.bEmailFile) {
            CLogger::coutstr({"*** EMAIL FILE TASK ***"});
        }

        // Signal file copy task

        if (argData.bFileCopy) {
            CLogger::coutstr({"*** FILE COPY TASK ***"});
        }

        // Signal video conversion task

        if (argData.bVideoConversion) {
            CLogger::coutstr({"*** VIDEO CONVERSION TASK ***"});
        }

        // Signal run command task

        if (argData.bRunCommand) {
            CLogger::coutstr({"*** RUN COMMAND TASK ***"});
        }

        // Signal quiet mode

        if (argData.bQuiet) {
            CLogger::coutstr({"*** QUIET MODE ***"});
        }

        // Signal source will be deleted on success

        if (argData.bDeleteSource) {
            CLogger::coutstr({"*** DELETE SOURCE FILE ON SUCESSFUL PROCESSING ***"});
        }

        // Signal using single thread

        if (argData.bSingleThread) {
            CLogger::coutstr({"*** SINGLE THREAD ***"});
        }

        // Signal using killCount

        if (argData.killCount) {
            CLogger::coutstr({"*** KILL COUNT = ", std::to_string(argData.killCount), " ***"});
        }

        // Output to log file ( CRedirect(std::cout) is the simplest solution). Once the try is exited
        // CRedirect object will be destroyed and cout restored.

        if (!argData.logFileName.empty()) {
            CLogger::coutstr({"*** LOG FILE = [", argData.logFileName, "] ***"});
            logFile.change(argData.logFileName, std::ios_base::out | std::ios_base::app);
            CLogger::coutstr({std::string(100, '=')});
        }

        // Create task object

        if (argData.bFileCopy) {
            createTaskAndRun(std::string("File Copy"), argData, copyFile);
        } else if (argData.bVideoConversion) {
            createTaskAndRun(std::string("Video Conversion"), argData, handBrake);
        } else if (argData.bEmailFile) {
            createTaskAndRun(std::string("Email Attachment"), argData, emailFile);
        } else {
            createTaskAndRun(std::string("Run Command"), argData, runCommand);
        }

    //
    // Catch any errors
    //    

    } catch (const fs::filesystem_error & e) {
        exitWithError(std::string("BOOST file system exception occured: [")+e.what()+"]");
    } catch (const std::system_error &e) {
        exitWithError(std::string("Caught a system_error exception: [")+e.what()+"]");
    } catch (const std::exception & e) {
        exitWithError(std::string("Standard exception occured: [")+e.what()+"]");
    }

    CLogger::coutstr({"FPE Exiting."});

    // Closedown mail
    
    CMailSend::closedown();
 
    exit(EXIT_SUCCESS);

} 