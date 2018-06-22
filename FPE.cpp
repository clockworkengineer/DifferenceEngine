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
// added directories are also watched (ie.this is recursive) but any added files
// are be processed using one of its built in task action functions
// 
// 1) File copy
// 2) Video file conversion (using Handbrake)
// 3) Run shell command
// 4) Email file as a attachment ( or add file to mailbox for IMAP server)
// 5) File append to ZIP archive
// 
// All of this can be setup by using options  passed to the program from
// command line (FPE --help for a full list).
// 
// Dependencies:
// 
// C11++              : Use of C11++ features.
// Antikythera Classes: CTask, CSMTP, CIMAP, CIMAPParse, CZIP, CMIME, CLogger
// Linux              : Target platform
// Boost              : File system, program option.
//

// =============
// INCLUDE FILES
// =============

//
// Program components.
//

#include "FPE.hpp"
#include "FPE_ProcCmdLine.hpp"
#include "FPE_ActionFuncs.hpp"

//
// Antikythera Classes
//

#include "CRedirect.hpp"

//
// Boost file system library
//

#include <boost/filesystem.hpp>

// =========
// NAMESPACE
// =========

namespace FPE {

    // =======
    // IMPORTS
    // =======

    using namespace std;

    using namespace Antik::File;
    using namespace Antik::Util;

    using namespace FPE_ProcCmdLine;
    using namespace FPE_ActionFuncs;

    // ===============
    // LOCAL FUNCTIONS
    // ===============

    //
    // Exit with error message/status
    //

    static void exitWithError(const string& errmsgStr) {

        // Closedown action functions, display error and exit.

        CLogger::cerrstr({errmsgStr});
        exit(EXIT_FAILURE);

    }


    //
    // Create task and run in thread.
    //

    static void createTaskAndRun(FPEOptions& optionData) {

        // ASSERT if task name length == 0 , action function pointer == nullptr

        assert(optionData.action->name.length() != 0);
        assert(optionData.action != nullptr);

        // Create function data (wrap in void shared pointer for passing to task).

        shared_ptr<void> fnData(new ActFnData{optionData.optionsMap,
            (getOption<bool>(optionData,kQuietOption) ? CLogger::noOp : CLogger::coutstr),
            (getOption<bool>(optionData,kQuietOption) ? CLogger::noOp : CLogger::cerrstr) });

        // Use function data to access set coutstr/cerrstr

        ActFnData *funcData = static_cast<ActFnData *> (fnData.get());

        // Set task options ( kill count and all output to locally defined  coutstr/cerrstr).

        shared_ptr<CTask::TaskOptions> options;

        options.reset(new CTask::TaskOptions{getOption<int>(optionData, kKillCountOption)});

        // Create task object

        CTask task(optionData.action->name, 
                   optionData.optionsMap[kWatchOption], 
                   optionData.action, 
                   fnData, getOption<int>(optionData, kKillCountOption), options);

        // Create task object thread and start to watch else use FPE thread.

        if (getOption<bool>(optionData,kSingleOption)) {
            unique_ptr<thread> taskThread;
            taskThread.reset(new thread(&CTask::monitor, &task));
            taskThread->join();
        } else {
            task.monitor();
        }

        // If an exception occurred rethrow (end of chain)

        if (task.getThrownException()) {
            rethrow_exception(task.getThrownException());
        }

    }

    // ================
    // PUBLIC FUNCTIONS
    // ================

    void FileProcessingEngine(int argc, char** argv) {

        FPEOptions optionData; // Command line options  

        try {

            // Date and Time stamp output

            CLogger::setDateTimeStamped(true);

            // cout to logfile if option specified.

            CRedirect logFile{cout};

            // Get FPE command line options.

            optionData = fetchCommandLineOptionData(argc, argv);

            // Display BOOST version

            CLogger::coutstr({"*** boost version = [",
                to_string(BOOST_VERSION / 100000)+"."+
                to_string(BOOST_VERSION / 100 % 1000)+"."+
                to_string(BOOST_VERSION % 100)+"] ***"});

            // FPE up and running

            CLogger::coutstr({"FPE Running..."});

            // Output to log file ( CRedirect(cout) is the simplest solution). 
            // Once the try is exited CRedirect object will be destroyed and 
            // cout restored.

            if (!optionData.optionsMap[kLogOption].empty()) {
                logFile.change(optionData.optionsMap[kLogOption], ios_base::out | ios_base::app);
                CLogger::coutstr({string(100, '=')});
            }

            // Create task object

            createTaskAndRun(optionData);

        //
        // Catch any errors
        //    

        } catch (const boost::filesystem::filesystem_error & e) {
            exitWithError(string("BOOST file system exception occured: [") + e.what() + "]");
        } catch (const system_error &e) {
            exitWithError(string("Caught a system_error exception: [") + e.what() + "]");
        } catch (const exception & e) {
            exitWithError(string("Standard exception occured: [") + e.what() + "]");
        }

        CLogger::coutstr({"FPE Exiting."});


    }

} // namespace FPE

// ============================
// ===== MAIN ENTRY POINT =====
// ============================

int main(int argc, char** argv) {

    FPE::FileProcessingEngine(argc, argv);
    exit(EXIT_SUCCESS);

}
