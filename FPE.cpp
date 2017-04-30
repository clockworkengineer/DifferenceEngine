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
// 5) File append to ZIP archive
// 
// All of this can be setup by using parameters  passed to the program from
// command line (FPE --help for a full list).
// 
// Dependencies: C11++, Classes (CTask, CSMTP, CIMAP, CIMAPParse,
//               CFileZIP, CFileMIME, CLogger), Linux, Boost C++ Libraries.
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
// Boost file system library definitions
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

    namespace fs = boost::filesystem;

    // ===============
    // LOCAL FUNCTIONS
    // ===============

    //
    // Exit with error message/status
    //

    static void exitWithError(const string& errmsgStr) {

        // Closedown action functions, display error and exit.

        actionFuncCloseDown();

        CLogger::cerrstr({errmsgStr});

        exit(EXIT_FAILURE);

    }


    //
    // Create task and run in thread.
    //

    static void createTaskAndRun(const ParamArgData& argumentData) {

        // ASSERT if strings length 0 , pointer parameters NULL

        assert(argumentData.taskFunc.name.length() != 0);
        assert(argumentData.taskFunc.actFcn != nullptr);

        // Create function data (wrap in void shared pointer for passing to task).

        shared_ptr<void> fnData(new ActFnData{
            argumentData.watchFolderStr,
            argumentData.destinationFolderStr,
            argumentData.commandToRunStr,
            argumentData.bDeleteSource,
            argumentData.extensionStr,
            argumentData.userNameStr,
            argumentData.userPasswordStr,
            argumentData.serverURLStr,
            argumentData.emailRecipientStr,
            argumentData.mailBoxNameStr,
            argumentData.zipArchiveStr,
            ((argumentData.bQuiet) ? CLogger::noOp : CLogger::coutstr),
            ((argumentData.bQuiet) ? CLogger::noOp : CLogger::cerrstr)
        }
        );

        // Use function data to access set coutstr/cerrstr

        ActFnData *funcData = static_cast<ActFnData *> (fnData.get());

        // Set task options ( kill count and all output to locally defined  coutstr/cerrstr.

        shared_ptr<CTask::TaskOptions> options;

        options.reset(new CTask::TaskOptions{argumentData.killCount, funcData->coutstr, funcData->cerrstr});

        // Create task object

        CTask task(argumentData.taskFunc.name, argumentData.watchFolderStr, argumentData.taskFunc.actFcn, fnData, argumentData.maxWatchDepth, options);

        // Create task object thread and start to watch else use FPE thread.

        if (!argumentData.bSingleThread) {
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

        ParamArgData argumentData; // Command line arguments  

        try {

            // Date and Time stamp output

            CLogger::setDateTimeStamped(true);

            // Action function initialization.

            actionFuncInit();

            // cout to logfile if parameter specified.

            CRedirect logFile{cout};

            // Get FPE command line arguments.

            argumentData = fetchCommandLineArgumentData(argc, argv);

            // FPE up and running

            CLogger::coutstr({"FPE Running..."});

            // Display BOOST version

            CLogger::coutstr({"Using Boost ",
                to_string(BOOST_VERSION / 100000), ".", // major version
                to_string(BOOST_VERSION / 100 % 1000), ".", // minor version
                to_string(BOOST_VERSION % 100)}); // patch level

            // Process program argument data

            processArgumentData(argumentData);

            // Output to log file ( CRedirect(cout) is the simplest solution). 
            // Once the try is exited CRedirect object will be destroyed and 
            // cout restored.

            if (!argumentData.logFileNameStr.empty()) {
                logFile.change(argumentData.logFileNameStr, ios_base::out | ios_base::app);
                CLogger::coutstr({string(100, '=')});
            }

            // Create task object

            createTaskAndRun(argumentData);
    
        //
        // Catch any errors
        //    

        } catch (const fs::filesystem_error & e) {
            exitWithError(string("BOOST file system exception occured: [") + e.what() + "]");
        } catch (const system_error &e) {
            exitWithError(string("Caught a system_error exception: [") + e.what() + "]");
        } catch (const exception & e) {
            exitWithError(string("Standard exception occured: [") + e.what() + "]");
        }

        CLogger::coutstr({"FPE Exiting."});

        // Closedown action functions

        actionFuncCloseDown();


    }

} // namespace FPE

// ============================
// ===== MAIN ENTRY POINT =====
// ============================

int main(int argc, char** argv) {

    FPE::FileProcessingEngine(argc, argv);
    exit(EXIT_SUCCESS);

}
