#include "HOST.hpp"
/*
 * File: FPE.cpp
 *
 * Author: Robert Tizzard
 *
 * Created on October 24, 2016, 2:34 PM
 *
 * Description: This is a generic file processing engine that sets up a
 * watch folder and waits for files/directories to be copied to it. Any 
 * added directories are also watched (this is recursive) but any added files
 * are be processed using one of its built in task action tasks
 * 
 * 1) File copy
 * 2) Video file conversion (using handbrake)
 * 3) Run shell command
 * 
 * All of this can be setup from using parameters  passed to the program from
 * command line (FPE --help for a full list).
 * 
 * Dependencies: C11++, classes (CFileTask, Redirect, CIFileApprise), Linux, Boost C++ Libraries.
 *
 * Copyright 2016.
 *
 */

// =============
// INCLUDE FILES
// =============

#include "FPE.hpp"

// ===============
// LOCAL FUNCTIONS
// ===============

//
// Standard cout for string of vectors. All calls to this function from different
// threads are guarded by mutex mOutput (this is static but local to the function).
//

void coutstr(const std::vector<std::string>& outstr) {

    static std::mutex mOutput;
    std::lock_guard<std::mutex> locker(mOutput);

    if (!outstr.empty()) {

        for (auto str : outstr) {
            std::cout << str;
        }

        std::cout << std::endl;

    }

}

//
// Standard cerr for string of vectors. All calls to this function from different
// threads are guarded by mutex mError (this is static but local to the function).
//

void cerrstr(const std::vector<std::string>& errstr) {

    static std::mutex mError;
    std::lock_guard<std::mutex> locker(mError);

    if (!errstr.empty()) {

        for (auto str : errstr) {
            std::cerr << str;
        }

        std::cerr << std::endl;

    }

}

//
// Get string for current date time
//

const std::string currentDateAndTime() {

    return(pt::to_simple_string(pt::second_clock::local_time()));

}

//
// Add timestamp to coutstr output
//

void coutstrTimeStamped(const std::vector<std::string>& outstr) {

    if (!outstr.empty()) {
        std::vector<std::string> newstr { "[" + currentDateAndTime() + "]" };
        newstr.insert(newstr.end(), outstr.begin(), outstr.end() );
        coutstr(newstr);
    }

}

//
// Add timestamp to cerrstr output
//

void cerrstrTimeStamped(const std::vector<std::string>& errstr) {

    if (!errstr.empty()) {
        std::vector<std::string> newstr { "[" + currentDateAndTime() + "]" };
        newstr.insert(newstr.end(), errstr.begin(), errstr.end() );
        cerrstr(errstr);
    }
}

//
// Create task and run in thread.
//

void createTaskAndRun(const std::string& taskName, ParamArgData& argData, CFileTask::TaskActionFcn taskActFcn) {

    // ASSERT if strings length 0 , pointer parameters NULL

    assert(taskName.length() != 0);
    assert(taskActFcn != nullptr);

    // Create function data (wrap in void shared pointer for passing to task).

    std::shared_ptr<void> fnData(new ActFnData{argData.watchFolder,
        argData.destinationFolder, argData.commandToRun, argData.bDeleteSource,
        argData.extension, ((argData.bQuiet) ? nullptr : coutstrTimeStamped), ((argData.bQuiet) ? nullptr : cerrstrTimeStamped)});

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

    try {

        // std::cout to logfile if parameter specified.
        
        Redirect logFile{std::cout};

        // Process FPE command line arguments.

        ParamArgData argData;

        procCmdLine(argc, argv, argData);

        // FPE up and running

        coutstr({"FPE Running..."});

        // Display BOOST version

        coutstr({"Using Boost ",
            std::to_string(BOOST_VERSION / 100000), ".", // major version
            std::to_string(BOOST_VERSION / 100 % 1000), ".", // minor version
            std::to_string(BOOST_VERSION % 100)}); // patch level

        // Create watch folder for task.

        if (!fs::exists(argData.watchFolder)) {
            coutstr({"Watch folder [", argData.watchFolder, "] DOES NOT EXIST."});
            if (fs::create_directory(argData.watchFolder)) {
                coutstr({"Creating watch folder [", argData.watchFolder, "]"});
            }
        }

        // Create destination folder for task

        if (!fs::exists(argData.destinationFolder)) {
            coutstr({"Destination folder ", argData.destinationFolder, " does not exist."});
            if (fs::create_directory(argData.destinationFolder)) {
                coutstr({"Creating destination folder ", argData.destinationFolder});
            }
        }

        // Signal file copy task

        if (argData.bFileCopy) {
            coutstr({"*** FILE COPY TASK ***"});
        }

        // Signal video conversion task

        if (argData.bVideoConversion) {
            coutstr({"*** VIDEO CONVERSION TASK ***"});
        }

        // Signal run command task

        if (argData.bRunCommand) {
            coutstr({"*** RUN COMMAND TASK ***"});
        }

        // Signal quiet mode

        if (argData.bQuiet) {
            coutstr({"*** QUIET MODE ***"});
        }

        // Signal source will be deleted on success

        if (argData.bDeleteSource) {
            coutstr({"*** DELETE SOURCE FILE ON SUCESSFUL PROCESSING ***"});
        }

        // Signal using single thread

        if (argData.bSingleThread) {
            coutstr({"*** SINGLE THREAD ***"});
        }

        // Signal using killCount

        if (argData.killCount) {
            coutstr({"*** KILL COUNT = ", std::to_string(argData.killCount), " ***"});
        }

        // Output to log file ( Redirect(std::cout) is the simplest solution). Once the try is exited
        // Redirect object will be destroyed and cout restored.

        if (!argData.logFileName.empty()) {
            coutstr({"*** LOG FILE = ", argData.logFileName, " ***"});
            logFile.change(argData.logFileName, std::ios_base::out | std::ios_base::app);
            coutstr({std::string(100, '=')});
        }

        // Create task object

        if (argData.bFileCopy) {
            createTaskAndRun(std::string("File Copy"), argData, copyFile);
        } else if (argData.bVideoConversion) {
            createTaskAndRun(std::string("Video Conversion"), argData, handBrake);
        } else {
            createTaskAndRun(std::string("Run Command"), argData, runCommand);
        }

    //
    // Catch any errors
    //    

    } catch (const fs::filesystem_error & e) {
        cerrstr({"BOOST file system exception occured: [", e.what(), "]"});
    } catch (const std::system_error &e) {
        cerrstr({"Caught a runtime_error exception: [", e.what(), "]"});
    } catch (const std::exception & e) {
        cerrstr({"Standard exception occured: [", e.what(), "]"});
    }

    coutstr({"FPE Exiting."});

    exit(0);

} 