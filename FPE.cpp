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
// 6) Import CSV file to MongoDB
// 
// All of this can be setup by using options  passed to the program from
// command line (FPE --help for a full list).
// 
// Dependencies:
// 
// C11++        : Use of C11++ features.
// Antik Classes: CTask, CRedirect, CFile, CPath.
// Linux        : Target platform
// Boost        : File system.
//

// =============
// INCLUDE FILES
// =============

//
// Program components.
//

#include "FPE.hpp"
#include "FPE_ProcCmdLine.hpp"
#include "FPE_TaskAction.hpp"

//
// Antik Classes
//

#include "CRedirect.hpp"
#include "CFile.hpp"
#include "CPath.hpp"

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
    using namespace FPE_TaskActions;

    // ===============
    // LOCAL FUNCTIONS
    // ===============

    //
    // Exit with error message/status
    //

    static void exitWithError(const string& errmsg) {

        // Closedown action functions, display error and exit.

        cerr << errmsg << endl;
        exit(EXIT_FAILURE);

    }


    //
    // Create task and run in thread.
    //

    static void createTaskAndLaunch(FPEOptions& options) {

        options.action->setActionData(options.map);
        
        // Create task object

        CTask task(options.map[kWatchOption], 
                   options.action, 
                   getOption<int>(options, kMaxDepthOption),
                   getOption<int>(options, kKillCountOption));

        // Create task object thread and start to watch else use FPE thread.

        if (getOption<bool>(options,kSingleOption)) {
            unique_ptr<thread> taskThread;
            taskThread.reset(new thread(&CTask::monitor, &task));
            taskThread->join();
        } else {
            task.monitor();
        }

        // If an exception occurred re-throw (end of chain)

        if (task.getThrownException()) {
            rethrow_exception(task.getThrownException());
        }

    }

    // ================
    // PUBLIC FUNCTIONS
    // ================

    void FileProcessingEngine(int argc, char** argv) {

        FPEOptions options {}; // Command line options  

        try {

            // cout to logfile if option specified.

            CRedirect logFile{cout};
            
            // Get FPE command line options.

            options = fetchCommandLineOptions(argc, argv);

            // FPE up and running

            cout << "FPE Running..." << endl;

            // Output to log file ( CRedirect(cout) is the simplest solution). 
            // Once the try is exited CRedirect object will be destroyed and 
            // cout restored.

            if (!options.map[kLogOption].empty()) {
                logFile.change(options.map[kLogOption], ios_base::out | ios_base::app);
                cout << string(100, '=') << endl;
            }

            // Create task object

            createTaskAndLaunch(options);

        //
        // Catch any errors
        //    

        } catch (const CFile::Exception &e) {
            exitWithError(e.what());
        } catch (const system_error &e) {
            exitWithError(e.what());
        } catch (const exception & e) {
            exitWithError(e.what());
        }

        cout << "FPE Exiting." << endl;


    }

} // namespace FPE

// ============================
// ===== MAIN ENTRY POINT =====
// ============================

int main(int argc, char** argv) {

    FPE::FileProcessingEngine(argc, argv);
    exit(EXIT_SUCCESS);

}
