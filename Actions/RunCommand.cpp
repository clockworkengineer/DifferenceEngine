#include "HOST.hpp"
/*
 * File:   RunCommand.cpp
 *
 * Author: Robert Tizzard
 * 
 * Created on October 24, 2016, 2:34 PM
 *
 * Copyright 2016.
 *
 */

//
// Module: RunCommand
//
// Description Take passed in file and run shell a shell command on it.
// 
// Dependencies:
// 
// C11++              : Use of C11++ features.
// Linux              : Target platform
// Boost              : File system.
//

// =============
// INCLUDE FILES
// =============

//
// C++ STL
//

#include <iostream>

//
// Program components.
//

#include "FPE.hpp"
#include "FPE_Actions.hpp"

//
// Process wait
//

#include <sys/wait.h>

//
// Boost file system, format library
//

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

namespace FPE_TaskActions {

    // =======
    // IMPORTS
    // =======

    using namespace std;

    using namespace FPE;

    namespace fs = boost::filesystem;

    // ===============
    // LOCAL VARIABLES
    // ===============

    // ===============
    // LOCAL FUNCTIONS
    // ===============

    //
    // Fork and execute Shell command
    //

    static int forkCommand(char *const argv[]) {

        pid_t pid; // Process id
        int status; // wait status
        int exitStatus = 0; // child exit status

        if ((pid = fork()) < 0) { /* fork a child process           */

            throw system_error(error_code(errno, system_category()), "Error: forking child process failed:");

        } else if (pid == 0) { /* for the child process: */

            // Redirect stdout/stderr

            freopen("/dev/null", "w", stdout);
            freopen("/dev/null", "w", stderr);

            if (execvp(*argv, argv) < 0) { /* execute the command  */
                exit(1);
            }

        } else { // for the parent:

            while (wait(&status) != pid) { /* wait for completion  */
                continue;
            }

            if (WIFEXITED(status)) { // Set any exit status
                exitStatus = WEXITSTATUS(status);
            }

        }

        return (exitStatus);

    }

    //
    // Run shell command. Split command into argv components before passing onto exec.
    // All heap memory cleaned up when function returns due to unique_pointers.
    //

    static int runShellCommand(const string& shellCommand) {

        int exitStatus = 0;
        int argc = 0;

        vector<char *> argvs;
        unique_ptr<char*> argv;
        unique_ptr<char> command{ new char[shellCommand.length() + 1]};

        // Take a 'C' string copy

        shellCommand.copy(command.get(), shellCommand.length());
        command.get()[shellCommand.length()] = 0; // null terminate

        // Loop through command splitting into substrings for argv

        char *p2 = strtok(command.get(), " ");

        while (p2) {
            argvs.push_back(p2);
            p2 = strtok(0, " ");
        }

        // Allocate argv array and copy vector over

        argv.reset(new char*[argvs.size() + 1]);

        for (char *arg : argvs) {
            argv.get()[argc++] = arg;
        }

        argv.get()[argc] = 0; // Last element null

        // Fork command

        exitStatus = forkCommand(argv.get());

        return (exitStatus);

    }
    // ================
    // PUBLIC FUNCTIONS
    // ================

    //
    // Run a specified command on the file (%1% source, %2% destination)
    //

    bool RunCommand::process(const string &file) {

        // ASSERT for any invalid options.

        assert(file.length() != 0);

        bool bSuccess = false;

        // Form source and destination file paths

        fs::path sourceFile(file);
        fs::path destinationFile(this->m_actionData[kDestinationOption] + sourceFile.filename().string());

        // Create correct command for whether source and destination specified or just source or none

        bool srcFound = (this->m_actionData[kCommandOption].find("%1%") != string::npos);
        bool dstFound = (this->m_actionData[kCommandOption].find("%2%") != string::npos);

        string command;
        if (srcFound && dstFound) {
            command = (boost::format(this->m_actionData[kCommandOption]) % sourceFile.string() % destinationFile.string()).str();
        } else if (srcFound) {
            command = (boost::format(this->m_actionData[kCommandOption]) % sourceFile.string()).str();
        } else {
            command = this->m_actionData[kCommandOption];
        }

        auto result = 0;
        if ((result = runShellCommand(command)) == 0) {
            bSuccess = true;
            cout << "Command success." << endl;
            if (!this->m_actionData[kDeleteOption].empty()) {
                cout << "Deleting Source [" << sourceFile.string() << "]" << endl;
                fs::remove(sourceFile);
            }

        } 

        return (bSuccess);
    }

} // namespace FPE_Actions

